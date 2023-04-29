#include "ServerClient.h"
#include "Display.h"
#include "DatetimeClient.h"
#include "Buttons.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>

// FSM states enum
typedef enum {
  Idle,
  SelectMsg,
  PingServer,
  Offline,
  SendMsg0,
  UpdateTime,
  LED,
  UpdateSleep,
  Error
} FSMState_t;

/*
   -----------------------------------------------------------------------------------
   settings
   -----------------------------------------------------------------------------------
*/

// Internet and Proxy Server settings
String domain = "https://iotserver.mattlemcke.repl.co";
String token = "##";
const char* ssid = "##";
const char* password = "##";

int serverCode;
ProxyServerClient server = ProxyServerClient(domain, token);

// Date and Time settings
const Date_t startDate = {16, 3, 2020};
Date_t currDate, newDate;
Time_t currTime;

DatetimeClient datetime = DatetimeClient(-4); // EST timezone = GMT - 4 hours

// Deep sleep settings
Time_t sleepStart = {0, 0, 0};
unsigned int sleepDurationSec = 0;

// FSM Settings
FSMState_t currState = Idle, nextState = Idle, prevState = Idle;

// Hardware settings
// I2C Pins
#define ClkPin D3
#define DioPin D2

// LED Pins
#define StatusPin D6
#define HeartUpper1Pin D4
#define HeartUpper2Pin D5
#define HeartLowerPin D8

// Button Pin
#define ButtonPin D1

SegmentDisplay segDisplay = SegmentDisplay(ClkPin, DioPin);
Button userButton = Button(ButtonPin);
int buttonStatus = NONE;

// Timers
Ticker serverTimer, datetimeTimer, pollButtonTimer;
const unsigned int SERVER_TIMER_PERIOD = 5000;    // Ping proxy server every 5s
const unsigned int DATETIME_TIMER_PERIOD = 20000; // Check datetime server every 20s
const unsigned int POLLBUTTON_TIMER_PERIOD = 10;  // Check button state ever 10ms
volatile bool serverTimerFlag = 0, datetimeTimerFlag = 0;

unsigned int msgSelectIndex = 0;

/*
   -----------------------------------------------------------------------------------
   interrupt service routines
   -----------------------------------------------------------------------------------
*/

IRAM_ATTR void ButtonTimerCallback() {
  // Checks button state and raises event after debouncing
  userButton.debounce();
}

IRAM_ATTR void ServerTimerIntCallback() {
  serverTimerFlag = 1;
}

IRAM_ATTR void DatetimeTimerIntCallback() {
  datetimeTimerFlag = 1;
}

/*
   -----------------------------------------------------------------------------------
   application program
   -----------------------------------------------------------------------------------
*/
void enterDeepSleep(unsigned int durationSec);
void updateSleepParameters(void);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Setup begin");

  // Configure GPIO
  pinMode(StatusPin, OUTPUT);
  pinMode(ButtonPin, INPUT);
  pinMode(HeartUpper1Pin, OUTPUT);
  pinMode(HeartUpper2Pin, OUTPUT);
  pinMode(HeartLowerPin, OUTPUT);

  segDisplay.setup();

  // Connect to internet and proxy server
  digitalWrite(StatusPin, HIGH);
  server.connectWifi(ssid, password);
  while (!server.wifiConnected());
  digitalWrite(StatusPin, LOW);

  datetime.setup();
  currDate = datetime.getDate();
  currTime = datetime.getTime();
  updateSleepParameters();

  // Signal that startup was complete
  for (int i = 0; i < 3; i++) {
    digitalWrite(StatusPin, HIGH);
    delay(250);
    digitalWrite(StatusPin, LOW);
    delay(250);
  }

  segDisplay.displayDays(getDifference(startDate, currDate));

  // Attach interrupt to timers
  serverTimer.attach_ms(SERVER_TIMER_PERIOD, ServerTimerIntCallback);
  datetimeTimer.attach_ms(DATETIME_TIMER_PERIOD, DatetimeTimerIntCallback);
  pollButtonTimer.attach_ms(POLLBUTTON_TIMER_PERIOD, ButtonTimerCallback);

  Serial.println("Setup finished");
  Serial.println("FSM: Idle");
}

void loop() {

  // Move FMS states
  if (nextState != currState) {
    prevState = currState;
    currState = nextState;
  }

  // Check for internet connection
  if (!server.wifiConnected()) {
    currState = Offline;
  }

  // Only check for button events during Idle and SelectMsg states
  if(currState == Idle || currState == SelectMsg){
    buttonStatus = userButton.getStatus();
  }

  // FSM
  switch (currState) {

    // Idle state
    case Idle:
      if (buttonStatus == LONG_PRESS) {
        // Move to select message state
        Serial.println("FSM: SelectMsg");
        msgSelectIndex = 0;
        segDisplay.displayMessage(msgSelectIndex);
        nextState = SelectMsg;
      }
      else if (buttonStatus == SHORT_PRESS) {
        // Move to send message 0 state
        nextState = SendMsg0;
      }
      else if (datetimeTimerFlag) {
        // Move to update time state after timer event
        datetimeTimerFlag = 0;
        nextState = UpdateTime;
      }
      else if (serverTimerFlag) {
        // Move to ping server state after timer event
        serverTimerFlag = 0;
        nextState = PingServer;
      }
      break;
      
    // Ping server state
    case PingServer:
      Serial.println("FSM: PingServer");
      
      // Read if a flag is set on the server
      serverCode = server.checkFlag();
      if (serverCode == LED_CODE) {
        // Move to LED state
        nextState = LED;
      }
      else if (serverCode == RESET_CODE) {
        // Reset the device by entering deepsleep for 1s
        enterDeepSleep(1);
      }
      else if (serverCode == SLEEP_UPDATE_CODE) {
        // Move to update sleep state
        nextState = UpdateSleep;
      }
      else if (serverCode == ERROR_CODE && server.isUnresponsive()) {
        // Move to offline state if server doesn't respond or throws error code
        nextState = Offline;
      }
      // No flag set
      else{
        // Return to Idle state
        nextState = Idle;
        Serial.println("FSM: Idle");
      }
      break;

    // Update sleep state
    case UpdateSleep:
      // Update the local sleep start time and duration parameters set by the server
      Serial.println("FSM: UpdateSleep");
      updateSleepParameters();

      // Return to Idle state
      nextState = Idle;
      Serial.println("FSM: Idle");
      break;

    // LED state
    case LED:
      // Start the LED animation on the box
      Serial.println("FSM: LED");
      heartLED();

      // Return to Idle state
      nextState = Idle;
      Serial.println("FSM: Idle");
      break;
      
    case SendMsg0:
      // Send message 0 to the server
      Serial.println("FSM: SendMsg0");
      server.sendMessage("i-love-you");

      // Return to Idle state
      nextState = Idle;
      Serial.println("FSM: Idle");
      break;

    // Select message state
    case SelectMsg:
      if (buttonStatus == LONG_PRESS) {
        // Send the selected message to the server and return to displaying the days counter
        Serial.println("FSM: SelectMsg Send i");
        String msg = getNotifMessage(msgSelectIndex);
        segDisplay.displayDays(getDifference(startDate, currDate));
        server.sendMessage(msg);

        // Move to update time state
        nextState = UpdateTime;
      }
      else if (buttonStatus == SHORT_PRESS) {
        // Change selected message to the next in the list
        Serial.println("FSM: SelectMsg i++");
        msgSelectIndex = (msgSelectIndex + 1) % NUM_MESSAGES;
        segDisplay.displayMessage(msgSelectIndex);
      }
      break;

    // Update time state
    case UpdateTime:
      // Get the date and time from NTP client
      Serial.println("FSM: UpdateTime");
      currTime = datetime.getTime();
      newDate = datetime.getDate();
      if (newDate.d != currDate.d){
        // Only update the display if the days have increased
        currDate = newDate;
        segDisplay.displayDays(getDifference(startDate, currDate));
      }
      if (currTime.h == sleepStart.h && currTime.m == sleepStart.m) {
        // Enter deepsleep when the current time equals the start time
        Serial.println("FSM: DeepSleep");
        enterDeepSleep(sleepDurationSec);
      }
      if (getDifference(startDate, currDate) >= 10000) {
        // Alert of error if days counter exceed 4 digits
        server.sendMessage("days-exceed-9999");
        Serial.println("Days exceed digits on display");

        // Enter error state
        nextState = Error;
      }
      else{
        // Return to Idle state
        Serial.println("FSM: Idle");
        nextState = Idle;
      }
      break;

    // Offline state
    case Offline:
      digitalWrite(StatusPin, HIGH);
      Serial.println("Box is offline");
      
      // Wait device to reconnect to internet
      while (!server.wifiConnected()) delay(500);
      Serial.println("Wifi Connected");

      // Wait for server to respond
      while (server.isUnresponsive()){
        delay(3000);
        server.checkFlag();
      }
      
      // Server is back online
      Serial.println("Server responding");
      Serial.println("Box is back online");
      digitalWrite(StatusPin, LOW);

      // Return to Idle state
      nextState = Idle;
      Serial.println("FSM: Idle");
      break;

    // Error state
    default:
      // Clear display, alert the server, and wait for a manual reset
      segDisplay.displayError();
      server.sendMessage("state-error");
      Serial.println("FSM state error");
      while (1);
      break;
   
  }
}

/*
   -----------------------------------------------------------------------------------
   helper functions
   -----------------------------------------------------------------------------------
*/

// Force device into deep sleep mode
void enterDeepSleep(unsigned int durationSec) {
  if (durationSec) {
    unsigned long durationUsec = durationSec * 1e6;
    segDisplay.clear();
    ESP.deepSleep(durationUsec);
  }
}

// Update sleep start time and duration locally from server values
void updateSleepParameters(void) {
  sleepStart.h = server.getSleepStart();
  sleepDurationSec = server.getSleepTime() * 60;
}

// Heart LED animation
void heartLED(void) {
  for (int i = 0; i < 6; i++) {
    digitalWrite(HeartLowerPin, HIGH);
    digitalWrite(HeartUpper1Pin, LOW);
    digitalWrite(HeartUpper2Pin, LOW);
    delay(500);
    digitalWrite(HeartLowerPin, LOW);
    digitalWrite(HeartUpper1Pin, HIGH);
    digitalWrite(HeartUpper2Pin, HIGH);
    delay(500);
  }
  digitalWrite(HeartUpper1Pin, LOW);
  digitalWrite(HeartUpper2Pin, LOW);
  delay(100);
  digitalWrite(HeartLowerPin, HIGH);
  digitalWrite(HeartUpper1Pin, HIGH);
  digitalWrite(HeartUpper2Pin, HIGH);
  delay(5000);
  digitalWrite(HeartLowerPin, LOW);
  digitalWrite(HeartUpper1Pin, LOW);
  digitalWrite(HeartUpper2Pin, LOW);
}
