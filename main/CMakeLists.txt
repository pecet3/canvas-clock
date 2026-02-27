idf_component_register(SRCS "canvas-clock.c" "wifi.c" 
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES buzzer esp_wifi nvs_flash captive_portal display data scene
                    )
