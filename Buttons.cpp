#include "Buttons.h"

#define RISING_EDGE_SEQUENCE  0x03 // Sequence 0,1,1
#define FALLING_EDGE_SEQUENCE 0x04 // Sequence 1,0,0

const int BUTTON_HOLD_THRESHOLD = 500;  // Time in ms that the button has to be held to be a long press

Button::Button(unsigned char pin) {
  buttonPin = pin;
}

// Debounce the button by executing this function on regular time intervals (between 10-20ms)
void Button::debounce(void) {
  // Shift register tracks the sequences of state reads
  stateSequence = ((stateSequence << 1) | digitalRead(buttonPin)) & 0x07;

  // Check if the sequence shows a falling or rising edge
  if(stateSequence == FALLING_EDGE_SEQUENCE){
    // Button was released, raise event flag
    lastRelease = millis();
    eventFlag = 1;
    Serial.println("Falling edge");
  }
  else if(stateSequence == RISING_EDGE_SEQUENCE){
    // Button was pressed
    lastPress = millis();
  }
}

// Returns the status of the button as short or long press, currently pressing, or none
int Button::getStatus(void) {
  int returnStatus = NONE;
  
  if(lastPress > lastRelease){
    // Button is currently pressed
    returnStatus = PRESSING;
  }
  else if (eventFlag){
    // Button is currently open and there is an unserviced event
    if((lastRelease - lastPress) > BUTTON_HOLD_THRESHOLD){
      returnStatus = LONG_PRESS;
      Serial.println("Long press");
    }
    else{
      returnStatus = SHORT_PRESS;
      Serial.println("Short press");
    }
    // Reset event flag
    eventFlag = 0;
  }
  return returnStatus;
}
