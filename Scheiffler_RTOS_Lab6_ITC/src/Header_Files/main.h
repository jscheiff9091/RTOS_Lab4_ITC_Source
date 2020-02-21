#ifndef MAIN_H
#define MAIN_H

// ----- Include files -----

// ----- Macros -----
//#define PART1
//#define PART2
//#define uCProbe


/* Start Task Macros */
#define START_TASK_PRIO 	21u
#define START_STACK_SIZE	1000u

#define IDLE_TASK_PRIO		20u
#define IDLE_STACK_SIZE		1000u

// ----- Type Definitions -----


// ----- Global Variables -----
extern OS_TCB startTaskTCB;

extern CPU_STK startTaskStack[START_STACK_SIZE];

// ----- Function Prototypes -----
void StartTask(void* p_args);

void IdleTask(void* p_args);

#endif
