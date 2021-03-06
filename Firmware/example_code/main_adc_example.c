//*****************************************************************************
//
// main_adc_example.c
//
// Example main code showing how to use the adc_driver_if library
//
// This source file uses code snippets from Texas Instruments Incorporated's
// CC3200-LAUNCHXL sample projects. Copyright notice is moved to the end of
// this file.
//
// Project: Human Interface for Robotic Control
//
// Created:
// December 28, 2015
//
// Modified:
// January 6, 2015
//
//*****************************************************************************

// Standard includes
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Driverlib includes
#include "utils.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_adc.h"
#include "hw_ints.h"
#include "hw_gprcm.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "pin.h"
#include "adc.h"

#include "pin_mux_config.h"
#include "adc_driver_if.h"
#include "uart_if.h"
#include "msg_util_if.h"

#define UART_PRINT Report
#define FOREVER 1

//*****************************************************************************
//                      GLOBAL VARIABLES
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif


static void BoardInit(void);

//*****************************************************************************
// Board Initialization & Configuration
//
// \param  None
//
// \return None
//*****************************************************************************
static void BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void main()
{
	unsigned short sensorReading;
	unsigned char highByte, lowByte;

    // Initialize Board configurations
    BoardInit();

    // Configuring UART for Receiving input and displaying output
    // 1. PinMux setting
    // 2. Initialize UART
    PinMuxConfig();
    InitTerm();

    InitSensorADC();

    // Continously get sensor reading
    while(FOREVER)
    {
        //PIN_57 is muxed between UART0 RX and ADC0, cannot do both at same time
        //UART_PRINT("FINGER_THUMB: %f (V)\n\r", GetSensorReading(FINGER_THUMB));

    	sensorReading = GetSensorReading(SENSOR_FINGER_INDEX);
    	UnsignedShort_to_UnsignedChar(sensorReading, &highByte, &lowByte);
    	UART_PRINT("FINGER_INDEX - ");
    	UART_PRINT("SensorReading: %d. In volts: %f.", sensorReading, ConvertADCtoVolts(sensorReading));
    	UART_PRINT("HighByte: %d, LowByte: %d\n\r", (unsigned int) highByte, (unsigned int) lowByte);

    	sensorReading = GetSensorReading(SENSOR_FINGER_MIDDLE);
    	UnsignedShort_to_UnsignedChar(sensorReading, &highByte, &lowByte);
    	UART_PRINT("FINGER_MIDDLE - ");
    	UART_PRINT("SensorReading: %d. In volts: %f.", sensorReading, ConvertADCtoVolts(sensorReading));
    	UART_PRINT("HighByte: %d, LowByte: %d\n\r", (unsigned int) highByte, (unsigned int) lowByte);

    	sensorReading = GetSensorReading(SENSOR_FINGER_RING);
    	UnsignedShort_to_UnsignedChar(sensorReading, &highByte, &lowByte);
    	UART_PRINT("FINGER_RING - ");
    	UART_PRINT("SensorReading: %d. In volts: %f.", sensorReading, ConvertADCtoVolts(sensorReading));
    	UART_PRINT("HighByte: %d, LowByte: %d\n\r", (unsigned int) highByte, (unsigned int) lowByte);

        MAP_UtilsDelay(20000000);
        UART_PRINT("=========================================\n\r");
    }
}

//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - ADC
// Application Overview - The application is a reference to usage of ADC DriverLib 
//                        functions on CC3200. Developers/Users can refer to this 
//                        simple application and re-use the functions in 
//                        their applications.
//
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_ADC
// or
// docs\examples\CC32xx_ADC.pdf
//
//*****************************************************************************
