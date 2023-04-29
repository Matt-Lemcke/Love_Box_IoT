#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H
#include <Arduino.h>

#define ERROR_CODE -1
#define NULL_CODE 0
#define LED_CODE 1
#define RESET_CODE 2
#define SLEEP_UPDATE_CODE 3

class ProxyServerClient {

    bool unresponsive = 0;
    String serverDomain, authToken;
    String getUrl(String endpoint);
    String getRequest(String endpoint);

  public:

    ProxyServerClient(String domain, String token);
    void connectWifi(const char* ssid, const char* password);
    int checkFlag(void);
    bool clearFlag(void);
    unsigned int getSleepStart(void);
    unsigned int getSleepTime(void);
    bool sendMessage(String msg);
    bool wifiConnected(void);
    bool isUnresponsive(void);
};

#endif
