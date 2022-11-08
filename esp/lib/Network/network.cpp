#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <String.h>
#include <user_interface.h>
#include "Ticker.h"
#include "hw.h"
#include "memory.h"
#include "network.h"

WiFiEventHandler wifiConnectHandler;    /**< Handling Wi-Fi connect event. */
WiFiEventHandler wifiDisconnectHandler; /**< Handling Wi-Fi disconnect event. */

String wifi_status = "Not connected.";

void Network_create_AP();

Ticker create_ap_ticker(Network_create_AP, 6000, 1);

void OTA_init()
{
   ArduinoOTA.setHostname(DEVICE_NAME);
   ArduinoOTA.onStart([]()
                      {
#ifdef DEBUG
                         Serial.printf("Network: Start OTA updating %s\n",
                                       (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
#endif
                         if (ArduinoOTA.getCommand() == U_FS)
                            LittleFS.end(); });

   ArduinoOTA.onEnd([]()
                    {
#ifdef DEBUG
                     Serial.println("Network: OTA updating ended");
#endif 
                  Restart_device(true); });
#ifdef DEBUG
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                         { Serial.printf("Network: OTA update progress: %u%%\r", (progress / (total / 100))); });
#endif
   ArduinoOTA.onError([](ota_error_t error)
                      {
#ifdef DEBUG
      Serial.printf("Network: OTA error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)
         Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR)
         Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR)
         Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR)
         Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR)
         Serial.println("End Failed");
#endif
                   Restart_device(true); });

   ArduinoOTA.begin();
#ifdef DEBUG
   Serial.println("Network: OTA ready");
#endif
}

/**
 * @brief WiFi disconnect callback function.
 *
 * Currently used only for debugging.
 *
 */
void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
#ifdef DEBUG
   Serial.println("Network: Disconnected from Wi-Fi.");
#endif
   wifi_status = "Disconnected from " + WiFi.SSID();
   Set_clock_state(CLOCK_STATE_SERVER_DOWN);
}

/**
 * @brief WiFi connect callback function.
 *
 * After connecting to the WiFi, the WiFi Manager's webserver has to be stopped,
 * the device's webserver is started instead, and the Mqtt submodule is initialized.
 *
 */
void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
#ifdef DEBUG
   Serial.printf("Network: Connected to Wi-Fi as %s, IP: %s\n", DEVICE_NAME, WiFi.localIP().toString().c_str());
#endif
   create_ap_ticker.stop();
   wifi_status = "Connected to " + WiFi.SSID();
   WiFi.setAutoReconnect(true);
   wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
   MDNS.notifyAPChange();
   OTA_init();
   Webserver_start();
   Set_clock_state(CLOCK_STATE_IP);
   Mqtt_init();
   Mqtt_connect();
}

void Network_create_AP()
{
   wifi_status = "AP Mode";
   Set_clock_state(CLOCK_STATE_AP);
   WiFi.softAP(DEVICE_NAME);
   Webserver_start();
}

/**
 * @brief Initializing Network components.
 *
 * The function connects the device to the Wi-Fi network, and prepares the own webserver
 * Connecting to the MQTT server is done through the Wi-Fi connected callback function.
 *
 */
void Network_init()
{
   Webserver_init();
   Clock_init();
   WiFi.mode(WIFI_STA);
   WiFi.setHostname(DEVICE_NAME);
   wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
   MDNS.begin(DEVICE_NAME);
   WiFi.begin();
}

/**
 * @brief Resetting Network components
 *
 * The function erases all the Wi-Fi credentials, and restarts the device.
 *
 */
void Network_reset()
{
#ifdef DEBUG
   Serial.println("\nNetwork: Resetting Wifi\n");
#endif
   wifiDisconnectHandler = NULL;
   ESP.eraseConfig();
}

String Get_wifi_status()
{
   return wifi_status;
}
String Get_wifi_ssid()
{
   return WiFi.SSID();
}

void Set_wifi_credentials(String ssid, String pwd)
{
#ifdef DEBUG
   Serial.printf("Network: New Wi-Fi saved: %s\n", ssid.c_str());
#endif
   wifiDisconnectHandler = NULL;
   struct station_config conf;
   memcpy(reinterpret_cast<char *>(conf.ssid), ssid.c_str(), 32);
   memcpy(reinterpret_cast<char *>(conf.password), pwd.c_str(), 64);
   wifi_station_set_config(&conf);
}

void Disable_WifiDisconnectHandler()
{
   wifiDisconnectHandler = NULL;
}

void Network_100ms_task()
{
   create_ap_ticker.update();

   if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP_STA && create_ap_ticker.state() != RUNNING)
      create_ap_ticker.start();

   ArduinoOTA.handle();
   MDNS.update();
   Mqtt_100ms_task();
}

void Network_1000ms_task()
{
   Clock_task_1000ms();
}