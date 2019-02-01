// Adapted from https://developer.mbed.org/teams/myDevicesIoT/code/Cayenne-LPP/

// Copyright © 2017 The Things Network
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#include "CayenneLPP.h"

//TODO add, uint8_t maxsize
CayenneLPP::CayenneLPP(uint8_t channel) {
  buffer = (uint8_t *)malloc(maxsize);
  cursor = 0;
  buffer[cursor++] = channel;
}

CayenneLPP::~CayenneLPP(void) {
  free(buffer);
  //free(result);
}

void CayenneLPP::reset(void) {
  //free(result);
  cursor = 1;
}

uint8_t CayenneLPP::getSize(void) {
  return cursor;
}

uint8_t *CayenneLPP::getBuffer(void) {
  //uint8_t result[cursor];
  //memcpy(result, buffer, cursor);
  //return result;
  return buffer;
}

uint8_t CayenneLPP::copy(uint8_t *dst) {
  memcpy(dst, buffer, cursor);
  return cursor;
}

uint8_t CayenneLPP::addDigitalInput(uint8_t value, uint8_t channel) {

  if ((cursor + LPP_DIGITAL_INPUT_SIZE) > maxsize) return 0;
 
  buffer[cursor++] = LPP_DIGITAL_INPUT;
  buffer[cursor++] = value;

  return cursor;
}

uint8_t CayenneLPP::addDigitalOutput(uint8_t value, uint8_t channel) {

  if ((cursor + LPP_DIGITAL_OUTPUT_SIZE) > maxsize) return 0;
 
  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_DIGITAL_OUTPUT;
  buffer[cursor++] = value;

  return cursor;
}

uint8_t CayenneLPP::addAnalogInput(float value, uint8_t channel) {

  if ((cursor + LPP_ANALOG_INPUT_SIZE) > maxsize) return 0;

  int16_t val = value * 100;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_INPUT;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addAnalogOutput(float value, uint8_t channel) {

  if ((cursor + LPP_ANALOG_OUTPUT_SIZE) > maxsize) return 0;

  int16_t val = value * 100;
  
  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ANALOG_OUTPUT;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addLuminosity(uint16_t lux, uint8_t channel) {

  if ((cursor + LPP_LUMINOSITY_SIZE) > maxsize) return 0;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_LUMINOSITY;
  buffer[cursor++] = lux >> 8;
  buffer[cursor++] = lux;

  return cursor;
}

uint8_t CayenneLPP::addPresence(uint8_t value, uint8_t channel) {

  if ((cursor + LPP_PRESENCE_SIZE) > maxsize) return 0;
 
  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_PRESENCE;
  buffer[cursor++] = value;

  return cursor;
}

uint8_t CayenneLPP::addTemperature(float celsius, uint8_t channel) {

  if ((cursor + LPP_TEMPERATURE_SIZE) > maxsize) return 0;

  int16_t val = celsius * 10;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_TEMPERATURE;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addRelativeHumidity(float rh, uint8_t channel) {

  if ((cursor + LPP_RELATIVE_HUMIDITY_SIZE) > maxsize) return 0;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_RELATIVE_HUMIDITY;
  buffer[cursor++] = rh * 2; // TODO: why two?

  return cursor;
}

uint8_t CayenneLPP::addAccelerometer(float x, float y, float z, uint8_t channel) {

  if ((cursor + LPP_ACCELEROMETER_SIZE) > maxsize) return 0;

  int16_t vx = x * 1000;
  int16_t vy = y * 1000;
  int16_t vz = z * 1000;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_ACCELEROMETER;
  buffer[cursor++] = vx >> 8;
  buffer[cursor++] = vx;
  buffer[cursor++] = vy >> 8;
  buffer[cursor++] = vy;
  buffer[cursor++] = vz >> 8;
  buffer[cursor++] = vz;

  return cursor;
}

uint8_t CayenneLPP::addBarometricPressure(float hpa, uint8_t channel) {

  if ((cursor + LPP_BAROMETRIC_PRESSURE_SIZE) > maxsize) return 0;
 
  int16_t val = hpa * 10;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_BAROMETRIC_PRESSURE;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addVoltage(float voltage, uint8_t channel) {

  if ((cursor + LPP_VOLTAGE_SIZE) > maxsize) return 0;
 
  int16_t val = voltage * 1000;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_VOLTAGE;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addUnixTime(uint32_t unixtime, uint8_t channel) {

  if ((cursor + LPP_UNIXTIME_SIZE) > maxsize) return 0;
 
  int32_t val = unixtime;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_UNIXTIME;
  buffer[cursor++] = val >> 24;
  buffer[cursor++] = val >> 16;
  buffer[cursor++] = val >> 8;
  buffer[cursor++] = val;

  return cursor;
}

uint8_t CayenneLPP::addGyrometer(float x, float y, float z, uint8_t channel) {

  if ((cursor + LPP_GYROMETER_SIZE) > maxsize) return 0;
 
  int16_t vx = x * 100;
  int16_t vy = y * 100;
  int16_t vz = z * 100;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_GYROMETER;
  buffer[cursor++] = vx >> 8;
  buffer[cursor++] = vx;
  buffer[cursor++] = vy >> 8;
  buffer[cursor++] = vy;
  buffer[cursor++] = vz >> 8;
  buffer[cursor++] = vz;

  return cursor;
}

uint8_t CayenneLPP::addGPS(float latitude, float longitude, float meters, uint8_t channel) {

  if ((cursor + LPP_GPS_SIZE) > maxsize) return 0;
 
  int32_t lat = latitude * 10000;
  int32_t lon = longitude * 10000;
  int32_t alt = meters * 100;

  if (channel > 0) buffer[cursor++] = channel;
  buffer[cursor++] = LPP_GPS;
  buffer[cursor++] = lat >> 16;
  buffer[cursor++] = lat >> 8;
  buffer[cursor++] = lat;
  buffer[cursor++] = lon >> 16;
  buffer[cursor++] = lon >> 8;
  buffer[cursor++] = lon;
  buffer[cursor++] = alt >> 16;
  buffer[cursor++] = alt >> 8;
  buffer[cursor++] = alt;

  return cursor;
}
