idf_component_register(
    SRCS "main.c"
    "spinner.c"
    "display.c"
    "inventory.c"
    "actions.c"
    "inbox.c"
    "toolbar.c"
    "state.c"
    "shop.c"
    "wallet.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "mosquitto_ca.crt"
    REQUIRES oled buzzer mqttt wifi rotary_encoder http json icons
)
