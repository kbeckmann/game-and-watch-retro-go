TARGET = gw_retro_go

DEBUG = 1

OPT = -O2 -ggdb3

# To enable verbose, append VERBOSE=1 to make, e.g.:
# make VERBOSE=1
ifneq ($(strip $(VERBOSE)),1)
V = @
endif

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Src/bilinear.c \
Core/Src/gw_buttons.c \
Core/Src/gw_flash.c \
Core/Src/gw_lcd.c \
Core/Src/main.c \
Core/Src/bq24072.c \
Core/Src/porting/common.c \
Core/Src/porting/odroid_audio.c \
Core/Src/porting/odroid_display.c \
Core/Src/porting/odroid_input.c \
Core/Src/porting/odroid_netplay.c \
Core/Src/porting/odroid_overlay.c \
Core/Src/porting/odroid_sdcard.c \
Core/Src/porting/odroid_system.c \
Core/Src/porting/crc32.c \
Core/Src/porting/lib/lz4_depack.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/system_stm32h7xx.c

GNUBOY_C_SOURCES = \
Core/Src/porting/gb/main_gb.c \
Core/Src/porting/gb/loader.c \
retro-go-stm32/gnuboy-go/components/gnuboy/cpu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/debug.c \
retro-go-stm32/gnuboy-go/components/gnuboy/emu.c \
retro-go-stm32/gnuboy-go/components/gnuboy/hw.c \
retro-go-stm32/gnuboy-go/components/gnuboy/lcd.c \
retro-go-stm32/gnuboy-go/components/gnuboy/mem.c \
retro-go-stm32/gnuboy-go/components/gnuboy/rtc.c \
retro-go-stm32/gnuboy-go/components/gnuboy/sound.c \

NES_C_SOURCES = \
Core/Src/porting/nes/main_nes.c \
Core/Src/porting/nes/nofrendo_stm32.c \
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

PCE_C_SOURCES = \
retro-go-stm32/huexpress-go/components/huexpress/engine/gfx.c \
retro-go-stm32/huexpress-go/components/huexpress/engine/h6280.c \
retro-go-stm32/huexpress-go/components/huexpress/engine/hard_pce.c \
Core/Src/porting/pce/sound_pce.c \
Core/Src/porting/pce/main_pce.c 

C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/cpu \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/mappers \
-Iretro-go-stm32/nofrendo-go/components/nofrendo/nes \
-Iretro-go-stm32/nofrendo-go/components/nofrendo \
-Iretro-go-stm32/components/odroid \
-Iretro-go-stm32/gnuboy-go/components \
-Iretro-go-stm32/smsplusgx-go/components/smsplus \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/cpu \
-Iretro-go-stm32/smsplusgx-go/components/smsplus/sound \
-Iretro-go-stm32/huexpress-go/components/huexpress/engine 

C_DEFS += \
-DIS_LITTLE_ENDIAN \
-DDISABLE_AHBRAM_DCACHE


include Makefile.common


$(BUILD_DIR)/$(TARGET)_extflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j ._itcram_hot -j ._ram_exec -j ._extflash -j .overlay_nes -j .overlay_gb -j .overlay_sms -j .overlay_pce $< $(BUILD_DIR)/$(TARGET)_extflash.bin

$(BUILD_DIR)/$(TARGET)_intflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j .isr_vector -j .text -j .rodata -j .ARM.extab -j .preinit_array -j .init_array -j .fini_array -j .data $< $(BUILD_DIR)/$(TARGET)_intflash.bin

