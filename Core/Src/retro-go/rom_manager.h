#pragma once

#include <stdint.h>

#include "rg_emulators.h"

typedef struct {
    const char *rom_name;
    const uint8_t *flash_address;
    uint32_t size;
    const uint8_t *save_address;
    uint32_t save_size;
    rom_region_t region;
} rom_entry_t;

typedef struct {
    char system_name[64];
    const rom_entry_t *roms;
    char *extension;
    uint32_t roms_count;
} rom_system_t;

typedef struct {
    const rom_system_t *systems;
    uint32_t systems_count;
} rom_manager_t;

extern const rom_manager_t rom_mgr;
extern unsigned char *ROM_DATA;
extern unsigned ROM_DATA_LENGTH;
extern retro_emulator_file_t *ACTIVE_FILE;

const rom_system_t *rom_manager_system(const rom_manager_t *mgr, char *name);
void rom_manager_set_active_file(retro_emulator_file_t *file);
