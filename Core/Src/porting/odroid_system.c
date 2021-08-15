#include <assert.h>

#include "odroid_system.h"
#include "rom_manager.h"

static panic_trace_t *panicTrace = (void *)0x0;

static rg_app_desc_t currentApp;
static runtime_stats_t statistics;
static runtime_counters_t counters;
static uint skip;

void odroid_system_panic(const char *reason, const char *function, const char *file)
{
    printf("*** PANIC: %s\n  *** FUNCTION: %s\n  *** FILE: %s\n", reason, function, file);

    strcpy(panicTrace->message, reason);
    strcpy(panicTrace->file, file);
    strcpy(panicTrace->function, function);

    panicTrace->magicWord = PANIC_TRACE_MAGIC;

    while(1) {
        ;
    }
}

void odroid_system_init(int appId, int sampleRate)
{
    currentApp.id = appId;

    odroid_settings_init();
    odroid_audio_init(sampleRate);
    odroid_display_init();

    counters.resetTime = get_elapsed_time();

    printf("%s: System ready!\n\n", __func__);
}

void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb)
{
    // currentApp.gameId = crc32_le(0, buffer, sizeof(buffer));
    currentApp.gameId = 0;
    currentApp.loadState = load;
    currentApp.saveState = save;

    printf("%s: Init done. GameId=%08lX\n", __func__, currentApp.gameId);
}

rg_app_desc_t *odroid_system_get_app()
{
    return &currentApp;
}


bool odroid_system_emu_load_state(int slot)
{
    (*currentApp.loadState)("");
    return true;
}

bool odroid_system_emu_save_state(int slot)
{
    (*currentApp.saveState)("");
    return true;
}

IRAM_ATTR void odroid_system_tick(uint skippedFrame, uint fullFrame, uint busyTime)
{
    if (skippedFrame) counters.skippedFrames++;
    else if (fullFrame) counters.fullFrames++;
    counters.totalFrames++;

    // Because the emulator may have a different time perception, let's just skip the first report.
    if (skip) {
        skip = 0;
    } else {
        counters.busyTime += busyTime;
    }

    statistics.lastTickTime = get_elapsed_time();
}

extern uint32_t __INTFLASH__;  // From linker, usually value 0x08000000 for bank 1, or 0x08100000 for bank 2
void odroid_system_switch_app(int app)
{
    printf("%s: Switching to app %d.\n", __FUNCTION__, app);

    switch (app) {
    case 0:
        /**
         * Setting these two places in memory tell tim's patched firmware
         * bootloader running in bank 1 (0x08000000) to boot into retro-go
         * immediately instead of the patched-stock-firmware..
         *
         * These are the last 8 bytes of the 128KB of TCM RAM.
         *
         * This uses a technique described here:
         *      https://stackoverflow.com/a/56439572
         *
         *
         * For stuff not running a bootloader like this, these commands are
         * harmless.
         */
        *((uint32_t *)0x2001FFF8) = 0x544F4F42; // "BOOT"
        *((uint32_t *)0x2001FFFC) = &__INTFLASH__; // vector table

        odroid_settings_StartupFile_set(0);
        odroid_settings_commit();
        NVIC_SystemReset();
        break;
    default:
        assert(0);
    }
}

runtime_stats_t odroid_system_get_stats()
{
    float tickTime = (get_elapsed_time() - counters.resetTime);

    statistics.battery = odroid_input_read_battery();
    statistics.busyPercent = counters.busyTime / tickTime * 100.f;
    statistics.skippedFPS = counters.skippedFrames / (tickTime / 1000.f);
    statistics.totalFPS = counters.totalFrames / (tickTime / 1000.f);

    skip = 1;
    counters.busyTime = 0;
    counters.totalFrames = 0;
    counters.skippedFrames = 0;
    counters.resetTime = get_elapsed_time();

    return statistics;
}

void odroid_system_sleep(void)
{
    odroid_settings_StartupFile_set(ACTIVE_FILE);

    // odroid_settings_commit();
    gui_save_current_tab();

    GW_EnterDeepSleep();
}
