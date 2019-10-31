/********************************************************************
*	Filename:    usertasks.h
*
*	Description: APIs for the user tasks to run using FreeRTOS, the APIs
*				 naming convention is FreeRTOS-like for more compatibility. 
*
*	Author:      Abdulmaguid Eissa
*
*	Date:        Oct 23, 2019
**********************************************************************/

#ifndef USER_TASKS_H
#define USER_TASKS_H

#define ON           (1)
#define OFF          (0)
#define usertasksMULTI_TASKS   ON

/***********************************************************************
* name:        UserTasks_Init
* Description: Initializing the hardware used by the application tasks.
* Inputs:      none
* Outputs:     none
************************************************************************/
void UserTasks_Init(void);

/***********************************************************************
* name:        vPeriodicRedBlinkTask
* Description: Set the red LED and clear the blue and green ones.
* Inputs:      parameters list to be passed to the task create service 
* 			   in FreeRTOS
* Outputs:     none
************************************************************************/
void vPeriodicRedBlinkTask(void* pvParamters);

/***********************************************************************
* name:        vPeriodicBlueBlinkTask
* Description: Set the blue LED and clear the blue and green ones.
* Inputs:      parameters list to be passed to the task create service
* 			   in FreeRTOS
* Outputs:     none
************************************************************************/
void vPeriodicBlueBlinkTask(void* pvParamters);

/***********************************************************************
* name:        vPeriodicGreenBlinkTask
* Description: Set the green LED and clear the blue and green ones.
* Inputs:      parameters list to be passed to the task create service
* 			   in FreeRTOS
* Outputs:     none
************************************************************************/
void vPeriodicGreenBlinkTask(void* pvParamters);

#endif /* USER_TASKS_H */
