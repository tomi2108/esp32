idf_component_register(
    SRCS "wifi.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_event esp_netif esp_wifi nvs_flash
)
