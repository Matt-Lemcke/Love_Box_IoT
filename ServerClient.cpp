#include "ServerClient.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// Proxy server API endpoints
String apiPath = "/api/key/";
String getFlagEndpoint = "/box/alert-pending";
String clearFlagEndpoint = "/box/clear-alert";
String sleepStartEndpoint = "/box/get-sleep-start";
String sleepTimeEndpoint = "/box/get-sleep-time";
String sendMessageEndpoint = "/box/send-message/";

ESP8266WiFiMulti WifiMulti;

// Parse the results from the raw text of HTTPS response
int parseResult(String getResponse) {
  int result = ERROR_CODE;
  int commaIndex = getResponse.indexOf(',');
  int endBracketIndex = getResponse.indexOf(']');
  if (commaIndex != -1 && endBracketIndex != -1) {
    int sum = 0;
    for (int i = commaIndex + 1; i < endBracketIndex; i++) {
      sum *= 10;
      sum += int(getResponse.charAt(i) - '0');
    }
    result = sum;
  }
  return result;
}

ProxyServerClient::ProxyServerClient(String domain, String token) {
  serverDomain = domain;
  authToken = token;
}

// Get the full URL string for the given endpoint
String ProxyServerClient::getUrl(String endpoint) {
  String result = serverDomain + apiPath + authToken + endpoint;
  return result;
}

// Send GET request to the proxy server at the endpoint and return the payload
String ProxyServerClient::getRequest(String endpoint) {

  // Get the full URL
  String url = ProxyServerClient::getUrl(endpoint);
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String payload = "";
  if (https.begin(*client, url)) {
    int httpsCode = https.GET();
    if (httpsCode > 0) {
      if (httpsCode == HTTP_CODE_OK || httpsCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
      }
    }
    else {
      Serial.printf("HTTPS GET failed, error: %s\n", https.errorToString(httpsCode).c_str());
    }
  }
  else {
    Serial.println("HTTPS unable to connect");
  }
  // Check if payload is empty
  if (payload != "") {
    unresponsive = 0;
  }
  else {
    // Server is unresponsive
    unresponsive = 1;
  }
  return payload;
}

// Connect to the internet
void ProxyServerClient::connectWifi(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WifiMulti.addAP(ssid, password);
}

// Check if device is still connected to internet
bool ProxyServerClient::wifiConnected(void) {
  return WifiMulti.run() == WL_CONNECTED;
}

// Check if server was responsive after the last GET request
bool ProxyServerClient::isUnresponsive(void) {
  if (!ProxyServerClient::wifiConnected()) {
    return 1;
  }
  return unresponsive;
}

// Check the status flag on the proxy server
int ProxyServerClient::checkFlag(void) {
  if (!ProxyServerClient::wifiConnected()) {
    return ERROR_CODE;
  }

  // Check the flag endpoint
  String response = ProxyServerClient::getRequest(getFlagEndpoint);

  // Parse flag code from results
  int flagCode = parseResult(response);

  // Clear the flag if it was set
  if(flagCode != NULL_CODE){
    ProxyServerClient::clearFlag();
  }
  Serial.printf("\nFlag code: %d\n", flagCode);
  return flagCode;
}

// Clear the status flag on the proxy server
bool ProxyServerClient::clearFlag(void) {
  if (!ProxyServerClient::wifiConnected()) {
    return 0;
  }

  // Send request to the clear flag endpoint
  String response = ProxyServerClient::getRequest(clearFlagEndpoint);
  Serial.println("Flag cleared");
  return 1;
}

// Get the sleep start parameter on the server database
unsigned int ProxyServerClient::getSleepStart(void) {
  if (!ProxyServerClient::wifiConnected()) {
    return 0;
  }

  // Send request to the sleep start endpoint
  String response = ProxyServerClient::getRequest(sleepStartEndpoint);

  // Parse the results
  int sleepStart = parseResult(response);
  Serial.printf("\nUpdated sleep start: %d:00\n", sleepStart);
  return sleepStart;
}

// Get the sleep duration parameter on the server database
unsigned int ProxyServerClient::getSleepTime(void) {
  if (!ProxyServerClient::wifiConnected()) {
    return 0;
  }

  // Send request to the sleep time endpoint
  String response = ProxyServerClient::getRequest(sleepTimeEndpoint);

  // Parse the results
  int sleepTime = parseResult(response);
  Serial.printf("\nUpdated sleep time: %d minutes\n", sleepTime);
  return sleepTime;
}

// Send a notification message to the server
bool ProxyServerClient::sendMessage(String msg) {
  if (!ProxyServerClient::wifiConnected()) {
    return 0;
  }

  // Send request to send message endpoint with message parameter
  String response = ProxyServerClient::getRequest(sendMessageEndpoint + msg);
  if (response == "") {
    return 0;
  }
  Serial.printf("\nSent message: %s\n", msg);
  return 1;
}
