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

#include "common/util.h"
#include "hollywood/sound.h"

namespace Hollywood {

SoundChannel::~SoundChannel() {
	stop();
}

void SoundChannel::play(Audio::Mixer *mixer, Audio::Mixer::SoundType type, Audio::AudioStream *stream,
		byte volume, const Common::Point *position, int listenerX, float rolloff, PositionMode mode) {
	assert(mixer && stream && rolloff >= 0);
	stop();
	_mixer = mixer;
	_volume = volume;
	_positional = position != nullptr;
	_position = position ? *position : Common::Point();
	_listenerX = listenerX;
	_rolloff = rolloff;
	_positionMode = mode;
	int8 balance;
	mixerLevels(volume, balance);
	_mixer->playStream(type, &_handle, stream, -1, volume, balance, DisposeAfterUse::YES);
}

void SoundChannel::stop() {
	if (_mixer) {
		_mixer->stopHandle(_handle);
		_mixer = nullptr;
	}
}

bool SoundChannel::isPlaying() const {
	return _mixer && _mixer->isSoundHandleActive(_handle);
}

void SoundChannel::setVolume(byte volume) {
	_volume = volume;
	updateVolume();
}

void SoundChannel::setListenerPosition(int x) {
	if (!_positional || _listenerX == x) {
		return;
	}
	_listenerX = x;
	updateVolume();
}

void SoundChannel::mixerLevels(byte &volume, int8 &balance) const {
	volume = _volume;
	balance = 0;
	if (!_positional) {
		return;
	}
	const float x = _position.x - _listenerX;
	const float z = _position.y;
	float gain = 1.0f;
	if (_positionMode == kSpatial) {
		const float distance = MAX<float>(1.0f, sqrt(x * x + z * z));
		gain = 1.0f / (1.0f + (distance - 1.0f) * _rolloff);
		volume = CLIP<int>(_volume * gain + 0.5f, 0, Audio::Mixer::kMaxChannelVolume);
	}
	if (!_mixer->getOutputStereo()) {
		return;
	}
	// Project the source onto stereo speakers with equal-power panning.
	// Express the two gains using the mixer's volume and linear balance.
	const float pan = atan2(x, fabs(z)) / M_PI + 0.5f;
	const float left = sqrt(1.0f - pan);
	const float right = sqrt(pan);
	const float loudest = MAX(left, right);
	// Panning alone preserves the authored volume on the louder channel.
	if (_positionMode == kSpatial) {
		volume = CLIP<int>(_volume * gain * loudest + 0.5f, 0, Audio::Mixer::kMaxChannelVolume);
	}
	balance = CLIP<int>(127.0f * (right - left) / loudest, -127, 127);
}

void SoundChannel::updateVolume() {
	if (isPlaying()) {
		byte volume;
		int8 balance;
		mixerLevels(volume, balance);
		_mixer->setChannelVolume(_handle, volume);
		_mixer->setChannelBalance(_handle, balance);
	}
}

} // End of namespace Hollywood
