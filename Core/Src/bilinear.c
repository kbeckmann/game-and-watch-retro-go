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


#include <odroid_system.h>
#include <string.h>

#include "main.h"
#include "bilinear.h"

uint8_t row_scratch_buf[320 * 2];


#define OMV_ATTR_ALWAYS_INLINE  inline __attribute__((always_inline))

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define IM_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define IM_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })

#define UINT32_T_BITS   (sizeof(uint32_t) * 8)
#define UINT32_T_MASK   (UINT32_T_BITS - 1)
#define UINT32_T_SHIFT  IM_LOG2(UINT32_T_MASK)


#define IMAGE_BINARY_LINE_LEN(image) (((image)->w + UINT32_T_MASK) >> UINT32_T_SHIFT)
#define IMAGE_BINARY_LINE_LEN_BYTES(image) (IMAGE_BINARY_LINE_LEN(image) * sizeof(uint32_t))

#define IMAGE_GRAYSCALE_LINE_LEN(image) ((image)->w)
#define IMAGE_GRAYSCALE_LINE_LEN_BYTES(image) (IMAGE_GRAYSCALE_LINE_LEN(image) * sizeof(uint8_t))

#define IMAGE_RGB565_LINE_LEN(image) ((image)->w)
#define IMAGE_RGB565_LINE_LEN_BYTES(image) (IMAGE_RGB565_LINE_LEN(image) * sizeof(uint16_t))

#define IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->w * _y); \
})


#define IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

int OMV_ATTR_ALWAYS_INLINE fast_floorf(float x)
{
    int i;
    asm volatile (
            "vcvt.S32.f32  %[r], %[x]\n"
            : [r] "=t" (i)
            : [x] "t"  (x));
    return i;
}


size_t image_size(image_t *ptr)
{
    if (ptr->bpp < 0) {
        return 0;
    }

    switch (ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            return IMAGE_BINARY_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return IMAGE_GRAYSCALE_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_RGB565: {
            return IMAGE_RGB565_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_BAYER: {
            return ptr->w * ptr->h;
        }
        default: { // JPEG
            return ptr->bpp;
        }
    }
}


void imlib_draw_row_setup(imlib_draw_row_data_t *data)
{
    //size_t image_row_size = image_size(data->dst_img) / data->dst_img->h;

    data->toggle = 0;
    // data->row_buffer[0] = fb_alloc(image_row_size, FB_ALLOC_NO_HINT);
    data->row_buffer[0] = row_scratch_buf;
    data->row_buffer[1] = data->row_buffer[0];

    int alpha = data->alpha, max = 256;

    if (data->dst_img->bpp == IMAGE_BPP_RGB565) {
        alpha >>= 3; // 5-bit alpha for RGB565
        max = 32;
    }

    // Set smuad_alpha and smuad_alpha_palette even if we don't use them with DMA2D as we may have
    // to fallback to using them if the draw_image calls imlib_draw_row_put_row_buffer().
    data->smuad_alpha = data->black_background ? alpha : ((alpha << 16) | (max - alpha));
    data->smuad_alpha_palette = NULL;
}

void *imlib_draw_row_get_row_buffer(imlib_draw_row_data_t *data)
{
    void *result = data->row_buffer[data->toggle];
    data->toggle = !data->toggle;
    return result;
}

void imlib_draw_image(image_t *dst_img, image_t *src_img, int dst_x_start, int dst_y_start, float x_scale, float y_scale, rectangle_t *roi,
                      int rgb_channel, int alpha, const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint,
                      imlib_draw_row_callback_t callback, void *dst_row_override)
{
    int dst_delta_x = 1; // positive direction
    if (x_scale < 0.f) { // flip X
        dst_delta_x = -1;
        x_scale = -x_scale;
    }

    int dst_delta_y = 1; // positive direction
    if (y_scale < 0.f) { // flip Y
        dst_delta_y = -1;
        y_scale = -y_scale;
    }

    int src_img_w = roi ? roi->w : src_img->w, w_limit = src_img_w - 1; // w_limit_m_1 = w_limit - 1;
    int src_img_h = roi ? roi->h : src_img->h, h_limit = src_img_h - 1; // h_limit_m_1 = h_limit - 1;

    int src_width_scaled = fast_floorf(x_scale * src_img_w);
    int src_height_scaled = fast_floorf(y_scale * src_img_h);

    // Nothing to draw
    if ((src_width_scaled < 1) || (src_height_scaled < 1)) return;

    // If alpha is 0 then nothing changes.
    if (alpha == 0) return;

    if (alpha_palette) {
        int i = 0;
        while ((i < 256) && (!alpha_palette[i])) i++;
        if (i == 256) return; // zero alpha palette
    }

    // Center src if hint is set.
    if (hint & IMAGE_HINT_CENTER) {
        dst_x_start -= src_width_scaled / 2;
        dst_y_start -= src_height_scaled / 2;
    }

    // Clamp start x to image bounds.
    int src_x_start = 0;
    if (dst_x_start < 0) {
        src_x_start -= dst_x_start; // this is an add becasue dst_x_start is negative
        dst_x_start = 0;
    }

    if (dst_x_start >= dst_img->w) return;
    int src_x_dst_width = src_width_scaled - src_x_start;
    if (src_x_dst_width <= 0) return;

    // Clamp start y to image bounds.
    int src_y_start = 0;
    if (dst_y_start < 0) {
        src_y_start -= dst_y_start; // this is an add becasue dst_y_start is negative
        dst_y_start = 0;
    }

    if (dst_y_start >= dst_img->h) return;
    int src_y_dst_height = src_height_scaled - src_y_start;
    if (src_y_dst_height <= 0) return;

    // Clamp end x to image bounds.
    int dst_x_end = dst_x_start + src_x_dst_width;
    if (dst_x_end > dst_img->w) dst_x_end = dst_img->w;

    // Clamp end y to image bounds.
    int dst_y_end = dst_y_start + src_y_dst_height;
    if (dst_y_end > dst_img->h) dst_y_end = dst_img->h;

    if (dst_delta_x < 0) {
        // Since we are drawing backwards we have to slide our drawing offset forward by an amount
        // limited by the size of the drawing area left. E.g. when we hit the right edge we have
        // advance the offset to prevent the image from sliding.
        int allowed_offset_width = src_width_scaled - (dst_x_end - dst_x_start);
        src_x_start = IM_MIN(dst_x_start, allowed_offset_width);
    }

    // Apply roi offset
    if (roi) src_x_start += fast_floorf(roi->x * x_scale);

    if (dst_delta_y < 0) {
        // Since we are drawing backwards we have to slide our drawing offset forward by an amount
        // limited by the size of the drawing area left. E.g. when we hit the bottom edge we have
        // advance the offset to prevent the image from sliding.
        int allowed_offset_height = src_height_scaled - (dst_y_end - dst_y_start);
        src_y_start = IM_MIN(dst_y_start, allowed_offset_height);
    }

    // Apply roi offset
    if (roi) src_y_start += fast_floorf(roi->y * y_scale);

    // For all of the scaling algorithms (nearest neighbor, bilinear, bicubic, and area)
    // we use a 32-bit fraction instead of a floating point value for iteration. Below,
    // we calculate an increment which fits in 32-bits. We can then add this value
    // successively as we loop over the destination pixels and then shift this sum
    // right by 16 to get the corresponding source pixel. If we want the fractional
    // position we just have to look at the bottom 16-bits.
    //
    // top 16-bits = whole part, bottom 16-bits = fractional part.

    int dst_x_reset = (dst_delta_x < 0) ? (dst_x_end - 1) : dst_x_start;
    long src_x_frac = fast_floorf(65536.0f / x_scale);  // src_x_frac_size = (src_x_frac + 0xFFFF) >> 16;
    long src_x_accum_reset = fast_floorf((src_x_start << 16) / x_scale);

    int dst_y_reset = (dst_delta_y < 0) ? (dst_y_end - 1) : dst_y_start;
    long src_y_frac = fast_floorf(65536.0f / y_scale); // src_y_frac_size = (src_y_frac + 0xFFFF) >> 16;
    long src_y_accum_reset = fast_floorf((src_y_start << 16) / y_scale);

    // Nearest Neighbor
    if ((src_x_frac == 65536) && (src_y_frac == 65536)) hint &= ~(IMAGE_HINT_AREA | IMAGE_HINT_BICUBIC | IMAGE_HINT_BILINEAR);

    // Nearest Neighbor
    if ((hint & IMAGE_HINT_AREA) && (x_scale >= 1.f) && (y_scale >= 1.f)) hint &= ~(IMAGE_HINT_AREA | IMAGE_HINT_BICUBIC | IMAGE_HINT_BILINEAR);

    // Cannot interpolate.
    if ((src_img_w <= 3) || (src_img_h <= 3)) {
        if (hint & IMAGE_HINT_BICUBIC) hint |= IMAGE_HINT_BILINEAR;
        hint &= ~IMAGE_HINT_BICUBIC;
    }

    // Cannot interpolate.
    if ((src_img_w <= 1) || (src_img_h <= 1)) hint &= ~(IMAGE_HINT_AREA | IMAGE_HINT_BILINEAR);

    // Bicbuic and bilinear both shift the image right by (0.5, 0.5) so we have to undo that.
    if (hint & (IMAGE_HINT_BICUBIC | IMAGE_HINT_BILINEAR)) {
        src_x_accum_reset -= 0x8000;
        src_y_accum_reset -= 0x8000;
    }

    // rgb_channel extracted / color_palette applied image
    image_t new_src_img;

    if (((hint & IMAGE_HINT_EXTRACT_RGB_CHANNEL_FIRST) && (src_img->bpp == IMAGE_BPP_RGB565) && (rgb_channel != -1))
    || ((hint & IMAGE_HINT_APPLY_COLOR_PALETTE_FIRST) && color_palette)) {
        new_src_img.w = src_img_w; // same width as source image
        new_src_img.h = src_img_h; // same height as source image
        new_src_img.bpp = color_palette ? IMAGE_BPP_RGB565 : IMAGE_BPP_GRAYSCALE;
        // new_src_img.data = fb_alloc(image_size(&new_src_img), FB_ALLOC_NO_HINT);
        // TODO
        imlib_draw_image(&new_src_img, src_img, 0, 0, 1.f, 1.f, NULL, rgb_channel, 256, color_palette, NULL, 0, NULL, NULL);
        src_img = &new_src_img;
        rgb_channel = -1;
        color_palette = NULL;
    }

    imlib_draw_row_data_t imlib_draw_row_data;
    imlib_draw_row_data.dst_img = dst_img;
    imlib_draw_row_data.src_img_bpp = src_img->bpp;
    imlib_draw_row_data.rgb_channel = rgb_channel;
    imlib_draw_row_data.alpha = alpha;
    imlib_draw_row_data.color_palette = color_palette;
    imlib_draw_row_data.alpha_palette = alpha_palette;
    imlib_draw_row_data.black_background = hint & IMAGE_HINT_BLACK_BACKGROUND;
    imlib_draw_row_data.callback = callback;
    imlib_draw_row_data.dst_row_override = dst_row_override;

    imlib_draw_row_setup(&imlib_draw_row_data);

    // Y loop iteration variables
    int dst_y = dst_y_reset;
    long src_y_accum = src_y_accum_reset;
    int next_src_y_index = src_y_accum >> 16;
    int y = dst_y_start;
    bool y_not_done = y < dst_y_end;



    while (y_not_done) {
        int src_y_index = next_src_y_index;
        uint16_t *src_row_ptr_0, *src_row_ptr_1;

        // keep row pointers in bounds
        if (src_y_index < 0) {
            src_row_ptr_0 = src_row_ptr_1 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, 0);
        } else if (src_y_index >= h_limit) {
            src_row_ptr_0 = src_row_ptr_1 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, h_limit);
        } else { // get 2 neighboring rows
            int src_y_index_p_1 = src_y_index + 1;
            src_row_ptr_0 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, src_y_index);
            src_row_ptr_1 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src_img, src_y_index_p_1);
        }

        do { // Cache the results of getting the source rows
            // used to mix pixels vertically
            long smuad_y = (src_y_accum >> 11) & 0x1f;
            smuad_y |= (32 - smuad_y) << 16;

            // Must be called per loop to get the address of the temp buffer to blend with
            uint16_t *dst_row_ptr = (uint16_t *) imlib_draw_row_get_row_buffer(&imlib_draw_row_data);

            // X loop iteration variables
            int dst_x = dst_x_reset;
            long src_x_accum = src_x_accum_reset;
            int next_src_x_index = src_x_accum >> 16;
            int x = dst_x_start;
            bool x_not_done = x < dst_x_end;

            while (x_not_done) {
                int src_x_index = next_src_x_index;
                int pixel_00, pixel_10, pixel_01, pixel_11;

                // keep pixels in bounds
                if (src_x_index < 0) {
                    pixel_00 = pixel_10 = src_row_ptr_0[0];
                    pixel_01 = pixel_11 = src_row_ptr_1[0];
                } else if (src_x_index >= w_limit) {
                    pixel_00 = pixel_10 = src_row_ptr_0[w_limit];
                    pixel_01 = pixel_11 = src_row_ptr_1[w_limit];
                } else { // get 4 neighboring pixels
                    int src_x_index_p_1 = src_x_index + 1;
                    pixel_00 = src_row_ptr_0[src_x_index]; pixel_10 = src_row_ptr_0[src_x_index_p_1];
                    pixel_01 = src_row_ptr_1[src_x_index]; pixel_11 = src_row_ptr_1[src_x_index_p_1];
                }

                const long mask_r = 0x7c007c00, mask_g = 0x07e007e0, mask_b = 0x001f001f;
                const long avg_rb = 0x4010, avg_g = 0x200;

                uint32_t rgb_l = (pixel_00 << 16) | pixel_01;
                long rb_l = ((rgb_l >> 1) & mask_r) | (rgb_l & mask_b);
                long g_l = rgb_l & mask_g;
                int rb_out_l = (__SMLAD(smuad_y, rb_l, avg_rb) >> 5) & 0x7c1f;
                int g_out_l = (__SMLAD(smuad_y, g_l, avg_g) >> 5) & 0x07e0;

                uint32_t rgb_r = (pixel_10 << 16) | pixel_11;
                long rb_r = ((rgb_r >> 1) & mask_r) | (rgb_r & mask_b);
                long g_r = rgb_r & mask_g;
                int rb_out_r = (__SMLAD(smuad_y, rb_r, avg_rb) >> 5) & 0x7c1f;
                int g_out_r = (__SMLAD(smuad_y, g_r, avg_g) >> 5) & 0x07e0;

                long rb = (rb_out_l << 16) | rb_out_r;
                long g = (g_out_l << 16) | g_out_r;

                do { // Cache the results of getting the source pixels
                    // used to mix pixels horizontally
                    long smuad_x = (src_x_accum >> 11) & 0x1f;
                    smuad_x |= (32 - smuad_x) << 16;

                    int rb_out = __SMLAD(smuad_x, rb, avg_rb) >> 5;
                    int g_out = __SMLAD(smuad_x, g, avg_g) >> 5;
                    int pixel = ((rb_out << 1) & 0xf800) | (g_out & 0x07e0) | (rb_out & 0x001f);

                    IMAGE_PUT_RGB565_PIXEL_FAST(dst_row_ptr, dst_x, pixel);

                    // Increment offsets
                    dst_x += dst_delta_x;
                    src_x_accum += src_x_frac;
                    next_src_x_index = src_x_accum >> 16;
                    x_not_done = ++x < dst_x_end;
                } while (x_not_done && (src_x_index == next_src_x_index));
            } // while x

            // imlib_draw_row(dst_x_start, dst_x_end, dst_y, &imlib_draw_row_data);
            memcpy(dst_img->pixels + dst_x_start * 2 + dst_y * 320 * 2, imlib_draw_row_data.row_buffer[0], (dst_x_end - dst_x_start) * 2);

            // Increment offsets
            dst_y += dst_delta_y;
            src_y_accum += src_y_frac;
            next_src_y_index = src_y_accum >> 16;
            y_not_done = ++y < dst_y_end;
        } while (y_not_done && (src_y_index == next_src_y_index));
    } // while y
}

