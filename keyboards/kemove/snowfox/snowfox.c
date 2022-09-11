#include "snowfox.h"
#include "snowfox_ble.h"
#include "string.h"

thread_t *led_thread = NULL;
thread_t *ble_thread = NULL;

SerialConfig serialCfg = {
    9600
};

void dip_switch_init_kb(void){}

void matrix_scan_kb(void) {
    matrix_scan_user();
}

__attribute__((weak)) void dip_switch_update_user(uint8_t idx, bool active) {}

void dip_switch_update_kb(uint8_t index, bool active) {
    dip_switch_update_user(index, active);
}

void snowfox_early_init(void) {
    chMtxObjectInit(&ble_ok_mutex);
    chCondObjectInit(&ble_ok_cond);

    snowfox_early_led_init();
}

void matrix_init_kb(void) {
    led_thread = chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO, LEDThread, NULL);
    ble_thread = chThdCreateStatic(waBLEThread, sizeof(waBLEThread), NORMALPRIO, BLEThread, NULL);
    matrix_init_user();
}

void bootloader_jump(void) {
    *((volatile uint32_t*) 0x100001F0) = 0xDEADBEEF;
    __asm__ __volatile__("dsb");
    SCB->AIRCR = 0x05FA0004; // Sys Reset
    __asm__ __volatile__("dsb");
    while(1) {}
}

/*!
 * @returns false   processing for this keycode has been completed.
 */
bool OVERRIDE process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case SNOWFOX_LED_ON:
                snowfox_led_on();
                return false;
            case SNOWFOX_LED_OFF:
                snowfox_led_off();
                return false;
            case SNOWFOX_LED_NEXT:
                snowfox_led_next();
                return false;
            case SNOWFOX_LED_PUP:
                return false;
            case SNOWFOX_LED_PDN:
                return false;
            case SNOWFOX_LED_BUP:
                if ((0xFF - led_brightness) >= SF_LED_STEP) {
                    led_brightness += SF_LED_STEP;
                } else {
                    led_brightness = 0xFF;
                }
                return false;
            case SNOWFOX_LED_BDN:
                if (led_brightness > (0x10 + SF_LED_STEP)) {
                    led_brightness -= SF_LED_STEP;
                } else {
                    led_brightness = 0x10;
                }
                return false;
            case SNOWFOX_BLE_CONN:
                snowfox_ble_connect();
                return false;
            case SNOWFOX_BLE_DISCOVER:
                snowfox_ble_discover();
                return false;
            case SNOWFOX_BLE_DISCONN:
                snowfox_ble_disconnect();
                return false;
            case SNOWFOX_BLE_KB1:
                snowfox_ble_select(KEYBOARD1);
                return false;
            case SNOWFOX_BLE_KB2:
                snowfox_ble_select(KEYBOARD2);
                return false;
            case SNOWFOX_BLE_KB3:
                snowfox_ble_select(KEYBOARD3);
                return false;
            default:
                break;
        }
    }
    return process_record_user(keycode, record);
}
