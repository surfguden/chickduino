/* Example code for EK006 8x Serial Relay Board.
   Read inputs D0 - D7 and toggle relays. */

const int latchPin = 8;                           // Pin LT on Relay board
const int dataPin  = 11;                          // Pin DS on Relay board
const int clockPin = 12;                          // Pin CL on Relay board

void setup() {
  
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

}

void loop() {

  DDRD = 0b00000000;                              // Set PORTD (D0 - D7) to inputs
  PORTD = 0b11111111;                             // Enable pull-up for PORTD
  static byte last_data = 0xFF;                   // Declare variable for comparing last button state
  static byte toggle = 0x00;                      // Declare variable for the relays
  
  byte data = ~PIND;                              // Read and invert PORTD (D0 - D7) and store to data

  toggle ^= ((last_data ^ data) & data);          // Compare last button state to new state using XOR
  last_data = data;                               // Write back current state to last state...
  
  digitalWrite(latchPin, LOW);                    // Latch is low while we load new data to register
  shiftOut(dataPin, clockPin, MSBFIRST, toggle);  // Load the data to register using ShiftOut function
  digitalWrite(latchPin, HIGH);                   // Toggle latch to present the new data on register outputs

  delay(24);                                      // Short delay to debounce buttons
}
  

