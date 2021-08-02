#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "rom_manager.h"

#include "gw_lcd.h"
#include "gw_linker.h"


void app_main_snes(uint8_t load_state, uint8_t start_paused)
{
    printf("Snes9x Load: %s\n", ACTIVE_FILE->name);

    while (1) {
        HAL_Delay(1000);
        printf("Hello!\n");
        __NOP();
    }
}
