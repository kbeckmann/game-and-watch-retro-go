#include "odroid_system.h"
#include "odroid_audio.h"

uint8_t audio_level = ODROID_AUDIO_VOLUME_MAX;

void odroid_audio_submit(short* stereoAudioBuffer, int frameCount)
{
}

void odroid_audio_volume_set(int level)
{
    audio_level = level;
}

int odroid_audio_volume_get()
{
    return audio_level;
}
