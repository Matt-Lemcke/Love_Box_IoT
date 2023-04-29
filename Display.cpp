#include "Display.h"

// 7-segment representation for various letters
#define DISP_M SEG_E|SEG_G|SEG_C|SEG_A
#define DISP_O SEG_E|SEG_G|SEG_C|SEG_D
#define DISP_R SEG_E|SEG_G
#define DISP_N SEG_E|SEG_G|SEG_C
#define DISP_I SEG_C
#define DISP_T SEG_F|SEG_E|SEG_D|SEG_G
#define DISP_E SEG_A|SEG_F|SEG_G|SEG_E|SEG_D
#define DISP_H SEG_F|SEG_E|SEG_G|SEG_C|SEG_B
#define DISP_G SEG_A|SEG_B|SEG_C|SEG_D|SEG_G|SEG_F
#define DISP_S SEG_A|SEG_F|SEG_G|SEG_C|SEG_D
#define DISP_A SEG_A|SEG_B|SEG_C|SEG_F|SEG_E|SEG_G
#define DISP_D SEG_B|SEG_G|SEG_E|SEG_D|SEG_C
#define DISP_none 0x00

// Messages to send as notifications
String notifMessages[] = {"good-morning", "good-night", "i-am-hungry", "i-am-sad"};

// 4 letter abriviations to display during message select menu
const uint8_t disp_message[][4] = {
  {DISP_M, DISP_O, DISP_R, DISP_N}, // Good morning
  {DISP_N, DISP_I, DISP_T, DISP_E}, // Good night
  {DISP_H, DISP_N, DISP_G, DISP_R}, // I am hungry
  {DISP_S, DISP_A, DISP_D, DISP_none} // I am sad
};

// Array that shows error message
const uint8_t error[] = {
  SEG_G,
  SEG_G,
  SEG_G,
  SEG_G
};

// Returns notification message string at the given list index
String getNotifMessage(unsigned int index) {
  if(index < NUM_MESSAGES){
    return notifMessages[index];
  }
  return "";
}

SegmentDisplay::SegmentDisplay(unsigned char clk, unsigned char dio): display(clk, dio) {
}

// Setup display settings
void SegmentDisplay::setup(void) {
  // Clear the display and set max brightness
  display.clear();
  display.setBrightness(7);
}

// Display the number given
bool SegmentDisplay::displayDays(unsigned int days) {
  display.clear();
  if (days > 10000) {
    Serial.println("Days exceed display length");
    display.setSegments(error);
    return 0;
  }
  else {
    Serial.printf("\n%d days\n", days);
    display.showNumberDec(days);
    return 1;
  }
}

// Display message abriviation at the given index
void SegmentDisplay::displayMessage(unsigned int index) {
  if(index < NUM_MESSAGES){
    display.clear();
    display.setSegments(disp_message[index]);
  }
}

// Display the error message
void SegmentDisplay::displayError(void) {
  display.clear();
  display.setSegments(error);
}

// Clear the display
void SegmentDisplay::clear(void) {
  display.clear();
}
