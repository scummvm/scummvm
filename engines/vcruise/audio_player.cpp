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

AudioPlayer::AudioPlayer(Audio::Mixer *mixer, const Common::SharedPtr<Audio::AudioStream> &baseStream, Audio::Mixer::SoundType soundType)
	: _exhausted(false), _isPlaying(false), _mixer(mixer), _baseStream(baseStream), _soundType(soundType) {
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

void AudioPlayer::play(byte volume, int8 balance) {
	if (!_isPlaying) {
		_isPlaying = true;
		_exhausted = false;
		_mixer->playStream(_soundType, &_handle, this, -1, volume, balance, DisposeAfterUse::NO);
	}

}

void AudioPlayer::setVolume(byte volume) {
	_mixer->setChannelVolume(_handle, volume);
}

void AudioPlayer::setBalance(int8 balance) {
	_mixer->setChannelBalance(_handle, balance);
}

void AudioPlayer::setVolumeAndBalance(byte volume, int8 balance) {
	Common::StackLock lock(_mixer->mutex());

	_mixer->setChannelVolume(_handle, volume);
	_mixer->setChannelBalance(_handle, balance);
}

void AudioPlayer::stop() {
	if (_isPlaying) {
		_mixer->stopHandle(_handle);
		_isPlaying = false;
	}

	_exhausted = true;
}

} // End of namespace VCruise
