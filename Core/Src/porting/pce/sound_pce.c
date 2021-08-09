// sound.c - Sound emulation
//
#include "sound_pce.h"
#include "pce.h"

typedef int16_t sample_t;
static uint32_t noise_rand[PSG_CHANNELS];
static int32_t noise_level[PSG_CHANNELS];
// The buffer should be signed but it seems to sound better
// unsigned. I am still reviewing the implementation bellow.
// static uint8_t mix_buffer[44100 / 60 * 2];
static sample_t mix_buffer[PCE_SAMPLE_RATE / 60 * 2];

struct host_machine host;

static inline void
psg_update_chan(sample_t *buf, int ch, size_t dwSize)
{
    psg_chan_t *chan = &PCE.PSG.chan[ch];
    int sample = 0;
    uint32_t Tp;
    sample_t *buf_end = buf + dwSize;

    /*
    * This gives us a volume level of (0...15).
    */
    int lvol = (((chan->balance >> 4) * 1.1) * (chan->control & 0x1F)) / 32;
    int rvol = (((chan->balance & 0xF) * 1.1) * (chan->control & 0x1F)) / 32;

    if (!host.sound.stereo) {
        lvol = (lvol + rvol) / 2;
    }

    // This isn't very accurate, we don't track how long each DA sample should play
    // but we call psg_update() often enough (10x per frame) that guessing should be good enough...
    if (chan->dda_count) {
        // Cycles per frame: 119318
        // Samples per frame: 368

        // Cycles per scanline: 454
        // Samples per scanline: ~1.4

        // One sample = 324 cycles

        // const int cycles_per_sample = CYCLES_PER_FRAME / (host.sound.sample_freq / 60);

        // float repeat = (float)elapsed / cycles_per_sample / chan->dda_count;
        // printf("%.2f\n", repeat);

        int start = (int)chan->dda_index - chan->dda_count;
        if (start < 0)
            start += 0x100;

        int repeat = 3; // MIN(2, (dwSize / 2) / chan->dda_count) + 1;

        while (buf < buf_end && (chan->dda_count || chan->control & PSG_DDA_ENABLE)) {
            if (chan->dda_count) {
                // sample = chan->dda_data[(start++) & 0x7F];
                if ((sample = (chan->dda_data[(start++) & 0xFF] - 16)) >= 0)
                    sample++;
                chan->dda_count--;
            }

            for (int i = 0; i < repeat; i++) {
                *buf++ = (sample * lvol);

                if (host.sound.stereo) {
                    *buf++ = (sample * rvol);
                }
            }
        }
    }

    /*
    * Do nothing if there is no audio to be played on this channel.
    */
    if (!(chan->control & PSG_CHAN_ENABLE)) {
        chan->wave_accum = 0;
    }
    /*
    * PSG Noise generation (it has priority over DDA and WAVE)
    */
    else if ((ch == 4 || ch == 5) && (chan->noise_ctrl & PSG_NOISE_ENABLE)) {
        int Np = (chan->noise_ctrl & 0x1F);

        while (buf < buf_end) {
            chan->noise_accum += 3000 + Np * 512;

            if ((Tp = (chan->noise_accum / host.sound.freq)) >= 1) {
                if (noise_rand[ch] & 0x00080000) {
                    noise_rand[ch] = ((noise_rand[ch] ^ 0x0004) << 1) + 1;
                    noise_level[ch] = -15;
                } else {
                    noise_rand[ch] <<= 1;
                    noise_level[ch] = 15;
                }
                chan->noise_accum -= host.sound.freq * Tp;
            }

            *buf++ = (noise_level[ch] * lvol);

            if (host.sound.stereo) {
                *buf++ = (noise_level[ch] * rvol);
            }
        }
    }
    /*
    * There is 'direct access' audio to be played.
    */
    else if (chan->control & PSG_DDA_ENABLE) {

    }
    /*
    * PSG Wave generation.
    */
    else if ((Tp = chan->freq_lsb + (chan->freq_msb << 8)) > 0) {
        /*
         * Thank god for well commented code!  The original line of code read:
         * fixed_inc = ((uint32_t) (3.2 * 1118608 / host.sound.freq) << 16) / Tp;
         * and had nary a comment to be found.  It took a little head scratching to get
         * it figured out.  The 3.2 * 1118608 comes out to 3574595.6 which is obviously
         * meant to represent the 3.58mhz cpu clock speed used in the pc engine to
         * decrement the sound 'frequency'.  I haven't figured out why the original
         * author had the two numbers multiplied together to get the odd value instead of
         * just using 3580000.  I did some checking and the value will compute the same
         * using either value divided by any standard soundcard samplerate.  The
         * host.sound.freq is our soundcard's samplerate which is quite a bit slower than
         * the pce's cpu (3580000 vs. 22050/44100 typically).
         *
         * Taken from the PSG doc written by Paul Clifford (paul@plasma.demon.co.uk)
         * <in reference to the 12 bit frequency value in PSG registers 2 and 3>
         * "For waveform output, a copy of this value is, in effect, decremented 3,580,000
         *  times a second until zero is reached.  When this happens the PSG advances an
         *  internal pointer into the channel's waveform buffer by one."
         *
         * So all we need to do to emulate original pc engine behaviour is take our soundcard's
         * sampling rate into consideration with regard to the 3580000 effective pc engine
         * samplerate.  We use 16.16 fixed arithmetic for speed.
         */
        uint32_t fixed_inc = ((CLOCK_PSG / host.sound.freq) << 16) / Tp;

        while (buf < buf_end) {
            if ((sample = (chan->wave_data[chan->wave_index] - 16)) >= 0)
                sample++;

            *buf++ = (sample * lvol);

            if (host.sound.stereo) {
                *buf++ = (sample * rvol);
            }

            chan->wave_accum += fixed_inc;
            chan->wave_accum &= 0x1FFFFF;    /* (31 << 16) + 0xFFFF */
            chan->wave_index = chan->wave_accum >> 16;
        }
    }

//pad_and_return:
    if (buf < buf_end) {
        memset(buf, 0, (void*)buf_end - (void*)buf);
    }
}

void osd_snd_init(void) {
    host.sound.stereo = true;
    host.sound.freq = PCE_SAMPLE_RATE;
    host.sound.sample_size = 1;
    //xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 2, NULL, 5, NULL, 1);
}

int pce_snd_init(void) {
    noise_rand[4] = 0x51F63101;
    noise_rand[5] = 0x1F631042;
    osd_snd_init();
    return 0;
}


void pce_snd_term(void) {
    osd_snd_shutdown();
}


void pce_snd_update(int16_t *output, unsigned length) {
    int lvol = (PCE.PSG.volume >> 4);
    int rvol = (PCE.PSG.volume & 0x0F);

    if (host.sound.stereo) {
        length *= 2;
    }

    memset(output, 0, length * 2);

    for (int i = 0; i < PSG_CHANNELS; i++) {
        psg_update_chan((void*)mix_buffer, i, length);
        for (int j = 0; j < length; j += 2) {
            output[j] += mix_buffer[j] * lvol;
            output[j + 1] += mix_buffer[j + 1] * rvol;
        }
    }
}
