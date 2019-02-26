// os.c
// Runs on LM4F120/TM4C123/MSP432
// A priority/blocking real-time operating system 
// Lab 4 starter file.
// Daniel Valvano
// March 25, 2016
// Hint: Copy solutions from Lab 3 into Lab 4
#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"
#include "../inc/tm4c123gh6pm.h"

// function definitions in osasm.s
void StartOS(void);

#define NUMTHREADS       8        // maximum number of threads
#define NUMPERIODIC      2        // maximum number of periodic threads
#define STACKSIZE        100      // number of 32-bit words in stack per thread
#define TIMER_FREQ       1000
#define TIMER_PRIORITY	 6
#define THUMB_BIT   0x01000000  // thumb bit in Process Stack Pointer PSR
#define REG14       0x14141414
#define REG12       0x12121212
#define REG11       0x11111111
#define REG10       0x10101010
#define REG09       0x09090909
#define REG08       0x08080808
#define REG07       0x07070707
#define REG06       0x06060606
#define REG05       0x05050505
#define REG04       0x04040404
#define REG03       0x03030303
#define REG02       0x02020202
#define REG01       0x01010101
#define REG00       0x00000000
#define NULL        (void*)0


// **************** TCB ***************
struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running)
  int32_t Sleep;     // nonzero if this thread is sleeping.
  int32_t *Blocked;  // nonzero if this thread is blocked.
  int32_t Priority; // threads with high priority run more frequently.
  struct tcb *next;  // linked-list pointer
};

typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];
void static RunPeriodicEvents(void);


// ******* threads values enumerator *******
enum threads {
	THREAD0,
	THREAD1,
	THREAD2,
	THREAD3,
	THREAD4,
	THREAD5,
	THREAD6,
	THREAD7
};

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  uint8_t i;
  for ( i = 0; i < NUMTHREADS; i++ ) {	 // Initializing threads.
	  tcbs[i].Blocked = NULL;
	  tcbs[i].next = NULL;
	  tcbs[i].sp = NULL;
	  tcbs[i].Priority = 0;
	  tcbs[i].Sleep = 0;
  }
  BSP_PeriodicTask_Init(&RunPeriodicEvents, TIMER_FREQ, TIMER_PRIORITY);				
  // set up periodic timer to run runperiodicevents to implement sleeping
}

void SetInitialStack(int i){
	tcbs[i].sp = &Stacks[i][STACKSIZE - 16];    // Set stack pointer of each thread.
	Stacks[i][STACKSIZE - 1] = THUMB_BIT;      // enable thumb bit	in PSR
	Stacks[i][STACKSIZE - 3] = REG14;      // R14
	Stacks[i][STACKSIZE - 4] = REG12;      // R12
	Stacks[i][STACKSIZE - 5] = REG03;      // R3
	Stacks[i][STACKSIZE - 6] = REG02;      // R2
	Stacks[i][STACKSIZE - 7] = REG01;      // R1
	Stacks[i][STACKSIZE - 8] = REG00;      // R0
	Stacks[i][STACKSIZE - 9] = REG11;      // R11
	Stacks[i][STACKSIZE - 10] = REG10;     // R10
	Stacks[i][STACKSIZE - 11] = REG09;     // R9
	Stacks[i][STACKSIZE - 12] = REG08;     // R8
	Stacks[i][STACKSIZE - 13] = REG07;     // R7
	Stacks[i][STACKSIZE - 14] = REG06;     // R6
	Stacks[i][STACKSIZE - 15] = REG05;     // R5
	Stacks[i][STACKSIZE - 16] = REG04;     // R4
}

//******** OS_AddThreads ***************
// Add eight main threads to the scheduler
// Inputs: function pointers to eight void/void main threads
//         priorites for each main thread (0 highest)
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void), uint32_t p0,
                  void(*thread1)(void), uint32_t p1,
                  void(*thread2)(void), uint32_t p2,
                  void(*thread3)(void), uint32_t p3,
                  void(*thread4)(void), uint32_t p4,
                  void(*thread5)(void), uint32_t p5,
                  void(*thread6)(void), uint32_t p6,
                  void(*thread7)(void), uint32_t p7){
	uint16_t cr = StartCritical();
	uint8_t THREAD;
	uint16_t PRIORITY[NUMTHREADS];
	PRIORITY[0] = p0;
	PRIORITY[1] = p1;
	PRIORITY[2] = p2;
	PRIORITY[3] = p3;
	PRIORITY[4] = p4;
	PRIORITY[5] = p5;
	PRIORITY[6] = p6;
	PRIORITY[7] = p7;

	tcbs[THREAD0].next = &tcbs[THREAD1];	 // Circular linked list
	tcbs[THREAD1].next = &tcbs[THREAD2];
	tcbs[THREAD2].next = &tcbs[THREAD3];
	tcbs[THREAD3].next = &tcbs[THREAD4];
	tcbs[THREAD4].next = &tcbs[THREAD5];
	tcbs[THREAD5].next = &tcbs[THREAD6];
	tcbs[THREAD6].next = &tcbs[THREAD7];
	tcbs[THREAD7].next = &tcbs[THREAD0];

	for ( THREAD = 0; THREAD < NUMTHREADS; THREAD++ ) {	 // Initialize stacks and threads priority
		SetInitialStack(THREAD);
		tcbs[THREAD].Priority = PRIORITY[THREAD];     
	}
	RunPt = &tcbs[THREAD0];			   // Initialize run pointer to first thread
	EndCritical(cr);
  return 1;               // successful
}


void static RunPeriodicEvents(void){
	uint8_t THREAD;         // DECREMENT SLEEP COUNTERS
	for ( THREAD = 0; THREAD < NUMTHREADS; THREAD++ )
		if ( tcbs[THREAD].Sleep )
			tcbs[THREAD].Sleep--;
    // In Lab 4, handle periodic events in RealTimeEvents
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 = (SYSPRI3 & 0x00FFFFFF) | 0xE0000000;// priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}

// ***************** Scheduler *****************
// look at all threads in TCB list choose
// highest priority thread not blocked and not sleeping 
// If there are multiple highest priority (not blocked, not sleeping),
// run these round robin.
// runs every ms.
void Scheduler(void){      // every time slice
	uint8_t Max = 255;
	tcbType *pt = RunPt;
	tcbType *bestPt;
	do {
		pt = pt->next;
		if ( (pt->Blocked == 0) && (pt->Sleep == 0) && (pt->Priority < Max) ) {
			Max = pt->Priority;
			bestPt = pt;
		}
	} while ( pt != RunPt);
	RunPt = bestPt;
}

//******** OS_Suspend ***************
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
	RunPt->Sleep = sleepTime;   // set sleep parameter in TCB, same as Lab 3
	OS_Suspend();               // suspend, stops running.
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
	DisableInterrupts();
	*semaPt = value;
	EnableInterrupts();
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
		RunPt->Blocked = semaPt;
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
	tcbType *pt;
	DisableInterrupts();
	(*semaPt)++;
	if ( (*semaPt) < 0 ) {	  // if semaphore still less than 0.
		pt = RunPt->next;
		while ( pt->Blocked != semaPt )
			pt = pt->next;
		pt->Blocked = 0;     // Wake up this thread.
	}
	EnableInterrupts();
}

#define FIFOSIZE 10    // can be any size
uint32_t PutIndex;      // index of where to put next
uint32_t GetIndex;      // index of where to get next
uint32_t FIFO[FIFOSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data


enum FIFO_status {
	EMPTY,
	NONE = 0,
	FULL = 1
};
// ******** OS_FIFO_Init ************
// Initialize FIFO.  The "put" and "get" indices initially
// are equal, which means that the FIFO is empty.  Also
// initialize semaphores to track properties of the FIFO
// such as size and busy status for Put and Get operations,
// which is important if there are multiple data producers
// or multiple data consumers.
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){
	PutIndex = GetIndex = EMPTY;
	OS_InitSemaphore(&CurrentSize, EMPTY);
	LostData = NONE;
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is putting data into the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
	if ( CurrentSize == FIFOSIZE ) {
		LostData++;
		return -1;
	}
	else {
		FIFO[PutIndex] = data;
		PutIndex = (PutIndex + 1) % FIFOSIZE;
		OS_Signal(&CurrentSize);
	}
 return 0; // success
}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is getting data from the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){
	uint32_t data;
	OS_Wait(&CurrentSize);	  // Block if empty
	data = FIFO[GetIndex];	  // Retreive data. 
	GetIndex = (GetIndex + 1) % FIFOSIZE;
 return data;
}
// *****periodic events****************
int32_t *PeriodicSemaphore0;
uint32_t Period0; // time between signals
int32_t *PeriodicSemaphore1;
uint32_t Period1; // time between signals

void RealTimeEvents(void){
	int flag = 0;
    static int32_t realCount = -10; // let all the threads execute once
  // Note to students: we had to let the system run for a time so all user threads ran at least one
  // before signalling the periodic tasks
  realCount++;
  if(realCount >= 0){
	if((realCount % Period0) == 0){
		OS_Signal(PeriodicSemaphore0);
		flag = 1;
	}
    if((realCount % Period1) == 0){
	 	OS_Signal(PeriodicSemaphore1);
	 	flag = 1;
	 }
    if(flag){
	 	OS_Suspend();
    }
  }
}
// ******** OS_PeriodTrigger0_Init ************
// Initialize periodic timer interrupt to signal 
// Inputs:  semaphore to signal
//          period in ms
// priority level at 0 (highest
// Outputs: none
void OS_PeriodTrigger0_Init(int32_t *semaPt, uint32_t period){
	PeriodicSemaphore0 = semaPt;
	Period0 = period;
	BSP_PeriodicTask_InitC(&RealTimeEvents, 1000, 0);
}
// ******** OS_PeriodTrigger1_Init ************
// Initialize periodic timer interrupt to signal 
// Inputs:  semaphore to signal
//          period in ms
// priority level at 0 (highest
// Outputs: none
void OS_PeriodTrigger1_Init(int32_t *semaPt, uint32_t period){
	PeriodicSemaphore1 = semaPt;
	Period1 = period;
	BSP_PeriodicTask_InitC(&RealTimeEvents, 1000, 0);
}

//****edge-triggered event************
int32_t *edgeSemaphore;
#define PORTD_PIN6  0x40
// ******** OS_EdgeTrigger_Init ************
// Initialize button1, PD6, to signal on a falling edge interrupt
// Inputs:  semaphore to signal
//          priority
// Outputs: none
void OS_EdgeTrigger_Init(int32_t *semaPt, uint8_t priority){
	volatile uint32_t delay;
	edgeSemaphore = semaPt;
	SYSCTL_RCGCGPIO_R = 0x08;      // 1) activate clock for Port D
	delay = SYSCTL_RCGCGPIO_R;    // allow time for clock to stabilize
	GPIO_PORTD_AMSEL_R &= ~PORTD_PIN6;   // 3) disable analog on PD6
	GPIO_PORTD_DIR_R &= ~PORTD_PIN6;      // 4) configure PD6 as GPIO
	GPIO_PORTD_AFSEL_R &= ~PORTD_PIN6;    // 6) disable alt funct on PD6
	GPIO_PORTD_PUR_R &= ~PORTD_PIN6;      // disable pull-up on PD6
	GPIO_PORTD_DIR_R |= PORTD_PIN6;       // 7) enable digital I/O on PD6  
	GPIO_PORTD_IS_R &= ~PORTD_PIN6;       // (d) PD6 is edge-sensitive 
	GPIO_PORTD_IBE_R &= ~PORTD_PIN6;      //     PD6 is not both edges 
	GPIO_PORTD_IEV_R &= ~PORTD_PIN6;      //     PD6 is falling edge event 
	GPIO_PORTD_ICR_R |= PORTD_PIN6;       // (e) clear PD6 flag
	GPIO_PORTD_IM_R |= PORTD_PIN6;        // (f) arm interrupt on PD6
	NVIC_PRI0_R = (NVIC_PRI0_R & 0x0FFFFFFF) | (priority << 29);      // priority on Port D edge trigger is NVIC_PRI0_R	31 – 29
    NVIC_EN0_R |= 0x00000004;      // enable is bit 3 in NVIC_EN0_R
 }

// ******** OS_EdgeTrigger_Restart ************
// restart button1 to signal on a falling edge interrupt
// rearm interrupt 3 in NVIC
// clear flag6
// rearm interrupt
// Inputs:  none
// Outputs: none
void OS_EdgeTrigger_Restart(void){
	GPIO_PORTD_ICR_R |= PORTD_PIN6;
	NVIC_EN0_R |= 0x00000004;
}

// ************ GPIOPortD_Handler ************
// step 1 acknowledge by clearing flag
// step 2 signal semaphore (no need to run scheduler)
// step 3 disarm interrupt to prevent bouncing to create multiple signals
void GPIOPortD_Handler(void){
	GPIO_PORTD_ICR_R |= PORTD_PIN6;
	OS_Signal(edgeSemaphore);
	NVIC_EN0_R &= ~0x00000004;
}


