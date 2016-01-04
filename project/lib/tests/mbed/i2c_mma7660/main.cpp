

#include "main.h"
#include "mbed.h"
#include "../env/test_env.h"
#include "../../peripherals/MMA7660/MMA7660.h"
#include "DT101.H"
CLING_MAIN_CTX cling;
#if defined(TARGET_FF_ARDUINO)
MMA7660 MMA(I2C_SDA, I2C_SCL);
#else
DT101 MMA(p16, p15);
#endif
#include "C12832.h"
C12832 LCD;
int main() {

    MBED_HOSTTEST_TIMEOUT(15);
    MBED_HOSTTEST_SELECT(default_auto);
    MBED_HOSTTEST_DESCRIPTION(I2C MMA7660 accelerometer);
    MBED_HOSTTEST_START("MBED_A13");
	while(1);
    //if (!MMA.test_connection())
   //     MBED_HOSTTEST_RESULT(false);

    for(int i = 0; i < 5; i++) {
       // printf("x: %f, y: %f, z: %f\r\n", MMA.x(), MMA.y(), MMA.z());
        wait(0.2);
    }

    MBED_HOSTTEST_RESULT(true);
		
}
