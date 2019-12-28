/*
Written by Nejc Klemenčič, December 19th, 2019

This is a library for the SCD30 CO2 Sensor Module. 
The sensor uses either I2C or UART to comminucate.
This library is intented for the I2C interface.

The library with the UART interface can be found at: <insert later>


The SCD30 measures CO2 with an accuracy of +/- 30ppm.

This library handles the initialization of the SCD30
and outputs CO2, humidity and temperature levels.

Sensor interface description can be found at: 
https://cdn.sparkfun.com/assets/d/c/0/7/2/SCD30_Interface_Description.pdf.
*/

#include <Wire.h>
#include "SCD30_I2C_lib.h"

SCD30 scdSensor;

void setup()  
{
    Serial.begin(9600);
    Serial.println("SCD30 Example");
    scdSensor.begin(); //this will cause readings to occur every two seconds
    scdSensor.setMeasurementInterval(10); //we want to change the measurement interval that to 10 seconds

    scdSensor.setAltitudeCompensation(240); //set altitude in m

    scdSensor.setAmbientPressure(1020); //set pressure in mBar
}

void loop()  
{
    Serial.print("co2(ppm):");
    Serial.print(scdSensor.getCO2());

    Serial.print(" temp(C):");
    Serial.print(scdSensor.getTemperatureC(), 1);

    Serial.print(" humidity(%):");
    Serial.print(scdSensor.getHumidity(), 1);
    Serial.println();

    delay(2000); //print values every two seconds
}
