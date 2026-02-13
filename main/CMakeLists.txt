idf_component_register(SRCS "canvas-display.c" "wifi.c" 
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_wifi nvs_flash captive_portal display data scene
                    )
