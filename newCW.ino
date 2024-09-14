/*
With the introduction of the new Arduino Uno R4, I decided to abandon the
hardware based keyer/practice oscillator for the Ten Tec Omni D (or any other
old radio) https://ad2cc.blogspot.com/2022/04/ten-tec-transceiver-enhancements.html
and build a (sort of) software based solution.

It has two basic functions, one an automatic keyer for the radio
and the other is a practice oscillator.  

It gets its commands from a remote computer (in my case, my Stationmaster software)
over the serial port, but you can also interface it to any kind of mechanical switch 
to set the speed and the keyer/practice oscillator option.

These days, I mostly work faster than 20 wpm, so I have dropped the Farnswoth
functionality, but I am using the QEX April 1990 morse code timing article as 
a guide

The hardware, in addition to the Arduino Uno R4 Minima, is a speaker (Amazon),
a class D audio amplifier (Adafruit), a software programmable attenuator (also
from Adafruit).  The programmable attenuator gives me the ability to set the
sound volume in software.  I also use an Adafruit USB to serial convertor to
interface to the computer so I have more options how to power the Arduino 
(not from the computer only).  

I use the Arduino "Tome" function to getnerate the practice tones and set
the frequency.  
*/

#include <Wire.h>
#include <Adafruit_DS1841.h>

const byte address = 0x80;
int ditPin = 10;
int dahPin = 9;
int tonePin = 11;
int keyOutPin = 8;

int dit = 55;
int dah = 165;
int freq;
int recMsgLen = 8;
unsigned char rBuff[8];
unsigned char wBuff[1];
int keyer = 0;

Adafruit_DS1841 res;

void initBuffers();
void sendAck();
void sendNack();
unsigned int calcCRC(unsigned char[], int);
bool checkCRC(unsigned char[], int);

void setup() {
  //Serial.begin(115200);
  //while (!Serial) delay(10);
  //Serial.println("Opened Serial");
  Serial1.begin(38400);
  while (!Serial1) delay(10);
  //Serial.println("Opened Serial1");

  res.begin();

  pinMode(ditPin, INPUT_PULLUP);
  pinMode(dahPin, INPUT_PULLUP);
  pinMode(keyOutPin, OUTPUT);
  digitalWrite(keyOutPin, 0);
}

void loop() {
  initBuffers();
  int validAddress = 0;
  int n = 0;
  // if (Serial1.available()) {
  //   Serial1.readBytes(rBuff, 8);

  //   for (int i = 0; i < 8; i++) {
  //     Serial.print(rBuff[i]);
  //     Serial.print(" ");
  //   }
  //   Serial.println("");
  // }
  n = Serial1.available();
  if (n == recMsgLen) {
    for (int i = 0; i < n; ++i) {
      int s = Serial1.read();
      if (i == 0 && s != address) {
        Serial1.flush();
      }
      // Serial.print(s);
      // Serial.print(" ");
      rBuff[i] = s;
    }
    //Serial.println("");
  }
  if (rBuff[0] == address) {
    validAddress = 1;
  }

  if (validAddress) {
    switch (rBuff[1]) {
      case 0x01:  //test if I am here
        if (checkCRC(rBuff, recMsgLen)) {
          sendAck();
        } else {
          sendNack();
        }
        break;
      case 0x02:
        if (checkCRC(rBuff, recMsgLen)) {
          freq = (rBuff[2] << 8) | rBuff[3];
          dit = rBuff[5];
          dah = dit + dit + dit;
          res.setWiper((int)rBuff[4]);
          keyer = 0;
          sendAck();
        } else {
          sendNack();
        }
        break;
      case 0x03:
        if (checkCRC(rBuff, recMsgLen)) {
          dit = rBuff[5];
          dah = dit + dit + dit;
          keyer = 1;
          sendAck();
        } else {
          sendNack();
        }
        break;
    }
  }
  if (keyer == 0) {
    if (!digitalRead(ditPin)) {
      tone(tonePin, freq, dit);
      delay(dit + dit);
    }
    if (!digitalRead(dahPin)) {
      tone(tonePin, freq, dah);
      delay(dah + dit);
    }
  }
  if (keyer == 1) {
    if (!digitalRead(ditPin)) {
      digitalWrite(keyOutPin, 1);
      delay(dit);
      digitalWrite(keyOutPin, 0);
      delay(dit);
    }
    if (!digitalRead(dahPin)) {
      digitalWrite(keyOutPin, 1);
      delay(dah);
      digitalWrite(keyOutPin, 0);
      delay(dit);
    }
  }
  //delay(20);
}



void sendAck() {
  Serial1.write((char)0xFF);
}

void sendNack() {
  Serial1.write((char)0x00);
}

//call this function with the exact number of bytes you want processed
unsigned int calcCRC(unsigned char msg[], int nBytes) {
  //unsigned char msg[nBytes];
  unsigned int crc = 0x0000;
  for (int byte = 0; byte < nBytes; byte++) {
    crc = crc ^ ((unsigned int)msg[byte] << 8);
    for (unsigned char bit = 0; bit < 8; bit++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

bool checkCRC(unsigned char msg[], int nBytes) {
  unsigned int crc = calcCRC(msg, nBytes - 2);
  unsigned char byte1 = crc >> 8;  //High byte
  unsigned char byte0 = crc;       //Low byte
  if (byte1 == msg[nBytes - 2] && byte0 == msg[nBytes - 1]) {
    return true;
  }
  return false;
}

void initBuffers() {
  wBuff[0] = 0;
  for (int i = 0; i < 8; i++) {
    rBuff[i] = 0;
  }
}
