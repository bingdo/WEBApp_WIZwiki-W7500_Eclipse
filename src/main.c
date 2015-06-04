/**
  ******************************************************************************
  * @file    main.c
  * @author  IOP Team
  * @version V1.0.0
  * @date    01-May-2015
  * @brief   Main program body
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
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "W7500x.h"
#include "common.h"
#include "uartHandler.h"
#include "flashHandler.h"
#include "storageHandler.h"
#include "gpioHandler.h"
#include "timerHandler.h"
#include "ConfigData.h"
#include "ConfigMessage.h"
#include "extiHandler.h"
#include "DHCP/dhcp.h"
#include "DNS/dns.h"
#include "S2E.h"
#include "dhcp_cb.h"
#include "atcmd.h"
#include "httpServer.h"
#include "userHandler.h"
#include "adcHandler.h"
#include "ftpd.h"
#include "boardutil.h"

#ifdef _USE_SDCARD_
#include "ff.h"
#include "mmcHandler.h"
#include "ffconf.h"
#include "mmc_sd.h"
#include "diskio.h"
#endif

/* Private typedef -----------------------------------------------------------*/
UART_InitTypeDef UART_InitStructure;

/* Private define ------------------------------------------------------------*/
#define __DEF_USED_MDIO__
#define __DEF_USED_IC101AG__ //for W7500 Test main Board V001

///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_

///////////////////////////
// Demo Firmware Version //
///////////////////////////
#define VER_H		1
#define VER_L		00

/* Private function prototypes -----------------------------------------------*/
void delay(__IO uint32_t milliseconds); //Notice: used ioLibray
void TimingDelay_Decrement(void);

/* Private variables ---------------------------------------------------------*/
/* Transmit and receive buffers */
static __IO uint32_t TimingDelay;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint8_t g_send_buf[_MAX_SS];
uint8_t g_recv_buf[_MAX_SS];
uint8_t RX_BUF[WORK_BUF_SIZE];
uint8_t TX_BUF[WORK_BUF_SIZE];
#if defined(F_APP_FTP)
uint8_t FTP_DBUF[_MAX_SS];
#endif

#if defined(F_APP_FTP)
#define MAX_HTTPSOCK	2
#else
#define MAX_HTTPSOCK	4
#endif

#if defined(F_APP_FTP)
uint8_t socknumlist[] = {6, 7};
#else
uint8_t socknumlist[] = {4, 5, 6, 7};
#endif

int g_mkfs_done = 0;
int g_sdcard_done = 0;

uint8_t run_dns = 1;
uint8_t op_mode;
uint8_t factory_flag = 0;

#if 0
static int vector_in_ram[128] __attribute__((section ("vectortable")));

typedef struct {			/*!< SYSCTL Structure */
	__IO uint32_t  SYSCFGREMAP;		/*!< System Memory remap register */
} SYSCFG_T;

#define SYSCFG		((SYSCFG_T	*) 0x4001F000UL)

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void vector_relocation_ram(void)
{
	int i;
	volatile uint32_t *p = (volatile uint32_t *)APP_BASE;

	/* Vector Table Relocation */
	__disable_irq();

	for(i = 0 ; i < 128 ; i++) {
		vector_in_ram[i] = *(p + i);
	}
	SYSCFG->SYSCFGREMAP = (uint32_t) 0x02;

	__enable_irq();
}
#endif

static void display_SDcard_Info(uint8_t mount_ret);

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main()
{
    //uint8_t tx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    //uint8_t rx_size[8] = { 2, 2, 2, 2, 2, 2, 2, 2 };
    //uint8_t mac_addr[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33};
    //uint8_t src_addr[4] = {192, 168,  0,  80};
    //uint8_t gw_addr[4]  = {192, 168,  0,  1};
    //uint8_t sub_addr[4] = {255, 255, 255,  0};
    //uint8_t dns_server[4] = {8, 8, 8, 8};           // for Example domain name server
    //uint8_t tmp[8];
	//int ret;
	int i;
#if defined (_MAIN_DEBUG_) && defined (_USE_SDCARD_)
	int ret;
#endif
#if defined(F_APP_FTP)
	wiz_NetInfo gWIZNETINFO;
#endif
	S2E_Packet *value = get_S2E_Packet_pointer();
	uint8_t dns_server_ip[4];

    /* External Clock */
    CRG_PLL_InputFrequencySelect(CRG_OCLK);

    /* Set System init */
    SystemInit();

	//vector_relocation_ram();

    /* UART Init */
    UART_StructInit(&UART_InitStructure);
    UART_Init(UART_DEBUG,&UART_InitStructure);

    /* SysTick_Config */
    SysTick_Config((GetSystemClock()/1000));

    /* Set WZ_100US Register */
    setTIC100US((GetSystemClock()/10000));
    //getTIC100US();	
    //printf(" GetSystemClock: %X, getTIC100US: %X, (%X) \r\n", 
    //      GetSystemClock, getTIC100US(), *(uint32_t *)TIC100US);        

	LED_Init(LED1);
	LED_Init(LED2);
	LED_Init(LED3);

	LED_Off(LED1);
	LED_Off(LED2);
	LED_Off(LED3);

	g_sdcard_done = 0;

	BOOT_Pin_Init();
	Board_factory_Init();
	UART_Configuration();
	EXTI_Configuration();

	/* Load Configure Information */
	load_S2E_Packet_from_storage();

	/* Check MAC Address */
	check_mac_address();

	Timer0_Configuration();

    // ADC initialize
    ADC_Init();

#ifdef _MAIN_DEBUG_
	uint8_t tmpstr[6] = {0,};

	ctlwizchip(CW_GET_ID,(void*)tmpstr);
    printf("\r\n============================================\r\n");
	printf(" WIZnet %s EVB Demo v%d.%.2d\r\n", tmpstr, VER_H, VER_L);
	printf("============================================\r\n");
	printf(" WIZwiki Platform based WEBApp Example\r\n");
	printf("============================================\r\n");
#endif

#ifdef __DEF_USED_IC101AG__ //For using IC+101AG
    *(volatile uint32_t *)(0x41003068) = 0x64; //TXD0 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x4100306C) = 0x64; //TXD1 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003070) = 0x64; //TXD2 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003074) = 0x64; //TXD3 - set PAD strengh and pull-up
    *(volatile uint32_t *)(0x41003050) = 0x64; //TXE  - set PAD strengh and pull-up
#endif

#ifdef __DEF_USED_MDIO__ 
    /* mdio Init */
    mdio_init(GPIOB, MDC, MDIO );
    /* PHY Link Check via gpio mdio */
    while( link() == 0x0 )
    {
        printf(".");  
        delay(500);
    }
    printf("PHY is linked. \r\n");  
#else
    delay(1000);
    delay(1000);
#endif

	Mac_Conf();
	DHCP_init(SOCK_DHCP, TX_BUF);

	/* Initialize Network Information */
	if(value->options.dhcp_use) {		// DHCP
		uint32_t ret;
		uint8_t dhcp_retry = 0;

		//printf("Start DHCP...\r\n");
		while(1) {
			ret = DHCP_run();

			if(ret == DHCP_IP_LEASED)
				break;
			else if(ret == DHCP_FAILED)
				dhcp_retry++;

			if(dhcp_retry > 3) {
				Net_Conf();
				break;
			}
			do_udp_config(SOCK_CONFIG);
		}
	} else 								// Static
		Net_Conf();

	DNS_init(SOCK_DNS, TX_BUF);
	if(value->options.dns_use) {
		uint8_t dns_retry = 0;

		memcpy(dns_server_ip, value->options.dns_server_ip, sizeof(dns_server_ip));

		while(1) {
			if(DNS_run(dns_server_ip, (uint8_t *)value->options.dns_domain_name, value->network_info[0].remote_ip) == 1)
				break;
			else
				dns_retry++;

			if(dns_retry > 3) {
				break;
			}

			do_udp_config(SOCK_CONFIG);

			if(value->options.dhcp_use)
				DHCP_run();
		}
	}

#ifdef _MAIN_DEBUG_
	display_Net_Info();
#endif

#ifdef _USE_SDCARD_
	// SD card Initialization
	ret = mmc_mount();
	if(ret <= 0)
	{
#ifdef _MAIN_DEBUG_
		printf("\r\n - Can't mount SD card: Please Reboot WIZ750WEB Board or Insert SD card\r\n");
#endif
		//while(!(ret = mmc_mount()));
	}

	if(ret > 0)
	{
#ifdef _MAIN_DEBUG_
		display_SDcard_Info(ret);
#endif
	}
#endif

	atc_init(&rxring, &txring);

	op_mode = OP_DATA;

	httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist);
#ifdef _USE_WATCHDOG_
	reg_httpServer_cbfunc(NVIC_SystemReset, IWDG_ReloadCounter); // Callback: STM32 MCU Reset / WDT Reset (IWDG)
#else
	reg_httpServer_cbfunc(NVIC_SystemReset, NULL); // Callback: STM32 MCU Reset
#endif
	IO_status_init();

#if defined(F_APP_FTP)
	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	ftpd_init(gWIZNETINFO.ip);	// Added by James for FTP
#endif

#ifdef _USE_WATCHDOG_
	// IWDG Initialization: STM32 Independent WatchDog
	IWDG_Configureation();
#endif

	while (1) {
#ifdef _USE_WATCHDOG_
		IWDG_ReloadCounter(); // Feed IWDG
#endif

#ifdef F_USE_DATA_MODE
		if(op_mode == OP_COMMAND) {			// Command Mode
			atc_run();
			sockwatch_run();
		} else {							// DATA Mode
			s2e_run(SOCK_DATA);
		}
#else
		atc_run();
#endif
		
		do_udp_config(SOCK_CONFIG);

		if(value->options.dhcp_use)
			DHCP_run();

		if(value->options.dns_use && run_dns == 1) {
			memcpy(dns_server_ip, value->options.dns_server_ip, sizeof(dns_server_ip));

			if(DNS_run(dns_server_ip, (uint8_t *)value->options.dns_domain_name, value->network_info[0].remote_ip) == 1) {
				run_dns = 0;
			}
		}

		for(i = 0; i < MAX_HTTPSOCK; i++)	httpServer_run(i);	// HTTP server handler

#if defined(F_APP_FTP)
		ftpd_run(FTP_DBUF);
#endif

#ifdef _USE_WATCHDOG_
		IWDG_ReloadCounter(); // Feed IWDG
#endif

	}

    return 0;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
  TimingDelay = milliseconds;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

#ifdef _USE_SDCARD_
static void display_SDcard_Info(uint8_t mount_ret)
{
	uint32_t totalSize = 0, availableSize = 0;

	printf("\r\n - Storage mount succeed\r\n");
	printf(" - Type : ");

	switch(mount_ret)
	{
		case CARD_MMC: printf("MMC\r\n"); 	break;
		case CARD_SD: printf("SD\r\n"); 	break;
		case CARD_SD2: printf("SD2\r\n"); 	break;
		case CARD_SDHC: printf("SDHC\r\n"); break;
		case SPI_FLASHM: printf("sFlash\r\n"); break;
		default: printf("\r\n"); 	break;
	}

#if 1
	if(_MAX_SS == 512)
	{
		getMountedMemorySize(mount_ret, &totalSize, &availableSize);
		printf(" - Available Memory Size : %ld kB / %ld kB ( %ld kB is used )\r\n", availableSize, totalSize, (totalSize - availableSize));
	}
	printf("\r\n");
#endif
}
#endif
