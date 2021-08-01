#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "rom_manager.h"

#include "mikmod.h"
#include "mikmod_internals.h"

#include "gw_lcd.h"
#include "gw_linker.h"

uint8_t *badheap;
size_t heapsize = (size_t) &__RAM_EMU_LENGTH__;
size_t heap_offset;

void* MikMod_realloc(void *data, size_t size)
{
    if (data) {
        void *new_data = MikMod_malloc(size);
        memcpy(new_data, data, size);
        return new_data;
    } else {
        return MikMod_calloc(1, size);
    }
}

/* Same as malloc, but sets error variable _mm_error when fails */
void* MikMod_malloc(size_t size)
{
    void *ptr = &badheap[heap_offset];
    memset(ptr, 0, size);
    heap_offset += (size + 0b11) & ~0b11; // 32-bit
    assert(heap_offset <= heapsize);
    return ptr;
}

/* Same as calloc, but sets error variable _mm_error when fails */
void* MikMod_calloc(size_t nitems, size_t size)
{
    void *ptr = MikMod_malloc(nitems * size);
    memset(ptr, 0, nitems * size);
    return ptr;
}

void MikMod_free(void *data)
{
    // Heh
}

/* like strdup(), but the result must be freed using MikMod_free() */
CHAR *MikMod_strdup(const CHAR *s)
{
    size_t l;
    CHAR *d;

    if (!s) return NULL;

    l = strlen(s) + 1;
    d = (CHAR *) MikMod_calloc(1, l * sizeof(CHAR));
    if (d) strcpy(d, s);
    return d;
}


void app_main_mikmod(uint8_t load_state, uint8_t start_paused)
{
    MODULE *module;

    printf("MikMod: Play %s\n", ACTIVE_FILE->name);

    badheap = (uint8_t *) &__RAM_EMU_START__;
    heap_offset = 0;

    /* initialize MikMod threads */
    MikMod_InitThreads ();

    /* register all the drivers */
    // MikMod_RegisterAllDrivers();
    _mm_registerdriver(&drv_gw);

    /* register all the module loaders */
    MikMod_RegisterAllLoaders();

    /* init the library */
    md_mode =
        // These ones take effect only after MikMod_Init or MikMod_Reset
        DMODE_16BITS | // enable 16 bit output
        DMODE_STEREO | // enable stereo output
        DMODE_SOFT_SNDFX | // Process sound effects via software mixer
        DMODE_SOFT_MUSIC | // Process music via software mixer
        DMODE_HQMIXER | // Use high-quality (slower) software mixer
        // DMODE_FLOAT | // enable float output

        // These take effect immediately.
        //DMODE_SURROUND | // enable surround sound
        DMODE_INTERP | // enable interpolation
        //DMODE_REVERSE | // reverse stereo
        DMODE_SIMDMIXER | // enable SIMD mixing
        DMODE_NOISEREDUCTION // Low pass filtering
    ;

    // md_reverb = 5;
    // md_pansep = 128;

    md_mixfreq = 48000;
    if (MikMod_Init("")) {
        fprintf(stderr, "Could not initialize sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
        return 2;
    }

    /* load module */
    module = Player_LoadMem(ACTIVE_FILE->address, ACTIVE_FILE->size, 1, 0);
    if (module) {
        /* start module */
        printf("Playing %s (%d chn)\n", module->songname, (int) module->numchn);
        Player_Start(module);

        while (Player_Active()) {
            MikMod_Update();
        }

        Player_Stop();
        Player_Free(module);
    } else
        fprintf(stderr, "Could not load module, reason: %s\n",
                MikMod_strerror(MikMod_errno));

    printf("MikMod: Done\n");

    MikMod_Exit();
}
