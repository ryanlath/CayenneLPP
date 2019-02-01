// Moteino Low Power RFM69/H Node
// 
// License
// This program is free software; you can redistribute it and/or modify it under 
// the terms of the GNU General Public License as published by the Free Software       
// Foundation; either version 2 of the License, or (at your option) any later 
// version.                    
//                                                        
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.                              
//                                                        
// Licence can be viewed at: http://www.gnu.org/licenses/gpl-2.0.txt
//******************************************************************************
#include <RFM69.h>         // https://github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     // https://github.com/lowpowerlab/rfm69
#include <RFM69_OTA.h>     // https://github.com/lowpowerlab/rfm69
#include <SPIFlash.h>      // https://github.com/lowpowerlab/spiflash
#include <SPI.h>           // standard
#include <Wire.h>          // standard
#include <BME280I2C.h>     // ???
#include <LowPower.h>      // https://github.com/lowpowerlab/lowpower
#include <Vcc.h>           // https://github.com/Yveaux/Arduino_Vcc
#include <CayenneLPP.h>    // https://github.com/ryanlath/CayenneLPP
//******************************************************************************
// USER SETTINGS - Change for your hardware / setup
//******************************************************************************
#define NODEID      `  // <-- Change for every node
#define GATEWAYID   1
#define NETWORKID   100

//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
//#define FLASH_SS      8 // and FLASH SS on D8

#define SERIAL_BAUD   38400
//#define DEBUG

#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "sampleEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!

#define TX_INTERVAL   304 // seconds (multiple of 8's) (304s~5m, 904s~15m)
// read battery voltage every TX_INTERVAL intervals 
// e.g. tx=304s, bri=12=~1/hr, bri=36=~3hrs
#define BATT_INTERVAL 36

// Auto Transmission Control - dials down transmit power to save battery (-100 is the noise floor, -90 is still pretty good)
// For indoor nodes that are pretty static and at pretty stable temperatures (like a MotionMote) -90dBm is quite safe
// For more variable nodes that can expect to move or experience larger temp drifts a lower margin like -70 to -80 would probably be better
// Always test your ATC mote in the edge cases in your own environment to ensure ATC will perform as you expect
#define ENABLE_ATC
#define ATC_RSSI      -75

//******************************************************************************

#ifdef DEBUG
  #define dprint(...) { Serial.print(__VA_ARGS__); }
  #define dprintln(...) { Serial.println(__VA_ARGS__); }
  #define dflush() { Serial.flush(); }
#else
  #define dprint(...) {}
  #define dprintln(...) {}
  #define dflush() {}
#endif

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

#ifdef FLASH_SS
  SPIFlash flash(FLASH_SS, 0xEF30); //WINDBOND 4MBIT flash chip on CS pin D8 (default for Moteino)
#endif

BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Sleep,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
);

BME280I2C bme(settings);

const float VccMin   = 0.0;           // Minimum expected Vcc level, in Volts.
const float VccMax   = 3.0;           // Maximum expected Vcc level, in Volts.
const float VccCorrection = 1.0/1.0; // Measured Vcc by multimeter divided by reported Vcc
Vcc vcc(VccCorrection);

void setup(void) {
 
#ifdef DEBUG
  Serial.begin(SERIAL_BAUD);
#endif
   
#ifdef FLASH_SS
  if (flash.initialize()) flash.sleep();
#endif
  
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif

  radio.encrypt(ENCRYPTKEY);

#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

#ifdef DEBUG
  char buffer[50];
  sprintf(buffer, "Node transmitting at: %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  dprintln(buffer);
#endif

  Wire.begin();
  Wire.setClock(400000); //Increase to fast I2C speed!

  //hangs or no ouput when no sensor???
  while(!bme.begin()) {
    dprintln("Could not find BME280 sensor!");
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);  
    delay(1000);
  }

  for (uint8_t i=0; i<=A5; i++) {
    if (i == RF69_SPI_CS) continue;
    #ifdef FLASH_SS
    if (i == FLASH_SS) continue;
    #endif
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  } 
}

CayenneLPP lpp(NODEID);
byte battCount = 0;

void loop() {  

  float temp(NAN), hum(NAN), pres(NAN);
  lpp.reset();

  if (battCount == 0) { 
    dprintln("Battery Read");
    float voltage = vcc.Read_Volts();
    lpp.addVoltage(voltage);
    battCount = BATT_INTERVAL;
  }
  battCount--;  

  settings.mode = BME280::Mode_Forced;
  bme.setSettings(settings);
  BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
  BME280::PresUnit presUnit(BME280::PresUnit_inHg);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  settings.mode = BME280::Mode_Sleep;
  bme.setSettings(settings);

  lpp.addTemperature(temp);
  lpp.addBarometricPressure(pres);
  lpp.addRelativeHumidity(hum);

  dprint("Sending..."); 
  uint8_t len = lpp.getSize();
  uint8_t *buff = (uint8_t *)malloc(len);
  lpp.copy(buff);
  
  radio.sendWithRetry(GATEWAYID, buff, len, 1); //retry one time
  
  free(buff);
  dprintln("Sent."); 
  dflush();

  for (uint16_t i = 0; i < TX_INTERVAL; i+=8) {
    dprint(i);
    dprint('.');
    dflush();
#ifdef FLASH_SS
    flash.sleep();
#endif
    radio.sleep();
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);          
  }  
  
  dprintln("\nWakeup!");
}
