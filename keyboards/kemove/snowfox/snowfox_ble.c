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
#include <string.h>


BLEState ble_state = OFF;
BLEKeyboard ble_keyboard = UNKNOWN;
uint8_t ble_led_status = 0;

mutex_t ble_ok_mutex;
condition_variable_t ble_ok_cond;
uint8_t ble_ok;

static char uart_rx_buffer[BLE_UART_BUFFER_SIZE];

/* chibios-driver is not assigned until the usersapce initialization has finished.
   Therefore there's no way to story the default qmk driver over USB/serial during
   initialization stage in userspace. */
host_driver_t *qmk_default_driver = NULL;

/* -------------------- Static Function Prototypes -------------------------- */
static uint8_t snowfox_ble_leds(void);
static void snowfox_ble_mouse(report_mouse_t *report);
static void snowfox_ble_keyboard(report_keyboard_t *report);
static void snowfox_ble_system(uint16_t data);
static void snowfox_ble_consumer(uint16_t data);

static void snowfox_ble_swtich_ble_driver(void);
static void snowfox_ble_update_name(void);

static void ble_command(const char* cmd);
static void ble_command_withOk(const char* cmd);

static void set_ok(void);
static void clear_ok(void);
static uint8_t read_uart_msg(char *buffer);

/* -------------------- Static Local Variables ------------------------------ */
static host_driver_t snowfox_ble_driver = {
    snowfox_ble_leds,
    snowfox_ble_keyboard,
    snowfox_ble_mouse,
    snowfox_ble_system,
    snowfox_ble_consumer,
};

#ifdef NKRO_ENABLE
static bool lastNkroStatus = false;
#endif // NKRO_ENABLE


/* -------------------- BLE ready semaphore --------------------------------- */
void set_ok() {
    chMtxLock(&ble_ok_mutex);
    ble_ok = 1;
    chCondSignal(&ble_ok_cond);
    chMtxUnlock(&ble_ok_mutex);
}

void clear_ok() {
    chMtxLock(&ble_ok_mutex);
    ble_ok = 0;
    chMtxUnlock(&ble_ok_mutex);
}

static uint8_t read_uart_msg() {
    char *p_write = uart_rx_buffer;
    char *p_end = uart_rx_buffer + BLE_UART_BUFFER_SIZE;
    uint8_t length = 0;

    while(p_write < p_end - 1) {
        msg_t raw = sdGetTimeout(&SD1, TIME_MS2I(10));

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

/* -------------------- Static Function Implementation ---------------------- */
static uint8_t snowfox_ble_leds(void) {
    return ble_led_status;
}
static void snowfox_ble_mouse(report_mouse_t *report) {}
static void snowfox_ble_keyboard(report_keyboard_t *report) {
    uint8_t buffer[15] = {'A', 'T', '+', 'H', 'I', 'D','=', 0x1,
    report->raw[0], report->raw[2], report->raw[3], report->raw[4], report->raw[5], report->raw[6], report->raw[7]};
    sdWrite(&SD1, buffer, 15);
}
static void snowfox_ble_system(uint16_t data) {}
static void snowfox_ble_consumer(uint16_t data) {}

static void snowfox_ble_swtich_ble_driver(void) {
    if (host_get_driver() == &snowfox_ble_driver) {
        return;
    }
    clear_keyboard();
#ifdef NKRO_ENABLE
    lastNkroStatus = keymap_config.nkro;
#endif
    keymap_config.nkro = false;
    qmk_default_driver = host_get_driver();
    host_set_driver(&snowfox_ble_driver);
}

static void snowfox_restore_driver(void) {
    /* Skip if the driver is already enabled */
    if (host_get_driver() != &snowfox_ble_driver) {
        ble_command("dbg: skip_restore_drv\r\n");
        return;
    }

    clear_keyboard();
#ifdef NKRO_ENABLE
    keymap_config.nkro = lastNkroStatus;
#endif
    host_set_driver(qmk_default_driver);
}

static void snowfox_ble_update_name() {
    char cmd[BLE_UART_BUFFER_SIZE] = "AT+NAME=SnowfoxQMK?\r\n";
    cmd[18] = ble_keyboard + '0';
    ble_command_withOk(cmd);
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

static void switch_led_page(uint8_t page) {
    if (page <= 9) {
        ble_led_status = page;
    } else { // fallback, should not happen
        ble_led_status = 0;
    }
}

static void update_event(uint8_t flag) {
    switch (flag){
        case BLE_EVENT_POST:
            ble_state = STANDBY;
            break;

        case BLE_EVENT_DISCOVER:
            ble_state = DISCOVERING;
            break;

        case BLE_EVENT_CONNECTED:
            ble_state = CONNECTED;
            snowfox_ble_swtich_ble_driver();
            break;

        case BLE_EVENT_DROP:
            ble_state = STANDBY;
            snowfox_restore_driver();
            break;

        case BLE_EVENT_CONNECTING:
            ble_state = CONNECTING;
            break;

        default:
    }
}

static void process_response(char* buffer) {
    if (strncmp("OK", buffer, 2) == 0) {
        set_ok();
    }
    
    else if (strncmp("+LEDPAGE:", buffer, 9) == 0) {
        switch_led_page(buffer[9] - (uint8_t)'0');
    }

    else if (strncmp("+KEYBOARD:", buffer, 10) == 0) {
        ble_keyboard = buffer[10] - '0';
        snowfox_ble_update_name();
    }
    
    else if (strncmp("+EVENT:", buffer, 7) == 0) {
        update_event((uint8_t) strtol(buffer+7, NULL, 16));
    }
}

/* BLE THREAD */
THD_WORKING_AREA(waBLEThread, 128);
THD_FUNCTION(BLEThread, arg) {
    (void)arg;
    chRegSetThreadName("BLEThread");
    snowfox_ble_startup();

    while(1) {
        uint8_t length = read_uart_msg(uart_rx_buffer);
        if (length > 0) {
            process_response(uart_rx_buffer);
            memset(uart_rx_buffer, 0, BLE_UART_BUFFER_SIZE);
        }
    }
}

/* -------------------- Public Function Implementation ---------------------- */

bool snowfox_ble_is_active(void) {
    return host_get_driver() == &snowfox_ble_driver;
}

void snowfox_ble_startup(void) {
    sdStart(&SD1, &serialCfg);
    palClearLine(LINE_BLE_RSTN);
    chThdSleepMilliseconds(1);
    palSetLine(LINE_BLE_RSTN);
    chThdSleepMilliseconds(500);
    ble_command("\r\n");
    ble_command("AT+UART=115200\r\n");
    sdStop(&SD1);
    serialCfg.speed=115200;
    sdStart(&SD1, &serialCfg);
    chThdSleepMilliseconds(300);
    while(1) {
        chThdSleepMilliseconds(50);
        if (ble_check_ok()) {
            break;
        }
    }
    ble_command_withOk("AT+UART=115200\r\n");
    ble_command_withOk("AT+KEYBOARD?\r\n");
    ble_command_withOk("AT+DISCONN\r\n");
}

static void ble_command_withOk(const char* string) {
    chMtxLock(&ble_ok_mutex);
    ble_ok = 0;
    ble_command(string);
    while(ble_ok == 0) {
        if(chCondWaitTimeout(&ble_ok_cond, TIME_MS2I(100)) == MSG_TIMEOUT) {
            break;
        }
    }
    ble_ok = 0;
    chMtxUnlock(&ble_ok_mutex);
}

void snowfox_ble_select(BLEKeyboard port) {
    switch (port) {
        case KEYBOARD1:
            ble_command_withOk("AT+KEYBOARD=1\r\n");
            break;
        case KEYBOARD2:
            ble_command_withOk("AT+KEYBOARD=2\r\n");
            break;
        case KEYBOARD3:
            ble_command_withOk("AT+KEYBOARD=3\r\n");
            break;
        default:
    }
    ble_keyboard = port;
    snowfox_ble_update_name();
}

void snowfox_ble_discover(void) {
    ble_command_withOk("AT+DISCOVER\r\n");
}
void snowfox_ble_connect(void) {
    ble_command_withOk("AT+CONN\r\n");
}
void snowfox_ble_disconnect(void) {
    ble_command_withOk("AT+DISCONN\r\n");
}
