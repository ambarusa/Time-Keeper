#include <String.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "hw.h"
#include "memory.h"
#include "network.h"

#if defined(FLEURIE)
const String light_form = R"=====(
            <div class="form-check mb-3">
                <input class="form-check-input" type="radio" name="light_mode" id="light_automatic">
                <label class="form-check-label" for="light_auto">Automatic</label>
            </div>
            <div class="form-check">
                <input class="form-check-input" type="radio" name="light_mode" id="light_manual">
                <label class="form-check-label" for="light_manual">Manual</label>
            </div>
            <div class="form-check">
                <input type="range" class="form-range" id="brightness" min="3" max="100" step="1"
                    value="%BRIGHTNESS_PCT%">
            </div>)=====";

#elif defined(PIXIE)
const String light_form = R"=====(
            <div class="form-check">
                <input class="form-check-input" type="radio" name="light_mode" id="light_manual">
                <label class="form-check-label" for="light_manual">ON</label>
            </div>)=====";
#endif

AsyncWebServer webserver(80);
AsyncWebSocket websocket("/ws");

String processor(const String &var);

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        String first = strtok((char *)data, " ");
        String second = strtok(NULL, " ");

#ifdef DEBUG
        Serial.printf("\nWebserver: Received WS message: %s %s\n", first.c_str(), second.c_str());
#endif

        if (first == "LIGHTMODE")
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

void onSaveConfig(AsyncWebServerRequest *request)
{
    request->send(LittleFS, "/save_config.html", "text/html", false, processor);

    String form_type, name, value, ssid;

    form_type = request->getParam(0)->name();
#ifdef DEBUG
    Serial.printf("\nWebserver: %s form received\n", form_type.c_str());
#endif
    for (int i = 1; i < (uint8_t)request->params(); i++)
    {
        name = request->getParam(i)->name();
        value = request->getParam(i)->value();

        if (value.isEmpty() && name != "pwd")
            continue;

#ifdef DEBUG
        Serial.printf("Webserver: Received param: %s=%s\n", name.c_str(), value.c_str());
#endif

        if (form_type == "TIME")
        {
            if (name == "manual")
            {
                if (value.toInt())
                {
                    Set_manual_mode(true);
                    Set_timestamp(CLOCK_STATE_VALID, value.toInt());
                }
                else
                    Set_manual_mode(false);
            }
            if (name == "server")
            {
                if (!Get_manual_mode())
                    Set_ntp_server(value);
            }
            if (name == "tz")
            {
                Set_timezone(value.toInt());
            }
        }
        if (form_type == "MQTT")
        {
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
            if (i == (uint8_t)request->params() - 1)
                Restart_device(true);
        }
        if (form_type == "Wi-Fi")
        {
            if (name == "ssid")
                ssid = value;
            if (name == "pwd")
                Set_wifi_credentials(ssid.c_str(), value.c_str());
            if (i == (uint8_t)request->params() - 1)
                Restart_device(false);
        }
    }
}

void onResetConfig(AsyncWebServerRequest *request)
{
    request->send(LittleFS, "/reset_config.html", "text/html", false, processor);

    Memory_reset();
    Network_reset();

    Restart_device(false);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
#ifdef DEBUG
        Serial.printf("\nWebserver: WS client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
#endif
        Notify_ws_clients("LIGHTMODE", Get_light_mode_str());
#if defined(FLEURIE)
        Notify_ws_clients("BRIGHTNESS", String(Get_esp_states().lightBrightness));
#endif
        break;
    case WS_EVT_DISCONNECT:
#ifdef DEBUG
        Serial.printf("Webserver: WS client #%u disconnected\n", client->id());
#endif
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
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
#ifdef DEBUG
        if (key != "TIME")
            Serial.printf("Webserver: Send [%s %s] to WS clients\n", key.c_str(), data.c_str());
#endif
        websocket.textAll(key + " " + data);
    }
}

void Webserver_init()
{
#ifdef DEBUG
    if (LittleFS.begin())
        Serial.println("Webserver: LittleFS mounted successfully");
    else
        Serial.println("Webserver: An error has occurred while mounting LittleFS");
#else
    LittleFS.begin();
#endif

    websocket.onEvent(onEvent);
    webserver.addHandler(&websocket);
}

void Webserver_start()
{
    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/index.html", "text/html", false, processor); });

    webserver.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/settings.html", "text/html", false, processor); });

    webserver.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/bootstrap.min.css", "text/css"); });

    webserver.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/bootstrap.min.js", "text/javascript"); });

    webserver.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/index.js", "text/javascript"); });

    webserver.on("/settings.js", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(LittleFS, "/settings.js", "text/javascript"); });

    webserver.on("/save_config", HTTP_POST, onSaveConfig);

    webserver.on("/save_config", HTTP_GET, onSaveConfig);

    webserver.on("/reset_config", HTTP_GET, onResetConfig);

    webserver.onNotFound([](AsyncWebServerRequest *request)
                         { request->send(404, "text/plain", "Not found"); });

#ifdef DEBUG
    Serial.println("Webserver: Starting the webserver");
#endif
    webserver.begin();
}

void Webserver_stop()
{
#ifdef DEBUG
    Serial.println("Webserver: Stopping the webserver");
#endif
    webserver.end();
}

String processor(const String &var)
{
    if (var == "DEV_NAME")
        return String(DEVICE_NAME);
    else if (var == "LIGHT_FORM")
        return light_form;
    else if (var == "BRIGHTNESS_PCT")
        return String(Get_esp_states().lightBrightness);
    else if (var == "MANUAL_MODE")
        return Get_manual_mode() ? "checked" : "";
    else if (var == "NTP_SERVER")
        return Get_ntp_server();
    else if (var == "NTP_TZ")
        return String(Get_ntp_timezone());
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