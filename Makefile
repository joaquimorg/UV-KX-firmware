
# 0 = disable
# 1 = enable

ENABLE_REMOTE_CONTROL			?= 0
ENABLE_UART_DEBUG			  	?= 0
ENABLE_TXRX_MSG                 ?= 0

# compile options (see README.md for descriptions)
# 0 = disable
# 1 = enable

# ---- STOCK QUANSHENG FEATURES ----
ENABLE_FMRADIO                  ?= 0
ENABLE_UART                     ?= 1
ENABLE_VOICE                    ?= 0
ENABLE_VOX                      ?= 0
ENABLE_ALARM                    ?= 0
ENABLE_TX1750                   ?= 1
ENABLE_DTMF_CALLING             ?= 0

# ---- CUSTOM MODS ----
ENABLE_SPECTRUM                 ?= 1
ENABLE_KEEP_MEM_NAME            ?= 1
ENABLE_WIDE_RX                  ?= 1
ENABLE_TX_WHEN_AM               ?= 0
ENABLE_F_CAL_MENU               ?= 0
ENABLE_CTCSS_TAIL_PHASE_SHIFT   ?= 0
ENABLE_BOOT_BEEPS               ?= 0
ENABLE_SHOW_CHARGE_LEVEL        ?= 0
ENABLE_NO_CODE_SCAN_TIMEOUT     ?= 1
ENABLE_AM_FIX                   ?= 1
ENABLE_SQUELCH_MORE_SENSITIVE   ?= 1
ENABLE_FASTER_CHANNEL_SCAN      ?= 1
ENABLE_RSSI_BAR                 ?= 1
ENABLE_AUDIO_BAR                ?= 0
ENABLE_COPY_CHAN_TO_VFO         ?= 0
ENABLE_REDUCE_LOW_MID_TX_POWER  ?= 0
ENABLE_BYP_RAW_DEMODULATORS     ?= 0
ENABLE_BLMIN_TMP_OFF            ?= 0
ENABLE_SCAN_RANGES              ?= 1

# ---- CONTRIB MODS ----

# Thank you @reppad
ENABLE_EXTRA_UART_CMD           ?= 0

# ---- F4HWN MODS ----

ENABLE_FEAT_F4HWN               ?= 1
ENABLE_FEAT_F4HWN_SPECTRUM      ?= 1
ENABLE_FEAT_F4HWN_RX_TX_TIMER   ?= 1
ENABLE_FEAT_F4HWN_SLEEP         ?= 1
ENABLE_FEAT_F4HWN_RESUME_STATE  ?= 0
ENABLE_FEAT_F4HWN_NARROWER      ?= 1
ENABLE_FEAT_F4HWN_CTR           ?= 1
ENABLE_FEAT_F4HWN_RESCUE_OPS    ?= 0
ENABLE_FEAT_F4HWN_VOL           ?= 0
ENABLE_FEAT_F4HWN_RESET_CHANNEL ?= 0
ENABLE_FEAT_F4HWN_PMR           ?= 0
ENABLE_FEAT_F4HWN_GMRS_FRS_MURS	?= 0
ENABLE_FEAT_F4HWN_CA            ?= 0
ENABLE_FEAT_F4HWN_DEBUG         ?= 0

# ---- DEBUGGING ----
ENABLE_AM_FIX_SHOW_DATA         ?= 0
ENABLE_AGC_SHOW_DATA            ?= 0
ENABLE_UART_RW_BK_REGS          ?= 0

#------------------------------------------------------------------------------
AUTHOR_NAME ?= JOAQUIM
AUTHOR_STRING ?= $(AUTHOR_NAME).ORG
VERSION_STRING ?= V0.0.1
PROJECT_NAME := uv-kx_$(VERSION_STRING)

AUTHOR_STRING_1 ?= EGZUMER
VERSION_STRING_1 ?= V0.22

AUTHOR_STRING_2 ?= F4HWN
VERSION_STRING_2 ?= V4.3

EDITION_STRING ?= $(AUTHOR_STRING)

#AUTHOR_STRING ?= $(AUTHOR_STRING_1)+$(AUTHOR_STRING_2)
#VERSION_STRING ?= $(VERSION_STRING_2)

BUILD := _build
BIN := firmware

MAKEFLAGS += -j4

EXTERNAL_LIB := external
LINKER := linker
BSP := bsp
SRC := src

LD_FILE := $(LINKER)/firmware.ld

#------------------------------------------------------------------------------
# Tool Configure
#------------------------------------------------------------------------------

PYTHON = python

# Toolchain commands
# Should be added to your PATH
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

# Set make directory command, Windows tries to create a directory named "-p" if that flag is there.
ifeq ($(OS), Windows_NT) # windows
#	MKDIR = mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0)
	MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
	RM = powershell -Command "Remove-Item -Recurse -Force"
#	RM = rmdir /s /q
	FixPath = $(subst /,\,$1)
	WHERE = where
	DEL = del /q
	K5PROG = utils/k5prog/k5prog.exe -D -F -YYYYY -p /dev/$(COMPORT) -b
	DEV_NULL = nul
	# Detect if PowerShell is available for directory listing
	PS_EXISTS := $(shell $(WHERE) powershell 2>$(DEV_NULL))
	MKDIR_IF_NOT_EXIST = if not exist "$(1)" $(MKDIR) "$(1)"
else
    MKDIR = mkdir -p
	RM = rm -rf
	FixPath = $1
    WHERE = which
    DEL = del
    K5PROG = utils/k5prog/k5prog -D -F -YYYYY -p /dev/$(COMPORT) -b	
	DEV_NULL = /dev/null
	MKDIR_IF_NOT_EXIST = [ -d "$(1)" ] || $(MKDIR) "$(1)"
endif

# Function to create directory if not exist
define ensure_dir
	@$(call MKDIR_IF_NOT_EXIST,$(1))
endef

ifneq (, $(shell $(WHERE) python))
	MY_PYTHON := python
else ifneq (, $(shell $(WHERE) python3))
	MY_PYTHON := python3
endif

ifdef MY_PYTHON
	HAS_CRCMOD := $(shell $(MY_PYTHON) -c "import crcmod" 2>&1)
endif

ifndef MY_PYTHON
$(info )
$(info !!!!!!!! PYTHON NOT FOUND, *.PACKED.BIN WON'T BE BUILT)
$(info )
else ifneq (,$(HAS_CRCMOD))
$(info )
$(info !!!!!!!! CRCMOD NOT INSTALLED, *.PACKED.BIN WON'T BE BUILT)
$(info !!!!!!!! run: pip install crcmod)
$(info )
endif

#------------------------------------------------------------------------------
# ASM flags
ASMFLAGS =
ASMFLAGS += -mcpu=cortex-m0

# C flags
CCFLAGS = 
CCFLAGS += -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -MMD -g
CCFLAGS += -flto=1
CCFLAGS += -ftree-vectorize -funroll-loops
CCFLAGS += -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unknown-pragmas 
#-Wunused-parameter -Wconversion
CCFLAGS += -fno-math-errno -pipe -ffunction-sections -fdata-sections -ffast-math -fno-strict-aliasing
CCFLAGS += -fsingle-precision-constant -finline-functions-called-once
CCFLAGS += -Os -g3 -fno-exceptions -fno-non-call-exceptions -fno-delete-null-pointer-checks
CCFLAGS += -DARMCM0

# C++ flags
#CXXFLAGS =
#CXXFLAGS += -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -MMD -g
#CXXFLAGS += -flto=1
#CXXFLAGS += -ftree-vectorize -funroll-loops
#CXXFLAGS += -Wextra -Wunused-parameter -Wconversion -Wno-unknown-pragmas
#CXXFLAGS += -fno-math-errno -pipe -ffunction-sections -fdata-sections -ffast-math
#CXXFLAGS += -fsingle-precision-constant -finline-functions-called-once
#CXXFLAGS += -std=c++20 -pedantic -Wno-expansion-to-defined -fno-rtti
#CXXFLAGS += -Os -g3 -fno-exceptions -fno-non-call-exceptions -fno-delete-null-pointer-checks
#CXXFLAGS += -DARMCM0
#-O3

# Linker flags
LDFLAGS =
LDFLAGS += -z noseparate-code -z noexecstack -mcpu=cortex-m0 -nostartfiles -Wl,-L,linker -Wl,-T,$(LD_FILE) -Wl,--gc-sections
LDFLAGS += -Wl,--build-id=none
LDFLAGS += -Wl,-Map,$(BUILD)/$(PROJECT_NAME).map

# Use newlib-nano instead of newlib
LDFLAGS += --specs=nano.specs -lc -lnosys -mthumb -mabi=aapcs -lm -fno-rtti -fno-exceptions

#show size
LDFLAGS += -Wl,--print-memory-usage

#------------------------------------------------------------------------------

CCFLAGS += -DPRINTF_INCLUDE_CONFIG_H
#CXXFLAGS += -DAUTHOR_NAME=\"$(AUTHOR_NAME)\" -DAUTHOR_STRING=\"$(AUTHOR_STRING)\" -DVERSION_STRING=\"$(VERSION_STRING)\"

ifeq ($(ENABLE_UART_DEBUG),1)
	CXXFLAGS += -DENABLE_UART_DEBUG
endif
ifeq ($(ENABLE_REMOTE_CONTROL),1)
	CXXFLAGS += -DENABLE_REMOTE_CONTROL
endif

ifeq ($(ENABLE_TXRX_MSG),1)
	CCFLAGS += -DENABLE_TXRX_MSG
endif

ifeq ($(ENABLE_SPECTRUM),1)
	CCFLAGS += -DENABLE_SPECTRUM
endif
ifeq ($(ENABLE_SWD),1)
	CCFLAGS += -DENABLE_SWD
endif
ifeq ($(ENABLE_OVERLAY),1)
	CCFLAGS += -DENABLE_OVERLAY
endif
ifeq ($(ENABLE_FMRADIO),1)
	CCFLAGS += -DENABLE_FMRADIO
endif
ifeq ($(ENABLE_UART),1)
	CCFLAGS += -DENABLE_UART
endif
ifeq ($(ENABLE_VOICE),1)
	CCFLAGS  += -DENABLE_VOICE
endif
ifeq ($(ENABLE_VOX),1)
	CCFLAGS  += -DENABLE_VOX
endif
ifeq ($(ENABLE_ALARM),1)
	CCFLAGS  += -DENABLE_ALARM
endif
ifeq ($(ENABLE_TX1750),1)
	CCFLAGS  += -DENABLE_TX1750
endif
ifeq ($(ENABLE_KEEP_MEM_NAME),1)
	CCFLAGS  += -DENABLE_KEEP_MEM_NAME
endif
ifeq ($(ENABLE_WIDE_RX),1)
	CCFLAGS  += -DENABLE_WIDE_RX
endif
ifeq ($(ENABLE_TX_WHEN_AM),1)
	CCFLAGS  += -DENABLE_TX_WHEN_AM
endif
ifeq ($(ENABLE_F_CAL_MENU),1)
	CCFLAGS  += -DENABLE_F_CAL_MENU
endif
ifeq ($(ENABLE_CTCSS_TAIL_PHASE_SHIFT),1)
	CCFLAGS  += -DENABLE_CTCSS_TAIL_PHASE_SHIFT
endif
ifeq ($(ENABLE_BOOT_BEEPS),1)
	CCFLAGS  += -DENABLE_BOOT_BEEPS
endif
ifeq ($(ENABLE_SHOW_CHARGE_LEVEL),1)
	CCFLAGS  += -DENABLE_SHOW_CHARGE_LEVEL
endif
ifeq ($(ENABLE_NO_CODE_SCAN_TIMEOUT),1)
	CCFLAGS  += -DENABLE_NO_CODE_SCAN_TIMEOUT
endif
ifeq ($(ENABLE_AM_FIX),1)
	CCFLAGS  += -DENABLE_AM_FIX
endif
ifeq ($(ENABLE_AM_FIX_SHOW_DATA),1)
	CCFLAGS  += -DENABLE_AM_FIX_SHOW_DATA
endif
ifeq ($(ENABLE_SQUELCH_MORE_SENSITIVE),1)
	CCFLAGS  += -DENABLE_SQUELCH_MORE_SENSITIVE
endif
ifeq ($(ENABLE_FASTER_CHANNEL_SCAN),1)
	CCFLAGS  += -DENABLE_FASTER_CHANNEL_SCAN
endif
ifeq ($(ENABLE_BACKLIGHT_ON_RX),1)
	CCFLAGS  += -DENABLE_BACKLIGHT_ON_RX
endif
ifeq ($(ENABLE_RSSI_BAR),1)
	CCFLAGS  += -DENABLE_RSSI_BAR
endif
ifeq ($(ENABLE_AUDIO_BAR),1)
	CCFLAGS  += -DENABLE_AUDIO_BAR
endif
ifeq ($(ENABLE_COPY_CHAN_TO_VFO),1)
	CCFLAGS  += -DENABLE_COPY_CHAN_TO_VFO
endif
ifeq ($(ENABLE_SINGLE_VFO_CHAN),1)
	CCFLAGS  += -DENABLE_SINGLE_VFO_CHAN
endif
ifeq ($(ENABLE_BAND_SCOPE),1)
	CCFLAGS += -DENABLE_BAND_SCOPE
endif
ifeq ($(ENABLE_REDUCE_LOW_MID_TX_POWER),1)
	CCFLAGS  += -DENABLE_REDUCE_LOW_MID_TX_POWER
endif
ifeq ($(ENABLE_BYP_RAW_DEMODULATORS),1)
	CCFLAGS  += -DENABLE_BYP_RAW_DEMODULATORS
endif
ifeq ($(ENABLE_BLMIN_TMP_OFF),1)
	CCFLAGS  += -DENABLE_BLMIN_TMP_OFF
endif
ifeq ($(ENABLE_SCAN_RANGES),1)
	CCFLAGS  += -DENABLE_SCAN_RANGES
endif
ifeq ($(ENABLE_DTMF_CALLING),1)
	CCFLAGS  += -DENABLE_DTMF_CALLING
endif
ifeq ($(ENABLE_AGC_SHOW_DATA),1)
	CCFLAGS  += -DENABLE_AGC_SHOW_DATA
endif
ifeq ($(ENABLE_UART_RW_BK_REGS),1)
	CCFLAGS  += -DENABLE_UART_RW_BK_REGS
endif
ifeq ($(ENABLE_FEAT_F4HWN),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN
	CCFLAGS  += -DALERT_TOT=10
	CCFLAGS  += -DSQL_TONE=550
	#CFLAGS  += -DSQL_TONE=$(SQL_TONE)
	CCFLAGS  += -DAUTHOR_STRING_1=\"$(AUTHOR_STRING_1)\" -DVERSION_STRING_1=\"$(VERSION_STRING_1)\"
	CCFLAGS  += -DAUTHOR_STRING_2=\"$(AUTHOR_STRING_2)\" -DVERSION_STRING_2=\"$(VERSION_STRING_2)\"
	CCFLAGS  += -DEDITION_STRING=\"$(EDITION_STRING)\"
else
	CCFLAGS  += -DSQL_TONE=550
endif
ifeq ($(ENABLE_FEAT_F4HWN_GAME),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_GAME
endif
ifeq ($(ENABLE_FEAT_F4HWN_SPECTRUM),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_SPECTRUM
endif
ifeq ($(ENABLE_FEAT_F4HWN_RX_TX_TIMER),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_RX_TX_TIMER
endif
ifeq ($(ENABLE_FEAT_F4HWN_SLEEP),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_SLEEP
endif
ifeq ($(ENABLE_FEAT_F4HWN_RESUME_STATE),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_RESUME_STATE
endif
ifeq ($(ENABLE_FEAT_F4HWN_NARROWER),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_NARROWER
endif
ifeq ($(ENABLE_FEAT_F4HWN_CTR),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_CTR
endif
ifneq ($(filter $(ENABLE_FEAT_F4HWN_RESCUE_OPS),1 2),)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_RESCUE_OPS=$(ENABLE_FEAT_F4HWN_RESCUE_OPS)
endif
ifeq ($(ENABLE_FEAT_F4HWN_VOL),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_VOL
endif
ifeq ($(ENABLE_FEAT_F4HWN_RESET_CHANNEL),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_RESET_CHANNEL
endif
ifeq ($(ENABLE_FEAT_F4HWN_PMR),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_PMR
endif
ifeq ($(ENABLE_FEAT_F4HWN_GMRS_FRS_MURS),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_GMRS_FRS_MURS
endif
ifeq ($(ENABLE_FEAT_F4HWN_CA),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_CA
endif
ifeq ($(ENABLE_FEAT_F4HWN_DEBUG),1)
	CCFLAGS  += -DENABLE_FEAT_F4HWN_DEBUG
endif
ifeq ($(ENABLE_EXTRA_UART_CMD),1)
	CCFLAGS  += -DENABLE_EXTRA_UART_CMD
endif

#------------------------------------------------------------------------------
ASM_SRC = $(LINKER)/start.S
ASM_OBJS = $(addprefix $(BUILD)/, $(ASM_SRC:.S=.o))

#------------------------------------------------------------------------------
# CMSIS Library source and object files

INCLUDE_PATH += $(EXTERNAL_LIB)/CMSIS_5/CMSIS/Core/Include/.
INCLUDE_PATH += $(EXTERNAL_LIB)/CMSIS_5/Device/ARM/ARMCM0/Include/.

#------------------------------------------------------------------------------
# printf Library source and object files
PRINTF_SRCS = $(wildcard $(EXTERNAL_LIB)/printf/*.c)
PRINTF_OBJS = $(addprefix $(BUILD)/, $(PRINTF_SRCS:.c=.o))

INCLUDE_PATH += $(EXTERNAL_LIB)/printf/.

#------------------------------------------------------------------------------
# u8g2 Library source and object files

U8G2_SRCS = $(wildcard $(EXTERNAL_LIB)/U8G2/csrc/*.c)
U8G2_OBJS = $(addprefix $(BUILD)/, $(U8G2_SRCS:.c=.o))

#U8G2_SRCSXX = $(wildcard $(EXTERNAL_LIB)/U8G2/cppsrc/*.cpp)
#U8G2_OBJSXX = $(addprefix $(BUILD)/, $(U8G2_SRCSXX:.cpp=.o))

INCLUDE_PATH += $(EXTERNAL_LIB)/U8G2/csrc/.
#INCLUDE_PATH += $(EXTERNAL_LIB)/U8G2/cppsrc/.
#------------------------------------------------------------------------------

# Main firmware source and object files
APP_SRCS += $(wildcard $(SRC)/*.c)
APP_SRCS += $(wildcard $(SRC)/radio/*.c)
APP_SRCS += $(wildcard $(SRC)/driver/*.c)
APP_SRCS += $(wildcard $(SRC)/helper/*.c)
APP_SRCS += $(wildcard $(SRC)/app/*.c)
APP_SRCS += $(wildcard $(SRC)/ui/*.c)
APP_OBJS = $(addprefix $(BUILD)/, $(APP_SRCS:.c=.o))

# Find all include directories recursively
#INCLUDE_PATH += /.
INCLUDE_PATH += $(BSP)/.
INCLUDE_PATH += $(EXTERNAL_LIB)/.
INCLUDE_PATH += $(SRC)/.
ifeq ($(PS_EXISTS),)
INCLUDE_PATH += $(shell find $(SRC) -type d)
else
INCLUDE_PATH += $(shell powershell -Command "Get-ChildItem -Path $(SRC) -Directory -Recurse | Resolve-Path -Relative")
endif

#------------------------------------------------------------------------------

INC_PATHS = $(addprefix -I,$(INCLUDE_PATH))

#------------------------------------------------------------------------------
# Phony targets
.PHONY: all app directories clean prog

# Default target
#all: $(BUILD) $(BUILD)/$(PROJECT_NAME).out $(BIN)

all: directories app

# Create necessary directories
directories:	
	$(call ensure_dir,$(BUILD))
	$(call ensure_dir,$(BIN))

#------------------------------------------------------------------------------

OBJECTS = $(ASM_OBJS) $(CMSIS_OBJS) $(PRINTF_OBJS) $(U8G2_OBJS) $(APP_OBJS)

-include $(OBJECTS:.o=.d)

$(BUILD)/%.o: %.c
	@echo CC $<
	$(call ensure_dir,$(@D))
	@$(CC) $(CCFLAGS) $(INC_PATHS) -c $< -o $@

#$(BUILD)/%.o: %.cpp
#	@echo GCC $<
#	$(call ensure_dir,$(@D))
#	@$(CXX) $(CXXFLAGS) $(INC_PATHS) -c $< -o $@


# Assemble files
$(BUILD)/%.o: %.S
	@echo AS $<
	$(call ensure_dir,$(@D))
	@$(CXX) -x assembler-with-cpp $(ASMFLAGS) $(INC_PATHS) -c $< -o $@
#------------------------------------------------------------------------------

# Main firmware
app: $(BUILD)/$(PROJECT_NAME).out

$(BUILD)/$(PROJECT_NAME).out: $(OBJECTS)
	@echo LD $@
	@$(CC) $(CCFLAGS) $(LDFLAGS) $^ -o $@

#------------------------------------------------------------------------------
#------------------- Binary generator -----------------------------------------	
	@echo Create $(notdir $@)
	@$(OBJCOPY) -O binary $(BUILD)/$(PROJECT_NAME).out $(BIN)/$(PROJECT_NAME).bin	

prog: all	
	@echo Create $(PROJECT_NAME).packed.bin
	@-$(MY_PYTHON) utils/fw-pack.py $(BIN)/$(PROJECT_NAME).bin $(AUTHOR_STRING) $(VERSION_STRING) $(BIN)/$(PROJECT_NAME).packed.bin
	@echo Flashing firmware to device...
	$(K5PROG) $(BIN)/$(PROJECT_NAME).bin

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

# Clean build artifacts
clean:
	@if exist $(BUILD) $(RM) $(BUILD)
	@if exist $(BIN) $(RM) $(BIN)

# Print help information
help:
	@echo Makefile targets:
	@echo   all     - Build all
	@echo   prog    - Flash firmware
	@echo   clean   - Remove all build artifacts
