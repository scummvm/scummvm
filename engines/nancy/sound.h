/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef NANCY_SOUND_H
#define NANCY_SOUND_H

#include "engines/nancy/commontypes.h"

#include "audio/mixer.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {
class SeekableAudioStream;
class QueuingAudioStream;
}

namespace Nancy {

class IFF;
class NancyConsole;
class NancyEngine;

class SoundManager {
	friend class NancyConsole;
public:
	// Settings for playing a sound, used in nancy3 and up
	// Older versions had a different, non-bitflag enum, but testing 
	// indicates those were never actually implemented
	enum PlayCommandFlags {
		kPlaySequential				= 0x0001, 		// Play normally
		kPlaySequentialPosition		= 0x0003, 		// Play at fixed position in 3D space
		kPlaySequentialFrameAnchor	= 0x0007,		// Position in 3D space is tied to a background frame, ignoring 3D coordinates

		kPlayRandomTime				= 0x0010,	// Play at random time intervals
		kPlayRandomPosition 		= 0x0020,	// Play at random 3D positions

		kPlayMoveLinear				= 0x0100,	// Move sound position in 3D space. The movement is linear unless kPlayMoveCircular is also set
		kPlayMoveCircular			= 0x0300,	// Move sound position in a circular direction (see SoundRotationAxis)
		kPlayRandomMove				= 0x0500	// Move along random vector. Does not combine with kPlayMoveCircular
	};

	enum SoundRotationAxis {
		kRotateAroundX = 0,
		kRotateAroundY = 1,
		kRotateAroundZ = 2
	};
	
	SoundManager();
	~SoundManager();

	void loadCommonSounds(IFF *boot);
	void initSoundChannels();

	// Load a sound into a channel without starting it
	void loadSound(const SoundDescription &description, SoundEffectDescription **effectData = nullptr);

	void playSound(uint16 channelID);
	void playSound(const SoundDescription &description);
	void playSound(const Common::String &chunkName);

	void pauseSound(uint16 channelID, bool pause);
	void pauseSound(const SoundDescription &description, bool pause);
	void pauseSound(const Common::String &chunkName, bool pause);
	void pauseAllSounds(bool pause);

	bool isSoundPlaying(uint16 channelID) const;
	bool isSoundPlaying(const SoundDescription &description) const;
	bool isSoundPlaying(const Common::String &chunkName) const;

	void stopSound(uint16 channelID);
	void stopSound(const SoundDescription &description);
	void stopSound(const Common::String &chunkName);
	void stopAllSounds();

	byte getVolume(uint16 channelID);
	byte getVolume(const SoundDescription &description);
	byte getVolume(const Common::String &chunkName);

	void setVolume(uint16 channelID, uint16 volume);
	void setVolume(const SoundDescription &description, uint16 volume);
	void setVolume(const Common::String &chunkName, uint16 volume);

	uint32 getRate(uint16 channelID);
	uint32 getRate(const SoundDescription &description);
	uint32 getRate(const Common::String &chunkName);

	uint32 getBaseRate(uint16 channelID);
	uint32 getBaseRate(const SoundDescription &description);
	uint32 getBaseRate(const Common::String &chunkName);

	void setRate(uint16 channelID, uint32 rate);
	void setRate(const SoundDescription &description, uint32 rate);
	void setRate(const Common::String &chunkName, uint32 rate);

	void soundEffectMaintenance();
	void recalculateSoundEffects();

	// Used when changing scenes
	void stopAndUnloadSpecificSounds();

	static Audio::SeekableAudioStream *makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 overrideSamplesPerSec = 0);

protected:
	struct Channel {
		~Channel();
		Common::String name;
		Audio::Mixer::SoundType type = Audio::Mixer::SoundType::kPlainSoundType;
		uint16 playCommands = 1;
		uint16 numLoops = 0;
		uint volume = 0;
		uint16 panAnchorFrame = 0;
		bool isPanning = false;
		Audio::SeekableAudioStream *stream = nullptr;
		Audio::SoundHandle handle;
		bool isPersistent = false;

		// Sound effect data, not applicable to nancy2 and below
		SoundEffectDescription *effectData = nullptr;
		Math::Vector3d position;
		Math::Vector3d positionDelta;
		uint32 nextStepTime = 0;
		uint16 stepsLeft = 0;
		uint32 nextRepeatTime = 0;
	};

	void soundEffectMaintenance(uint16 channelID);

	Audio::Mixer *_mixer;

	Common::Array<Channel> _channels;
	Common::HashMap<Common::String, SoundDescription> _commonSounds;

	bool _shouldRecalculate;
	Math::Vector3d _orientation;
};

} // End of namespace Nancy

#endif // NANCY_SOUND_H
