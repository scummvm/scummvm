/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/audio/audio_channel.h"
#include "ultima/ultima8/audio/audio_sample.h"
#include "common/memstream.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Ultima {
namespace Ultima8 {


AudioChannel::AudioChannel(Audio::Mixer *mixer, uint32 sampleRate, bool stereo) :
		_mixer(mixer), _decompressorSize(0), _frameSize(0), _loop(0), _sample(nullptr),
		_frameEvenOdd(0), _paused(false), _priority(0), _lVol(0), _rVol(0), _pitchShift(0) {
}

AudioChannel::~AudioChannel(void) {
}

void AudioChannel::playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitchShift, int lvol, int rvol) {
	_sample = sample;
	_loop = loop;
	_priority = priority;
	_lVol = lvol;
	_rVol = rvol;
	_paused = paused;
	_pitchShift = pitchShift;

	if (!_sample)
		return;

	// Setup buffers
	_decompressorSize = _sample->getDecompressorDataSize();
	_frameSize = _sample->getFrameSize();

	if ((_decompressorSize + _frameSize * 2) > _playData.size()) {
		_playData.resize(_decompressorSize + _frameSize * 2);
	}

	// Init the _sample decompressor
	_sample->initDecompressor(&_playData[0]);

	// Reset counter and stuff
	_frameEvenOdd = 0;

	// Get the data for the _sample
	Common::MemoryWriteStreamDynamic streamData(DisposeAfterUse::NO);
	int frameSize;
	byte *framePtr = &_playData[_decompressorSize];

	while ((frameSize = _sample->decompressFrame(&_playData[0], framePtr)) != 0)
		streamData.write(framePtr, frameSize);

	// Create the _sample
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(
		new Common::MemoryReadStream(streamData.getData(), streamData.size(), DisposeAfterUse::YES),
		_sample->getRate(),
		_sample->isStereo() ? Audio::FLAG_STEREO | Audio::FLAG_UNSIGNED : Audio::FLAG_UNSIGNED,
		DisposeAfterUse::YES
	);

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
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, stream, -1, vol, balance);
	if (paused)
		_mixer->pauseHandle(_soundHandle, true);
}

void AudioChannel::playMusicStream(Audio::AudioStream *stream) {
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream);
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

void AudioChannel::decompressNextFrame() {
	// Get next frame of data
	uint8 *playData = &_playData[0];
	uint8 *src2 = playData + _decompressorSize + (_frameSize * (1 - _frameEvenOdd));
	(void)_sample->decompressFrame(playData, src2);
}

} // End of namespace Ultima8
} // End of namespace Ultima
