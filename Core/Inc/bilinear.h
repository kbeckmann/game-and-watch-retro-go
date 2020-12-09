#pragma once

/*

https://github.com/openmv/openmv/

The MIT License (MIT)

Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

*/



typedef enum image_bpp
{
    IMAGE_BPP_BINARY,       // BPP = 0
    IMAGE_BPP_GRAYSCALE,    // BPP = 1
    IMAGE_BPP_RGB565,       // BPP = 2
    IMAGE_BPP_BAYER,        // BPP = 3
    IMAGE_BPP_JPEG          // BPP > 3
}
image_bpp_t;

typedef struct rectangle {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} rectangle_t;

typedef struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;

typedef enum image_hint {
    IMAGE_HINT_AREA = 1 << 0,
    IMAGE_HINT_BILINEAR = 1 << 1,
    IMAGE_HINT_BICUBIC = 1 << 2,
    IMAGE_HINT_CENTER = 1 << 7,
    IMAGE_HINT_EXTRACT_RGB_CHANNEL_FIRST = 1 << 8,
    IMAGE_HINT_APPLY_COLOR_PALETTE_FIRST = 1 << 9,
    IMAGE_HINT_BLACK_BACKGROUND = 1 << 31
} image_hint_t;

typedef struct imlib_draw_row_data {
    image_t *dst_img; // user
    int src_img_bpp; // user
    int rgb_channel; // user
    int alpha; // user
    const uint16_t *color_palette; // user
    const uint8_t *alpha_palette; // user
    bool black_background; // user
    void *callback; // user
    void *dst_row_override; // user
    int toggle; // private
    void *row_buffer[2]; // private
    #ifdef IMLIB_ENABLE_DMA2D
    bool dma2d_request; // user
    bool dma2d_enabled; // private
    bool dma2d_initialized; // private
    DMA2D_HandleTypeDef dma2d; // private
    #endif
    long smuad_alpha; // private
    uint32_t *smuad_alpha_palette; // private
} imlib_draw_row_data_t;

typedef void (*imlib_draw_row_callback_t)(int x_start, int x_end, int y_row, imlib_draw_row_data_t *data);

void imlib_draw_image(image_t *dst_img, image_t *src_img, int dst_x_start, int dst_y_start, float x_scale, float y_scale, rectangle_t *roi,
                      int rgb_channel, int alpha, const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint,
                      imlib_draw_row_callback_t callback, void *dst_row_override);
