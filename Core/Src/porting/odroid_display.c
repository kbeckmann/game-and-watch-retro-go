#include "odroid_system.h"
#include "odroid_display.h"
#include "gw_lcd.h"

short odroid_display_queue_update(odroid_video_frame_t *frame, odroid_video_frame_t *previousFrame)
{
    return SCREEN_UPDATE_FULL;
}

void odroid_display_write_rect(short left, short top, short width, short height, short stride, const uint16_t* buffer)
{
    pixel_t *dest = active_framebuffer ? framebuffer2 : framebuffer1;

    for (short y = 0; y < height; y++) {
        pixel_t *dest_row = &dest[(y + top) * GW_LCD_WIDTH + left];
        memcpy(dest_row, &buffer[y * stride], width * sizeof(pixel_t));
    }
}

// Same as odroid_display_write_rect but stride is assumed to be width (for backwards compat)
void odroid_display_write(short left, short top, short width, short height, const uint16_t* buffer)
{
    odroid_display_write_rect(left, top, width, height, width, buffer);
}

void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}

