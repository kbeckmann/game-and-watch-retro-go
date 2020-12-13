#include "rom_manager.h"
#include <string.h>
#include "gb_rom.h"

unsigned char *ROM_DATA = NULL;
unsigned ROM_DATA_LENGTH;

#include "gb_roms.c"
#include "nes_roms.c"

const rom_system systems[] = {
    nes_system,
    gb_system
};

const rom_manager rom_mgr = {
    .systems = systems,
    .systems_count = sizeof(systems) / sizeof(rom_system)
};

const rom_system *rom_manager_system(const rom_manager *mgr, char *name) {
    for(int i=0; i < mgr->systems_count; i++) {
        if(strcmp(mgr->systems[i].system_name, name) == 0) {
            return &mgr->systems[i];
        }
    }
    return NULL;
}
