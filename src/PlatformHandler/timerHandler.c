
#include <stdbool.h>
#include "W7500x.h"
#include "timerHandler.h"
#include "DHCP/dhcp.h"
#include "httpServer.h" // for function httpServer_time_handler()

volatile uint16_t msec_cnt = 0;
volatile uint8_t sec_cnt = 0;

void DUALTIMER0_Handler(void)
{
	if(DUALTIMER_GetIntStatus(DUALTIMER0_0))
	{
		DUALTIMER_IntClear(DUALTIMER0_0);

		//Todo
		msec_cnt++; // milli second
		if(msec_cnt >= 1000 - 1) // second
		{
			sec_cnt++;

			httpServer_time_handler();
			DHCP_time_handler();
			msec_cnt = 0;
		}

		if(sec_cnt >= 60 - 1)
		{
			sec_cnt = 0;
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
