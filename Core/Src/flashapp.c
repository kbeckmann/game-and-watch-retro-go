#include <odroid_system.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "githash.h"
#include "gui.h"
#include "bitmaps.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "main.h"
#include "rg_emulators.h"

#include "utils.h"
#include "sha256.h"

#define DBG(...) printf(__VA_ARGS__)
// #define DBG(...)

#define STATUS_HEIGHT (33)
#define HEADER_HEIGHT (47)
#define IMAGE_BANNER_HEIGHT (32)
#define IMAGE_BANNER_WIDTH  (ODROID_SCREEN_WIDTH)

static const int font_height = 8; //odroid_overlay_get_font_size();
static const int font_width = 8; //odroid_overlay_get_font_width();

#define LIST_X_OFFSET    (0)
#define LIST_Y_OFFSET    (STATUS_HEIGHT)
#define LIST_WIDTH       (ODROID_SCREEN_WIDTH)
#define LIST_HEIGHT      (ODROID_SCREEN_HEIGHT - STATUS_HEIGHT - HEADER_HEIGHT)
#define LIST_LINE_HEIGHT (font_height + 2)
#define LIST_LINE_COUNT  (LIST_HEIGHT / LIST_LINE_HEIGHT)

#define PROGRESS_X_OFFSET (ODROID_SCREEN_WIDTH / 5 / 2)
#define PROGRESS_Y_OFFSET (LIST_Y_OFFSET + 9 * LIST_LINE_HEIGHT)
#define PROGRESS_WIDTH    (4 * (PROGRESS_X_OFFSET * 2))
#define PROGRESS_HEIGHT   (2 * LIST_LINE_HEIGHT)

// TODO: Make this nicer
extern OSPI_HandleTypeDef hospi1;

typedef enum {
    FLASHAPP_INIT                   = 0x00,
    FLASHAPP_IDLE                   = 0x01,
    FLASHAPP_START                  = 0x02,
    FLASHAPP_CHECK_HASH_RAM_NEXT    = 0x03,
    FLASHAPP_CHECK_HASH_RAM         = 0x04,
    FLASHAPP_ERASE_NEXT             = 0x05,
    FLASHAPP_ERASE                  = 0x06,
    FLASHAPP_PROGRAM_NEXT           = 0x07,
    FLASHAPP_PROGRAM                = 0x08,
    FLASHAPP_CHECK_HASH_FLASH_NEXT  = 0x09,
    FLASHAPP_CHECK_HASH_FLASH       = 0x0A,

    FLASHAPP_TEST_NEXT              = 0x0B,
    FLASHAPP_TEST                   = 0x0C,

    FLASHAPP_FINAL                  = 0x0D,
    FLASHAPP_ERROR                  = 0x0E,
} flashapp_state_t;

typedef enum {
    FLASHAPP_STATUS_BAD_HASH_RAM    = 0xbad00001,
    FLASHAPP_STATUS_BAD_HAS_FLASH   = 0xbad00002,
    FLASHAPP_STATUS_NOT_ALIGNED     = 0xbad00003,

    FLASHAPP_STATUS_IDLE            = 0xcafe0000,
    FLASHAPP_STATUS_DONE            = 0xcafe0001,
    FLASHAPP_STATUS_BUSY            = 0xcafe0002,
} flashapp_status_t;

typedef struct {
    tab_t    tab;
    uint32_t erase_address;
    uint32_t erase_bytes_left;
    uint32_t current_program_address;
    uint32_t program_bytes_left;
    uint8_t* program_buf;
    uint32_t progress_max;
    uint32_t progress_value;
} flashapp_t;

// framebuffer1 is used as an actual framebuffer.
// framebuffer2 and onwards is used as a buffer for the flash.
static uint8_t *flash_buffer = (uint8_t *) framebuffer2;

// Values below are read or written by the debugger

// Store state in a uint32_t
uint32_t flashapp_state;

// Set to non-zero to start programming
uint32_t program_start;

// Status register
uint32_t program_status;

// Number of bytes to program in the flash
uint32_t program_size;

// Where to program in the flash
uint32_t program_address;

// Control if chip should be erased or not
uint32_t program_erase;

// Number of bytes to be erased from program_address
int32_t program_erase_bytes;

// Current chunk index
uint32_t program_chunk_idx;

// Number of chunks
uint32_t program_chunk_count;

// The expected sha256 of the loaded binary
uint8_t program_expected_sha256[65];

// TODO: Expose properly
int odroid_overlay_draw_text_line(uint16_t x_pos,
                                  uint16_t y_pos,
                                  uint16_t width,
                                  const char *text,
                                  uint16_t color,
                                  uint16_t color_bg);

static void draw_text_line_centered(uint16_t y_pos,
                                    const char *text,
                                    uint16_t color,
                                    uint16_t color_bg)
{
    int width = strlen(text) * font_width;
    int x_pos = ODROID_SCREEN_WIDTH / 2 - width / 2;

    odroid_overlay_draw_text_line(x_pos, y_pos, width, text, color, color_bg);
}

static void draw_progress(flashapp_t *flashapp)
{
    char progress_str[16];

    odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET, LIST_WIDTH, LIST_HEIGHT, curr_colors->bg_c);

    odroid_overlay_draw_text_line(8, LIST_Y_OFFSET + LIST_LINE_HEIGHT, strlen(flashapp->tab.status) * font_width, flashapp->tab.status, curr_colors->sel_c, curr_colors->bg_c);

    draw_text_line_centered(LIST_Y_OFFSET + 5 * LIST_LINE_HEIGHT, flashapp->tab.name, curr_colors->sel_c, curr_colors->bg_c);

    if (flashapp->progress_max != 0) {
        int32_t progress_percent = (100 * (uint64_t)flashapp->progress_value) / flashapp->progress_max;
        int32_t progress_width = (PROGRESS_WIDTH * (uint64_t)flashapp->progress_value) / flashapp->progress_max;

        sprintf(progress_str, "%ld%%", progress_percent);

        odroid_overlay_draw_fill_rect(PROGRESS_X_OFFSET,
                                      PROGRESS_Y_OFFSET,
                                      PROGRESS_WIDTH,
                                      PROGRESS_HEIGHT,
                                      curr_colors->main_c);

        odroid_overlay_draw_fill_rect(PROGRESS_X_OFFSET,
                                      PROGRESS_Y_OFFSET,
                                      progress_width,
                                      PROGRESS_HEIGHT,
                                      curr_colors->sel_c);

        draw_text_line_centered(LIST_Y_OFFSET + 8 * LIST_LINE_HEIGHT, progress_str, curr_colors->sel_c, curr_colors->bg_c);
    }
}

static void redraw(flashapp_t *flashapp)
{
    // Re-use header, status and footer from the retro-go code
    gui_draw_header(&flashapp->tab);
    gui_draw_status(&flashapp->tab);

    // Empty logo
    //odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - IMAGE_BANNER_HEIGHT - 15,
    //                              IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, curr_colors->main_c);

    draw_progress(flashapp);
    lcd_swap();
}


static bool validate_erased(uint32_t address, uint32_t size)
{
    assert((size & 0b11) == 0);
    assert((address & 0b11) == 0);

    uint32_t *flash_ptr_u32 = (uint32_t *)(0x90000000 + address);

    for (uint32_t i = 0; i < size / 4; i++) {
        if (flash_ptr_u32[i] != 0xffffffff) {
            return false;
        }
    }

    return true;
}

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}

static void generate_random(uint32_t *buf, uint32_t size, uint32_t seed)
{
    for (int i = 0; i < size / 4; i++) {
        buf[i] = xorshift32(&seed);
    }
}

const uint32_t tests[][2] = {
    //     start,        end
    {   0 * 1024,   4 * 1024 }, //        1 *  4k
    {  32 * 1024,  64 * 1024 }, //        1 * 32k
    {  64 * 1024, 128 * 1024 }, //        1 * 64k
    { 252 * 1024, 260 * 1024 }, //   8k = 2 *  4k
    { 384 * 1024, 508 * 1024 }, // 124k = 64k + 32k + 7 * 4k
};

static uint32_t test_read(uint32_t addr, uint32_t len)
{
    const uint8_t *flash_ptr = (const uint8_t *) 0x90000000;
    uint32_t t0;
    uint32_t t1;

    t0 = HAL_GetTick();
    while (len != 0) {
        uint32_t chunk_len = (len > sizeof(emulator_framebuffer)) ? sizeof(emulator_framebuffer) : len;
        memcpy(emulator_framebuffer, flash_ptr, chunk_len);
        flash_ptr += chunk_len;
        len -= chunk_len;
    }
    t1 = HAL_GetTick();

    return t1 - t0;
}

static void test_flash(flashapp_t *flashapp)
{
    const uint8_t *flash_ptr = (const uint8_t *) 0x90000000;
    uint32_t address;
    uint32_t size;
    uint32_t start;
    uint32_t end;

    const uint32_t rand_size = 512 * 1024;

    sprintf(flashapp->tab.status, "Game and Watch Flash App TEST");
    sprintf(flashapp->tab.name, "Erase and program..");
    lcd_swap();
    lcd_wait_for_vblank();
    redraw(flashapp);

    // Erase 512kB at 0
    address = 0;
    size = rand_size;
    OSPI_DisableMemoryMappedMode();
    OSPI_EraseSync(address, size);
    OSPI_EnableMemoryMappedMode();
    assert(validate_erased(address, size));

    generate_random((uint32_t *) flash_buffer, rand_size, 0x12345678);

    // Write and verify 512kB random data
    address = 0;
    size = rand_size;
    OSPI_DisableMemoryMappedMode();
    OSPI_Program(address, &flash_buffer[address], size);
    OSPI_EnableMemoryMappedMode();

    // Erase parts of the flash and verify that the non erased data is still intact
    for (int i = 0; i < ARRAY_SIZE(tests); i++) {
        start = tests[i][0];
        end = tests[i][1];
        size = end - start;

        DBG("Erase test %d start=%08lx end=%08lx\n", i, start, end);
        sprintf(flashapp->tab.name, "Erase test %d start=%08lx end=%08lx", i, start, end);

        // Check that data is ok first
        assert(memcmp(&flash_ptr[start], &flash_buffer[start], size) == 0);

        // Erase
        OSPI_DisableMemoryMappedMode();
        OSPI_EraseSync(start, size);
        OSPI_EnableMemoryMappedMode();

        // Check that erased is actually erased
        assert(validate_erased(start, size));

        // Check that the rest of the data is still there
        assert(memcmp(&flash_ptr[start + size], &flash_buffer[start + size], rand_size - end) == 0);

        lcd_swap();
        lcd_wait_for_vblank();
        redraw(flashapp);
    }

    DBG("[OK] Erase ok\n");
    sprintf(flashapp->tab.name, "Flash diag test OK");
    redraw(flashapp);

    // Do a read test of the first 1MB
    uint32_t read_ms = test_read(0, 1024 * 1024);
    sprintf(flashapp->tab.name, "All OK. Flash read: %ld.%02ld MB/s", 1024 / read_ms, (100 * 1024 / read_ms) % 100);
    lcd_swap();
    lcd_wait_for_vblank();
    redraw(flashapp);
}

static void state_set(flashapp_state_t state_next)
{
    printf("State: %ld -> %d\n", flashapp_state, state_next);

    flashapp_state = state_next;
}

static void state_inc(void)
{
    state_set(flashapp_state + 1);
}

static void flashapp_run(flashapp_t *flashapp)
{
    uint8_t program_calculated_sha256[65];

    switch (flashapp_state) {
    case FLASHAPP_INIT:
        // Clear variables shared with the host
        program_size = 0;
        program_address = 0;
        program_status = 0;
        program_erase = 0;
        program_erase_bytes = 0;
        program_chunk_idx = 1;
        program_chunk_count = 1;
        memset(program_expected_sha256, 0, sizeof(program_expected_sha256));
        memset(program_calculated_sha256, 0, sizeof(program_calculated_sha256));

        flashapp->progress_value = 0;
        flashapp->progress_max = 0;

        state_inc();
        break;
    case FLASHAPP_IDLE:
        sprintf(flashapp->tab.name, "1. Waiting for data");

        // Notify that we are ready to start
        program_status = FLASHAPP_STATUS_IDLE;
        flashapp->progress_value = 0;
        flashapp->progress_max = 0;

        // program_start is set by the flash script
        switch (program_start) {
        case 1: // Normal flash operation
            program_start = 0;
            state_inc();
            break;
        case 2: // Test flash
            program_start = 0;
            state_set(FLASHAPP_TEST_NEXT);
            break;
        default:
            break;
        }

        break;
    case FLASHAPP_START:
        program_status = FLASHAPP_STATUS_BUSY;
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_RAM_NEXT:
        sprintf(flashapp->tab.name, "2. Checking hash in RAM (%ld bytes)", program_size);
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_RAM:
        // Calculate sha256 hash of the RAM first
        sha256_to_string(program_calculated_sha256, (const BYTE*) flash_buffer, program_size);

        if (strncmp((char *)program_calculated_sha256, (char *)program_expected_sha256, 64) != 0) {
            // Hashes don't match even in RAM, openocd loading failed.
            sprintf(flashapp->tab.name, "*** Hash mismatch in RAM ***");
            program_status = FLASHAPP_STATUS_BAD_HASH_RAM;
            state_set(FLASHAPP_ERROR);
            break;
        } else {
            sprintf(flashapp->tab.name, "3. Hash OK in RAM");
            state_inc();
        }
        break;
    case FLASHAPP_ERASE_NEXT:
        OSPI_DisableMemoryMappedMode();

        if (program_erase) {
            if (program_erase_bytes == 0) {
                sprintf(flashapp->tab.name, "4. Performing Chip Erase (takes time)");
            } else {
                flashapp->erase_address = program_address;
                flashapp->erase_bytes_left = program_erase_bytes;

                uint32_t smallest_erase = OSPI_GetSmallestEraseSize();

                if (flashapp->erase_address & (smallest_erase - 1)) {
                    sprintf(flashapp->tab.name, "** Address not aligned to smallest erase size! **");
                    program_status = FLASHAPP_STATUS_NOT_ALIGNED;
                    state_set(FLASHAPP_ERROR);
                    break;
                }

                // Round size up to nearest erase size if needed ?
                if ((flashapp->erase_bytes_left & (smallest_erase - 1)) != 0) {
                    flashapp->erase_bytes_left += smallest_erase - (flashapp->erase_bytes_left & (smallest_erase - 1));
                }

                sprintf(flashapp->tab.name, "4. Erasing %ld bytes...", flashapp->erase_bytes_left);
                printf("Erasing %ld bytes at 0x%08lx\n", flashapp->erase_bytes_left, flashapp->erase_address);
                flashapp->progress_max = program_erase_bytes;
                flashapp->progress_value = 0;
            }
            state_inc();
        } else {
            state_set(FLASHAPP_PROGRAM_NEXT);
        }
        break;
    case FLASHAPP_ERASE:
        if (program_erase_bytes == 0) {
            OSPI_NOR_WriteEnable();
            OSPI_ChipErase();
            state_inc();
        } else {
            if (OSPI_Erase(&flashapp->erase_address, &flashapp->erase_bytes_left)) {
                flashapp->progress_max = 0;
                state_inc();
            }
            flashapp->progress_value = flashapp->progress_max - flashapp->erase_bytes_left;
        }
        break;
    case FLASHAPP_PROGRAM_NEXT:
        sprintf(flashapp->tab.name, "5. Programming...");
        flashapp->progress_value = 0;
        flashapp->progress_max = program_size;
        flashapp->current_program_address = program_address;
        flashapp->program_bytes_left = program_size;
        flashapp->program_buf = flash_buffer;
        state_inc();
        break;
    case FLASHAPP_PROGRAM:
        if (flashapp->program_bytes_left > 0) {
            uint32_t dest_page = flashapp->current_program_address / 256;
            uint32_t bytes_to_write = flashapp->program_bytes_left > 256 ? 256 : flashapp->program_bytes_left;
            OSPI_NOR_WriteEnable();
            OSPI_PageProgram(dest_page * 256, flashapp->program_buf, bytes_to_write);
            flashapp->current_program_address += bytes_to_write;
            flashapp->program_buf += bytes_to_write;
            flashapp->program_bytes_left -= bytes_to_write;
            flashapp->progress_value = program_size - flashapp->program_bytes_left;
        } else {
            state_inc();
        }
        break;
    case FLASHAPP_CHECK_HASH_FLASH_NEXT:
        sprintf(flashapp->tab.name, "6. Checking hash in FLASH");
        OSPI_EnableMemoryMappedMode();
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_FLASH:
        // Calculate sha256 hash of the FLASH.
        sha256_to_string(program_calculated_sha256,
                         (const BYTE*) (0x90000000 + program_address),
                         program_size);

        if (strncmp((char *)program_calculated_sha256, (char *)program_expected_sha256, 64) != 0) {
            // Hashes don't match in FLASH, programming failed.
            sprintf(flashapp->tab.name, "*** Hash mismatch in FLASH ***");
            program_status = FLASHAPP_STATUS_BAD_HAS_FLASH;
            state_set(FLASHAPP_ERROR);
        } else {
            sprintf(flashapp->tab.name, "7. Hash OK in FLASH.");

            if (program_chunk_idx != program_chunk_count) {
                // More chunks will be programmed, skip the init state.
                program_chunk_idx++;
                state_set(FLASHAPP_IDLE);
            } else {
                sprintf(flashapp->tab.name, "Programming done!");
                program_status = FLASHAPP_STATUS_DONE;
                state_set(FLASHAPP_FINAL);
            }
        }
        break;
    case FLASHAPP_TEST_NEXT:
        test_flash(flashapp);
        state_inc();
        break;
    case FLASHAPP_TEST:
    case FLASHAPP_FINAL:
    case FLASHAPP_ERROR:
        // Stay in state until reset.
        break;
    }
}


void flashapp_main(void)
{
    flashapp_t flashapp = {};
    flashapp.tab.img_header = &logo_flash;

    SCB_InvalidateDCache();
    SCB_DisableDCache();

    odroid_system_init(0, 32000);
    lcd_set_buffers(framebuffer1, framebuffer1);

    while (true) {
        if (program_chunk_count == 1) {
            sprintf(flashapp.tab.status, "Game and Watch Flash App");
        } else {
            sprintf(flashapp.tab.status, "Game and Watch Flash App (%ld/%ld)",
                    program_chunk_idx, program_chunk_count);
        }

        // Run multiple times to skip rendering when programming
        for (int i = 0; i < 128; i++) {
            wdog_refresh();
            flashapp_run(&flashapp);
            if (flashapp_state != FLASHAPP_PROGRAM) {
                break;
            }
        }

        lcd_sync();
        lcd_swap();
        lcd_wait_for_vblank();
        redraw(&flashapp);
    }
}
