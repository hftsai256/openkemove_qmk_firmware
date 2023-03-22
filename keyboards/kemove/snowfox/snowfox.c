#include "snowfox.h"
#include "print.h"
#include "snowfox_ble.h"
#include "string.h"

thread_t *led_thread = NULL;
thread_t *ble_thread = NULL;

SerialConfig serialCfg = {
    9600
};

bool dip_switch_update_kb(uint8_t index, bool active) {
if (!dip_switch_update_user(index, active)) { return false; }
  switch (index) {
    case 0:
      // v1.5: DIP on = Windows Layer, DIP off = Mac Layer
      default_layer_set(1UL << (active ? 0 : 1));
      break;
  }
  return true;
}

void matrix_init_kb(void) {
    snowfox_early_led_init();
    led_thread = chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO, LEDThread, NULL);
    ble_thread = chThdCreateStatic(waBLEThread, sizeof(waBLEThread), NORMALPRIO, BLEThread, NULL);
}

void bootloader_jump(void) {
    *((volatile uint32_t*) 0x100001F0) = 0xDEADBEEF;
    __asm__ __volatile__("dsb");
    SCB->AIRCR = 0x05FA0004; // Sys Reset
    __asm__ __volatile__("dsb");
    while(1) {}
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u, time: %5u, int: %u, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 
  return true;
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
                snowfox_ble_select(BLE_KEYBOARD1);
                return false;
            case SNOWFOX_BLE_KB2:
                snowfox_ble_select(BLE_KEYBOARD2);
                return false;
            case SNOWFOX_BLE_KB3:
                snowfox_ble_select(BLE_KEYBOARD3);
                return false;
            default:
                break;
        }
    }
    return process_record_user(keycode, record);
}

