#include <stdint.h>
#include "snowfox.h"
#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [MAC_BASE] = LAYOUT_ansi_61( /* Base */
    KC_GRAVE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL,   KC_BSPC,
    KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC,  KC_BSLS,
    LT(_FN1, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT,         KC_ENT,
    KC_LSFT,KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, RSFT_T(KC_UP),
    KC_LCTL,KC_LALT, KC_LGUI,      KC_SPC,             KC_RGUI,  LT(_FN2, KC_LEFT),  RALT_T(KC_DOWN), RCTL_T(KC_RIGHT)
  ),
  [WIN_BASE] = LAYOUT_ansi_61( /* Base */
    KC_GRAVE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL,   KC_BSPC,
    KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC,  KC_BSLS,
    LT(_FN1, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT,         KC_ENT,
    KC_LSFT,KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, RSFT_T(KC_UP),
    KC_LCTL,KC_LGUI, KC_LALT,      KC_SPC,             KC_RALT,  LT(_FN2, KC_LEFT),  LWIN_T(KC_DOWN), RCTL_T(KC_RIGHT)
  ),
  [_FN1] = LAYOUT_ansi_61( /* Base */
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_HOME, KC_END,  KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, KC_PGUP, KC_PGDN,         KC_TRNS,
    KC_TRNS,KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DELETE, KC_END, KC_PGDN, KC_DELETE, KC_UP,
    KC_TRNS,KC_TRNS, KC_TRNS,      KC_TRNS,             KC_TRNS,  KC_LEFT,  KC_DOWN, KC_RIGHT
  ),
  [_FN2] = LAYOUT_ansi_61( /* Base */
    KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,   SNOWFOX_LED_OFF,
    SNOWFOX_LED_NEXT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_TRNS, KC_PAUS, SNOWFOX_LED_BDN, SNOWFOX_LED_BUP, SNOWFOX_LED_OFF,  SNOWFOX_LED_ON,
    KC_CAPS, KC_MPLY, KC_MPRV, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, KC_INS, KC_HOME, KC_PGUP, SNOWFOX_LED_PDN, SNOWFOX_LED_PUP,         KC_TRNS,
    KC_TRNS, LT(_FN2, SNOWFOX_BLE_KB1), LT(_FN2, SNOWFOX_BLE_KB2), LT(_FN2, SNOWFOX_BLE_KB3), KC_TRNS, KC_TRNS, KC_DEL, KC_END, KC_PGDN, KC_TRNS, KC_DELETE, KC_PGUP,
    KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,             KC_TRNS,  KC_TRNS,  KC_PGDN, KC_PGDN
  ),
  [_FN3] = LAYOUT_ansi_61( /* Base */
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_HOME, KC_END,  KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, KC_PGUP, KC_PGDN,         KC_TRNS,
    KC_TRNS,KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DELETE, KC_END, KC_PGDN, KC_DELETE, KC_UP,
    KC_TRNS,KC_TRNS, KC_TRNS,      KC_TRNS,             KC_TRNS,  KC_LEFT,  KC_DOWN, KC_RIGHT
  ),
};

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  //debug_matrix=true;
  debug_keyboard=true;
  //debug_mouse=true;
}

