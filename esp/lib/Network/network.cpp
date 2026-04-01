#include "network.h"

#include <ArduinoOTA.h>
#include <LittleFS.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#endif
#include <DNSServer.h>
#include "Ticker.h"
#include "clock.h"
#include "hw.h"
#include "memory.h"
#include "mqtt.h"
#include "webserver.h"

#ifdef ESP32
WiFiEventId_t wifiConnectHandler;
WiFiEventId_t wifiDisconnectHandler;
#else
WiFiEventHandler wifiConnectHandler;    /**< Handling Wi-Fi connect event. */
WiFiEventHandler wifiDisconnectHandler; /**< Handling Wi-Fi disconnect event. */
#endif

Ticker network_reset_ticker([]()
                            {
#ifdef ESP32
                               wifiDisconnectHandler = 0;
                               WiFi.disconnect(true, true);
#else
                               wifiDisconnectHandler = nullptr;
                               ESP.eraseConfig();
#endif
                            },
                            2000, 1);

String wifi_status = "Not connected.";
boolean was_connected_b = false; /**< This will prevent to create an AP, if the connection is lost during runtime. */

DNSServer dnsServer;
IPAddress apIP(4, 3, 2, 1);
IPAddress netMsk(255, 255, 255, 0);

static const char *Get_device_hostname()
{
   static char hostname[32];
   static boolean initialized = false;
   if (!initialized)
   {
#ifdef ESP32
      snprintf(hostname, sizeof(hostname), "%s-%u", DEVICE_NAME, (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF));
#else
      snprintf(hostname, sizeof(hostname), "%s-%u", DEVICE_NAME, ESP.getChipId());
#endif
      initialized = true;
   }
   return hostname;
}

void Network_create_AP();
void Network_start_MDNS();

Ticker create_ap_ticker(Network_create_AP, 10000, 1);

void OTA_init()
{
   ArduinoOTA.setHostname(DEVICE_NAME);
   ArduinoOTA.onStart([]()
                      { DEBUG_PRINTF("Network: Start OTA updating %s\n",
                           (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
                        Set_clock_state(CLOCK_STATE_OTA);
#ifdef ESP32
                         if (ArduinoOTA.getCommand() == U_SPIFFS)
#else
                         if (ArduinoOTA.getCommand() == U_FS)
#endif
                            LittleFS.end(); });

   /* Make a clean restart to indicate the update was successful */
   ArduinoOTA.onEnd([]()
                    { DEBUG_PRINTLN("Network: OTA updating ended"); });

   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                         { DEBUG_PRINTF("Network: OTA update progress: %u%%\r", (progress / (total / 100))); });

   ArduinoOTA.onError([](ota_error_t error)
                      { DEBUG_PRINTF("Network: OTA error[%u]: ", error);
                        if (error == OTA_AUTH_ERROR)
                           DEBUG_PRINTLN("Auth Failed");
                        else if (error == OTA_BEGIN_ERROR)
                           DEBUG_PRINTLN("Begin Failed");
                        else if (error == OTA_CONNECT_ERROR)
                           DEBUG_PRINTLN("Connect Failed");
                        else if (error == OTA_RECEIVE_ERROR)
                           DEBUG_PRINTLN("Receive Failed");
                        else if (error == OTA_END_ERROR)
                           DEBUG_PRINTLN("End Failed"); });

   ArduinoOTA.begin();
   DEBUG_PRINTLN("Network: OTA ready");
}

void onWifiDisconnect(
#ifdef ESP32
    WiFiEvent_t event, WiFiEventInfo_t info
#else
    const WiFiEventStationModeDisconnected &event
#endif
)
{
   DEBUG_PRINTLN("Network: Disconnected from Wi-Fi");
   wifi_status = "Disconnected from Wi-Fi";
   Set_clock_state(CLOCK_STATE_SERVER_DOWN);
}

void onWifiConnect(
#ifdef ESP32
    WiFiEvent_t event, WiFiEventInfo_t info
#else
    const WiFiEventStationModeGotIP &event
#endif
)
{
   DEBUG_PRINTF("Network: Connected to Wi-Fi as %s, IP: %s\n", WiFi.getHostname(), WiFi.localIP().toString().c_str());
   create_ap_ticker.stop();
   wifi_status = "Connected to " + WiFi.SSID();
   WiFi.setAutoReconnect(true);
   Network_start_MDNS();
   if (!was_connected_b)
   {
      was_connected_b = true;
#ifdef ESP32
      wifiDisconnectHandler = WiFi.onEvent(onWifiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#else
      wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
#endif
      OTA_init();
      Webserver_start();
   }
   Mqtt_connect();
   Set_clock_state(CLOCK_STATE_IP);
   /* Force set to NTP synchronization, for better experience. */
   Set_manual_mode(false);
}

void Network_create_AP()
{
   wifi_status = "AP Mode";
   Set_clock_state(CLOCK_STATE_AP);
   /* Force set to manual mode, for better experience. */
   Set_manual_mode(true);
   WiFi.disconnect(); // Stop trying to connect to the WiFi.
   WiFi.softAPConfig(apIP, apIP, netMsk);
   DEBUG_PRINTF("Creating AP, with IP: %s\n", WiFi.softAPIP().toString().c_str());
   WiFi.softAP(Get_device_hostname(), "12345678");
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
   Network_start_MDNS();
   OTA_init();
   Webserver_start();
}

void Network_start_MDNS()
{
   if (!MDNS.begin(DEVICE_NAME))
   {
      DEBUG_PRINTLN("Network: MDNS begin failed!");
      return;
   }
   DEBUG_PRINTLN("Network: MDNS responder started!");
   MDNS.addService("_http", "_tcp", 80);
}

void Network_init()
{
   Clock_init();
   Mqtt_init();
   WiFi.setHostname(DEVICE_NAME);
   WiFi.mode(WIFI_STA);
#ifdef ESP32
   wifiConnectHandler = WiFi.onEvent(onWifiConnect, ARDUINO_EVENT_WIFI_STA_GOT_IP);
#else
   wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
#endif
   WiFi.begin();
}

void Network_reset()
{
   DEBUG_PRINTLN("\nNetwork: Resetting Wifi\n");
   network_reset_ticker.start();
}

String Get_wifi_status()
{
   return wifi_status;
}
String Get_wifi_ssid()
{
   return WiFi.SSID();
}
String Get_wifi_ip_address()
{
   if (WiFi.status() == WL_CONNECTED)
      return WiFi.localIP().toString();
   else if (WiFi.getMode() == WIFI_AP_STA)
      return WiFi.softAPIP().toString();
   return "";
}

void Set_wifi_credentials(const char *ssid, const char *pwd)
{
   DEBUG_PRINTF("Network: New Wi-Fi saved: %s\n", ssid);
#ifdef ESP32
   wifiDisconnectHandler = 0;
#else
   wifiDisconnectHandler = nullptr;
#endif
#ifdef ESP32
   WiFi.begin(ssid, pwd);
#else
   struct station_config conf;
   memset(&conf, 0, sizeof(conf));
   memcpy(reinterpret_cast<char *>(conf.ssid), ssid, min(strlen(ssid), sizeof(conf.ssid) - 1));
   memcpy(reinterpret_cast<char *>(conf.password), pwd, min(strlen(pwd), sizeof(conf.password) - 1));
   wifi_station_set_config(&conf);
#endif
}

void Disable_WifiDisconnectHandler()
{
#ifdef ESP32
   wifiDisconnectHandler = 0;
#else
   wifiDisconnectHandler = nullptr;
#endif
}

void Network_100ms_task()
{
   create_ap_ticker.update();
   network_reset_ticker.update();

   if (!was_connected_b && WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP_STA && create_ap_ticker.state() != RUNNING)
      create_ap_ticker.start();
   else if (WiFi.getMode() == WIFI_AP_STA)
      dnsServer.processNextRequest();

   ArduinoOTA.handle();
#ifndef ESP32
   MDNS.update();
#endif
   Mqtt_100ms_task();
}

void Network_1000ms_task()
{
   Clock_task_1000ms();
}