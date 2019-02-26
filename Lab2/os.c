// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 2 starter file.
// Daniel Valvano
// Changed by ME
// This OS operates with spin-lock semaphores 
// Implementation of WAIT and SIGNAL binary semaphores 
// is based on spin-lock criteria.
// February 20, 2016

#include <stdint.h>
#include "os.h"
#include "../inc/CortexM.h"
#include "../inc/BSP.h"

// function definitions in osasm.s
void StartOS(void);

// Global variables for periodic threads.
void (*PeriodicTask1)(void) = 0;	  // function pointer to periodic function
void (*PeriodicTask2)(void) = 0;
volatile uint64_t TaskCounter = 0;		    	  // global counter for running event threads
volatile uint32_t TaskPeriod1 = 0;					  // shared globals for periodic tasks events
volatile uint32_t TaskPeriod2 = 0;

 // threads values enumerator
enum threads{
	THREAD0,
	THREAD1,
	THREAD2,
	THREAD3 
};

// threads status enumerator
enum threads_status {
	BUSY,
	FREE
};

// mailbox status enumerator
enum mailbox_status {
	EMPTY,
	FULL
};

// thread control block data structure init.
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];


// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: systick, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
}

void SetInitialStack(int i){
	tcbs[i].sp = &Stacks[i][STACKSIZE - 16];    // stack pointer to R4
	Stacks[i][STACKSIZE - 1] = 0x010000000;     // enable thumb bit	in PSR
	Stacks[i][STACKSIZE - 3] = 0x14141414;      // R14
	Stacks[i][STACKSIZE - 4] = 0x12121212;      // R12
    Stacks[i][STACKSIZE - 5] = 0x03030303;      // R3
	Stacks[i][STACKSIZE - 6] = 0x02020202;      // R2
	Stacks[i][STACKSIZE - 7] = 0x01010101;      // R1
	Stacks[i][STACKSIZE - 8] = 0x00000000;      // R0
	Stacks[i][STACKSIZE - 9] = 0x11111111;      // R11
	Stacks[i][STACKSIZE - 10] = 0x10101010;     // R10
	Stacks[i][STACKSIZE - 11] = 0x09090909;     // R9
	Stacks[i][STACKSIZE - 12] = 0x08080808;     // R8
	Stacks[i][STACKSIZE - 13] = 0x07070707;     // R7
	Stacks[i][STACKSIZE - 14] = 0x06060606;     // R6
	Stacks[i][STACKSIZE - 15] = 0x05050505;     // R5
	Stacks[i][STACKSIZE - 16] = 0x04040404;     // R4
}

//******** OS_AddThreads ***************
// Add four main threads to the scheduler
// Inputs: function pointers to four void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
                  void(*thread1)(void),
                  void(*thread2)(void),
                  void(*thread3)(void)){
	int32_t status;
	status = StartCritical();
    // initialize TCB circular list
    tcbs[THREAD0].next = &tcbs[THREAD1];      // thread 0 points to thread 1
    tcbs[THREAD1].next = &tcbs[THREAD2];      // thread 1 points to thread 2
    tcbs[THREAD2].next = &tcbs[THREAD3];      // thread 2 points to thread 3
    tcbs[THREAD3].next = &tcbs[THREAD0]; 	  // thread 3 points to thread 0 to complete circular linked list 	
	  									
    // initialize four stacks, including initial PC
    SetInitialStack(THREAD0);     // Stack of the first thread
    Stacks[THREAD0][STACKSIZE - 2] = (uint32_t)(thread0);
    SetInitialStack(THREAD1);     // Stack of the second thread
    Stacks[THREAD1][STACKSIZE - 2] = (uint32_t)(thread1);
    SetInitialStack(THREAD2);     // Stack of the third thread
    Stacks[THREAD2][STACKSIZE - 2] = (uint32_t)(thread2);
    SetInitialStack(THREAD3);     // Stack of the fourth thread
    Stacks[THREAD3][STACKSIZE - 2] = (uint32_t)(thread3);					
	  									
    // initialize RunPt
    RunPt = &tcbs[THREAD0];                 // Running the first thread.
    EndCritical(status);
    return 1;               // successful
}

//******** OS_AddThreads3 ***************
// add three foregound threads to the scheduler
// This is needed during debugging and not part of final solution
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads3(void(*task0)(void),
                 void(*task1)(void),
                 void(*task2)(void)){
	uint32_t status;
	status = StartCritical();

// initialize TCB circular list (same as RTOS project)
	tcbs[THREAD0].next = &tcbs[THREAD1];
	tcbs[THREAD1].next = &tcbs[THREAD2];
	tcbs[THREAD2].next = &tcbs[THREAD0];

// initialize RunPt
	RunPt = &tcbs[THREAD0];

// initialize four stacks, including initial PC
	SetInitialStack(THREAD0);
	Stacks[THREAD0][STACKSIZE - 2] = (uint32_t)(task0);
	SetInitialStack(THREAD1);
	Stacks[THREAD0][STACKSIZE - 2] = (uint32_t)(task1);
	SetInitialStack(THREAD2);
	Stacks[THREAD0][STACKSIZE - 2] = (uint32_t)(task2);

	EndCritical(status);
  
  return 1;               // successful
}
                 
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
  void(*thread2)(void), uint32_t period2){
	(PeriodicTask1) = (thread1);
	TaskPeriod1 = period1;
	(PeriodicTask2) = (thread2);
	TaskPeriod2 = period2;
  return 1;
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}

// ************ Scheduler **************
// Running for two periodic threads, one is 1 ms
// and the other one is 100 ms.
// runs every ms
void Scheduler(void){ // every time slice
  // run any periodic event threads if needed
	TaskCounter = (TaskCounter + 1) % 100; // bound the counter to the Least Common Multiple of the periods
	if ( TaskCounter % TaskPeriod1 == 0 )  // wrap around task1 period, will run every 1 ms
		(PeriodicTask1)();				   // Runs at 0, 1, 2, 3.. etc. 
	if ( TaskCounter % TaskPeriod2 == 1 )  // wrap around task2 period, runs every 100 ms
		(PeriodicTask2)();				   // Runs at 1, 101, 201.. etc.
	RunPt = RunPt->next;				   // Go to the next thread.
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
	long cr = StartCritical();
	*semaPt = value;        // initially semaphore is free.
	EndCritical(cr);
}

// ******** OS_Wait ************
// Decrement semaphore
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
	DisableInterrupts();
	while ( *semaPt == BUSY ) {  // Spin on semaphore till data is available, semaphore is busy
		EnableInterrupts();      // serve interrupt if occured here.
		DisableInterrupts();
	}
	*semaPt = *semaPt - 1;
	EnableInterrupts();
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
	DisableInterrupts();
	*semaPt = *semaPt + 1;
	EnableInterrupts();
}


// ******** OS_MailBox_Init ************
// Initialize communication channel
// Producer is an event thread, consumer is a main thread
// Inputs:  none
// Outputs: none
// Shared Global variables should be volatile 
volatile uint32_t Mail;	      // mailbox 
volatile uint32_t Mail_Data = EMPTY;
volatile uint32_t Send = 0;	  // for main threads
volatile uint32_t Ack = 0;	  // for main threads
volatile uint32_t Lost = 0;	  // for event threads

void OS_MailBox_Init(void){
  // include data field and semaphore
	OS_InitSemaphore(&Mail, EMPTY);
}

// ******** OS_MailBox_Send ************
// Enter data into the MailBox, do not spin/block if full
// Use semaphore to synchronize with OS_MailBox_Recv
// Inputs:  data to be sent
// Outputs: none
// Errors: data lost if MailBox already has data
void OS_MailBox_Send(uint32_t data){
	long cr = StartCritical();  // prevent other threads from any modifications.
	Mail_Data = data;
	EndCritical(cr);			// release mailbox for other threads. 
	if ( Send )              // mailbox has unread data
		Lost++;              // notify of lost data
	else		             // mailbox is empty
		OS_Signal(&Send);	 // notify of new arrived data

}

// ******** OS_MailBox_Recv ************
// retreive mail from the MailBox
// Use semaphore to synchronize with OS_MailBox_Send
// Lab 2 spin on semaphore if mailbox empty
// Lab 3 block on semaphore if mailbox empty
// Inputs:  none
// Outputs: data retreived
// Errors:  none
uint32_t OS_MailBox_Recv(void){ 
	uint32_t data;
	long cr;
	OS_Wait(&Send);            // spin on mailbox, main thread
	cr = StartCritical();	   // prevent other threads from any modifications.
	data = Mail_Data;
	EndCritical(cr);		   // release mailbox for other threads. 
    return data;
}


