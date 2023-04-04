#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "report.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void ble_custom_init(void);
extern void ble_custom_task(void);
extern void ble_custom_send_keyboard(report_keyboard_t *report);
extern void ble_custom_send_mouse(report_mouse_t *report);
extern void ble_custom_send_consumer(uint16_t usage);
extern bool ble_custom_is_connected(void);

#ifdef __cplusplus
}
#endif

