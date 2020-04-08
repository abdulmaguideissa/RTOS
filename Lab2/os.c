// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 2 starter file.
// Daniel Valvano
// February 20, 2016
// Implemented OS functions by Abdulmaguid Eissa, April 1, 2020

#include <stdint.h>
#include "os.h"
#include "../inc/CortexM.h"
#include "../inc/BSP.h"

// function definitions in osasm.s
void StartOS(void);

/* 
    Globals used in os.c.
*/
tcbType *RunPt;
tcbType tcbs[NUMTHREADS];               /* private to os.c source file. */
int32_t Stacks[NUMTHREADS][STACKSIZE];  /* private to os.c source file. */

/* 
    Declarations for periodic event threads, to be used in
    OS_AddPeriodicEventThreads function.
*/
static PeriodicTaskType PeriodicEventThreads[PERIODIC_TASKS_NUM];
static volatile uint8_t TaskCounter;

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: systick, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void)
{
    DisableInterrupts();
    BSP_Clock_InitFastest();// set processor clock to fastest speed

    for ( uint8_t tcb = 0; tcb < NUMTHREADS; tcb++ )  // init tcbs
    {
        tcbs[tcb].sp = NULL_PTR;
        tcbs[tcb].next = NULL_PTR;
    }

    for ( uint8_t task = 0; task < PERIODIC_TASKS_NUM; task++ )  // init periodic tasks
    {
        PeriodicEventThreads[task].PeriodicTaskPeriod = 0;
    }
    TaskCounter = 0;
    RunPt = NULL_PTR;   // init RunPt
}


 /* Private function: SetInitialStack()
    @ parameter in:  stack number, must be in the range of NUMTHREADS.
    @ parameter out: none
    @ description:   setting the initial stack pointer and thumb bit for
                     specific thread.
 */
static void SetInitialStack(int stack)
{
    tcbs[stack].sp = &Stacks[stack][STACKSIZE - 16];   // thread stack pointer.
    Stacks[stack][STACKSIZE - 1] = R16;   // thumb bit.
    Stacks[stack][STACKSIZE - 3] = R14;
    Stacks[stack][STACKSIZE - 4] = R12;

    Stacks[stack][STACKSIZE - 5] = R3;
    Stacks[stack][STACKSIZE - 6] = R2;
    Stacks[stack][STACKSIZE - 7] = R1;
    Stacks[stack][STACKSIZE - 8] = R0;

    Stacks[stack][STACKSIZE - 9]  = R11;
    Stacks[stack][STACKSIZE - 10] = R10;
    Stacks[stack][STACKSIZE - 11] = R9;
    Stacks[stack][STACKSIZE - 12] = R8;
    Stacks[stack][STACKSIZE - 13] = R7;
    Stacks[stack][STACKSIZE - 14] = R6;
    Stacks[stack][STACKSIZE - 15] = R5;
    Stacks[stack][STACKSIZE - 16] = R4;
}

//******** OS_AddThreads ***************
// Add four main threads to the scheduler
// Inputs: function pointers to four void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
    void(*thread1)(void),
    void(*thread2)(void),
    void(*thread3)(void)) 
{
    uint32_t status;
    status = StartCritical();

    // initialize TCB circular list
    tcbs[0].next = &tcbs[1];
    tcbs[1].next = &tcbs[2];
    tcbs[2].next = &tcbs[3];
    tcbs[3].next = &tcbs[0];

    // initialize four stacks, including initial PC
    for ( uint8_t stack = 0; stack < NUMTHREADS; stack++ )
    {
        SetInitialStack(stack);
    }
	
    /* set PC for each thread. */
    Stacks[0][STACKSIZE - 2] = (int32_t)(thread0);  
    Stacks[1][STACKSIZE - 2] = (int32_t)(thread1);
    Stacks[2][STACKSIZE - 2] = (int32_t)(thread2);
    Stacks[3][STACKSIZE - 2] = (int32_t)(thread3);
	
	 // initialize RunPt
    RunPt = &tcbs[0];
	
    EndCritical(status);
    return 1;               // successful
}

#if LAB_STEP == 3
//******** OS_AddThreads3 ***************
// add three foregound threads to the scheduler
// This is needed during debugging and not part of final solution
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads3(void(*task0)(void),
    void(*task1)(void),
    void(*task2)(void)) 
{
    uint32_t status;
    status = StartCritical();

    // initialize TCB circular list
    tcbs[0].next = &tcbs[1];
    tcbs[1].next = &tcbs[2];
    tcbs[2].next = &tcbs[0];

    // initialize four stacks, including initial PC
    for ( uint8_t stack = 0; stack < THREADS_NUM_3; stack++ )
    {
        SetInitialStack(stack);
    }

    if ( NULL_PTR == (task0) ||
         NULL_PTR == (task1) ||
         NULL_PTR == (task2))
    {
        EndCritical(status);
        return 0;
    }

    /* set PC for each thread. */
    Stacks[0][STACKSIZE - 2] = (uint32_t)(task0);
    Stacks[1][STACKSIZE - 2] = (uint32_t)(task1);
    Stacks[2][STACKSIZE - 2] = (uint32_t)(task2);
	
	// initialize RunPt
    RunPt = &tcbs[0];

    EndCritical(status);
    return 1;               // successful
}
#endif // LAB_STEP == 3

//******** OS_AddPeriodicEventThreads ***************
// Add two background periodic event threads
// Typically this function receives the highest priority
// Inputs: pointers to a void/void event thread function2
//         periods given in units of OS_Launch (Lab 2 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
int OS_AddPeriodicEventThreads(void(*thread1)(void), uint32_t period1,
    void(*thread2)(void), uint32_t period2)
{
    PeriodicEventThreads[0].PeriodicTaskPtr = thread1;
    PeriodicEventThreads[0].PeriodicTaskPeriod = period1;

    PeriodicEventThreads[1].PeriodicTaskPtr = thread2;
    PeriodicEventThreads[1].PeriodicTaskPeriod = period2;
    return 1;
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
// Fperiodic = Fbus / STRELAOD - 1
// Fperiodic is 1000 Hz/ 1 ms
void OS_Launch(uint32_t TheTimeSlice)
{
    STCTRL = 0;                  // disable SysTick during setup
    STCURRENT = 0;               // any write to current clears it
    SYSPRI3 = (SYSPRI3 & 0x00FFFFFF) | 0xE0000000; // priority 7
    STRELOAD = TheTimeSlice - 1; // reload value
    STCTRL = 0x00000007;         // enable, core clock and interrupt arm
    StartOS();                   // start on the first task
}

/**************************************  
@ function name: Scheduler()
@ parameter in:  none
@ parameter out: none
@ description:   OS scheduler runs one periodic thread every 1 ms and
                 the other one every 100 ms in Round Robin criteria.
@ note:          this function is linked to the osasm SysTick_Handler
                 context switcher, for future features to be added.
*/
void Scheduler(void) // every time slice
{
    TaskCounter++;

    // periodic task 1 runs every 1 ms, 1, 2, 3...
    // periodic task 2 runs every 100 ms, 1, 101...
    if ( TaskCounter >= PeriodicEventThreads[0].PeriodicTaskPeriod ) // runs every 1 ms
    {
        (PeriodicEventThreads[0].PeriodicTaskPtr)();  // Task0
    }
    if ( TaskCounter >= PeriodicEventThreads[1].PeriodicTaskPeriod ) // runs every 100 ms
    {
        (PeriodicEventThreads[1].PeriodicTaskPtr)();  // Task1
        TaskCounter = 0;
    }
    
    RunPt = RunPt->next;
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t* semaPt, int32_t value)
{
    DisableInterrupts();
    *semaPt = value;
    EnableInterrupts();
}

// ******** OS_Wait ************
// Decrement semaphore
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t* semaPt)
{
	long cr;
    cr = StartCritical();  // ensure mutual exclusion

    while ( (*semaPt) <= 0 )
    {
        EndCritical(cr);   // interrupts could happen here
        cr = StartCritical();
    }

    (*semaPt) = (*semaPt) - 1;  // decrement as indication of sucessful reception
    EndCritical(cr);
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t* semaPt)
{
    long cr = StartCritical();        // ensure mutual exclusion 
    (*semaPt) = (*semaPt) + 1;  // increment as indication of new available data
    EndCritical(cr);
}

// ******** OS_MailBox_Init ************
// Initialize communication channel
// Producer is an event thread, consumer is a main thread
// Inputs:  none
// Outputs: none

// Shared globals within the MailBox functions
static int32_t MailSend;   // signal mailbox semaphore
static volatile int32_t MailLost;   // record lost data
static volatile uint32_t MailData;

void OS_MailBox_Init(void)
{
	MailData = 0;
	MailLost = 0;
    OS_InitSemaphore(&MailSend, 0);
}

// ******** OS_MailBox_Send ************
// Enter data into the MailBox, do not spin/block if full
// Use semaphore to synchronize with OS_MailBox_Recv
// Inputs:  data to be sent
// Outputs: none
// Errors: data lost if MailBox already has data
void OS_MailBox_Send(uint32_t data)
{
    long cr = StartCritical();
    MailData = data;

    if ( MailSend )  // mailbox has unread data
    {
        MailLost++;
    }
	EndCritical(cr);
    OS_Signal(&MailSend);
}

// ******** OS_MailBox_Recv ************
// retreive mail from the MailBox
// Use semaphore to synchronize with OS_MailBox_Send
// Lab 2 spin on semaphore if mailbox empty
// Lab 3 block on semaphore if mailbox empty
// Inputs:  none
// Outputs: data retreived
// Errors:  none
uint32_t OS_MailBox_Recv(void)
{
    uint32_t data = 0;
    long cr;

    OS_Wait(&MailSend);
    cr = StartCritical();
    data = MailData;
    EndCritical(cr);
    return data;
}


