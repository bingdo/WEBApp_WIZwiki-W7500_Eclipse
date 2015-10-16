/**
 * @file	uartHandler.h
 * @brief	Header File for UART Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#ifndef __UARTHANDLER_H__
#define __UARTHANDLER_H__

#include "W7500x.h"
#include "ring_buffer.h"
#include "ConfigData.h"

#define UART_SRB_SIZE 1024	/* Send */
#define UART_RRB_SIZE 1024	/* Receive */

#define UART_DEBUG UART1
#define UART_DATA UART0
#define UART0_DATA
//#define UART1_DATA

extern RINGBUFF_T txring, rxring;
extern uint32_t baud_table[11];

void UART_Configuration(void);

uint32_t Chip_UART_SendRB(UART_TypeDef *pUART, RINGBUFF_T *pRB, const void *data, int bytes);
int UART_read(void *data, int bytes);
uint32_t UART_write(void *data, int bytes);
int UART_read_blk(void *data, int bytes);
void myprintf(char *fmt, ...);
void serial_info_init(UART_InitTypeDef* UART_InitStruct, struct __serial_info *serial);
void UART_buffer_flush(RINGBUFF_T *buf);
void UART_flush(void);
int UART_RxRB_GetCount();
uint8_t * getUSART1buf(void);

#endif

