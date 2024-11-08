#include <String.h>
#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "hw.h"
#include "memory.h"
#include "network.h"
#include "html_pages.h"

AsyncWebServer webserver(80);
AsyncWebSocket websocket("/ws");

String processor(const String &var);

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        String first = strtok((char *)data, " ");
        String second = strtok(NULL, " ");

        DEBUG_PRINTF("\nWebserver: Received WS message: %s %s\n", first.c_str(), second.c_str());

        if (first == "PAGE")
        {
            JsonDocument root;
            char payload[512];
            if (second == "INDEX")
            {
                root["light_mode"] = Get_light_mode_str();
                root["brightness"] = String(Get_esp_states().lightBrightness);
            }
            else if (second == "TIME")
            {
                root["ntp_tz"] = String(Get_timezone());
                root["manual_mode"] = String(Get_manual_mode());
                root["ntp_server"] = Get_ntp_server();
            }
            else if (second == "NET")
            {
                root["wifi_status"] = Get_wifi_status();
                root["mqtt_status"] = Get_mqtt_status();
                root["wifi_ssid"] = Get_wifi_ssid();
                root["mqtt_en"] = String(Get_mqtt_enabled());
                root["mqtt_host"] = Get_mqtt_host();
                root["mqtt_port"] = String(Get_mqtt_port());
                root["mqtt_qossub"] = String(Get_mqtt_qossub());
                root["mqtt_qospub"] = String(Get_mqtt_qospub());
                root["mqtt_cli"] = Get_mqtt_clientid();
                root["mqtt_user"] = Get_mqtt_username();
                root["mqtt_autodisc"] = Get_mqtt_autodiscovery();
            }
            serializeJson(root, payload);
            websocket.text(client->id(), payload);
        }

        else if (first == "LIGHTMODE")
        {
            if (second == "automatic")
                Set_lightMode(LIGHT_MODE_AUTO);
            else if (second == "manual")
                Set_lightMode(LIGHT_MODE_MANUAL);
            else if (second == "off")
                Set_lightMode(LIGHT_MODE_OFF);
        }
        else if (first == "BRIGHTNESS")
            Set_lightBrightness(second.toInt());
    }
}

void onSaveTime(AsyncWebServerRequest *request)
{
    /* If it is triggered, by a GET method, provide basic authentication */
    if (request->method() == HTTP_GET)
    {
        if (!request->authenticate(DEVICE_NAME, "12345678"))
            return request->requestAuthentication();
    }
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", save_config_html, save_config_html_size);
    response->addHeader(F("Content-Encoding"), "gzip");
    request->send(response);

    DEBUG_PRINTLN("\nWebserver: Time form received\n");

    for (int i = 0; i < (uint8_t)request->params(); i++)
    {
        const char *name = request->getParam(i)->name().c_str();
        const char *value = request->getParam(i)->value().c_str();

        if (strlen(value) == 0)
            continue;

        DEBUG_PRINTF("Webserver: Received param: %s=%s\n", name, value);

        // Check if the parameter is "manual" and handle accordingly
        if (strcmp(name, "manual") == 0)
        {
            uint32 value_u32 = atoi(value); // Convert value to integer

            if (value_u32)
            {
                Set_manual_mode(true);
                Set_timestamp(CLOCK_STATE_VALID, value_u32);
                return;
            }
            else
                Set_manual_mode(false);
        }
        // Check if the parameter is "server" and update the NTP server if needed
        if (strcmp(name, "server") == 0)
        {
            if (!Get_manual_mode())
                Set_ntp_server(value);
        }
        // Check if the parameter is "server" and update the NTP server if needed
        if (strcmp(name, "tz") == 0)
        {
            Set_timezone(atoi(value));
        }
    }
    if (WiFi.isConnected())
        Restart_device(RESTART_SOFT);
}

void onSaveMqtt(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", save_config_html, save_config_html_size);
    response->addHeader(F("Content-Encoding"), "gzip");
    request->send(response);

    String name, value;

    DEBUG_PRINTLN("\nWebserver: Mqtt form received\n");
    for (int i = 0; i < (uint8_t)request->params(); i++)
    {
        name = request->getParam(i)->name();
        value = request->getParam(i)->value();

        if (value.isEmpty() && name != "pwd")
            continue;

        DEBUG_PRINTF("Webserver: Received param: %s=%s\n", name.c_str(), value.c_str());

        if (name == "enabled")
            Set_mqtt_enabled(value.toInt());
        if (name == "host")
            Set_mqtt_host(value.c_str());
        if (name == "port")
            Set_mqtt_port(value.toInt());
        if (name == "qossub")
            Set_mqtt_qossub(value.toInt());
        if (name == "qospub")
            Set_mqtt_qospub(value.toInt());
        if (name == "clientid")
            Set_mqtt_clientid(value.c_str());
        if (name == "user")
            Set_mqtt_username(value.c_str());
        if (name == "pwd")
            Set_mqtt_password(value.c_str());
        if (name == "autodisc")
            Set_mqtt_autodiscovery(value);
    }
    /* MQTT shall be only saved in AP mode, no need to restart */
    if (WiFi.isConnected())
        Restart_device(RESTART_SOFT);
}

void onSaveWifi(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", save_config_html, save_config_html_size);
    response->addHeader(F("Content-Encoding"), "gzip");
    request->send(response);

    String name, value, ssid;

    DEBUG_PRINTLN("\nWebserver: Wi-Fi form received\n");
    for (int i = 0; i < (uint8_t)request->params(); i++)
    {
        name = request->getParam(i)->name();
        value = request->getParam(i)->value();

        if (value.isEmpty() && name != "pwd")
            continue;

        DEBUG_PRINTF("Webserver: Received param: %s=%s\n", name.c_str(), value.c_str());

        if (name == "ssid")
            ssid = value;
        if (name == "pwd")
            Set_wifi_credentials(ssid.c_str(), value.c_str());
    }
    Restart_device(RESTART_HARD);
}

void onResetConfig(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", reset_config_html, reset_config_html_size);
    response->addHeader(F("Content-Encoding"), "gzip");
    request->send(response);

    Memory_reset();
    Network_reset();

    Restart_device(RESTART_HARD);
}

void onRestart(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", save_config_html, save_config_html_size);
    response->addHeader(F("Content-Encoding"), "gzip");
    request->send(response);

    Restart_device(RESTART_HARD);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        DEBUG_PRINTF("\nWebserver: WS client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        DEBUG_PRINTF("Webserver: WS client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(client, arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void Notify_ws_clients(String key, String data)
{
    if (!websocket.getClients().isEmpty())
    {
        DEBUG_PRINTF("Webserver: Send [%s %s] to WS clients\n", key.c_str(), data.c_str());
        websocket.textAll(key + " " + data);
    }
}

bool isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

boolean CaptivePortalHandled(AsyncWebServerRequest *request)
{
    // Serve Captive Portal only in AP mode or if there's a valid host in the HTTP Header
    if (ON_STA_FILTER(request) || !request->hasHeader("Host"))
        return false;

    String hostHeader = request->getHeader("Host")->value();
    if (!isIp(hostHeader) && hostHeader.indexOf(String(DEVICE_NAME) + ".local") < 0)
    {
        AsyncWebServerResponse *response = request->beginResponse(302);
        response->addHeader(F("Location"), F("http://4.3.2.1"));
        request->send(response);
        return true;
    }
    return false;
}

void Webserver_start()
{
    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  if (CaptivePortalHandled(request))
                        return;
                    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, index_html_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/time_config", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
                        time_config_html, time_config_html_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/net_settings", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
                        net_settings_html, net_settings_html_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/other", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
                        other_html, other_html_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css",
                        bootstrap_min_css, bootstrap_min_css_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
                        bootstrap_min_js, bootstrap_min_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", 
                        index_js, index_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/net_settings.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
                        net_settings_js, net_settings_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/other.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
                        other_js, other_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/misc.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
                        misc_js, misc_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });
    webserver.on("/time_config.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 {  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
                        time_config_js, time_config_js_size);
                    response->addHeader(F("Content-Encoding"),"gzip");
                    response->addHeader(F("Cache-Control"),"no-cache");
                    request->send(response); });

    webserver.on("/save_time", HTTP_POST, onSaveTime);
    webserver.on("/save_mqtt", HTTP_POST, onSaveMqtt);
    webserver.on("/save_wifi", HTTP_POST, onSaveWifi);
    webserver.on("/save_time", HTTP_GET, onSaveTime);
    webserver.on("/reset_config", HTTP_GET, onResetConfig);
    webserver.on("/restart", HTTP_GET, onRestart);

    webserver.onNotFound([](AsyncWebServerRequest *request)
                         {  if (CaptivePortalHandled(request))
                                return;
                            request->send(404, "text/plain", "Not found"); });

    DEBUG_PRINTLN("Webserver: Starting the webserver");
    websocket.onEvent(onEvent);
    webserver.addHandler(&websocket);
    webserver.begin();
}

void Webserver_stop()
{
    DEBUG_PRINTLN("Webserver: Stopping the webserver");
    webserver.end();
}

String processor(const String &var)
{
    if (var == "MANUAL_MODE")
        return Get_manual_mode() ? "checked" : "";
    else if (var == "NTP_SERVER")
        return Get_ntp_server();
    else if (var == "NTP_TZ")
        return String(Get_timezone());
    else if (var == "MQTT_STATUS")
        return Get_mqtt_status();
    else if (var == "MQTT_EN")
        return Get_mqtt_enabled() ? "checked" : "";
    else if (var == "MQTT_HOST")
        return Get_mqtt_host();
    else if (var == "MQTT_PORT")
        return Get_mqtt_port();
    else if (var == "MQTT_QOSSUB")
        return Get_mqtt_qossub();
    else if (var == "MQTT_QOSPUB")
        return Get_mqtt_qospub();
    else if (var == "MQTT_CLI")
        return Get_mqtt_clientid();
    else if (var == "MQTT_USER")
        return Get_mqtt_username();
    else if (var == "MQTT_AUTODISC")
        return Get_mqtt_autodiscovery();
    else if (var == "WIFI_STATUS")
        return Get_wifi_status();
    else if (var == "WIFI_SSID")
        return Get_wifi_ssid();
    return "";
}