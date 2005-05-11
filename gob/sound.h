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
#ifndef GOB_SOUND_H
#define GOB_SOUND_H

namespace Gob {

void snd_initSound(void);
void snd_loopSounds(void);
int16 snd_checkProAudio(void);
int16 snd_checkAdlib(void);
int16 snd_checkBlaster(void);
void snd_setBlasterPort(int16 port);
void snd_speakerOn(int16 frequency, int32 length);
void snd_speakerOff(void);
void snd_stopSound(int16 arg);
void snd_setResetTimerFlag(char flag);

extern int16 snd_soundPort;
extern char snd_playingSound;

typedef void (*CleanupFuncPtr) (int16);
extern CleanupFuncPtr snd_cleanupFunc;

void snd_writeAdlib(int16 port, int16 data);

typedef struct Snd_SoundDesc {
	Audio::SoundHandle handle;
	char *data;
	int32 size;
	int16 repCount;
	int16 timerTicks;
	int16 inClocks;
	int16 frequency;
	int16 flag;
} Snd_SoundDesc;

void snd_playSample(Snd_SoundDesc *sndDesc, int16 repCount, int16 frequency);
Snd_SoundDesc *snd_loadSoundData(const char *path);
void snd_freeSoundData(Snd_SoundDesc *sndDesc);
void snd_playComposition(Snd_SoundDesc **samples, int16 *composit, int16 freqVal);
void snd_waitEndPlay(void);

}				// End of namespace Gob

#endif
