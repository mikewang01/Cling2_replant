/* Copyright (c) <year> <copyright holders>, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mbed.h"


#ifndef DT101_H
#define DT101_H

#define DT101_ADDRESS     0xb0
#define DT101_SENSITIVITY 21.33

#define DT101_XOUT_R      0x00
#define DT101_YOUT_R      0x01
#define DT101ZOUT_R       0x02
#define DT101_TILT_R      0x03
#define DT101_INT_R       0x06
#define DT101_MODE_R      0x07
#define DT101_SR_R        0x08


#define TUNING                                     0x01
#define DURATOUCH_I2C_ADDRESS_MAIN_0x48            0x90        // 0x48
#define DURATOUCH_I2C_ADDRESS_BOOTLOADER_0x58      0xB0        // 0x58

/** An interface for the Dt101 touch sensor from uico
 *
 * @code
 * //Uses read the touch or swipe inforamtion through the i2c protocol and blink led2 and led3
 *
 * #include "mbed.h"
 * #include "Dt101.h"
 *
 * DT101 uico_touch(p28, p27);
 *
 * DigitalOut connectionLed(LED1);
 * PwmOut Zaxis_p(LED2);
 * PwmOut Zaxis_n(LED3);
 *
 * int main() {
 *     if (uico_touch.testConnection())
 *         connectionLed = 1;
 *
 *     while(1) {
 *         Zaxis_p = uico_touch.z();
 *         Zaxis_n = -uico_touch.z();
 *     }
 *
 * }
 * @endcode
 */
class DT101
{
public:
    /**
    * The 6 different orientations and unknown
    *
    * Up & Down = X-axis
    * Right & Left = Y-axis
    * Back & Front = Z-axis
    *
    */
		
    /**
    * chip status 
    *
		* UICO_NOT_EXISTED :chip not existed in the bus
    * UICO_APP_NOT_EXISTED£ºchip existed in i2c bus but without any applicatiion in chip
    * UICO_IN_NORMAL_STATUS£ºuico chip works normally
    * UICO_IN_UNKOWN_STATUS£ºdefault chip status
    */
    enum uico_chip_status {
        UICO_NOT_EXISTED = 0,
        UICO_APP_NOT_EXISTED,
        UICO_IN_NORMAL_STATUS,
        UICO_IN_UNKOWN_STATUS
    } ;
    enum uico_power_mode {
        POWER_FAST_SCAN,
        POWER_SLOW_SCAN,
        POWER_DEEP_SLEEP,
    } ;

    struct uico_response_event {
        uint8_t type;
        uint8_t x;
        uint32_t ts;
    } ;
		
		struct version{
					uint8_t major;
					uint8_t minor;
					uint8_t revison;
		};
    /**
    * get instance of dt101 to make sure only one object get created based on clss dt101
    *
    *
    * @param return - none 
    */
		
		static DT101* get_instance( void );
    /**
    * Tests if communication is possible with the DT101
    *
    * Because the DT101 lacks a WHO_AM_I register, this function can only check
    * if there is an I2C device that responds to the DT101 address
    *
    * @param return - UICO_NOT_EXISTED : chip not existed on bus
    	*									UICO_APP_NOT_EXISTED :chip detected but app not get burned
    	*									UICO_IN_NORMAL_STATUS:chip in normal status
    	*									UICO_IN_UNKOWN_STATUS:chip in an unkown status
    */
    uico_chip_status test_connection( void );

    /**
    * Sets the active state of the DT101
    *
    * Note: This is unrelated to awake/sleep mode
    *
    * @param state - true for active, false for standby
    */
    bool set_active( bool state);
    /**
    * Reads acceleration data from the sensor
    *
    * When the parameter is a pointer to an integer array it will be the raw data.
    * When it is a pointer to a float array it will be the acceleration in g's
    *
    * @param data - pointer to array with length 3 where the acceleration data will be stored, X-Y-Z
    */
    void read_data( int *data);
    void read_data( float *data);
    /**
    * firmware_update order
    *
    * The entered samplerate will be rounded to nearest supported samplerate.
    * Supported samplerates are: 120 - 64 - 32 - 16 - 8 - 4 - 2 - 1 samples/second.
    *
    * @param samplerate - the samplerate that will be set
    */
		int firmware_update();
		
		void _isr_process(void);
			
private:
		    /**
    * Creates a new dt101 object
    *
    * @param sda - I2C data pin
    * @param scl - I2C clock pin
    * @param active - true (default) to enable the device, false to keep it standby
    */

    DT101(PinName sda = DT101_I2C_SDA, PinName scl = DT101_I2C_SCL, PinName rst = DT101_I2C_RST,PinName intr = DT101_I2C_INT, bool active = true);

    /** Write to an UICO chip slave
     *
     * Performs a complete write transaction. The bottom bit of
     * the address is forced to 0 to indicate a write.
     *
     *  @param data Pointer to the byte-array data to send
     *  @param length Number of bytes to send
     *
     *  @returns
     *       0 on success (ack),
     *   non-0 on failure (nack)
     */
    int  app_write( char *data, size_t lenth );
		int  bootloader_write( char *data, size_t lenth );
    /**
    * Read data from the device app area
     * @param length - number of bytes to read
     * @param data - pointer where the data needs to be written to
		 *   0 on success (ack),
     *   non-0 on failure (nack)
     */
     int app_read(char *data, int length);

    /**
     * Read multiple regigsters from the dt101 bootloader, more efficient than using multiple normal reads.
     * @param length - number of bytes to read
     * @param data - pointer where the data needs to be written to
		 *   true on success (ack),
     *   false on failure (nack)
     */
    int bootloader_read(char *data, int length);
		    /**
     * Read unkhown lenth data  from the dt101 , more efficient than using multiple normal reads.
     * @param rx_lenth - number of bytes to read
     * @param buffer_p - pointer where the data needs to be written to
		 *   true  on success (ack),
     *   fasle on failure (nack)
     */
		int read_unkown_lenth_from_chip(char *buffer_p, uint16_t *rx_lenth);
		int start_acknowledge(void);
		int stop_acknowledge(void);
		bool chip_init(void);
		int read_version_from_app(struct version *p);
    /**
    * _execute_bootloader operation
    */
		uint32_t _execute_bootloader(uint32_t payload_length, unsigned char *s);
    I2C _i2c;
		DigitalOut _reset;
    bool active;
		uico_chip_status chip_status;
		struct version chip_version;
		static DT101* p_instance;
		/*interrrupy handler object*/
		InterruptIn _event;
		FunctionPointer _gesture_callback;
};


#endif
