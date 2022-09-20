


#include "sled17341.h"
#include "hal.h"
#include "ch.h"
#include "board.h"
#include <string.h>

uint8_t led_matrix[288];
mutex_t led_mutex;

const uint8_t led_map[61] = {
  2,3,4,5,6,7,8,9,10,11,36,37,189,190,
  46,47,72,73,74,75,76,77,80,151,152,153,185,154,
  81,82,83,108,109,110,111,112,113,182,155,181,180,
  114,116,119,146,147,148,149,150,186,191,216,187,
  38,41,42,43,45,44,115,188
};

static void spi_write(spi_tx_handler handle) {
    palClearLine(handle.line);

    uint8_t header = handle.frame | SLED_REGS_SANITY | SLED_OPERATION_WRITE;
    spiStartSend(&SPID1, 1, &header);
    spiStartSend(&SPID1, 1, &handle.offset);
    spiStartSend(&SPID1, handle.length, handle.buffer);

    palSetLine(handle.line);
}

/*
static void sled_write_reg
(
  ioline_t line,
  uint8_t page_num, uint8_t address,
  uint8_t value
) {
  palClearLine(line);
  spiStartSend(&SPID1, 1, page_num | SLED_REGS_SANITY | SLED_OPERATION_WRITE);
  spiStartSend(&SPID1, 1, &address);
  spiStartSend(&SPID1, 1, &value);
  palSetLine(line);
}
*/

static inline void led_configure(ioline_t line, led_fn_t operation, uint8_t value) {
    spi_tx_handler handle = {
        .line = line,
        .frame = SLED_REGS_FRAME_FUNC,
        .offset = operation,
        .length = 1,
        .buffer = &value};
    spi_write(handle);
}

static void setup_led_controller(ioline_t line) {
  led_configure(line, SLED_SHUTDOWN, SLED_SW_SHUTDOWN);
  led_configure(line, SLED_PICTURE_DISPLAY, SLED_MATRIX_TYPE_2);
  led_configure(line, SLED_STAGGERED_DELAY,
    (0 << SLED_STD1_OFFSET) | (1 << SLED_STD2_OFFSET) | (2 << SLED_STD3_OFFSET) | (3 << SLED_STD4_OFFSET));
  led_configure(line, SLED_SLEW_RATE, SLED_SLEW_RATE_ENABLE);
  led_configure(line, SLED_VAF_1, (SLED_VAF_BASELINE << SLED_VAF1_OFFSET) | (SLED_VAF_BASELINE << SLED_VAF2_OFFSET));
  led_configure(line, SLED_VAF_2, (SLED_VAF_BASELINE << SLED_VAF3_OFFSET) | SLED_FVAF_FTIME_ENABLE);
  led_configure(line, SLED_CURRENT_CTRL, SLED_CONST_CURRENT_ENABLE | 25); // Constant Current @20.5mA
}

static uint8_t buffer[0xB6 - 144];

void sled_init_update(void) {
  buffer[0] = 0x20;
  buffer[1] = 0;
  memset(&buffer[0x0 + 2], 0xFF, 0x12); // Turn ON
  memset(&buffer[0x12 + 2], 0x00, 18); // No Blink

  palClearLine(LINE_LED1_CS);
  spiStartSend(&SPID1, 0xB6 - 144, buffer);
  spiStartSend(&SPID1, 144, led_matrix);
  palSetLine(LINE_LED1_CS);

  palClearLine(LINE_LED2_CS);
  spiStartSend(&SPID1, 0xB6 - 144, buffer);
  spiStartSend(&SPID1, 144, &led_matrix[144]);
  palSetLine(LINE_LED2_CS);
}

void sled_early_init(void) {
  chMtxObjectInit(&led_mutex);
}

void sled_init(void) {
  chMtxLock(&led_mutex);
  setup_led_controller(LINE_LED1_CS);
  setup_led_controller(LINE_LED2_CS);
  memset(led_matrix, 0x0, 288);
  sled_init_update();
  chMtxUnlock(&led_mutex);
}

void sled_update_matrix(void) {
  uint8_t buffer2[2] = {0x20, 0x24};

  chMtxLock(&led_mutex);
  palClearLine(LINE_LED1_CS);
  spiStartSend(&SPID1, 2, buffer2);
  spiStartSend(&SPID1, 144, &led_matrix[0]);
  palSetLine(LINE_LED1_CS);

  palClearLine(LINE_LED2_CS);
  spiStartSend(&SPID1, 2, buffer2);
  spiStartSend(&SPID1, 144, &led_matrix[144]);
  palSetLine(LINE_LED2_CS);
  chMtxUnlock(&led_mutex);
}

void sled_on(void) {
  chMtxLock(&led_mutex);
  led_configure(LINE_LED1_CS, SLED_SHUTDOWN, SLED_PWR_NORMAL);
  led_configure(LINE_LED2_CS, SLED_SHUTDOWN, SLED_PWR_NORMAL);
  chMtxUnlock(&led_mutex);
}

void sled_off(void) {
  chMtxLock(&led_mutex);
  led_configure(LINE_LED1_CS, SLED_SHUTDOWN, SLED_SW_SHUTDOWN);
  led_configure(LINE_LED2_CS, SLED_SHUTDOWN, SLED_SW_SHUTDOWN);
  chMtxUnlock(&led_mutex);
}

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

// HsvColor RgbToHsv(RgbColor rgb)
// {
//     HsvColor hsv;
//     unsigned char rgbMin, rgbMax;

//     rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
//     rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

//     hsv.v = rgbMax;
//     if (hsv.v == 0)
//     {
//         hsv.h = 0;
//         hsv.s = 0;
//         return hsv;
//     }

//     hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
//     if (hsv.s == 0)
//     {
//         hsv.h = 0;
//         return hsv;
//     }

//     if (rgbMax == rgb.r)
//         hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
//     else if (rgbMax == rgb.g)
//         hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
//     else
//         hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

//     return hsv;
// }
