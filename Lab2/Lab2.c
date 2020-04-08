/************************************************************************
 *
 * Lab2.c
 * Lab2.c application layer.
 * Shall include Tasks.h to run without Errors.
 * Tasks.h Folder is LabTasks_4C123.
 * 
*************************************************************************/

#define ERROR                 (255U)
#define THREADFREQ            (1000) // frequency in Hz of round robin scheduler, 1 ms

/* Dependencies header files. */
#include <stdint.h>
#include "Texas.h"
#include "Tasks.h"
#include "os.h"

#if MAIN_APP == STD_ON
int main(void)
{
    OS_Init();
    Profile_Init();  // initialize the 7 hardware profiling pins
    Task0_Init();    // microphone init
    Task1_Init();    // accelerometer init
    BSP_LCD_Init();
    BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
    Time = 0;
    OS_InitSemaphore(&NewData, 0);  // 0 means no data
    OS_InitSemaphore(&LCDmutex, 1); // 1 means free
    OS_MailBox_Init();              // initialize mailbox used to send data between Task1 and Task2

    // Task 0 should run every 1ms and Task 1 should run every 100ms
    OS_AddPeriodicEventThreads(&Task0, 1, &Task1, 100);
    // Task2, Task3, Task4, Task5 are main threads
    OS_AddThreads(&Task2, &Task3, &Task4, &Task5);
    // when grading change 1000 to 4-digit number from edX
    TExaS_Init(GRADER, 2688);          // initialize the Lab 2 grader
    //  TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 2 logic analyzer
    OS_Launch(BSP_Clock_GetFreq() / THREADFREQ); // doesn't return, interrupts enabled in here

    return ERROR;             // this never executes, error happend if it does.
}
#endif // MAIN_APP == STD_ON



#if LAB_STEP == 1
//******************Step 1**************************
// implement and test the semaphores
int32_t s1, s2;

int main(void)
{
    OS_InitSemaphore(&s1, 0);
    OS_InitSemaphore(&s2, 1);

    while ( 1 )
    {
        OS_Wait(&s2); //now s1=0, s2=0
        OS_Signal(&s1); //now s1=1, s2=0
        OS_Signal(&s2); //now s1=1, s2=1
        OS_Signal(&s1); //now s1=2, s2=1
        OS_Wait(&s1); //now s1=1, s2=1
        OS_Wait(&s1); //now s1=0, s2=1
    }
}
#endif // LAB_STEP == 1



#if LAB_STEP == 2
//***************Step 2*************************
// Implement the three mailbox functions as defined in OS.c and OS.h
// Use this a simple main program to test the mailbox functions.
uint32_t Out;

int main(void)
{
    uint32_t in = 0;
    OS_MailBox_Init();

    while ( 1 )
    {
        OS_MailBox_Send(in);
        Out = OS_MailBox_Recv();
        in++;
    }
}
#endif // LAB_STEP == 2


#if LAB_STEP == 3
//***************Step 3*************************
// Test the round robin scheduler
// The minimal set of functions you need to write to get the system running is
//  SysTick_Handler (without calling the C function and without running periodic threads)
//  StartOS
//  OS_Init
//  OS_AddThreads3 (with just 3 threads for now)
//  OS_Launch
int main(void)
{
    OS_Init();
    Profile_Init();  // initialize the 7 hardware profiling pins
    Task0_Init();    // microphone init
    Task1_Init();    // accelerometer init
    BSP_LCD_Init();
    BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
    Time = 0;
    // semaphores and mailbox not used
    // Tasks 0, 1, 2 will not run
    // Task3, Task4, Task5 are main threads
    // Task2 will stall

    OS_AddThreads3(&Task3, &Task4, &Task5);
    // when grading change 1000 to 4-digit number from edX
    //TExaS_Init(GRADER, 1571);          // initialize the Lab 2 grader
      TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 2 logic analyzer
    OS_Launch(BSP_Clock_GetFreq()/THREADFREQ); // doesn't return, interrupts enabled in here

    return 0;             // this never executes
}
#endif //  LAB_STEP == 3


#if LAB_STEP == 4
//***************Step 4*************************
// Increase to 4 threads
int main(void)
{
    OS_Init();
    Profile_Init();  // initialize the 7 hardware profiling pins
    Task0_Init();    // microphone init
    Task1_Init();    // accelerometer init
    BSP_LCD_Init();
    BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
    Time = 0;
    OS_InitSemaphore(&NewData, 0);  // 0 means no data
    OS_InitSemaphore(&LCDmutex, 1); // 1 means free
    OS_MailBox_Init();              // initialize mailbox used to send data between Task1 and Task2
    // Tasks 0, 1 will not run
    // Task2, Task3, Task4, Task5 are main threads
    // Tasks 2 and 5 will stall

    OS_AddThreads(&Task2, &Task3, &Task4, &Task5);
    // when grading change 1000 to 4-digit number from edX
    TExaS_Init(GRADER, 1000);          // initialize the Lab 2 grader
    //  TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 2 logic analyzer
    OS_Launch(BSP_Clock_GetFreq() / THREADFREQ); // doesn't return, interrupts enabled in here

    return 0;             // this never executes
}
#endif // LAB_STEP == 4

#if LAB_STEP == 5
//***************Step 5*************************
// add one periodic task
void Dummy(void){}; // place holder

int main(void)
{
    OS_Init();
    Profile_Init();  // initialize the 7 hardware profiling pins
    Task0_Init();    // microphone init
    Task1_Init();    // accelerometer init
    BSP_LCD_Init();
    BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
    Time = 0;
    OS_InitSemaphore(&NewData, 0);  // 0 means no data
    OS_InitSemaphore(&LCDmutex, 1); // 1 means free
    OS_MailBox_Init();              // initialize mailbox used to send data between Task1 and Task2

    // Task1 will not run
    // Task5 will stall
    // Task 0 should run every 1ms and dummy is not run
    OS_AddPeriodicEventThreads(&Task0, 1, &Dummy, 100);
    // Task2, Task3, Task4, Task5 are main threads
    OS_AddThreads(&Task2, &Task3, &Task4, &Task5);
    // when grading change 1000 to 4-digit number from edX
    TExaS_Init(GRADER, 1000);          // initialize the Lab 2 grader
    //  TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 2 logic analyzer
    OS_Launch(BSP_Clock_GetFreq()/THREADFREQ); // doesn't return, interrupts enabled in here

    return 0;             // this never executes
}
#endif //  LAB_STEP == 5
