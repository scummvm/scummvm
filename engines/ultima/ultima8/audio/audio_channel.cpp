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

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/audio/audio_channel.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/audio_sample.h"
#include "common/memstream.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Ultima {
namespace Ultima8 {


AudioChannel::AudioChannel(Audio::Mixer *mixer, uint32 sampleRate, bool stereo) :
		_mixer(mixer), _priority(0) {
}

AudioChannel::~AudioChannel(void) {
}

void AudioChannel::playSample(AudioSample *sample, int loop, int priority, bool isSpeech, uint32 pitchShift, byte volume, int8 balance) {
	if (!sample)
		return;

	_priority = priority;

	// Create the _sample
	Audio::SeekableAudioStream *audioStream = sample->makeStream();

	int loops = loop;
	if (loop == -1) {
		// loop forever
		loops = 0;
	}
	Audio::AudioStream *stream = (loop <= 1 && loop != -1) ?
		(Audio::AudioStream *)audioStream :
		new Audio::LoopingAudioStream(audioStream, loops);

	_mixer->stopHandle(_soundHandle);
	_mixer->playStream(isSpeech ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, volume, balance);
	if (pitchShift != AudioProcess::PITCH_SHIFT_NONE)
		_mixer->setChannelRate(_soundHandle, stream->getRate() * pitchShift / AudioProcess::PITCH_SHIFT_NONE);
}

bool AudioChannel::isPlaying() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void AudioChannel::stop() {
	_mixer->stopHandle(_soundHandle);
}

void AudioChannel::setPaused(bool paused) {
	_mixer->pauseHandle(_soundHandle, paused);
}

} // End of namespace Ultima8
} // End of namespace Ultima
