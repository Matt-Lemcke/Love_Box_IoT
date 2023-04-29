#ifndef DATETIME_CLIENT_H
#define DATETIME_CLIENT_H
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

typedef struct Time {
  int h, m, s;
} Time_t;

typedef struct Date {
  int d, m, y;
} Date_t;

int getDifference(Date_t dt1, Date_t dt2);
int countLeadYears(Date_t d);

class DatetimeClient {
    NTPClient timeClient;
    WiFiUDP ntpUDP;
  public:
    DatetimeClient(int utcOffset);
    void setup(void);
    Date_t getDate();
    Time_t getTime();
};

#endif
