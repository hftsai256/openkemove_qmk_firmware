#pragma once

#include <stdint.h>
#include <hal.h>
#include "ch.h"

// SLED17341 Header Byte Definition
#define SLED_REGS_FRAME_1                (0x0 << 0)
#define SLED_REGS_FRAME_2                (0x1 << 0)
#define SLED_REGS_FRAME_FUNC             (0xb << 0)
#define SLED_REGS_FRAME_DETECT           (0xc << 0)
#define SLED_REGS_FRAME_LOCATION         (0xd << 0)
#define SLED_REGS_SANITY                 (0x2 << 4)

#define SLED_OPERATION_WRITE             (0x0 << 7)
#define SLED_OPERATION_READ              (0x1 << 7)


// SLED17341 Function Register Value
// 00h: Config
#define SLED_ENABLE                0b00001000
#define SLED_ADC_ENABLE            0b00010000
#define SLED_SYNC_MASTER           0b01000000
#define SLED_SYNC_SLAVE            0b10000000
// 01h: Picture Display
#define SLED_MATRIX_TYPE_1         0b00000000
#define SLED_MATRIX_TYPE_2         0b00001000
#define SLED_MATRIX_TYPE_3         0b00010000
#define SLED_MATRIX_TYPE_4         0b00011000
// 05h: Display Option
#define SLED_BLINK_ACTIVE_327ms    0b00000001
#define SLED_BLINK_ACTIVE_MAX      0b00000111
#define SLED_BLINK_ENABLE          0b00001000
#define SLED_BLINK_FRAME_300       0b00000000
#define SLED_BLINK_FRAME_600       0b01000000
#define SLED_BLINK_FRAME_75        0b10000000
#define SLED_BLINK_FRAME_150       0b11000000
// 06h: Audio Synchronization
#define SLED_AUDIO_SYNC_ENABLE     0b00000001
// 08h: Breath Control 1
#define SLED_FADEIN_BASE2          0b00000001
#define SLED_FADEIN_MAX            0b00000111
#define SLED_FADEOUT_BASE2         0b00010000
#define SLED_FADEOUT_MAX           0b01110000
// 09h: Breath Control 2
#define SLED_EXTINGWISH_BASE2      0b00000001
#define SLED_EXTINGWISH_MAX        0b00000111
#define SLED_BREATH_ENABLE         0b00010000
#define SLED_BREATH_CONTINUOUS     0b00100000
// 0Ah: Shutdown
#define SLED_SW_SHUTDOWN           0b00000000
#define SLED_PWR_NORMAL            0b00000001
// 0Bh: AGC (Audio Gain Control)
#define SLED_AGC_3dB               0b00000001
#define SLED_AGC_MAX               0b00000111
#define SLED_AGC_ENABLE            0b00001000
#define SLED_AGC_SLOW              0b00000000
#define SLED_AGC_FAST              0b00010000
// 0Dh: Staggered Delay (0->3: Zero delay->Max delay)
#define SLED_STD1_OFFSET           0x0
#define SLED_STD2_OFFSET           0x2
#define SLED_STD3_OFFSET           0x4
#define SLED_STD4_OFFSET           0x6
// 0Eh: Slew rate
#define SLED_SLEW_RATE_ENABLE      0b00000001
// 0Fh: Constant current control
#define SLED_CONST_CURRENT_ENABLE  0b10000000
#define SLED_CONST_CURRENT_SAFE    0b00110001
#define SLED_CONST_CURRENT_MAX     0b00111111
// 14h: VAF 1
#define SLED_VAF_BASELINE          0b100
#define SLED_VAF1_OFFSET           0x0
#define SLED_VAF2_OFFSET           0x4
// 15h: VAF2
#define SLED_VAF3_OFFSET           0x0
#define SLED_FTIME_OFFSET          0x3
#define SLED_FVAF_ENABLE           0b00000000
#define SLED_FVAF_FTIME_ENABLE     0b01000000
#define SLED_FVAF_DISABLE          0b10000000


// SLED17341 Register Offsets
typedef union {
    uint8_t raw[168];
    struct {
        uint8_t enable[12];
        uint8_t blink[12];
        uint8_t pwm[144];
    };
} __attribute__((packed)) led_ctrl_t;

// SLED17341 Function Register Address
typedef enum {
    SLED_CONFIG = 0,
    SLED_PICTURE_DISPLAY,
    SLED_AUTOPLAY_DISPLAY,
    SLED_AUTOPLAY_DELAY,
    SLED_LED_RESERVED_00,
    SLED_DISPLAY_OPTION,
    SLED_AUDIO_SYNC,
    SLED_RESERVED_01,
    SLED_BREATH_FADE,
    SLED_BREATH_DELAY,
    SLED_SHUTDOWN,
    SLED_AGC,
    SLED_AUDIO_ADC_RATE,
    SLED_STAGGERED_DELAY,
    SLED_SLEW_RATE,
    SLED_CURRENT_CTRL,
    SLED_OPEN_SHORT_TEST_1,
    SLED_OPEN_SHORT_TEST_2,
    SLED_RESERVED_02,
    SLED_ADC_OUTPUT,
    SLED_VAF_1,
    SLED_VAF_2,
    SLED_THERMO_STATE,
    SLED_TYPE4_VAF_1,
    SLED_TYPE4_VAF_2,
    SLED_TYPE4_VAF_3,
    SLED_CHIP_ID
} led_fn_t;

typedef struct {
    ioline_t line;
    uint8_t frame;
    uint8_t offset;
    uint8_t length;
    uint8_t* buffer;
} spi_tx_handler;


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
