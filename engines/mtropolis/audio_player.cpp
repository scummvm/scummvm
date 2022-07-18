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

#include "mtropolis/audio_player.h"
#include "mtropolis/assets.h"

namespace MTropolis {

AudioPlayer::AudioPlayer(Audio::Mixer *mixer, byte volume, int8 balance, const Common::SharedPtr<AudioMetadata> &metadata, const Common::SharedPtr<CachedAudio> &audio, bool isLooping, size_t currentPos, size_t startPos, size_t endPos)
	: _metadata(metadata), _audio(audio), _isLooping(isLooping), _currentPos(currentPos), _startPos(startPos), _endPos(endPos), _exhausted(false), _mixer(nullptr) {
	if (_startPos >= _endPos) {
		// ???
		_exhausted = true;
		_isLooping = false;
	}
	if (_currentPos < _startPos)
		_currentPos = _startPos;

	if (!_exhausted) {
		_mixer = mixer;
		mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
	}
}

AudioPlayer::~AudioPlayer() {
	stop();
}

int AudioPlayer::readBuffer(int16 *buffer, const int numSamplesTimesChannelCount) {
	Common::StackLock lock(_mutex);

	int samplesRead = 0;
	if (_exhausted)
		return 0;

	uint8 numChannels = _metadata->channels;

	size_t numSamples = numSamplesTimesChannelCount / numChannels;

	while (numSamples > 0) {
		size_t samplesAvailable = _endPos - _currentPos;
		if (samplesAvailable == 0) {
			if (_isLooping) {
				_currentPos = _startPos;
				continue;
			} else {
				_exhausted = true;
				break;
			}
		}

		size_t numSamplesThisIteration = numSamples;
		if (numSamplesThisIteration > samplesAvailable)
			numSamplesThisIteration = samplesAvailable;

		size_t numSampleValues = numSamplesThisIteration * numChannels;
		// TODO: Support more formats
		if (_metadata->bitsPerSample == 8 && _metadata->encoding == AudioMetadata::kEncodingUncompressed) {
			const uint8 *inSamples = static_cast<const uint8 *>(_audio->getData()) + _currentPos * numChannels;
			for (size_t i = 0; i < numSampleValues; i++)
				buffer[i] = (inSamples[i] - 0x80) * 256;
		} else if (_metadata->bitsPerSample == 16 && _metadata->encoding == AudioMetadata::kEncodingUncompressed) {
			const int16 *inSamples = static_cast<const int16 *>(_audio->getData()) + _currentPos * numChannels;
			memcpy(buffer, inSamples, sizeof(int16) * numSampleValues);
		}

		buffer += numSampleValues;
		numSamples -= numSamplesThisIteration;

		samplesRead += numSamplesThisIteration * numChannels;
		_currentPos += numSamplesThisIteration;
	}

	return samplesRead;
}

bool AudioPlayer::isStereo() const {
	return _metadata->channels == 2;
}

int AudioPlayer::getRate() const {
	return _metadata->sampleRate;
}

bool AudioPlayer::endOfData() const {
	return _exhausted;
}

void AudioPlayer::sendToMixer(Audio::Mixer *mixer, byte volume, int8 balance) {
	mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
}

void AudioPlayer::stop() {
	if (_mixer)
		_mixer->stopHandle(_handle);

	_exhausted = true;
	_mixer = nullptr;
}

} // End of namespace MTropolis
