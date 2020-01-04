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

#ifndef SCD30_I2C_lib
#define SCD30_I2C_lib

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#include <Wire.h>

#define SCD30_I2C_ADDRESS 0x61 //default SCD30 I2C address

//defines for available commands
#define SCD30_START_CONTINUOUS_MEASUREMENT 0x0010
#define SCD30_STOP_CONTINUOUS_MEASUREMENT 0x0104
#define SCD30_SET_MEASUREMENT_INTERVAL 0x4600
#define SCD30_GET_READY_STATUS 0x0202
#define SCD30_READ_MEASUREMENT 0x0300
#define SCD30_SET_AUTOMATIC_SELFCALIBRATION 0x5306
#define SCD30_SET_FORCED_RECALIBRATION 0x5204
#define SCD30_SET_TEMPERATURE_OFFSET 0x5403
#define SCD30_SET_ALTITUDE_COMPENSATION 0x5102
#define SCD30_READ_FIRMWARE_VERSION 0xD100
#define SCD30_SOFT_RESET 0xD304

class SCD30 
{
    public:
        SCD30(); //constructor
        ~SCD30(); //destructor

        boolean begin(); //initialize library instance

        boolean beginMeasuring(void); //starts the measurements, with the default interval of 2s
        boolean beginMeasuring(uint16_t ambientPressureOffset); //starts the measurements with ambient pressure copensation in mBar, 
                                                                //if argument is 0, pressure compensation is deactivated
        boolean stopMeasuring(); //stops the measurements

        boolean dataAvailable(); //checks if data is available

        void enableAutomaticSelfCalibration(); //enables automatic self calibration
        void disableAutomaticSelfCalibration(); //disables automatic self calibration
       
        void setForcedRecalibrationValue(uint16_t concentration); //sets a reference CO2 value, values range from 400 ppm to 2000 ppm
        void setMeasurementInterval(uint16_t interval); //changes the default measurement interval
        void setTemperatureOffset(float tempOffset); //sets temperature offset for onboard RH/T sensor
        void setAmbientPressure(uint16_t abmientPressure); //sets ambientPressure, values range from 700 to 1200 mBar
        void setAltitudeCompensation(uint16_t altitude); //sets altitude, values range from 0 upwards, in meters
        
        uint16_t getMeasurementInterval(); //gets measurement interval
        boolean getAutomaticSelfCalibration(); //returns 1 if ASC is enabled, 0 otherwise
        uint16_t getForcedRecalibrationValue(); //gets set FRC value
        uint16_t getTemperatureOffset(); //gets set temperature offset
        uint16_t getAltituteCompensation(); //gets set altitute compensation value

        boolean readMeasurement(); //reads 18 byte measurement

        float getHumidity(); //gets humidity in %RH
        float getTemperatureC(); //gets temperature in °C
        float getTemperatureF(); //gets temperature in F
        float getTemperatureK(); //gets temperature in K
        uint16_t getCO2(); //gets CO2 concentration in ppm

        boolean sendCommand(uint16_t command); //sends command via I2C
        boolean sendCommand(uint16_t command, uint16_t argument); //sends command via I2C, with an additional argument

        uint16_t readRegister(uint16_t registerAddress); //reads specified register

        uint8_t computeCRC8(uint8_t data[], uint8_t len); //calculates crc checksum

        uint8_t* getFirmwareVersion(); //gets the firmware version in format major.minor

        boolean softReset(); //soft resets the sensor

        void attachExternalInterrupt(uint8_t pin, void (*function)(void)); //attaches interrupt to pin

    private:
        //measured values
        float co2 = 0;
        float temperature = 0;
        float humidity = 0;
        
        uint8_t* firmwareVersion;
};

#endif