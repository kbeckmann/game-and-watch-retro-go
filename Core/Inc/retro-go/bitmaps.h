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


extern const unsigned char IMG_SPEAKER[];
extern const unsigned char IMG_SUN[];
extern const unsigned char IMG_FOLDER[];
extern const unsigned char IMG_DISKETTE[];
extern const unsigned char IMG_0_5X[];
extern const unsigned char IMG_0_75X[]; 
extern const unsigned char IMG_1X[];
extern const unsigned char IMG_1_25X[];
extern const unsigned char IMG_1_5X[];
extern const unsigned char IMG_2X[];
extern const unsigned char IMG_3X[];
extern const unsigned char IMG_SC[];
extern const unsigned char IMG_BUTTON_A[];
extern const unsigned char IMG_BUTTON_A_P[];
extern const unsigned char IMG_BUTTON_B[];
extern const unsigned char IMG_BUTTON_B_P[];