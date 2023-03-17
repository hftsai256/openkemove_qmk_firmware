# List of all the board related files.
#BOARDSRC = $(BOARD_PATH)/boards/KEMOVE_SNOWFOX/v1_5/board.c

# Required include directories
#BOARDINC = $(BOARD_PATH)/boards/KEMOVE_SNOWFOX/v1_5

#BOARD = KEMOVE_SNOWFOX/v1_5

DIP_SWITCH_ENABLE = yes
#EEPROM_DRIVER = wear_leveling
#WEAR_LEVELING_DRIVER = embedded_flash

OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE

