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

#include "vcruise/audio_player.h"

namespace VCruise {

AudioPlayer::AudioPlayer(Audio::Mixer *mixer, const Common::SharedPtr<Audio::AudioStream> &baseStream, byte volume, int8 balance)
	: _exhausted(false), _mixer(nullptr), _baseStream(baseStream) {
	_mixer = mixer;
	mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
}

AudioPlayer::~AudioPlayer() {
	stop();
}

int AudioPlayer::readBuffer(int16 *buffer, const int numSamplesTimesChannelCount) {
	Common::StackLock lock(_mutex);

	int samplesRead = 0;
	if (_exhausted)
		return 0;

	samplesRead = _baseStream->readBuffer(buffer, numSamplesTimesChannelCount);

	if (samplesRead != numSamplesTimesChannelCount)
		_exhausted = true;

	return samplesRead;
}

bool AudioPlayer::isStereo() const {
	return _baseStream->isStereo();
}

int AudioPlayer::getRate() const {
	return _baseStream->getRate();
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

} // End of namespace VCruise
