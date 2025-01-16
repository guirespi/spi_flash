/*
 * API_spi_flash.h
 *
 *  Created on: Jul 28, 2024
 *      Author: guirespi
 */

#ifndef API_API_SPI_FLASH_INC_API_SPI_FLASH_H_
#define API_API_SPI_FLASH_INC_API_SPI_FLASH_H_

#include <stdint.h>

typedef void * spi_if_hdle;

typedef enum
{
	SPI_FLASH_OK = 0, /*< SPI FLASH OK*/
	SPI_FLASH_E_NULL, /*< Null input received*/
	SPI_FLASH_E_PARAM, /*< Invalid parameter */
	SPI_FLASH_E_READY, /*< Operation is set already */
	SPI_FLASH_E_ARCH, /*< Error in arch specific functions */
	SPI_FLASH_E_IO, /*< Input/Output error */
	SPI_FLASH_E_BUSY, /*< Busy */
	SPI_FLASH_E_FAIL, /*< Operation fail */
	SPI_FLASH_E_ADDRESS, /*< Invalid address to operate */
	SPI_FLASH_E_BOUNDARIES, /*< Out of flash boundaries */
	SPI_FLASH_E_MEM, /*< Internal allocation failed */
	SPI_FLASH_E_TIMEOUT, /*< Operation timeout */
}spi_flash_err_t;

typedef enum
{
	SPI_FLASH_STATE_DISABLE = 0,
	SPI_FLASH_STATE_INIT,
	SPI_FLASH_STATE_POWER_ON,
	SPI_FLASH_STATE_READY,
	SPI_FLASH_STATE_WRITE_READY,
	SPI_FLASH_STATE_BUSY,
	SPI_FLASH_STATE_ERROR,
}spi_flash_state_t;

typedef struct
{
	uint32_t port;
	uint16_t pin;
}spi_flash_cs_t;

/**
 * @brief Initialize SPI flash.
 *
 * @param spi_if_hdle SPI handle.
 * @param cs_gpio CS gpio info.
 * @return
 * 			- SPI_FLASH_OK if no error.
 */
int spi_flash_init(spi_if_hdle spi_if_hdle, spi_flash_cs_t cs_gpio);
/**
 * @brief Read SPI flash.
 *
 * @param buffer Buffer.
 * @param address Address to read.
 * @param size Size to read.
 * @return
 * 			- SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_BOUNDARIES Out of boundaries operation.
 */
int spi_flash_read(uint8_t * buffer, uint32_t address, uint32_t size);
/**
 * @brief Write into SPI flash.
 *
 * @param buffer Data to write.
 * @param address Address to write.
 * @param size Size to write.
 * @return
 * 			- SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_BOUNDARIES Out of boundaries operation.
 */
int spi_flash_write(uint8_t * buffer, uint32_t address, uint32_t size);
/**
 * @brief Erase SPI flash in a range. Address should be sector aligned in the majority of cases.
 *
 * @param address Start address.
 * @param size Size to delete
 * @return
 * 			- SPI_FLASH_OK if no error.
 * 			- SPI_FLASH_E_BOUNDARIES Out of boundaries operation.
 * 			- SPI_FLASH_E_ADDRESS invalid address to erase.
 */
int spi_flash_erase_range(size_t address, uint32_t size);

#endif /* API_API_SPI_FLASH_INC_API_SPI_FLASH_H_ */
