#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <String.h>
#include "WiFiManager.h"
#include "hw.h"
#include "network.h"

WiFiManager wm;                         /**< Wi-Fi manager object. */
WiFiEventHandler wifiConnectHandler;    /**< Handling Wi-Fi connect event. */
WiFiEventHandler wifiDisconnectHandler; /**< Handling Wi-Fi disconnect event. */

const char *hostname = DEVICE_NAME; /**< The device's hostname. */

void OTA_init()
{
   ArduinoOTA.setHostname(hostname);
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
                  Restart_device(); });
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
                   Restart_device(); });

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
   Serial.printf("Network: Connected to Wi-Fi as %s, IP: %s\n", hostname, WiFi.localIP().toString().c_str());
#endif
   wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
   MDNS.begin(hostname);
   OTA_init();
   wm.stopWebPortal();
   Webserver_start();
   Set_clock_state(CLOCK_STATE_IP);
   Mqtt_init();
   Mqtt_connect();
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
   wm.setDebugOutput(false);
   wm.setConfigPortalBlocking(false);
   wm.setDisableConfigPortal(false);
   WiFi.setAutoReconnect(true);
   wm.setHostname(hostname);
   wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
   if (!wm.autoConnect(hostname))
   {
      Set_clock_state(CLOCK_STATE_AP);
#ifdef DEBUG
      Serial.println("Network: Start Configportal");
#endif
   }
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
   Set_clock_state(CLOCK_STATE_START);
   wifiDisconnectHandler = NULL;
   ESP.eraseConfig();
}

String Get_wifi_ssid()
{
   return WiFi.SSID();
}

void Set_wifi_credentials(String ssid, String pwd)
{
#ifdef DEBUG
   Serial.printf("Network: Connecting to new Wi-Fi: %s\n", ssid.c_str());
#endif
}

void Disable_WifiDisconnectHandler()
{
   wifiDisconnectHandler = NULL;
}

void Network_100ms_task()
{
   wm.process();
   ArduinoOTA.handle();
   Mqtt_100ms_task();
}

void Network_1000ms_task()
{
   Clock_task_1000ms();
}