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

#include "cryo/sound.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Cryo {

CSoundChannel::CSoundChannel(Audio::Mixer *mixer, unsigned int sampleRate, bool stereo, bool is16bits) : _mixer(mixer), _sampleRate(sampleRate), _stereo(stereo) {
	_bufferFlags = is16bits ? (Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_16BITS) : Audio::FLAG_UNSIGNED;
	if (stereo)
		_bufferFlags |= Audio::FLAG_STEREO;
	_audioStream = nullptr;
	_volumeLeft = _volumeRight = Audio::Mixer::kMaxChannelVolume;
}

CSoundChannel::~CSoundChannel() {
	stop();
	if (_audioStream)
		delete _audioStream;
}

void CSoundChannel::queueBuffer(byte *buffer, unsigned int size, bool playNow, bool playQueue, bool buffering) {
	if (playNow)
		stop();

	if (!buffer || !size)
		return;

	if (!_audioStream)
		_audioStream = Audio::makeQueuingAudioStream(_sampleRate, _stereo);

	if (buffering) {
		byte *localBuffer = (byte*)malloc(size);
		memcpy(localBuffer, buffer, size);
		_audioStream->queueBuffer(localBuffer, size, DisposeAfterUse::YES, _bufferFlags);
	} else
		_audioStream->queueBuffer(buffer, size, DisposeAfterUse::NO, _bufferFlags);
	if (playNow || playQueue)
		play();
}

void CSoundChannel::play() {
	if (!_audioStream)
		return;
	if (!_mixer->isSoundHandleActive(_soundHandle)) {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		applyVolumeChange();
	}
}

void CSoundChannel::stop() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);

	if (_audioStream) {
		_audioStream->finish();
		delete _audioStream;
		_audioStream = nullptr;
	}
}

unsigned int CSoundChannel::numQueued() {
	return _audioStream ? _audioStream->numQueuedStreams() : 0;
}

unsigned int CSoundChannel::getVolume() {
	return (_volumeRight + _volumeLeft) / 2;
}

void CSoundChannel::setVolume(unsigned int volumeLeft, unsigned int volumeRight) {
	_volumeLeft = volumeLeft;
	_volumeRight = volumeRight;
	applyVolumeChange();
}

void CSoundChannel::setVolumeLeft(unsigned int volume) {
	setVolume(volume, _volumeRight);
}

void CSoundChannel::setVolumeRight(unsigned int volume) {
	setVolume(_volumeLeft, volume);
}

void CSoundChannel::applyVolumeChange() {
	unsigned int volume = (_volumeRight + _volumeLeft) / 2;
	int balance = (signed int)(_volumeRight - _volumeLeft) / 2;
	_mixer->setChannelVolume(_soundHandle, volume);
	_mixer->setChannelBalance(_soundHandle, balance);
}

}
