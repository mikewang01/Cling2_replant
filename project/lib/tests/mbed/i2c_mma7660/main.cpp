

#include "main.h"
#include "mbed.h"
#include "../env/test_env.h"
#include "../../peripherals/MMA7660/MMA7660.h"
#include "DT101.H"
 #include "InterruptManager.h"
  #include "nflash_spi_2.h"
CLING_MAIN_CTX cling;
#if defined(TARGET_FF_ARDUINO)
MMA7660 MMA(I2C_SDA, I2C_SCL);
#else

DT101 *touch_dev = DT101::get_instance();//MMA(p16, p15);
#endif
#include "C12832.h"
//C12832 LCD;


  Ticker flipper;

 extern "C"{
  void _ble_init(void);
	}
 void flip(void) {
     // led1 = !led1;
			printf("led1\r\n");
  }
 
  void handler(void) {
     // led2 = !led1;
		printf("handler\r\n");
  }
  void trigger() {
     printf("triggered!\n");
 }
	//  InterruptIn event(p17);
 /* DigitalOut led(LED1);
 *
 void trigger() {
     printf("triggered!\n");
 }
 *
 * int main() {
 *     event.rise(&trigger);
 *     while(1) {
 *         led = !led;
 *         wait(0.25);
 *     }
 * }
	*/

 SPI_FLASH *t = SPI_FLASH::get_instance();
  int main() {
		  NVIC_RemapVector();
			_ble_init();
//		touch_dev->test_connection();
      //led1 = led2 = 0;
      flipper.attach(&flip, 1.0);
		  //event.fall(&trigger);
      InterruptManager::get()->add_handler(handler, RTC1_IRQn);
			while(1);
  }

 
 
int main_3() {

   // MBED_HOSTTEST_TIMEOUT(15);
  //  MBED_HOSTTEST_SELECT(default_auto);
  //  MBED_HOSTTEST_DESCRIPTION(I2C MMA7660 accelerometer);
  //  MBED_HOSTTEST_START("MBED_A13");
    //if (!MMA.test_connection())
   //     MBED_HOSTTEST_RESULT(false);

    for(int i = 0; i < 5; i++) {
       // printf("x: %f, y: %f, z: %f\r\n", MMA.x(), MMA.y(), MMA.z());
        wait(0.2);
    }
   while(1);
   // MBED_HOSTTEST_RESULT(true);
		
}
