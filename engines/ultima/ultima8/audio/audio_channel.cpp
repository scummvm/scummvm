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

// We divide the data by 2, to prevent overshots. Imagine this _sample pattern:
// 0, 65535, 65535, 0. Now you want to compute a value between the two 65535.
// Obviously, it will be *bigger* than 65535 (it can get to about 80,000).
// It is possibly to clamp it, but that leads to a distored wave form. Compare
// this to turning up the volume of your stereo to much, it will start to sound
// bad at a certain level (depending on the power of your stereo, your speakers
// etc, this can be quite loud, though ;-). Hence we reduce the original range.
// A factor of roughly 1/1.2 = 0.8333 is sufficient. Since we want to avoid
// floating point, we approximate that by 27/32
#define RANGE_REDUX(x)  (((x) * 27) >> 5)

AudioChannel::AudioChannel(Audio::Mixer *mixer, uint32 sampleRate, bool stereo) :
		_mixer(mixer), _decompressorSize(0), _frameSize(0), _loop(0), _sample(nullptr),
		_frameEvenOdd(0), _paused(false), _priority(0) {
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

	Audio::AudioStream *stream = _loop <= 1 ? (Audio::AudioStream *)audioStream :
		new Audio::LoopingAudioStream(audioStream, _loop);

	// Play it
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, stream);
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

void AudioChannel::decompressNextFrame() {
	// Get next frame of data
	uint8 *playData = &_playData[0];
	uint8 *src2 = playData + _decompressorSize + (_frameSize * (1 - _frameEvenOdd));
	(void)_sample->decompressFrame(playData, src2);
}

} // End of namespace Ultima8
} // End of namespace Ultima
