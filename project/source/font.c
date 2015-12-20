#include "main.h"
#include <stdint.h>


// UTF-8 code(3 btye) converted to  unicode code (2 btye).
static I16U _font_utf_to_unicode(I8U *UTF_8_in)
{
	I16U unicode_16;
	unicode_16  = (UTF_8_in[0] & 0x0F) << 12;
	unicode_16 |= (UTF_8_in[1] & 0x3F) << 6;
	unicode_16 |= (UTF_8_in[2] & 0x3F);
	return unicode_16;
}


// Read one 5x7 size ascii characters.
static void _font_read_one_5x7_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_5X7_SPACE_START;;

	addr_in += ((ASCIICode-0x20)<<3);
	
	NOR_readData(addr_in, len, dataBuf);	
}


// Read one 8x16 size ascii characters.
static void _font_read_one_8x16_ascii(I8U ASCIICode,I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_8X16_SPACE_START;;

	addr_in += ((ASCIICode-0x20)<<4);

	NOR_readData(addr_in, len, dataBuf);	
}


// Read one 15x16 size simple chinese characters.
static void _font_read_one_simple_Chinese_characters(I8U *utf_8,I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_SIMPLE_CHINESE_SPACE_START;
  I16U unicode;	
	I8U  data[16];

	unicode = _font_utf_to_unicode(utf_8);

	if(unicode < 0x4E00 || unicode > 0x9FA5)  {
    N_SPRINTF("[FONTS] No search to the simple Chinese characters ...");	
		memset(dataBuf, 0, 32);
		// Use succedaneous ascii characters (' " ')in here.
		_font_read_one_8x16_ascii('"',16,data);
		memcpy(dataBuf+4,data,8);
		memcpy(dataBuf+20,&data[8],8);		
    return;		
	}
	
	addr_in += ((unicode - 0x4E00) << 5);
	NOR_readData(addr_in, len, dataBuf);	
}	


#ifdef _ENABLE_FONT_TRANSFER_
static void _read_block(I32U addr, I16U len, I8U *dataBuf)
{
	I8U  Font_tx_buf[4];
				 
	Font_tx_buf[0]=0x03;
	Font_tx_buf[1]=(I8U)(addr >> 16);	
	Font_tx_buf[2]=(I8U)(addr >> 8);			
	Font_tx_buf[3]=(I8U)(addr);
	N_SPRINTF("[FONT] %x %x %x %x", g_spi_tx_buf[0], g_spi_tx_buf[1], g_spi_tx_buf[2], g_spi_tx_buf[3]);

	spi_master_tx_rx(SPI_MASTER_0, Font_tx_buf, 4, 0, 0, dataBuf, 4, len, GPIO_SPI_0_CS_FONT);
//	spi_master_op_wait_done();
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_rw_core(I32U addr_out, I32U addr_in, I32U len)
{
	I32U pos = 0;
	I32U in, out;
	I32U data[32]; // 128 bytes
	in = addr_in;
	out = addr_out;
	
	while (pos < len) {
  N_SPRINTF("[FONTS] in: %d, out: %d, len: 128", in, out);
	_read_block(out, 128, (I8U *)data);
	NOR_pageProgram(in, 128, (I8U *)data);
	pos += 128;
	out += 128;
	in += 128;
	Watchdog_Feed();
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_verification()
{
	I32U in, i;
	I32U dataO[32]; // 128 bytes
	I8U *p1;

	in = 376832;
	for (i = 0; i < 256; i++) {
	p1 = (I8U *)dataO;
	NOR_readData(in, 128, (I8U *)dataO);
	
	N_SPRINTF("[FONTS] dataO: %02x %02x %02x %02x %02x %02x %02x %02x", 
	p1[0],p1[1],p1[2],p1[3],p1[4],p1[5],p1[6],p1[7]);
	in += 128;
	Watchdog_Feed();
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
static void _block_erase_core(I32U addr, I32U pages)
{
	I16U page_to_erase;
	page_to_erase = pages;
	
	N_SPRINTF("[FONTS] erase all the pages before setup");
	//
	// Note: each page is 4 KB
	while (page_to_erase > 0) {
		if (page_to_erase >= 256) {
			N_SPRINTF("[FS] erase: %d, Whole chip", addr);
			NFLASH_drv_sector_erasure(addr, ERASE_WHOLE_CHIP);
			addr += 1048576;
			page_to_erase -= 256;
		} else if ((page_to_erase >= 16) && ((addr%65536) == 0)) {
			N_SPRINTF("[FS] erase: %d, 64K", addr);
			// we should erase 64 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_64K);
			addr += 65536;
			page_to_erase -= 16;
		} else if ((page_to_erase >= 8) && ((addr%32768) == 0)) {
			N_SPRINTF("[FS] erase: %d, 32K", addr);
			// we should erase 32 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_32K);
			addr += 32768;
			page_to_erase -= 8;
		} else {
			N_SPRINTF("[FS] erase: %d, 4K", addr);
			// we should erase 4 KB block
			NFLASH_drv_sector_erasure(addr, ERASE_4K);
			addr += 4096;
			page_to_erase -= 1;
		}
	}
}
#endif

#ifdef _ENABLE_FONT_TRANSFER_
void FONT_flash_setup()
{
	I32U source_addr, dest_addr, len, pages;
	#if 1
	// Erase whole Nor Flash before setup fonts library
	dest_addr = 376832;
	len = 671744;
	pages = len >> 12; // overall pages in 4 kBytes
	_block_erase_core(dest_addr, pages);

	// ASCII 5x7
	source_addr = 0;
	 len = 768;
	_block_rw_core(source_addr, dest_addr, len);

	// ASCII 8x16
	source_addr = 2688;
	dest_addr += len;
	len = 1536;
	_block_rw_core(source_addr, dest_addr, len);
	
	// UNICODE 15x16
	source_addr = 663840;
	dest_addr += len;
 	len = 668864+576;
	_block_rw_core(source_addr, dest_addr, len);
#endif
	// Just for verificagtion
	//_block_verification();
}
#endif
	

/*To obtain meaningful len of string */
static I8U _font_get_display_len(I8U *data,I8U height)
{
	I8U  str_len,dis_len;  
	I8U  as_num=0;
	I8U  ch_num=0;
	I8U  ch_to_process;
	I16U str_pos = 0;

	// The entire length of the string
	str_len = strlen((char*)data);
	while(str_pos < str_len)
	{
		ch_to_process = (I8U)data[str_pos];
		// ASCII code encode format.
	  if((ch_to_process >= 0x20)&&(ch_to_process <= 0x7e)){
	  
		  as_num += 1;
			// One ascii characters occupy 1 Byte.
	    str_pos += 1;
	  }
		// Chinese characters Utf-8 encode format.	 
    else if(((data[str_pos]&0xF0) == 0xE0)&&((data[str_pos+1]&0xC0) == 0x80)&&((data[str_pos+2]&0xC0) == 0x80)){
	  
		  ch_num += 1;
			// One UTF-8 chinese characters occupy 3 Bytes.
	    str_pos += 3;
	  }
		else
			// Is not within the scope of the can display,continue to read the next.
			str_pos++;
	}
	
	if(height == 16){
		
		dis_len = (as_num << 3) + (ch_num << 4);
		
		if(dis_len >= 128)
			dis_len = 128;
		
		return dis_len;
	}
	
	// Only display 5x7 ascii string.
	if(height == 8){
		
		dis_len = (as_num << 3);

		if(dis_len >= 128)
			dis_len = 128;
		
		return dis_len;		
	}
		
	return 0;
}


/*height size :16 or 8.*/
/* If b_center is equal to the 'ture' : Display in the middle.*/
I8U FONT_load_characters(I8U *ptr,char *data,I8U height, BOOLEAN b_center)
{
	I8U  font_data[32];
  I8U  str_len,dis_len;  
	I8U  as_num = 0;
	I8U  ch_num = 0;	
	I8U  ch_to_process;
	I16U str_pos = 0;
  I16U p_data_offset = 0;
	// The length of the whole string.
	str_len  = strlen(data);
	
	// The length of the effective display.
  dis_len = _font_get_display_len((I8U*)data,height);
	
	while(str_pos < str_len)
	{
		ch_to_process = (I8U)data[str_pos];
		
		p_data_offset = (as_num << 3) + (ch_num << 4);
		
		// At present,can only display 128 bytes.in case the length of the string is too long,then break.
		if(p_data_offset > 128)
			break;
		
		// ASCII code encode format.
	  if((ch_to_process >= 0x20)&&(ch_to_process<=0x7e)){
	  
		  if(height==16){
				
		    // Need display 8x16 size ascii string.
				_font_read_one_8x16_ascii(ch_to_process,16,font_data);
				
				if(b_center==TRUE){
					
					// Display in the middle.
					memcpy(&ptr[((128-dis_len) >> 1) + p_data_offset],font_data,8);
					memcpy(&ptr[128+((128-dis_len) >> 1) + p_data_offset],&font_data[8],8);	
				}
				else{
					
					memcpy(&ptr[p_data_offset],font_data,8);
					memcpy(&ptr[128 + p_data_offset],&font_data[8],8);						
				}
		  }
		  else if(height==8){
				
			  // Need display 5x7 size ascii. 
			  _font_read_one_5x7_ascii(ch_to_process,8,font_data);
				
			  if(b_center==TRUE)
					// Display in the middle.
          memcpy(&ptr[((128-dis_len) >> 1) + 6*as_num],font_data,6);	 
			  else
          memcpy(&ptr[6*as_num],font_data,6);  
		  }
			
			as_num += 1;
		  str_pos += 1;
	  }	
		// Chinese characters Utf-8 encode format.	 
    else if(((data[str_pos]&0xF0) == 0xE0)&&((data[str_pos+1]&0xC0) == 0x80)&&((data[str_pos+2]&0xC0) == 0x80)){
			
		  _font_read_one_simple_Chinese_characters((I8U*)(data+str_pos),32,font_data);

		  if(b_center == TRUE){
				
				// Display in the middle.
		    memcpy(&ptr[((128-dis_len) >> 1) + p_data_offset],font_data,16);
        memcpy(&ptr[128+((128-dis_len) >> 1) + p_data_offset],&font_data[16],16);
		  }
		  else{
		 
		    memcpy(&ptr[p_data_offset],font_data,16);
        memcpy(&ptr[128 + p_data_offset],&font_data[16],16);
		  }
		   // One UTF-8 chinese characters occupy 3 Bytes.
		   ch_num += 1;
		   str_pos += 3;
	  }
		else
			// Is not within the scope of the can display,continue to read the next.
			str_pos++;
	}
	
	N_SPRINTF("[FONTS] display chinese number: %d,ascii number: %d ",ch_pos,as_pos);
	
	// At present,can only display 128 bytes.
	if(dis_len <= 128)
		return dis_len;
	
	return (0xff);
}


