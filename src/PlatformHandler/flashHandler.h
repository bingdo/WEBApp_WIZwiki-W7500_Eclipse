
#ifndef __FLASHHANDLER_H__
#define __FLASHHANDLER_H__

#include <stdint.h>

//#define MULTIFLASH_ENABLE

#define IAP_ENTRY 			0x1FFF1001            // Because Thum code
#define IAP_ERAS            0x010
#define IAP_ERAS_DAT0       (IAP_ERAS + 0)
#define IAP_ERAS_DAT1       (IAP_ERAS + 1)
#define IAP_ERAS_SECT       (IAP_ERAS + 2)
#define IAP_ERAS_BLCK       (IAP_ERAS + 3)
#define IAP_ERAS_CHIP       (IAP_ERAS + 4)
#define IAP_ERAS_MASS       (IAP_ERAS + 5)
#define IAP_PROG            0x022
#define DAT0_START_ADDR     0x0003FE00
#define DAT1_START_ADDR     0x0003FF00
#define BLOCK_SIZE      	4096
#define SECT_SIZE       	256

#define FLASH_PAGE0_BASE	0x00000000
#define FLASH_CONFIG_PAGE	6//4
#if !defined(MULTIFLASH_ENABLE)
#define FLASH_PAGE_SIZE		0x100
#define FLASH_PAGE			512
#define FLASH_BOOT_PAGE		0					// 1 page 256
#define CONFIG_PAGE_ADDR	(0x00000000+(FLASH_PAGE_SIZE*508))	// Page 508,509,510,511	(W7500, the last page of 128kB on-chip flash, 1kB size)
#define IO_PAGE_ADDR		(0x00000000+(FLASH_PAGE_SIZE*506))	// Page 506,507
#define FLASH_APP_PAGE		(FLASH_PAGE - FLASH_BOOT_PAGE - FLASH_CONFIG_PAGE)
#endif

#if defined(MULTIFLASH_ENABLE)
typedef struct __Probe_Flash {
	uint32_t flash_page0_base;
	uint32_t flash_app_base;
	uint16_t flash_page_size;
	uint16_t flash_page;
	uint8_t	 flash_boot_page;
	uint8_t	 flash_config_page;
	uint16_t flash_app_page ;
	uint32_t config_page_addr ;
} __attribute__((packed)) Probe_Flash;

extern Probe_Flash flash;

void probe_flash(void);
#endif

int addr_to_page(uint32_t addr);
void erase_flash_page(uint32_t page_addr);
int write_flash(uint32_t addr, uint8_t *data, uint32_t data_len);
int read_flash(uint32_t addr, uint8_t *data, uint32_t data_len);
void save_data(uint8_t *data, uint32_t data_len, uint16_t block_number);

void DO_IAP( uint32_t id, uint32_t dst_addr, uint8_t* src_addr, uint32_t size);

#endif
