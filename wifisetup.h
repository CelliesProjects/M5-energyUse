const char* WIFI_NETWORK = "xxx";
const char* WIFI_PASSWORD = "xxx";

const char*    WS_BRIDGE_HOST =    "192.168.0.90";          /* bridge adress */
const uint16_t WS_BRIDGE_PORT =    80;                       /* bridge port */
const char*    WS_BRIDGE_URL =     "/events";                   /* bridge url */

#define SET_STATIC_IP false                              /* If SET_STATIC_IP is set to true then STATIC_IP, GATEWAY, SUBNET and PRIMARY_DNS have to be set to some sane values */

const IPAddress STATIC_IP(192, 168, 0, 70);              /* This should be outside your router dhcp range! */
const IPAddress GATEWAY(192, 168, 0, 1);                 /* Set to your gateway ip address */
const IPAddress SUBNET(255, 255, 255, 0);                /* Usually 255,255,255,0 check in your router or pc connected to the same network */
const IPAddress PRIMARY_DNS(192, 168, 0, 30);            /* Check in your router */
const IPAddress SECONDARY_DNS(192, 168, 0, 50);          /* Check in your router */

/* settings for ntp time sync */
const char* NTP_POOL =                  "nl.pool.ntp.org";
const char* TIMEZONE =                  "CET-1CEST,M3.5.0/2,M10.5.0/3"; /* Central European Time - see https://sites.google.com/a/usapiens.com/opnode/time-zones */
