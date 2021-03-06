#include "nflash_spi_2.h"
#include "wait_api.h"
#include "stdio.h"
//
// InstructionsCode
//
enum {
    //Instruction set
    SPI_FLASH_INS_WREN        = 0x06,		// write enable
    SPI_FLASH_INS_WRDI        = 0x04,		// write disable
    SPI_FLASH_INS_RDSR        = 0x05,		// read status register
    SPI_FLASH_INS_WRSR        = 0x01,		// write status register
    SPI_FLASH_INS_READ        = 0x03,		// read data bytes
    SPI_FLASH_INS_FAST_READ   = 0x0B,		// read data bytes at higher speed
    SPI_FLASH_INS_PP          = 0x02,		// page program
    SPI_FLASH_INS_SE_64K      = 0xD8,   // block erase 64KB
    SPI_FLASH_INS_SE_4K       = 0x20,   // sector erase 4KB
    SPI_FLASH_INS_SE_32K      = 0x52,   // sector erase 32KB
    SPI_FLASH_INS_RES         = 0xAB,		// release from deep power-down
    SPI_FLASH_INS_DP          = 0xB9,		// deep power-down
    SPI_FLASH_INS_RDID        = 0x90,		// read identification
    SPI_FLASH_INS_BE          = 0xC7,		// bulk erase
    SPI_FLASH_INS_DUMMY       = 0xAA
};

/*******************************************************************************
Flash Status Register Definitions (see Datasheet)
*******************************************************************************/
enum {
    SPI_FLASH_SRWD	= 0x80,				// Status Register Write Protect
    SPI_FLASH_BP2	= 0x10,				// Block Protect Bit2
    SPI_FLASH_BP1	= 0x08,				// Block Protect Bit1
    SPI_FLASH_BP0	= 0x04,				// Block Protect Bit0
    SPI_FLASH_WEL	= 0x02,				// write enable latch
    SPI_FLASH_WIP	= 0x01				// write/program/erase in progress indicator
};


#define SPI_BUFFER_SIZE 10
static uint8_t  b_flash_pd_flag;


SPI_FLASH::SPI_FLASH(PinName mosi, PinName miso, PinName sck, PinName ncs, const char* name):
    _spi(mosi, miso, sck), _CS(ncs)
{
    _CS = 1;
    spi_flash_init();
}

SPI_FLASH::~SPI_FLASH()
{

}
SPI_FLASH* SPI_FLASH::p_instance = (SPI_FLASH*)NULL;
SPI_FLASH *SPI_FLASH::get_instance()
{
    if(SPI_FLASH::p_instance == NULL) {
        SPI_FLASH::p_instance = new SPI_FLASH();
    }
    return p_instance;
}
/*----------------------------------------------------------------------------------
*  Function:	void NOR_init(void)
*
*  Description:
*	Init the SPI interface
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::spi_flash_init(void)
{
#ifndef _CLING_PC_SIMULATION_
    uint8_t buf[SPI_BUFFER_SIZE];

    // Power up the chip
    release_power_down();
    // b_flash_pd_flag = true;

    // Read ID
    read_id((uint8_t *)buf);

    //cling.whoami.nor[0] = buf[0];
    //cling.whoami.nor[1] = buf[1];
#endif
}

int SPI_FLASH::_write(char *s, size_t size)
{
    _CS = 0;
    for(; size > 0; size--) {
        _spi.write(*s);
				s++;
    }
    _CS = 1;
    return true;
}
int SPI_FLASH::_read(char *s, size_t size)
{
    _CS = 0;
    for(int i = 0; i < size; i++) {
        s[i] = _spi.write(SPI_FLASH_INS_DUMMY);
    }
    _CS = 1;
    return true;

}
int SPI_FLASH::_write_then_read(char *w_p, size_t write_size, char *r_p, size_t read_size)
{
    _CS = 0;
    for(int i = 0; i < write_size ; i++) {
        /*there still more write data need to be written to the flash*/
        _spi.write(w_p[i]);
    }
    for(int i = 0; i < read_size ; i++) {

        r_p[i] = _spi.write(SPI_FLASH_INS_DUMMY);
    }
    _CS = 1;
    return true;
}

void SPI_FLASH::_wait_for_operation_completed()
{
#ifndef _CLING_PC_SIMULATION_
    uint32_t count = 0;
    uint8_t ret;
    // Maximum delay 2 seconds
    for (count = 0; count < 2000; count ++) {
        ret = read_status_register();
				printf("status register = 0x%02x\r\n", ret);
        if (ret & SPI_FLASH_WIP) {
            wait_ms(1);
        } else {
            break;
        }
    }
		printf("one process done\r\n");
#endif
}

/*----------------------------------------------------------------------------------
*  Function:	void write_enable(void)
*
*  Description:
*	send the write enable command (0x06), all write or erase operation must send
*  the command firstly
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::write_enable(void)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i++] = SPI_FLASH_INS_WREN;
    //N_SPRINTF("[spi] 1624");
    SPI_FLASH::_write((char*)spi_tx_buf, i);
    //read_write((char*)spi_tx_buf,1 ,(char*)spi_rx_buf,0);
    //_wait_for_operation_completed();
}

void SPI_FLASH::write_disable(void)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i++] = SPI_FLASH_INS_WRDI;
#ifndef _CLING_PC_SIMULATION_
    //	N_SPRINTF("[spi] 123");
    SPI_FLASH::_write((char*)spi_tx_buf, i);
    //read_write((char*)spi_tx_buf,1 ,(char*)spi_rx_buf,0);
    // SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 1, spi_rx_buf,0, GPIO_SPI_0_CS_NFLASH);
#endif
}
/*----------------------------------------------------------------------------------
*  Function:	uint8_t spi_flash_read_status_register(void)
*
*  Description:
*	read status register (0x04), return the status register value
*
*----------------------------------------------------------------------------------*/
uint8_t SPI_FLASH::read_status_register(void)
{
    uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    //uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
    spi_tx_buf[i++] = SPI_FLASH_INS_RDSR;
#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write_then_read((char*)spi_tx_buf, i, (char*)spi_rx_buf,  2);
    //SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 2, spi_rx_buf,  2,GPIO_SPI_0_CS_NFLASH);
#endif
    return spi_rx_buf[0];
}
/*----------------------------------------------------------------------------------
*  Function:	void NOR_readData(uint32_t addr, uint16_t len, uint8_t *dataBuf)
*
*  Description:
*	read the len's data to dataBuf
*  addr: the 24 bit address
*  len: read length
*  dataBuf: the out data buf
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::read_data(uint32_t addr, uint16_t len, uint8_t *dataBuf)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i++] = SPI_FLASH_INS_READ;
    spi_tx_buf[i++] = (uint8_t)(addr >> 16);
    spi_tx_buf[i++] = (uint8_t)(addr >> 8);
    spi_tx_buf[i++] = (uint8_t)(addr);
		printf("spi arryy read = 0x%02x, 0x%02x\r\n", dataBuf[0], dataBuf[1]);
#define PAGE_SIZE 256
#define ADDRESS_LENTH 3
//	N_SPRINTF("[NFLASH] %x %x %x %x", spi_tx_buf[0], spi_tx_buf[1], spi_tx_buf[2], spi_tx_buf[3]);
    uint8_t rx_data[PAGE_SIZE + ADDRESS_LENTH + 1];

#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write_then_read((char*)spi_tx_buf, i, (char*)dataBuf,  len);
		printf("spi arryy read = 0x%02x, 0x%02x\r\n", dataBuf[0], dataBuf[1]);
    //SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 2, spi_rx_buf,  2,GPIO_SPI_0_CS_NFLASH);
		//SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 4, (uint8_t *)rx_data,len +4 , GPIO_SPI_0_CS_NFLASH);
   //memcpy(dataBuf, rx_data, len);
#endif


}
/*----------------------------------------------------------------------------------
*  Function:	void spi_flash_page_program(uint32_t addr, uint16_t len, uint8_t *data)
*
*  Description:
*	read the len's data to dataBuf
*  addr: the 24 bit address
*  len: write length, must <=256
*  data: the write data buf
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::_page_program_core(uint32_t addr, uint16_t len, uint8_t *data)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    write_enable();

    spi_tx_buf[i++] = SPI_FLASH_INS_PP;
    spi_tx_buf[i++] = (uint8_t)(addr >> 16);
    spi_tx_buf[i++] = (uint8_t)(addr >> 8);
    spi_tx_buf[i++] = (uint8_t)(addr);
    uint8_t tx_data[PAGE_SIZE + ADDRESS_LENTH + 1];
    memcpy(tx_data, spi_tx_buf, i);
    memcpy(&tx_data[i], data, len);

#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)tx_data, len + i);
    //	SPI_master_tx_rx(SPI_MASTER_0, (uint8_t *)tx_data, len + 4, spi_rx_buf,0, GPIO_SPI_0_CS_NFLASH);
#endif
    _wait_for_operation_completed();
}



#ifdef SPI_FLASH_PAGE_SIZE
#undef SPI_FLASH_PAGE_SIZE
#endif
#define SPI_FLASH_PAGE_SIZE 0xff
void SPI_FLASH::page_write(uint32_t addr, uint16_t len, uint8_t *data)
{
    uint32_t start_addr = addr;
    uint32_t end_addr = addr + len - 1;
    uint32_t length_1, length_2;
    //
    // Note, page size is 256 bytes, and it can only be programmed within the page.
    //
    // if we cross the page boundary, we have to seperate it into two parts.
    //
    start_addr &= ~((uint32_t)SPI_FLASH_PAGE_SIZE);
    end_addr   &= ~((uint32_t)SPI_FLASH_PAGE_SIZE);
    erase_block_4k(start_addr);
		printf("data = 0x%02x, 0x%02x", data[0], data[1]);
    if (start_addr == end_addr) {
        _page_program_core(addr, len, data);
    } else {
        // First page
        length_1 = SPI_FLASH_PAGE_SIZE - (addr - start_addr);
        _page_program_core(addr, length_1, data);
        // Second page
        addr += length_1;
        length_2 = len - length_1;
        _page_program_core(addr, length_2, data + length_1);
    }

}
/*----------------------------------------------------------------------------------
*  Function:	spi_flash_erase_block_4k(uint32_t addr)
*
*  Description:
*	erase the sector(4K Bytes)
*  addr: the 24 bit address
*
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::erase_block_4k(uint32_t addr)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    write_enable();

    spi_tx_buf[i++] = SPI_FLASH_INS_SE_4K;
    spi_tx_buf[i++] = (uint8_t)(addr >> 16);
    spi_tx_buf[i++] = (uint8_t)(addr >> 8);
    spi_tx_buf[i++] = (uint8_t)(addr);
		printf("start to erase page 0x%02x\r\n", addr);
    // N_SPRINTF("[NFLASH] %x %x %x %x", spi_tx_buf[0], spi_tx_buf[1], spi_tx_buf[2], spi_tx_buf[3]);
#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)spi_tx_buf, i);
    // SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 4, spi_rx_buf,  0, GPIO_SPI_0_CS_NFLASH);
#endif
    //wait for the operation finished
    _wait_for_operation_completed();

}

/*----------------------------------------------------------------------------------
*  Function:	NOR_erase_block_32k(uint32_t addr)
*
*  Description:
*	erase the 32K block
*  addr: the 24 bit address
*
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::erase_block_32k(uint32_t addr)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    write_enable();

    spi_tx_buf[i++] = (uint8_t)SPI_FLASH_INS_SE_32K;
    spi_tx_buf[i++] = (uint8_t)(addr >> 16);
    spi_tx_buf[i++] = (uint8_t)(addr >> 8);
    spi_tx_buf[i++] = (uint8_t)(addr);
    // N_SPRINTF("[NFLASH] %x %x %x %x", spi_tx_buf[0], spi_tx_buf[1], spi_tx_buf[2], spi_tx_buf[3]);
#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)spi_tx_buf, i);
//    SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 4, spi_rx_buf,  0, GPIO_SPI_0_CS_NFLASH);
#endif
    //wait for the operation finished
    _wait_for_operation_completed();


}
/*----------------------------------------------------------------------------------
*  Function:	spi_flash_erase_block_64k(uint32_t addr)
*
*  Description:
*	erase the 64K block
*  addr: the 24 bit address
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::erase_block_64k(uint32_t addr)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    write_enable();
    spi_tx_buf[i++] = SPI_FLASH_INS_SE_64K;
    spi_tx_buf[i++] = (uint8_t)(addr >> 16);
    spi_tx_buf[i++] = (uint8_t)(addr >> 8);
    spi_tx_buf[i++] = (uint8_t)(addr);
    //N_SPRINTF("[NFLASH] %x %x %x %x", spi_tx_buf[0], spi_tx_buf[1], spi_tx_buf[2], spi_tx_buf[3]);
#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)spi_tx_buf, i);
//    SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 4, spi_rx_buf,  0, GPIO_SPI_0_CS_NFLASH);
#endif
    //N_SPRINTF("[NFLASH] waiting ...");
    //wait for the operation finished
    _wait_for_operation_completed();
    // N_SPRINTF("[NFLASH] completed!");


}
/*----------------------------------------------------------------------------------
*  Function:	spi_flash_chip_erase
*
*  Description:
*	erase the 64K block
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::chip_erase()
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    write_enable();

    spi_tx_buf[i++] = SPI_FLASH_INS_BE;

#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)spi_tx_buf, i);
//    SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 1,  spi_rx_buf,  0, GPIO_SPI_0_CS_NFLASH);
#endif
    //wait for the operation finished
    _wait_for_operation_completed();


}
/*----------------------------------------------------------------------------------
*  Function:	void spi_flash_power_down()
*
*  Description:
*  power down the nor flash to save the power consumption
*
*------------------------- ---------------------------------------------------------*/
void SPI_FLASH::power_down()
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i++] = SPI_FLASH_INS_DP;

#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write((char*)spi_tx_buf, i);
//    SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 1, spi_rx_buf, 0, GPIO_SPI_0_CS_NFLASH);
#endif
}

/*----------------------------------------------------------------------------------
*  Function:	void spi_flash_release_power_down()
*
*  Description:
*  exit the power down mode for normal operation
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::release_power_down()
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i] = SPI_FLASH_INS_RES;
    _CS = 0;
    wait_us(1);
    _CS = 1;
#ifndef _CLING_PC_SIMULATION_
    //SPI_FLASH::_write((char*)spi_tx_buf, i);
    //SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 1, spi_rx_buf,  0, GPIO_SPI_0_CS_NFLASH);
#endif
}
/*----------------------------------------------------------------------------------
*  Function:	void spi_flash_read_id(uint8_t *id)
*
*  Description:
*  read the manufacture ID and device ID
*	id: the array of uint8_t, the length >2
*
*----------------------------------------------------------------------------------*/
void SPI_FLASH::read_id(uint8_t *id)
{
    uint8_t spi_tx_buf[SPI_BUFFER_SIZE];
    uint8_t spi_rx_buf[SPI_BUFFER_SIZE];
    uint8_t i = 0;
    spi_tx_buf[i++] = SPI_FLASH_INS_RDID;
    spi_tx_buf[i++] = (uint8_t)SPI_FLASH_INS_DUMMY;
    spi_tx_buf[i++] = (uint8_t)SPI_FLASH_INS_DUMMY;
    /*move the manuactacture id first*/
    spi_tx_buf[i++] = (uint8_t)0;
#ifndef _CLING_PC_SIMULATION_
    SPI_FLASH::_write_then_read((char*)spi_tx_buf, i, (char*)spi_rx_buf, 2);
//    SPI_master_tx_rx(SPI_MASTER_0, spi_tx_buf, 1, spi_rx_buf,  6, GPIO_SPI_0_CS_NFLASH);
#endif
    /*manuacture id*/
    id[0] = (spi_rx_buf[0]);
    /*device id*/
    id[1] = spi_rx_buf[1];
    printf("[SPI_FLASH] READ ID = %02x:%02x\r\n", id[0], id[1]);
}

