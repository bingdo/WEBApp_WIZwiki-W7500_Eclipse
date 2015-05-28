/*
 * storageHandler.c
 *
 *  Created on: 2014. 9. 16.
 *      Author: Raphael Kwon
 */


#include <string.h>
#include "W7500x.h"
#include "storageHandler.h"
#include "flashHandler.h"

int read_storage(uint8_t isConfig, void *data, uint16_t size)
{
	uint32_t address;

	if(isConfig == 1) {
#if !defined(MULTIFLASH_ENABLE)
		address = (FLASH_PAGE_SIZE*2) + CONFIG_PAGE_ADDR;
#else
		address = flash.flash_page_size + flash.config_page_addr;
#endif
	}
	else {
#if !defined(MULTIFLASH_ENABLE)
		address = 0x00000000 + CONFIG_PAGE_ADDR;
#else
		address = 0x00000000 + flash.config_page_addr;
#endif
	}

	return read_flash(address, data, size);
}

int write_storage(uint8_t isConfig, void *data, uint16_t size)
{
	uint32_t address;
	int ret;

	if(isConfig == 1) {
#if !defined(MULTIFLASH_ENABLE)
		address = (FLASH_PAGE_SIZE*2) + CONFIG_PAGE_ADDR;
#else
		address = flash.flash_page_size + flash.config_page_addr;
#endif
	}
	else {
#if !defined(MULTIFLASH_ENABLE)
		address = 0x00000000 + CONFIG_PAGE_ADDR;
#else
		address = 0x00000000 + flash.config_page_addr;
#endif
	}

	erase_flash_page(address);
	erase_flash_page(address+FLASH_PAGE_SIZE);

	ret = write_flash(address, data, size);

	return ret;
}
