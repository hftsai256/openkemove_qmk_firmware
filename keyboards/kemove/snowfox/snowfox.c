#include "snowfox.h"
#include "print.h"
#include "snowfox_ble.h"
#include "string.h"

thread_t *led_thread = NULL;

#if DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) { return false; }
    switch (index) {
        case 0:
            default_layer_set(active == DIP0_WIN);
            break;
        }
    return true;

}
#endif

void matrix_init_kb(void) {
    snowfox_early_led_init();
    led_thread = chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO, LEDThread, NULL);
}

void bootloader_jump(void) {
    *((volatile uint32_t*) 0x100001F0) = 0xDEADBEEF;
    __asm__ __volatile__("dsb");
    SCB->AIRCR = 0x05FA0004; // Sys Reset
    __asm__ __volatile__("dsb");
    while(1) {}
}

bool OVERRIDE process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) { return false; }
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
                ble_cmdq->put(CONNECT);
                return false;
            case SNOWFOX_BLE_DISCOVER:
                ble_cmdq->put(DISCOVER);
                return false;
            case SNOWFOX_BLE_DISCONN:
                ble_cmdq->put(DROP_CONN);
                return false;
            case LT(_FN2, SNOWFOX_BLE_KB1):
                if (record->tap.count) {
                    ble_cmdq->put(PICK_KEYBOARD1);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(TOGGLE);
                } else {
                    ble_cmdq->put(PICK_KEYBOARD1);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(DISCOVER);
                }
                return false;
            case LT(_FN2, SNOWFOX_BLE_KB2):
                if (record->tap.count) {
                    ble_cmdq->put(PICK_KEYBOARD2);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(TOGGLE);
                } else {
                    ble_cmdq->put(PICK_KEYBOARD2);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(DISCOVER);
                }
                return false;
            case LT(_FN2, SNOWFOX_BLE_KB3):
                if (record->tap.count) {
                    ble_cmdq->put(PICK_KEYBOARD3);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(TOGGLE);
                } else {
                    ble_cmdq->put(PICK_KEYBOARD3);
                    ble_cmdq->put(QUERY_KEYBOARD);
                    ble_cmdq->put(DISCOVER);
                }
                return false;
            default:
                break;
        }
    }
    return true;
}

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  //debug_matrix=true;
  debug_keyboard=true;
  //debug_mouse=true;
}


