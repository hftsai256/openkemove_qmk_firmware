/*
    Copyright (C) 2020 Yaotian Feng, Codetector<codetector@codetector.cn>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "snowfox_ble.h"
#include "snowfox_led.h"
#include "ch.h"
#include "hal.h"
#include "host.h"
#include "host_driver.h"
#include "printf.h"

/* -------------------- Static Function Prototypes -------------------------- */
static uint8_t snowfox_ble_leds(void);
static void custom_ble_reset(void);

static void uart_start(uint32_t baud);
static void uart_stop(void);
static void ble_custom_on(void);
static void ble_custom_off(void);
static void ble_command(const char* cmd);
static void ble_command_lock(const char* cmd);
static void ack_take(void);
static void ack_release(void);
static bool ack_pass(void);

/* -------------------- Static Local Variables ------------------------------ */
ble_handle_t ble_handle = {
    .state = OFF,
    .keyboard = BLE_KEYBOARD1,
    .led_page = 0,
    .ack_lock = false,
    .ack_lock_timestmp = 0,
};

static char uart_rx_buffer[BLE_UART_BUFFER_SIZE];
const static char SENTINEL[] = "\r\n";

/* -------------------- Static Function Implementation ---------------------- */
static void uart_start(uint32_t baud) {
    SerialConfig cfg;
    if (baud > 0) {
        cfg.speed = baud;
    } else {
        cfg.speed = 9600;
    }
    if (SD1.state == SD_STOP) {
        sdStart(&SD1, &cfg);
    }
}

static void uart_stop(void) {
    if (SD1.state == SD_READY) {
        sdStop(&SD1);
    }
}

static void ble_custom_on(void) {
    palSetLine(LINE_BLE_RSTN);
}

static void ble_custom_off(void) {
    palClearLine(LINE_BLE_RSTN);
}

static void ack_take(void) {
    while (!ack_pass()) {
        wait_ms(100);
    }
    ble_handle.ack_lock = true;
    ble_handle.ack_lock_timestmp = timer_read32();
}

static void ack_release(void) {
    ble_handle.ack_lock = false;
    ble_handle.ack_lock_timestmp = 0;
}

static bool ack_pass(void) {
    if (ble_handle.ack_lock == true) {
        uint32_t elapsed_time = timer_elapsed32(ble_handle.ack_lock_timestmp);
        if (elapsed_time >= BLE_LOCK_TIMEOUT_MS) {
            ack_release();
        }
    }
    return !ble_handle.ack_lock;
}

static uint8_t snowfox_ble_leds(void) {
    return ble_handle.led_page;
}

static void ble_command(const char* raw_cmd) {
    sdWrite(&SD1, (uint8_t*) raw_cmd, strlen(raw_cmd));
    sdWrite(&SD1, (uint8_t*) SENTINEL, strlen((char*) SENTINEL));
}

static void ble_command_lock(const char* raw_cmd) {
    ble_command(raw_cmd);
    ack_take();
}

static void switch_led_page(uint8_t page) {
    if (page <= 9) {
        ble_handle.led_page = page;
    } else { // fallback, should not happen
        ble_handle.led_page = 0;
    }
}

static void update_event(uint8_t flag) {
    switch (flag){
        case BLE_EVENT_POST:
            ble_handle.state = STANDBY;
            break;

        case BLE_EVENT_DISCOVER:
            ble_handle.state = DISCOVERING;
            break;

        case BLE_EVENT_CONNECTED:
            ble_handle.state = CONNECTED;
            break;

        case BLE_EVENT_DROP:
            ble_handle.state = STANDBY;
            break;

        case BLE_EVENT_CONNECTING:
            ble_handle.state = CONNECTING;
            break;

        default:
            break;
    }
    ble_update_kb(&ble_handle);
}

static void process_response(char* buffer) {
    if (strncmp("OK", buffer, 2) == 0) {
        ack_release();
    }

    else if (strncmp("+LEDPAGE:", buffer, 9) == 0) {
        switch_led_page((uint8_t) strtol(buffer+9, NULL, 16));
    }

    else if (strncmp("+KEYBOARD:", buffer, 10) == 0) {
        ble_handle.keyboard = strtol(buffer+10, NULL, 10) - 1;
    }

    else if (strncmp("+EVENT:", buffer, 7) == 0) {
        update_event((uint8_t) strtol(buffer+7, NULL, 16));
    }
}

/* -------------------- Public Function Implementation ---------------------- */
void ble_custom_init(void) {
    uart_start(9600);
    ble_custom_on();
    wait_ms(10);

    ble_command("AT+UART=115200");

    uart_stop();
    uart_start(115200);
    ble_command("AT+NAME=SnowfoxQMK");
}

void ble_custom_task(void) {
    static char* p_write = uart_rx_buffer;

    while(1) {
        msg_t raw = sdGetTimeout(&SD1, TIME_IMMEDIATE);

        if (raw == MSG_TIMEOUT || raw == MSG_RESET) {
            if (!ack_pass()) {
                wait_ms(10);
                continue;
            } else {
                break;
            }
        }

        if (raw == '\r' || raw == '\n') {
            if (p_write > uart_rx_buffer) {
                process_response(uart_rx_buffer);

                memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
                p_write = uart_rx_buffer;
            }
        } else {
            *p_write++ = (char) raw;
        }
    }
}

bool ble_custom_is_connected(void) {
    return (ble_handle.state == CONNECTED);
}

void ble_custom_send_keyboard(report_keyboard_t *report) {
    sdWrite(&SD1, (uint8_t*) "AT+HID=\1", 8);
    sdWrite(&SD1, (uint8_t*) &report->mods, sizeof(report->mods));
    sdWrite(&SD1, (uint8_t*) &report->keys, sizeof(report->keys));
}

void ble_custom_send_mouse(report_mouse_t *report) {}
void ble_custom_send_consumer(uint16_t usage) {}

void snowfox_ble_select(ble_keyboard_t port) {
    switch (port) {
        case BLE_KEYBOARD1:
            ble_command_lock("AT+KEYBOARD=1");
            break;
        case BLE_KEYBOARD2:
            ble_command_lock("AT+KEYBOARD=2");
            break;
        case BLE_KEYBOARD3:
            ble_command_lock("AT+KEYBOARD=3");
            break;
        default:
            break;
    }
    ble_handle.keyboard = port;
    snowfox_ble_connect();
}

void snowfox_ble_discover(void) {
    ble_command_lock("AT+DISCOVER");
}
void snowfox_ble_connect(void) {
    ble_command_lock("AT+CONN");
}
void snowfox_ble_disconnect(void) {
    ble_command_lock("AT+DISCONN");
}

