// Moteino to Serial Receiver RFM69H
// with CayenneLPP format
//
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
#include <RFM69.h>            // https://github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>        // https://github.com/lowpowerlab/RFM69
#include <RFM69_OTA.h>        // https://github.com/lowpowerlab/RFM69
#include <SPIFlash.h>         // https://github.com/lowpowerlab/spiflash
#include <SPI.h>              // standard
#include <CayenneLPP.h>
#include <CayenneLPPDecode.h>
//******************************************************************************
// USER SETTINGS - Change for your hardware / setup
//******************************************************************************
#define NODEID           1 //the ID of this node
#define NETWORKID      100 //the network ID of all nodes this node listens/talks to

#define FREQUENCY RF69_915MHZ         // RF69_433MHZ, RF69_868MHZ)
                   
#define ENCRYPTKEY "sampleEncryptKey" //identical 16 characters/bytes on all nodes
#define IS_RFM69HW_HCW     // uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
#define ACK_TIME        30 // # of ms to wait for an ack packet
#define ENABLE_ATC         // comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI       -75 // target RSSI for RFM69_ATC (recommended > -80)
#define SERIAL_BAUD  38400
#define LED              9 // Moteinos have LEDs on D9
#define FLASH_SS         8 // and FLASH SS on D8

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit Windbond FlashMEM chip

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif

  radio.encrypt(ENCRYPTKEY);
  
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif
  
  char buff[30];
  sprintf(buff, "Transmitting at %d Mhz.", radio.getFrequency()/1000000);
  Serial.println(buff);

  if (flash.initialize()) {
    Serial.println("SPI Flash Init OK!");
  } else {
    Serial.println("SPI FlashMEM not found (is chip onboard?)");
  }
}

void Blink(int8_t delay_ms, byte count = 1) {
  for (byte i = 0; i < count; i++) {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    delay(delay_ms);
    digitalWrite(LED, LOW);
  }
}

CayenneLPPDecode lppd;
      
void loop() {
   
  if (radio.receiveDone()) {
    if (radio.DATALEN > 0) {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();

      lppd.write((uint8_t*)radio.DATA, radio.DATALEN);
      lppd.decode(root);
      root["rssi"] = radio.RSSI;

      Serial.print(radio.SENDERID);
      root.printTo(Serial);
      Serial.println();
      lppd.reset();
    }  
    
    CheckForWirelessHEX(radio, flash, false); //non verbose DEBUG

    if (radio.ACKRequested()) radio.sendACK();

    Blink(100, 3);
  }
}
