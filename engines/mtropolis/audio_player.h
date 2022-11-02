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

#ifndef MTROPOLIS_AUDIO_PLAYER_H
#define MTROPOLIS_AUDIO_PLAYER_H

#include "common/mutex.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace MTropolis {

struct AudioMetadata;
class CachedAudio;

// Audio player, this does not support requeueing.  If the sound exhausts, then you must create a
// new audio player.  In particular, since time is being tracked separately, if the loop status
// changes when the timer thinks the sound should still be playing, but the sound has actually
// exhausted, then the sound needs to be requeued.
class AudioPlayer : public Audio::AudioStream {
public:
	AudioPlayer(Audio::Mixer *mixer, byte volume, int8 balance, const Common::SharedPtr<AudioMetadata> &metadata, const Common::SharedPtr<CachedAudio> &audio, bool isLooping, size_t currentPos, size_t startPos, size_t endPos);
	~AudioPlayer();

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;

	void sendToMixer(Audio::Mixer *mixer, byte volume, int8 balance);
	void stop();

private:
	Common::Mutex _mutex;

	Common::SharedPtr<AudioMetadata> _metadata;
	Common::SharedPtr<CachedAudio> _audio;
	Audio::SoundHandle _handle;
	bool _isLooping;
	bool _exhausted;
	size_t _currentPos;
	size_t _startPos;
	size_t _endPos;
	Audio::Mixer *_mixer;
};

} // End of namespace MTropolis

#endif
