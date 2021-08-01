#pragma GCC optimize("-O0")

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#define DRV_GW

#ifdef DRV_GW

#include <malloc.h>
#include "mikmod_internals.h"

static SBYTE *N64_buffer = NULL;

static BOOL GW_IsThere(void)
{
	return 1;
}

static int GW_Init(void)
{
    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH * 2 );
    printf("Sound initialized\n");

	return VC_Init();
}

static void GW_Exit(void)
{
	VC_Exit();
}

int16_t render_buffer[AUDIO_BUFFER_LENGTH * 2];

static void GW_Update(void)
{
	int i;
	uint8_t volume = odroid_audio_volume_get();
	int32_t factor = volume_tbl[volume] / 2;

	size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH;
	int16_t *next_audio_buf = &audiobuffer_dma[offset];

	if (md_mode & DMODE_STEREO)
	{
		VC_WriteBytes(render_buffer, AUDIO_BUFFER_LENGTH * 4);
		for (i = 0; i < AUDIO_BUFFER_LENGTH; i++)
		{
			/* Convert stereo to mono. */
			int32_t sample = render_buffer[i*2] + render_buffer[i*2 + 1];
			next_audio_buf[i] = (sample * factor) >> 8;
		}
	}
	else
	{
		VC_WriteBytes(render_buffer, AUDIO_BUFFER_LENGTH * 2);
		for (i = 0; i < AUDIO_BUFFER_LENGTH; i++)
		{
			int32_t sample = render_buffer[i];
			next_audio_buf[i] = (sample * factor) >> 8;
		}
	}
	wdog_refresh();

	static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
	while (dma_state == last_dma_state) {
		cpumon_sleep();
	}
	last_dma_state = dma_state;
}

MIKMODAPI MDRIVER drv_gw={
	NULL,
	"Game and Watch",
	"GW Driver v1.0",
	255,255,
	"gw",
	NULL,
	NULL,
	GW_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	GW_Init,
	GW_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	GW_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

#else

#include "mikmod_internals.h"
MISSING(drv_gw);

#endif

/* ex:set ts=4: */
