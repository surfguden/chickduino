/* Example code for EK006 8x Serial Relay Board.
   Loop through all relays for testing... */

const int dataPin  = 5;                          // Pin DS on Relay board
const int latchPin =  6;                          // Pin LT on Relay board
const int clockPin = 7;                          // Pin CL on Relay board

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);
  pinMode(clockPin, OUTPUT);

  byte data=0;
  bitWrite(data, 3, 1);
  digitalWrite(latchPin, LOW);                  // Latch is low while we load new data to register
  shiftOut(dataPin, clockPin, MSBFIRST, data);  // Load the data to register using ShiftOut function
  digitalWrite(latchPin, HIGH);                 // Toggle latch to present the new data on register outputs

  
}

void loop() {
//  for(int i = 0; i < 8; i++) {                    // Iterate eight times
//    byte data = 1 << i;                           // Bit wise shift a "1" through the 8 bits
//    digitalWrite(latchPin, LOW);                  // Latch is low while we load new data to register
//    shiftOut(dataPin, clockPin, MSBFIRST, data);  // Load the data to register using ShiftOut function
//    digitalWrite(latchPin, HIGH);                 // Toggle latch to present the new data on register outputs
//    delay(420);                                   // Delay before next iteration
  
}
