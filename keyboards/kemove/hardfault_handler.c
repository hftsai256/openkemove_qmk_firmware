#include <board.h>
#include "ch.h"
#include "hal.h"

#define bkpt() __asm volatile("BKPT #0\n")
OSAL_IRQ_HANDLER(HardFault_Handler) {
    bkpt();
    NVIC_SystemReset(); // If no debugger connected, just reset the board
}
