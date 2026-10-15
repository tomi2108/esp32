idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "mosquitto_ca.crt"
    REQUIRES oled buzzer mqttt wifi rotary_encoder http json
)
