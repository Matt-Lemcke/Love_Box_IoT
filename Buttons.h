#ifndef BUTTONS_H
#define BUTTONS_H
#include <Arduino.h>

#define NONE 0
#define PRESSING 1
#define LONG_PRESS 2
#define SHORT_PRESS 3

class Button{
  unsigned char buttonPin;
  unsigned char stateSequence = 0;
  bool eventFlag = 0;
  unsigned long lastPress = 0;
  unsigned long lastRelease = 0;

public:
  Button(unsigned char pin);
  int getStatus(void);
  void debounce(void);
};


#endif
