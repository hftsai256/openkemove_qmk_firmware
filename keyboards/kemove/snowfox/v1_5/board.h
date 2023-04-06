#pragma once
#include_next <board.h>

#define LINE_DIP0                                       PAL_LINE(IOPORT0, 20) // 3.3v (left) pin

#define LINE_SSP1_MOSI                                  PAL_LINE(IOPORT0, 21)
#define LINE_SSP1_MISO                                  PAL_LINE(IOPORT1, 21)
#define LINE_SSP1_SCK                                   PAL_LINE(IOPORT1, 15)
#define LINE_LED1_CS                                    PAL_LINE(IOPORT0, 19)
#define LINE_LED2_CS                                    PAL_LINE(IOPORT1, 16)

#define LINE_UART_RX                                    PAL_LINE(IOPORT1, 26) // To pin 21 on BLE
#define LINE_UART_TX                                    PAL_LINE(IOPORT1, 27) // To pin 20 on BLE
#define LINE_BLE_RSTN                                   PAL_LINE(IOPORT0, 23) // To pin 11 on BLE

#define LINE_EEPROM_CS                                  PAL_LINE(IOPORT0, 18)
#define LINE_EEPROM_DO                                  PAL_LINE(IOPORT0,  8) // MISO0
#define LINE_EEPROM_DI                                  PAL_LINE(IOPORT0,  9) // MOSI0
#define LINE_EEPROM_CLK                                 PAL_LINE(IOPORT0, 10) // SCLK0
#define LINE_EEPROM_HOLD                                PAL_LINE(IOPORT0, 17) // WP

#define LINE_ADC_BAT                                    PAL_LINE(IOPORT0, 15) // To Battery
