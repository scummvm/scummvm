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

#ifndef HOLLYWOOD_MUSIC_H
#define HOLLYWOOD_MUSIC_H

#include "audio/mixer.h"
#include "common/path.h"

namespace Hollywood {

class MusicPlayer {
public:
	MusicPlayer();
	~MusicPlayer();

	void setArchive(const Common::Path &archiveName);
	const Common::Path &archiveName() const { return _archiveName; }
	bool playIntroMusic();
	bool playMusicCue(uint16 cueId, byte volumePercent = 100, bool loop = false);
	bool resumeLastCue();
	void setVolume(byte volumePercent);
	void stop();
	bool isPlaying() const;

private:
	bool readCueSpan(const Common::Path &fileName, uint16 cueId, uint32 &start, uint32 &size) const;

	Audio::SoundHandle _musicHandle;
	Common::Path _archiveName;
	uint16 _currentCueId;
	byte _currentVolumePercent;
	bool _currentLoop;
	bool _hasCurrentCue;
};

class SpeechPlayer {
public:
	SpeechPlayer();
	~SpeechPlayer();

	bool playSample(uint16 sampleId, byte volumePercent = 100, bool loop = false);
	void setVolume(byte volumePercent);
	void stop();
	bool isPlaying() const;
	uint32 lastSampleDurationMillis() const { return _lastSampleDurationMillis; }

private:
	bool readSampleSpan(uint16 sampleId, uint32 &start, uint32 &size) const;

	Audio::SoundHandle _speechHandle;
	uint32 _lastSampleDurationMillis;
};

class SoundBank0Player {
public:
	SoundBank0Player();
	~SoundBank0Player();

	void setArchive(const Common::Path &archiveName);
	const Common::Path &archiveName() const { return _archiveName; }
	bool playSample(uint16 sampleId, byte volumePercent = 100, bool loop = false);
	bool playSampleLooping(uint16 sampleId, byte volumePercent = 100);
	void setVolume(byte volumePercent);
	void stop();
	bool isPlaying() const;

private:
	bool readSampleSpan(uint16 sampleId, uint32 &start, uint32 &size) const;

	Audio::SoundHandle _soundHandle;
	Common::Path _archiveName;
};

// Plays the small one-based PCM effects embedded in RESOURCE.000, separately
// from the streamed RESOURCE.Sxx cue banks.
class ResidentSoundEffectPlayer {
public:
	ResidentSoundEffectPlayer();
	~ResidentSoundEffectPlayer();

	bool playSample(byte soundEffectId, byte volumePercent = 100);
	void stop();

private:
	enum {
		kSoundEffectCount = 16
	};

	bool readSampleSpan(byte soundEffectId, uint32 &start, uint32 &size) const;
	void stopSample(byte soundEffectId);

	Audio::SoundHandle _soundHandles[kSoundEffectCount];
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_MUSIC_H
