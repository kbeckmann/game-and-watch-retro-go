TARGET = gb

DEBUG = 1

OPT = -O2 -ggdb

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Src/porting/nes/common.c \
Core/Src/bilinear.c \
Core/Src/gw_buttons.c \
Core/Src/gw_flash.c \
Core/Src/gw_lcd.c \
Core/Src/main.c \
Core/Src/porting/odroid_audio.c \
Core/Src/porting/odroid_display.c \
Core/Src/porting/odroid_input.c \
Core/Src/porting/odroid_netplay.c \
Core/Src/porting/odroid_overlay.c \
Core/Src/porting/odroid_sdcard.c \
Core/Src/porting/odroid_system.c \
Core/Src/porting/crc32.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/system_stm32h7xx.c

GNUBOY_C_SOURCES = \
Core/Src/porting/gb/main_gb.c \
retro-go-stm32/gnuboy-go/components/gnuboy/cpu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/debug.c \
retro-go-stm32/gnuboy-go/components/gnuboy/emu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/hw.c \
retro-go-stm32/gnuboy-go/components/gnuboy/lcd.c \
retro-go-stm32/gnuboy-go/components/gnuboy/loader.c \
retro-go-stm32/gnuboy-go/components/gnuboy/mem.c \
retro-go-stm32/gnuboy-go/components/gnuboy/rtc.c \
retro-go-stm32/gnuboy-go/components/gnuboy/sound.c \

NES_C_SOURCES = \
Core/Src/porting/nes/main_nes.c \
Core/Src/porting/nes/nofrendo_stm32.c \
retro-go-stm32/nofrendo-go/components/nofrendo/bitmap.c \
retro-go-stm32/nofrendo-go/components/nofrendo/cpu/dis6502.c \
retro-go-stm32/nofrendo-go/components/nofrendo/cpu/nes6502.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map000.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map001.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map002.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map003.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map004.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map005.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map007.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map008.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map009.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map010.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map011.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map015.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map016.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map018.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map019.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map024.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map032.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map033.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map034.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map040.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map041.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map042.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map046.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map050.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map064.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map065.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map066.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map070.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map073.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map075.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map078.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map079.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map085.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map087.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map093.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map094.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map160.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map162.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map193.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map228.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map229.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/map231.c \
retro-go-stm32/nofrendo-go/components/nofrendo/mappers/mapvrc.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_apu.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_input.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mem.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_mmc.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_ppu.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_rom.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes_state.c \
retro-go-stm32/nofrendo-go/components/nofrendo/nes/nes.c

SMSPLUSGX_C_SOURCES = \
retro-go-stm32/smsplusgx-go/components/smsplus/loadrom.c \
retro-go-stm32/smsplusgx-go/components/smsplus/render.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sms.c \
retro-go-stm32/smsplusgx-go/components/smsplus/state.c \
retro-go-stm32/smsplusgx-go/components/smsplus/vdp.c \
retro-go-stm32/smsplusgx-go/components/smsplus/pio.c \
retro-go-stm32/smsplusgx-go/components/smsplus/tms.c \
retro-go-stm32/smsplusgx-go/components/smsplus/memz80.c \
retro-go-stm32/smsplusgx-go/components/smsplus/system.c \
retro-go-stm32/smsplusgx-go/components/smsplus/cpu/z80.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/emu2413.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/fmintf.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/sn76489.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/sms_sound.c \
retro-go-stm32/smsplusgx-go/components/smsplus/sound/ym2413.c \
Core/Src/porting/smsplusgx/main_smsplusgx.c

C_INCLUDES +=  \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/cpu \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/mappers \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/nes \
-Iretro-go-stm32/nofrendo-go/components/nofrendo \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-Iretro-go-stm32/smsplusgx-go/components/smsplus \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/cpu \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/sound


C_DEFS += \
-DIS_LITTLE_ENDIAN \
-DDISABLE_AHBRAM_DCACHE


#REQUIRED_FILE=roms/gb/loaded_gb_rom.c
#REQUIRED_FILE_MSG=Please run ./update_gb_rom.sh to import a GB ROM file


######################################
# building variables
######################################
# debug build?
DEBUG ?= 1
# optimization
OPT ?= -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR ?= build_$(TARGET)



# Common C sources
C_SOURCES +=  \
retro-go-stm32/components/lupng/lupng.c \
retro-go-stm32/components/miniz/miniz.c \
retro-go-stm32/retro-go/main/gui.c \
Core/Src/porting/gw_alloc.c \
Core/Src/retro-go/rg_main.c \
Core/Src/retro-go/rg_emulators.c \
Core/Src/retro-go/rom_manager.c \
Core/Src/porting/odroid_settings.c \
Core/Src/retro-go/bitmaps/header_gb.c \
Core/Src/retro-go/bitmaps/header_nes.c \
Core/Src/retro-go/bitmaps/header_sms.c \
Core/Src/retro-go/bitmaps/logo_gb.c \
Core/Src/retro-go/bitmaps/logo_nes.c \
Core/Src/retro-go/bitmaps/logo_sms.c


# Version and URL for the STM32CubeH7 SDK
SDK_VERSION ?= v1.8.0
SDK_URL ?= https://raw.githubusercontent.com/STMicroelectronics/STM32CubeH7

# Local path for the SDK
SDK_DIR ?= Drivers



# SDK C sources
SDK_C_SOURCES =  \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \


# SDK ASM sources
SDK_ASM_SOURCES =  \
Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b0xx.s

# SDK headers
SDK_HEADERS = \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7b0xx.h \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7xx.h \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/system_stm32h7xx.h \
Drivers/CMSIS/Include/cmsis_compiler.h \
Drivers/CMSIS/Include/cmsis_gcc.h \
Drivers/CMSIS/Include/cmsis_version.h \
Drivers/CMSIS/Include/core_cm7.h \
Drivers/CMSIS/Include/mpu_armv7.h \
Drivers/STM32H7xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_cortex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dac_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dac.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_def.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dma_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dma.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_exti.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_flash_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_flash.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_gpio_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_gpio.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_hsem.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_i2c_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_i2c.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ltdc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ltdc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_mdma.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ospi.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_pwr_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_pwr.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rcc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rcc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rtc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rtc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_sai_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_sai.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal.h \


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS +=

# C defines
C_DEFS +=  \
-DUSE_HAL_DRIVER \
-DSTM32H7B0xx \
-DVECT_TAB_ITCM \
-DIS_LITTLE_ENDIAN \
-D_FORTIFY_SOURCE=1 \
-DDEBUG_RG_ALLOC

# AS includes
AS_INCLUDES +=

# C includes
C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/retro-go \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/CMSIS/Include \
-Iretro-go-stm32/components/miniz \
-Iretro-go-stm32/components/lupng \

# compile gcc flags
ASFLAGS += $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT ?= STM32H7B0VBTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR +=
LDFLAGS += $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR) $(BUILD_DIR)/nes $(BUILD_DIR)/gnuboy $(BUILD_DIR)/smsplusgx $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET)_extflash.bin

#$(REQUIRED_FILE):
#	$(error $(REQUIRED_FILE_MSG))

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/core/,$(notdir $(C_SOURCES:.c=.o) $(SDK_C_SOURCES:.c=.o)))
GNUBOY_OBJECTS = $(addprefix $(BUILD_DIR)/gnuboy/,$(notdir $(GNUBOY_C_SOURCES:.c=.o)))
NES_OBJECTS = $(addprefix $(BUILD_DIR)/nes/,$(notdir $(NES_C_SOURCES:.c=.o)))
SMSPLUSGX_OBJECTS = $(addprefix $(BUILD_DIR)/smsplusgx/, $(notdir $(SMSPLUSGX_C_SOURCES:.c=.o)))

vpath %.c $(sort $(dir $(C_SOURCES) $(NES_C_SOURCES) $(GNUBOY_C_SOURCES) $(SMSPLUSGX_C_SOURCES) $(SDK_C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(SDK_ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(SDK_ASM_SOURCES)))

# function used to generate prerequisite rules for SDK objects
define sdk_obj_prereq_gen
$(BUILD_DIR)/$(patsubst %.c,%.o,$(patsubst %.s,%.o,$(notdir $1))): $1

endef
# note: the blank line above is intentional

# generate all object prerequisite rules
$(eval $(foreach obj,$(SDK_C_SOURCES) $(SDK_ASM_SOURCES),$(call sdk_obj_prereq_gen,$(obj))))

Core/Inc/githash.h:
	./githash.sh > $@
#.PHONY: Core/Inc/githash.h

$(BUILD_DIR)/core/%.o: %.c Core/Inc/githash.h
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/nes/%.o: %.c
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/nes/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/gnuboy/%.o: %.c
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/gnuboy/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/smsplusgx/%.o: %.c
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/smsplusgx/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s $(LDSCRIPT) | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(NES_OBJECTS) $(GNUBOY_OBJECTS) $(SMSPLUSGX_OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(NES_OBJECTS) $(GNUBOY_OBJECTS) $(SMSPLUSGX_OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@
	./size.sh $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR):
	mkdir $@
	mkdir $@/core
	mkdir $@/nes
	mkdir $@/gnuboy
	mkdir $@/smsplusgx


#######################################
# Flashing
#######################################

OPENOCD ?= openocd
OCDIFACE ?= interface/jlink.cfg
TRANSPORT ?= swd

# Starts openocd and attaches to the target. To be used with 'flashx' and 'gdb'
openocd:
	$(OPENOCD) -f $(OCDIFACE) -c "transport select $(TRANSPORT)" -f "target/stm32h7x.cfg" -c "reset_config none; init; halt"
.PHONY: openocd


# Flashes using a new openocd instance
flash: $(BUILD_DIR)/$(TARGET).elf
	$(OPENOCD) -f $(OCDIFACE) -c "transport select $(TRANSPORT)" -f "target/stm32h7x.cfg" -c "reset_config none; program $(BUILD_DIR)/$(TARGET).elf reset exit"
.PHONY: flash


# Flash without building or so
jflash:
	$(OPENOCD) -f $(OCDIFACE) -c "transport select $(TRANSPORT)" -f "target/stm32h7x.cfg" -c "reset_config none; program $(BUILD_DIR)/$(TARGET).elf reset exit"
.PHONY: jflash


# Flashes using an existing openocd instance
flashx: $(BUILD_DIR)/$(TARGET).elf
	echo "reset_config none; program $(BUILD_DIR)/$(TARGET).elf; reset run; exit" | nc localhost 4444
.PHONY: flashx


FLASHLOADER ?= ../game-and-watch-flashloader/flash.sh
flash_extmem: $(BUILD_DIR)/$(TARGET)_extflash.bin
	$(FLASHLOADER) $(BUILD_DIR)/$(TARGET)_extflash.bin
.PHONY: flash_extmem


# Programs both the external and internal flash.
flash_all:
	$(MAKE) -f Makefile.$(TARGET) flash_extmem
	$(MAKE) -f Makefile.$(TARGET) flash
.PHONY: flash_all


GDB ?= $(PREFIX)gdb
gdb: $(BUILD_DIR)/$(TARGET).elf
	$(GDB) $< -ex "target extended-remote :3333"
.PHONY: gdb


#######################################
# download SDK files
#######################################
$(SDK_DIR)/%:
	wget $(SDK_URL)/$(SDK_VERSION)/$@ -P $(dir $@)

.PHONY: download_sdk
download_sdk: $(SDK_HEADERS) $(SDK_C_SOURCES) $(SDK_ASM_SOURCES)

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

distclean: clean
	rm -rf $(SDK_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

$(BUILD_DIR)/$(TARGET)_extflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(BIN) -j ._itcram_hot -j ._ram_exec -j ._extflash -j .overlay_nes -j .overlay_gb -j .overlay_sms $< $(BUILD_DIR)/$(TARGET)_extflash.bin

