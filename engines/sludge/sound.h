/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SLUDGE_SOUND_H
#define SLUDGE_SOUND_H

#define HWND void *

#include "common/file.h"

#include "sludge/variable.h"

namespace Sludge {

// Sound list stuff
struct soundList {
	int sound;
	struct soundList *next;
	struct soundList *prev;
	int cacheIndex;
	int vol;
};
bool deleteSoundFromList(soundList *&s);
void playSoundList(soundList *s);
void handleSoundLists(); // to produce the same effects as end of stream call back functions

// GENERAL...
bool initSoundStuff(HWND);
void killSoundStuff();

// MUSIC...
bool playMOD(int, int, int);
void stopMOD(int);
void setMusicVolume(int a, int v);
void setDefaultMusicVolume(int v);

// SAMPLES...
int cacheSound(int f);
bool startSound(int, bool = false);
void huntKillSound(int a);
void huntKillFreeSound(int filenum);
void setSoundVolume(int a, int v);
void setDefaultSoundVolume(int v);
void setSoundLoop(int a, int s, int e);
bool stillPlayingSound(int ch);
bool getSoundCacheStack(stackHandler *sH);
int findInSoundCache(int a);

void debugSounds();
void loadSounds(Common::SeekableReadStream *stream);
void saveSounds(Common::WriteStream *stream);

uint getSoundSource(int index);

} // End of namespace Sludge

#endif
