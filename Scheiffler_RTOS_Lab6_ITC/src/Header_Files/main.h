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
extern OS_TCB startTaskTCB;							/**< Start task control block variable */

extern CPU_STK startTaskStack[START_STACK_SIZE];	/**< Start task stack */

// ----- Function Prototypes -----
/// @brief Task to create all other tasks and initialize kernel
void StartTask(void* p_args);


/// @brief Task which is scheduled if all other tasks are blocked/waiting
void IdleTask(void* p_args);

#endif
