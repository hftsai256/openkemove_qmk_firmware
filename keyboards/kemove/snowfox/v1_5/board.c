#include "quantum.h"
#include "dip_switch.h"

void early_hardware_init_post(void) {
    // Matrix column GPIOs
    palSetLineMode(B28, MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B31, MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A22, MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B29, MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A11, MODE_FUNC_ALT1 | MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A12, MODE_FUNC_ALT1 | MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A13, MODE_FUNC_ALT1 | MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A14, MODE_FUNC_ALT1 | MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B13, MODE_DIR_OUT | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);

    // Matrix row GPIOs
    palSetLineMode(B25, MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A0 , MODE_FUNC_ALT1 | MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B20, MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A5 , MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(A7 , MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B19, MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B23, MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    palSetLineMode(B24, MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);

    // Peripherals
    palSetLineMode(LINE_SSP1_MOSI, MODE_FUNC_ALT2 | MODE_AD_DIGITAL); // SSP1 MOSI
    palSetLineMode(LINE_SSP1_MISO, MODE_FUNC_ALT2 | MODE_AD_DIGITAL); // SSP1 MISO
    palSetLineMode(LINE_SSP1_SCK, MODE_FUNC_ALT3 | MODE_AD_DIGITAL);  // SSP1 SCK
    palSetLineMode(LINE_LED1_CS, MODE_DIR_OUT | MODE_AD_DIGITAL);
    palSetLineMode(LINE_LED2_CS, MODE_DIR_OUT | MODE_AD_DIGITAL);

    palSetLineMode(LINE_UART_RX, MODE_FUNC_ALT2 | MODE_AD_DIGITAL);
    palSetLineMode(LINE_UART_TX, MODE_FUNC_ALT2 | MODE_AD_DIGITAL);

    palSetLineMode(LINE_BLE_RSTN, MODE_DIR_OUT | MODE_AD_DIGITAL);
    palClearLine(LINE_BLE_RSTN);

    // Analog
    palSetLineMode(LINE_ADC_BAT, MODE_FUNC_ALT2 | MODE_AD_ANALOG);
}

void dip_switch_init(void) {
#ifdef DIP_SWITCH_PINS
    ioline_t dip_switch_pad[] = DIP_SWITCH_PINS;
    for (uint8_t i=0; i < sizeof(dip_switch_pad)/sizeof(dip_switch_pad[0]); i++) {
        palSetLineMode(dip_switch_pad[i], MODE_DIR_IN | MODE_MODE_PULL_UP | MODE_AD_DIGITAL);
    }
    dip_switch_read(true);
#endif
}
