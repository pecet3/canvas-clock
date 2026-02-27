idf_component_register(
    SRCS 
        "buzzer.c"

    INCLUDE_DIRS 
        "."
    PRIV_REQUIRES 
        driver
        esp_driver_ledc
)