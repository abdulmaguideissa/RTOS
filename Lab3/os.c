// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 3 starter file.
// Daniel Valvano
// March 24, 2016

#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"

// function definitions in osasm.s
void StartOS(void);
void static RunPeriodicEvents(void);

#define NUMTHREADS       6        // maximum number of threads
#define NUMPERIODIC      2        // maximum number of periodic threads
#define STACKSIZE        100      // number of 32-bit words in stack per thread
#define TIMER_FREQ       1000
#define TIMER_PRIORITY	 6

 // ************** TCB **************
struct tcb{
  int32_t *sp;        // pointer to stack (valid for threads not running
  int32_t sleep;	  // nonzero if this thread is sleeping
  int32_t *blockd;	  // nonzero if blocked on this semaphore
  struct tcb *next;   // linked-list pointer
};

typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];

// ************* Event task *************
typedef struct eventTask {
	void(*PeriodicEventTask)(void);
	uint32_t TaskPeriod;
	uint32_t TaskCounter;
}eventTask_t, *eventTaskPt;

eventTask_t event_tasks[NUMPERIODIC];

// ******* threads values enumerator *******
enum threads {
	THREAD0,
	THREAD1,
	THREAD2,
	THREAD3,
	THREAD4,
	THREAD5
};

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();   // set processor clock to fastest speed
  uint8_t i;
  for ( i = 0; i < NUMTHREADS; i++ ) {
	  tcbs[i].blockd = 0;
	  tcbs[i].next = NULL;
	  tcbs[i].sp = NULL;
	  tcbs[i].sleep = 0;
  }

  for ( i = 0; i < NUMPERIODIC; i++ ) {
	  event_tasks[i].PeriodicEventTask = NULL;
	  event_tasks[i].TaskPeriod = 0;
	  event_tasks[i].TaskCounter = 0;
  }
  RunPt = NULL;
  BSP_PeriodicTask_Init(&RunPeriodicEvents, TIMER_FREQ, TIMER_PRIORITY);
}

// ********* Initialize stack *********
void SetInitialStack(int i){
	tcbs[i].sp = &Stacks[i][STACKSIZE - 16];
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

//********** OS_AddThreads ***************
// Add six main threads to the scheduler
// Inputs: function pointers to six void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
                  void(*thread1)(void),
                  void(*thread2)(void),
                  void(*thread3)(void),
                  void(*thread4)(void),
                  void(*thread5)(void)){
	uint16_t cr = StartCritical();
	int8_t THREAD;
	tcbs[THREAD0].next = &tcbs[THREAD1];	 // Circular linked list
	tcbs[THREAD1].next = &tcbs[THREAD2];
	tcbs[THREAD2].next = &tcbs[THREAD3];
	tcbs[THREAD3].next = &tcbs[THREAD4];
	tcbs[THREAD4].next = &tcbs[THREAD5];
	tcbs[THREAD5].next = &tcbs[THREAD0];

	for ( THREAD = 0; THREAD < NUMTHREADS; THREAD++ )	 // Initialize stacks of threads.
		SetInitialStack(THREAD);

	// assign to stack pointer of each thread, pointer to that thread. 
	Stacks[THREAD0][STACKSIZE - 2] = (uint32_t)(thread0);  
	Stacks[THREAD1][STACKSIZE - 2] = (uint32_t)(thread1);
	Stacks[THREAD2][STACKSIZE - 2] = (uint32_t)(thread2);
	Stacks[THREAD3][STACKSIZE - 2] = (uint32_t)(thread3);
	Stacks[THREAD4][STACKSIZE - 2] = (uint32_t)(thread4);
	Stacks[THREAD5][STACKSIZE - 2] = (uint32_t)(thread5);
	RunPt = &tcbs[THREAD0];
	EndCritical(cr);
  return 1;               // successful
}

//******** OS_AddPeriodicEventThread ***************
// Add one background periodic event thread
// Typically this function receives the highest priority
// Inputs: pointer to a void/void event thread function
//         period given in units of OS_Launch (Lab 3 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
// In Lab 3 this will be called exactly twice
int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period){
	static uint8_t itr = 0;
	if ( itr < NUMPERIODIC ) {
		if ( event_tasks[itr].PeriodicEventTask == NULL ) {
			event_tasks[itr].PeriodicEventTask = thread;
			event_tasks[itr].TaskPeriod = period;
			itr++;
		}
		return 1;
	}
	else
		return 0;
}

// *********** Run periodic events *************
// Decrement sleep counter and run periodic threads.
void static RunPeriodicEvents(void){
	uint8_t i;
	for ( i = 0; i < NUMTHREADS; i++ ) {	// Decrement sleep counter of main threads
		if ( tcbs[i].sleep )
			tcbs[i].sleep--;
	}
	for ( i = 0; i < NUMPERIODIC; i++ ) {	// Run periodic event threads
		if ( event_tasks[i].PeriodicEventTask != NULL ) {
			event_tasks[i].TaskCounter = event_tasks[i].TaskCounter + 1;
			if ( event_tasks[i].TaskCounter >= event_tasks[i].TaskPeriod ) {
				(*(event_tasks[i].PeriodicEventTask))();
				event_tasks[i].TaskCounter = 0;
			}
		}
	}
}

// *********** OS_Launch ***************
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

// ************ Scheduler ************
// runs every ms
void Scheduler(void){         // every time slice
	//RunPeriodicEvents();
	RunPt = RunPt->next;      // ROUND ROBIN, skip blocked and sleeping threads
	while ( RunPt->blockd || RunPt->sleep )
		RunPt = RunPt->next;
}

// ************ OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status
void OS_Suspend(void){
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(uint32_t sleepTime){
	RunPt->sleep = sleepTime;  // set sleep parameter in TCB
	OS_Suspend();              // suspend, stops running
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
	uint16_t cr = StartCritical();
	*semaPt = value;
	EndCritical(cr);
}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
	DisableInterrupts();
	(*semaPt)--;
	if ( (*semaPt) < 0 ) {
		RunPt->blockd = *semaPt;
		EnableInterrupts();
		OS_Suspend();
	}
	EnableInterrupts();
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
	tcbType *ptr;
	DisableInterrupts();
	(*semaPt)++;
	if ( (*semaPt) < 0 ) {
		ptr = RunPt->next;
		while ( ptr->blockd != *semaPt )
			ptr = ptr->next;
		ptr->blockd = 0;            // Wake up thread, not blocked.
	}
	EnableInterrupts();
}

#define FIFOSIZE 10     // can be any size
uint32_t PutIndex;      // index of where to put next
uint32_t GetIndex;      // index of where to get next
uint32_t FIFO[FIFOSIZE];
int32_t CurrentSize;    // 0 means FIFO empty, FSIZE means full
uint32_t LostData;      // number of lost pieces of data

enum FIFO_status {
	EMPTY,
	NONE = 0,
	FULL = 1
};


// ******** OS_FIFO_Init ************
// Initialize FIFO.  
// One event thread producer, one main thread consumer
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){
	PutIndex = GetIndex = EMPTY;
	OS_InitSemaphore(&CurrentSize, EMPTY);
	LostData = NONE;
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  
// Exactly one event thread puts,
// do not block or spin if full
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
	if ( CurrentSize == FIFOSIZE ) {
		LostData++;
		return -1;
	}
	else {
		FIFO[PutIndex] = data;
		PutIndex = (PutIndex + 1) % FIFOSIZE;  // Wrap around FIFO size.
		OS_Signal(&CurrentSize);
	}
	return 0;
}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.   
// Exactly one main thread get,
// do block if empty
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){
	uint32_t data;
	OS_Wait(&CurrentSize);     // Block if empty
	data = FIFO[GetIndex];	   // Return data
	GetIndex = (GetIndex + 1) % FIFOSIZE;	 // Wrap around FIFO size.
  return data;
}



