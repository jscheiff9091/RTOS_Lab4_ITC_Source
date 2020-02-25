/*
 * slider.c
 *
 *  Created on: Jan 23, 2020
 *      Author: Jacob S
 */
#include "slider.h"
#include "gpio.h"
#include "capsense.h"
#include <kernel/include/os.h>
#include  <common/include/rtos_utils.h>

//SLD_SliderPressedState_t sld_rightSideState;
//SLD_SliderPressedState_t sld_leftSideState;

OS_TCB sliderInputTaskTCB;
CPU_STK sliderInputTaskStack[SLD_IN_STACK_SIZE];

OS_SEM sliderSem;
OS_TMR sliderTimer;

/* Slider Initialize */
void SLD_Init(void) {
	CAPSENSE_Init();			//Enable capctive sensing slider
}

/* Slider is Pressed? */
SLD_SliderPressedState_t SLD_IsPressed(SLD_SliderSide_t side) {

	if(side != SLD_LeftSide && side != SLD_RightSide) {
		return SLD_Released;
	}

	CAPSENSE_Sense();                  																//Read Sliders

	if(side == SLD_LeftSide && (CAPSENSE_getPressed(POS0) || CAPSENSE_getPressed(POS1))) {         	//Checking left side and one or both of the two left side positions are pressed
		return SLD_Pressed;
	}
	else if(side == SLD_RightSide && (CAPSENSE_getPressed(POS2) || CAPSENSE_getPressed(POS3))) {   	//Checking right side and one or both of the right side positions are pressed
		return SLD_Pressed;
	}

	return SLD_Released;
}

/* Slider - Get Slider Action */
LED_Action_t SLD_GetSLDAction(SLD_SliderPressedState_t sld_leftSideState, SLD_SliderPressedState_t sld_rightSideState) {
	//Determine which combination of left and right side is selected (or not)
	if(sld_leftSideState == SLD_Pressed && sld_rightSideState == SLD_Pressed) { 					//Both side selected, turn off LEDs
		return LED_BOTH_OFF;
	}
	else if(sld_leftSideState == SLD_Pressed && sld_rightSideState == SLD_Released) {				//Left side pressed, right side released, turn on LED0
		return LED0_ON;
	}
	else if(sld_leftSideState == SLD_Released && sld_rightSideState == SLD_Pressed) {				//LEft side released, right side pressed, turn on LED1
		return LED1_ON;
	}
	else {																//Neither side is pressed, turn both off
		return LED_BOTH_OFF;
	}
}

/* Touch slider signal timer */
void SLD_TimerCallback(void* p_tmr, void* p_args) {
	RTOS_ERR err;

	// Signal to slider input task to check touch slider state
	OSSemPost(&sliderSem, OS_OPT_POST_1, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
}

/* Slider Input Task */
void SliderInputTask(void * p_args) {

	RTOS_ERR err;
	CPU_TS timestamp;

	SLD_Init();       	//Initialize CAPSENSE driver and set initial slider state
	SLD_SliderPressedState_t sld_leftSideState = SLD_IsPressed(SLD_LeftSide);
	SLD_SliderPressedState_t sld_rightSideState = SLD_IsPressed(SLD_RightSide);
	LED_Action_t currAction = SLD_GetSLDAction(sld_leftSideState, sld_rightSideState);
	GPIO_LEDTaskMsg_t sliderMsg;
	sliderMsg.msgSource = SliderTaskMessage;
	sliderMsg.ledAction = currAction;

	//Create semaphore used to signal to check the touch slider
	OSSemCreate(&sliderSem,
			  	"Touch Slider Signal Semaphore",
				CNT_ZERO,
				&err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

	//Create slider value timer
	OSTmrCreate(&sliderTimer,
				"Periodic Slider Timer",
				NO_DLY,
				SLD_TIMER_CNT,
				OS_OPT_TMR_PERIODIC,
				&SLD_TimerCallback,
				DEF_NULL,
				&err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

	OSTmrStart(&sliderTimer, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

	while(1) {
		// Wait for timer to expire and signal the touch slide to be measured
		OSSemPend(&sliderSem, NO_TIMEOUT, OS_OPT_PEND_BLOCKING, &timestamp, &err);

		// Update slider state variables
		if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE) {
			sld_leftSideState = SLD_IsPressed(SLD_LeftSide);   		//Update left side of slider state variable
			sld_rightSideState = SLD_IsPressed(SLD_RightSide);		//update right side of slider state variable
		}
		else return;

		//Update temporary LED action variable
		currAction = SLD_GetSLDAction(sld_leftSideState, sld_rightSideState);

		if(currAction != sliderMsg.ledAction) {
			//Update LED state variable
			sliderMsg.ledAction = currAction;

			//Send message to LED driver task message
			OSQPost(&LEDTaskMsgQ,
					(void*) &sliderMsg,
					(OS_MSG_SIZE)sizeof(void*),
					OS_OPT_POST_ALL,
					&err);
			APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
		}
	}
}
