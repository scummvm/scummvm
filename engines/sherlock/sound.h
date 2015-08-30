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

#define MAX_MIXER_CHANNELS 10

class Sound {
private:
	SherlockEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _scalpelEffectsHandle;
	Audio::SoundHandle _tattooEffectsHandle[MAX_MIXER_CHANNELS];
	Audio::SoundHandle _speechHandle;
	int _curPriority;

	/**
	 * Decode a sound sample
	 */
	byte decodeSample(byte sample, byte& reference, int16& scale);

	/**
	 * Handle playing a sound or speech
	 */
	bool playSoundResource(const Common::String &name, const Common::String &libFilename,
		Audio::Mixer::SoundType soundType, Audio::SoundHandle &handle);
public:
	bool _digitized;
	int _voices;
	bool _soundOn;
	bool _speechOn;
	bool _soundPlaying;
	bool _speechPlaying;
	int _soundVolume;

	Common::String _talkSoundFile;
public:
	Sound(SherlockEngine *vm, Audio::Mixer *mixer);

	/**
	 * Saves sound-related settings
	 */
	void syncSoundSettings();

	/**
	 * Load a sound
	 */
	void loadSound(const Common::String &name, int priority);
	
	/**
	 * Play the sound in the specified resource
	 */
	bool playSound(const Common::String &name, WaitType waitType, int priority = 100, const char *libraryFilename = nullptr);
	
	/**
	 * Play a previously loaded sound
	 */
	void playLoadedSound(int bufNum, WaitType waitType);
	
	/**
	 * Free any previously loaded sounds
	 */
	void freeLoadedSounds();
	
	/**
	 * Stop playing any active sound
	 */
	void stopSound();

	void freeDigiSound();

	Audio::SoundHandle getFreeSoundHandle();

	void setVolume(int volume);

	/**
	 * Play a specified voice resource
	 */
	void playSpeech(const Common::String &name);

	/**
	 * Stop any currently playing speech
	 */
	void stopSpeech();

	/**
	 * Returns true if speech is currently playing
	 */
	bool isSpeechPlaying();
};

} // End of namespace Sherlock

#endif

