#ifndef __NFLASH_SPI2_H__
#define __NFLASH_SPI2_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include  "stdint.h"
#include  "string.h"
#include  "stdio.h"


#define EXPECTED_DEVICE         (0x13)        /* Device Identification for the M25P80 */
//#define FLASH_SIZE              (0x0100000)   /* Total device size in Bytes (1024 KB) */
#define FLASH_PAGE_COUNT        (0x01000)     /* Total device size in Pages (256 bytes per page) */
#define FLASH_SECTOR_COUNT      (0x10)        /* Total device size in Sectors (In 64 KB) */
#define FLASH_WRITE_BUFFER_SIZE 0x100         /* Write Buffer = 256 bytes */
#define FLASH_MWA               1             /* Minimum Write Access */
#define BE_TIMEOUT              (0x14)        /* Timeout in seconds suggested for Bulk Erase Operation*/
#if 0
    void spi_flash_init(void);

    void spi_flash_write_enable(void);

    void spi_flash_write_disable(void);

    uint8_t spi_flash_read_status_register(void);

    void spi_flash_read_data(uint32_t addr, uint16_t len, uint8_t *dataBuf);

    void spi_flash_page_program(uint32_t addr, uint16_t len, uint8_t *data);

    void spi_flash_erase_block_4k(uint32_t addr);

    void nor_flash_erase_block_32k(uint32_t addr);

    void spi_flash_erase_block_64k(uint32_t addr);

    void spi_flash_chip_erase(void);

    void spi_flash_power_down(void);

    void spi_flash_release_power_down(void);

    void spi_flash_read_id(uint8_t *id);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#include "mbed.h"
/**
 * The SPI_FLASH class
 */
class SPI_FLASH
{
public:

    void spi_flash_init(void);

    void write_enable(void);

    void write_disable(void);

    uint8_t read_status_register(void);

    void read_data(uint32_t addr, uint16_t len, uint8_t *dataBuf);

    void page_write(uint32_t addr, uint16_t len, uint8_t *data);

    void erase_block_4k(uint32_t addr);

    void erase_block_32k(uint32_t addr);

    void erase_block_64k(uint32_t addr);

    void chip_erase(void);

    void power_down(void);

    void release_power_down(void);

    void read_id(uint8_t *id);

		static SPI_FLASH *get_instance(void);
protected:
		    /** Vars     */
    SPI _spi;
    DigitalOut _CS;
	
private:
		SPI_FLASH(PinName mosi = FLASH_SPI_PSELMOSI, PinName miso = FLASH_SPI_PSELMISO, PinName sck = FLASH_SPI_PSELSCK, PinName ncs = FLASH_SPI_PSELSS, const char* name = "SPI_FLASH");
    ~SPI_FLASH();

		int _write(char *s, size_t size);
		int _read(char *s, size_t size);
		void _wait_for_operation_completed(void);
		void _page_program_core(uint32_t addr, uint16_t len, uint8_t *data);
		int _write_then_read(char *w_p, size_t write_size, char *r_p, size_t read_size);
		static SPI_FLASH* p_instance;

};

#endif
#endif /* __cplusplus */

#endif
