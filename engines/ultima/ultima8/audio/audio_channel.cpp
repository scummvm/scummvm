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
		_mixer(mixer), _loop(0), _sample(nullptr),
		_paused(false), _priority(0), _lVol(0), _rVol(0), _pitchShift(0) {
}

AudioChannel::~AudioChannel(void) {
}

void AudioChannel::playSample(AudioSample *sample, int loop, int priority, bool paused, bool isSpeech, uint32 pitchShift, int lvol, int rvol) {
	_sample = sample;
	_loop = loop;
	_priority = priority;
	_lVol = lvol;
	_rVol = rvol;
	_paused = paused;
	_pitchShift = pitchShift;

	if (!_sample)
		return;

	// Create the _sample
	Audio::SeekableAudioStream *audioStream = _sample->makeStream();

	int loops = _loop;
	if (loops == -1) {
		// loop forever
		loops = 0;
	}
	Audio::AudioStream *stream = (_loop <= 1 && _loop != -1) ?
		(Audio::AudioStream *)audioStream :
		new Audio::LoopingAudioStream(audioStream, loops);

	// Play it
	int vol = (_lVol + _rVol) / 2;		 // range is 0 ~ 255
	int balance = (_rVol - _lVol) / 2; // range is -127 ~ +127

	_mixer->stopHandle(_soundHandle);
	_mixer->playStream(isSpeech ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, vol, balance);
	if (_pitchShift != AudioProcess::PITCH_SHIFT_NONE)
		_mixer->setChannelRate(_soundHandle, stream->getRate() * pitchShift / AudioProcess::PITCH_SHIFT_NONE);
	if (paused)
		_mixer->pauseHandle(_soundHandle, true);
}

bool AudioChannel::isPlaying() {
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_sample = nullptr;

	return _sample != nullptr;
}

void AudioChannel::stop() {
	_mixer->stopHandle(_soundHandle);
	_sample = nullptr;
}

void AudioChannel::setPaused(bool paused) {
	_paused = paused;
	_mixer->pauseHandle(_soundHandle, paused);
}

} // End of namespace Ultima8
} // End of namespace Ultima
