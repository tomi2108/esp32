idf_component_register(
    SRCS "buzzer.c"
    INCLUDE_DIRS "."
    REQUIRES 
       melody
       esp_driver_ledc
       esp_driver_gpio
)
