/**
 * @file	uartHandler.c
 * @brief	UART Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "uartHandler.h"
#include "gpioHandler.h"

RINGBUFF_T txring, rxring;
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

uint32_t baud_table[11] = {
	300,
	600,
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200, 
	230400
};

#if 0
uint32_t Chip_UART_SendRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, const void *data, int bytes)
{
	uint32_t ret;
	uint8_t *p8 = (uint8_t *) data;

	/* Don't let UART transmit ring buffer change in the UART IRQ handler */
	Chip_UART_IntDisable(pUART, UART_IER_THREINT);

	/* Move as much data as possible into transmit ring buffer */
	ret = RingBuffer_InsertMult(pRB, p8, bytes);
	Chip_UART_TXIntHandlerRB(pUART, pRB);

	/* Add additional data to transmit ring buffer if possible */
	ret += RingBuffer_InsertMult(pRB, (p8 + ret), (bytes - ret));

	/* Enable UART transmit interrupt */
	Chip_UART_IntEnable(pUART, UART_IER_THREINT);

	return ret;
}
#else
uint32_t Chip_UART_SendRB(UART_TypeDef *pUART, RINGBUFF_T *pRB, const void *data, int bytes)
{
	uint32_t ret;
	uint8_t *p8 = (uint8_t *) data;
	uint8_t ch;

	/* Don't let UART transmit ring buffer change in the UART IRQ handler */
	//UART_ITConfig(pUART, UART_IT_FLAG_TXI, DISABLE);

	/* Move as much data as possible into transmit ring buffer */
	ret = RingBuffer_InsertMult(pRB, p8, bytes);

	/* Enable UART transmit interrupt */
	UART_ITConfig(pUART, UART_IT_FLAG_TXI, ENABLE);

	if(RingBuffer_Pop(pRB, &ch))
	{
		UART_SendData(pUART, ch);
	}
	//UART_SendData(pUART, 0x0a);

	return ret;
}
#endif

#if 0
int Chip_UART_ReadRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;
	int ret, is_full;

	is_full = RingBuffer_IsFull(pRB);
	ret = RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);

	if(is_full)
		Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	return ret;
}
#else
int Chip_UART_ReadRB(UART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;
	int ret;

	//UART_ITConfig(pUART, UART_IT_FLAG_RXI, DISABLE);

	ret = RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);

	UART_ITConfig(pUART, UART_IT_FLAG_RXI, ENABLE);

	return ret;
}
#endif

int Chip_UART_ReadRB_BLK(UART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;
	int ret;

	while(RingBuffer_IsEmpty(pRB));

	//UART_ITConfig(pUART, UART_IT_FLAG_RXI, DISABLE);

	ret = RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);

	UART_ITConfig(pUART, UART_IT_FLAG_RXI, ENABLE);

	return ret;
}

/**
 * @brief  USART Handler
 * @param  None
 * @return None
 */
void Chip_UART_IRQRBHandler(UART_TypeDef *pUART, RINGBUFF_T *pRXRB, RINGBUFF_T *pTXRB)
{
	uint8_t ch_tx;
	uint8_t ch_rx;

	/* Handle transmit interrupt if enabled */
	if(UART_GetITStatus(pUART, UART_IT_FLAG_TXI)) {
        UART_ClearITPendingBit(pUART,UART_IT_FLAG_TXI);
		if(RingBuffer_Pop(pTXRB, &ch_tx))
		{
			UART_SendData(pUART, ch_tx);
		}
		else												// RingBuffer Empty
		{
			UART_ITConfig(pUART, UART_IT_FLAG_TXI, DISABLE);
		}
	}

	/* Handle receive interrupt */
	if(UART_GetITStatus(pUART, UART_IT_FLAG_RXI)) {
		if(RingBuffer_IsFull(pRXRB)) {
			// Buffer Overflow
		} else {
			ch_rx = UART_ReceiveData(pUART);
			RingBuffer_Insert(pRXRB, &ch_rx);
		}
	}
}

/**
 * @brief  UART0 Interrupt Handler
 * @param  None
 * @return None
 */
void UART0_Handler(void)
{
	Chip_UART_IRQRBHandler(UART0, &rxring, &txring);
}

/**
 * @brief  UART1 Interrupt Handler
 * @param  None
 * @return None
 */
void UART1_Handler(void)
{
	Chip_UART_IRQRBHandler(UART1, &rxring, &txring);
}

/**
 * @brief  UART2 Interrupt Handler
 * @param  None
 * @return None
 */
void UART2_Handler(void)
{
	Chip_UART_IRQRBHandler(UART2, &rxring, &txring);
}

/**
 * @brief  Configures the USART0
 * @param  None
 * @return None
 */
void UART_Configuration(void)
{
	UART_InitTypeDef UART_InitStructure;
	struct __serial_info *serial = (struct __serial_info *)(get_S2E_Packet_pointer()->serial_info);

	/* Ring Buffer */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

    /* UART0 configuration*/
	serial_info_init(&UART_InitStructure, serial);
    //UART_StructInit(&UART_InitStructure);
    /* Configure UART0 */
    UART_Init(UART_DATA,&UART_InitStructure);
    /* Configure Uart0 Interrupt Enable*/
    UART_ITConfig(UART_DATA, (UART_IT_FLAG_TXI|UART_IT_FLAG_RXI),ENABLE);
    /* NVIC configuration */
#if defined(UART0_DATA)
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART0_IRQn);
#elif defined(UART1_DATA)
    NVIC_ClearPendingIRQ(UART1_IRQn);
    NVIC_EnableIRQ(UART1_IRQn);
#else
#endif
}

int UART_read(void *data, int bytes)
{
	return Chip_UART_ReadRB(UART_DATA , &rxring, data, bytes);
}

uint32_t UART_write(void *data, int bytes)
{
	return Chip_UART_SendRB(UART_DATA, &txring, data, bytes);
}

int UART_read_blk(void *data, int bytes)
{
	return Chip_UART_ReadRB_BLK(UART_DATA , &rxring, data, bytes);
}

void myprintf(char *fmt, ...)
{
	va_list arg_ptr;
	char etxt[128]; // buffer size

	va_start(arg_ptr, fmt);
	vsprintf(etxt, fmt, arg_ptr);
	va_end(arg_ptr);

#if 1
	UART_write(etxt, strlen(etxt));
#else
	UartPuts(UART_DATA, etxt);
#endif
}

void serial_info_init(UART_InitTypeDef* UART_InitStruct, struct __serial_info *serial)
{
	uint32_t i, loop, valid_arg = 0;

	loop = sizeof(baud_table) / sizeof(baud_table[0]);
	for(i = 0 ; i < loop ; i++) {
		if(serial->baud_rate == baud_table[i]) {
			UART_InitStruct->UART_BaudRate = serial->baud_rate;
			valid_arg = 1;
			break;
		}
	}
	if(!valid_arg)
		UART_InitStruct->UART_BaudRate = baud_115200;

	/* Set Data Bits */
	switch(serial->data_bits) {
		case word_len8:
			UART_InitStruct->UART_WordLength = UART_WordLength_8b;
			break;
		case word_len7:
			UART_InitStruct->UART_WordLength = UART_WordLength_7b;
			break;
		default:
			UART_InitStruct->UART_WordLength = UART_WordLength_8b;
			serial->data_bits = word_len8;
			break;
	}

	/* Set Stop Bits */
	switch(serial->stop_bits) {
		case stop_bit1:
			UART_InitStruct->UART_StopBits = UART_StopBits_1;
			break;
		case stop_bit2:
			UART_InitStruct->UART_StopBits = UART_StopBits_2;
			break;
		default:
			UART_InitStruct->UART_StopBits = UART_StopBits_1;
			serial->stop_bits = stop_bit1;
			break;
	}

	/* Set Parity Bits */
	switch(serial->parity) {
		case parity_none:
			UART_InitStruct->UART_Parity = UART_Parity_No;
			break;
		case parity_odd:
			UART_InitStruct->UART_Parity = UART_Parity_Odd;
			break;
		case parity_even:
			UART_InitStruct->UART_Parity = UART_Parity_Even;
			break;
		default:
			UART_InitStruct->UART_Parity = UART_Parity_No;
			serial->parity = parity_none;
			break;
	}

	/* Flow Control */
	switch(serial->parity) {
	case flow_none:
		UART_InitStruct->UART_HardwareFlowControl = UART_HardwareFlowControl_None;
		break;
	case flow_rts_cts:
		UART_InitStruct->UART_HardwareFlowControl = UART_HardwareFlowControl_RTS_CTS;
		break;
	default:
		UART_InitStruct->UART_HardwareFlowControl = UART_HardwareFlowControl_None;
		break;
	}

	/* Configure the USARTx */
	UART_InitStruct->UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
}

void UART_buffer_flush(RINGBUFF_T *buf)
{
	RingBuffer_Flush(buf);
}

void UART_flush(void)
{
	//RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	//RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);
	rxring.head = 0;
	rxring.tail = 0;
	memset(rxbuff, 0x00, UART_RRB_SIZE);
}

int UART_RxRB_GetCount()
{
	return RingBuffer_GetCount(&rxring);
}

uint8_t * getUSART1buf(void)
{
	uint8_t * u1rx_buf_ptr = rxbuff;
	uint16_t slen = strlen((char *)rxbuff);

	if(slen) u1rx_buf_ptr = rxbuff;
	else u1rx_buf_ptr = 0;

	return u1rx_buf_ptr;
}
