idf_component_register(
    SRCS "analog.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_adc esp_driver_dac
)
