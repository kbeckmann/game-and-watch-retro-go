#pragma once
#include <stdint.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    const char logo[];
} retro_logo_image;


extern const retro_logo_image logo_rgo;
extern const retro_logo_image logo_flash;
extern const retro_logo_image logo_gnw;

extern const retro_logo_image header_sg1000;
extern const retro_logo_image header_col;
extern const retro_logo_image header_gb;
extern const retro_logo_image header_gg;
extern const retro_logo_image header_nes;
extern const retro_logo_image header_pce;
extern const retro_logo_image header_sms;
extern const retro_logo_image header_gw;

extern const retro_logo_image logo_coleco;
extern const retro_logo_image logo_nitendo;
extern const retro_logo_image logo_sega;
extern const retro_logo_image logo_pce;