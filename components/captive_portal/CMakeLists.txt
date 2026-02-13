idf_component_register(
    SRCS 
        "dns.c"
        "web_server.c"
        "captive_portal.c"

    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_event nvs_flash esp_wifi esp_http_server esp_netif json display lvgl scene
    EMBED_FILES root.html
)