/*
 * stm32f4xx.c
 *
 *  Created on: Jul 28, 2024
 *      Author: guirespi
 */

#ifndef API_API_SPI_FLASH_ARCH_ARM_STM32F4XX_C_
#define API_API_SPI_FLASH_ARCH_ARM_STM32F4XX_C_

#include <ezm_spi_flash_arch_common.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_spi.h"


#define ARCH_STM32F4XX_SPI_HDLE ((SPI_HandleTypeDef *) ARCH_SPI_HANDLE_NAME)

static void * ARCH_SPI_HANDLE_NAME = NULL;
static spi_flash_arch_rx_it_hdle _spi_rx_hdle = NULL;
static uint32_t _port = 0;
static uint16_t _pin = 0;

/**
 * @brief Check if SPI handle is ready.
 *
 * @return true: read. false: not ready.
 */
static inline bool ezm_spi_flash_arch_ready(void);

static inline bool ezm_spi_flash_arch_ready(void)
{
	return (_spi_hdle == NULL?false:true);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi)
{
	if(hspi->Instance == ARCH_STM32F4XX_SPI_HDLE->Instance)
	{
		if(_spi_rx_hdle != NULL)
			_spi_rx_hdle((void *) hspi, hspi->pRxBuffPtr, hspi->RxXferSize);
	}
}

int ezm_spi_flash_arch_init_spi(void * spi_hdle, spi_flash_arch_rx_it_hdle spi_rx_it_hdle)
{
	if(ezm_spi_flash_arch_ready() == true) return EZM_SPI_FLASH_ARCH_E_READY;

	SPI_HandleTypeDef * hspi1 = (SPI_HandleTypeDef *) spi_hdle;

	/*We set the SPI in master role and mode 0. The IC we are using only admits mode 0 and 3*/
	hspi1->Instance = SPI1;
	hspi1->Init.Mode = SPI_MODE_MASTER;
	hspi1->Init.Direction = SPI_DIRECTION_2LINES;
	hspi1->Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1->Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1->Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1->Init.NSS = SPI_NSS_SOFT;
	hspi1->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1->Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1->Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1->Init.CRCPolynomial = 10;

	int rt = HAL_SPI_Init(hspi1);
	if(rt != HAL_OK)
		return rt;

	_spi_hdle = spi_hdle;
	_spi_rx_hdle = spi_rx_it_hdle;
	return EZM_SPI_FLASH_ARCH_OK;
}

int ezm_spi_flash_arch_init_cs(uint32_t port, uint16_t pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init((GPIO_TypeDef  *)port, &GPIO_InitStruct);

	/* Let's set the pin to maintain the CS line in idle (deselected) as we are initializing our
	 * IC yet */
	HAL_GPIO_WritePin((GPIO_TypeDef  *)port, pin, GPIO_PIN_SET);
	_port = port;
	_pin = pin;

	return EZM_SPI_FLASH_ARCH_OK;
}

void ezm_spi_flash_arch_select_cs(void)
{
	HAL_GPIO_WritePin((GPIO_TypeDef  *)_port, _pin, GPIO_PIN_RESET);
}

void ezm_spi_flash_arch_deselect_cs(void)
{
	HAL_GPIO_WritePin((GPIO_TypeDef  *)_port, _pin, GPIO_PIN_SET);
}

int ezm_spi_flash_arch_read_spi(uint8_t * buffer, uint16_t buffer_size, uint32_t timeout)
{
	if(ezm_spi_flash_arch_ready() == false) return EZM_SPI_FLASH_ARCH_E_READY;

	return 	HAL_SPI_Receive(ARCH_STM32F4XX_SPI_HDLE, buffer, buffer_size, timeout);
}

int ezm_spi_flash_arch_write_spi(uint8_t * data, uint16_t data_size, uint32_t timeout)
{
	if(ezm_spi_flash_arch_ready() == false) return EZM_SPI_FLASH_ARCH_E_READY;

	return 	HAL_SPI_Transmit(ARCH_STM32F4XX_SPI_HDLE, data, data_size, timeout);
}

int ezm_spi_flash_arch_read_it_spi(uint8_t * data, uint16_t data_size)
{
	if(ezm_spi_flash_arch_ready() == false) return EZM_SPI_FLASH_ARCH_E_READY;

	return HAL_SPI_Receive_IT(ARCH_STM32F4XX_SPI_HDLE, data, data_size);
}

void ezm_spi_flash_arch_block_delay(uint32_t milliseconds)
{
	HAL_Delay(milliseconds);
}

#endif /* API_API_SPI_FLASH_ARCH_ARM_STM32F4XX_C_ */
