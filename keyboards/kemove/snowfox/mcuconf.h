/*
    Copyright (C) 2020 Yaotian Feng

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

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

#include "nvic.h"

#define LPC11U35_MCUCONF

#define LPC_USE_SYSOSC                              TRUE
#define LPC_SYSOSC_FREQUENCY                        12000000UL // 12MHz
#define LPC_SYSPLLCLKSEL                            SYSCON_SYSPLLCLKSEL_SYSOSC
#define LPC_MAINCLKSEL                              SYSCON_MAINCLKSEL_PLLOUT
#define LPC_SYSPLL_MULT                             4   // MVALUE 4 -> bits 0b00011
#define LPC_SYSPLL_PDIV                             4   // PSEL 2 -> bits 0b01
#define LPC_SYS_DIV                                 1   // NO SYS DIV

#define LPC_USB_USE_USB1                            TRUE
#define LPC_USB_USB1_IRQ_PRIORITY                   3

#define LPC_SPI_USE_SPI1                            TRUE
#define LPC_SPI_SPI1_IRQ_PRIORITY                   2

#define LPC_UART_USE_UART1                          FALSE
#define LPC_UART_UART1_IRQ_PRIORITY                 1

#define LPC_SERIAL_USE_UART1                        TRUE


#endif /* _MCUCONF_H_ */
