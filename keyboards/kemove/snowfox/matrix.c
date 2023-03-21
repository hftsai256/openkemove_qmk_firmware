/*
 * Copyright (c) 2020 Yaotian Feng / Codetector
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

#include "quantum.h"

static matrix_row_t matrix[MATRIX_COLS];
static matrix_row_t matrix_debouncing[MATRIX_COLS];
static uint32_t debounce_times[MATRIX_COLS];

ioline_t row_list[] = {B25, A0,  B20, A5,  A7,  B19, B23, B24};
ioline_t col_list[] = {B28, B31, A22, B29, A11, A12, A13, A14, B13};

void matrix_init(void) {
    memset(matrix, 0, MATRIX_COLS * sizeof(matrix_row_t));
    memset(matrix_debouncing, 0, MATRIX_COLS * sizeof(matrix_row_t));
    memset(debounce_times, 0, MATRIX_COLS * sizeof(uint32_t));

    matrix_init_kb();
}

uint8_t matrix_scan(void) {
    // cache of input ports for columns
    static uint32_t port_cache[2];
    // scan each col
    for (int col = 0; col < MATRIX_COLS; col++) {
        palClearLine(col_list[col]);
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        // read i/o ports
        port_cache[0] = palReadPort(IOPORT0);
        port_cache[1] = palReadPort(IOPORT1);
        //palSetLine(col_list[col]);

        // get columns from ports
        matrix_row_t data = 0;
        for (int row = 0; row < MATRIX_ROWS; ++row) {
            ioline_t line = row_list[row];
            uint32_t port = port_cache[LPC_IOPORT_NUM(PAL_PORT(line))];
            data |= (((port & (1 << PAL_PAD(line))) ? 0 : 1) << row); // Inverted
        }

        // if a key event happens <5ms before the system time rolls over,
        // the event will "never" debounce
        // but any event on the same row will reset the debounce timer
        if (matrix_debouncing[col] != data) {
            // whenever row changes restart debouncing
            matrix_debouncing[col] = data;
            debounce_times[col] = timer_read32();
        } else if(debounce_times[col] && timer_elapsed32(debounce_times[col]) >= DEBOUNCE){
            // when debouncing complete, update matrix
            matrix[col] = matrix_debouncing[col];
            debounce_times[col] = 0;
        }
    }

    matrix_scan_kb();
    return 1;
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[col] & (1 << row));
}

matrix_row_t matrix_get_row(uint8_t row)
{
    matrix_row_t data = 0;
    for (uint8_t i = 0; i < MATRIX_COLS; ++i)
    {
        data |= ((matrix[i] >> row) & 1U) << i;
    }
    return data;
}

void matrix_print(void) {}

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

