# CayenneLPP

This is an Arduino Library that implements a version of the Cayenne Low Power Payload for my use in RFM69 radiocommunication.  It is NOT intended for TTN (or Cayenne for that matter).  It varies in that there is one "channel" per payload, in my case, the "node id."  It also adds the "Voltage" format.

It encodes.  It decodes to JSON via [ArduinoJson](https://arduinojson.org/).

## Documentation

##Examples

* Encode
```c
CayenneLPP lpp(NODEID);

lpp.reset();
lpp.addVoltage(3.028);
lpp.addTemperature(73.2);
lpp.addBarometricPressure(1073.21);
uint8_t len = lpp.getSize();
uint8_t *buff = (uint8_t *)malloc(len);
lpp.copy(buff);
  
// e.g. send it somewhere
radio.sendWithRetry(GATEWAYID, buff, len, 1);
  
free(buff);
```

* Decode
```c
CayenneLPPDecode lppd;

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

// e.g. "you know you can't live without your radio"
lppd.write((uint8_t*)radio.DATA, radio.DATALEN);
lppd.decode(root);
root["rssi"] = radio.RSSI;

Serial.print(radio.NODEID);
root.printTo(Serial);
Serial.println();
lppd.reset();
```

##Credits
* [Cayenne Low Power Payload](https://mydevices.com/cayenne/docs/#lora-cayenne-low-power-payload)
* Based in the work of [Johan Stokking](https://github.com/TheThingsNetwork/arduino-device-lib)
* Luiz Henrique Cassettari (decode stuff)
