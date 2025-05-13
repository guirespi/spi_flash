/*
 * API_spi_flash.h
 *
 *  Created on: Jul 28, 2024
 *      Author: guirespi
 */

#ifndef EZM_API_SPI_FLASH_H_
#define EZM_API_SPI_FLASH_H_

#include <stdint.h>

typedef void * ezm_spi_if_hdle;

typedef enum
{
	EZM_SPI_FLASH_OK = 0, /*< SPI FLASH OK*/
	EZM_SPI_FLASH_E_NULL, /*< Null input received*/
	EZM_SPI_FLASH_E_PARAM, /*< Invalid parameter */
	EZM_SPI_FLASH_E_READY, /*< Operation is set already */
	EZM_SPI_FLASH_E_ARCH, /*< Error in arch specific functions */
	EZM_SPI_FLASH_E_IO, /*< Input/Output error */
	EZM_SPI_FLASH_E_BUSY, /*< Busy */
	EZM_SPI_FLASH_E_FAIL, /*< Operation fail */
	EZM_SPI_FLASH_E_ADDRESS, /*< Invalid address to operate */
	EZM_SPI_FLASH_E_BOUNDARIES, /*< Out of flash boundaries */
	EZM_SPI_FLASH_E_MEM, /*< Internal allocation failed */
	EZM_SPI_FLASH_E_TIMEOUT, /*< Operation timeout */
}ezm_spi_flash_err_t;

typedef enum
{
	EZM_SPI_FLASH_STATE_DISABLE = 0,
	EZM_SPI_FLASH_STATE_INIT,
	EZM_SPI_FLASH_STATE_POWER_ON,
	EZM_SPI_FLASH_STATE_READY,
	EZM_SPI_FLASH_STATE_WRITE_READY,
	EZM_SPI_FLASH_STATE_BUSY,
	EZM_SPI_FLASH_STATE_ERROR,
}ezm_spi_flash_state_t;

typedef struct
{
	uint32_t port;
	uint16_t pin;
}ezm_spi_flash_cs_t;

/**
 * @brief Initialize SPI flash.
 *
 * @param ezm_spi_if_hdle SPI handle.
 * @param cs_gpio CS gpio info.
 * @return
 * 			- SPI_FLASH_OK if no error.
 */
int ezm_spi_flash_init(ezm_spi_if_hdle ezm_spi_if_hdle, ezm_spi_flash_cs_t cs_gpio);
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
int ezm_spi_flash_read(uint8_t * buffer, uint32_t address, uint32_t size);
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
int ezm_spi_flash_write(uint8_t * buffer, uint32_t address, uint32_t size);
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
int ezm_spi_flash_erase_range(size_t address, uint32_t size);

#endif /* EZM_API_SPI_FLASH_H_ */
