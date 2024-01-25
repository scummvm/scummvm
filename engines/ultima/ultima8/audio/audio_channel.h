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

#ifndef ULTIMA8_AUDIO_AUDIOCHANNEL_H
#define ULTIMA8_AUDIO_AUDIOCHANNEL_H

#include "audio/mixer.h"

namespace Ultima {
namespace Ultima8 {

class AudioSample;

class AudioChannel {
private:
	Audio::SoundHandle _soundHandle;
	Audio::Mixer *_mixer;
	int _priority;

public:
	AudioChannel(Audio::Mixer *mixer, uint32 sampleRate, bool stereo);
	~AudioChannel(void);

	void stop();

	void playSample(AudioSample *sample, int loop, int priority,
		bool isSpeech, uint32 pitchShift, byte volume, int8 balance);

	bool isPlaying();

	void setVolume(byte volume, int8 balance) {
		_mixer->setChannelVolume(_soundHandle, volume);
		_mixer->setChannelBalance(_soundHandle, balance);
	}

	void setPriority(int priority) {
		_priority = priority;
	}
	int getPriority() const {
		return _priority;
	}

	void setPaused(bool paused);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
