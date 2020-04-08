/*****************************************************************
 * Tasks.h
 * Contains functions declarations for Real-time Bluetooth Network 
   Systems Labs.
 * Lab1.c
 * Lab2.c
 * Lab3.c
 * Lab4.c
 * Lab5.c

 * Author: Abdulmaguid Eissa
******************************************************************/

#ifndef TASKS_H
#define TASKS_H

// Comonly used header files
#include <stdint.h>
#include "../inc/BSP.h"
#include "../inc/Profile.h"
#include "../inc/CortexM.h"

// *********Task0_Init*********
// initializes microphone
// Task0 measures sound intensity
// Inputs:  none
// Outputs: none
void Task0_Init(void);

// *********Task0*********
// collects data from microphone
// Inputs:  none
// Outputs: none
void Task0(void);

// *********Task1_Init*********
// initializes accelerometer
// Task1 counts Steps
// Inputs:  none
// Outputs: none
void Task1_Init(void);

// *********Task1*********
// collects data from accelerometer
// counts Steps
// Inputs:  none
// Outputs: none
void Task1(void);

// *********Task2_Init*********
// initializes light sensor
// Task2 measures light intensity
// Inputs:  none
// Outputs: none
void Task2_Init(void);

// *********Task2*********
// collects data from light sensor
// Inputs:  none
// Outputs: none
// must be called less than once a second
void Task2(void);

// *********Task3_Init*********
// initializes switches, buzzer, and LEDs
// Task3 checks the switches, updates the mode, and outputs to the buzzer and LED
// Inputs:  none
// Outputs: none
void Task3_Init(void);

// *********Task3*********
// non-real-time task
// checks the switches, updates the mode, and outputs to the buzzer and LED
// Inputs:  none
// Outputs: none
void Task3(void);

// *********Task4_Init*********
// initializes LCD
// Task4 updates the plot and Task5 updates the text at the top of the plot
// Inputs:  none
// Outputs: none
void Task4_Init(void);

// *********Task4*********
// updates the plot
// Inputs:  none
// Outputs: none
void Task4(void);

// *********Task5_Init*********
// initializes LCD
// Task5 updates the text at the top of the plot
// Inputs:  none
// Outputs: none
void Task5_Init(void);

// *********Task5*********
// updates the text at the top of the LCD
// Inputs:  none
// Outputs: none
void Task5(void);

//---------------- Global variables shared between tasks ----------------
extern uint32_t Time;              // elasped time in 100 ms units
extern uint32_t Steps;             // number of steps counted
extern uint32_t Magnitude;         // will not overflow (3*1,023^2 = 3,139,587)
                             // Exponentially Weighted Moving Average
extern uint32_t EWMA;              // https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
extern uint16_t SoundData;         // raw data sampled from the microphone
extern int32_t SoundAvg;
 
extern uint32_t LightData;
extern int32_t TemperatureData;    // 0.1C 

// semaphores
extern int32_t NewData;        // true when new numbers to display on top of LCD
extern int32_t LCDmutex;       // exclusive access to LCD
extern int ReDrawAxes;         // non-zero means redraw axes on next display task

#endif // !TASKS_H
