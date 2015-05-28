
#include <stdio.h>
#include <stdarg.h>
#include "boardutil.h"
#include "types.h"
#include "userHandler.h"

#if defined(FACTORY_FW)
#include "adcHandler.h"
#include "extiHandler.h"
#include "userHandler.h"
#include "gpioHandler.h"
#include "uartHandler.h"
#endif

//#define _WEB_DEBUG_

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#if defined(FACTORY_FW)
int teststep = 0;
int g_factoryfw_flag = 0;
#endif

#if defined(F_SPI_FLASH)
int g_spiflash_flag = 0;
#endif

#if defined(FACTORY_FW)
#define		FACTORY_TEST_STR		"TEST"
#define		FACTORY_CHECK_STR		"FTContinue"

extern IOStorage IOdata;
extern int teststep;

int check_factory_flag (void)
{
	int ret = 0;

	if(IOdata.factory_flag[0] == 0xFF && IOdata.factory_flag[1] == 0xFF)
	{
		ret = 0;
	}
	else if(IOdata.factory_flag[0] == 0xA5 && IOdata.factory_flag[1] == 0xA5)
	{
		ret = 1;
	}
	else
	{
		ret = 2;
	}

	return ret;
}

void check_factory_uart1 (void)
{
	uint8_t * buf_ptr;

	buf_ptr = getUSART1buf();

	if(strncmp(buf_ptr, FACTORY_CHECK_STR, sizeof(FACTORY_CHECK_STR)) == 0)
	{
		// Serial data buffer clear
		USART1_flush();

		factory_test_2nd();

#if defined(FACTORY_FW_FLASH)
		release_factory_flag();
#else
		//g_factoryfw_flag = 0;
#endif

		//delay_ms(200);
		//NVIC_SystemReset();
	}
}

void check_RS422 (uint8_t * buf)
{
#if defined(FACTORY_FW_FLASH)
	if(check_factory_flag() == 1)
#else
	if(g_factoryfw_flag == 1)
#endif
	{
		if(strncmp(buf, FACTORY_TEST_STR, sizeof(FACTORY_TEST_STR)) == 0)
		{
			printf("########## RS422 RX:%s OK.\r\n", buf);
		}
		else
		{
			printf("########## RS422 RX:%s Fail.\r\n", buf);
		}
	}
}

void save_factory_flag (void)
{
	IOdata.factory_flag[0] = 0xA5;
	IOdata.factory_flag[1] = 0xA5;

	write_IOstorage(&IOdata, sizeof(IOdata));
}

void release_factory_flag (void)
{
#if defined(FACTORY_FW_ONCE)
	IOdata.factory_flag[0] = 0xFF;
	IOdata.factory_flag[1] = 0xFF;
#endif
	IOdata.io_statuscode[0] = 0xFF;
	IOdata.io_statuscode[1] = 0xFF;

	write_IOstorage(&IOdata, sizeof(IOdata));
	//IO_status_init();
}

void factory_test_1st (void)
{
	int i;
	//uint16 adc_val = 0;
	int fail_count;

	// check A0~A3
	if (teststep == 0)
	{
		fail_count = 0;
#if 0
		for(i = 0; i < 4; i++)
		{
			adc_val = ADC_DualConvertedValueTab[i];
			if (adc_val > 100) //TODO
			{
				delay_ms(1);
		    	//printf("########## A%d[%d] OK.\r\n", i, adc_val);
			}
			else
			{
				fail_count += 1;
		    	printf("########## A%d[%d] Fail.\r\n", i, adc_val);
			}
		}
#endif
		if(fail_count == 0)
		{
			printf("########## A0 = %d || A1 = %d || A2 = %d || A3 = %d\r\n", ADC_DualConvertedValueTab[0]
			                                                                , ADC_DualConvertedValueTab[1]
			                                                                , ADC_DualConvertedValueTab[2]
			                                                                , ADC_DualConvertedValueTab[3]);
		}

		teststep = 1;
	}

	// check RS422
	if (teststep == 1)
	{
		printf("########## RS422 TX:TEST\r\n");
		UART2_flush();
		UART_write(FACTORY_TEST_STR, sizeof(FACTORY_TEST_STR));
		UART_write("\r", 1);
		teststep = 2;
	}

	// check SW1~SW3
	if (teststep == 2)
	{
		Delay(10000000);
		EXTI_Configuration();
		USART1_flush();
		teststep = 3;
	}

#if 0
		// check VCC, Temperature
		if (teststep == 5)
		{
			adc_val = ADC_DualConvertedValueTab[0];

			if (adc_val > 4000)
			{
				printf("########## VCC[%d] OK.\r\n", adc_val);

				adc_val = ADC_DualConvertedValueTab[1];
				if (adc_val > 100) //TODO
					printf("########## Temperature[%d] OK.\r\n", adc_val);
				else
					printf("########## Temperature[%d] Fail.\r\n", adc_val);
	    		teststep = 6;
			}
		}
#endif

	//check_factory_uart1();
}

void factory_test_2nd (void)
{
	int i;
	//uint16 adc_val = 0;
	int fail_count;
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTI_Line12 | EXTI_Line2 | EXTI_Line3 | EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);

	// check D0~D7(output/off) D8~15 input/off
	if (teststep == 3)
	{
		//delay_ms(100);
		fail_count = 0;
		for(i = 0; i < IOn; i++)
		{
			if (get_IO_Status(i) == Off)
			{
				if(i < 8)
				{
					delay_ms(1);
					//printf("########## D%d output/off OK.\r\n", i);
				}
				else
				{
					delay_ms(1);
					//printf("########## D%d input/off OK.\r\n", i);
				}
			}
			else
			{
				if(i < 8)
				{
					fail_count += 1;
					printf("########## D%d output/off Fail.\r\n", i);
				}
				else
				{
					printf("########## D%d input/off Fail.\r\n", i);
					fail_count += 1;
				}
			}
		}

		if(fail_count == 0)
		{
			printf("########## D0~D7[output/off] D8~D15[input/off] OK.\r\n");
		}

		teststep = 4;
	}

	// check D0~D7(output/on) D8~15 input/on
	if (teststep == 4)
	{
		fail_count = 0;
		for(i = 0; i < 8/*IOn*/; i++)
		{
			IOdata.ios[i] = On;
		}

	   	for(i = 0; i < 8/*IOn*/; i++)
	   	{
	   		if(IOdata.io[i] == Output)
	   		{
	   			if(IOdata.ios[i] == Off)	IO_Off(i);
	   			else						IO_On(i);
	   		}
	   	}

		for(i = 0; i < IOn; i++)
		{
			if (get_IO_Status(i) == On)
			{
				if(i < 8)
				{
					delay_ms(1);
					//printf("########## D%d output/on OK.\r\n", i);
				}
				else
				{
					delay_ms(1);
					//printf("########## D%d input/on OK.\r\n", i);
				}
			}
			else
			{
				if(i < 8)
				{
					fail_count += 1;
					printf("########## D%d output/on Fail.\r\n", i);
				}
				else
				{
					fail_count += 1;
					printf("########## D%d input/on Fail.\r\n", i);
				}
			}
		}

		if(fail_count == 0)
		{
			printf("########## D0~D7[output/on] D8~D15[input/on] OK.\r\n");
		}

		teststep = 5;
	}

	// check A0~A3
	if (teststep == 5)
	{
		fail_count = 0;
#if 0
		for(i = 0; i < 4; i++)
		{
			adc_val = ADC_DualConvertedValueTab[i];
			if (adc_val > 100) //TODO
			{
				delay_ms(1);
		    	//printf("########## A%d[%d] OK.\r\n", i, adc_val);
			}
			else
			{
				fail_count += 1;
		    	printf("########## A%d[%d] Fail.\r\n", i, adc_val);
			}
		}
#endif
		if(fail_count == 0)
		{
			printf("########## A0 = %d || A1 = %d || A2 = %d || A3 = %d\r\n", ADC_DualConvertedValueTab[0]
			                                                                , ADC_DualConvertedValueTab[1]
			                                                                , ADC_DualConvertedValueTab[2]
			                                                                , ADC_DualConvertedValueTab[3]);
		}

		teststep = 6;
	}

	printf("########## Test Completed.\r\n");
	//check_factory_uart1();
}

void factory_run(void)
{
	if ((get_IO_Status(D10) == On) && (get_IO_Status(D11) == On) && (g_factoryfw_flag == 0))
	{
		printf("\r\n########## Factory Test is started.\r\n");
		g_factoryfw_flag = 1;

#if defined(FACTORY_FW_FLASH)
		save_factory_flag();

		if (check_factory_flag() == 1)
		{
			factory_test_1st();
		}
#else
		factory_test_1st();
#endif
	}

}

#endif

#if defined(F_SPI_FLASH)
extern IOStorage IOdata;
int check_spiflash_flag(void)
{
	int ret = 0;

	read_IOstorage(&IOdata, sizeof(IOdata));

	if(IOdata.spiflash_flag[0] == 0xAE && IOdata.spiflash_flag[1] == 0xAE)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

void save_spiflash_flag(void)
{
	IOdata.spiflash_flag[0] = 0xAE;
	IOdata.spiflash_flag[1] = 0xAE;

	write_IOstorage(&IOdata, sizeof(IOdata));
}

void release_spiflash_flag(void)
{
	IOdata.spiflash_flag[0] = 0xFF;
	IOdata.spiflash_flag[1] = 0xFF;

	write_IOstorage(&IOdata, sizeof(IOdata));
}

void sflash_run(void)
{
	if ((get_IO_Status(D8) == On) && (get_IO_Status(D9) == On) && (g_spiflash_flag == 0))
	{
		printf("\r\n########## SW1 and SW2 were pressed.\r\n");
		printf("########## Data flash flag was cleared.\r\n");
		printf("########## Please reset a target.\r\n");
		g_spiflash_flag = 1;
		release_spiflash_flag();
	}
}

#endif

void kick_watchdog (void)
{
	// Depend on system.
	IWDG_ReloadCounter(); // Feed IWDG
}
