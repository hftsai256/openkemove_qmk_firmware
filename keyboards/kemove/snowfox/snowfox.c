#include "snowfox.h"
#include "print.h"
#include "snowfox_ble.h"
#include "string.h"

thread_t *led_thread = NULL;

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
}

void bootloader_jump(void) {
    *((volatile uint32_t*) 0x100001F0) = 0xDEADBEEF;
    __asm__ __volatile__("dsb");
    SCB->AIRCR = 0x05FA0004; // Sys Reset
    __asm__ __volatile__("dsb");
    while(1) {}
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
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
                ble_cmdq->put(CONNECT);
                return false;
            case SNOWFOX_BLE_DISCOVER:
                ble_cmdq->put(DISCOVER);
                return false;
            case SNOWFOX_BLE_DISCONN:
                ble_cmdq->put(DROP_CONN);
                return false;
            case SNOWFOX_BLE_KB1:
                ble_cmdq->put(PICK_KEYBOARD1);
                ble_cmdq->put(CONNECT);
                return false;
            case SNOWFOX_BLE_KB2:
                ble_cmdq->put(PICK_KEYBOARD2);
                ble_cmdq->put(CONNECT);
                return false;
            case SNOWFOX_BLE_KB3:
                ble_cmdq->put(PICK_KEYBOARD3);
                ble_cmdq->put(CONNECT);
                return false;
            case SNOWFOX_DEBUG:
                return false;
            default:
                break;
        }
    }
    return process_record_user(keycode, record);
}

