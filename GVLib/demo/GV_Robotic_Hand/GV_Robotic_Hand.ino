/*
 * Copyright (c) 2015, GreenVulcano Open Source Project. All rights reserved.
 *
 * This file is part of the GreenVulcano Communication Library for IoT.
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gvlib_arduino.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

using namespace gv;
using gv::CallbackParam;

const uint8_t myIp_[] = { 10, 100, 80, 28 };
byte mac[] = { 0xDE, 0x1D, 0xBC, 0xEE, 0xFE, 0xED };
const uint8_t serverIp_[] = { 10,100,80,39 };
//const uint8_t serverIp_[] = { 10, 100, 60, 103 };
const int port = 1883;
const char device_id[] = "GVDEV001";
const char device_name[] = "GV Robotic Hand";

int modality = 1;
const char* ON = "ON";
const char* OFF = "OFF";
const char* DEMO = "DEMO";

Servo servo_thumb;
Servo servo_index_finger;
Servo servo_middle_finger;
Servo servo_ring_finger;
Servo servo_little_finger;

int pin_thumb = 7;
int pin_index_finger = 6;
int pin_middle_finger = 5;
int pin_ring_finger = 4;
int pin_little_finger = 3;

/****************************************************
 * Callback for basic device operation
 ***************************************************/
gv::CallbackParam cbDevice(gv::CallbackParam payload) {
  StaticJsonBuffer<32> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((const char*) payload.data);

  Serial.println(F("CALLBACK DEVICE CALLED"));
  const char* root_value = (const char*)root["value"];

  if (strcmp(root_value, ON) == 0) {
    Serial.println(F("Mode ON"));
    modality = 1;
  } 
  else if (strcmp(root_value, OFF) == 0) {
    Serial.println(F("Mode OFF"));
    modality = 0;
  }
  else if (strcmp(root_value, DEMO) == 0) {
    Serial.println(F("Mode DEMO"));
    modality = 2;
  }
}

/*****************************************************
   GVLIB callback functions: they have to respect the
   prototype `CallbackParam function (CallbackParam p)`
   You need to register these functions in the
   `setup()` phase.
******************************************************/

gv::CallbackParam thumb(gv::CallbackParam payload) {
  if (modality == 1) {
    Serial.print(F("Position Thumb: "));
    Serial.println((char*)payload.data);
    int value = getServoValue((byte*)payload.data);
    handle_thumb(value, false);
  }
  return payload;
}

gv::CallbackParam index_finger(gv::CallbackParam payload) {
  if (modality == 1) {
    Serial.print(F("Position Index finger: "));
    Serial.println((char*)payload.data);
    int value = getServoValue((byte*)payload.data);
    handle_index_finger(value, false);
  }
  return payload;
}

gv::CallbackParam middle_finger(gv::CallbackParam payload) {
  if (modality == 1) {
    Serial.print(F("Position Middle finger: "));
    Serial.println((char*)payload.data);
    int value = getServoValue((byte*)payload.data);
    handle_middle_finger(value, true);
  }
  return payload;
}

gv::CallbackParam ring_finger(gv::CallbackParam payload) {
  if (modality == 1) {
    Serial.print(F("Position Ring finger: "));
    Serial.println((char*)payload.data);
    int value = getServoValue((byte*)payload.data);
    handle_ring_finger(value, false);
  }
  return payload;
}

gv::CallbackParam little_finger(gv::CallbackParam payload) {
  if (modality == 1) {
    Serial.print(F("Position Little finger: "));
    Serial.println((char*)payload.data);
    int value = getServoValue((byte*)payload.data);
    handle_little_finger(value, true);
  }
  return payload;
}

/****************************************************
   GVLIB initialization: prefer static, so you
   get little or no surprises (e.g. compared to when
   using "dynamic" memory.
*****************************************************/

IPAddr myIp (myIp_);
IPAddr serverIp (serverIp_);
DeviceInfo deviceInfo(device_id, device_name, myIp, port);
EthernetClient ethClient;

avr::arduino::MqttTransport mqttTransport(deviceInfo, serverIp, port, ethClient);
avr::DefaultProtocol protocol(deviceInfo, mqttTransport);
GVComm gvComm(deviceInfo, mqttTransport, protocol);

/****************************************************
 *  Arduino standard setup function
 ****************************************************/
void setup() {

  Serial.begin(115200);
  Ethernet.begin(mac, myIp.v4());

  servo_thumb.attach(pin_thumb);
  servo_index_finger.attach(pin_index_finger);
  servo_middle_finger.attach(pin_middle_finger);
  servo_ring_finger.attach(pin_ring_finger);
  servo_little_finger.attach(pin_little_finger);

  mqttTransport.connect();

  Serial.println(F("Sending Device Information: "));
  gvComm.addDevice(cbDevice);

  Serial.println(F("Sending Actuators Configuration: "));

  gvComm.addActuator("ACD00101",  "Thumb",  "NUMERIC", thumb);
  gvComm.addActuator("ACD00102",  "Index",  "NUMERIC", index_finger);
  gvComm.addActuator("ACD00103",  "Middle", "NUMERIC", middle_finger);
  gvComm.addActuator("ACD00104",  "Ring",   "NUMERIC", ring_finger);
  gvComm.addActuator("ACD00105",  "Little", "NUMERIC", little_finger);

  Serial.println(F("SETUP COMPLETED"));
}

/****************************************************
 *  Arduino standard loop function
 ****************************************************/
void loop() {
  gvComm.poll();

  if (modality == 2) {
    // device in demo
    demoStart();
  }
}

/* --------------------------------------------------------------------------------- */

int getServoValue(byte* payload) {
  StaticJsonBuffer<32> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)payload);

  return (int)root["value"];
}

/* --------------------------------------------------------------------------------- */

void handle_thumb(int value, boolean reverse) {
  if (reverse) {
    value = (int)(-1 * (value - 180));
  }
  servo_thumb.write(value);
}

/* --------------------------------------------------------------------------------- */
void handle_index_finger(int value, boolean reverse) {
  if (reverse) {
    value = (int)(-1 * (value - 180));
  }
  servo_index_finger.write(value);
}

/* --------------------------------------------------------------------------------- */
void handle_middle_finger(int value, boolean reverse) {
  if (reverse) {
    value = (int)(-1 * (value - 180));
  }
  servo_middle_finger.write(value);
}

/* --------------------------------------------------------------------------------- */
void handle_ring_finger(int value, boolean reverse) {
  if (reverse) {
    value = (int)(-1 * (value - 180));
  }
  servo_ring_finger.write(value);
}

/* --------------------------------------------------------------------------------- */
void handle_little_finger(int value, boolean reverse) {
  if (reverse) {
    value = (int)(-1 * (value - 180));
  }
  servo_little_finger.write(value);
}

/****************************************************
 *  Demo
 * *************************************************/
void demoStart() {
  // demo
  int pos = 0;

  for (pos = 0; pos <= 180; pos += 18) {
    servo_thumb.write(pos);
    delay(15);
    servo_index_finger.write(pos);
    delay(15);
    servo_middle_finger.write(pos);
    delay(15);
    servo_ring_finger.write(pos);
    delay(15);
    servo_little_finger.write(pos);
    delay(15);
  }

  for (pos = 180; pos >= 0; pos -= 18) {
    servo_thumb.write(pos);
    delay(15);
    servo_index_finger.write(pos);
    delay(15);
    servo_middle_finger.write(pos);
    delay(15);
    servo_ring_finger.write(pos);
    delay(15);
    servo_little_finger.write(pos);
    delay(15);
  }
}
