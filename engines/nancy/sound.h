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
}

namespace Nancy {

class IFF;

class NancyEngine;

class SoundManager {
public:
	// Settings for playing a sound, used in nancy3 and up
	// Older versions had a different, non-bitflag enum, but testing 
	// indicates those were never actually implemented
	enum PlayCommandFlags {
		kPlaySequential		= 1 << 0, // Same as kPlaySequentialAndDie
		kPlayPosition		= 1 << 1, // Play at fixed position in 3D space
		kPlayFrameAnchor	= 1 << 2, // Position in 3D space is tied to a background frame, ignoring 3D coordinates

		kPlayRandomTime		= 1 << 4, // Play at random time intervals
		kPlayRandomPosition = 1 << 5, // Play at random 3D positions

		kPlayMoveLinear		= 1 << 8, // Move sound position in 3D space. The movement is linear unless kPlayMoveCircular is also set
		kPlayMoveCircular	= 1 << 9, // Move sound position in a circular direction (see SoundRotationAxis)
		kPlayRandomMove		= 1 << 10 // Move along random vector. Does not combine with kPlayMoveCircular
	};

	enum SoundRotationAxis {
		kRotateAroundX = 0,
		kRotateAroundY = 1,
		kRotateAroundZ = 2
	};
	
	SoundManager();
	~SoundManager();

	void loadCommonSounds(IFF *boot);

	// Load a sound into a channel without starting it
	void loadSound(const SoundDescription &description, bool panning = false);

	void playSound(uint16 channelID);
	void playSound(const SoundDescription &description);
	void playSound(const Common::String &chunkName);

	void pauseSound(uint16 channelID, bool pause);
	void pauseSound(const SoundDescription &description, bool pause);
	void pauseSound(const Common::String &chunkName, bool pause);

	bool isSoundPlaying(uint16 channelID) const;
	bool isSoundPlaying(const SoundDescription &description) const;
	bool isSoundPlaying(const Common::String &chunkName) const;

	void stopSound(uint16 channelID);
	void stopSound(const SoundDescription &description);
	void stopSound(const Common::String &chunkName);
	void stopAllSounds();

	void calculatePan(uint16 channelID);
	void calculatePan(const SoundDescription &description);
	void calculatePanForAllSounds();

	void setVolume(uint16 channelID, uint16 volume);
	void setVolume(const SoundDescription &description, uint16 volume);
	void setVolume(const Common::String &chunkName, uint16 volume);

	void setRate(uint16 channelID, uint32 rate);
	void setRate(const SoundDescription &description, uint32 rate);
	void setRate(const Common::String &chunkName, uint32 rate);

	// Used when changing scenes
	void stopAndUnloadSpecificSounds();

	static Audio::SeekableAudioStream *makeHISStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 overrideSamplesPerSec = 0);

protected:
	struct Channel {
		Common::String name;
		Audio::Mixer::SoundType type;
		uint16 playCommands = 1;
		uint16 numLoops = 0;
		uint volume = 0;
		uint16 panAnchorFrame = 0;
		bool isPanning = false;
		Audio::SeekableAudioStream *stream = nullptr;
		Audio::SoundHandle handle;
		bool isPersistent = false;
	};

	void initSoundChannels();
	Audio::Mixer *_mixer;

	Channel _channels[32];
	Common::HashMap<Common::String, SoundDescription> _commonSounds;
};

} // End of namespace Nancy

#endif // NANCY_SOUND_H
