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

#ifndef VCRUISE_AUDIO_PLAYER_H
#define VCRUISE_AUDIO_PLAYER_H

#include "common/mutex.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace VCruise {

struct AudioMetadata;
class CachedAudio;

class AudioPlayer : public Audio::AudioStream {
public:
	AudioPlayer(Audio::Mixer *mixer, const Common::SharedPtr<Audio::AudioStream> &baseStream);
	~AudioPlayer();

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;

	void play(byte volume, int8 balance);
	void stop();

private:
	Common::Mutex _mutex;

	Audio::SoundHandle _handle;
	bool _isLooping;
	bool _isPlaying;
	bool _exhausted;
	Audio::Mixer *_mixer;
	Common::SharedPtr<Audio::AudioStream> _baseStream;
};

} // End of namespace VCruise

#endif
