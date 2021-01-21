#pragma once

#include <stdint.h>

#include "rg_emulators.h"

struct rom_system_t {
    char *system_name;
    const retro_emulator_file_t *roms;
    char *extension;
    uint32_t roms_count;
};

typedef struct {
    const rom_system_t **systems;
    uint32_t systems_count;
} rom_manager_t;

extern const rom_manager_t rom_mgr;
extern const unsigned char *ROM_DATA;
extern unsigned ROM_DATA_LENGTH;
extern retro_emulator_file_t *ACTIVE_FILE;

const rom_system_t *rom_manager_system(const rom_manager_t *mgr, char *name);
void rom_manager_set_active_file(retro_emulator_file_t *file);
