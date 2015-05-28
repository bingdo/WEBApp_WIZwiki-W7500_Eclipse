/**
  ******************************************************************************
  * @file    WZTOE/Loopback/W7500x_conf.h 
  * @author  IOP Team
  * @version V1.0.0
  * @date    01-May-2015
  * @brief   Library configuration file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W7500X_CONF_H
#define __W7500X_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
#include "W7500x_adc.h"
#include "W7500x_crg.h"
#include "W7500x_gpio.h"	/* Use MDIO over GPIO */
#include "W7500x_exti.h"
#include "W7500x_dualtimer.h"
#include "W7500x_i2c.h"
//#include "W7500x_pwm.h"
#include "W7500x_rng.h"
#include "W7500x_ssp.h"
#include "W7500x_uart.h"
#include "W7500x_wdt.h"
#include "W7500x_wztoe.h"	/* Use WZTOE */

/* ioLibrary */
//#include "../ioLibrary/Ethernet/socket.h"
#include "socket.h"
#include "W7500x_conf.h"
#include "loopback.h"
#include "W7500x_miim.h" /* Library for MDIO */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/

#ifdef USE_FULL_ASSERT
    #define assert_param(expr)  ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__,__LINE__))
#else
    #define assert_param(expr)   ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __W7500X_CONF_H */


/******************* (C) COPYRIGHT 2015 WIZnet Co.,Ltd. *****END OF FILE****/
