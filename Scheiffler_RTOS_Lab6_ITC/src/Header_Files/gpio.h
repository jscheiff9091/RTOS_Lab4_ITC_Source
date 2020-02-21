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

// ----- Typedefs ------
typedef enum
{
	GPIO_BTNReleased,
	GPIO_BTNPressed
}GPIO_BTNState_t;

typedef enum
{
	LED_BOTH_OFF,
	LED0_ON,
	LED1_ON
}LED_Action_t;

typedef enum
{
	LED_OFF,
	LED_ON
}LED_State_t;

// ----- Global Variables ------
extern GPIO_BTNState_t btn0_state;
extern GPIO_BTNState_t btn1_state;

extern OS_TCB buttonInputTaskTCB;
extern OS_TCB LEDDriverTaskTCB;

extern CPU_STK buttonInputTaskStack[BTN_IN_STACK_SIZE];
extern CPU_STK LEDDriverTaskStack[LED_DRV_STACK_SIZE];

// ----- Function Prototypes ------
void GPIO_InitLEDs();

void GPIO_InitBTNs();

GPIO_BTNState_t GPIO_GetBTNState(uint8_t btn);

LED_Action_t GPIO_GetButtonAction(void);

void SetLEDs(LED_Action_t btn_action, LED_Action_t sld_action);

void GPIO_ODD_IRQHandler(void);

void GPIO_EVEN_IRQHandler(void);

void ButtonInputTask(void* p_args);

void LEDDriverTask(void* p_args);


#endif /* SRC_HEADER_FILES_GPIO_H_ */
