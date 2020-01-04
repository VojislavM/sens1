/*
Written by Nejc Klemenčič, December 19th, 2019

This is a library for the SCD30 CO2 Sensor Module. 
The sensor uses either I2C or UART to comminucate.
This library is intented for the I2C interface.

The library with the UART interface can be found at: https://github.com/NejcKle/sens2

The SCD30 measures CO2 with an accuracy of +/- 30ppm.

This library handles the initialization of the SCD30
and outputs CO2, humidity and temperature levels.

It also implements the option to read data with an interrupt connected to the SCD30's RDY pin.

Sensor interface description can be found at: 
https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Interface_Description.pdf.
*/

#include <Wire.h>
#include "SCD30_I2C_lib.h"

SCD30 scdSensor;

void setup()  
{
    Serial.begin(9600);
    Serial.println("SCD30 Example");

    scdSensor.begin(); //this will cause readings to occur every two seconds
    Serial.print("Default measurement interval: ");
    Serial.println(scdSensor.getMeasurementInterval());

    scdSensor.setMeasurementInterval(10); //then we change it to 10
    Serial.print("Set measurement interval: ");
    Serial.println(scdSensor.getMeasurementInterval());

    scdSensor.setForcedRecalibrationValue(1500);
    Serial.print("FRC value: ");
    Serial.println(scdSensor.getForcedRecalibrationValue());

    //then we want to reset the sensor and read the values again
    scdSensor.stopMeasuring();
    scdSensor.softReset();

    delay(15000);
    
    scdSensor.begin();
    //we wait a while for the sensor to reset itself and read the values again
    Serial.print("Set measurement interval: ");
    Serial.println(scdSensor.getMeasurementInterval());
    Serial.print("FRC value: ");
    Serial.println(scdSensor.getForcedRecalibrationValue());
}

void loop()  
{
    //it will take some time for the sensor to recalibrate itself
    Serial.print("co2(ppm):");
    Serial.print(scdSensor.getCO2());

    Serial.print(" temp(C):");
    Serial.print(scdSensor.getTemperatureC(), 1);

    Serial.print(" humidity(%):");
    Serial.print(scdSensor.getHumidity(), 1);
    Serial.println();

    delay(2000); //print values every two seconds
}