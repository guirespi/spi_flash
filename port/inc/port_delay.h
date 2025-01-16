/*
 * port_delay.h
 *
 *  Created on: Aug 13, 2024
 *      Author: guirespi
 */

#ifndef API_API_SPI_FLASH_PORT_INC_PORT_DELAY_H_
#define API_API_SPI_FLASH_PORT_INC_PORT_DELAY_H_

#include <stdbool.h>
#include <stdint.h>

/*< void * instance to represent a port delay object */
typedef void * port_delay_hdle;

/**
 * @brief Init an instance of non blocking delay.
 *
 * @param ms Milliseconds to delay.
 * @return Non blocking delay instance. NULL if error.
 */
port_delay_hdle port_delay_init(uint32_t ms);
/**
 * @brief Read a delay status.
 *
 * @param delay_hdle Previously initialized delay handle
 * @return true: Delay reached.
 * 		   false: Delay not reached.
 */
bool port_delay_read(port_delay_hdle delay_hdle);
/**
 * @brief Delay a previously initialized delay instance.
 *
 * @param delay_hdle Previously initialized delay handle pointer.
 */
void port_delay_deinit(port_delay_hdle * delay_hdle);

#endif /* API_API_SPI_FLASH_PORT_INC_PORT_DELAY_H_ */
