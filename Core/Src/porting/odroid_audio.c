#include "odroid_system.h"
#include "odroid_audio.h"
#include "common.h"
#include <assert.h>

#include "stm32h7xx_hal.h"

uint8_t audio_level = ODROID_AUDIO_VOLUME_MAX;

/* set audio frequency  */
static void set_audio_frequency(uint32_t frequency)
{

    /** reconfig PLL2 and SAI */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;

    /* Reconfigure on the fly PLL2 */
    /* config to get 32768Hz */
    /* The audio clock frequency is derived directly */
    /* SAI mode is MCKDIV mode */
    if (frequency == 32768)
    {

        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 196;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2Q = 2;
        PeriphClkInitStruct.PLL2.PLL2R = 5;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 5000;

    /* config to get 48KHz and multiple */
    /* SAI mode is in standard frequency mode */
    }
    else
    {

        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 192;
        PeriphClkInitStruct.PLL2.PLL2P = 5;
        PeriphClkInitStruct.PLL2.PLL2Q = 2;
        PeriphClkInitStruct.PLL2.PLL2R = 5;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    }

    // keep PLL3 unchanged
    PeriphClkInitStruct.PLL3.PLL3M = 4;
    PeriphClkInitStruct.PLL3.PLL3N = 9;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 24;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* remove the current configuration */
    HAL_SAI_DeInit(&hsai_BlockA1);

    /* Set Audio sample rate at 32768Hz using MCKDIV mode */
    if (frequency == 32768)
    {

        hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_MCKDIV;
        hsai_BlockA1.Init.Mckdiv = 6;

        /* config to get 48KHz and other standard values */
        /*
    SAI_AUDIO_FREQUENCY_192K      192000U
    SAI_AUDIO_FREQUENCY_96K        96000U
    SAI_AUDIO_FREQUENCY_48K        48000U
    SAI_AUDIO_FREQUENCY_44K        44100U
    SAI_AUDIO_FREQUENCY_32K        32000U
    SAI_AUDIO_FREQUENCY_22K        22050U
    SAI_AUDIO_FREQUENCY_16K        16000U
    SAI_AUDIO_FREQUENCY_11K        11025U
    SAI_AUDIO_FREQUENCY_8K          8000U
    */

    /* Set Audio sample rate at various standard frequencies using AudioFrequency mode */
    } else {
        /* default value 48KHz */
        hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;

        /* check from the different possible values */
        if ((frequency == SAI_AUDIO_FREQUENCY_192K) ||
            (frequency == SAI_AUDIO_FREQUENCY_96K) ||
            (frequency == SAI_AUDIO_FREQUENCY_48K) ||
            (frequency == SAI_AUDIO_FREQUENCY_44K) ||
            (frequency == SAI_AUDIO_FREQUENCY_32K) ||
            (frequency == SAI_AUDIO_FREQUENCY_22K) ||
            (frequency == SAI_AUDIO_FREQUENCY_16K) ||
            (frequency == SAI_AUDIO_FREQUENCY_11K) ||
            (frequency == SAI_AUDIO_FREQUENCY_8K))
            hsai_BlockA1.Init.AudioFrequency = frequency;
    }

    /* apply the new configuration */
    HAL_SAI_Init(&hsai_BlockA1);
}

void odroid_audio_init(int sample_rate)
{
    set_audio_frequency(sample_rate);
    audio_level = odroid_settings_Volume_get();
}

void odroid_audio_submit(short* stereoAudioBuffer, int frameCount)
{
}

void odroid_audio_volume_set(int level)
{
    audio_level = level;
    odroid_settings_Volume_set(audio_level);
}

int odroid_audio_volume_get()
{
    return audio_level;
}
