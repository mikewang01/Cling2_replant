/***************************************************************************/
/**
 * File: font.c
 *
 * Description: Characters display function.
 *
 * Created on December 22, 2015
 *
 ******************************************************************************/

#include "main.h"
#include <stdint.h>


/**@brief Function for string encoding conversion.
 * 
 * Description: UTF-8 code(3 B) converted to unicode code (2 B).
 */
static I16U _encode_conversion_pro(I8U *utf_8)
{
	I16U unicode;
	
	unicode  = (utf_8[0] & 0x0F) << 12;
	unicode |= (utf_8[1] & 0x3F) << 6;
	unicode |= (utf_8[2] & 0x3F);
	
	return unicode;
}

/**@brief Function for reading font.
 * 
 * Description: Get one 5x7 size ascii string OLED dot matrix data(8 B).
 */
static void _font_read_one_5x7_ascii(I8U ASCII_code, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_5X7_SPACE_START;;

	addr_in += ((ASCII_code - 0x20) << 3);
	
	NOR_readData(addr_in, len, dataBuf);	
}

/**@brief Function for reading font.
 * 
 * Description: Get one 8x16 size ascii string OLED dot matrix data(16 B).
 */
static void _font_read_one_8x16_ascii(I8U ASCII_code, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_ASCII_8X16_SPACE_START;;

	addr_in += ((ASCII_code - 0x20) << 4);

	NOR_readData(addr_in, len, dataBuf);	
}

/**@brief Function for reading font.
 * 
 * Description: Get one 15x16 size Chinese characters OLED dot matrix data(32 B).
 */
static void _font_read_one_Chinese_characters(I8U *utf_8, I16U len, I8U *dataBuf)
{
	I32U addr_in = FONT_CHINESE_15X16_SPACE_START;
  I16U unicode;	
	I8U  data[16];

	unicode = _encode_conversion_pro(utf_8);

	if(unicode < 0x4E00 || unicode > 0x9FA5)  {
		
    N_SPRINTF("[FONTS] No search to the simple Chinese characters.");	
		
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

/**@brief Function for get entire display length.
 * 
 * Description: display length, must <= 128.
 *
 * height is 8: only need display 5x7 ascii sting.
 *
 * height is 16: display 8x16 ascii sting and 15x16 chinese characters.
 */
static I8U _font_get_display_len(I8U *data, I8U height)
{
	I8U  s_len,d_len;  
	I8U  a_num=0;
	I8U  c_num=0;
	I8U  ch_to_process;
	I16U s_pos = 0;

	s_len = strlen((char*)data);
	
	while(s_pos < s_len)
	{
		ch_to_process = (I8U)data[s_pos];
		
	  if((ch_to_process >= 0x20)&&(ch_to_process <= 0x7e)){
	    // ASCII code encode format.
		  a_num += 1;
	    s_pos += 1;
	  }
    else if(((data[s_pos]&0xF0) == 0xE0)&&((data[s_pos+1]&0xC0) == 0x80)&&((data[s_pos+2]&0xC0) == 0x80)){
	    // Chinese characters Utf-8 encode format.	
		  c_num += 1;
	    s_pos += 3;
	  }
		else{
			// Is not within the scope of the can display,continue to read the next.
			s_pos++;
		}
	}
	
	if(height == 16)	
		d_len = (a_num << 3) + (c_num << 4);	
	else if(height == 8)
		d_len = (a_num << 3);
	else
		return 0;

	// At present,can only display 128 bytes.	
	if(d_len >= 128)
		d_len = 128;
	
	return d_len;	
}

/**@brief Function for loading characters.
 * 
 * Description: get OLED dot matrix data of a continuous string.
 *
 * height is 8: only need display 5x7 ascii sting.
 *
 * height is 16: display 8x16 ascii sting and 15x16 chinese characters.
 *
 * b_center ：display in the middle(TRUE),display from the top(FALSE). 
 */
I8U FONT_load_characters(I8U *ptr, char *data, I8U height, BOOLEAN b_center)
{	
	I8U  font_data[32];
  I8U  s_len,d_len;  
	I8U  a_num = 0;
	I8U  c_num = 0;	
	I8U  ch_to_process;
	I16U s_pos = 0;
  I16U p_data_offset = 0;

	s_len  = strlen(data);
	
	// The length of the effective display.
  d_len = _font_get_display_len((I8U*)data,height);
	
	while(s_pos < s_len)
	{
		ch_to_process = (I8U)data[s_pos];
		
		p_data_offset = (a_num << 3) + (c_num << 4);
		
		if(p_data_offset > 128)
			break;
		
	  if((ch_to_process >= 0x20)&&(ch_to_process<=0x7e)){
	  	// ASCII code encode format.
		  if(height==16){
				
				_font_read_one_8x16_ascii(ch_to_process,16,font_data);
				
				if(b_center){
					memcpy(&ptr[((128 - d_len) >> 1) + p_data_offset],font_data,8);
					memcpy(&ptr[128 + ((128 - d_len) >> 1) + p_data_offset],&font_data[8],8);	
				}else{
					memcpy(&ptr[p_data_offset],font_data,8);
					memcpy(&ptr[128 + p_data_offset],&font_data[8],8);						
				}
		  } else {
		 
			  _font_read_one_5x7_ascii(ch_to_process,8,font_data);
				
			  if(b_center)
          memcpy(&ptr[((128 - d_len) >> 1) + 6*a_num],font_data,6);	 
			  else
          memcpy(&ptr[6*a_num],font_data,6);  
		  }
			
			a_num += 1;
		  s_pos += 1;
	  }	
    else if(((data[s_pos]&0xF0) == 0xE0)&&((data[s_pos+1]&0xC0) == 0x80)&&((data[s_pos+2]&0xC0) == 0x80)){
			// Chinese characters Utf-8 encode format.	 
		  _font_read_one_Chinese_characters((I8U*)(data+s_pos),32,font_data);

		  if(b_center){
		    memcpy(&ptr[((128 - d_len) >> 1) + p_data_offset],font_data,16);
        memcpy(&ptr[128+((128 - d_len) >> 1) + p_data_offset],&font_data[16],16);
		  }else{
		    memcpy(&ptr[p_data_offset],font_data,16);
        memcpy(&ptr[128 + p_data_offset],&font_data[16],16);
		  }
		   
		   c_num += 1;
		   s_pos += 3;
	  } else{
			// Is not within the scope of the can display,continue to read the next.
			s_pos++;
		}
	}
	
	N_SPRINTF("[FONTS] display chinese number: %d,ascii number: %d ",ch_pos,as_pos);

	if(d_len <= 128)
		return d_len;
	
	return (0xff);
}
