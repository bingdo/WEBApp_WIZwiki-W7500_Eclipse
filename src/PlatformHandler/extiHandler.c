
#include "W7500x.h"
#include "timerHandler.h"

void EXTI_Handler(void)
{
	factory_flag = 1;

	NVIC_ClearPendingIRQ(EXTI_IRQn);
}

/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void EXTI_Configuration(void)
{
    NVIC_ClearPendingIRQ(EXTI_IRQn); // Pending bit Clear
    NVIC_EnableIRQ(EXTI_IRQn);       // EXTI Interrupt Enable
}
