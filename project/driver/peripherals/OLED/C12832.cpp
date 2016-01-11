/* mbed library for the mbed Lab Board  128*32 pixel LCD
 * use C12832 controller
 * Copyright (c) 2012 Peter Drescher - DC2PD
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// 13.10.12    initial design
// 25.10.12    add autorefresh of screen
// 25.10.12    add standart font
// 20.12.12    add bitmap graphics

// optional defines :
// #define debug_lcd  1

#include "C12832.h"
#include "mbed.h"
#include "stdio.h"
#include "Small_7.h"

#define BPP    			16       // Bits per pixel
#define OLED_WIDTH  96
#define OLED_HEIGHT 64

C12832::C12832(PinName mosi, PinName miso, PinName sck, PinName reset, PinName a0, PinName ncs, PinName pwr, const char* name)
    : GraphicsDisplay(name), _spi(mosi, miso, sck), _reset(reset), _A0(a0), _CS(ncs), _POWER(pwr)
{
    orientation = 1;
    draw_mode = NORMAL;
    char_x = 0;
    lcd_reset();
}


int C12832::width()
{
    if (orientation == 0 || orientation == 2) return 32;
    else return OLED_WIDTH;
}

int C12832::height()
{
    if (orientation == 0 || orientation == 2) return 128;
    else return OLED_HEIGHT;
}


void C12832::invert(unsigned int o)
{
    if(o == 0) wr_cmd(0xA6);
    else wr_cmd(0xA7);
}


void C12832::set_contrast(unsigned int o)
{
    contrast = o;
    wr_cmd(0x81);      //  set volume
    wr_cmd(o & 0x3F);
}

unsigned int C12832::get_contrast(void)
{
    return(contrast);
}


// write command to lcd controller

void C12832::wr_cmd(unsigned char cmd)
{
    _A0 = 0;
    _CS = 0;
		 wait_us(50);
    _spi.write(cmd);
    _CS = 1;
}

// write data to lcd controller

void C12832::wr_dat(unsigned char dat)
{
    _A0 = 1;
    _CS = 0;
	 wait_us(50);
    _spi.write(dat);
    _CS = 1;
}

// write data to lcd controller

void C12832::wr_dat(unsigned char *dat, uint16_t size)
{
    _A0 = 1;
    _CS = 0;
		for(; size>0; size--,dat++){
			_spi.write(*dat);
		}
    _CS = 1;
}

void C12832::lcd_set_contrast(oled_color_t type, uint8_t step)
{
    wr_cmd( 0x83 - type);
    wr_cmd(step);
}

void C12832::lcd_reset()
{

    _spi.format(8, 0);                // 8 bit spi mode 3
    _spi.frequency(20000000);          // 19,2 Mhz SPI clock
    _POWER = 1;
    _A0 = 0;
    _CS = 1;
    _reset = 0;                        // display reset
    wait_ms(5);
    _reset = 1;                       // end reset
    wait_ms(5);
    /* Start Initial Sequence ----------------------------------------------------*/
    wr_cmd(0xfd); // Command lock
    wr_cmd(0x12);
    wr_cmd(0xae); // Display off
    wr_cmd(0xa4); // Normal Display mode

    wr_cmd(0x15); // Set column address
    wr_cmd(0x00); // Column address start 00
    wr_cmd(0x5f); // Column address end 95

    wr_cmd(0x75); // Set row address
    wr_cmd(0x00); // Row address start 00
    wr_cmd(0x3f); // Row address end 63

    wr_cmd(0x87); // Master current control
    wr_cmd(0x09); // 9/16(160uA)
    // Set Contrast for Color Red
    lcd_set_contrast(OLED_COLOR_RED, 0xc0);
    // Set Contrast for Color Green
    lcd_set_contrast(OLED_COLOR_GREEN, 0x65);
    // Set Contrast for Color Blue
    lcd_set_contrast(OLED_COLOR_BLUE, 0x95);
    wr_cmd(0x8a);
    wr_cmd(0x61);
    wr_cmd(0x8b);
    wr_cmd(0x62);
    wr_cmd(0x8c);
    wr_cmd(0x63);

    wr_cmd(0xa0); // Set re-map & data format
    wr_cmd(0x72); // Horizontal address increment
    wr_cmd(0xa1); // Set display start line
    wr_cmd(0x00); // Start 00 line
    wr_cmd(0xa2); // Set display offset
    wr_cmd(0x00);
    wr_cmd(0xa8); // Set multiplex ratio
    wr_cmd(0x3f); // 64MUX
    wr_cmd(0xad);
    wr_cmd(0x8f);
    wr_cmd(0xb0); // set power save
    wr_cmd(0x1a); //
    wr_cmd(0xb1);
    wr_cmd(0xf1); // Phase 2 period Phase 1 period
    wr_cmd(0xb3); // Set Display Clock Divide Ratio/ Oscillator Frequency
    wr_cmd(0xd0); // 0.97MHZ 0xd0
    wr_cmd(0xbb); // set pre-charge
    wr_cmd(0x3e); //
    wr_cmd(0xbe); // set Vcomh
    wr_cmd(0x30); // 0.83Vref
    wr_cmd(0xB9);
    wr_cmd(0xB8); // Set Gray Scale Table


    for(int pwm = 1; pwm <= 63; pwm += 2) {
        wr_cmd(pwm);//pw1
    }

    wr_cmd(0xad);  // Select external VCC supply at Display ON
    wr_cmd(0x8e);  // Select External VP voltage supply
    // clear and update LCD
    memset(buffer, 0x00, 512); // clear display buffer
    copy_to_lcd();
    auto_up = 1;              // switch on auto update
    // dont do this by default. Make the user call
    //claim(stdout);           // redirekt printf to lcd
    locate(0, 0);
    set_font((unsigned char*)Small_7);  // standart font
}

// set one pixel in buffer

void C12832::pixel(int x, int y, int color)
{
    // first check parameter
    if(x > 128 || y > 32 || x < 0 || y < 0) return;

    if(draw_mode == NORMAL) {
        if(color == 0)
            buffer[x + ((y / 8) * 128)] &= ~(1 << (y % 8)); // erase pixel
        else
            buffer[x + ((y / 8) * 128)] |= (1 << (y % 8)); // set pixel
    } else { // XOR mode
        if(color == 1)
            buffer[x + ((y / 8) * 128)] ^= (1 << (y % 8)); // xor pixel
    }
}


#if 1
void C12832::pour_screen(uint16_t color)
{
		//wr_cmd(0xAE);
    uint16_t num_1, num_2;
    for(num_1 = 0; num_1 < OLED_WIDTH; num_1++) {
        for(num_2 = 0; num_2 < OLED_HEIGHT; num_2++) {
            C12832::wr_dat((uint8_t*)&color, sizeof(uint16_t));           
        }
    }
		// wr_cmd(0xAF);  //display on
}
#endif
// update lcd

void C12832::copy_to_lcd(void)
{

    _A0 = 1;
    uint8_t num_1, num_2;
    for(num_1 = 0; num_1 < 64; num_1++) {
        for(num_2 = 0; num_2 < 96; num_2++) {

						uint8_t i[] = {0x07, 0xe0};
            wr_dat(i,2);
        }

    }
    wr_cmd(0xAF);  //display on
#if 0
    for(i = 0; i < 128; i++) {
        wr_dat(buffer[i]);
    }
    // page 1
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB1);      // set page address  1
    _A0 = 1;
    for(i = 128; i < 256; i++) {
        wr_dat(buffer[i]);
    }

    //page 2
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB2);      // set page address  2
    _A0 = 1;
    for(i = 256; i < 384; i++) {
        wr_dat(buffer[i]);
    }

    //page 3
    wr_cmd(0x00);      // set column low nibble 0
    wr_cmd(0x10);      // set column hi  nibble 0
    wr_cmd(0xB3);      // set page address  3
    _A0 = 1;

    _CS = 0;

    for(i = 384; i < 512; i++) {
        wr_dat(buffer[i]);
    }
#endif
}

void C12832::cls(void)
{
    memset(buffer, 0x00, 512); // clear display buffer
    copy_to_lcd();
}


void C12832::line(int x0, int y0, int x1, int y1, int color)
{
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1 - x0;
    dy = y1 - y0;

    //  if (dx == 0) {        /* vertical line */
    //      if (y1 > y0) vline(x0,y0,y1,color);
    //      else vline(x0,y1,y0,color);
    //      return;
    //  }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    //  if (dy == 0) {        /* horizontal line */
    //      if (x1 > x0) hline(x0,x1,y0,color);
    //      else  hline(x1,x0,y0,color);
    //      return;
    //  }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym * dx;
    dy = dy_sym * dy;

    dx_x2 = dx * 2;
    dy_x2 = dy * 2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            pixel(x0, y0, color);
            x0 += dx_sym;
            if (di < 0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        pixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            pixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        pixel(x0, y0, color);
    }
    if(auto_up) copy_to_lcd();
}

void C12832::rect(int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) line(x0, y0, x1, y0, color);
    else  line(x1, y0, x0, y0, color);

    if (y1 > y0) line(x0, y0, x0, y1, color);
    else line(x0, y1, x0, y0, color);

    if (x1 > x0) line(x0, y1, x1, y1, color);
    else  line(x1, y1, x0, y1, color);

    if (y1 > y0) line(x1, y0, x1, y1, color);
    else line(x1, y1, x1, y0, color);

    if(auto_up) copy_to_lcd();
}

void C12832::fillrect(int x0, int y0, int x1, int y1, int color)
{
    int l, c, i;
    if(x0 > x1) {
        i = x0;
        x0 = x1;
        x1 = i;
    }

    if(y0 > y1) {
        i = y0;
        y0 = y1;
        y1 = i;
    }

    for(l = x0; l <= x1; l ++) {
        for(c = y0; c <= y1; c++) {
            pixel(l, c, color);
        }
    }
    if(auto_up) copy_to_lcd();
}



void C12832::circle(int x0, int y0, int r, int color)
{

    int draw_x0, draw_y0;
    int draw_x1, draw_y1;
    int draw_x2, draw_y2;
    int draw_x3, draw_y3;
    int draw_x4, draw_y4;
    int draw_x5, draw_y5;
    int draw_x6, draw_y6;
    int draw_x7, draw_y7;
    int xx, yy;
    int di;
    //WindowMax();
    if (r == 0) {       /* no radius */
        return;
    }

    draw_x0 = draw_x1 = x0;
    draw_y0 = draw_y1 = y0 + r;
    if (draw_y0 < height()) {
        pixel(draw_x0, draw_y0, color);     /* 90 degree */
    }

    draw_x2 = draw_x3 = x0;
    draw_y2 = draw_y3 = y0 - r;
    if (draw_y2 >= 0) {
        pixel(draw_x2, draw_y2, color);    /* 270 degree */
    }

    draw_x4 = draw_x6 = x0 + r;
    draw_y4 = draw_y6 = y0;
    if (draw_x4 < width()) {
        pixel(draw_x4, draw_y4, color);     /* 0 degree */
    }

    draw_x5 = draw_x7 = x0 - r;
    draw_y5 = draw_y7 = y0;
    if (draw_x5 >= 0) {
        pixel(draw_x5, draw_y5, color);     /* 180 degree */
    }

    if (r == 1) {
        return;
    }

    di = 3 - 2 * r;
    xx = 0;
    yy = r;
    while (xx < yy) {

        if (di < 0) {
            di += 4 * xx + 6;
        } else {
            di += 4 * (xx - yy) + 10;
            yy--;
            draw_y0--;
            draw_y1--;
            draw_y2++;
            draw_y3++;
            draw_x4--;
            draw_x5++;
            draw_x6--;
            draw_x7++;
        }
        xx++;
        draw_x0++;
        draw_x1--;
        draw_x2++;
        draw_x3--;
        draw_y4++;
        draw_y5++;
        draw_y6--;
        draw_y7--;

        if ( (draw_x0 <= width()) && (draw_y0 >= 0) ) {
            pixel(draw_x0, draw_y0, color);
        }

        if ( (draw_x1 >= 0) && (draw_y1 >= 0) ) {
            pixel(draw_x1, draw_y1, color);
        }

        if ( (draw_x2 <= width()) && (draw_y2 <= height()) ) {
            pixel(draw_x2, draw_y2, color);
        }

        if ( (draw_x3 >= 0 ) && (draw_y3 <= height()) ) {
            pixel(draw_x3, draw_y3, color);
        }

        if ( (draw_x4 <= width()) && (draw_y4 >= 0) ) {
            pixel(draw_x4, draw_y4, color);
        }

        if ( (draw_x5 >= 0) && (draw_y5 >= 0) ) {
            pixel(draw_x5, draw_y5, color);
        }
        if ( (draw_x6 <= width()) && (draw_y6 <= height()) ) {
            pixel(draw_x6, draw_y6, color);
        }
        if ( (draw_x7 >= 0) && (draw_y7 <= height()) ) {
            pixel(draw_x7, draw_y7, color);
        }
    }
    if(auto_up) copy_to_lcd();
}

void C12832::fillcircle(int x, int y, int r, int color)
{
    int i, up;
    up = auto_up;
    auto_up = 0;   // off
    for (i = 0; i <= r; i++)
        circle(x, y, i, color);
    auto_up = up;
    if(auto_up) copy_to_lcd();
}

void C12832::setmode(int mode)
{
    draw_mode = mode;
}

void C12832::locate(int x, int y)
{
    char_x = x;
    char_y = y;
}



int C12832::columns()
{
    return width() / font[1];
}



int C12832::rows()
{
    return height() / font[2];
}



int C12832::_putc(int value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + font[2];
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    } else {
        character(char_x, char_y, value);
        if(auto_up) copy_to_lcd();
    }
    return value;
}

void C12832::character(int x, int y, int c)
{
    unsigned int hor, vert, offset, bpl, j, i, b;
    unsigned char* zeichen;
    unsigned char z, w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (char_x + hor > width()) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= height() - font[2]) {
            char_y = 0;
        }
    }

    zeichen = &font[((c - 32) * offset) + 4]; // start of char bitmap
    w = zeichen[0];                          // width of actual char
    // construct the char into the buffer
    for (j = 0; j < vert; j++) { //  vert line
        for (i = 0; i < hor; i++) { //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3) + 1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
                pixel(x + i, y + j, 0);
            } else {
                pixel(x + i, y + j, 1);
            }

        }
    }

    char_x += w;
}


void C12832::set_font(unsigned char* f)
{
    font = f;
}

void C12832::set_auto_up(unsigned int up)
{
    if(up ) auto_up = 1;
    else auto_up = 0;
}

unsigned int C12832::get_auto_up(void)
{
    return (auto_up);
}

void C12832::print_bm(Bitmap bm, int x, int y)
{
    int h, v, b;
    char d;

    for(v = 0; v < bm.ySize; v++) { // lines
        for(h = 0; h < bm.xSize; h++) { // pixel
            if(h + x > 127) break;
            if(v + y > 31) break;
            d = bm.data[bm.Byte_in_Line * v + ((h & 0xF8) >> 3)];
            b = 0x80 >> (h & 0x07);
            if((d & b) == 0) {
                pixel(x + h, y + v, 0);
            } else {
                pixel(x + h, y + v, 1);
            }
        }
    }

}


