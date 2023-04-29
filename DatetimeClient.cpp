#include "DatetimeClient.h"

// Counts number of leap years before the given date
int countLeapYears(Date d)
{
  int years = d.y;

  if (d.m <= 2) {
    years--;
  }
  return years / 4 - years / 100 + years / 400;
}

// Returns number of days between two given dates
int getDifference(Date_t dt1, Date_t dt2)
{
  const int monthDays[12] = {31, 28, 31, 30, 31, 30,
                             31, 31, 30, 31, 30, 31
                            };

  // COUNT TOTAL NUMBER OF DAYS BEFORE FIRST DATE 'dt1'
  long int n1 = dt1.y * 365 + dt1.d;
  for (int i = 0; i < dt1.m - 1; i++) {
    n1 += monthDays[i];
  }
  n1 += countLeapYears(dt1);

  // COUNT TOTAL NUMBER OF DAYS BEFORE 'dt2'

  long int n2 = dt2.y * 365 + dt2.d;
  for (int i = 0; i < dt2.m - 1; i++) {
    n2 += monthDays[i];
  }
  n2 += countLeapYears(dt2);

  // return difference between two counts
  return (n2 - n1);
}

DatetimeClient::DatetimeClient(int utcOffset): timeClient(ntpUDP, "pool.ntp.org", 3600 * utcOffset) {
}

// Setup the NTP client
void DatetimeClient::setup(void) {
  timeClient.begin();
  timeClient.update();
}

// Get the current date from NTP client
Date_t DatetimeClient::getDate(void) {
  timeClient.update();
  Date d;
  String formattedDate = timeClient.getFormattedDate();

  // Format the date info
  int splitT = formattedDate.indexOf("T");
  String dayStamp = formattedDate.substring(0, splitT);
  d.y = dayStamp.substring(0, 4).toInt();
  d.m = dayStamp.substring(5, 7).toInt();
  d.d = dayStamp.substring(8, 10).toInt();
  return d;
}

// Get the current time (24 hour clock) from NTP client
Time_t DatetimeClient::getTime(void) {
  timeClient.update();
  Time t;
  String formattedTime = timeClient.getFormattedTime();

  // Format the time info
  t.h = formattedTime.substring(0, 2).toInt();
  t.m = formattedTime.substring(3, 5).toInt();
  t.s = formattedTime.substring(6, 8).toInt();
  return t;
}
