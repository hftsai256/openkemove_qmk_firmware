


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

static void sled_write_reg
(
  ioline_t line,
  uint8_t page_num, uint8_t address,
  uint8_t value
) {
  palClearLine(line);
  uint8_t buffer[3];
  buffer[0] = page_num | 0x20;
  buffer[1] = address;
  buffer[2] = value;
  spiStartSend(&SPID1, 3, buffer);
  palSetLine(line);
}

static void setup_led_controller(ioline_t line) {
  sled_write_reg(line, 0xB, 0xA, 0x0); // Power Down
  sled_write_reg(line, 0xB, 0x1, 0x8); // Matrix type 2
  sled_write_reg(line, 0xB, 0xD, 0xE4); // Staggered Delay???
  sled_write_reg(line, 0xB, 0xE, 0x1); // Slew Rate Enable???
  sled_write_reg(line, 0xB, 0x14, 0x44); // Vaf Settings
  sled_write_reg(line, 0xB, 0x15, 0x44); // Vaf Settings (cont.)
  sled_write_reg(line, 0xB, 0xF, 0x99); // Constant Current Settings (20.5mA)
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
  sled_write_reg(LINE_LED1_CS, 0xB, 0xA, 0x1); // Power Down
  sled_write_reg(LINE_LED2_CS, 0xB, 0xA, 0x1); // Power Down
  chMtxUnlock(&led_mutex);
}

void sled_off(void) {
  chMtxLock(&led_mutex);
  sled_write_reg(LINE_LED1_CS, 0xB, 0xA, 0x0); // Power Down
  sled_write_reg(LINE_LED2_CS, 0xB, 0xA, 0x0); // Power Down
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
