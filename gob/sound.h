/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __SOUND_H
#define __SOUND_H

namespace Gob {

int16 snd_checkProAudio(void);
int16 snd_checkAdlib(void);
int16 snd_checkBlaster(void);
void snd_setBlasterPort(int16 port);
void snd_speakerOn(int16 frequency);
void snd_speakerOff(void);
void snd_stopSound(int16 arg);
void snd_setResetTimerFlag(char flag);

extern int16 snd_soundPort;
extern char snd_playingSound;

typedef void (*CleanupFuncPtr) (int16);
extern CleanupFuncPtr snd_cleanupFunc;

void snd_writeAdlib(int16 port, int16 data);

typedef struct Snd_SoundDesc {
	char *data;
	int32 size;
	int16 timerTicks;
	int16 inClocks;
	int16 frequency;
	int16 flag;
} Snd_SoundDesc;

void snd_playSample(Snd_SoundDesc * soundDesc, int16 repCount, int16 frequency);
Snd_SoundDesc *snd_loadSoundData(const char *path);
void snd_freeSoundData(Snd_SoundDesc * sndDesc);
void snd_playComposition(Snd_SoundDesc ** samples, int16 *composit, int16 freqVal);
void snd_waitEndPlay(void);

}				// End of namespace Gob

#endif
