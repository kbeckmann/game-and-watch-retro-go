#include <odroid_system.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "porting.h"
#include "crc32.h"

#include <gfx.h>
#include "gw_lcd.h"
#include <pce.h>
#include <romdb.h>

#undef printf
#define APP_ID 20

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

#define NVS_KEY_SAVE_SRAM "sram"

#define WIDTH    352
#define HEIGHT   242
#define BPP      2
#define SCALE    4

typedef uint16_t pixel_t;
static uint16_t mypalette[256];
#define COLOR_RGB(r, g, b) ((((r) << 13) & 0xf800) + (((g) << 8) & 0x07e0) + (((b) << 3) & 0x001f))


#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

// Use 60Hz for GB
#define AUDIO_BUFFER_LENGTH_GB (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_GB ((2 * AUDIO_SAMPLE_RATE) / 60)

#define FB_INTERNAL_OFFSET (((XBUF_HEIGHT - current_height) / 2 + 16) * XBUF_WIDTH + (XBUF_WIDTH - current_width) / 2)
static uint8_t emulator_framebuffer_pce[XBUF_WIDTH * XBUF_HEIGHT * 4];

extern unsigned char ROM_DATA[];
extern unsigned int cart_rom_len;


static odroid_video_frame_t update1 = {WIDTH, HEIGHT, WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t update2 = {WIDTH, HEIGHT, WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};

static bool saveSRAM = false;

// 3 pages
uint8_t state_save_buffer[192 * 1024];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *fb_texture;
uint16_t fb_data[WIDTH * HEIGHT * BPP];

SDL_AudioSpec wanted;
void fill_audio(void *udata, Uint8 *stream, int len);

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;

static uint8_t PCE_EXRAM_BUF[0x8000];
static int framePerSecond=0;

static int current_height, current_width;
#define PCE_SAMPLE_RATE   (22050)
#define AUDIO_BUFFER_LENGTH_PCE  (PCE_SAMPLE_RATE / 60)
//static short audioBuffer_pce[ AUDIO_BUFFER_LENGTH_PCE * 2];

/**
 * Describes what is saved in a save state. Changing the order will break
 * previous saves so add a place holder if necessary. Eventually we could use
 * the keys to make order irrelevant...
 */
#define SVAR_1(k, v) { 1, k, &v }
#define SVAR_2(k, v) { 2, k, &v }
#define SVAR_4(k, v) { 4, k, &v }
#define SVAR_A(k, v) { sizeof(v), k, &v }
#define SVAR_N(k, v, n) { n, k, &v }
#define SVAR_END { 0, "\0\0\0\0", 0 }

const char SAVESTATE_HEADER[8] = "PCE_V004";

static const struct
{
	size_t len;
	char key[16];
	void *ptr;
} SaveStateVars[] =
{
    // Arrays
    SVAR_A("RAM", PCE.RAM),      SVAR_A("VRAM", PCE.VRAM),  SVAR_A("SPRAM", PCE.SPRAM),
    SVAR_A("PAL", PCE.Palette),  SVAR_A("MMR", PCE.MMR),

    // CPU registers
    SVAR_2("CPU.PC", CPU.PC),    SVAR_1("CPU.A", CPU.A),    SVAR_1("CPU.X", CPU.X),
    SVAR_1("CPU.Y", CPU.Y),      SVAR_1("CPU.P", CPU.P),    SVAR_1("CPU.S", CPU.S),

    // Misc
    SVAR_4("Cycles", Cycles),                   SVAR_4("MaxCycles", PCE.MaxCycles),
    SVAR_1("SF2", PCE.SF2),

    // IRQ
    SVAR_1("irq_mask", CPU.irq_mask),           SVAR_1("irq_lines", CPU.irq_lines),

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

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t col = 0xffff;
    if (index != 255)  {
        col = COLOR_RGB(r,g,b);
    }
    mypalette[index] = col;
}

void init_color_pals() {
    printf("init_color_pals()\n");

    for (int i = 0; i < 255; i++) {
        // GGGRR RBB
          set_color(i, (i & 0x1C)>>2, (i & 0xE0) >> 5, (i & 0x03) );
    }
    set_color(255, 0x3f, 0x3f, 0x3f);
}

void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}


void fill_audio(void *udata, Uint8 *stream, int len)
{

}

uint8_t *osd_gfx_framebuffer(void){
    return emulator_framebuffer_pce + FB_INTERNAL_OFFSET;
}

void osd_gfx_set_mode(int width, int height) {
	init_color_pals();
    printf("current_width: %d \ncurrent_height: %d\n", width, height);
    current_width = width;
    current_height = height;
    SDL_SetWindowSize( window, current_width * SCALE, current_height * SCALE);
    SDL_DestroyTexture(fb_texture);
    fb_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
        current_width, current_height);

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

int init_window(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 0;

    window = SDL_CreateWindow("emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width * SCALE, height * SCALE,
        0);
    if (!window)
        return 0;

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
        return 0;

    fb_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
        width, height);
    if (!fb_texture)
        return 0;

    return 0;
}

static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

//int SaveState(const char *pathName)
static bool SaveStateStm(char *name)
{
    printf("Loading state from %s...\n", name);

	char buffer[512];

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	fread(&buffer, 8, 1, fp);

	if (memcmp(&buffer, SAVESTATE_HEADER, 8) != 0)
	{
		MESSAGE_ERROR("Loading state failed: Header mismatch\n");
		fclose(fp);
		return -1;
	}

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		printf("Loading %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fread(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	for(int i = 0; i < 8; i++)
	{
		pce_bank_set(i, PCE.MMR[i]);
	}

	gfx_reset(true);

	osd_gfx_set_mode(IO_VDC_SCREEN_WIDTH, IO_VDC_SCREEN_HEIGHT);

	fclose(fp);

	return 0;
}

//int LoadState(const char *pathName)
static bool LoadStateStm(char *name)
{
    printf("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;

	fwrite(SAVESTATE_HEADER, sizeof(SAVESTATE_HEADER), 1, fp);

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		printf("Saving %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fwrite(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	fclose(fp);

	return 0;  
}

void pcm_submit(void)
{

}

size_t
pce_osd_getromdata(unsigned char **data)
{
    /* src pointer to the ROM data in the external flash (raw or LZ4) */
    *data = (unsigned char *)ROM_DATA;
    return cart_rom_len;
}

const struct {
	const uint32_t crc;
	const char *Name;
	const uint32_t Flags;
} pceRomFlags[] = {
	{0x00000000, "Unknown", JAP},
	{0xF0ED3094, "Blazing Lazers", USA | TWO_PART_ROM},
	{0xB4A1B0F6, "Blazing Lazers", USA | TWO_PART_ROM},
	{0x55E9630D, "Legend of Hero Tonma", USA | US_ENCODED},
	{0x083C956A, "Populous", JAP | ONBOARD_RAM},
	{0x0A9ADE99, "Populous", JAP | ONBOARD_RAM},
};

int LoadCard(const char *name) {
    int offset;
    size_t rom_length = pce_osd_getromdata(&PCE.ROM);
    offset = rom_length & 0x1fff;
       
       
       PCE.ROM_SIZE = (rom_length - offset) / 0x2000;
       PCE.ROM_DATA = PCE.ROM + offset;
       PCE.ROM_CRC = crc32_le(0, PCE.ROM, rom_length);
       
       uint8_t IDX = 0;
       uint8_t ROM_MASK = 1;

       while (ROM_MASK < PCE.ROM_SIZE) ROM_MASK <<= 1;
       ROM_MASK--;

       printf("Rom Size: %d, B1:%X, B2:%X, B3:%X, B4:%X\n" , rom_length, PCE.ROM[0], PCE.ROM[1],PCE.ROM[2],PCE.ROM[3]);

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

		unsigned char inverted_nibble[16] = {
			0, 8, 4, 12, 2, 10, 6, 14,
			1, 9, 5, 13, 3, 11, 7, 15
		};

		for (int x = 0; x < PCE.ROM_SIZE * 0x2000; x++) {
			unsigned char temp = PCE.ROM_DATA[x] & 15;

			PCE.ROM_DATA[x] &= ~0x0F;
			PCE.ROM_DATA[x] |= inverted_nibble[PCE.ROM_DATA[x] >> 4];

			PCE.ROM_DATA[x] &= ~0xF0;
			PCE.ROM_DATA[x] |= inverted_nibble[temp] << 4;
		}
    }

	// For example with Devil Crush 512Ko
    if (pceRomFlags[IDX].Flags & TWO_PART_ROM) 
        PCE.ROM_SIZE = 0x30;

    // Game ROM
    for (int i = 0; i < 0x80; i++) {
        if (PCE.ROM_SIZE == 0x30) {
            switch (i & 0x70) {
            case 0x00:
            case 0x10:
            case 0x50:
                PCE.MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
                break;
            case 0x20:
            case 0x60:
                PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
                break;
            case 0x30:
            case 0x70:
                PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x10) & ROM_MASK) * 0x2000;
                break;
            case 0x40:
                PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
                break;
            }
        } else {
            PCE.MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
        }
        PCE.MemoryMapW[i] = PCE.NULLRAM;
    }

    // Allocate the card's onboard ram
    if (pceRomFlags[IDX].Flags & ONBOARD_RAM) {
        PCE.ExRAM = PCE.ExRAM ?: PCE_EXRAM_BUF;
        PCE.MemoryMapR[0x40] = PCE.MemoryMapW[0x40] = PCE.ExRAM;
        PCE.MemoryMapR[0x41] = PCE.MemoryMapW[0x41] = PCE.ExRAM + 0x2000;
        PCE.MemoryMapR[0x42] = PCE.MemoryMapW[0x42] = PCE.ExRAM + 0x4000;
        PCE.MemoryMapR[0x43] = PCE.MemoryMapW[0x43] = PCE.ExRAM + 0x6000;
    }

    // Mapper for roms >= 1.5MB (SF2, homebrews)
    if (PCE.ROM_SIZE >= 192)
        PCE.MemoryMapW[0x00] = PCE.IOAREA;

    return 0;
}

int
InitPCE(int samplerate, bool stereo, const char *huecard)
{
	if (gfx_init())
		return 1;

//	if (psg_init(samplerate, stereo))
//		return 1;

	if (pce_init())
		return 1;

	if (huecard && LoadCard(huecard))
		return 1;

	gfx_reset(0);
	pce_reset(0);

	return 0;
}

void init(void)
{
    printf("init()\n");
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadStateStm, &SaveStateStm, &netplay_callback);

    // Hack: Use the same buffer twice
    update1.buffer = fb_data;
    update2.buffer = fb_data;

    //saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);
    saveSRAM = false;

    // Load ROM
    InitPCE(0,0,"game.pce");

    // Video
    memset(fb_data, 0, sizeof(fb_data));
}

void pce_osd_gfx_blit(bool drawFrame) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;

    //no need to slow down intentionally
    //int wantedTime = 1000 / 60;
    //SDL_Delay(wantedTime); // rendering takes basically "0ms"

    if (!drawFrame) {
        memset(fb_data,0,sizeof(fb_data));
        return;
    }

    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;
    if (delta >= 1000) {
        framePerSecond = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %d, delta %d ms\n", framePerSecond / 10, framePerSecond % 10, frames, delta);
        frames = 0;
        lastFPSTime = currentTime;
    }

    uint8_t *emuFrameBuffer = osd_gfx_framebuffer();
    pixel_t *framebuffer_active = fb_data;//lcd_get_active_buffer();
    int y=0, offsetY;
    uint8_t *fbTmp;

    for(y=0;y<current_height;y++) {
        fbTmp = emuFrameBuffer+(y*XBUF_WIDTH);
        offsetY = y*current_width;
        // No scaling, 1:1
        for(int x=0;x<current_width;x++) {
               framebuffer_active[offsetY+x]=mypalette[fbTmp[x]];
        }
    }
    // Temporary, Y scaling is not yet implemented
    /*for(;y<HEIGHT;y++) {
        fbTmp = emuFrameBuffer+(y*XBUF_WIDTH);
        offsetY = y*WIDTH;
        for(int x=0;x<WIDTH;x++) {
            framebuffer_active[offsetY+x]=0;
        }
    }*/

    SDL_UpdateTexture(fb_texture, NULL, fb_data, current_width * BPP);
    SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    memset(fb_data,0,sizeof(fb_data));
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
                CPU.irq_lines |= INT_TIMER;
            }
            PCE.Timer.counter--;
        }
    }
}

void odroid_input_read_gamepad_pce(odroid_gamepad_state_t* out_state)
{
    SDL_Event event;
    static SDL_Event last_down_event;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // printf("Press %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                out_state->values[ODROID_INPUT_A] = 1;
                break;
            case SDLK_z:
                out_state->values[ODROID_INPUT_B] = 1;
                break;
            case SDLK_LSHIFT:
                out_state->values[ODROID_INPUT_START] = 1;
                break;
            case SDLK_LCTRL:
                out_state->values[ODROID_INPUT_SELECT] = 1;
                break;
            case SDLK_UP:
                out_state->values[ODROID_INPUT_UP] = 1;
                break;
            case SDLK_DOWN:
                out_state->values[ODROID_INPUT_DOWN] = 1;
                break;
            case SDLK_LEFT:
                out_state->values[ODROID_INPUT_LEFT] = 1;
                break;
            case SDLK_RIGHT:
                out_state->values[ODROID_INPUT_RIGHT] = 1;
                break;
            case SDLK_ESCAPE:
                exit(1);
                break;
            default:
                break;
            }
            last_down_event = event;
        } else if (event.type == SDL_KEYUP) {
            // printf("Release %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                out_state->values[ODROID_INPUT_A] = 0;
                break;
            case SDLK_z:
                out_state->values[ODROID_INPUT_B] = 0;
                break;
            case SDLK_LSHIFT:
                out_state->values[ODROID_INPUT_START] = 0;
                break;
            case SDLK_LCTRL:
                out_state->values[ODROID_INPUT_SELECT] = 0;
                break;
            case SDLK_UP:
                out_state->values[ODROID_INPUT_UP] = 0;
                break;
            case SDLK_DOWN:
                out_state->values[ODROID_INPUT_DOWN] = 0;
                break;
            case SDLK_LEFT:
                out_state->values[ODROID_INPUT_LEFT] = 0;
                break;
            case SDLK_RIGHT:
                out_state->values[ODROID_INPUT_RIGHT] = 0;
                break;
            case SDLK_F1:
                if (last_down_event.key.keysym.sym == SDLK_F1)
                    SaveStateStm("save_pce.bin");
                break;
            case SDLK_F4:
                if (last_down_event.key.keysym.sym == SDLK_F4)
                    LoadStateStm("save_pce.bin");
                break;                
            default:
                break;
            }
        }
    }
}

void osd_log(int type, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

int main(int argc, char *argv[])
{
    init_window(WIDTH, HEIGHT);

    init();
    odroid_gamepad_state_t joystick = {0};

    while (true)
    {
        //wdog_refresh();
        bool drawFrame = true;// common_emu_frame_loop();

        odroid_input_read_gamepad_pce(&joystick);
        pce_input_read(&joystick);

        for (PCE.Scanline = 0; PCE.Scanline < 263; ++PCE.Scanline) {
            PCE.MaxCycles += CYCLES_PER_LINE;
            h6280_run();
            pce_timer_run();
            gfx_run();
        }
        pce_osd_gfx_blit(drawFrame);
//        if(drawFrame) pce_pcm_submit();

        // Prevent overflow
        int trim = MIN(Cycles, PCE.MaxCycles);
        PCE.MaxCycles -= trim;
        Cycles -= trim;
    }

    SDL_Quit();

    return 0;
}
