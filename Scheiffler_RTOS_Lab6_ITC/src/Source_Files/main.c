/***************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for SLSTK3402A
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "bsp.h"

#include "gpio.h"
#include "slider.h"
#include "capsense.h"
#include "cmu.h"
#include "main.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"

#include  <bsp_os.h>

#include  <cpu/include/cpu.h>
#include  <common/include/common.h>
#include  <kernel/include/os.h>
#include  <kernel/include/os_trace.h>

#include  <common/include/lib_def.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/toolchains.h>

#define HFRCO_FREQ 		40000000

// RTOS Global variables
OS_TCB startTaskTCB;
OS_TCB idleTaskTCB;

CPU_STK startTaskStack[START_STACK_SIZE];
CPU_STK idleTaskStack[IDLE_STACK_SIZE];


/* Main */
int main(void)
{
	EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
	CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

	/* Chip errata */
	CHIP_Init();

	/* Init DCDC regulator and HFXO with kit specific parameters */
	/* Init DCDC regulator and HFXO with kit specific parameters */
	/* Initialize DCDC. Always start in low-noise mode. */
	EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;
	EMU_DCDCInit(&dcdcInit);
	em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
	EMU_EM23Init(&em23Init);
	CMU_HFXOInit(&hfxoInit);

	/* Switch HFCLK to HFRCO and disable HFRCO */
	//CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);


	CMU_RouteGPIOClock();       //Enable GPIO Clock

	/* Initialize tasks, OS, etc. */
	RTOS_ERR err;

	CPU_Init();								//Example Code called these functions...why not???
	BSP_SystemInit();
	OS_TRACE_INIT();

	OSInit(&err);							//Initialize kernel
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	OSTaskCreate(&startTaskTCB,     		//Create start task
				 "Start Task",
				 StartTask,
				 DEF_NULL,
				 START_TASK_PRIO,
				 &startTaskStack[0],
				 (START_STACK_SIZE / 10u),
				 START_STACK_SIZE,
				 10u,
				 0u,
				 DEF_NULL,
				 OS_OPT_TASK_STK_CLR,
				 &err);

	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	OSStart(&err);                       		//Start the kernel, LET'S DO THIS!!!!
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	return(1);
}


/* Start Task */
void StartTask(void* p_arg) {
    RTOS_ERR  err;

    PP_UNUSED_PARAM(p_arg);       				//Prevent compiler warning.

    BSP_TickInit();                           	//Initialize Kernel tick source.
	Common_Init(&err);                          // Call common module initialization example.
	APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
	BSP_OS_Init();								//Initialize the kernel

#if defined(uCProbe)
	/* Call these functions to set up uC Probe */
	CPU_TS_TmrInit();							//Initialize timestamp source

	OSStatTaskCPUUsageInit(&err);   			//Call function to initialize stat task
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	CPU_IntDisMeasMaxCurReset();				//Return something about interrupt timing

	OSStatReset(&err);							//Reset Stats? Also said to call this in Appendix B
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
#endif

	/* Create LED Driver, Slider Input, and Button Input tasks */
	OSTaskCreate(&LEDDriverTaskTCB,     		//Create LED driver task
				 "LED Driver Task",
				 LEDDriverTask,
				 DEF_NULL,
				 LED_DRV_TASK_PRIO,
				 &LEDDriverTaskStack[0],
				 (LED_DRV_STACK_SIZE / 2u),
				 LED_DRV_STACK_SIZE,
				 0u,
				 0u,
				 DEF_NULL,
				 OS_OPT_TASK_STK_CLR,
				 &err);

	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    OSTaskCreate(&buttonInputTaskTCB,     		//Create button input task
				 "Button Input Task",
				 ButtonInputTask,
				 DEF_NULL,
				 BTN_IN_TASK_PRIO,
				 &buttonInputTaskStack[0],
				 (BTN_IN_STACK_SIZE / 2u),
				 BTN_IN_STACK_SIZE,
				 0u,
				 0u,
				 DEF_NULL,
				 OS_OPT_TASK_STK_CLR,
				 &err);

    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    OSTaskCreate(&sliderInputTaskTCB,     		//Create slider input task
				 "Slider Input Task",
				 SliderInputTask,
				 DEF_NULL,
				 SLD_IN_TASK_PRIO,
				 &sliderInputTaskStack[0],
				 (SLD_IN_STACK_SIZE / 2u),
				 SLD_IN_STACK_SIZE,
				 0u,
				 0u,
				 DEF_NULL,
				 OS_OPT_TASK_STK_CLR,
				 &err);

    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    __enable_irq();						//Global Enable Interrupts

    OSTaskCreate(&idleTaskTCB,     		//Create LED driver task
				 "Idle Task",
				 IdleTask,
				 DEF_NULL,
				 IDLE_TASK_PRIO,
				 &idleTaskStack[0],
				 (IDLE_STACK_SIZE / 2u),
				 IDLE_STACK_SIZE,
				 0u,
				 0u,
				 DEF_NULL,
				 OS_OPT_TASK_STK_CLR,
				 &err);

	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

	while(1);
}

/* Idle Task */
void IdleTask(void* p_args) {

	RTOS_ERR err;

	OSTaskDel(&startTaskTCB, &err);
	APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

	while(1) {
		EMU_EnterEM1();
	}
}
