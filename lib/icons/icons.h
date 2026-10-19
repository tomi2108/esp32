#ifndef ICONS_H
#define ICONS_H

#include <stdint.h>

#define ICON_WIDTH 16
#define ICON_HEIGHT 16

typedef const uint8_t *Icon;

Icon icons_email_unread(void);
Icon icons_email_read(void);

#endif
