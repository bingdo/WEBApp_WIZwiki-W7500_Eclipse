
#include <stdbool.h>
#include "W7500x.h"
#include "gpioHandler.h"
#include "userHandler.h"

GPIO_TypeDef* GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN};
//PAD_Type GPIO_PAD[LEDn] = {LED1_GPIO_PAD, LED2_GPIO_PAD, LED3_GPIO_PAD};
//PAD_AF_TypeDef GPIO_AF[LEDn] = {LED1_GPIO_AF, LED2_GPIO_AF, LED3_GPIO_AF};

#if 1
GPIO_TypeDef* IO_PORT[IOn] = {D0_GPIO_PORT, D1_GPIO_PORT, D2_GPIO_PORT, D3_GPIO_PORT,
							  D4_GPIO_PORT, D5_GPIO_PORT, D6_GPIO_PORT, D7_GPIO_PORT,
							  D8_GPIO_PORT, D9_GPIO_PORT, D10_GPIO_PORT, D11_GPIO_PORT,
							  D12_GPIO_PORT, D13_GPIO_PORT, D14_GPIO_PORT, D15_GPIO_PORT
							  };
const uint16_t IO_PIN[IOn] = {D0_PIN, D1_PIN, D2_PIN, D3_PIN,
							  D4_PIN, D5_PIN, D6_PIN, D7_PIN,
							  D8_PIN, D9_PIN, D10_PIN, D11_PIN,
							  D12_PIN, D13_PIN, D14_PIN, D15_PIN
							  };
PAD_Type IO_PAD[IOn] = {D0_PAD, D1_PAD, D2_PAD, D3_PAD,
						D4_PAD, D5_PAD, D6_PAD, D7_PAD,
						D8_PAD, D9_PAD, D10_PAD, D11_PAD,
						D12_PAD, D13_PAD, D14_PAD, D15_PAD
						};
#else
GPIO_TypeDef* IO_PORT[IOn] = {D0_GPIO_PORT, D1_GPIO_PORT, D2_GPIO_PORT, D3_GPIO_PORT,
							  D4_GPIO_PORT, D5_GPIO_PORT, D6_GPIO_PORT, D7_GPIO_PORT
							  };
const uint16_t IO_PIN[IOn] = {D0_PIN, D1_PIN, D2_PIN, D3_PIN,
							  D4_PIN, D5_PIN, D6_PIN, D7_PIN
							  };
#endif

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

  GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);

  PAD_AFConfig(PAD_PC, GPIO_PIN[Led], PAD_AF1);

  delay_cnt(10000);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_On(Led_TypeDef Led)
{
  GPIO_ResetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Off(Led_TypeDef Led)
{
  GPIO_SetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Toggle(Led_TypeDef Led)
{
  GPIO_PORT[Led]->DATAOUT ^= GPIO_PIN[Led];
}

uint8_t get_LED_Status(Led_TypeDef Led)
{
	return GPIO_ReadOutputDataBit(GPIO_PORT[Led], GPIO_PIN[Led]);
}

/**
  * @brief	Configures GPIO for D0~D15
  * @param	io Specifies the GPIO to be configured.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void BOOT_Pin_Init()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
  	/* Configure the Boot trigger pin */
  	GPIO_InitStructure.GPIO_Pin = BOOT_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	
  	GPIO_Init(BOOT_GPIO_PORT, &GPIO_InitStructure);

    PAD_AFConfig(PAD_PA, BOOT_PIN, PAD_AF1);
}

/**
  * @brief	To get the status of GPIO.
  * @param	io Specifies the GPIO to get a status
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return The status of GPIO
  */
uint8_t get_bootpin_Status()
{
	return GPIO_ReadInputDataBit(BOOT_GPIO_PORT, BOOT_PIN);
}

bool Board_factory_get(void)
{
	return GPIO_ReadInputDataBit(BOOT_GPIO_PORT, FACT_PIN);
}

void Board_factory_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
    EXTI_InitTypeDef 	EXTI_InitDef;

  	/* Configure the Boot trigger pin */
  	GPIO_InitStructure.GPIO_Pin = FACT_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;

  	GPIO_Init(FACT_GPIO_PORT, &GPIO_InitStructure);

    PAD_AFConfig(PAD_PA, FACT_PIN, PAD_AF1);

    /* Set to GPIO_Pin_0 to External Interrupt Port */
    EXTI_InitDef.EXTI_Line = FACT_PIN; // Connecting FACT_PIN(EXTI Input)
    EXTI_InitDef.EXTI_Trigger = EXTI_Trigger_Falling; // Set to Trigger to Falling
    EXTI_Init(PAD_PA, &EXTI_InitDef); // Set to PAD_PA
    EXTI_Polarity_Set(PAD_PA,FACT_PIN,EXTI_Trigger_Falling); // Set to Polarity
}

/**
  * @brief	Configures GPIO for D0~D15
  * @param	io Specifies the GPIO to be configured.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void IO_Init(IO_Type io)
{
	GPIO_InitTypeDef	GPIO_InitStructure;

  	/* Configure the GPIO_LED pin */
  	GPIO_InitStructure.GPIO_Pin = IO_PIN[io];
  	if(IOdata.io[io] == Input)
 		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  	else
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

  	GPIO_Init(IO_PORT[io], &GPIO_InitStructure);

    //PAD_AFConfig(IO_PAD[io], IO_PIN[io], PAD_AF0);
}

/**
  * @brief	Turns selected GPIO On.
  * @param	io Specifies the GPIO to be set on.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void IO_On(IO_Type io)
{
	if(IOdata.io[io] == Output)
  		GPIO_SetBits(IO_PORT[io], IO_PIN[io]);
}

/**
  * @brief	Turns selected GPIO Off.
  * @param	io Specifies the GPIO to be set off.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void IO_Off(IO_Type io)
{
	if(IOdata.io[io] == Output)
		GPIO_ResetBits(IO_PORT[io], IO_PIN[io]);
}

/**
  * @brief	Toggles the selected GPIO.
  * @param	io Specifies the GPIO to be toggled.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void IO_Toggle(IO_Type io)
{
	if(IOdata.io[io] == Output)
  		IO_PORT[io]->DATAOUT ^= IO_PIN[io];
}

/**
  * @brief	To get the status of GPIO.
  * @param	io Specifies the GPIO to get a status
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return The status of GPIO
  */
uint8_t get_IO_Status(IO_Type io)
{
	if(IOdata.io[io] == Input)
		return GPIO_ReadInputDataBit(IO_PORT[io], IO_PIN[io]);
	else if (IOdata.io[io] == Output)
		return GPIO_ReadOutputDataBit(IO_PORT[io], IO_PIN[io]);
	else
		return RET_OK;
}

