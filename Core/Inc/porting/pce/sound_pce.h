#ifndef _INCLUDE_SOUND_H
#define _INCLUDE_SOUND_H

int  pce_snd_init(void);
void pce_snd_term(void);
void pce_snd_update(short *buffer, unsigned length);

#endif
