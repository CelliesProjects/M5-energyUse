#define LGFX_TTGO_TM // use LGFX_TTGO_TM for TTGO-TM-ESP32 -this board-> https://github.com/LilyGO/TTGO-TM-ESP32
                     // or LGFX_AUTODETECT for M5Stack and maybe others

#include <vector>
#include <LovyanGFX.hpp>      /* Arduino library manager */
#include <WebSocketsClient.h> /* https://github.com/Links2004/arduinoWebSockets */

// see this for lovyan ttgo-tm-st7789 setup https://github.com/lovyan03/LovyanGFX/issues/159

#include "wifisetup.h"
#if defined(LGFX_TTGO_TM)
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();

            cfg.spi_host = VSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 14000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = 1;
            cfg.pin_sclk = 18;
            cfg.pin_mosi = 23;
            cfg.pin_miso = -1;
            cfg.pin_dc = 16;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = 5;
            cfg.pin_rst = 17;
            cfg.pin_busy = -1;

            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 1;
            cfg.dummy_read_pixel = 16;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};
#endif

LGFX lcd;

static WebSocketsClient ws_bridge;

void connectToWebSocketBridge()
{
    ws_bridge.onEvent(ws_bridge_onEvents);
    ws_bridge.begin(WS_BRIDGE_HOST, WS_BRIDGE_PORT, WS_BRIDGE_URL);
}

void drawWattage(uint32_t wattage)
{
    time_t rawtime;
    time(&rawtime);

    struct tm *timeinfo;
    timeinfo = localtime(&rawtime);

    const char *weekday[]{"zondag", "maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag"};
    const char *month[]{"januari", "februari", "maart", "april", "mei", "juni", "juli", "augustus", "september", "oktober", "november", "december"};

    char buffer[80];
    snprintf(buffer, sizeof(buffer), "             %s %i %s             ",
             weekday[timeinfo->tm_wday],
             timeinfo->tm_mday,
             month[timeinfo->tm_mon]);

    lcd.setTextDatum(textdatum_t::top_center);
    lcd.setTextColor(0xFFFF00U, 0x000000U);
    lcd.setFont(&fonts::Font4);
    lcd.drawString(buffer, lcd.width() / 2, 1);
    lcd.setTextColor(0xFFFFFFU, 0x000000U);
    lcd.setFont(&fonts::Font8);
    lcd.drawString("   " + String(wattage) + "   ", lcd.width() / 2, 45);
}

void drawElectricToday(uint32_t watthours)
{
    lcd.setTextDatum(textdatum_t::top_center);
    lcd.setTextColor(0xFFFFFFU, 0x000000U);
    lcd.setFont(&fonts::Font4);
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "     %.3f kWh     ", watthours / 1000.0);
    lcd.drawString(tmp, lcd.width() / 2, 175);
}

void drawGasToday(uint32_t cubicmeters)
{
    lcd.setTextDatum(textdatum_t::top_center);
    lcd.setTextColor(0xFFFFFFU, 0x000000U);
    lcd.setFont(&fonts::Font4);
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "     %.3f m3     ", cubicmeters / 1000.0);
    lcd.drawString(tmp, lcd.width() / 2, 205);
}

std::vector<String> arg;

void setup()
{
    lcd.init();

    lcd.setBrightness(10);

#if defined(LGFX_ESP_WROVER_KIT)
    lcd.setRotation(1);
#endif

    lcd.setTextDatum(textdatum_t::top_center);
    lcd.setTextColor(0xFFFF00U, 0x000000U);
    lcd.setFont(&fonts::Font4);

    lcd.drawString("WiFi starten", lcd.width() / 2, 0);

    Serial.begin(115200);

    if (SET_STATIC_IP && !WiFi.config(STATIC_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS))
        Serial.println("Geen statisch IP mogelijk");

    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
    WiFi.setSleep(false);

    while (!WiFi.isConnected())
        delay(10);

    lcd.drawString("         klok gelijkzetten         ", lcd.width() / 2, 0);

    WiFi.onEvent(WiFiEvent);
    Serial.printf("connected to '%s' as %s\n", WIFI_NETWORK, WiFi.localIP().toString().c_str());
    Serial.println("syncing NTP");

    /* sync the clock with ntp */
    configTzTime(TIMEZONE, NTP_POOL);

    tm now{};

    while (!getLocalTime(&now, 0))
        delay(10);

    lcd.drawString("verbinden met host", lcd.width() / 2, 0);
    lcd.drawString("verbruikt vandaag:", lcd.width() / 2, lcd.height() / 2 + 20);

    arg.reserve(9);
    connectToWebSocketBridge();
}

void loop()
{
    ws_bridge.loop();
    delay(1);
}

void ws_bridge_onEvents(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {

    case WStype_CONNECTED:
        Serial.printf("connected to websocket bridge 'ws://%s:%i%s'\n", WS_BRIDGE_HOST, WS_BRIDGE_PORT, WS_BRIDGE_URL);
        break;

    case WStype_DISCONNECTED:
        Serial.println("websocket bridge down - reconnecting");
        connectToWebSocketBridge();
        break;

    case WStype_TEXT:
    {
        log_d("payload: %s", payload);
        if (!ws_bridge.isConnected())
            return;
        payload[length] = 0;

        char *pch = strtok(reinterpret_cast<char *>(payload), "\n");
        if (!pch)
            return;

        while (pch)
        {
            log_d("argument: %s", pch);
            arg.push_back(pch);
            pch = strtok(NULL, "\n");
        }
        /*
                if (arg.size() == 2 && arg[0].equals("electric_saved")) {
                }
        */
        if (arg.size() == 9 && arg[0].equals("current"))
        {
            /*
                for (const auto& item : arg) {
                log_i("argument: %s", item.c_str());
                }
            */
            drawWattage(arg[1].toInt());
            drawElectricToday(arg[5].toInt() + arg[6].toInt());
            drawGasToday(arg[7].toInt());
        }
        arg.clear();
    }
    break;

    case WStype_ERROR:
        log_e("websocket bridge error");
        break;

    case WStype_PING:
        log_i("received ping");
        break;

    case WStype_PONG:
        log_i("received pong");
        break;

    default:
        log_e("unhandled websocket bridge event");
    }
}

void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_START:
        log_d("STA Started");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        log_d("STA Connected");
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        log_d("STA IPv6: ");
        log_d("%s", WiFi.localIPv6().toString());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        log_d("STA IPv4: %s", WiFi.localIP());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        log_i("STA Disconnected");
        WiFi.begin();
        break;
    case SYSTEM_EVENT_STA_STOP:
        log_i("STA Stopped");
        break;
    default:
        break;
    }
}
