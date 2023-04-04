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

static void ble_command(const char* cmd);
static void ble_command_wait(const char* cmd, const uint16_t waittime_ms);
static uint8_t ble_check_ok(void);

/* -------------------- Static Local Variables ------------------------------ */
ble_handle_t ble_handle = {
    .state = OFF,
    .keyboard = BLE_KEYBOARD1,
    .led_page = 0,

    /* chibios-driver is not assigned until the usersapce initialization has finished.
   Therefore there's no way to story the default qmk driver over USB/serial during
   initialization stage in userspace. */
    .last_driver = NULL
};

static char uart_rx_buffer[BLE_UART_BUFFER_SIZE];
const static char SENTINEL[] = "\r\n";

/* -------------------- Static Function Implementation ---------------------- */
static uint8_t snowfox_ble_leds(void) {
    return ble_handle.led_page;
}

static void ble_command(const char* raw_cmd) {
    sdWrite(&SD1, (uint8_t*) raw_cmd, strlen(raw_cmd));
    sdWrite(&SD1, (uint8_t*) SENTINEL, strlen((char*) SENTINEL));
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
#ifdef CONSOLE_ENABLE
    uprintf("UART Message Received: %s", buffer);
#endif 
    if (strncmp("OK", buffer, 2) == 0) {
        // TODO: join scheduled tasks
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
    serialCfg.speed = 9600;
    sdStart(&SD1, &serialCfg);
    palSetLine(LINE_BLE_RSTN);
    wait_ms(100);

    ble_command("AT+UART=115200");

    sdStop(&SD1);
    serialCfg.speed=115200;
    sdStart(&SD1, &serialCfg);
    ble_command("AT+NAME=SnowfoxQMK");
}

void ble_custom_stop(void) {
    if (SD1.state == SD_READY) {
        sdStop(&SD1);
    }
    palClearLine(LINE_BLE_RSTN);
}

void ble_custom_reset(void) {
    ble_custom_stop();
    wait_ms(10);
    ble_custom_init();
}

void ble_custom_task(void) {
    const char* SENTINEL_END = SENTINEL + strlen(SENTINEL);
    static char* p_write = uart_rx_buffer;
    static char* p_check = (char*) SENTINEL;

    while(1) {
        msg_t raw = sdGetTimeout(&SD1, TIME_IMMEDIATE);

        if (raw == MSG_TIMEOUT || raw == MSG_RESET) {
            break;
        }

        if (*p_check == raw) {
            p_check++;
            if (p_check >= SENTINEL_END) {
                *p_write = 0;
                process_response(uart_rx_buffer);

                p_write = uart_rx_buffer;
                p_check = (char*) SENTINEL;
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
            ble_command("AT+KEYBOARD=1");
            break;
        case BLE_KEYBOARD2:
            ble_command("AT+KEYBOARD=2");
            break;
        case BLE_KEYBOARD3:
            ble_command("AT+KEYBOARD=3");
            break;
        default:
            break;
    }
    ble_handle.keyboard = port;
    ble_custom_reset();
    snowfox_ble_connect();
}

void snowfox_ble_discover(void) {
    ble_command("AT+DISCOVER");
}
void snowfox_ble_connect(void) {
    ble_command("AT+CONN");
}
void snowfox_ble_disconnect(void) {
    ble_command("AT+DISCONN");
}

