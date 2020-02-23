/*
 * gpio.h
 *
 *  Created on: Jan 23, 2020
 *      Author: Jacob S
 */

#ifndef GPIO_H_
#define GPIO_H_

// ----- Included Files ------
#include <stdint.h>
#include <kernel/include/os.h>
#include "main.h"

// ----- Macros ------
#define LED0_PORT 		gpioPortF
#define LED0_PIN		4u
#define LED1_PORT		gpioPortF
#define LED1_PIN		5u

#define BTN0_PORT		gpioPortF
#define BTN0_PIN		6u
#define BTN1_PORT		gpioPortF
#define BTN1_PIN		7u
#define BTN_PORT 		gpioPortF
#define BTN0			0u
#define BTN1			1u
#define BTN0_BIT		0x40
#define BTN1_BIT		0x80
#define BTN0_INT		4
#define BTN1_INT		6

#define LED_DEFAULT		0
#define PULLUP			1

#define BTN_IN_TASK_PRIO 		17u
#define BTN_IN_STACK_SIZE		1000u
#define BTN_IN_TASK_DLY			100u

#define LED_DRV_TASK_PRIO 		19u
#define LED_DRV_STACK_SIZE		1000u
#define	LED_DRV_TASK_DLY		100u

#define LED_DRV_Q_SIZE			10u

#define BTN0_EVENT				(1 << 0u)
#define BTN1_EVENT				(1 << 1u)
#define BTN_EVENT_ALL			(BTN0_EVENT | BTN1_EVENT)
#define BTN_EVENT_NONE			0

#define NO_TIMEOUT				0

// ----- Typedefs ------
/// @brief enumeration to track possible button states
typedef enum
{
	GPIO_BTNReleased,
	GPIO_BTNPressed
}GPIO_BTNState_t;

/// @brief enumeration to track possible LED actions from buttons and touch slider
typedef enum
{
	LED_BOTH_OFF,
	LED0_ON,
	LED1_ON
}LED_Action_t;

/// @brief enumeration to track states of the LEDs
typedef enum
{
	LED_OFF,
	LED_ON
}LED_State_t;

/// @brief enumeration to identify the source of the LED message
typedef enum
{
	SliderTaskMessage,
	ButtonTaskMessage
}GPIO_LEDTaskMsgSource_t;

/// @brief structure which holds LED Driver Task Messages
typedef struct
{
	GPIO_LEDTaskMsgSource_t msgSource;
	LED_Action_t ledAction;
}GPIO_LEDTaskMsg_t;

// ----- Global Variables ------
//extern GPIO_BTNState_t btn0_state;                         	/**< Variable to hold state of button 0*/
//extern GPIO_BTNState_t btn1_state;         					/**< Variable to hold state of button 1*/

extern OS_TCB buttonInputTaskTCB;							/**< Button input task control block variable */
extern OS_TCB LEDDriverTaskTCB;								/**< LED driver task control block variable */

extern CPU_STK buttonInputTaskStack[BTN_IN_STACK_SIZE];		/**< Button input task stack */
extern CPU_STK LEDDriverTaskStack[LED_DRV_STACK_SIZE]; 		/**< LED driver task stack */

extern OS_Q LEDTaskMsgQ;									/**< LED driver task message queue */

// ----- Function Prototypes ------
/// @brief Initialize LED0 and LED1 on the Pearl Gecko starter kit
///
void GPIO_InitLEDs();


/// @brief Initialize BTN0 and BTN1 on the Pearl Gecko starter kit
///
void GPIO_InitBTNs();


/// @brief Get State of external button switch
///
/// @param[in] external switch number
///
/// @return the state of the external switch
GPIO_BTNState_t GPIO_GetBTNState(uint8_t btn);


/// @brief Get desired action of the button selections
///
/// @param[in] state of button 0
/// @param[in] state of button 1
///
/// @return the selection of LEDs the buttons wish to drive
LED_Action_t GPIO_GetButtonAction(GPIO_BTNState_t btn0State, GPIO_BTNState_t btn1State);


/// @brief Turn LEDs on/off based off of desired actions of slider and buttons
///
/// @param[in] desired LED state indicated by the external button switches
/// @param[in] desired LED state indicated by the touch slider
void SetLEDs(LED_Action_t btn_action, LED_Action_t sld_action);


/// @brief ISR for the even gpio interrupts
///
void GPIO_EVEN_IRQHandler(void);


/// @brief Task which updates the button state variables
///
/// @param[in] pointer to arguments
void ButtonInputTask(void* p_args);


/// @brief Task to determine state of LEDs based on button and slider inputs
///
/// @param[in] pointer to arguments
void LEDDriverTask(void* p_args);


#endif /* SRC_HEADER_FILES_GPIO_H_ */
