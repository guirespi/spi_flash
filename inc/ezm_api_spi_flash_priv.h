#ifndef EZM_API_SPI_FLASH_PRIV_H_
#define EZM_API_SPI_FLASH_PRIV_H_

#include <stdint.h>
#include <ezm_api_spi_flash.h>

/* For W25Q64JV */ 
#define SPI_FLASH_BLOCK64_SIZE (1024*64)
#define SPI_FLASH_BLOCK32_SIZE (1024*32)
#define SPI_FLASH_SECTOR_SIZE (1024*4)
#define SPI_FLASH_PAGE_SIZE (256)

#define SPI_FLASH_HTONL(address) (((address & 0x000000ff)<<24)|((address & 0x0000ff00)<<8|((address & 0x00ff0000)>>8)|(address & 0xff000000)>>24))

typedef struct
{
	uint8_t vendor_id; /* Vendor ID */
	uint8_t chip_type; /* Memory type */
	uint32_t chip_size; /* Chip size to check boundaries */
	ezm_spi_flash_state_t chip_state; /*  Chip state */
}ezm_spi_flash_chip_t;

#ifdef TEST
extern ezm_spi_flash_chip_t spi_flash_chip;
#endif

#endif /* EZM_API_SPI_FLASH_PRIV_H_ */

