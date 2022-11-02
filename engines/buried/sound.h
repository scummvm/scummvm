/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_SOUND_H
#define BURIED_SOUND_H

#include "audio/mixer.h"
#include "common/str.h"

namespace Audio {
class RewindableAudioStream;
class SoundHandle;
}

namespace Buried {

class BuriedEngine;

class SoundManager {
public:
	SoundManager(BuriedEngine *vm);
	~SoundManager();

	// STARTUP/SHUTDOWN FUNCTIONS
	bool startup();
	void shutDown();

	// AMBIENT SOUND CHANNEL FUNCTIONS
	bool setAmbientSound(const Common::String &fileName = "", bool fade = false, byte finalVolumeLevel = 64);
	bool adjustAmbientSoundVolume(byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength);
	bool isAmbientSoundPlaying();

	bool setSecondaryAmbientSound(const Common::String &fileName = "", bool fade = false, byte finalVolumeLevel = 64);
	bool adjustSecondaryAmbientSoundVolume(byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength);
	uint32 getSecondaryAmbientPosition();
	bool restartSecondaryAmbientSound();

	// AI SOUND CHANNEL FUNCTIONS
	bool playSynchronousAIComment(const Common::String &fileName);
	bool playAsynchronousAIComment(const Common::String &fileName);
	bool isAsynchronousAICommentPlaying();
	void stopAsynchronousAIComment();

	// SOUND EFFECTS FUNCTIONS
	int playSoundEffect(const Common::String &fileName, int volume = 127, bool loop = false, bool oneShot = true);
	bool playSynchronousSoundEffect(const Common::String &fileName, int volume = 127);
	bool stopSoundEffect(int effectID);
	bool isSoundEffectPlaying (int effectID);
	bool adjustSoundEffectSoundVolume(int effectID, byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength);

	// Interface sound functions
	bool playInterfaceSound(const Common::String &fileName);
	bool stopInterfaceSound();
	bool isInterfaceSoundPlaying();

	// START AND STOP SPECIFIED FOOTSTEPS SOUND
	bool startFootsteps(int footstepsID);
	bool stopFootsteps();

	// Pause functions
	// stop()/restart() do as they say on the tin and aren't true pause functions.
	// This is what the original does for pausing, and it needs to be done this way.
	// pause() is used for implementing pauseEngineIntern(). Since stop()/restart()
	// are not re-entrant, they're not suitable for that purpose.
	bool stop();
	void stopSound(int soundId);
	bool restart();
	void pause(bool shouldPause);

	// TIMER CALLBACK FUNCTION
	void timerCallback();

private:
	enum {
		kAmbientIndexBase = 0,
		kAmbientIndexA = 0,
		kAmbientIndexB = 1,

		kEffectsIndexBase = 2,
		kEffectsIndexA = 2,
		kEffectsIndexB = 3,

		kInterfaceIndex = 4,
		kAIVoiceIndex = 5,
		kFootstepsIndex = 6,

		kMaxSounds = 7
	};

	class Sound {
	friend class SoundManager;

	public:
		Sound();
		~Sound();

		bool load(const Common::String &fileName);
		bool start();
		bool isPlaying() const;
		bool stop();
		void pause(bool shouldPause);

	protected:
		Audio::RewindableAudioStream *_soundData; // Stream to the data
		Audio::SoundHandle *_handle;              // Handle

		int32 _volume;                            // Volume of this sample
		bool _loop;                               // Is this sample looping?
		byte _flags;                              // Sound flags
		byte _timedEffectIndex;                   // Timed effect index
		int _timedEffectSteps;                    // Number of steps remaining in timed effect
		int32 _timedEffectDelta;                  // Amount to change target value in each step
		uint32 _timedEffectStart;                 // The last starting time for the timed effect
		uint32 _timedEffectRemaining;             // The remaining amount of time for the effect

		bool _wasPlaying;
		Audio::Mixer::SoundType _soundType;
	};

	BuriedEngine *_vm;

	Sound *_soundData[kMaxSounds];

	bool _paused;

	int _fileIDFootsteps;
	Common::String _ambientFileNames[2];
	int _lastAmbient;
	Common::String _effectsFileNames[2];
	Common::String _interfaceFileName;
	Common::String _arthurFileName;
};

} // End of namespace Buried

#endif
