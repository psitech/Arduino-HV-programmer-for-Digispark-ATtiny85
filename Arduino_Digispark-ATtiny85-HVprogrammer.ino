// https://www.rickety.us/2010/03/arduino-avr-high-voltage-serial-programmer/
// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
// https://www.instructables.com/id/How-to-unlock-Digispark-ATtiny85-and-convert-it-to/
// https://eleccelerator.com/fusecalc/fusecalc.php?chip=attiny85&LOW=E1&HIGH=5D&EXTENDED=FE&LOCKBIT=FF

// intended fuse settings
#define  LFUSE  0xE1    // Digispark default
#define  HFUSE  0xDD    // 0xDD: pin 5 acts as reset pin, 0x5D: pin 5 acts as GPIO pin [Digispark default]
#define  EFUSE  0xFE    // Digispark default

#define  HVb  13        // connect to 2N3904 base pin via 1K resistor
#define  SCI  12        // connect to Digispark pin 3 [ATtiny85 PB3]
#define  SDO  11        // connect to Digispark pin 2 [ATtiny85 PB2]
#define  SII  10        // connect to Digispark pin 1 [ATtiny85 PB1]
#define  SDI   9        // connect to Digispark pin 0 [ATtiny85 PB0]
#define  VCC   8        // connect to Digispark 5V 
//                      // connect Arduino GND to Digispark GND

uint8_t SDObyte = 0;    // SDO byte from ATtiny85

void setup() {
  // initialize pins
  pinMode(VCC, OUTPUT);
  pinMode(HVb, OUTPUT);   // active low HV enable
  pinMode(SDI, OUTPUT);
  pinMode(SII, OUTPUT);
  pinMode(SCI, OUTPUT);
  pinMode(SDO, OUTPUT);   // configured as input when in programming mode

  digitalWrite(HVb, HIGH); // turn off 12V
  digitalWrite(VCC, LOW);  // turn off 5V
  digitalWrite(SCI, LOW);

  // send message to serial monitor
  Serial.begin(9600);
  Serial.println("Enter a character to start the programming cycle");
  Serial.println();

  // wait for user input
  while (!Serial.available());  // wait for character from serial monitor
  Serial.println("Entering programming mode");
  Serial.println();

  // initialize pins for HV serial programming mode
  digitalWrite(SDI, LOW);
  digitalWrite(SII, LOW);
  digitalWrite(SDO, LOW);

  digitalWrite(VCC, HIGH);  // turn on Vcc
  delay(1);                 // wait 1 msec for Vcc to stabilize
  digitalWrite(HVb, LOW);   // turn on 12V
  delay(1);                 // wait 1 msec for 12V to stabilize
  pinMode(SDO, INPUT);      // make SDO input
  delay(1);                 // wait 1 msec before serial instructions on SDI/SII

  readFuses();              // read the current fuse settings
  writeFuses();             // write the new fuse settings
  readFuses();              // read the new fuse settings

  Serial.print("Exiting programming mode...");
  digitalWrite(HVb, HIGH);  // turn off 12V [= ATtiny reset, active low]
  digitalWrite(SII, LOW);
  digitalWrite(SCI, LOW);
  pinMode(SDO, OUTPUT);
  digitalWrite(SDO, LOW);
  digitalWrite(VCC, LOW);   // turn off Vcc
  Serial.println("done");
}

void loop() {
}

uint8_t shiftOut2(uint8_t SDIbyte, uint8_t SIIbyte) {
  uint8_t i;
  SDObyte = 0;
  while (!digitalRead(SDO));  // wait until SDO goes high

  // 1 start-bit "0"
  digitalWrite(SDI, LOW);
  digitalWrite(SII, LOW);
  digitalWrite(SCI, HIGH);
  digitalWrite(SCI, LOW);

  for (i = 0; i < 8; i++)  {
    digitalWrite(SDI, !!(SDIbyte & (1 << (7 - i))));
    digitalWrite(SII, !!(SIIbyte & (1 << (7 - i))));

    SDObyte <<= 1;
    SDObyte |= digitalRead(SDO);
    digitalWrite(SCI, HIGH);
    digitalWrite(SCI, LOW);
  }

  // 2 stop-bits "00"
  digitalWrite(SDI, LOW);
  digitalWrite(SII, LOW);
  digitalWrite(SCI, HIGH);
  digitalWrite(SCI, LOW);
  digitalWrite(SCI, HIGH);
  digitalWrite(SCI, LOW);

  return SDObyte;
}

void readFuses() {
  // read lfuse
  shiftOut2(0x04, 0x4C);
  shiftOut2(0x00, 0x68);
  SDObyte = shiftOut2(0x00, 0x6C);
  Serial.print("lfuse reads as ");
  Serial.println(SDObyte, HEX);

  // read hfuse
  shiftOut2(0x04, 0x4C);
  shiftOut2(0x00, 0x7A);
  SDObyte = shiftOut2(0x00, 0x7E);
  Serial.print("hfuse reads as ");
  Serial.println(SDObyte, HEX);

  // read efuse
  shiftOut2(0x04, 0x4C);
  shiftOut2(0x00, 0x6A);
  SDObyte = shiftOut2(0x00, 0x6E);
  Serial.print("efuse reads as ");
  Serial.println(SDObyte, HEX);
  Serial.println();
}

void writeFuses() {
  // write lfuse
  Serial.print("set lfuse to ");
  Serial.println(LFUSE, HEX);
  shiftOut2(0x40, 0x4C);
  shiftOut2(LFUSE, 0x2C);
  shiftOut2(0x00, 0x64);
  shiftOut2(0x00, 0x6C);

  // write hfuse
  Serial.print("set hfuse to ");
  Serial.println(HFUSE, HEX);
  shiftOut2(0x40, 0x4C);
  shiftOut2(HFUSE, 0x2C);
  shiftOut2(0x00, 0x74);
  shiftOut2(0x00, 0x7C);

  // write efuse
  Serial.print("set efuse to ");
  Serial.println(EFUSE, HEX);
  shiftOut2(0x40, 0x4C);
  shiftOut2(EFUSE, 0x2C);
  shiftOut2(0x00, 0x66);
  shiftOut2(0x00, 0x6E);
  Serial.println();
}
