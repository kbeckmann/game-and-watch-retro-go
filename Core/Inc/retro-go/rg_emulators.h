#pragma once

#include <odroid_sdcard.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    REGION_NTSC = 0,
    REGION_PAL
} rom_region_t;

typedef struct rom_system_t rom_system_t;

typedef struct {
    const char *name;
    const char *ext;
    // char folder[32];
    const uint8_t *address;
    size_t size;
    const uint8_t *save_address;
    uint32_t save_size;
    size_t crc_offset;
    uint32_t checksum;
    bool missing_cover;
    rom_region_t region;
    const rom_system_t *system;
} retro_emulator_file_t;

typedef struct {
    char system_name[64];
    char dirname[16];
    char ext[8];
    uint16_t crc_offset;
    uint16_t partition;
    struct {
        const retro_emulator_file_t *files;
        int count;
    } roms;
    bool initialized;
    const rom_system_t *system;
} retro_emulator_t;

void emulators_init();
void emulator_init(retro_emulator_t *emu);
void emulator_start(retro_emulator_file_t *file, bool load_state);
void emulator_show_file_menu(retro_emulator_file_t *file);
void emulator_show_file_info(retro_emulator_file_t *file);
void emulator_crc32_file(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);
const char *emu_get_file_path(retro_emulator_file_t *file);
retro_emulator_t *file_to_emu(retro_emulator_file_t *file);
