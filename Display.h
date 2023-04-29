#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <TM1637Display.h>

#define NUM_MESSAGES 4

String getNotifMessage(unsigned int index);

class SegmentDisplay {
    TM1637Display display;
  public:
    SegmentDisplay(unsigned char clk, unsigned char dio);
    void setup(void);
    bool displayDays(unsigned int days);
    void displayMessage(unsigned int index);
    void displayError(void);
    void clear(void);
};

#endif
