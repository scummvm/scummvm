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

#ifndef SHERLOCK_SOUND_H
#define SHERLOCK_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "access/files.h"
#include "audio/midiplayer.h"
#include "audio/midiparser.h"

namespace Sherlock {

class SherlockEngine;

enum WaitType {
	WAIT_RETURN_IMMEDIATELY = 0, WAIT_FINISH = 1, WAIT_KBD_OR_FINISH = 2
};

class Sound {
private:
	SherlockEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _effectsHandle;
	int _curPriority;

	byte decodeSample(byte sample, byte& reference, int16& scale);
public:
	bool _digitized;
	bool _music;
	int _voices;
	bool _soundOn;
	bool _musicOn;
	bool _speechOn;
	bool _playingEpilogue;
	bool _diskSoundPlaying;
	bool _soundPlaying;
	bool *_soundIsOn;
	byte *_digiBuf;
public:
	Sound(SherlockEngine *vm, Audio::Mixer *mixer);

	void syncSoundSettings();
	void loadSound(const Common::String &name, int priority);
	bool playSound(const Common::String &name, WaitType waitType, int priority = 100);
	void playLoadedSound(int bufNum, WaitType waitType);
	void freeLoadedSounds();
	void stopSound();

	int loadSong(int songNumber);
	void startSong();
	void freeSong();
	
	void playMusic(const Common::String &name);
	void stopMusic();
	void stopSndFuncPtr(int v1, int v2);
	void waitTimerRoland(uint time);
	void freeDigiSound();
};

} // End of namespace Sherlock

#endif

