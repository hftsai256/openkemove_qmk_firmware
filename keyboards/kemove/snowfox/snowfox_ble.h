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

#pragma once

#include "snowfox.h"
#include "quantum.h"
#include "ch.h"

#define BLE_UART_BUFFER_SIZE   32U

#define BLE_EVENT_POST         0x0001
#define BLE_EVENT_DISCOVER     0x0002
#define BLE_EVENT_CONNECTED    0x0004
#define BLE_EVENT_DROP         0x0008
#define BLE_EVENT_CONNECTING   0x0040

#define BLE_LOCK_TIMEOUT_MS    500
#define BLE_MAX_COMMAND_QUEUE  8

typedef enum {
    OFF = 0,
    STANDBY,
    DISCOVERING,
    CONNECTING,
    CONNECTED
} ble_state_t;

typedef enum {
    BLE_KEYBOARD1,
    BLE_KEYBOARD2,
    BLE_KEYBOARD3,
    BLE_KEYBOARD_SIZE
} ble_keyboard_t;

typedef enum {
    CONNECT = 1,
    DISCOVER,
    DROP_CONN,
    CHANGE_NAME,
    PICK_KEYBOARD1,
    PICK_KEYBOARD2,
    PICK_KEYBOARD3,
    BLE_QUEUE_COMMAND_SIZE
} ble_queue_cmd_t;

typedef struct {
    ble_state_t state;
    ble_keyboard_t keyboard;
    uint8_t led_page;
    bool ack_lock;
    uint32_t ack_lock_timestmp;
} ble_handle_t;

typedef struct {
    ble_queue_cmd_t buffer[BLE_MAX_COMMAND_QUEUE];
    size_t counter;
    size_t p_start;
    size_t p_end;
} _cmd_queue_t;

typedef struct {
    bool (*is_empty)(void);
    bool (*is_full)(void);
    void (*put)(ble_queue_cmd_t);
    ble_queue_cmd_t (*pop)(void);
} cmd_queue_api;

extern cmd_queue_api* ble_cmdq;

bool bluetooth_custom_is_connected(void);

