#pragma once

#include <stdint.h>
#include "ch.h"

// SLED17341 Header Byte Definition
#define HEADER_FRAME_1                (0x0 << 0)
#define HEADER_FRAME_2                (0x1 << 0)
#define HEADER_FRAME_FUNC             (0xb << 0)
#define HEADER_FRAME_DETECT           (0xc << 0)
#define HEADER_FRAME_LOCATION         (0xd << 0)

#define HEADER_CHECKING               (0x2 << 4)

#define HEADER_OPERATION_WRITE        (0x0 << 7)
#define HEADER_OPERATION_READ         (0x1 << 7)


// SLED17341 Register Offsets
#define OP_LED_CTRL                   (0x0 )
#define OP_BLINK_CTRL                 (0x12)
#define OP_PWM                        (0x24)
#define OP_END_PAGE                   (0xb4)

#define FN_OPEN                       (0x0 )
#define FN_SHORT                      (0x12)
#define FN_CURRENT_TUNE               (0x24)    // Current fine tune register
#define FN_END_PAGE                   (0xb4)

#define DE_LED_VAF                    (0x0 )    // Anti-forward register
#define DE_END_PAGE                   (0x24)


extern mutex_t led_mutex;
extern uint8_t led_matrix[288];

extern const uint8_t led_map[61];

typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

void sled_early_init(void);
void sled_init(void);
void sled_update_matrix(void);
void sled_on(void);
void sled_off(void);
static void sled_set_color(uint8_t key, uint8_t r, uint8_t g, uint8_t b) {
    led_matrix[led_map[key]] = r;
    led_matrix[led_map[key]+12] = g;
    led_matrix[led_map[key]+24] = b;
}

RgbColor HsvToRgb(HsvColor hsv);
