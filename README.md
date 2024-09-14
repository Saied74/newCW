# newCW
CW keyer and practice oscillator

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
