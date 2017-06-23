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

#include "common/memstream.h"
#include "audio/mixer.h"
#include "neverhood/sound.h"
#include "neverhood/resource.h"
#include "neverhood/resourceman.h"

// Convert volume from percent to 0..255
#define VOLUME(volume) (Audio::Mixer::kMaxChannelVolume / 100 * (volume))

// Convert panning from percent (50% equals center) to -127..0..+127
#define PANNING(panning) (254 / 100 * (panning) - 127)

namespace Neverhood {

SoundResource::SoundResource(NeverhoodEngine *vm)
	: _vm(vm), _soundIndex(-1) {
}

SoundResource::~SoundResource() {
	unload();
}

bool SoundResource::isPlaying() {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	return soundItem ? soundItem->isPlaying() : false;
}

void SoundResource::load(uint32 fileHash) {
	unload();
	_soundIndex = _vm->_audioResourceMan->addSound(fileHash);
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->loadSound();
}

void SoundResource::unload() {
	if (_soundIndex >= 0) {
		_vm->_audioResourceMan->removeSound(_soundIndex);
		_soundIndex = -1;
	}
}

void SoundResource::play(uint32 fileHash) {
	load(fileHash);
	play();
}

void SoundResource::play() {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->playSound(false);
}

void SoundResource::playLooping() {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->playSound(true);
}

void SoundResource::stop() {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->stopSound();
}

void SoundResource::setVolume(int16 volume) {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->setVolume(volume);
}

void SoundResource::setPan(int16 pan) {
	AudioResourceManSoundItem *soundItem = getSoundItem();
	if (soundItem)
		soundItem->setPan(pan);
}

AudioResourceManSoundItem *SoundResource::getSoundItem() {
	return _vm->_audioResourceMan->getSoundItem(_soundIndex);
}

MusicResource::MusicResource(NeverhoodEngine *vm)
	: _vm(vm), _musicIndex(-1) {
}

bool MusicResource::isPlaying() {
	AudioResourceManMusicItem *musicItem = getMusicItem();
	return musicItem && musicItem->isPlaying();
}

void MusicResource::load(uint32 fileHash) {
	unload();
	_musicIndex = _vm->_audioResourceMan->loadMusic(fileHash);
}

void MusicResource::unload() {
	AudioResourceManMusicItem *musicItem = getMusicItem();
	if (musicItem) {
		musicItem->unloadMusic();
		_musicIndex = -1;
	}
}

void MusicResource::play(int16 fadeVolumeStep) {
	AudioResourceManMusicItem *musicItem = getMusicItem();
	if (musicItem)
		musicItem->playMusic(fadeVolumeStep);
}

void MusicResource::stop(int16 fadeVolumeStep) {
	AudioResourceManMusicItem *musicItem = getMusicItem();
	if (musicItem)
		musicItem->stopMusic(fadeVolumeStep);
}

void MusicResource::setVolume(int16 volume) {
	AudioResourceManMusicItem *musicItem = getMusicItem();
	if (musicItem)
		musicItem->setVolume(volume);
}

AudioResourceManMusicItem *MusicResource::getMusicItem() {
	return _vm->_audioResourceMan->getMusicItem(_musicIndex);
}

MusicItem::MusicItem(NeverhoodEngine *vm, uint32 groupNameHash, uint32 musicFileHash)
	: _vm(vm), _musicResource(NULL) {

	_groupNameHash = groupNameHash;
	_fileHash = musicFileHash;
	_play = false;
	_stop = false;
	_fadeVolumeStep = 0;
	_countdown = 24;
	_musicResource = new MusicResource(_vm);
	_musicResource->load(musicFileHash);
}

MusicItem::~MusicItem() {
	if (_musicResource)
		_musicResource->unload();
	delete _musicResource;
}

void MusicItem::startMusic(int16 countdown, int16 fadeVolumeStep) {
	_play = true;
	_stop = false;
	_countdown = countdown;
	_fadeVolumeStep = fadeVolumeStep;
}

void MusicItem::stopMusic(int16 countdown, int16 fadeVolumeStep) {
	_play = false;
	_stop = true;
	_countdown = countdown;
	_fadeVolumeStep = fadeVolumeStep;
}

void MusicItem::update() {
	if (_countdown) {
		--_countdown;
	} else if (_play && !_musicResource->isPlaying()) {
		debug(1, "MusicItem: play music %08X (fade %d)", _fileHash, _fadeVolumeStep);
		_musicResource->play(_fadeVolumeStep);
		_fadeVolumeStep = 0;
	} else if (_stop) {
		debug(1, "MusicItem: stop music %08X (fade %d)", _fileHash, _fadeVolumeStep);
		_musicResource->stop(_fadeVolumeStep);
		_fadeVolumeStep = 0;
		_stop = false;
	}
}

SoundItem::SoundItem(NeverhoodEngine *vm, uint32 groupNameHash, uint32 soundFileHash,
	bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
	bool playOnceAfterCountdown, int16 initialCountdown, bool playLooping, int16 currCountdown)
	: _vm(vm), _soundResource(NULL), _groupNameHash(groupNameHash), _fileHash(soundFileHash),
	_playOnceAfterRandomCountdown(false), _minCountdown(0), _maxCountdown(0),
	_playOnceAfterCountdown(playOnceAfterCountdown), _initialCountdown(initialCountdown),
	_playLooping(false), _currCountdown(currCountdown) {

	_soundResource = new SoundResource(vm);
	_soundResource->load(soundFileHash);
}

SoundItem::~SoundItem() {
	if (_soundResource)
		_soundResource->unload();
	delete _soundResource;
}

void SoundItem::setSoundParams(bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
	int16 firstMinCountdown, int16 firstMaxCountdown) {

	_playOnceAfterCountdown = false;
	_playLooping = false;
	_playOnceAfterRandomCountdown = playOnceAfterRandomCountdown;
	if (minCountdown > 0)
		_minCountdown = minCountdown;
	if (maxCountdown > 0)
		_maxCountdown = maxCountdown;
	if (firstMinCountdown > firstMaxCountdown)
		_currCountdown = firstMinCountdown;
	else if (firstMinCountdown > 0 && firstMaxCountdown > 0 && firstMinCountdown < firstMaxCountdown)
		_currCountdown = _vm->_rnd->getRandomNumberRng(firstMinCountdown, firstMaxCountdown);
}

void SoundItem::playSoundLooping() {
	_playOnceAfterRandomCountdown = false;
	_playOnceAfterCountdown = false;
	_playLooping = true;
}

void SoundItem::stopSound() {
	_playOnceAfterRandomCountdown = false;
	_playOnceAfterCountdown = false;
	_playLooping = false;
	_soundResource->stop();
}

void SoundItem::setVolume(int volume) {
	_soundResource->setVolume(volume);
}

void SoundItem::update() {
	if (_playOnceAfterCountdown) {
		if (_currCountdown == 0)
			_currCountdown = _initialCountdown;
		else if (--_currCountdown <= 0)
			_soundResource->play();
	} else if (_playOnceAfterRandomCountdown) {
		if (_currCountdown == 0) {
			if (_minCountdown > 0 && _maxCountdown > 0 && _minCountdown < _maxCountdown)
				_currCountdown = _vm->_rnd->getRandomNumberRng(_minCountdown, _maxCountdown);
		} else if (--_currCountdown <= 0)
			_soundResource->play();
	} else if (_playLooping && !_soundResource->isPlaying())
		_soundResource->playLooping();
}

// SoundMan

SoundMan::SoundMan(NeverhoodEngine *vm)
	: _vm(vm), _soundIndex1(-1), _soundIndex2(-1), _soundIndex3(-1),
	  _initialCountdown(15), _playOnceAfterCountdown(false),
	  _initialCountdown3(9), _playOnceAfterCountdown3(false) {
}

SoundMan::~SoundMan() {
	stopAllMusic();
	stopAllSounds();
}

void SoundMan::stopAllMusic() {
	for (uint i = 0; i < _musicItems.size(); ++i) {
		if (_musicItems[i]) {
			_musicItems[i]->stopMusic(0, 0);
			delete _musicItems[i];
			_musicItems[i] = NULL;
		}
	}
}

void SoundMan::stopAllSounds() {
	for (uint i = 0; i < _soundItems.size(); ++i) {
		if (_soundItems[i]) {
			_soundItems[i]->stopSound();
			delete _soundItems[i];
			_soundItems[i] = NULL;
		}
	}

	_soundIndex1 = _soundIndex2 = _soundIndex3 = -1;
}

void SoundMan::addMusic(uint32 groupNameHash, uint32 musicFileHash) {
	addMusicItem(new MusicItem(_vm, groupNameHash, musicFileHash));
}

void SoundMan::deleteMusic(uint32 musicFileHash) {
	MusicItem *musicItem = getMusicItemByHash(musicFileHash);
	if (musicItem) {
		delete musicItem;
		for (uint i = 0; i < _musicItems.size(); ++i)
			if (_musicItems[i] == musicItem) {
				_musicItems[i] = NULL;
				break;
			}
	}
}

void SoundMan::startMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep) {
	MusicItem *musicItem = getMusicItemByHash(musicFileHash);
	if (musicItem)
		musicItem->startMusic(countdown, fadeVolumeStep);
}

void SoundMan::stopMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep) {
	MusicItem *musicItem = getMusicItemByHash(musicFileHash);
	if (musicItem)
		musicItem->stopMusic(countdown, fadeVolumeStep);
}

void SoundMan::addSound(uint32 groupNameHash, uint32 soundFileHash) {
	addSoundItem(new SoundItem(_vm, groupNameHash, soundFileHash, false, 50, 600, false, 0, false, 0));
}

void SoundMan::addSoundList(uint32 groupNameHash, const uint32 *soundFileHashList) {
	while (*soundFileHashList)
		addSound(groupNameHash, *soundFileHashList++);
}

void SoundMan::deleteSound(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		delete soundItem;
		for (uint i = 0; i < _soundItems.size(); ++i)
			if (_soundItems[i] == soundItem) {
				_soundItems[i] = NULL;
				break;
			}
	}
}

void SoundMan::setSoundParams(uint32 soundFileHash, bool playOnceAfterRandomCountdown,
	int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown) {

	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->setSoundParams(playOnceAfterRandomCountdown, minCountdown, maxCountdown,
			firstMinCountdown, firstMaxCountdown);
}

void SoundMan::setSoundListParams(const uint32 *soundFileHashList, bool playOnceAfterRandomCountdown,
	int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown) {

	while (*soundFileHashList)
		setSoundParams(*soundFileHashList++, playOnceAfterRandomCountdown,
			minCountdown, maxCountdown, firstMinCountdown, firstMaxCountdown);
}

void SoundMan::playSoundLooping(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->playSoundLooping();
}

void SoundMan::stopSound(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->stopSound();
}

void SoundMan::setSoundVolume(uint32 soundFileHash, int volume) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->setVolume(volume);
}

void SoundMan::update() {

	for (uint i = 0; i < _soundItems.size(); ++i) {
		SoundItem *soundItem = _soundItems[i];
		if (soundItem)
			soundItem->update();
	}
	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem)
			musicItem->update();
	}

}

void SoundMan::deleteGroup(uint32 groupNameHash) {
	deleteMusicGroup(groupNameHash);
	deleteSoundGroup(groupNameHash);
}

void SoundMan::deleteMusicGroup(uint32 groupNameHash) {
	for (uint index = 0; index < _musicItems.size(); ++index) {
		MusicItem *musicItem = _musicItems[index];
		if (musicItem && musicItem->getGroupNameHash() == groupNameHash) {
			delete musicItem;
			_musicItems[index] = NULL;
		}
	}
}

void SoundMan::deleteSoundGroup(uint32 groupNameHash) {

	if (_soundIndex1 != -1 && _soundItems[_soundIndex1]->getGroupNameHash() == groupNameHash) {
		deleteSoundByIndex(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1 && _soundItems[_soundIndex2]->getGroupNameHash() == groupNameHash) {
		deleteSoundByIndex(_soundIndex2);
		_soundIndex2 = -1;
	}

	for (uint index = 0; index < _soundItems.size(); ++index)
		if (_soundItems[index] && _soundItems[index]->getGroupNameHash() == groupNameHash)
			deleteSoundByIndex(index);

}

void SoundMan::playTwoSounds(uint32 groupNameHash, uint32 soundFileHash1, uint32 soundFileHash2, int16 initialCountdown) {

	int16 currCountdown1 = _initialCountdown;
	int16 currCountdown2 = _initialCountdown / 2;

	if (_soundIndex1 != -1) {
		currCountdown1 = _soundItems[_soundIndex1]->getCurrCountdown();
		deleteSoundByIndex(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1) {
		currCountdown2 = _soundItems[_soundIndex2]->getCurrCountdown();
		deleteSoundByIndex(_soundIndex2);
		_soundIndex2 = -1;
	}

	if (initialCountdown > 0)
		_initialCountdown = initialCountdown;

	if (soundFileHash1 != 0) {
		SoundItem *soundItem = new SoundItem(_vm, groupNameHash, soundFileHash1, false, 0, 0,
			_playOnceAfterCountdown, _initialCountdown, false, currCountdown1);
		soundItem->setVolume(80);
		_soundIndex1 = addSoundItem(soundItem);
	}

	if (soundFileHash2 != 0) {
		SoundItem *soundItem = new SoundItem(_vm, groupNameHash, soundFileHash2, false, 0, 0,
			_playOnceAfterCountdown, _initialCountdown, false, currCountdown2);
		soundItem->setVolume(80);
		_soundIndex2 = addSoundItem(soundItem);
	}

}

void SoundMan::playSoundThree(uint32 groupNameHash, uint32 soundFileHash) {

	if (_soundIndex3 != -1) {
		deleteSoundByIndex(_soundIndex3);
		_soundIndex3 = -1;
	}

	if (soundFileHash != 0) {
		SoundItem *soundItem = new SoundItem(_vm, groupNameHash, soundFileHash, false, 0, 0, false, _initialCountdown3, false, 0);
		_soundIndex3 = addSoundItem(soundItem);
	}

}

void SoundMan::setTwoSoundsPlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex1 != -1)
		_soundItems[_soundIndex1]->setPlayOnceAfterCountdown(playOnceAfterCountdown);
	if (_soundIndex2 != -1)
		_soundItems[_soundIndex2]->setPlayOnceAfterCountdown(playOnceAfterCountdown);
	_playOnceAfterCountdown = playOnceAfterCountdown;
}

void SoundMan::setSoundThreePlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex3 != -1)
		_soundItems[_soundIndex3]->setPlayOnceAfterCountdown(playOnceAfterCountdown);
	_playOnceAfterCountdown3 = playOnceAfterCountdown;
}

MusicItem *SoundMan::getMusicItemByHash(uint32 musicFileHash) {
	for (uint i = 0; i < _musicItems.size(); ++i)
		if (_musicItems[i] && _musicItems[i]->getFileHash() == musicFileHash)
			return _musicItems[i];
	return NULL;
}

SoundItem *SoundMan::getSoundItemByHash(uint32 soundFileHash) {
	for (uint i = 0; i < _soundItems.size(); ++i)
		if (_soundItems[i] && _soundItems[i]->getFileHash() == soundFileHash)
			return _soundItems[i];
	return NULL;
}

int16 SoundMan::addMusicItem(MusicItem *musicItem) {
	for (uint i = 0; i < _musicItems.size(); ++i)
		if (!_musicItems[i]) {
			_musicItems[i] = musicItem;
			return i;
		}
	int16 musicIndex = _musicItems.size();
	_musicItems.push_back(musicItem);
	return musicIndex;
}

int16 SoundMan::addSoundItem(SoundItem *soundItem) {
	for (uint i = 0; i < _soundItems.size(); ++i)
		if (!_soundItems[i]) {
			_soundItems[i] = soundItem;
			return i;
		}
	int16 soundIndex = _soundItems.size();
	_soundItems.push_back(soundItem);
	return soundIndex;
}

void SoundMan::deleteSoundByIndex(int index) {
	delete _soundItems[index];
	_soundItems[index] = NULL;
}

// NeverhoodAudioStream

NeverhoodAudioStream::NeverhoodAudioStream(int rate, byte shiftValue, bool isLooping, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream)
	: _rate(rate), _shiftValue(shiftValue), _isLooping(isLooping), _isStereo(false), _stream(stream, disposeStream), _endOfData(false), _buffer(0),
	_isCompressed(_shiftValue != 0xFF), _prevValue(0) {
	// Setup our buffer for readBuffer
	_buffer = new byte[kSampleBufferLength * (_isCompressed ? 1 : 2)];
	assert(_buffer);
}

NeverhoodAudioStream::~NeverhoodAudioStream() {
	delete[] _buffer;
}

int NeverhoodAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesLeft = numSamples;

	while (samplesLeft > 0 && !_endOfData) {

		const int maxSamples = MIN<int>(kSampleBufferLength, samplesLeft);
		const int bytesToRead = maxSamples * (_isCompressed ? 1 : 2);
		int bytesRead = _stream->read(_buffer, bytesToRead);
		int samplesRead = bytesRead / (_isCompressed ? 1 : 2);

		samplesLeft -= samplesRead;

		const byte *src = _buffer;
		if (_isCompressed) {
			while (samplesRead--) {
				_prevValue += (int8)(*src++);
				*buffer++ = _prevValue << _shiftValue;
			}
		} else {
			while (samplesRead--) {
				*buffer++ = READ_LE_UINT16(src);
				src += 2;
			}
		}

		if (bytesRead < bytesToRead || _stream->pos() >= _stream->size() || _stream->err() || _stream->eos()) {
			if (_isLooping) {
				_stream->seek(0);
				_prevValue = 0;
			} else {
				_endOfData = true;
			}
		}

	}

	return numSamples - samplesLeft;
}

AudioResourceManSoundItem::AudioResourceManSoundItem(NeverhoodEngine *vm, uint32 fileHash)
	: _vm(vm), _fileHash(fileHash), _data(NULL), _isLoaded(false), _isPlaying(false),
	_volume(100), _panning(50) {

	_vm->_res->queryResource(_fileHash, _resourceHandle);
	_soundHandle = new Audio::SoundHandle();
}

AudioResourceManSoundItem::~AudioResourceManSoundItem() {
	delete _soundHandle;
}

void AudioResourceManSoundItem::loadSound() {
	if (!_data && _resourceHandle.isValid() &&
		(_resourceHandle.type() == kResTypeSound || _resourceHandle.type() == kResTypeMusic)) {
		_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
		_data = _resourceHandle.data();
	}
}

void AudioResourceManSoundItem::unloadSound() {
	if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->stopHandle(*_soundHandle);
	_vm->_res->unloadResource(_resourceHandle);
	_data = NULL;
}

void AudioResourceManSoundItem::setVolume(int16 volume) {
	_volume = MIN<int16>(volume, 100);
	if (_isPlaying && _vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->setChannelVolume(*_soundHandle, VOLUME(_volume));
}

void AudioResourceManSoundItem::setPan(int16 pan) {
	_panning = MIN<int16>(pan, 100);
	if (_isPlaying && _vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->setChannelVolume(*_soundHandle, PANNING(_panning));
}

void AudioResourceManSoundItem::playSound(bool looping) {
	if (!_data)
		loadSound();
	if (_data) {
		const byte *shiftValue = _resourceHandle.extData();
		Common::MemoryReadStream *stream = new Common::MemoryReadStream(_data, _resourceHandle.size(), DisposeAfterUse::NO);
		NeverhoodAudioStream *audioStream = new NeverhoodAudioStream(22050, *shiftValue, looping, DisposeAfterUse::YES, stream);
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _soundHandle,
			audioStream, -1, VOLUME(_volume), PANNING(_panning));
		debug(1, "playing sound %08X", _fileHash);
		_isPlaying = true;
	}
}

void AudioResourceManSoundItem::stopSound() {
	if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->stopHandle(*_soundHandle);
	_isPlaying = false;
}

bool AudioResourceManSoundItem::isPlaying() {
	return _vm->_mixer->isSoundHandleActive(*_soundHandle);
}

AudioResourceManMusicItem::AudioResourceManMusicItem(NeverhoodEngine *vm, uint32 fileHash)
	: _vm(vm), _fileHash(fileHash), _terminate(false), _canRestart(false),
	_volume(100), _panning(50),	_start(false), _isFadingIn(false), _isFadingOut(false), _isPlaying(false),
	_fadeVolume(0), _fadeVolumeStep(0) {

	_soundHandle = new Audio::SoundHandle();
}

AudioResourceManMusicItem::~AudioResourceManMusicItem() {
	delete _soundHandle;
}

void AudioResourceManMusicItem::playMusic(int16 fadeVolumeStep) {
	if (!_isPlaying) {
		_isFadingIn = false;
		_isFadingOut = false;
		if (fadeVolumeStep != 0) {
			_isFadingIn = true;
			_fadeVolume = 0;
			_fadeVolumeStep = fadeVolumeStep;
		}
		_start = true;
		_terminate = false;
	}
}

void AudioResourceManMusicItem::stopMusic(int16 fadeVolumeStep) {
	if (_vm->_mixer->isSoundHandleActive(*_soundHandle)) {
		if (fadeVolumeStep != 0) {
			if (_isFadingIn)
				_isFadingIn = false;
			else
				_fadeVolume = _volume;
			_isFadingOut = true;
			_fadeVolumeStep = fadeVolumeStep;
		} else {
			_vm->_mixer->stopHandle(*_soundHandle);
		}
		_isPlaying = false;
	}
}

void AudioResourceManMusicItem::unloadMusic() {
	if (_isFadingOut) {
		_canRestart = true;
	} else {
		if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
			_vm->_mixer->stopHandle(*_soundHandle);
		_isPlaying = false;
		_terminate = true;
	}
}

void AudioResourceManMusicItem::setVolume(int16 volume) {
	_volume = MIN<int16>(volume, 100);
	if (_isPlaying && _vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->setChannelVolume(*_soundHandle, VOLUME(_volume));
}

void AudioResourceManMusicItem::restart() {
	_canRestart = false;
	_isFadingOut = false;
	_isFadingIn = true;
}

void AudioResourceManMusicItem::update() {

	if (_start && !_vm->_mixer->isSoundHandleActive(*_soundHandle)) {
		ResourceHandle resourceHandle;
		_vm->_res->queryResource(_fileHash, resourceHandle);
		Common::SeekableReadStream *stream = _vm->_res->createStream(_fileHash);
		const byte *shiftValue = resourceHandle.extData();
		NeverhoodAudioStream *audioStream = new NeverhoodAudioStream(22050, *shiftValue, true, DisposeAfterUse::YES, stream);
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, _soundHandle,
			audioStream, -1, VOLUME(_isFadingIn ? _fadeVolume : _volume),
			PANNING(_panning));
		_start = false;
		_isPlaying = true;
	}

	if (_vm->_mixer->isSoundHandleActive(*_soundHandle)) {
		if (_isFadingIn) {
			_fadeVolume += _fadeVolumeStep;
			if (_fadeVolume >= _volume) {
				_fadeVolume = _volume;
				_isFadingIn = false;
			}
			_vm->_mixer->setChannelVolume(*_soundHandle, VOLUME(_fadeVolume));
		}
		if (_isFadingOut) {
			_fadeVolume -= _fadeVolumeStep;
			if (_fadeVolume < 0)
				_fadeVolume = 0;
			_vm->_mixer->setChannelVolume(*_soundHandle, VOLUME(_fadeVolume));
			if (_fadeVolume == 0) {
				_isFadingOut = false;
				stopMusic(0);
				if (_canRestart)
					unloadMusic();
			}
		}
	}

}

AudioResourceMan::AudioResourceMan(NeverhoodEngine *vm)
	: _vm(vm) {
}

void AudioResourceMan::stopAllMusic() {
	for (uint i = 0; i < _musicItems.size(); ++i) {
		if (_musicItems[i]) {
			_musicItems[i]->stopMusic(0);
			delete _musicItems[i];
			_musicItems[i] = NULL;
		}
	}
}

void AudioResourceMan::stopAllSounds() {
	for (uint i = 0; i < _soundItems.size(); ++i) {
		if (_soundItems[i]) {
			_soundItems[i]->stopSound();
			delete _soundItems[i];
			_soundItems[i] = NULL;
		}
	}
}

AudioResourceMan::~AudioResourceMan() {
	stopAllMusic();
	stopAllSounds();
}

int16 AudioResourceMan::addSound(uint32 fileHash) {
	AudioResourceManSoundItem *soundItem = new AudioResourceManSoundItem(_vm, fileHash);

	for (uint i = 0; i < _soundItems.size(); ++i)
		if (!_soundItems[i]) {
			_soundItems[i] = soundItem;
			return i;
		}

	int16 soundIndex = (int16)_soundItems.size();
	_soundItems.push_back(soundItem);
	return soundIndex;
}

void AudioResourceMan::removeSound(int16 soundIndex) {
	AudioResourceManSoundItem *soundItem = getSoundItem(soundIndex);
	if (soundItem) {
		soundItem->unloadSound();
		delete soundItem;
		_soundItems[soundIndex] = NULL;
	}
}

int16 AudioResourceMan::loadMusic(uint32 fileHash) {
	AudioResourceManMusicItem *musicItem;

	for (uint i = 0; i < _musicItems.size(); ++i) {
		musicItem = _musicItems[i];
		if (musicItem && musicItem->getFileHash() == fileHash && musicItem->canRestart()) {
			musicItem->restart();
			return i;
		}
	}

	musicItem = new AudioResourceManMusicItem(_vm, fileHash);

	for (uint i = 0; i < _musicItems.size(); ++i) {
		if (!_musicItems[i]) {
			_musicItems[i] = musicItem;
			return i;
		}
	}

	int16 musicIndex = _musicItems.size();
	_musicItems.push_back(musicItem);
	return musicIndex;

}

void AudioResourceMan::updateMusic() {
	for (uint musicIndex = 0; musicIndex < _musicItems.size(); ++musicIndex) {
		AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
		if (musicItem) {
			musicItem->update();
			if (musicItem->isTerminated()) {
				delete musicItem;
				_musicItems[musicIndex] = NULL;
			}
		}
	}
}

AudioResourceManSoundItem *AudioResourceMan::getSoundItem(int16 index) {
	return (index >= 0 && index < (int16)_soundItems.size()) ? _soundItems[index] : NULL;
}

AudioResourceManMusicItem *AudioResourceMan::getMusicItem(int16 index) {
	return (index >= 0 && index < (int16)_musicItems.size()) ? _musicItems[index] : NULL;
}

} // End of namespace Neverhood
