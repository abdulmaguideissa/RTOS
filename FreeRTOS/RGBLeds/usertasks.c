/********************************************************************
*	Filename:    usertasks.c
*
*	Description: FreeRTOS user tasks implementations.
*
*	Author:      Abdulmaguid Eissa
*
*	Date:        Oct 23, 2019
**********************************************************************/

#include "usertasks.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h" 

/***********************************************************************
*                           UserTasks_Init  						   *
************************************************************************/
void UserTasks_Init(void)
{
	/*
	 * Enable the GPIOF peripheral
	*/
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	/*
	 * Wait for the GPIOA module to be ready.
	*/
	while ( !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF) )
	{
	}
	/*
	 * Setting pins 1-3 as output in PF.
	*/
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, 
		GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}


/***********************************************************************
*                        vPeriodicRedBlinkTask						   *
************************************************************************/
void vPeriodicRedBlinkTask(void* pvParamters)
{	
	pvParamters = (uint8_t*)pvParamters + 1;
	
	for ( ;;)
	{
		/* Write PF1 and clear PF2-3 */
		GPIOPinWrite(GPIO_PORTF_BASE,
			GPIO_PIN_1, GPIO_PIN_1);
		GPIOPinWrite(GPIO_PORTF_BASE,
			(GPIO_PIN_2 | GPIO_PIN_3), OFF);
	}
}

/***********************************************************************
*                        vPeriodicBlueBlinkTask						   *
************************************************************************/
void vPeriodicBlueBlinkTask(void* pvParamters)
{
	pvParamters = (uint8_t*)pvParamters + 1;
	
	for ( ;;)
	{
		/* Write PF2 and clear PF1, 3 */
		GPIOPinWrite(GPIO_PORTF_BASE,
			GPIO_PIN_2, GPIO_PIN_2);
		GPIOPinWrite(GPIO_PORTF_BASE,
			(GPIO_PIN_1 | GPIO_PIN_3), OFF);
	}
}

/***********************************************************************
*                        vPeriodicGreenBlinkTask					   *
************************************************************************/
void vPeriodicGreenBlinkTask(void* pvParamters)
{
	pvParamters = (uint8_t*)pvParamters + 1;
	
	for ( ;;)
	{
		/* Write PF3 and clear PF1-2 */
		GPIOPinWrite(GPIO_PORTF_BASE,
			GPIO_PIN_3, GPIO_PIN_3);
		GPIOPinWrite(GPIO_PORTF_BASE,
			(GPIO_PIN_1 | GPIO_PIN_2), OFF);
	}
}

/* End of usertasks.c */
