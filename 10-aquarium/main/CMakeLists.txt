idf_component_register(
    SRCS "main.c"
    "assets/images.c"
    "common/dates.c"
    "common/random.c"
    "common/task.c"
    "common/vector.c"
    "services/aquarium.c"
    "services/wallet.c"
    "ui/components/toolbar.c"
    "ui/components/wallet.c"
    "ui/components/fish.c"
    "ui/overlay.c"
    "ui/screen.c"
    "ui/views/aquarium.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "mosquitto_ca.crt"
    REQUIRES oled buzzer mqttt wifi rotary_encoder http json icons modal animation
)
