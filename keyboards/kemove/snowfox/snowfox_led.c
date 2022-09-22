#include "snowfox.h"
#include "snowfox_ble.h"

const SPIConfig spi1Config = {
  .clock_divider = 1, // No Division
  .clock_prescaler = 24, // To 2MHz
  .clock_rate = 4, // Divide 2 again to be 1MHz
  .data_size = 8, // 8 bits per transfer
};

uint8_t led_brightness = 0xFF;

static mutex_t led_profile_mutex;

typedef void (*led_udpate_cb_t)(void);
typedef void (*led_action_up_t)(void);
typedef void (*led_action_dn_t)(void);

typedef struct {
    led_udpate_cb_t update;
    // led_action_up_t up;
    // led_action_dn_t dn;
} led_profile_t;

// ========= BEGIN PROFILES =========

void led_profile_cycle_update(void) {
    static hsv_color_t currentColor = {.h = 0, .s = 0xFF, .v = 0xFF};
    currentColor.h += 1;
    currentColor.v = led_brightness;
    rgb_color_t rgb = hsv_to_rgb(currentColor);

    for (uint8_t i=0; i<MATRIX_ROWS; i++) {
        for (uint8_t j=0; j<MATRIX_COLS; j++) {
            sled_set_cache_color(keypos_led_map[i][j], rgb);
        }
    }
}

void led_profile_static_update(void) {
    static hsv_color_t currentColor = {.h = 0, .s = 0x00, .v = 0xFF};
    rgb_color_t rgb = hsv_to_rgb(currentColor);
    currentColor.v = led_brightness;

    for (uint8_t i=0; i<MATRIX_ROWS; i++) {
        for (uint8_t j=0; j<MATRIX_COLS; j++) {
            sled_set_cache_color(keypos_led_map[i][j], rgb);
        }
    }
}

// ========= END PROFILES ===========

const led_profile_t led_profiles[] = {
    {led_profile_cycle_update},
    {led_profile_static_update},
};

static uint8_t current_profile = 0;
const uint8_t num_profiles = sizeof(led_profiles) / sizeof(led_profiles[0]);
static bool led_active = false;

void static usb_led_relay(void) {
    led_t led_state = host_keyboard_led_state();
    rgb_color_t color_on = {
            .r = (uint16_t)led_brightness * 255 / 255,
            .g = (uint16_t)led_brightness * 150 / 255,
            .b = (uint16_t)led_brightness * 0
    };
    rgb_color_t color_off = {0};

    if (led_state.num_lock) {
        sled_set_cache_color(status_indicator.num_lock, color_on);
    } else if (!led_active) {
        sled_set_cache_color(status_indicator.num_lock, color_off);
    }

    if (led_state.caps_lock) {
        sled_set_cache_color(status_indicator.caps_lock, color_on);
    } else if (!led_active) {
        sled_set_cache_color(status_indicator.caps_lock, color_off);
    }

    if (led_state.scroll_lock) {
        sled_set_cache_color(status_indicator.scrl_lock, color_on);
    } else if (!led_active) {
        sled_set_cache_color(status_indicator.scrl_lock, color_off);
    }
}


void snowfox_early_led_init(void) {
    sled_early_init();
    chMtxObjectInit(&led_profile_mutex);
}

THD_WORKING_AREA(waLEDThread, 128);
THD_FUNCTION(LEDThread, arg) {
    (void)arg;
    chRegSetThreadName("LEDThread");
    sled_init();
    while(1) {
        chThdSleepMilliseconds(50);

        if (led_active) {
            led_udpate_cb_t updateFn = led_profiles[current_profile].update;
            (*updateFn)();
        }

        usb_led_relay();
        sled_apply();
    }
}

void snowfox_led_next(void) {
    chMtxLock(&led_profile_mutex);
    current_profile = (current_profile + 1) % num_profiles;
    chMtxUnlock(&led_profile_mutex);
}

void suspend_power_down_kb(void) {
#if ENABLE_SLEEP_LED == TRUE
    if (led_active && !snowfox_ble_is_active()) {
        sled_off();
    }
#endif
}

void suspend_wakeup_init_kb(void) {
#if ENABLE_SLEEP_LED == TRUE
    if (led_active) {
        sled_on();
    }
#endif
}

void snowfox_led_on(void) {
    if (led_active) {
        snowfox_led_next();
    } else {
        led_active = true;
    }
}

void snowfox_led_off(void) {
    sled_cache_init();
    sled_apply();
    led_active = false;
}
