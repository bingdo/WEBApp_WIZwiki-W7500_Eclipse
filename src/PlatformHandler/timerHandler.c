
#include <stdbool.h>
#include "W7500x.h"
#include "socket.h"
#include "ConfigData.h"
#include "timerHandler.h"
#include "uartHandler.h"
#include "S2E.h"
#include "DHCP/dhcp.h"
#include "DNS/dns.h"
#include "gpioHandler.h"
#include "httpServer.h" // for function httpServer_time_handler()

uint8_t nagle_flag = 0;
uint32_t nagle_time = 0;
uint32_t uart_recv_count = 0;

uint8_t reconn_flag = 0;			/* 0 : connect / 1 : NONE */
uint32_t reconn_time = 0;

uint8_t inactive_flag = 0;
uint32_t inactive_time = 0;
uint32_t ether_send_prev;
uint32_t ether_recv_prev;

uint8_t trigger_flag = 0;
uint32_t trigger_time = 0;

extern uint8_t factory_flag;
uint32_t factory_time = 0;

uint8_t auth_flag = 0;
uint32_t auth_time = 0;

extern uint8_t run_dns;

static uint32_t mill_cnt = 0;
static uint32_t sec_cnt = 0;

extern bool Board_factory_get(void);

void DUALTIMER0_Handler(void)
{
	struct __network_info *net = (struct __network_info *)get_S2E_Packet_pointer()->network_info;
	struct __options *option = (struct __options *)&(get_S2E_Packet_pointer()->options);
	uint32_t count = 0;

	if(DUALTIMER_GetIntStatus(DUALTIMER0_0))
	{
		DUALTIMER_IntClear(DUALTIMER0_0);

		mill_cnt++;

		/* UART Packing Timer Process */
		if(!nagle_flag && net->packing_time) {
			count = RingBuffer_GetCount(&rxring);

			if(count != 0 && count == uart_recv_count)
				nagle_time++;
			else {
				nagle_time = 0;
				uart_recv_count = count;
			}

			if(nagle_time >= net->packing_time) {
				nagle_flag = 1;
				nagle_time = 0;
			}
		}

		/* Reconnection Process */
		if(reconn_flag)
			reconn_time++;

		if(net->reconnection <= reconn_time) {
			reconn_flag = 0;
			reconn_time = 0;
		}

		/* Factory Reset Process */
		if(factory_flag) {
			factory_time++;

			if(Board_factory_get()) {
				factory_time = factory_flag = 0;
				NVIC_EnableIRQ(EXTI_IRQn);
			}

			if(factory_time >= 5000) {
				/* Factory Reset */
				set_S2E_Packet_to_factory_value();
				save_S2E_Packet_to_storage();

				NVIC_SystemReset();
			}
		}

		/* Serial Trigger Timer Process */
		if(trigger_flag == 1)
			trigger_time++;

		if(trigger_time >= 500) {
			trigger_flag = 2;
			trigger_time = 0;
		}

		/* Second Process */
		if((mill_cnt % 1000) == 0) {
			LED_Toggle(LED2);
			mill_cnt = 0;
			sec_cnt++;

			/* DHCP Process */
			if(option->dhcp_use)
				DHCP_time_handler();

			/* DNS Process */
			if(option->dns_use) {
				DNS_time_handler();
			}

			/* httpServer Process */
			httpServer_time_handler();

			/* Inactive Time Process */
			if(inactive_flag == 1) {
				if((ether_recv_cnt == ether_recv_prev) && (ether_send_cnt == ether_send_prev))
					inactive_time++;
				else {
					ether_send_prev = ether_send_cnt;
					ether_recv_prev = ether_recv_cnt;
					inactive_time = 0;
				}
			}

			if(net->inactivity && (net->inactivity <= inactive_time)) {
				inactive_flag = 2;
				inactive_time = 0;
			}

			/* Connect Password Process */
			if(auth_flag)
				auth_time++;

			/* Minute Process */
			if((sec_cnt % 60) == 0) {
				sec_cnt = 0;

				/* DNS Process */
				if(option->dns_use) {
					run_dns = 1;
				}
			}
		}
	}
}

/**
 * @brief  Configures the Timer
 * @param  None
 * @return None
 */
void Timer0_Configuration(void)
{
	DULATIMER_InitTypDef Dualtimer_InitStructure;

	NVIC_EnableIRQ(DUALTIMER0_IRQn);

    /* Dualtimer 0_0 clock enable */
    DUALTIMER_ClockEnable(DUALTIMER0_0);

    /* Dualtimer 0_0 configuration */
    Dualtimer_InitStructure.TimerLoad = 1000;//20000;//0x4FFFF;
    Dualtimer_InitStructure.TimerControl_Mode = DUALTIMER_TimerControl_Periodic;
    Dualtimer_InitStructure.TimerControl_OneShot = DUALTIMER_TimerControl_Wrapping;
    Dualtimer_InitStructure.TimerControl_Pre = DUALTIMER_TimerControl_Pre_16;
    Dualtimer_InitStructure.TimerControl_Size = DUALTIMER_TimerControl_Size_32;

    DUALTIMER_Init(DUALTIMER0_0, &Dualtimer_InitStructure);

    /* Dualtimer 0_0 Interrupt enable */
    DUALTIMER_IntConfig(DUALTIMER0_0, ENABLE);

    /* Dualtimer 0_0 start */
    DUALTIMER_Start(DUALTIMER0_0);
}

void delay_cnt(uint32_t count)
{
	volatile uint32_t tmp = count;

	while(tmp--);
}
