#ifndef __SNOWFOX_H__
#define __SNOWFOX_H__

#include "quantum.h"
#include "ch.h"
#include "hal.h"
#include "sled17341.h"
#include "board.h"

#define SF_LED_STEP 0x40

enum SNOWFOX_KEYCODES {
    SNOWFOX_LED_ON = SAFE_RANGE,
    SNOWFOX_LED_OFF,
    SNOWFOX_LED_NEXT, // Next Profile
    SNOWFOX_LED_PUP, // Profile Specific action Up
    SNOWFOX_LED_PDN, // Profile Specific action Down
    SNOWFOX_LED_BUP, // Brightness Up
    SNOWFOX_LED_BDN, // Brightness Down
    SNOWFOX_BLE_CONN,
    SNOWFOX_BLE_DISCOVER,
    SNOWFOX_BLE_DISCONN,
    SNOWFOX_BLE_KB1,
    SNOWFOX_BLE_KB2,
    SNOWFOX_BLE_KB3,
    SNOWFOX_SAFE_RANGE,
};

#define OVERRIDE __attribute__ ((noinline))

extern THD_WORKING_AREA(waLEDThread, 128);
THD_FUNCTION(LEDThread, arg);

extern SerialConfig serialCfg;
extern thread_t *ble_thread;

extern const SPIConfig spi1Config;
extern thread_t *led_thread;
extern uint8_t led_brightness;

void snowfox_early_init(void);
void snowfox_early_led_init(void);

void snowfox_led_on(void);
void snowfox_led_off(void);
void snowfox_led_next(void);

#endif
