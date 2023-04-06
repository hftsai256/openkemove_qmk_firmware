 /*
 * Copyright (c) 2018 Charlie Waters
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define ENABLE_SLEEP_LED    TRUE
#define MAX_LAYERS          5
#define DIP_SWITCH_PINS     { LINE_DIP0 }

/*
#define KEYMAP( 
    K00, K01, K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, 
    K10, K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, 
    K20, K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C,      
    K30,      K31, K32, K33, K34, K35, K36, K37, K38, K39, K3A, K3B,      
    K40, K41, K42,                K43,           K44,  K45,   K46,   K47  
) { 
             COL0   COL1   COL2   COL3   COL4   COL5   COL6   COL7   COL8     
     ROW0   { K00,   K01,   K02,   K03,   K04,   K05,   K06,   K07,   K08,  },
     ROW1   { K10,   K11,   K12,   K13,   K14,   K15,   K16,   K17,   K18,  },
     ROW2   { K20,   K21,   K22,   K23,   K24,   K25,   K26,   K27,   K28,  },
     ROW3   { K30,   KC_NO, K31,   K32,   K33,   K34,   K35,   K36,   K37,  },
     ROW4   { K40,   KC_NO, K41,   K39,   K3A,   K42,   KC_NO, K46,   KC_NO,},
     ROW5   { K09,   K0A,   K0B,   K0C,   K0D,   KC_NO, K43,   K45,   K44,  },
     ROW6   { K19,   K1A,   K1B,   K1D,   K2A,   KC_NO, KC_NO, K47,   KC_NO,},
     ROW7   { K2B,   K29,   K1C,   K38,   K3B,   KC_NO, KC_NO, K2C,   KC_NO,},
}
*/

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

/*
 * Force NKRO
 *
 * Force NKRO (nKey Rollover) to be enabled by default, regardless of the saved
 * state in the bootmagic EEPROM settings. (Note that NKRO must be enabled in the
 * makefile for this to work.)
 *
 * If forced on, NKRO can be disabled via magic key (default = LShift+RShift+N)
 * until the next keyboard reset.
 *
 * NKRO may prevent your keystrokes from being detected in the BIOS, but it is
 * fully operational during normal computer usage.
 *
 * For a less heavy-handed approach, enable NKRO via magic key (LShift+RShift+N)
 * or via bootmagic (hold SPACE+N while plugging in the keyboard). Once set by
 * bootmagic, NKRO mode will always be enabled until it is toggled again during a
 * power-up.
 *
 */
#define NKRO_ENABLE
#define FORCE_NKRO

