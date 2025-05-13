/*
 * API_spi_flash.c
 *
 *  Created on: Jul 28, 2024
 *      Author: guirespi
 */
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "ezm_spi_flash_arch_common.h"

#include "ezm_api_spi_flash_def.h"
#include "ezm_api_spi_flash_priv.h"

#include "ezm_api_delay.h"

#include "ezm_mock_utils.h"

#define SPI_FLASH_GET_CHIP_STATE (spi_flash_chip.chip_state)
#define SPI_FLASH_SET_CHIP_STATE(new_state) (spi_flash_chip.chip_state = new_state)
#define SPI_FLASH_DEFAULT_WRITE_TIMEOUT (10) /* 10 milliseconds */
#define SPI_FLASH_DEFAULT_READ_TIMEOUT (10) /* 10 milliseconds */
#define SPI_FLASH_RESET_DEVICE_WAIT (1) /* 1 millisecond*/

#define SPI_FLASH_COMMAND_AND_ADDRESS_SIZE (4) /*One byte for command, three bytes for 24-bit address of chip */

/* The following values are set based in W25Q64JV datasheet and the max time for each operarion */
#define SPI_FLASH_WRITE_STATUS_MAX_TIMEOUT 	(15) /*< milliseconds */
#define SPI_FLASH_PROGRAM_PAGE_MAX_TIMEOUT 	(3) /*< milliseconds */
#define SPI_FLASH_SECTOR_ERASE_MAX_TIMEOUT 	(400) /*< milliseconds */
#define SPI_FLASH_BLOCK32_ERASE_MAX_TIMEOUT (1600) /*< milliseconds */
#define SPI_FLASH_BLOCK64_ERASE_MAX_TIMEOUT (2000) /*< milliseconds */
#define SPI_FLASH_CHIP_ERASE_MAX_TIMEOUT 	(100*1000) /*< milliseconds */

/* We initialize the chip state to EZM_SPI_FLASH_STATE_DISABLE */
STATIC_GLOBAL ezm_spi_flash_chip_t spi_flash_chip = {0};

/**
 * @brief SPI IT rx handler.
 *
 * @param spi_hdle SPI handle.
 * @param data Data received.
 * @param size Data size received.
 */
static void ezm_spi_flash_rx_it_hdle(void * spi_hdle, uint8_t * data, uint16_t size);
/**
 * @brief Send a command with arguments and poll to receive.
 *
 * @param command Command with arguments array.
 * @param command_size Command size.
 * @param response Response buffer.
 * @param response_size Expcted response size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_send_advanced_command_receive(uint8_t * command, uint16_t command_size, uint8_t * response, uint16_t response_size);
/**
 * @brief Send a command with arguments. No response expected.
 *
 * @param command Command with arguments array.
 * @param command_size Command size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_send_advanced_command(uint8_t * command, uint16_t command_size);
/**
 * @brief Send a basic command. No response expected.
 *
 * @param command Command.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_send_basic_command(uint8_t command);
/**
 * @brief Send basic command and poll to receive.
 *
 * @param command Command.
 * @param response Response buffer.
 * @param response_size Expected response size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_send_basic_command_receive(uint8_t command, uint8_t * response, uint16_t response_size);
/**
 * @brief Poll operation to get status register 1 of SPI flash.
 *
 * @param reg Buffer for register.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_get_status_reg_1(uint8_t * reg);
/**
 * @brief Wait until chip if status register is in write enable state.
 *
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_wait_until_chip_write_enable(void);
/**
 * @brief Wait until the chip finish a write/erase operation.
 *
 * @param ms Milliseconds for timeout.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_wait_until_chip_ready(uint32_t ms);
/**
 * @brief Polled operation to program a SPI flash page.
 *
 * @param buffer Buffer to program.
 * @param address Address to program.
 * @param size Size to program.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_program_page(uint8_t * buffer, uint32_t address, uint16_t size);
/**
 * @brief Polled operation to read SPI flash address.
 *
 * @param buffer Buffer.
 * @param address Address to read.
 * @param size Size to read.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 */
static int ezm_spi_flash_read_address(uint8_t * buffer, uint32_t address, uint32_t size);
/**
 * @brief Polled operation to erase a sector.
 *
 * @param address Address to erase. Aligned with sector size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_erase_sector(uint32_t address);
/**
 * @brief Polled operation to erase a 32kB block.
 *
 * @param address Address to erase. Aligned with sector size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_erase_block32(uint32_t address);
/**
 * @brief Polled operation to erase 64kB block.
 *
 * @param address Address to erase. Aligned with sector size.
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_erase_block64(uint32_t address);
/**
 * @brief Polled operation to erase all SPI FLASH.
 *
 * @return
 * 			- EZM_SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_TIMEOUT timeout reached.
 */
static int ezm_spi_flash_erase_chip(void);

static void ezm_spi_flash_rx_it_hdle(void * spi_hdle, uint8_t * data, uint16_t size)
{
	/* Do nothing. We did not use SPI IT functions */
	return;
}

static int ezm_spi_flash_send_advanced_command_receive(uint8_t * command, uint16_t command_size, uint8_t * response, uint16_t response_size)
{
	ezm_spi_flash_arch_select_cs();
	int rt = ezm_spi_flash_arch_write_spi(command, command_size, SPI_FLASH_DEFAULT_WRITE_TIMEOUT);
	rt |= ezm_spi_flash_arch_read_spi(response, response_size, SPI_FLASH_DEFAULT_READ_TIMEOUT);
	ezm_spi_flash_arch_deselect_cs();
	return rt;
}

static int ezm_spi_flash_send_advanced_command(uint8_t * command, uint16_t command_size)
{
	ezm_spi_flash_arch_select_cs();
	int rt = ezm_spi_flash_arch_write_spi(command, command_size, SPI_FLASH_DEFAULT_WRITE_TIMEOUT);
	ezm_spi_flash_arch_deselect_cs();
	return rt;
}


static int ezm_spi_flash_send_basic_command_receive(uint8_t command, uint8_t * response, uint16_t response_size)
{
	ezm_spi_flash_arch_select_cs();
	int rt = ezm_spi_flash_arch_write_spi(&command, sizeof(command), SPI_FLASH_DEFAULT_WRITE_TIMEOUT);
	rt |= ezm_spi_flash_arch_read_spi(response, response_size, SPI_FLASH_DEFAULT_READ_TIMEOUT);
	ezm_spi_flash_arch_deselect_cs();
	return rt;
}

static int ezm_spi_flash_send_basic_command(uint8_t command)
{
	ezm_spi_flash_arch_select_cs();
	int rt = ezm_spi_flash_arch_write_spi(&command, sizeof(command), SPI_FLASH_DEFAULT_WRITE_TIMEOUT);
	ezm_spi_flash_arch_deselect_cs();
	return rt;
}

static int ezm_spi_flash_get_status_reg_1(uint8_t * reg)
{
	int rt = ezm_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_1, reg, sizeof(*reg));
	uint8_t reg2 = 0, reg3 = 0;
	ezm_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_2, &reg2, sizeof(*reg));
	ezm_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_3, &reg3, sizeof(*reg));

	return rt;
}

static int ezm_spi_flash_wait_until_chip_write_enable(void)
{
	uint8_t reg = 0;
	int rt = ezm_spi_flash_send_basic_command(API_SPI_FLASH_CMD_WRITE_EN);
	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	ezm_delay_t read_delay = {0};
	ezm_delay_init(&read_delay, SPI_FLASH_WRITE_STATUS_MAX_TIMEOUT);

	bool timeout = false;
	while((timeout = ezm_delay_read(&read_delay)) == false)
	{
		rt = ezm_spi_flash_get_status_reg_1(&reg);

		if(rt != EZM_SPI_FLASH_OK)
		{
			rt = EZM_SPI_FLASH_E_IO;
			break;
		}

		if(API_SPI_FLASH_WEL_IS_SET(reg))
			break;
	}

	if(timeout)
		rt = EZM_SPI_FLASH_E_TIMEOUT;

	return rt;
}

static int ezm_spi_flash_wait_until_chip_ready(uint32_t ms)
{
	uint8_t reg = 0;
	int rt = EZM_SPI_FLASH_OK;

	ezm_delay_t read_delay = {0};
	ezm_delay_init(&read_delay, ms);

	bool timeout = false;
	while((timeout = ezm_delay_read(&read_delay)) == false)
	{
		rt = ezm_spi_flash_get_status_reg_1(&reg);

		if(rt != EZM_SPI_FLASH_OK)
		{
			rt = EZM_SPI_FLASH_E_IO;
			break;
		}

		if(!API_SPI_FLASH_WEL_IS_SET(reg) && !API_SPI_FLASH_BSY_IS_SET(reg))
			break;
	}

	if(timeout)
		rt = EZM_SPI_FLASH_E_TIMEOUT;

	return rt;
}

static int ezm_spi_flash_program_page(uint8_t * buffer, uint32_t address, uint16_t size)
{
	uint8_t * command = calloc(SPI_FLASH_COMMAND_AND_ADDRESS_SIZE + size, sizeof(*command));

	uint32_t command_address = (API_SPI_FLASH_CMD_WRITE_PAGE | SPI_FLASH_HTONL(address));
	memcpy((void *)command, (void *)&command_address, sizeof(command_address));
	memcpy((void *)(command + sizeof(command_address)), buffer, size);
	int rt = ezm_spi_flash_send_advanced_command(command, SPI_FLASH_COMMAND_AND_ADDRESS_SIZE + size);

	free(command);

	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	return ezm_spi_flash_wait_until_chip_ready(SPI_FLASH_PROGRAM_PAGE_MAX_TIMEOUT);
}

static int ezm_spi_flash_read_address(uint8_t * buffer, uint32_t address, uint32_t size)
{
	uint32_t command_address = (API_SPI_FLASH_CMD_READ_DATA | SPI_FLASH_HTONL(address));
	return ezm_spi_flash_send_advanced_command_receive((uint8_t *)&command_address, sizeof(command_address), buffer, size);
}

static int ezm_spi_flash_erase_sector(uint32_t address)
{
	uint32_t command_address = (API_SPI_FLASH_CMD_DEL_SECTOR | SPI_FLASH_HTONL(address));
	int rt = ezm_spi_flash_send_advanced_command((uint8_t *)&command_address, sizeof(command_address));
	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	return ezm_spi_flash_wait_until_chip_ready(SPI_FLASH_SECTOR_ERASE_MAX_TIMEOUT);
}

static int ezm_spi_flash_erase_block32(uint32_t address)
{
	uint32_t command_address = (API_SPI_FLASH_CMD_DEL_32KB_BLOCK | SPI_FLASH_HTONL(address));
	int rt = ezm_spi_flash_send_advanced_command((uint8_t *)&command_address, sizeof(command_address));
	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	return ezm_spi_flash_wait_until_chip_ready(SPI_FLASH_BLOCK32_ERASE_MAX_TIMEOUT);
}

static int ezm_spi_flash_erase_block64(uint32_t address)
{
	uint32_t command_address = (API_SPI_FLASH_CMD_DEL_64KB_BLOCK | SPI_FLASH_HTONL(address));
	int rt = ezm_spi_flash_send_advanced_command((uint8_t *)&command_address, sizeof(command_address));
	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	return ezm_spi_flash_wait_until_chip_ready(SPI_FLASH_BLOCK64_ERASE_MAX_TIMEOUT);
}

static int ezm_spi_flash_erase_chip(void)
{
	int rt = ezm_spi_flash_send_basic_command(API_SPI_FLASH_CMD_DEL_CHIP);
	if(rt != EZM_SPI_FLASH_OK)
		return rt;

	return ezm_spi_flash_wait_until_chip_ready(SPI_FLASH_CHIP_ERASE_MAX_TIMEOUT);
}

int ezm_spi_flash_init(ezm_spi_if_hdle spi_if_hdle, ezm_spi_flash_cs_t cs_gpio)
{
	int rt  = EZM_SPI_FLASH_E_IO;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_DISABLE)
	{
		rt = ezm_spi_flash_arch_init_spi(spi_if_hdle, ezm_spi_flash_rx_it_hdle);
		if(rt != EZM_SPI_FLASH_ARCH_OK) return EZM_SPI_FLASH_E_ARCH;
		rt = ezm_spi_flash_arch_init_cs(cs_gpio.port, cs_gpio.pin);
		if(rt != EZM_SPI_FLASH_ARCH_OK) return EZM_SPI_FLASH_E_ARCH;
		SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_INIT);
	}

	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_INIT)
	{
		do
		{
			uint8_t reg = 0;
			rt =  ezm_spi_flash_get_status_reg_1(&reg);
			if(rt != EZM_SPI_FLASH_OK)
				return EZM_SPI_FLASH_E_IO;

			if(!API_SPI_FLASH_WEL_IS_SET(reg) && !API_SPI_FLASH_BSY_IS_SET(reg))
				break;
			rt = ezm_spi_flash_send_basic_command(API_SPI_FLASH_CMD_ENABLE_RESET);
			if(rt != EZM_SPI_FLASH_OK)
				return EZM_SPI_FLASH_E_IO;

			rt = ezm_spi_flash_send_basic_command(API_SPI_FLASH_CMD_RESET_DEVICE);
			if(rt != EZM_SPI_FLASH_OK)
				return EZM_SPI_FLASH_E_IO;

			/* Specifications says that the device will take 30us when the reset command is accepted.
			 * The bare minimum delay we can do is 1 milliseconds.*/
			ezm_spi_flash_arch_block_delay(SPI_FLASH_RESET_DEVICE_WAIT);
		}while(true);
		SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_POWER_ON);
	}

	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_POWER_ON)
	{
		spi_flash_jedec_id jedec_id = {0};

		rt = ezm_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_JEDEC_ID, (uint8_t *)&jedec_id, (uint16_t)sizeof(jedec_id));
		if(rt != 0) return rt;

		/* No memory can have capacity of zero bytes. So, the readed JEDEC ID is a no valid response */
		if(jedec_id.memory_capacity == 0)
			return EZM_SPI_FLASH_E_FAIL;

		/* The chip size is the power of two of the third byte of the JEDEC ID */
		spi_flash_chip.vendor_id = jedec_id.manufacturer_id;
		spi_flash_chip.chip_type = jedec_id.memory_type;
		spi_flash_chip.chip_size = (1 << jedec_id.memory_capacity);

		SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_READY);
	}

	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_READY)
		rt = EZM_SPI_FLASH_OK;

	return rt;
}

int ezm_spi_flash_read(uint8_t * buffer, uint32_t address, uint32_t size)
{
	if(SPI_FLASH_GET_CHIP_STATE < EZM_SPI_FLASH_STATE_READY)
		return EZM_SPI_FLASH_E_READY;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_ERROR)
		return EZM_SPI_FLASH_E_FAIL;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_BUSY)
		return EZM_SPI_FLASH_E_BUSY;

	if(buffer == 0) return EZM_SPI_FLASH_E_NULL;
	if(size == 0) return EZM_SPI_FLASH_OK;
	if(address > spi_flash_chip.chip_size || (address + size) > spi_flash_chip.chip_size) return EZM_SPI_FLASH_E_BOUNDARIES;

	/* Save our last 'allowed' state for this operation */
	ezm_spi_flash_state_t last_state = SPI_FLASH_GET_CHIP_STATE;
	SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_BUSY);
	size_t read = 0;
	size_t remaining = size;
	while(remaining)
	{
		/* We are going to read sector by sector if possible to be sure the chip will respond faster the read command.
		 * If we try to read a lot, maybe the default time for read operation would not be enough*/
		size_t to_read = (remaining > SPI_FLASH_SECTOR_SIZE)? SPI_FLASH_SECTOR_SIZE : remaining;
		int err = ezm_spi_flash_read_address(buffer + read, address + read, to_read);
		if(err != EZM_SPI_FLASH_OK)
		{
			SPI_FLASH_SET_CHIP_STATE(last_state);
			return EZM_SPI_FLASH_E_IO;
		}
		read += to_read;
		remaining -= to_read;
	}
	SPI_FLASH_SET_CHIP_STATE(last_state);
	return EZM_SPI_FLASH_OK;
}

int ezm_spi_flash_write(uint8_t * buffer, uint32_t address, uint32_t size)
{
	if(SPI_FLASH_GET_CHIP_STATE < EZM_SPI_FLASH_STATE_READY)
		return EZM_SPI_FLASH_E_READY;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_ERROR)
		return EZM_SPI_FLASH_E_FAIL;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_BUSY)
		return EZM_SPI_FLASH_E_BUSY;

	if(size == 0) return EZM_SPI_FLASH_OK;
	if(address > spi_flash_chip.chip_size || (address + size) > spi_flash_chip.chip_size) return EZM_SPI_FLASH_E_BOUNDARIES;

	int rt = EZM_SPI_FLASH_OK;

	size_t wrote = 0;
	size_t remaining = size;

	SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_BUSY);

	while(remaining)
	{
		rt = ezm_spi_flash_wait_until_chip_write_enable();
		if(rt != EZM_SPI_FLASH_OK)
			return rt;

		size_t to_write = SPI_FLASH_PAGE_SIZE;
		if((address + wrote)%SPI_FLASH_PAGE_SIZE != 0)
		{
			/* We are writing to a not aligned page address. Set the max value 'to_write' can made.
			 * Later will validate with 'remaining' */
			uint16_t offset =  (address + wrote)%SPI_FLASH_PAGE_SIZE;
			to_write = (SPI_FLASH_PAGE_SIZE - offset);
		}

		/* If what are we going to write is greater than remaining, just write the remaining.
		 * If we are in a non aligned address, we compare with the max bytes we can write into
		 * that address. If we are aligned, we check if the default 'to write' value is lower
		 * than the remaining. Then write */
		if(to_write > remaining)
			to_write = remaining;

		int rt = ezm_spi_flash_program_page(buffer + wrote, address + wrote, to_write);
		if(rt != EZM_SPI_FLASH_OK)
		{
			SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_ERROR);
			return rt;
		}
		wrote += to_write;
		remaining -= to_write;
	}
	SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_READY);
	return rt;
}

int ezm_spi_flash_erase_range(size_t address, uint32_t size)
{
	if(SPI_FLASH_GET_CHIP_STATE < EZM_SPI_FLASH_STATE_READY)
		return EZM_SPI_FLASH_E_READY;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_ERROR)
		return EZM_SPI_FLASH_E_FAIL;
	if(SPI_FLASH_GET_CHIP_STATE == EZM_SPI_FLASH_STATE_BUSY)
		return EZM_SPI_FLASH_E_BUSY;

	if(size == 0) return EZM_SPI_FLASH_OK;
	if(address % SPI_FLASH_SECTOR_SIZE != 0) return EZM_SPI_FLASH_E_ADDRESS;
	if(size % SPI_FLASH_SECTOR_SIZE != 0) return EZM_SPI_FLASH_E_ADDRESS;
	if(address > spi_flash_chip.chip_size || (address + size) > spi_flash_chip.chip_size) return EZM_SPI_FLASH_E_BOUNDARIES;

	int rt = EZM_SPI_FLASH_OK;

	size_t to_erase = size;
	size_t erased = 0;

	SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_BUSY);

	while(to_erase)
	{
		rt = ezm_spi_flash_wait_until_chip_write_enable();
		if(rt != EZM_SPI_FLASH_OK)
			return rt;

		/* If the address is the beginning and the size is the total of the chip, delete everything.*/
		if(address == 0 && size == spi_flash_chip.chip_size)
		{
			rt = ezm_spi_flash_erase_chip();
			if(rt == EZM_SPI_FLASH_OK)
				to_erase = 0;
			else
				break;
		}
		else if(to_erase % SPI_FLASH_BLOCK64_SIZE == 0)
		{
			rt = ezm_spi_flash_erase_block64(address + erased);
			if(rt == EZM_SPI_FLASH_OK)
			{
				erased += SPI_FLASH_BLOCK64_SIZE;
				to_erase -= SPI_FLASH_BLOCK64_SIZE;
			}
			else
				break;
		}
		else if(to_erase % SPI_FLASH_BLOCK32_SIZE == 0)
		{
			rt = ezm_spi_flash_erase_block32(address + erased);
			if(rt == EZM_SPI_FLASH_OK)
			{
				erased += SPI_FLASH_BLOCK32_SIZE;
				to_erase -= SPI_FLASH_BLOCK32_SIZE;
			}
			else
				break;
		}
		else
		{
			rt = ezm_spi_flash_erase_sector(address + erased);
			if(rt == EZM_SPI_FLASH_OK)
			{
				erased += SPI_FLASH_SECTOR_SIZE;
				to_erase -= SPI_FLASH_SECTOR_SIZE;
			}
			else
				break;
		}
	}
	SPI_FLASH_SET_CHIP_STATE(EZM_SPI_FLASH_STATE_READY);
	return rt;
}
