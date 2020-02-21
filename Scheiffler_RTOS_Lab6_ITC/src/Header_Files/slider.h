/*
 * slider.h
 *
 *  Created on: Jan 23, 2020
 *      Author: Jacob S
 */

#ifndef SLIDER_H_
#define SLIDER_H_

// ----- Included Files ------
#include <kernel/include/os.h>
#include "gpio.h"

// ----- Macros ------
#define POS0			0
#define POS1			1
#define POS2			2
#define POS3			3

#define SLD_IN_TASK_PRIO 		18u
#define SLD_IN_STACK_SIZE		1000u
#define SLD_IN_TIME_DLY			100u

// ----- Typedefs ------
typedef enum
{
	SLD_Released,
	SLD_Pressed
}SLD_SliderPressedState_t;

typedef enum
{
	SLD_LeftSide,
	SLD_RightSide
}SLD_SliderSide_t;

// ----- Global Variables ------
extern SLD_SliderPressedState_t sld_rightSideState;
extern SLD_SliderPressedState_t sld_leftSideState;

extern OS_TCB sliderInputTaskTCB;
extern CPU_STK sliderInputTaskStack[SLD_IN_STACK_SIZE];

// ----- Function Prototypes ------
void SLD_Init(void);

SLD_SliderPressedState_t SLD_IsPressed(SLD_SliderSide_t side);

LED_Action_t SLD_GetSLDAction(void);

void SliderInputTask(void* p_args);


#endif /* SRC_HEADER_FILES_SLIDER_H_ */
