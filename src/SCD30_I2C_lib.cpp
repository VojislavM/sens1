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

#include "SCD30_I2C_lib.h"

SCD30::SCD30()
{
    //constructor
}

SCD30::~SCD30()
{
    //destructor
}

boolean SCD30::begin()
{
    Wire.begin(); //initiate the Wire library and join the I2C bus as a master

    //check for device to respond correctly
    if(beginMeasuring() == true) //start continuous measurements
    {
        setMeasurementInterval(2); //2 seconds between measurements - default value
        return true;
    }

    return false; //something went wrong
}

//begins continuous measurements, status is saved in non-volatile memory
//the device continues measuring after repowering without sending the measurement command 
//returns true if successful
//begin measuring without pressureOffset
boolean SCD30::beginMeasuring(void)
{
  return(beginMeasuring(0)); //triggers continuous measurement, argument = 0 deactivates pressure compensation
}

//overload 
//see 1.4.1 in document
boolean SCD30::beginMeasuring(uint16_t ambientPressureOffset)
{
  return(sendCommand(SCD30_START_CONTINUOUS_MEASUREMENT, ambientPressureOffset));
}

//stops continuous measuring, measuring can be resumed with beginMeasuring
//see 1.4.2 in document
boolean SCD30::stopMeasuring()
{
    return(sendCommand(SCD30_STOP_CONTINUOUS_MEASUREMENT));
}

//returns true when data from the sensor is available
//see 1.4.4 in document
boolean SCD30::dataAvailable()
{
    uint16_t response = readRegister(SCD30_GET_READY_STATUS);

    if (response == 1) return true;
    else return false;
}

//enables automatic self calibration 
//see 1.4.5 in document
void SCD30::enableAutomaticSelfCalibration()
{
    sendCommand(SCD30_SET_AUTOMATIC_SELFCALIBRATION, 1);
}

//disables automatic self calibration 
//see 1.4.5 in document
void SCD30::disableAutomaticSelfCalibration()
{
    sendCommand(SCD30_SET_AUTOMATIC_SELFCALIBRATION, 0);
}

//sets reference CO2 concentration in ppm
//valid values in range from 400 to 2000 ppm
//see 1.4.5 in document
void SCD30::setForcedRecalibrationValue(uint16_t concentration)
{
    if (concentration < 400 || concentration > 2000) return;
    sendCommand(SCD30_SET_FORCED_RECALIBRATION, concentration);
}

//sets measurement interval
//valid values in range from 2 seconds to 1800 seconds (30 minutes)
//see 1.4.3 in document
void SCD30::setMeasurementInterval(uint16_t interval)
{
    sendCommand(SCD30_SET_MEASUREMENT_INTERVAL, interval);
}

//sets temperature offset
//see 1.4.6 in document
void SCD30::setTemperatureOffset(float tempOffset)
{
    uint16_t tickOffest = tempOffset * 100;
    sendCommand(SCD30_SET_TEMPERATURE_OFFSET, tickOffest);
}

//sets ambient pressure after initialization
//ambient pressure can also be set with beginMeasuring(uint16_t ambientPressureOffset)
//valid values in range from 700 to 1200 mBar
//see 1.4.1 in document
void SCD30::setAmbientPressure(uint16_t ambientPressure)
{
    if (ambientPressure < 700 || ambientPressure > 1200) 
    {
        ambientPressure = 0;
    }

    sendCommand(SCD30_START_CONTINUOUS_MEASUREMENT, ambientPressure);
}

//sets altitude compensation, valid values range from 0 upwards, in m
//see 1.4.7 in document
void SCD30::setAltitudeCompensation(uint16_t altitude)
{
    sendCommand(SCD30_SET_ALTITUDE_COMPENSATION, altitude);
}

//gets set measurement interval 
//see 1.4.3 in document
uint16_t SCD30::getMeasurementInterval()
{
    uint16_t interval = readRegister(SCD30_SET_MEASUREMENT_INTERVAL);
    return interval;
}

//gets state of automatic self calibration
//see 1.4.5 in document
boolean SCD30::getAutomaticSelfCalibration()
{
    uint16_t ASC = readRegister(SCD30_SET_AUTOMATIC_SELFCALIBRATION);
    if (ASC == 1) return true;
    else return false;
}


//gets state of forced recalibration value
//see 1.4.5 in document
uint16_t SCD30::getForcedRecalibrationValue()
{
    uint16_t FRC = readRegister(SCD30_SET_FORCED_RECALIBRATION);
    return FRC;
}

//gets set temperature offset
//see 1.4.6 in document
uint16_t SCD30::getTemperatureOffset()
{
    uint16_t temperature = readRegister(SCD30_SET_TEMPERATURE_OFFSET);
    return temperature;
}

//gets set altitude compensation
//see 1.4.7 in document
uint16_t SCD30::getAltitudeCompensation()
{
    uint16_t interval = readRegister(SCD30_SET_ALTITUDE_COMPENSATION);
    return interval;
}

//reads 18 bytes from sensor
//updates global variables with read value
//see 1.4.4 in document
boolean SCD30::readMeasurement()
{
    uint32_t tempCO2 = 0;
    uint32_t tempHumidity = 0;
    uint32_t tempTemperature = 0;

    Wire.beginTransmission(SCD30_I2C_ADDRESS); //begins transmission to slave

    //we split the 16 bit hex value int MSB and LSB
    Wire.write(SCD30_READ_MEASUREMENT >> 8); //cmd MSB - first 8 bits of command
    Wire.write(SCD30_READ_MEASUREMENT & 0x00FF); //cmd LSB - last 8 bits of command

    if (Wire.endTransmission() != 0)
        return false; //sensor did not ACK

    Wire.requestFrom((uint8_t)SCD30_I2C_ADDRESS, (uint8_t)18); //we're receiving an 18 byte message

    if (Wire.available()) 
    {
        for (uint8_t b = 0; b < 18; b++) 
        {
            uint8_t incoming = Wire.read();

            switch(b) 
            {
                //bytes 1, 2, 4 and 5 contain CO2 data
                case 0: 
                case 1:
                //byte 3 cotains CRC
                case 3:
                case 4:
                    tempCO2 <<= 8;
                    tempCO2 |= incoming;
                    break;
                //byte 6 cotains CRC
                //bytes 7, 8, 10 and 11 contain temperature data
                case 6: 
                case 7:
                //byte 9 contains CRC
                case 9:
                case 10:
                    tempTemperature <<= 8;
                    tempTemperature |= incoming;
                    break;
                //byte 12 cotains CRC
                //bytes 13, 14, 16, 17 contain humidity data
                case 12: 
                case 13:
                //byte 15 contains CRC
                case 15:
                case 16:
                    tempHumidity <<= 8;
                    tempHumidity |= incoming;
                    break;
                //byte 18 contains CRC
                default:
                    //skip all CRC bytes
                    break;
            }
        }
    }
    
    //copy the uint32_t CO2 values into uint16_t
    memcpy(&co2, &tempCO2, sizeof(co2));
    //copy the uint32_t temperature and humidity into their associated floats
    memcpy(&temperature, &tempTemperature, sizeof(temperature));
    memcpy(&humidity, &tempHumidity, sizeof(humidity));

    return true;
}

//returns latest available humidity
float SCD30::getHumidity() 
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return humidity;
}

//returns latest available temperature in °C
float SCD30::getTemperatureC()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature;
}

//returns latest available temperature in F
float SCD30::getTemperatureF()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature * 1.8 + 32;
}

//returns latest available temperature in K
float SCD30::getTemperatureK()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature + 273.15;
}

//returns latest available CO2 level
uint16_t SCD30::getCO2()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return co2;
}

//sends a command without arguments
boolean SCD30::sendCommand(uint16_t command)
{
    Wire.beginTransmission(SCD30_I2C_ADDRESS);
    Wire.write(command >> 8); //MSB of command
    Wire.write(command & 0x00FF); //LSB of command

    if (Wire.endTransmission() != 0)
        return false; //sensor did not ACK

    return true;
}

//sends a command with an argument
//we need to calculate CRC on the argument
boolean SCD30::sendCommand(uint16_t command, uint16_t argument)
{
    uint8_t data[2];
    data[0] = argument >> 8; //MSB of argument
    data[1] = argument & 0x00FF; //LSB of argument
    uint8_t crc = computeCRC8(data, 2); //calculate CRC on argument

    Wire.beginTransmission(SCD30_I2C_ADDRESS);
    Wire.write(command >> 8); //MSB of command
    Wire.write(command & 0x00FF); //LSB of command
    Wire.write(argument >> 8); //MSB of argument
    Wire.write(argument & 0x00FF); //LSB of argument
    Wire.write(crc); //we have to also send the CRC

    if (Wire.endTransmission() != 0)
        return false; //sensor did not ACK

    return true;
}

//reads value of register
//only used when checking if data is available
uint16_t SCD30::readRegister(uint16_t registerAddress)
{
    Wire.beginTransmission(SCD30_I2C_ADDRESS);
    Wire.write(registerAddress >> 8); //MSB of register address
    Wire.write(registerAddress & 0x00FF); //LSB of register address

    if (Wire.endTransmission() != 0)
        return 0; //sensor did not ACK

    Wire.requestFrom((uint8_t)SCD30_I2C_ADDRESS, (uint8_t)2); //we're receiving a 2 byte message

    if (Wire.available()) 
    {
        uint8_t MSB = Wire.read();
        uint8_t LSB = Wire.read();

        uint16_t response = MSB << 8;
        response |= LSB;
        return response;
    }

    else return 0;

}

//calculates crc on the arguments being sent
//we do not need to compute crc on the command
//polynomial is: x^8+x^5+x^4+1 = 0x31
//from: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
uint8_t SCD30::computeCRC8(uint8_t data[], uint8_t len)
{
    uint8_t crc = 0x00FF; //initialize with 0xFF
    uint16_t generator = 0x0031;

    for (uint8_t x = 0 ; x < len ; x++)
    {
        crc ^= data[x]; //XOR-in the next input byte

        //crc on one byte data
        for (uint8_t i = 0 ; i < 8 ; i++)
        { 
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ generator);
            else
                crc <<= 1; //most significant bit is not 1, shift left
        }
    }

    return crc;
}

//gets current firmware version, formatted as [Major, Minor]
//see 1.4.8 in document
uint8_t* SCD30::getFirmwareVersion()
{
    uint16_t versionData = readRegister(SCD30_READ_FIRMWARE_VERSION);
    firmwareVersion[0] = versionData >> 8;
    firmwareVersion[1] = versionData & 0xFF;
    return firmwareVersion;
}

//soft resets the sensor
//see 1.4.9 in document
boolean SCD30::softReset()
{
    return(sendCommand(SCD30_SOFT_RESET));
}

//used to attach interrupt from RDY pin of SCD30, parameters are the pin on the MCU and a function
//see attached interruptExample example
void SCD30::attachExternalInterrupt(uint8_t pin, void(*function)(void)) 
{
    attachInterrupt(digitalPinToInterrupt(pin), function, HIGH);    //when a new measurement is ready the value of RDY is 1 (high)
}