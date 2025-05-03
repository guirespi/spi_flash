/************************************************************************************************
Copyright (c) 2025, Guido Ramirez <guidoramirez7@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/**
 * @file test_API_spi_flash.c
 * @brief Unit tests for the LED module.
 */

/* === Headers files inclusions =============================================================== */
#include "unity.h"
#include "api_spi_flash.h"
#include "api_spi_flash_def.h"
#include "mock_port_delay.h"
#include "mock_spi_flash_arch_common.h"

#include <stdint.h>

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/**
 * @brief SPI handle variable. This is a mock handle. 
 */
static int spi_handle_mem = 0;
/**
 * @brief SPI handle. This is a mock handle. In a real application, 
 * this would be replaced with the actual SPI handle
 */
static spi_if_hdle spi_handle = &spi_handle_mem;

/* === Private function implementation ========================================================= */

/* === Public function implementation ========================================================== */

void setUp(void) {
    ;
}

static void expect_success_spi_flash_send_basic_command_receive(uint8_t command, uint8_t * response, uint16_t response_size)
{
	spi_flash_arch_select_cs_Expect();

    uint8_t expected_data[] = {command};
    spi_flash_arch_write_spi_ExpectWithArrayAndReturn(expected_data, sizeof(command), 1, 0, 0);
    // spi_flash_arch_write_spi_ExpectAndReturn(command, 1, 0, 0);
    spi_flash_arch_write_spi_IgnoreArg_timeout();
    spi_flash_arch_write_spi_IgnoreArg_data();

    // Dont care about response pointer.
    uint8_t * aux_ptr = NULL;
    spi_flash_arch_read_spi_ExpectAndReturn(aux_ptr, response_size, 0, 0);
    spi_flash_arch_read_spi_IgnoreArg_buffer();
    spi_flash_arch_read_spi_IgnoreArg_timeout();
	spi_flash_arch_read_spi_ReturnArrayThruPtr_buffer(response, response_size);

	spi_flash_arch_deselect_cs_Expect();
}

void test_spi_flash_init_success(void){
    spi_flash_cs_t cs_gpio = {0};
    cs_gpio.port = 0;
    cs_gpio.pin = 0;

    spi_flash_arch_init_spi_IgnoreAndReturn(0);
    spi_flash_arch_init_cs_IgnoreAndReturn(0);
    
    uint8_t exp_reg1 = 0, exp_reg2, exp_reg3; // We only care for reg 1.
    
    // First, expect reg 1
    expect_success_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_1, &exp_reg1, sizeof(exp_reg1));
    
    // Second, expect reg 2
    expect_success_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_2, &exp_reg2, sizeof(exp_reg2));
    
    // Third, expect reg 3
    expect_success_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_STATUS_REG_3, &exp_reg3, sizeof(exp_reg3));


    // Now set the expectation for JEDEC ID (this comes last)
    spi_flash_jedec_id expected_jedec_id = {.manufacturer_id = 0x1F, .memory_type = 0x00, .memory_capacity = 64};
    expect_success_spi_flash_send_basic_command_receive(API_SPI_FLASH_CMD_READ_JEDEC_ID, (uint8_t *)&expected_jedec_id, sizeof(expected_jedec_id));
    

    // Initialize SPI f lash
    int result = spi_flash_init(spi_handle, cs_gpio);
    TEST_ASSERT_EQUAL(SPI_FLASH_OK, result);
}

/* === End of documentation ==================================================================== */
