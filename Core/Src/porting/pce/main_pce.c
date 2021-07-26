#include <odroid_system.h>
#include <string.h>
#include <hard_pce.h>
#include <romdb_pce.h>
#include <assert.h>
#include <gfx.h>
#include "main.h"
#include "bilinear.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "shared.h"
#include "rom_manager.h"
#include "common.h"
#include "sound_pce.h"

#define APP_ID 4

//#define PCE_SHOW_DEBUG
//#define XBUF_WIDTH 	(480 + 32)
//#define XBUF_HEIGHT	(242 + 32)
//#define GW_LCD_WIDTH  (320)
//#define GW_LCD_HEIGHT (240)
//#define AUDIO_SAMPLE_RATE   (22050)
#define FB_INTERNAL_OFFSET (((XBUF_HEIGHT - current_height) / 2 + 16) * XBUF_WIDTH + (XBUF_WIDTH - current_width) / 2)
#define AUDIO_BUFFER_LENGTH_PCE  (AUDIO_SAMPLE_RATE / 60)
#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80
#define SVAR_1(k, v) { 1, k, &v }
#define SVAR_2(k, v) { 2, k, &v }
#define SVAR_4(k, v) { 4, k, &v }
#define SVAR_A(k, v) { sizeof(v), k, &v }
#define SVAR_N(k, v, n) { n, k, &v }
#define SVAR_END { 0, "\0\0\0\0", 0 }
#define COLOR_RGB(r, g, b) ((((r) << 13) & 0xf800) + (((g) << 8) & 0x07e0) + (((b) << 3) & 0x001f))

typedef struct
{
    uint len;
    char key[16];
    void *ptr;
} svar_t;

static uint16_t mypalette[256];
static int current_height, current_width;
static short audioBuffer_pce[ AUDIO_BUFFER_LENGTH_PCE * 2];
static uint8_t emulator_framebuffer_pce[XBUF_WIDTH * XBUF_HEIGHT];
static uint8_t OBJ_CACHE_buf[0x10000];
static uint8_t PCE_EXRAM_BUF[0x8000];
static uint skipFrames = 0;
static int framePerSecond=0;

// TODO: Move to lcd.c/h
extern LTDC_HandleTypeDef hltdc;
static char pce_log[100];
const char SAVESTATE_HEADER[8] = "PCE_V004";


const svar_t SaveStateVars[] =
{
    // Arrays
    SVAR_A("RAM", PCE.RAM),      SVAR_A("VRAM", PCE.VRAM),  SVAR_A("SPRAM", PCE.SPRAM),
    SVAR_A("PAL", PCE.Palette),  SVAR_A("MMR", PCE.MMR),

    // CPU registers
    SVAR_2("CPU.PC", reg_pc),    SVAR_1("CPU.A", reg_a),    SVAR_1("CPU.X", reg_x),
    SVAR_1("CPU.Y", reg_y),      SVAR_1("CPU.P", reg_p),    SVAR_1("CPU.S", reg_s),

    // Misc
    SVAR_4("Cycles", Cycles),                   SVAR_4("MaxCycles", PCE.MaxCycles),
    SVAR_1("SF2", PCE.SF2),

    // IRQ
    SVAR_1("irq_mask", PCE.irq_mask),           SVAR_1("irq_status", PCE.irq_status),

    // PSG
    SVAR_1("psg.ch", PCE.PSG.ch),               SVAR_1("psg.vol", PCE.PSG.volume),
    SVAR_1("psg.lfo_f", PCE.PSG.lfo_freq),      SVAR_1("psg.lfo_c", PCE.PSG.lfo_ctrl),
    SVAR_N("psg.ch0", PCE.PSG.chan[0], 40),     SVAR_N("psg.ch1", PCE.PSG.chan[1], 40),
    SVAR_N("psg.ch2", PCE.PSG.chan[2], 40),     SVAR_N("psg.ch3", PCE.PSG.chan[3], 40),
    SVAR_N("psg.ch4", PCE.PSG.chan[4], 40),     SVAR_N("psg.ch5", PCE.PSG.chan[5], 40),

    // VCE
    SVAR_A("vce_regs", PCE.VCE.regs),           SVAR_2("vce_reg", PCE.VCE.reg),

    // VDC
    SVAR_A("vdc_regs", PCE.VDC.regs),           SVAR_1("vdc_reg", PCE.VDC.reg),
    SVAR_1("vdc_status", PCE.VDC.status),       SVAR_1("vdc_satb", PCE.VDC.satb),
    SVAR_4("vdc_pending_irqs", PCE.VDC.pending_irqs),

    // Timer
    SVAR_4("timer_reload", PCE.Timer.reload),   SVAR_4("timer_running", PCE.Timer.running),
    SVAR_4("timer_counter", PCE.Timer.counter), SVAR_4("timer_next", PCE.Timer.cycles_counter),

    SVAR_END
};

uint8_t *osd_gfx_framebuffer(void){
    return emulator_framebuffer_pce + FB_INTERNAL_OFFSET;
}

void* osd_alloc(size_t size) {
    assert(size==0x10000);
    return OBJ_CACHE_buf;
}

void osd_gfx_set_mode(int width, int height) {
    current_width = width;
    current_height = height;
}

void osd_gfx_init(void) {

}

void osd_log(const char *format, ...) {

}

static void netplay_callback(netplay_event_t event, void *arg) {
    // Where we're going we don't need netplay!
}

static bool SaveState(char *pathName) {
    int pos=0;
    uint8_t *pce_save_buf = emulator_framebuffer_pce;
    memset(pce_save_buf, 0x00, 76*1024); // 76K save size

    uint8_t *pce_save_header=(uint8_t *)SAVESTATE_HEADER;
    for(int i=0;i<sizeof(SAVESTATE_HEADER);i++) {
        pce_save_buf[pos]=pce_save_header[i];
        pos++;
    }
    pce_save_buf[pos]=0; pos++;
    uint32_t *crc_ptr = (uint32_t *)(pce_save_buf + pos);
    crc_ptr[0] = PCE.ROM_CRC; pos+=sizeof(uint32_t);

    for (int i = 0; SaveStateVars[i].len > 0; i++) {
        uint8_t *pce_save_ptr = (uint8_t *)SaveStateVars[i].ptr;
        for(int j=0;j<SaveStateVars[i].len;j++) {
            pce_save_buf[pos]=pce_save_ptr[j];
            pos++;
        }
    }
    assert(pos<76*1024);
    store_save(ACTIVE_FILE->save_address, pce_save_buf, 76*1024);
    sprintf(pce_log,"%08lX",PCE.ROM_CRC);
    memset(emulator_framebuffer_pce,0,sizeof(emulator_framebuffer_pce));
    return false;
}

static bool LoadState(char *pathName) {
    uint8_t *pce_save_buf = (uint8_t *)ACTIVE_FILE->save_address;
    if (ACTIVE_FILE->save_size==0) return true;
    sprintf(pce_log,"%ld",ACTIVE_FILE->save_size);

    pce_save_buf+=sizeof(SAVESTATE_HEADER) + 1;

    uint32_t *crc_ptr = (uint32_t *)pce_save_buf;
    sprintf(pce_log,"%08lX",crc_ptr[0]);
    if (crc_ptr[0]!=PCE.ROM_CRC) {
        return true;
    }

    pce_save_buf+=sizeof(uint32_t);


    int pos=0;
    for (int i = 0; SaveStateVars[i].len > 0; i++) {
        printf("Loading %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
        uint8_t *pce_save_ptr = (uint8_t *)SaveStateVars[i].ptr;
        for(int j=0;j<SaveStateVars[i].len;j++) {
            pce_save_ptr[j] = pce_save_buf[pos];
            pos++;
        }
    }
    for(int i = 0; i < 8; i++) {
        pce_bank_set(i, PCE.MMR[i]);
    }
    gfx_clear_cache();
    osd_gfx_set_mode(IO_VDC_SCREEN_WIDTH, IO_VDC_SCREEN_HEIGHT);
    return true;
}

void LoadCartPCE() {
    int offset;
    PCE.ROM = (uint8 *)ROM_DATA;
    offset = ROM_DATA_LENGTH & 0x1fff;
    PCE.ROM_SIZE = (ROM_DATA_LENGTH - offset) / 0x2000;
     PCE.ROM_DATA = PCE.ROM + offset;
       PCE.ROM_CRC = crc32_le(0, PCE.ROM, ROM_DATA_LENGTH);
       uint IDX = 0;
       uint ROM_MASK = 1;

       while (ROM_MASK < PCE.ROM_SIZE) ROM_MASK <<= 1;
       ROM_MASK--;

#ifdef PCE_SHOW_DEBUG
       printf("Rom Size: %d, B1:%X, B2:%X, B3:%X, B4:%X" , ROM_DATA_LENGTH, PCE.ROM[0], PCE.ROM[1],PCE.ROM[2],PCE.ROM[3]);
#endif

       for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
           if (PCE.ROM_CRC == pceRomFlags[index].crc) {
               IDX = index;
               break;
           }
       }

       printf("Game Name: %s\n", pceRomFlags[IDX].Name);
       printf("Game Region: %s\n", (pceRomFlags[IDX].Flags & JAP) ? "Japan" : "USA");

       // US Encrypted
    if ((pceRomFlags[IDX].Flags & US_ENCODED) || PCE.ROM_DATA[0x1FFF] < 0xE0) {
        printf("This rom is probably US encrypted, Not supported!!!\n");
        assert(0);
       }

    if (pceRomFlags[IDX].Flags & TWO_PART_ROM) PCE.ROM_SIZE = 0x30;

    // Game ROM
    for (int i = 0; i < 0x80; i++) {
        if (PCE.ROM_SIZE == 0x30) {
            switch (i & 0x70) {
            case 0x00:
            case 0x10:
            case 0x50:
                MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
                break;
            case 0x20:
            case 0x60:
                MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
                break;
            case 0x30:
            case 0x70:
                MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x10) & ROM_MASK) * 0x2000;
                break;
            case 0x40:
                MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
                break;
            }
        } else {
            MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
        }
        MemoryMapW[i] = PCE.NULLRAM;
    }

    // Allocate the card's onboard ram
    if (pceRomFlags[IDX].Flags & ONBOARD_RAM) {
        PCE.ExRAM = PCE.ExRAM ?: PCE_EXRAM_BUF;
        MemoryMapR[0x40] = MemoryMapW[0x40] = PCE.ExRAM;
        MemoryMapR[0x41] = MemoryMapW[0x41] = PCE.ExRAM + 0x2000;
        MemoryMapR[0x42] = MemoryMapW[0x42] = PCE.ExRAM + 0x4000;
        MemoryMapR[0x43] = MemoryMapW[0x43] = PCE.ExRAM + 0x6000;
    }

    // Mapper for roms >= 1.5MB (SF2, homebrews)
    if (PCE.ROM_SIZE >= 192)
        MemoryMapW[0x00] = PCE.IOAREA;
}

void ResetPCE() {
    gfx_clear_cache();
    pce_reset();

}

static inline void pce_timer_run(void) {
    PCE.Timer.cycles_counter -= CYCLES_PER_LINE;

    // Trigger when it underflows
    if (PCE.Timer.cycles_counter > CYCLES_PER_TIMER_TICK) {
        PCE.Timer.cycles_counter += CYCLES_PER_TIMER_TICK;
        if (PCE.Timer.running) {
            // Trigger when it underflows from 0
            if (PCE.Timer.counter > 0x7F) {
                PCE.Timer.counter = PCE.Timer.reload;
                PCE.irq_status |= INT_TIMER;
            }
            PCE.Timer.counter--;
        }
    }
}

void pce_input_read(odroid_gamepad_state_t* out_state) {
    unsigned char rc = 0;
    if (out_state->values[ODROID_INPUT_LEFT])   rc |= JOY_LEFT;
    if (out_state->values[ODROID_INPUT_RIGHT])  rc |= JOY_RIGHT;
    if (out_state->values[ODROID_INPUT_UP])     rc |= JOY_UP;
    if (out_state->values[ODROID_INPUT_DOWN])   rc |= JOY_DOWN;
    if (out_state->values[ODROID_INPUT_A])      rc |= JOY_A;
    if (out_state->values[ODROID_INPUT_B])      rc |= JOY_B;
    if (out_state->values[ODROID_INPUT_START])  rc |= JOY_RUN;
    if (out_state->values[ODROID_INPUT_SELECT]) rc |= JOY_SELECT;
    PCE.Joypad.regs[0] = rc;
}

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t col = 0xffff;
    if (index != 255)  {
        col = COLOR_RGB(r,g,b);
    }
    mypalette[index] = col;
}

void init_color_pals() {
    for (int i = 0; i < 255; i++) {
        // GGGRR RBB
          set_color(i, (i & 0x1C)>>2, (i & 0xE0) >> 5, (i & 0x03) );
    }
    set_color(255, 0x3f, 0x3f, 0x3f);
}

void pce_osd_gfx_blit(bool drawFrame) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    if (!drawFrame) {
        memset(emulator_framebuffer_pce,0,sizeof(emulator_framebuffer_pce));
        return;
    }

    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;
    if (delta >= 1000) {
        framePerSecond = (10000 * frames) / delta;
        //printf("FPS: %d.%d, frames %ld, delta %ld ms\n", framePerSecond / 10, framePerSecond % 10, frames, delta);
        frames = 0;
        lastFPSTime = currentTime;
    }

#ifdef PCE_SHOW_DEBUG1
    // Calculate no. of active Tiles and Sprites
    int sprCount=0, tileCount=0;
    for(int j=0;j<2048;j++) {
        if (TILE_CACHE[j]) tileCount++;
     }
    for(int j=0;j<512;j++) {
        if (SPR_CACHE[j]) sprCount++;
    }
#endif

    uint8_t *emuFrameBuffer = osd_gfx_framebuffer();
    uint16_t *framebuffer_active = (active_framebuffer == 0 ? framebuffer1 : framebuffer2);
    int x2=0,y=0, offsetY;
    int xScaleDownModulo = 0;
    int xScaleUpModulo = 0;
    uint8_t *fbTmp;
    if (GW_LCD_WIDTH<current_width) xScaleDownModulo = current_width/(current_width-GW_LCD_WIDTH);
    if (GW_LCD_WIDTH>current_width) xScaleUpModulo = current_width/(GW_LCD_WIDTH-current_width);
    int renderHeight = (current_height<=GW_LCD_HEIGHT)?current_height:GW_LCD_HEIGHT;

    for(y=0;y<renderHeight;y++) {
        x2=0;
        fbTmp = emuFrameBuffer+(y*XBUF_WIDTH);
        offsetY = y*GW_LCD_WIDTH;
        if (xScaleUpModulo) {
            // Horizontal - Scale up
            for(int x=0;x<current_width;x++) {
                framebuffer_active[offsetY+x2]=mypalette[fbTmp[x]];
                x2++;
                if ((x+1)%xScaleUpModulo==0) {
                    framebuffer_active[offsetY+x2]=mypalette[fbTmp[x]];
                    x2++;
                }
            }
        } else if (xScaleDownModulo) {
            // Horizontal - Scale down
            for(int x=0;x<current_width;x++) {
                if (x%xScaleDownModulo!=0) {
                    framebuffer_active[offsetY+x2]=mypalette[fbTmp[x]];
                    x2++;
                }
            }
        } else {
            // No scaling, 1:1
            for(int x=0;x<current_width;x++) {
                   framebuffer_active[offsetY+x]=mypalette[fbTmp[x]];
            }
        }
    }
    // Temporary, Y scaling is not yet implemented
    for(;y<GW_LCD_HEIGHT;y++) {
        fbTmp = emuFrameBuffer+(y*XBUF_WIDTH);
        offsetY = y*GW_LCD_WIDTH;
        for(int x=0;x<GW_LCD_WIDTH;x++) {
            framebuffer_active[offsetY+x]=0;
        }
    }

#ifdef PCE_SHOW_DEBUG
    char debugMsg[100];
    sprintf(debugMsg,"FPS:%d.%d,W:%d,H:%d,L:%s", framePerSecond / 10,framePerSecond % 10,current_width,current_height,pce_log);
    odroid_overlay_draw_text(0,0, GW_LCD_WIDTH, debugMsg,  C_GW_YELLOW, C_GW_RED);
#endif

    lcd_swap();

    memset(emulator_framebuffer_pce,0,sizeof(emulator_framebuffer_pce));
}

void pce_pcm_submit() {
    uint8_t volume = odroid_audio_volume_get();
    //int32_t factor = volume_tbl[volume]  ;
    int32_t factor = volume_tbl[volume] / 2 ;
    pce_snd_update(audioBuffer_pce, AUDIO_BUFFER_LENGTH_PCE );
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH_PCE;
    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_PCE; i++) {
            audiobuffer_dma[offset + i] = 0;
        }
    } else {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_PCE; i++) {
            int32_t sample = (audioBuffer_pce[i*2] + audioBuffer_pce[i*2+1]);
            //int32_t sample = (audioBuffer_pce[i*2] );
            audiobuffer_dma[offset + i] = (sample * factor) >> 8;
        }
    }
    static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
    while (dma_state == last_dma_state) {
        __NOP();
    }
    last_dma_state = dma_state;
}

int app_main_pce(uint8_t load_state, uint8_t start_paused) {

    uint8_t pause_pressed = 0;
    uint8_t power_pressed = 0;
    uint8_t pause_after_frames;
    uint8_t pauseFrames = 0;
    uint8_t frames_since_last_skip = 0;

    if (start_paused) {
        pause_after_frames = 2;
        odroid_audio_mute(true);
    } else {
        pause_after_frames = 0;
    }

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);
    rg_app_desc_t *app = odroid_system_get_app();
    pce_log[0]=0;

    // Init Graphics
    init_color_pals();
    const int refresh_rate = FPS_NTSC;
    sprintf(pce_log,"%d",refresh_rate);
    const int frameTime = 1000 / refresh_rate;
    bool fullFrame = false;
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));
    gfx_init();
    printf("Graphics initialized\n");

    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH_PCE * 2 );
    pce_snd_init();
    printf("Sound initialized\n");

    // Init PCE Core
    pce_init();
    LoadCartPCE();
    ResetPCE();
    printf("PCE Core initialized\n");

    // If user select "RESUME" in main menu
    if (load_state) LoadState(NULL);

    // Main emulator loop
    printf("Main emulator loop start\n");

    while (true) {
        wdog_refresh();
        bool drawFrame = !skipFrames;
        if(drawFrame) frames_since_last_skip += 1;
        else frames_since_last_skip = 0;

        odroid_gamepad_state_t joystick;

        odroid_input_read_gamepad(&joystick);

        if (pause_pressed != joystick.values[ODROID_INPUT_VOLUME]) {
            if (pause_pressed) {
                // TODO: Sync framebuffers in a nicer way
                lcd_sync();

                odroid_dialog_choice_t options[] = {
                    ODROID_DIALOG_CHOICE_LAST
                };
                odroid_overlay_game_menu(options);
                memset(framebuffer1, 0x0, sizeof(framebuffer1));
                memset(framebuffer2, 0x0, sizeof(framebuffer2));
            }
            pause_pressed = joystick.values[ODROID_INPUT_VOLUME];
        }

        uint startTime = get_elapsed_time();

        if (power_pressed != joystick.values[ODROID_INPUT_POWER]) {
            printf("Power toggle %ld=>%d\n", power_pressed, !power_pressed);
            power_pressed = joystick.values[ODROID_INPUT_POWER];
            if (power_pressed) {
                printf("Power PRESSED %ld\n", power_pressed);
                HAL_SAI_DMAStop(&hsai_BlockA1);
                if(!joystick.values[ODROID_INPUT_VOLUME]) {
                    // Always save as long as PAUSE is not pressed
                    SaveState(NULL);
                }

                odroid_system_sleep();
            }
        }

        pce_input_read(&joystick);

        for (Scanline = 0; Scanline < 263; ++Scanline) {
            PCE.MaxCycles += CYCLES_PER_LINE;
            h6280_run();
            pce_timer_run();
            gfx_run();
        }

        pce_osd_gfx_blit(drawFrame);

        // See if we need to skip a frame to keep up
        if (skipFrames == 0) {
            if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
            switch(app->speedupEnabled){
                case SPEEDUP_0_5x:
                    pauseFrames++;
                    break;
                case SPEEDUP_0_75x:
                    if(frames_since_last_skip % 4 == 0) pauseFrames++;
                    break;
                case SPEEDUP_1_25x:
                    if(frames_since_last_skip % 4 == 0) skipFrames++;
                    break;
                case SPEEDUP_1_5x:
                    if(frames_since_last_skip % 2 == 0) skipFrames++;
                    break;
                case SPEEDUP_2x:
                    skipFrames++;
                    break;
                case SPEEDUP_3x:
                    skipFrames+=2;
                    break;
            }
        } else if (skipFrames > 0) {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (drawFrame) {
            for(uint8_t p = 0; p < pauseFrames + 1; p++) {
                pce_pcm_submit();
            }
            pauseFrames = 0;
        }

        // Prevent overflow
        int trim = MIN(Cycles, PCE.MaxCycles);
        PCE.MaxCycles -= trim;
        Cycles -= trim;

        // Render frames before faking a pause button press
        if (pause_after_frames > 0) {
            pause_after_frames--;
            if (pause_after_frames == 0) {
                pause_pressed = 1;
            }
        }
    }

}
