/**
 * @file	adcHandler.c
 * @brief	ADC Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include "adcHandler.h"
#include "boardutil.h"

int ADC_Read(ADC_CH num)
{
    ADC_ChannelSelect (num); ///< Select ADC channel to CH0
    ADC_Start(); ///< Start ADC
    while(ADC_IsEOC()); ///< Wait until End of Conversion
    return (ADC_ReadData()); ///< read ADC Data
}
