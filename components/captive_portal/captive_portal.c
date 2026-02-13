#include "esp_http_server.h"
#include "dns.h"
#include "web_server.h"
void captive_portal_init()
{
    web_server_init();
    dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*", "WIFI_AP_DEF");
    start_dns_server(&config);
}