/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/sound.h"
namespace Gob {

SoundHandle soundHandle;

int16 snd_checkProAudio(void) {return 0;}
int16 snd_checkAdlib(void) {return 0;}
int16 snd_checkBlaster(void) {return 0;}
void snd_setBlasterPort(int16 port) {return;}
void snd_speakerOn(int16 frequency) {return;}
void snd_speakerOff(void) {return;}
void snd_stopSound(int16 arg){return;}
void snd_setResetTimerFlag(char flag){return;}

// This is mostly to see if we are interpreting the sound effects correctly. If
// we want to implement looping etc. we're probably going to need something a
// bit more elaborate.
//
// Currently we ignore looping completely. We also ignore samples with negative
// sample frequency. Something wacky is going on here, and I don't think it's
// a simple signed/unsigned issue.

void snd_playSample(Snd_SoundDesc *sndDesc, int16 repCount, int16 frequency) {
	if (repCount != 1)
		warning("snd_playSample: repCount = %d - not implemented", repCount);
	if (frequency < 0) {
		warning("snd_playSample: frequency = %d - this is weird", frequency);
		return;
	}

	_vm->_mixer->playRaw(&soundHandle, sndDesc->data, sndDesc->size, frequency, 0);
}

void snd_cleanupFuncCallback() {;}
CleanupFuncPtr (snd_cleanupFunc);
//CleanupFuncPtr snd_cleanupFunc;// = &snd_cleanupFuncCallback();

int16 snd_soundPort;
char snd_playingSound;

void snd_writeAdlib(int16 port, int16 data) {
	return;
}

Snd_SoundDesc *snd_loadSoundData(const char *path) {
	Snd_SoundDesc *sndDesc;
	int32 size;

	size = data_getDataSize(path);
	sndDesc = (Snd_SoundDesc *)malloc(size);
	sndDesc->size = size;
	sndDesc->data = data_getData(path);

	return sndDesc;
}

void snd_freeSoundData(Snd_SoundDesc *sndDesc) {
	free(sndDesc->data);
	free(sndDesc);
}

void snd_playComposition(Snd_SoundDesc ** samples, int16 *composit, int16 freqVal) {;}
void snd_waitEndPlay(void) {;}

}                               // End of namespace Gob



