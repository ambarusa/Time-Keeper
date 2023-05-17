#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <String.h>
#include <ESP8266mDNS.h>
#include <user_interface.h>
#include <DNSServer.h>
#include "Ticker.h"
#include "hw.h"
#include "memory.h"
#include "network.h"

WiFiEventHandler wifiConnectHandler;    /**< Handling Wi-Fi connect event. */
WiFiEventHandler wifiDisconnectHandler; /**< Handling Wi-Fi disconnect event. */

String wifi_status = "Not connected.";
boolean was_connected_b = false; /**< This will prevent to create an AP, if the connection is lost during runtime. */

DNSServer dnsServer;
IPAddress apIP(4, 3, 2, 1);
IPAddress netMsk(255, 255, 255, 0);

void Network_create_AP();
void Network_start_MDNS();

Ticker create_ap_ticker(Network_create_AP, 10000, 1);

void OTA_init()
{
   ArduinoOTA.setHostname(DEVICE_NAME);
   ArduinoOTA.onStart([]()
                      { DEBUG_PRINTF("Network: Start OTA updating %s\n",
                           (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
                         if (ArduinoOTA.getCommand() == U_FS)
                            LittleFS.end(); });

   /* Make a clean restart to indicate the update was successful */
   ArduinoOTA.onEnd([]()
                    { DEBUG_PRINTLN("Network: OTA updating ended");
                     Restart_device(false); });

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
                           DEBUG_PRINTLN("End Failed");
                        Restart_device(true); });

   ArduinoOTA.begin();
   DEBUG_PRINTLN("Network: OTA ready");
}

/**
 * @brief WiFi disconnect callback function.
 *
 * Currently used only for debugging.
 *
 */
void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
   DEBUG_PRINTLN("Network: Disconnected from Wi-Fi.");
   wifi_status = "Disconnected from Wi-Fi.";
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
   DEBUG_PRINTF("Network: Connected to Wi-Fi as %s, IP: %s\n", DEVICE_NAME, WiFi.localIP().toString().c_str());
   create_ap_ticker.stop();
   was_connected_b = true;
   wifi_status = "Connected to " + WiFi.SSID();
   WiFi.setAutoReconnect(true);
   Network_start_MDNS();
   OTA_init();
   wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
   Webserver_start();
   Mqtt_connect();
   Set_clock_state(CLOCK_STATE_IP);
   Set_manual_mode(false);
}

void Network_create_AP()
{
   wifi_status = "AP Mode";
   Set_clock_state(CLOCK_STATE_AP);
   Set_manual_mode(true);
   WiFi.disconnect(); // Stop trying to connect to the WiFi.
   WiFi.softAPConfig(apIP, apIP, netMsk);
   DEBUG_PRINTF("Creating AP, with IP: %s\n", WiFi.softAPIP().toString().c_str());
   WiFi.softAP(DEVICE_NAME, "12345678");
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
   Network_start_MDNS();
   OTA_init();
   Webserver_start();
}

void Network_start_MDNS()
{
   if (!MDNS.begin(DEVICE_NAME))
      DEBUG_PRINTLN("Network: MDNS begin failed!");
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
   Clock_init();
   Mqtt_init();
   WiFi.mode(WIFI_STA);
   WiFi.setHostname(DEVICE_NAME);
   wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
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
   DEBUG_PRINTLN("\nNetwork: Resetting Wifi\n");
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
String Get_wifi_ip_address()
{
   if (WiFi.status() == WL_CONNECTED)
      return WiFi.localIP().toString();
   else if (WiFi.getMode() == WIFI_AP_STA)
      return WiFi.softAPIP().toString();
   return "";
}

void Set_wifi_credentials(String ssid, String pwd)
{
   DEBUG_PRINTF("Network: New Wi-Fi saved: %s\n", ssid.c_str());
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

/**
 * @brief Network's cyclic task @ 100ms
 *
 * The function updates the Wi-Fi manager's webserver, if it's active, and updated the MQTT submodule.
 *
 */
void Network_100ms_task()
{
   create_ap_ticker.update();
   if (!was_connected_b && WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP_STA && create_ap_ticker.state() != RUNNING)
      create_ap_ticker.start();
   else if (WiFi.getMode() == WIFI_AP_STA)
      dnsServer.processNextRequest();

   ArduinoOTA.handle();
   MDNS.update();
   Mqtt_100ms_task();
}

void Network_1000ms_task()
{
   Clock_task_1000ms();
}