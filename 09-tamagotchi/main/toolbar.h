#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "esp_log.h"
#include "icons.h"
#include "inbox.h"
#include "inventory.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define TOOLBAR_LENGTH 5

void toolbar_display(void);
