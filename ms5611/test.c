#include "MS5611.h"

int main()
{
    MS5611 baro();
   
    baro.initialize();

    while (true) {
        baro.refreshPressure();
        usleep(10000); // Waiting for pressure data ready
        baro.readPressure();

        baro.refreshTemperature();		
        usleep(10000); // Waiting for temperature data ready
        baro.readTemperature();

        baro.calculatePressureAndTemperature();  

        printf("Temperature(C): %f Pressure(millibar): %fn", 
                baro.getTemperature(), baro.getPressure());
        sleep(1);
    }

    return 0;
}

