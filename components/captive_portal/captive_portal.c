#include "esp_http_server.h"
#include "dns.h"
#include "web_server.h"
void captive_portal_init()
{
    web_server_init();
}