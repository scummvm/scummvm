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

void CSoundChannel::queueBuffer(byte *buffer, unsigned int size, bool playNow, bool playQueue, bool buffering = true) {
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

/****************************************************************/

SoundGroup::SoundGroup(CryoEngine *vm, int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) : _vm(vm) {
	if (numSounds < kCryoMaxClSounds)
		_numSounds = numSounds;
	else
		error("SoundGroup - numSounds >= kCryoMaxClSounds");

	for (int i = 0; i < _numSounds; i++) {
		_sounds[i] = new Sound(length, rate, sampleSize, mode);
		_sounds[i]->_maxLength = length;
	}
	_soundIndex = 0;
	_playIndex = 0;
}

// Original name: CLSoundGroup_Free
SoundGroup::~SoundGroup() {
	for (int16 i = 0; i < _numSounds; i++)
		delete(_sounds[i]);
}

// Original name: CLSoundGroup_GetNextBuffer
void *SoundGroup::getNextBuffer() {
	Sound *sound = _sounds[_soundIndex];
	return sound->_sndHandle + sound->_headerLen;
}

// Original name: CLSoundGroup_AssignDatas
bool SoundGroup::assignDatas(void *buffer, int length, bool isSigned) {
	Sound *sound = _sounds[_soundIndex];

	sound->_buffer = (char *)buffer;
	sound->setLength(length);
	sound->_length = length;
	//	if(sound->reversed && sound->sampleSize == 16)
	//		ReverseBlock16(buffer, length);
	//	if(isSigned)
	//		CLSound_Signed2NonSigned(buffer, length);
	if (_soundIndex == _numSounds - 1)
		_soundIndex = 0;
	else
		_soundIndex++;

	return true;
}

// Original name: CLSoundGroup_SetDatas
bool SoundGroup::setDatas(void *data, int length, bool isSigned) {
	Sound *sound = _sounds[_soundIndex];
	if (length >= sound->_maxLength)
		error("CLSoundGroup_SetDatas - Unexpected length");

	void *buffer = sound->_sndHandle + sound->_headerLen;
	sound->_buffer = (char *)buffer;
	memcpy(buffer, data, length);
	sound->setLength(length);
	sound->_length = length;
	//	if(sound->reversed && sound->sampleSize == 16)
	//		ReverseBlock16(buffer, length);
	//	if(isSigned)
	//		CLSound_Signed2NonSigned(buffer, length);
	if (_soundIndex == _numSounds - 1)
		_soundIndex = 0;
	else
		_soundIndex++;

	return true;
}

// Original name: CLSoundGroup_PlayNextSample
void SoundGroup::playNextSample(SoundChannel *ch) {
	ch->play(_sounds[_playIndex]);
	if (_playIndex == _numSounds - 1)
		_playIndex = 0;
	else
		_playIndex++;
}


}
