#pragma once

#define SMSPLUSGX_ENGINE_SG1000  2
#define SMSPLUSGX_ENGINE_COLECO  1
#define SMSPLUSGX_ENGINE_OTHERS  0

int app_main_smsplusgx(uint8_t load_state, uint8_t start_paused, uint8_t is_coleco);
