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
#include "ch.h"
#include "hal.h"
#include "host.h"
#include "host_driver.h"
#include "printf.h"
#include <string.h>


/* -------------------- Static Function Prototypes -------------------------- */
static uint8_t snowfox_ble_leds(void);
static void snowfox_ble_mouse(report_mouse_t *report);
static void snowfox_ble_keyboard(report_keyboard_t *report);
static void snowfox_ble_system(uint16_t data);
static void snowfox_ble_consumer(uint16_t data);

static void snowfox_ble_reset(void);
static void snowfox_ble_swtich_ble_driver(void);
static void snowfox_ble_update_name(void);

static void ble_command(const char* cmd);
static void ble_command_wait(const char* cmd, const uint16_t waittime_ms);
static uint8_t ble_check_ok(void);

static uint8_t read_uart_msg(char *buffer);

/* -------------------- Static Local Variables ------------------------------ */
static host_driver_t snowfox_ble_driver = {
    snowfox_ble_leds,
    snowfox_ble_keyboard,
    snowfox_ble_mouse,
    snowfox_ble_system,
    snowfox_ble_consumer,
};

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

/* -------------------- Static Function Implementation ---------------------- */
static uint8_t snowfox_ble_leds(void) {
    return ble_handle.led_page;
}

static void snowfox_ble_mouse(report_mouse_t *report) {}

static void snowfox_ble_keyboard(report_keyboard_t *report) {
    sdWrite(&SD1, (uint8_t*) "AT+HID=\1", 8);
    sdWrite(&SD1, (uint8_t*) &report->mods, sizeof(report->mods));
    sdWrite(&SD1, (uint8_t*) &report->keys, sizeof(report->keys));
}

static void snowfox_ble_system(uint16_t data) {}

static void snowfox_ble_consumer(uint16_t data) {}

static void snowfox_ble_swtich_ble_driver(void) {
    if (host_get_driver() == &snowfox_ble_driver) {
        return;
    }
    clear_keyboard();
#ifdef NKRO_ENABLE
    ble_handle.last_nkro_state = keymap_config.nkro;
#endif
    keymap_config.nkro = false;
    ble_handle.last_driver = host_get_driver();
    host_set_driver(&snowfox_ble_driver);
}

static void snowfox_restore_driver(void) {
    /* Skip if the driver is already enabled */
    if (host_get_driver() != &snowfox_ble_driver) {
        return;
    }

    clear_keyboard();
#ifdef NKRO_ENABLE
    keymap_config.nkro = ble_handle.last_nkro_state;
#endif
    host_set_driver(ble_handle.last_driver);
}

static void snowfox_ble_reset() {
    if (SD1.state == SD_READY) {
        sdStop(&SD1);
    }
    serialCfg.speed = 9600;
    sdStart(&SD1, &serialCfg);
    palClearLine(LINE_BLE_RSTN);
    chThdSleepMilliseconds(1);
    palSetLine(LINE_BLE_RSTN);
    chThdSleepMilliseconds(500);

    ble_command("AT+UART=115200\r\n");

    sdStop(&SD1);
    serialCfg.speed=115200;
    sdStart(&SD1, &serialCfg);

    chThdSleepMilliseconds(50);
}

static void snowfox_ble_update_name() {
    char buffer[24];
    sprintf(buffer, "AT+NAME=SnowfoxQMK:%d", ble_handle.keyboard+1);
    ble_command_wait(buffer, 100);
}

static void ble_command(const char* cmd) {
    sdWrite(&SD1, (uint8_t*)cmd, strlen(cmd));
}

static uint8_t ble_check_ok(void) {
    uint8_t length = read_uart_msg(uart_rx_buffer);
    uint8_t result = 0;
    if (strncmp("OK", uart_rx_buffer, length) == 0) {
        result = 1;
    }

    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
    return result;
}

static void ble_command_wait(const char* cmd, const uint16_t waittime_ms) {
    ble_command(cmd);
    chThdSleepMilliseconds(waittime_ms);
}

static void switch_led_page(uint8_t page) {
    if (page <= 9) {
        ble_handle.led_page = page;
    } else { // fallback, should not happen
        ble_handle.led_page = 0;
    }
}

void ble_update_kb(ble_handle_t);

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
            snowfox_ble_swtich_ble_driver();
            break;

        case BLE_EVENT_DROP:
            ble_handle.state = STANDBY;
            snowfox_restore_driver();
            break;

        case BLE_EVENT_CONNECTING:
            ble_handle.state = CONNECTING;
            break;

        default:
    }
    ble_update_kb(ble_handle);
}

static void process_response(char* buffer) {
    if (strncmp("OK", buffer, 2) == 0) {
        // TODO: join scheduled tasks
    }

    else if (strncmp("+LEDPAGE:", buffer, 9) == 0) {
        switch_led_page((uint8_t) strtol(buffer+9, NULL, 16));
    }

    else if (strncmp("+KEYBOARD:", buffer, 10) == 0) {
        ble_handle.keyboard = strtol(buffer+10, NULL, 10) - 1;
        snowfox_ble_update_name();
    }

    else if (strncmp("+EVENT:", buffer, 7) == 0) {
        update_event((uint8_t) strtol(buffer+7, NULL, 16));
    }
}

/* -------------------- BLE ready semaphore --------------------------------- */
static uint8_t read_uart_msg() {
    char *p_write = uart_rx_buffer;
    char *p_end = uart_rx_buffer + BLE_UART_BUFFER_SIZE;
    uint8_t length = 0;

    while(p_write < p_end - 1) {
        msg_t raw = sdGetTimeout(&SD1, TIME_MS2I(100));

        if (raw == MSG_TIMEOUT || raw == MSG_RESET || raw == '\r' || raw == '\n') {
            break;

        } else {
            *p_write++ = (char) raw;
            length++;
        }
    }
    *p_write++ = 0x0; // Null terminator
    length++;

    return length;
}

/* BLE THREAD */
THD_WORKING_AREA(waBLEThread, 128);
THD_FUNCTION(BLEThread, arg) {
    (void)arg;
    chRegSetThreadName("BLEThread");

    snowfox_ble_reset();
    ble_command("AT+DISCONN\r\n");
    ble_command("AT+KEYBOARD?\r\n");

    while(1) {
        uint8_t length = read_uart_msg(uart_rx_buffer);
        if (length > 0) {
            process_response(uart_rx_buffer);
            memset(uart_rx_buffer, 0, BLE_UART_BUFFER_SIZE);
        }
    }
}

/* -------------------- Public Function Implementation ---------------------- */
void snowfox_ble_select(ble_keyboard_t port) {
    switch (port) {
        case BLE_KEYBOARD1:
            ble_command_wait("AT+KEYBOARD=1\r\n", 100);
            break;
        case BLE_KEYBOARD2:
            ble_command_wait("AT+KEYBOARD=2\r\n", 100);
            break;
        case BLE_KEYBOARD3:
            ble_command_wait("AT+KEYBOARD=3\r\n", 100);
            break;
        default:
    }
    ble_handle.keyboard = port;
    snowfox_ble_update_name();
    snowfox_ble_reset();
    snowfox_ble_connect();
}

void snowfox_ble_discover(void) {
    ble_command_wait("AT+DISCOVER\r\n", 100);
}
void snowfox_ble_connect(void) {
    ble_command_wait("AT+CONN\r\n", 100);
}
void snowfox_ble_disconnect(void) {
    ble_command_wait("AT+DISCONN\r\n", 100);
}

bool snowfox_ble_is_active(void) {
    return host_get_driver() == &snowfox_ble_driver;
}
