idf_component_register(
    SRCS "potentiometer.c"
    INCLUDE_DIRS "."
    REQUIRES 
       esp_driver_gpio analog
)
