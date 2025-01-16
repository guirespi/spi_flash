/*
 * common.h
 *
 *  Created on: Jul 28, 2024
 *      Author: guirespi
 */

#ifndef API_API_SPI_FLASH_ARCH_COMMON_SPI_FLASH_ARCH_COMMON_H_
#define API_API_SPI_FLASH_ARCH_COMMON_SPI_FLASH_ARCH_COMMON_H_

#include <stdint.h>

#define ARCH_SPI_HANDLE_NAME _spi_hdle

typedef enum
{
	SPI_FLASH_ARCH_E_READY = -1,
	SPI_FLASH_ARCH_OK = 0,
}arch_spi_flash_err_t;

typedef void (*spi_flash_arch_rx_it_hdle)(void * spi_hdle, uint8_t * data, uint16_t data_size);

/**
 * @brief Initialize chip select (CS) pin.
 *
 * @param port Port of pin.
 * @param pin Pin number.
 * @return
 * 			- SPI_FLASH_ARCH_OK if no error.
 */
int spi_flash_arch_init_cs(uint32_t port, uint16_t pin);
/**
 * @brief Select previously initialized CS.
 *
 */
void spi_flash_arch_select_cs(void);
/**
 * @brief De-select previously initialized CS.
 *
 */
void spi_flash_arch_deselect_cs(void);
/**
 * @brief Initialize SPI handle.
 *
 * @param spi_hdle arch specific SPI handle.
 * @param spi_rx_it_hdle Optional function to receive via IT through SPI.
 * @return
 */
int spi_flash_arch_init_spi(void * spi_hdle, spi_flash_arch_rx_it_hdle spi_rx_it_hdle);
/**
 * @brief Poll read SPI.
 *
 * @param buffer Buffer.
 * @param buffer_size Buffer size expected.
 * @param timeout Timeout for operation.
 * @return
 * 			- SPI_FLASH_ARCH_OK if no error.
 */
int spi_flash_arch_read_spi(uint8_t * buffer, uint16_t buffer_size, uint32_t timeout);
/**
 * @brief Write throug SPI.
 *
 * @param data Data
 * @param data_size Data size.
 * @param timeout Timeout for operation.
 * @return
 * 			- SPI_FLASH_ARCH_OK if no error.
 */
int spi_flash_arch_write_spi(uint8_t * data, uint16_t data_size, uint32_t timeout);
/**
 * @brief IT read through SPI.
 *
 * @param data Buffer
 * @param data_size Expected size.
 * @return
 * 			- SPI_FLASH_ARCH_OK if no error.
 */
int spi_flash_arch_read_it_spi(uint8_t * data, uint16_t data_size);
/**
 * @brief Arch specific block delay.
 *
 * @param milliseconds Milliseconds to delay.
 */
void spi_flash_arch_block_delay(uint32_t milliseconds);

#endif /* API_API_SPI_FLASH_ARCH_COMMON_SPI_FLASH_ARCH_COMMON_H_ */
