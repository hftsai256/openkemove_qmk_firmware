# KEMOVE_SNOWFOX
SRC = matrix.c \
	  sled17341.c \
      snowfox_ble.c \
      snowfox_led.c \
      hardfault_handler.c

# MCU
MCU = cortex-m0
ARMV = 6
USE_FPU = no
MCU_FAMILY = LPC
MCU_SERIES = LPC11Uxx
MCU_LDSCRIPT = LPC11U35_401_app
MCU_STARTUP = lpc11uxx

OPT_DEFS = -Wno-unused-function -fdump-rtl-dfinit -fstack-usage
EXTRALDFLAGS = -Wl,--print-memory-usage

# Options
DEBOUNCE_TYPE = sym_defer_g

# Keys
CUSTOM_MATRIX = lite
NKRO_ENABLE = no
MOUSEKEY_ENABLE = yes
EXTRAKEY_ENABLE = yes
KEY_LOCK_ENABLE = no

# Other featues
BOOTMAGIC_ENABLE = yes
CONSOLE_ENABLE = yes
COMMAND_ENABLE = yes
RAW_ENABLE = no
MIDI_ENABLE = no
VIRTSER_ENABLE = no
COMBO_ENABLE = no
BLUETOOTH_ENABLE = yes
BLUETOOTH_DRIVER = custom
