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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BLADERUNNER_AUDIO_PLAYER_H
#define BLADERUNNER_AUDIO_PLAYER_H

#include "common/array.h"
#include "common/mutex.h"
#include "common/str.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "bladerunner/bladerunner.h" // For BLADERUNNER_ORIGINAL_BUGS and BLADERUNNER_ORIGINAL_SETTINGS symbols

namespace BladeRunner {

class BladeRunnerEngine;
class AudioCache;
class AudStream;

enum AudioPlayerFlags {
	kAudioPlayerLoop = 1,
	kAudioPlayerOverrideVolume = 2
};

class AudioPlayer {
#if BLADERUNNER_ORIGINAL_BUGS
	static const int kTracks = 6;
#else
	// increase tracks, reduce probability of tracks being skipped
	static const int kTracks = 12;
#endif // BLADERUNNER_ORIGINAL_BUGS
	// Use SFX sound type if none is specified
	static const Audio::Mixer::SoundType kAudioPlayerSoundType = Audio::Mixer::kSFXSoundType;

	struct Track {
		bool                isActive;
		int                 channel;
		int                 priority;
		int                 volume;   // should be in [0, 100]
		int                 pan;      // should be in [-100, 100]
		AudStream          *stream;
	};

	BladeRunnerEngine *_vm;

	Common::Mutex _mutex;
	Track         _tracks[kTracks];
	int           _sfxVolumeFactorOriginalEngine; // should be in [0, 100] - Unused in ScummVM Engine, used in original engine

public:
	AudioPlayer(BladeRunnerEngine *vm);
	~AudioPlayer();

	int playAud(const Common::String &name, int volume, int panStart, int panEnd, int priority, byte flags = 0, Audio::Mixer::SoundType type = kAudioPlayerSoundType);
	bool isActive(int track) const;
	uint32 getLength(int track) const;
	void stop(int track, bool immediately);
	void stopAll();
	void adjustVolume(int track, int volume, uint32 delaySeconds, bool explicitVolumeAdjustment);
	void adjustPan(int track, int pan, uint32 delaySeconds);

#if BLADERUNNER_ORIGINAL_SETTINGS
	void setVolume(int volume);
	int getVolume() const;
#endif // BLADERUNNER_ORIGINAL_SETTINGS
	void playSample();

private:
	void remove(int channel);
	static void mixerChannelEnded(int channel, void *data);
};

} // End of namespace BladeRunner

#endif
