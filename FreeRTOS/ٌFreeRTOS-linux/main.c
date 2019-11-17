/********************************************************************
*	Filename:    main.c
*
*	Description: The main thread, running the application tasks. The 
*				 system tick is set to be 1 Hz. Hardware is configured 
*				 using TivaWare software pack. 
*				 Three main tasks, red led is on, blue led is on and green 
*				 led is on, all the tasks hold the same priority.
*				 Purpose of this application is to get engaged with both 
*                FreeRTOS and TivaWare using Multi Layers Architecture.
*
*	Author:      Abdulmaguid Eissa
*
*	Date:        Oct 23, 2019
**********************************************************************/

#include <stdint.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* User application headers */
#include "usertasks.h"

/*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
******************************************************************************/
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

/* File globlas */
uint32_t SystemCoreClock = 16000000;   /* External definition of the FreeRTOS declaration */
volatile uint8_t *g_watch_counter = 0;  /* for debugging purposes, to be passed as task parameters */



int main()
{
	/* Initialize the hardware related to the application */
	UserTasks_Init();
	
	/* Create new tasks */
    xTaskCreate(vPeriodicRedBlinkTask, "Red LED", 
		configMINIMAL_STACK_SIZE, 
		(uint8_t*)g_watch_counter, 
		1, 
		NULL_PTR);

#if ( usertasksMULTI_TASKS == ON )
	xTaskCreate(vPeriodicBlueBlinkTask, "Blue LED", 
		configMINIMAL_STACK_SIZE, 
		(uint8_t*)g_watch_counter,
		1, 
		NULL_PTR);

	xTaskCreate(vPeriodicGreenBlinkTask, "Green LED", 
		configMINIMAL_STACK_SIZE, 
		(uint8_t*)g_watch_counter,
		1, 
		NULL_PTR);
#endif /* usertasksMULTI_TASKS == ON */

    // Startup of the FreeRTOS scheduler.  The program should block here.
    vTaskStartScheduler();

    /* If all is well then main() will never reach here as the scheduler will  
    now be running the tasks.  If main() does reach here then it is likely that  
    there was insufficient heap memory available for the idle task to be created.  
    Chapter 2 provides more information on heap memory management. */ 
    for (;;);
}
