#include "odroid_system.h"
#include "odroid_display.h"

short odroid_display_queue_update(odroid_video_frame_t *frame, odroid_video_frame_t *previousFrame)
{
    return SCREEN_UPDATE_FULL;
}
