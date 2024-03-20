/*
This keyer is expected to be eventually a part of the stationmaster hardware.
My plan is to move from the current Raspberry Pi to a mini Mac.  Furthermore
Because of the real time issues with a go runtime environment and Linux OS,
I will move the keyer to its own Arduino Uno R4 and instead of using the current
bistable multivibrator, I will use the Arduion "Tone" function to generate tones.
I will also use an Adafruit class D mono audio amp to drive the speaker.  The 
combination fo the two will give me control over the tone and loudness.  The
interface to the Mac will be USB.
*/

/*
The first release will not depend on a Mac and will work the following way:
+ I will use the QEX April 1990 morse code timing article as a guide
+ Imitially, I will only support the following speeds
  + 18, 20, 22, 25, 28, 30, and 35 WPM
+ That will allow me to avoid the Farnsworth charachter timing
+ The dit and dah timing will be pre calculated as program constants
+ 7 input pins with pull up resistors to select one of the 7 speeds by grounding
+ 
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
int roundCnt = 0;  //loop counts for triggering serial1 read
int maxRound = 1;  //when reached, trigger serial1 read


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
  Serial1.begin(115200);
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
