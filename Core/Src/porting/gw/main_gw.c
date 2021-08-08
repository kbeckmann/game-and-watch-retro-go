#include <odroid_system.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"

/* TO move elsewhere */
#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"

/* G&W system support */
#include "gw_system.h"

/* access to internals for debug purpose */
#include "sm510.h"

/* Uncoment to enable debug menu in overlay */
//#define GW_EMU_DEBUG_OVERLAY

#define ODROID_APPID_GW 6

/* Audio buffer length */
#define GW_AUDIO_BUFFER_LENGTH_DMA ((2 * GW_AUDIO_FREQ) / GW_REFRESH_RATE)

static odroid_gamepad_state_t joystick;

static unsigned char state_save_buffer[sizeof(gw_state_t)];

static bool gw_system_SaveState(char *pathName)
{
    printf("Saving state...\n");

    memset(state_save_buffer, '\x00', sizeof(state_save_buffer));
    gw_state_save(state_save_buffer);
    store_save(ACTIVE_FILE->save_address, state_save_buffer, sizeof(state_save_buffer));
    printf("Saving state done!\n");
    return false;
}

static bool gw_system_LoadState(char *pathName)
{
    printf("Loading state...\n");
    gw_state_load(ACTIVE_FILE->save_address);
    printf("Loading state done!\n");
    return true;
}

/* callback to get buttons state */
unsigned int gw_get_buttons() { return buttons_get(); }

static void gw_sound_init()
{

    /* init emulator sound system with shared audio buffer */
    gw_system_sound_init();

    /* clear DMA audio buffer */
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    /* Start SAI DMA */
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, GW_AUDIO_BUFFER_LENGTH_DMA);
}

static void gw_sound_submit()
{

    uint8_t volume = odroid_audio_volume_get();
    int16_t factor = volume_tbl[volume];

    /** Enables the following code to track audio rendering issues **/
    /*
    if (gw_audio_buffer_idx < GW_AUDIO_BUFFER_LENGTH) {
        printf("audio underflow:%u < %u \n",gw_audio_buffer_idx , GW_AUDIO_BUFFER_LENGTH);
        assert(0);
    }

    if (gw_audio_buffer_idx > (GW_AUDIO_BUFFER_LENGTH +12) ) {
        printf("audio oveflow:%u < %u \n",gw_audio_buffer_idx , GW_AUDIO_BUFFER_LENGTH);
        assert(0);
    }
    */

    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : GW_AUDIO_BUFFER_LENGTH;

    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN)
    {
        for (int i = 0; i < GW_AUDIO_BUFFER_LENGTH; i++)
        {
            audiobuffer_dma[i + offset] = 0;
        }
    }
    else
    {
        for (int i = 0; i < GW_AUDIO_BUFFER_LENGTH; i++)
        {
            audiobuffer_dma[i + offset] = (factor) * (gw_audio_buffer[i] << 4);
        }
    }

    gw_audio_buffer_copied = true;
}

/************************ Debug function in overlay START *******************************/

/* performance monitoring */
/* Emulator loop monitoring
    ( unit is 1/systemcoreclock 1/280MHz )
    loop_cycles 
        -measured duration of the loop.
    proc_cycles
        - estimated duration of emulated CPU for a bunch of emulated system clock.
    blit_cycles
        - estimated duration of graphics rendering.
    end_cycles
        - estimated duration of overall processing.
    */
static unsigned int loop_duration_us = 1, end_duration_us = 1, proc_duration_us = 1, blit_duration_us = 1;
static unsigned int loop_cycles = 1, end_cycles = 1, proc_cycles = 1, blit_cycles = 1;
static const unsigned int SYSTEM_CORE_CLOCK_MHZ = 280;

/* DWT counter used to measure time execution */
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int *LAR = (unsigned int *)0xE0001FB0; // <-- lock access register

#define get_dwt_cycles() *DWT_CYCCNT
#define clear_dwt_cycles() *DWT_CYCCNT = 0

static void enable_dwt_cycles()
{

    /* Use DWT cycle counter to get precision time elapsed during loop.
    The DWT cycle counter is cleared on every loop
    it may crash if the DWT is used during trace profiling */

    *DEMCR = *DEMCR | 0x01000000;    // enable trace
    *LAR = 0xC5ACCE55;               // <-- added unlock access to DWT (ITM, etc.)registers
    *DWT_CYCCNT = 0;                 // clear DWT cycle counter
    *DWT_CONTROL = *DWT_CONTROL | 1; // enable DWT cycle counter
}

static void gw_debug_bar()
{

#ifdef GW_EMU_DEBUG_OVERLAY

    static bool debug_init_done = false;

    if (!debug_init_done)
    {
        enable_dwt_cycles();
        debug_init_done = true;
    }

    static unsigned int overflow_count = 0;
    static unsigned int busy_percent = 0;

    char debugMsg[120];

    proc_duration_us = proc_cycles / SYSTEM_CORE_CLOCK_MHZ;
    blit_duration_us = blit_cycles / SYSTEM_CORE_CLOCK_MHZ;
    end_duration_us = end_cycles / SYSTEM_CORE_CLOCK_MHZ;
    loop_duration_us = loop_cycles / SYSTEM_CORE_CLOCK_MHZ;

    busy_percent = 100 * (proc_duration_us + blit_duration_us) / loop_duration_us;

    if (end_duration_us > 1000000 / GW_REFRESH_RATE)
        overflow_count++;

    if (m_halt != 0)
        sprintf(debugMsg, "%04dus EMU:%04dus FX:%04dus %d%%+%d HALT", loop_duration_us, proc_duration_us, blit_duration_us, busy_percent, overflow_count);
    else
        sprintf(debugMsg, "%04dus EMU:%04dus FX:%04dus %d%%+%d", loop_duration_us, proc_duration_us, blit_duration_us, busy_percent, overflow_count);

    odroid_overlay_draw_text(0, 0, GW_SCREEN_WIDTH, debugMsg, C_GW_YELLOW, C_GW_RED);

#endif
}
/************************ Debug function in overlay END ********************************/

/* Main */
int app_main_gw(uint8_t load_state)
{

    odroid_system_init(ODROID_APPID_GW, GW_AUDIO_FREQ);
    odroid_system_emu_init(&gw_system_LoadState, &gw_system_SaveState, NULL);
    rg_app_desc_t *app = odroid_system_get_app();

    // const int frameTime = get_frame_time(GW_REFRESH_RATE);

    /*** load ROM  */
    bool rom_status = gw_system_romload();

    if (!rom_status)
        odroid_system_switch_app(0);

    /*** Clear audio buffer */
    gw_sound_init();
    printf("Sound initialized\n");

    /*** Configure the emulated system */
    gw_system_config();
    printf("G&W configured\n");

    /*** Start and Reset the emulated system */
    gw_system_start();
    printf("G&W start\n");

    gw_system_reset();
    printf("G&W reset\n");

    /*** Main emulator loop */
    unsigned int power_pressed = 0;
    printf("Main emulator loop start\n");

    /* check if we to have to load state */
    if (load_state != 0)
        gw_system_LoadState(NULL);

    clear_dwt_cycles();

    while (true)
    {
        /* clear DWT counter used to monitor performances */
        clear_dwt_cycles();

        wdog_refresh();

        odroid_input_read_gamepad(&joystick);

        if (joystick.values[ODROID_INPUT_VOLUME])
        {

            // TODO: Sync framebuffers in a nicer way
            lcd_sync();

            odroid_dialog_choice_t options[] = {
                ODROID_DIALOG_CHOICE_LAST};
            odroid_overlay_game_menu(options);
        }

        if (power_pressed != joystick.values[ODROID_INPUT_POWER])
        {
            printf("Power toggle %d=>%d\n", power_pressed, !power_pressed);
            power_pressed = joystick.values[ODROID_INPUT_POWER];
            if (power_pressed)
            {
                printf("Power PRESSED %d\n", power_pressed);
                HAL_SAI_DMAStop(&hsai_BlockA1);
                GW_EnterDeepSleep();
            }
        }

        /* Emulate and Blit */
        // Call the emulator function with number of clock cycles
        // to execute on the emulated device
        // multiply the number of cycles to emulate by speedup factor
        gw_system_run(GW_SYSTEM_CYCLES * (1 + app->speedupEnabled));

        /* get how many cycles have been spent in the emulator */
        proc_cycles = get_dwt_cycles();

        /* update the screen only if there is no pending frame to render */
        if (!is_lcd_swap_pending())
        {
            gw_system_blit(lcd_get_active_buffer());
            gw_debug_bar();
            lcd_swap();

            /* get how many cycles have been spent in graphics rendering */
            blit_cycles = get_dwt_cycles() - proc_cycles;
        }
        /****************************************************************************/

        /* copy audio samples for DMA */
        gw_sound_submit();

        /* get how many cycles have been spent to process everything */
        end_cycles = get_dwt_cycles();

        static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
        while (dma_state == last_dma_state)
        {
            __NOP();
        }
        last_dma_state = dma_state;

        /* get how cycles have been spent inside this loop */
        loop_cycles = get_dwt_cycles();

    } // end of loop
}
