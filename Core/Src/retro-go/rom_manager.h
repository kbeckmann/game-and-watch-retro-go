#pragma once

#include <stdint.h>


typedef struct {
    const char *rom_name;
    uint32_t flash_address;
    uint32_t size;
} rom_entry;

typedef struct {
    char system_name[64];
    const rom_entry *roms;
    char *extension;
    uint32_t roms_count;
} rom_system;

typedef struct {
    const rom_system *systems;
    uint32_t systems_count;
} rom_manager;

extern const rom_manager rom_mgr;

const rom_system *rom_manager_system(const rom_manager *mgr, char *name);
