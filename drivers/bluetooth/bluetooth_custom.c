#include "bluetooth_custom.h"

__attribute__((weak)) void bluetooth_custom_init(void) {} 
__attribute__((weak)) void bluetooth_custom_task(void) {}
__attribute__((weak)) void bluetooth_custom_send_keyboard(report_keyboard_t *report) {}
__attribute__((weak)) void bluetooth_custom_send_mouse(report_mouse_t *report) {}
__attribute__((weak)) void bluetooth_custom_send_consumer(uint16_t usage) {}
__attribute__((weak)) bool bluetooth_custom_is_connected(void) { return false; }


