idf_component_register(
    SRCS "main.c"
    "actions.c"
    "assets.c"
    "display.c"
    "games.c"
    "inbox.c"
    "inventory.c"
    "main_menu.c"
    "shop.c"
    "spinner.c"
    "state.c"
    "tictactoe.c"
    "toolbar.c"
    "wallet.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "mosquitto_ca.crt"
    REQUIRES oled buzzer mqttt wifi rotary_encoder http json icons
)
