/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/debug.h"
#include "gob/sound.h"
namespace Gob {
	int16 snd_checkProAudio(void) {return 0;}
	int16 snd_checkAdlib(void) {return 0;}
	int16 snd_checkBlaster(void) {return 0;}
	void snd_setBlasterPort(int16 port) {return;}
	void snd_speakerOn(int16 frequency) {return;}
	void snd_speakerOff(void) {return;}
	void snd_stopSound(int16 arg){return;}
	void snd_setResetTimerFlag(char flag){return;}

	void snd_playSample(Snd_SoundDesc * soundDesc, int16 repCount, int16 frequency) {;}
	void snd_cleanupFuncCallback() {;}
	CleanupFuncPtr (snd_cleanupFunc);
	//CleanupFuncPtr snd_cleanupFunc;// = &snd_cleanupFuncCallback();

	int16 snd_soundPort;
	char snd_playingSound;

	void snd_writeAdlib(int16 port, int16 data) {
		return;
	}

	Snd_SoundDesc *snd_loadSoundData(const char *path) {
		return NULL;
	}
void snd_freeSoundData(Snd_SoundDesc * sndDesc) {;}
void snd_playComposition(Snd_SoundDesc ** samples, int16 *composit, int16 freqVal) {;}
void snd_waitEndPlay(void) {;}

}                               // End of namespace Gob



