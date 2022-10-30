#define latchPin1  13      //Pin connected to ST_CP of 74HC595
#define clockPin1  12       //Pin connected to SH_CP of 74HC595
#define dataPin1   11       //Pin connected to DS of 74HC595

#define latchPin2  10      //Pin connected to ST_CP of 74HC595
#define clockPin2  9      //Pin connected to SH_CP of 74HC595
#define dataPin2   8      //Pin connected to DS of 74HC595

#define colonPin 7

//#define latchPin3  4      //Pin connected to ST_CP of 74HC595
//#define clockPin3  3      //Pin connected to SH_CP of 74HC595
//#define dataPin3   2      //Pin connected to DS of 74HC595

uint8_t nixies = 0x88; //initiate the byte to be sent to the shift register and set it to blank the nixies
boolean colon = LOW;
                    
//the process of sending a byte to the shift register
void updateShiftRegister(uint8_t retVal){
  digitalWrite(latchPin1, LOW);
  shiftOut(dataPin1, clockPin1, MSBFIRST, retVal);
  digitalWrite(latchPin1, HIGH);

  digitalWrite(latchPin2, LOW);
  shiftOut(dataPin2, clockPin2, MSBFIRST, retVal);
  digitalWrite(latchPin2, HIGH);

  digitalWrite(colonPin, HIGH);
  colon = !colon;
//  digitalWrite(latchPin3, LOW);
//  shiftOut(dataPin3, clockPin3, MSBFIRST, retVal);
//  digitalWrite(latchPin3, HIGH);
}



void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin1, OUTPUT);
  pinMode(clockPin1, OUTPUT);
  pinMode(dataPin1, OUTPUT);

  pinMode(latchPin2, OUTPUT);
  pinMode(clockPin2, OUTPUT);
  pinMode(dataPin2, OUTPUT);

  pinMode(colonPin, OUTPUT);
//  pinMode(latchPin3, OUTPUT);
//  pinMode(clockPin3, OUTPUT);
//  pinMode(dataPin3, OUTPUT);

  Serial.begin(9600);
}



void loop() {
  static uint8_t tens, units, retVal;
  
//  nixies = (nixies + 1) % 100;
//
//  tens = nixies / 10;
//  units = nixies % 10;
//
//  retVal = ((tens << 4) & 0xF0) + (units & 0x0F);
  retVal = ++nixies;

  updateShiftRegister(retVal);
  Serial.println(retVal);
  delay(500);
}
