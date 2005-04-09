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



