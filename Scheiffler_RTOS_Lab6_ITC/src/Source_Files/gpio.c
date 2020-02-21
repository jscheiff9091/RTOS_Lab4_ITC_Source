/*
 * gpio.c
 *
 *  Created on: Jan 23, 2020
 *      Author: Jacob S
 */
#include "em_gpio.h"
#include "slider.h"
#include "gpio.h"
#include  <common/include/rtos_utils.h>

// Global Variables
GPIO_BTNState_t btn0_state;
GPIO_BTNState_t btn1_state;

OS_TCB buttonInputTaskTCB;
OS_TCB LEDDriverTaskTCB;

CPU_STK buttonInputTaskStack[BTN_IN_STACK_SIZE];
CPU_STK LEDDriverTaskStack[LED_DRV_STACK_SIZE];

//----- Function Definitions -----

//Initialize LED0 and LED1 on the Pearl Gecko starter kit
void GPIO_InitLEDs() {
	//LED0
	GPIO_DriveStrengthSet(LED0_PORT, gpioDriveStrengthStrongAlternateStrong);	//Set drive strength
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, LED_DEFAULT);		//Set LED0 pin Mode														//Initialize LED0 state

	//LED1
	GPIO_DriveStrengthSet(LED1_PORT, gpioDriveStrengthStrongAlternateStrong);	//Set drive strength
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, LED_DEFAULT);		//Set LED1 pin mode														//Initialize LED1 state
}

//Initialize BTN0 and BTN1 on the Pearl Gecko starter kit
void GPIO_InitBTNs() {
	GPIO_PinModeSet(BTN0_PORT, BTN0_PIN, gpioModeInputPullFilter, PULLUP);		//Set buttons to input with pullup resistor (pressed means bit is low)
	GPIO_PinModeSet(BTN1_PORT, BTN1_PIN, gpioModeInputPullFilter, PULLUP);
	btn0_state = GPIO_GetBTNState(BTN0);										//Initialize button state variables
	btn1_state = GPIO_GetBTNState(BTN1);
}

//Get BTN State
GPIO_BTNState_t GPIO_GetBTNState(uint8_t btn) {
	if(btn != BTN0 && btn != BTN1) {    					//Ensure btn passed is BTN1 or BTN0
		return GPIO_BTNReleased;
	}

	uint32_t GPIO_PortFVal = GPIO_PortInGet(BTN_PORT); 		//Get value of DIN Register for port F
	if(btn == BTN0 && (~GPIO_PortFVal & BTN0_BIT)) {   		//If btn is BTN0 and BTN0 bit is a zero, BTN 0 Pressed
		return GPIO_BTNPressed;
	}
	else if(btn == BTN1 && (~GPIO_PortFVal & BTN1_BIT)) {  	//If btn is BTN1 and BTN1 bit is a zero, BTN1 Pressed
		return GPIO_BTNPressed;
	}

	return GPIO_BTNReleased;                              	//Return default value, that button is released
}

//Get desired action of the button selections
LED_Action_t GPIO_GetButtonAction(void) {
	//Determine the LEDs that should be on based on the button state
	if(btn0_state == GPIO_BTNPressed && btn1_state == GPIO_BTNPressed) {				//Both BTN0 and BTN1 are pressed
		return LED_BOTH_OFF;
	}
	else if(btn0_state == GPIO_BTNPressed && btn1_state != GPIO_BTNPressed) {			//BTN0 pressed and BTN1 is not pressed
		return LED0_ON;
	}
	else if(btn0_state != GPIO_BTNPressed && btn1_state == GPIO_BTNPressed) {			//BTN0 is not pressed and BTN1 pressed
		return LED1_ON;
	}
	else {																	//Neither BTN0 nor BTN1 are pressed
		return LED_BOTH_OFF;
	}
}

//Turn LEDs on/off based off of desired actions of slider and buttons
void SetLEDs(LED_Action_t btn_action, LED_Action_t sld_action) {
	if(btn_action == LED_BOTH_OFF && sld_action == LED_BOTH_OFF) {          	//Both the buttons and slider want both LEDs off
		GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED0_ON && sld_action == LED_BOTH_OFF) {				//Button 0 pressed, Slider is either both pressed or neither
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED1_ON && sld_action == LED_BOTH_OFF) {				//Button 1 pressed, Slider is either both pressed or neither
		GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED_BOTH_OFF && sld_action == LED0_ON) {				//Buttons either both pressed or neither, Left side of slider pressed down
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED_BOTH_OFF && sld_action == LED1_ON) {				//Buttons either both pressed or neither, Right side of slider pressed down
		GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED0_ON && sld_action == LED0_ON) {					//Button 0 pressed and left side of slider pressed
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED0_ON && sld_action == LED1_ON) {                   //Button 0 pressed and right side of slider pressed
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED1_ON && sld_action == LED0_ON) { 					//Button 1 pressed and left side of slider pressed
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else if(btn_action == LED1_ON && sld_action == LED1_ON) {					//Button 1 pressed and right side of slider pressed
		GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
}

void GPIO_EVEN_IRQHandler(void) {
	uint32_t gpio_int = GPIO_IntGet();				//Read IF register
	GPIO_IntClear(gpio_int);						//Set IFC register
	for(int i = 0; i < 5000; i++);     				//Button Debounce

	if(gpio_int & (1 << BTN0_INT)) {				//Button 0 interrupt triggered, update its state variable
		btn0_state = GPIO_GetBTNState(BTN0);
	}
	else if(gpio_int & (1 << BTN1_INT)) {			//Button 1 interrupt triggered, update its state variable
		btn1_state = GPIO_GetBTNState(BTN1);
	}
}

/* Get state of buttons every 100ms */
void ButtonInputTask(void* p_args) {

	RTOS_ERR err;
	PP_UNUSED_PARAM(p_args);       				//Prevent compiler warning.

	GPIO_InitBTNs();							//Enable push buttons

	while(1) {
		btn0_state = GPIO_GetBTNState(BTN0);	//Update button state variables
		btn1_state = GPIO_GetBTNState(BTN1);

		OSTimeDly(BTN_IN_TASK_DLY,
				  OS_OPT_TIME_DLY,
				  &err);						//Delay button input task for 100ms
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
	}
}

/* Determine State of LEDs based on button and slider inputs every 100 ms */
void LEDDriverTask(void* p_args) {

	RTOS_ERR err;
	PP_UNUSED_PARAM(p_args);       				//Prevent compiler warning.

	LED_Action_t btn_action;					//Create variables to
	LED_Action_t sld_action;
	GPIO_InitLEDs();							//Enable LEDs

	while(1) {
		btn_action = GPIO_GetButtonAction();	//Get desired LED action from buttons
		sld_action = SLD_GetSLDAction();		//Get desired LED action from slider

		SetLEDs(btn_action, sld_action);		//Set LEDs based on button and slider desired LED actions

		OSTimeDly(LED_DRV_TASK_DLY,
				  OS_OPT_TIME_DLY,
				  &err);						//Delay LED Driver task for 100ms
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
	}
}

