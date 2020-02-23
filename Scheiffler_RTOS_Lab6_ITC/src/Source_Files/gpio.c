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
#include <stdlib.h>

// Global Variables
//GPIO_BTNState_t btn0_state;
//GPIO_BTNState_t btn1_state;

OS_TCB buttonInputTaskTCB;
OS_TCB LEDDriverTaskTCB;

CPU_STK buttonInputTaskStack[BTN_IN_STACK_SIZE];
CPU_STK LEDDriverTaskStack[LED_DRV_STACK_SIZE];

OS_Q LEDTaskMsgQ;
OS_FLAG_GRP buttonEvent;

//----- Function Definitions -----


/* GPIO - Init LEDs */
void GPIO_InitLEDs() {
	//LED0
	GPIO_DriveStrengthSet(LED0_PORT, gpioDriveStrengthStrongAlternateStrong);	//Set drive strength
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, LED_DEFAULT);		//Set LED0 pin Mode														//Initialize LED0 state

	//LED1
	GPIO_DriveStrengthSet(LED1_PORT, gpioDriveStrengthStrongAlternateStrong);	//Set drive strength
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, LED_DEFAULT);		//Set LED1 pin mode														//Initialize LED1 state
}


/* GPIO - Initialize Buttons */
void GPIO_InitBTNs() {
	GPIO_PinModeSet(BTN0_PORT, BTN0_PIN, gpioModeInputPullFilter, PULLUP);		//Set buttons to input with pullup resistor (pressed means bit is low)
	GPIO_PinModeSet(BTN1_PORT, BTN1_PIN, gpioModeInputPullFilter, PULLUP);

	GPIO_ExtIntConfig(BTN0_PORT, BTN0_PIN, BTN0_INT, true, true, true);			//Config falling and rising edge interrupts for buttons
	GPIO_ExtIntConfig(BTN1_PORT, BTN1_PIN, BTN1_INT, true, true, true);

	NVIC_EnableIRQ(GPIO_EVEN_IRQn);												//Enable interrupt in the NVIC
}


/* GPIO - Get Button State */
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


/* GPIO - Get Button Action */
LED_Action_t GPIO_GetButtonAction(GPIO_BTNState_t btn0State, GPIO_BTNState_t btn1State) {
	//Determine the LEDs that should be on based on the button state
	if(btn0State == GPIO_BTNPressed && btn1State == GPIO_BTNPressed) {				//Both BTN0 and BTN1 are pressed
		return LED_BOTH_OFF;
	}
	else if(btn0State == GPIO_BTNPressed && btn1State != GPIO_BTNPressed) {			//BTN0 pressed and BTN1 is not pressed
		return LED0_ON;
	}
	else if(btn0State != GPIO_BTNPressed && btn1State == GPIO_BTNPressed) {			//BTN0 is not pressed and BTN1 pressed
		return LED1_ON;
	}
	else {																	//Neither BTN0 nor BTN1 are pressed
		return LED_BOTH_OFF;
	}
}


/* Set LEDs */
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


/* GPIO Even IRQ Handler */
void GPIO_EVEN_IRQHandler(void) {
	OSIntEnter();                                   //Let OS know it is currently executing an ISR

	uint32_t gpio_int = GPIO_IntGet();				//Read IF register
	GPIO_IntClear(gpio_int);						//Set IFC register
	for(int i = 0; i < 5000; i++);     				//Button Debounce
	RTOS_ERR err;

	if(gpio_int & (1 << BTN0_INT)) {				//Button 0 interrupt triggered, update its state variable
		OSFlagPost(&buttonEvent, BTN0_EVENT, OS_OPT_POST_FLAG_SET, &err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	}
	else if(gpio_int & (1 << BTN1_INT)) {			//Button 1 interrupt triggered, update its state variable
		OSFlagPost(&buttonEvent, BTN1_EVENT, OS_OPT_POST_FLAG_SET, &err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	}

	OSIntExit();									//Exit ISR
}


/* Button Input Task */
void ButtonInputTask(void* p_args) {

	RTOS_ERR err;
	PP_UNUSED_PARAM(p_args);       							//Prevent compiler warning.

	CPU_TS timestamp;
	OS_FLAGS btnEventFlags = BTN_EVENT_NONE;
	OSFlagCreate(&buttonEvent, "Button Event Group", btnEventFlags, &err);  	//Create button event flags
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	GPIO_InitBTNs();										//Enable push buttons

	GPIO_BTNState_t btn0State = GPIO_GetBTNState(BTN0);		//Initialize button state variables
	GPIO_BTNState_t btn1State = GPIO_GetBTNState(BTN1);
	GPIO_LEDTaskMsg_t btnMessage;
	btnMessage.msgSource = ButtonTaskMessage;

	while(1) {
		//Block until a button event is received
		btnEventFlags = OSFlagPend(&buttonEvent,
								   BTN_EVENT_ALL,
								   NO_TIMEOUT,
								   OS_OPT_PEND_FLAG_SET_ANY,
								   &timestamp,
								   &err);
		if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE) {
			OSFlagPost(&buttonEvent, btnEventFlags, OS_OPT_POST_FLAG_CLR, &err);
			APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

			// Update appropriate button state variable(s)
			if((btnEventFlags & BTN1_EVENT) && (btnEventFlags & BTN0_EVENT)) {
				btn0State = GPIO_GetBTNState(BTN0);					//Update button state variables
				btn1State = GPIO_GetBTNState(BTN1);
			}
			else if(btnEventFlags & BTN0_EVENT) btn0State = GPIO_GetBTNState(BTN0);
			else btn1State = GPIO_GetBTNState(BTN1);
		}
		else return;

		//Update message variable
		btnMessage.ledAction = GPIO_GetButtonAction(btn0State, btn1State);

		//Send message to LED driver task
		OSQPost(&LEDTaskMsgQ,
				(void*) &btnMessage,
				(OS_MSG_SIZE)sizeof(void*),
				OS_OPT_POST_ALL,
				&err);
		APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
	}
}


/* LED Driver Task */
void LEDDriverTask(void* p_args) {

	RTOS_ERR err;
	PP_UNUSED_PARAM(p_args);       				//Prevent compiler warning.

	OS_MSG_SIZE msgSize;                     	//Size of message received (in bytes)
	CPU_TS timestamp;
	void* rawMsg;                               //Raw message pulled from the queue
	GPIO_LEDTaskMsg_t* msg;                     //Message pulled from queue
	bool msgsReceived;                           //Message Received?

	OSQCreate(&LEDTaskMsgQ,
			  "LED Driver Message Queue",
			  LED_DRV_Q_SIZE,
			  &err);                            //Create the message queue
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	LED_Action_t btnAction = LED_BOTH_OFF;		//Create variables to track desired LED action from buttons and slider
	LED_Action_t sldAction = LED_BOTH_OFF;
	GPIO_InitLEDs();							//Enable LEDs

	while(1) {
		//Wait until a message is in the queue
		rawMsg = OSQPend(&LEDTaskMsgQ,
				         NO_TIMEOUT,
						 OS_OPT_PEND_BLOCKING,
						 &msgSize,
						 &timestamp,
						 &err);

		//Ensure message received properly and cast message appropriately
		if(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE) msg = (GPIO_LEDTaskMsg_t*) rawMsg;
		else return;

		//Update the appropriate action variable
		if(msg->msgSource == SliderTaskMessage) sldAction = msg->ledAction;
		else btnAction = msg->ledAction;

		//Continue to read messages until the queue is empty
		msgsReceived = false;

		while(!msgsReceived) {
			rawMsg = OSQPend(&LEDTaskMsgQ, 1, OS_OPT_PEND_NON_BLOCKING, &msgSize, &timestamp, &err);
			//APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

			//If queue non-empty pointer will not be NULL, get appropriate LED action, else all messages received, update LEDs
			if(rawMsg != NULL) {
				msg = (GPIO_LEDTaskMsg_t*) rawMsg;
				if(msg->msgSource == SliderTaskMessage) sldAction = msg->ledAction;
				else btnAction = msg->ledAction;
			}
			else msgsReceived = true;
		}

		SetLEDs(btnAction, sldAction);			//Set LEDs based on button and slider desired LED actions
	}
}

