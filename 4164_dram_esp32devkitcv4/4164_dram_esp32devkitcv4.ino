/*
KM4164 DRAM testing, based on:
https://ezcontents.org/4164-dynamic-ram-arduino
https://app.arduino.cc/sketches/b565f0c6-8055-41e2-a38b-35dc9009f218?view-mode=preview

USING Esp32 DevKit C V4
*/

#define RAS         19  // ROW ADDRESS STROBE
#define CAS         18  // COLUMN ADDRESS STROBE
#define DATA_IN     5   // D = DATA IN
#define DATA_OUT    17  // Q = DATA OUT
#define NOT_WRITE   16  // READ/WRITE INPUT
#define GREEN_LED   4
#define RED_LED     0
// gpios out 2, 15 are OK

const int ADDR_PINS[8] = {13, 12, 14, 27, 26, 25, 33, 32};
int blink = 1;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  for(int address=0; address<8; ++address) {
    pinMode(ADDR_PINS[address], OUTPUT); 
  }

  pinMode(DATA_IN, OUTPUT); 
  pinMode(DATA_OUT, INPUT); 

  pinMode(RAS, OUTPUT); 
  pinMode(CAS, OUTPUT); 
  pinMode(NOT_WRITE, OUTPUT); 

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
}

// the loop function runs over and over again forever
void loop() {

  Serial.println("Starting test...");

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(500);

/*
  writeByte(8, 0xD2);
  int value = readByte(8);
  Serial.println("value expected 210: " + String(value));
  writeByte(10, 0x2D);
  value = readByte(10);
  Serial.println("value expected 45: " + String(value));
*/

  int nbErrors = 0;
  boolean hasError = false;

  for(int row=0; row<256; row++) {
    blink = 1 - blink;

    if (blink) {
      digitalWrite(hasError ? RED_LED : GREEN_LED, blink);
    } else {
      digitalWrite(GREEN_LED, false);
      digitalWrite(RED_LED, false);
    }

    writeBits(row);
    int numberOfBits = readBits(row);
  
    hasError = (numberOfBits != 256);
    if (hasError) {
      ++nbErrors;
      Serial.println("");
      Serial.println("Error row: " + String(row));
    } else {
      Serial.print(String(row) + ", ");
    }

  }

  Serial.println("Test DONE, Errors : " + String(nbErrors));

  if (nbErrors > 0) {
      digitalWrite(GREEN_LED, false);
      digitalWrite(RED_LED, true);
  } else {
      digitalWrite(GREEN_LED, true);
      digitalWrite(RED_LED, false);
  }

  while(1) {
    delay(500);
  }
  
}

// write Byte : NOT really possible as the speed is not accurate
/*
void writeByte(int address, int value) {

  address = address & 0x1FFF;   // only 8 kbytes
  int bit = address * 8;
  int row = bit >> 8;
  int column = bit & 0xFF;

  // Pull RAS and CAS HIGH
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);
  digitalWrite(NOT_WRITE, HIGH);

  unsigned long startTime = 0;
  unsigned long stopTime = 0;

  // Loop though all the columns
  for (int i=0; i<8; i++) {

    // Set row address
    for(int pin=0; pin<8; ++pin) {
      digitalWrite(ADDR_PINS[pin], bitRead(row, pin));
    }

    startTime = micros();
    // Pull RAS LOW
    digitalWrite(RAS, LOW);

    // Pull Write LOW (Enables write)
    digitalWrite(NOT_WRITE, LOW);   

    // Set Data (little endian)
    int data = bitRead(value, i);
    digitalWrite(DATA_IN, data);

    // Set column address
    for(int pin=0; pin<8; ++pin) {
      digitalWrite(ADDR_PINS[pin], bitRead(column+i, pin));
    }
    stopTime = micros();
    // Pull CAS LOW
    digitalWrite(CAS, LOW);

    digitalWrite(NOT_WRITE, HIGH);
    digitalWrite(CAS, HIGH);
    digitalWrite(RAS, HIGH);
  }

  unsigned long diff = stopTime - startTime;
  Serial.println("duration : " + String(diff) + " micros");
}
*/
void writeBits(int row) {

  // Pull RAS and CAS HIGH
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);

  // Loop though all the columns
  for (int i=0; i<=255; i++) {

    // Set row address
    for(int address=0; address<8; ++address) {
      digitalWrite(ADDR_PINS[address], bitRead(row, address));
    }

    // Pull RAS LOW
    digitalWrite(RAS, LOW);

    // Pull Write LOW (Enables write)
    digitalWrite(NOT_WRITE, LOW);   

    // Set Data in pin to HIGH (write a one)
    digitalWrite(DATA_IN, HIGH);

    // Set column address
    for(int address=0; address<8; ++address) {
      digitalWrite(ADDR_PINS[address], bitRead(i, address));
    }

    // Pull CAS LOW
    digitalWrite(CAS, LOW);

    digitalWrite(RAS, HIGH);
    digitalWrite(CAS, HIGH);
  }
}

// readByte: NOT really possible as the speed is not accurate
/*
int readByte(int address) {

  address = address & 0x1FFF;   // only 8 kbytes
  int bit = address * 8;
  int row = bit >> 8;
  int column = bit & 0xFF;
  int value = 0;

    // Pull RAS, CAS and Write HIGH
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);
  digitalWrite(NOT_WRITE, LOW);

  // Loop though all the columns
  for (int i=0; i<8; i++) {

    // Set row address
    for(int pin=0; pin<8; ++pin) {
      digitalWrite(ADDR_PINS[pin], bitRead(row, pin));
    }

    // Pull RAS LOW
    digitalWrite(RAS, LOW);

    digitalWrite(NOT_WRITE, HIGH);

    // Set column address
    for(int pin=0; pin<8; ++pin) {
      digitalWrite(ADDR_PINS[pin], bitRead(column+i, pin));
    }
    // Pull CAS LOW
    digitalWrite(CAS, LOW);

    // Read the stored bit
    int data = digitalRead(DATA_OUT);
    value += (data << i);

    // Pull RAS and CAS HIGH
    digitalWrite(CAS, HIGH);
    digitalWrite(RAS, HIGH);
    digitalWrite(NOT_WRITE, LOW);
  }

  return value;
}
*/

int readBits(int row) {

  // Bit counter
  int numberOfBits = 0;

  // Pull RAS, CAS and Write HIGH
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);
  digitalWrite(NOT_WRITE, HIGH);

  // Loop though all the columns
  for (int i=0; i<=255; i++) {

    // Set row address
    for(int address=0; address<8; ++address) {
      digitalWrite(ADDR_PINS[address], bitRead(row, address));
    }

    // Pull RAS LOW
    digitalWrite(RAS, LOW);

    // Set column address
    for(int address=0; address<8; ++address) {
      digitalWrite(ADDR_PINS[address], bitRead(i, address));
    }

    // Pull CAS LOW
    digitalWrite(CAS, LOW);

    // Read the stored bit and add to bit counter
    numberOfBits += digitalRead(DATA_OUT);

    // Pull RAS and CAS HIGH
    digitalWrite(RAS, HIGH);
    digitalWrite(CAS, HIGH);
  }

  return numberOfBits;
}
