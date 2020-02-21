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

SLD_SliderPressedState_t sld_rightSideState;
SLD_SliderPressedState_t sld_leftSideState;

OS_TCB sliderInputTaskTCB;
CPU_STK sliderInputTaskStack[SLD_IN_STACK_SIZE];


//Initialize the slider and slider state variables
void SLD_Init(void) {
	CAPSENSE_Init();			//Enable capctive sensing slider

	sld_leftSideState = SLD_IsPressed(SLD_LeftSide);
	sld_rightSideState = SLD_IsPressed(SLD_RightSide);
}

//Check if either side of the slider is pressed
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

//Determine action desired from the slider input
LED_Action_t SLD_GetSLDAction(void) {
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

// Get state of Slider every 100 ms
void SliderInputTask(void * p_args) {

	RTOS_ERR err;
	SLD_Init();       	//Initialize CAPSENSE driver and set initial slider state

	while(1) {
		sld_leftSideState = SLD_IsPressed(SLD_LeftSide);   		//Update left side of slider state variable
		sld_rightSideState = SLD_IsPressed(SLD_RightSide);		//update right side of slider state variable

		OSTimeDly(SLD_IN_TIME_DLY,
				  OS_OPT_TIME_DLY,
				  &err);										//Delay slider input task for 100ms
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
	}
}
