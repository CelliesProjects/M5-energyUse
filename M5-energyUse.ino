#define LGFX_AUTODETECT // 自動認識 (M5Stack, M5StickC/CPlus, ODROID-GO, TTGO T-Watch, TTGO T-Wristband, LoLin D32 Pro, ESP-WROVER-KIT)

#include <vector>
#include <WebSocketsClient.h>      /* https://github.com/Links2004/arduinoWebSockets */
#include <LovyanGFX.hpp>

#include "wifisetup.h"

static LGFX lcd;

static WebSocketsClient ws_bridge;

void connectToWebSocketBridge() {
  ws_bridge.onEvent(ws_bridge_onEvents);
  ws_bridge.begin(WS_BRIDGE_HOST, WS_BRIDGE_PORT, WS_BRIDGE_URL);
}

void drawWattage(uint32_t wattage) {
  time_t rawtime;
  time(&rawtime);

  struct tm * timeinfo;
  timeinfo = localtime(&rawtime);

  const char* weekday[] {"zondag", "maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag"};
  const char* month[] {"januari", "februari", "maart", "april", "mei", "juni", "juli", "augustus", "september", "oktober", "november", "december"};

  char buffer[80];
  snprintf(buffer, sizeof(buffer), "           %s %i %s %i           ",
           weekday[timeinfo->tm_wday],
           timeinfo->tm_mday,
           month[timeinfo->tm_mon],
           timeinfo->tm_year + 1900);

  lcd.setTextDatum(textdatum_t::top_center);
  lcd.setTextColor(0xFFFF00U, 0x000000U);
  lcd.setFont(&fonts::Font4);
  lcd.drawString(buffer, lcd.width() / 2,  1);
  lcd.setTextColor(0xFFFFFFU, 0x000000U);
  lcd.setFont(&fonts::Font8);
  lcd.drawString("   " + String(wattage) + "   ",  lcd.width() / 2,  45);
}

void drawElectricToday(uint32_t watthours) {
  lcd.setTextDatum(textdatum_t::top_center);
  lcd.setTextColor(0xFFFFFFU, 0x000000U);
  lcd.setFont(&fonts::Font4);
  float f_Wh = watthours / 1000.0;
  char tmp[100];
  snprintf(tmp, sizeof(tmp), "     %.3f kWh     ", f_Wh);
  lcd.drawString(tmp, lcd.width() / 2,  175);
}

void drawGasToday(uint32_t cubicmeters) {
  lcd.setTextDatum(textdatum_t::top_center);
  lcd.setTextColor(0xFFFFFFU, 0x000000U);
  lcd.setFont(&fonts::Font4);
  float f_cm = cubicmeters / 1000.0;
  char tmp[100];
  snprintf(tmp, sizeof(tmp), "     %.3f m3     ", f_cm);
  lcd.drawString(tmp, lcd.width() / 2,  205);
}

void setup() {
  lcd.init();
  lcd.setTextDatum(textdatum_t::top_center);
  lcd.setTextColor(0xFFFF00U, 0x000000U);
  lcd.setFont(&fonts::Font4);

  lcd.drawString("connecting WiFi",  lcd.width() / 2,  0);

  Serial.begin(115200);

  if (SET_STATIC_IP && !WiFi.config(STATIC_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS))
    Serial.println("Setting static IP failed");

  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  WiFi.setSleep(false);

  while (!WiFi.isConnected())
    delay(10);

  lcd.drawString("         syncing ntp         ",  lcd.width() / 2,  0);

  WiFi.onEvent(WiFiEvent);
  Serial.printf("connected to '%s' as %s\n", WIFI_NETWORK, WiFi.localIP().toString().c_str());
  Serial.println("syncing NTP");

  /* sync the clock with ntp */
  configTzTime(TIMEZONE, NTP_POOL);

  tm now {
    0
  };

  while (!getLocalTime(&now, 0))
    delay(10);

  lcd.drawString("verbinden met host",  lcd.width() / 2,  0);
  lcd.drawString("verbruik vandaag", lcd.width() / 2,  lcd.height() / 2 + 20);

  connectToWebSocketBridge();
}

void loop() {
  ws_bridge.loop();
  delay(1);
}

void ws_bridge_onEvents(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {

    case WStype_CONNECTED :
      Serial.printf("connected to websocket bridge 'ws://%s:%i%s'\n", WS_BRIDGE_HOST, WS_BRIDGE_PORT, WS_BRIDGE_URL);
      break;

    case WStype_DISCONNECTED :
      Serial.println("websocket bridge down - reconnecting");
      connectToWebSocketBridge();
      break;

    case WStype_TEXT : {
        ESP_LOGD(TAG, "payload: %s", payload);
        if (!ws_bridge.isConnected()) return;
        payload[length] = 0;

        std::vector<String> arg;

        arg.reserve(9);

        char *pch = strtok(reinterpret_cast<char*>(payload), "\n");
        if (!pch) return;

        while (pch) {
          ESP_LOGD(TAG, "argument: %s", pch);
          arg.push_back(pch);
          pch = strtok(NULL, "\n");
        }
        /*
                if (arg.size() == 2 && arg[0].equals("electric_saved")) {
                }
        */
        if (arg.size() == 9 && arg[0].equals("current")) {
          /*
            for (const auto& item : arg) {
              ESP_LOGI(TAG, "argument: %s", item.c_str());
            }
          */
          drawWattage(arg[1].toInt());
          drawElectricToday(arg[5].toInt() + arg[6].toInt());
          drawGasToday(arg[7].toInt());
        }
      }
      break;

    case WStype_ERROR :
      ESP_LOGE(TAG, "websocket bridge error");
      break;

    case WStype_PING :
      ESP_LOGI(TAG, "received ping");
      break;

    case WStype_PONG :
      ESP_LOGI(TAG, "received pong");
      break;

    default : ESP_LOGE(TAG, "unhandled websocket bridge event");
  }
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGD(TAG, "STA Started");
      //WiFi.setHostname( DEFAULT_HOSTNAME_PREFIX.c_str();
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      ESP_LOGD(TAG, "STA Connected");
      //WiFi.enableIpV6();
      break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      ESP_LOGD(TAG, "STA IPv6: ");
      ESP_LOGD(TAG, "%s", WiFi.localIPv6().toString());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGD(TAG, "STA IPv4: %s", WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "STA Disconnected");
      WiFi.begin();
      break;
    case SYSTEM_EVENT_STA_STOP:
      ESP_LOGI(TAG, "STA Stopped");
      break;
    default:
      break;
  }
}
