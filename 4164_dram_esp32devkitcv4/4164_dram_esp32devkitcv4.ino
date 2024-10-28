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
  
  for(int row=0; row<=255; row++) {
    Serial.println("Testing row: " + String(row));
    blink = 1 - blink;
    digitalWrite(GREEN_LED, blink);

    writeBits(row);
    int numberOfBits = readBits(row);
  
    if (numberOfBits != 256) {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Serial.println("ERROR: row " + String(row) + " number of bits was: " + String(numberOfBits)) + ", but should be 255.";
      while(1);
    }
  }

  Serial.println("Test DONE. All OK!");

  while(1) {
    digitalWrite(GREEN_LED, HIGH);
    delay(500);
  }
  
}

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
