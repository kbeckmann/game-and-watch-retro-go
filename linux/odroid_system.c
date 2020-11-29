#include "odroid_system.h"

static panic_trace_t *panicTrace = (void *)0x0;

static rg_app_desc_t currentApp;
static runtime_stats_t statistics;
static runtime_counters_t counters;

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
    printf("%s: System ready!\n\n", __func__);
}

void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb)
{
    // currentApp.gameId = crc32_le(0, buffer, sizeof(buffer));
    currentApp.gameId = 0;
    currentApp.loadState = load;
    currentApp.saveState = save;

    printf("%s: Init done. GameId=%08X\n", __func__, currentApp.gameId);
}

rg_app_desc_t *odroid_system_get_app()
{
    return &currentApp;
}


bool odroid_system_emu_load_state(int slot)
{

}

IRAM_ATTR void odroid_system_tick(uint skippedFrame, uint fullFrame, uint busyTime)
{
    if (skippedFrame) counters.skippedFrames++;
    else if (fullFrame) counters.fullFrames++;
    counters.totalFrames++;
    counters.busyTime += busyTime;

    statistics.lastTickTime = get_elapsed_time();
}
