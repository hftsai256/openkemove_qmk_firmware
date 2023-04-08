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

cmd_queue_api* ble_cmdq;

/* -------------------- Static Function Prototypes -------------------------- */
static cmd_queue_api* queue_init(void);

static uint8_t snowfox_ble_leds(void);
static void custom_ble_reset(void);

static void uart_start(uint32_t baud);
static void uart_stop(void);
static void bluetooth_custom_on(void);
static void bluetooth_custom_off(void);
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

static _cmd_queue_t _cqdata = {0};

static char uart_rx_buffer[BLE_UART_BUFFER_SIZE];
const static char SENTINEL[] = "\r\n";
/* -------------------- Static Function Implementation ---------------------- */
static bool _queue_empty(void) {
    return (_cqdata.counter == 0);
}

static bool _queue_full(void) {
    return (_cqdata.counter == BLE_MAX_COMMAND_QUEUE);
}

static ble_queue_cmd_t _queue_pop(void) {
    if (_cqdata.counter > 0) {
        _cqdata.counter--;
    } else {
        return 0;
    }

    ble_queue_cmd_t ret = _cqdata.buffer[_cqdata.p_start++];
    if (_cqdata.p_start >= BLE_MAX_COMMAND_QUEUE) {
        _cqdata.p_start -= BLE_MAX_COMMAND_QUEUE;
    }
    return ret;
}

static void _queue_put(ble_queue_cmd_t cmd) {
    if (_cqdata.counter < BLE_MAX_COMMAND_QUEUE) {
        _cqdata.counter++;
    } else {
        dprint("BLE command queue full\n");
        return;
    }

    _cqdata.buffer[_cqdata.p_end++] = cmd;
    if (_cqdata.p_end >= BLE_MAX_COMMAND_QUEUE) {
        _cqdata.p_end -= BLE_MAX_COMMAND_QUEUE;
    }
}

static cmd_queue_api* queue_init(void) {
    memset(&_cqdata, 0, sizeof(_cqdata));
    static cmd_queue_api handle = {
        .is_empty = _queue_empty,
        .is_full = _queue_full,
        .put = _queue_put,
        .pop = _queue_pop
    };
    return &handle;
}

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

static void bluetooth_custom_on(void) {
    palSetLine(LINE_BLE_RSTN);
}

static void bluetooth_custom_off(void) {
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

static void ble_command(const char* raw_cmd) {
    dprintf("BLE UART Tx: %s\n", raw_cmd);
    sdWrite(&SD1, (uint8_t*) raw_cmd, strlen(raw_cmd));
    sdWrite(&SD1, (uint8_t*) SENTINEL, strlen((char*) SENTINEL));
}

static void ble_command_lock(const char* raw_cmd) {
    ble_command(raw_cmd);
    ack_take();
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
            clear_keyboard();
            keymap_config.nkro = false;
            break;

        case BLE_EVENT_DROP:
            ble_handle.state = STANDBY;
            clear_keyboard();
            keymap_config.nkro = true;
            break;

        case BLE_EVENT_CONNECTING:
            ble_handle.state = CONNECTING;
            break;

        default:
            break;
    }
    ble_update_kb(&ble_handle);
}

static void dispatch_command(cmd_queue_api* qu) {
    if (qu->is_empty())
        return;

    ble_queue_cmd_t cmd = qu->pop();
    switch (cmd) {
        case CONNECT:
            ble_command_lock("AT+CONN");
            break;
        case DISCOVER:
            ble_command_lock("AT+DISCOVER");
            break;
        case DROP_CONN:
            ble_command_lock("AT+DISCONN");
            break;
        case TOGGLE:
            if (ble_handle.state == STANDBY) {
                qu->put(CONNECT);
            } else {
                qu->put(DROP_CONN);
            }
        case CHANGE_NAME:
            ble_command_lock("AT+NAME=SnowfoxQMK");
            break;
        case PICK_KEYBOARD1:
            ble_handle.keyboard = BLE_KEYBOARD1;
            ble_command_lock("AT+KEYBOARD=1");
            break;
        case PICK_KEYBOARD2:
            ble_handle.keyboard = BLE_KEYBOARD2;
            ble_command_lock("AT+KEYBOARD=2");
            break;
        case PICK_KEYBOARD3:
            ble_handle.keyboard = BLE_KEYBOARD3;
            ble_command_lock("AT+KEYBOARD=3");
            break;
        case QUERY_KEYBOARD:
            ble_command_lock("AT+KEYBOARD?");
            break;
        default:
            break;
    }
}

static void process_response(char* buffer) {
    dprintf("BLE UART Rx: %s\n", buffer);
    if (strncmp("OK", buffer, 2) == 0) {
        ack_release();
    }

    else if (strncmp("+LEDPAGE:", buffer, 9) == 0) {
        ble_handle.led_page = strtol(buffer+9, NULL, 16);
    }

    else if (strncmp("+KEYBOARD:", buffer, 10) == 0) {
        ble_handle.keyboard = strtol(buffer+10, NULL, 10) - 1;
    }

    else if (strncmp("+EVENT:", buffer, 7) == 0) {
        update_event((uint8_t) strtol(buffer+7, NULL, 16));
    }
}

/* -------------------- Public Function Implementation ---------------------- */
void bluetooth_custom_init(void) {
    uart_start(9600);
    bluetooth_custom_on();
    wait_ms(10);

    ble_command("AT+UART=115200");

    uart_stop();
    uart_start(115200);
    ble_cmdq = queue_init();
    ble_cmdq->put(CHANGE_NAME);
    ble_cmdq->put(QUERY_KEYBOARD);
}

void bluetooth_custom_task(void) {
    static char* p_write = uart_rx_buffer;

    if (bluetooth_custom_is_connected()) {
        led_update_kb((led_t) ble_handle.led_page);
    }

    while(!ble_cmdq->is_empty()) {
        if (ack_pass()) {
            dispatch_command(ble_cmdq);
        } else {
            break;
        }
    }

    while(1) {
        msg_t raw = sdGetTimeout(&SD1, TIME_IMMEDIATE);

        if (raw == MSG_TIMEOUT || raw == MSG_RESET) {
            break;
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

bool bluetooth_custom_is_connected(void) {
    return (ble_handle.state == CONNECTED);
}

void bluetooth_custom_send_keyboard(report_keyboard_t *report) {
#ifdef CONSOLE_ENABLE
    dprintf("uart tx: [%d bytes]: ", sizeof(report->raw));
    for (uint8_t i=0; i < sizeof(report->raw); i++) {
      dprintf(" %02X", ((uint8_t*)report)[i]);
    }
    dprint("\n");
#endif 
    sdWrite(&SD1, (uint8_t*) "AT+HID=\1", 8);
    sdWrite(&SD1, (uint8_t*) &report->mods, sizeof(report->mods));
    sdWrite(&SD1, (uint8_t*) &report->keys, sizeof(report->keys));
}

void bluetooth_custom_send_mouse(report_mouse_t *report) {}
void bluetooth_custom_send_consumer(uint16_t usage) {}


