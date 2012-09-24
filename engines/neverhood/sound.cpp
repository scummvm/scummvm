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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/memstream.h"
#include "graphics/palette.h"
#include "neverhood/sound.h"
#include "neverhood/resourceman.h"

namespace Neverhood {

// TODO Put more stuff into the constructors/destructors of the item structs
// TODO Some parts are quite bad here, but my priority is to get sound working at all

SoundResource::SoundResource(NeverhoodEngine *vm)
	: _vm(vm), _soundIndex(-1) {
}

SoundResource::~SoundResource() {
	unload();
}

bool SoundResource::isPlaying() { 
	return _soundIndex >= 0 &&
		_vm->_audioResourceMan->isSoundPlaying(_soundIndex); 
}

void SoundResource::load(uint32 fileHash) {
	unload();
	_soundIndex = _vm->_audioResourceMan->addSound(fileHash);
	_vm->_audioResourceMan->loadSound(_soundIndex);
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
	if (_soundIndex >= 0)
		_vm->_audioResourceMan->playSound(_soundIndex, false);
}

void SoundResource::stop() {
	if (_soundIndex >= 0)
		_vm->_audioResourceMan->stopSound(_soundIndex);
}

void SoundResource::setVolume(int16 volume) {
	if (_soundIndex >= 0)
		_vm->_audioResourceMan->setSoundVolume(_soundIndex, volume);
}

void SoundResource::setPan(int16 pan) {
	if (_soundIndex >= 0)
		_vm->_audioResourceMan->setSoundPan(_soundIndex, pan);
}

MusicResource::MusicResource(NeverhoodEngine *vm)
	: _vm(vm), _musicIndex(-1) {
}

bool MusicResource::isPlaying() { 
	return _musicIndex >= 0 &&
		_vm->_audioResourceMan->isMusicPlaying(_musicIndex); 
}

void MusicResource::load(uint32 fileHash) {
	unload();
	_musicIndex = _vm->_audioResourceMan->loadMusic(fileHash);
}

void MusicResource::unload() {
	if (_musicIndex >= 0) {
		_vm->_audioResourceMan->unloadMusic(_musicIndex);
		_musicIndex = -1;
	}
}

void MusicResource::play(int16 fadeVolumeStep) {
	if (_musicIndex >= 0)
		_vm->_audioResourceMan->playMusic(_musicIndex, fadeVolumeStep);
}

void MusicResource::stop(int16 fadeVolumeStep) {
	if (_musicIndex >= 0)
		_vm->_audioResourceMan->stopMusic(_musicIndex, fadeVolumeStep);
}

void MusicResource::setVolume(int16 volume) {
	if (_musicIndex >= 0)
		_vm->_audioResourceMan->setMusicVolume(_musicIndex, volume);
}

MusicItem::MusicItem()
	: _musicResource(NULL) {
}

MusicItem::~MusicItem() {
	if (_musicResource)
		_musicResource->unload();
	delete _musicResource;
}

SoundItem::SoundItem(NeverhoodEngine *vm, uint32 nameHash, uint32 soundFileHash,
		bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
		bool playOnceAfterCountdown, int16 initialCountdown, bool playLooping, int16 currCountdown)
	: _soundResource(NULL),	_nameHash(nameHash), _soundFileHash(soundFileHash),
	_playOnceAfterRandomCountdown(false), _minCountdown(0), _maxCountdown(0),
	_playOnceAfterCountdown(_playOnceAfterCountdown), _initialCountdown(initialCountdown),
	_playLooping(false), _currCountdown(currCountdown) {
	
	_soundResource = new SoundResource(vm);
	_soundResource->load(soundFileHash);
}

SoundItem::~SoundItem() {
	if (_soundResource)
		_soundResource->unload();
	delete _soundResource;
}

// SoundMan

SoundMan::SoundMan(NeverhoodEngine *vm)
	: _vm(vm),
	_soundIndex1(-1), _soundIndex2(-1), _soundIndex3(-1) {
}

SoundMan::~SoundMan() {
	// TODO Clean up
}

void SoundMan::addMusic(uint32 nameHash, uint32 musicFileHash) {
	MusicItem *musicItem = new MusicItem();
	musicItem->_nameHash = nameHash;
	musicItem->_musicFileHash = musicFileHash;
	musicItem->_play = false;
	musicItem->_stop = false;
	musicItem->_fadeVolumeStep = 0;
	musicItem->_countdown = 24;
	musicItem->_musicResource = new MusicResource(_vm);
	musicItem->_musicResource->load(musicFileHash);
	_musicItems.push_back(musicItem);
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
	if (musicItem) {
		musicItem->_play = true;
		musicItem->_stop = false;
		musicItem->_countdown = countdown;
		musicItem->_fadeVolumeStep = fadeVolumeStep;
	}
}

void SoundMan::stopMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep) {
	MusicItem *musicItem = getMusicItemByHash(musicFileHash);
	if (musicItem) {
		musicItem->_play = false;
		musicItem->_stop = true;
		musicItem->_countdown = countdown;
		musicItem->_fadeVolumeStep = fadeVolumeStep;
	}
}

void SoundMan::addSound(uint32 nameHash, uint32 soundFileHash) {
	SoundItem *soundItem = new SoundItem(_vm, nameHash, soundFileHash, false, 50, 600, false, 0, false, 0);
	_soundItems.push_back(soundItem);
}

void SoundMan::addSoundList(uint32 nameHash, const uint32 *soundFileHashList) {
	while (*soundFileHashList)
		addSound(nameHash, *soundFileHashList++);
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
	if (soundItem) {
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = false;
		soundItem->_playOnceAfterRandomCountdown = playOnceAfterRandomCountdown;
		if (minCountdown > 0)
			soundItem->_minCountdown = minCountdown;
		if (maxCountdown > 0)
			soundItem->_maxCountdown = maxCountdown;
		if (firstMinCountdown >= firstMaxCountdown)
			soundItem->_currCountdown = firstMinCountdown;
		else if (firstMinCountdown > 0 && firstMaxCountdown > 0 && firstMinCountdown < firstMaxCountdown)
			soundItem->_currCountdown = _vm->_rnd->getRandomNumberRng(firstMinCountdown, firstMaxCountdown);
	}
}

void SoundMan::setSoundListParams(const uint32 *soundFileHashList, bool playOnceAfterRandomCountdown,
	int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown) {

	while (*soundFileHashList)
		setSoundParams(*soundFileHashList++, playOnceAfterRandomCountdown,
			minCountdown, maxCountdown, firstMinCountdown, firstMaxCountdown);
}

void SoundMan::playSoundLooping(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = true;
	}
}

void SoundMan::stopSound(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = false;
		soundItem->_soundResource->stop();
	}
}

void SoundMan::setSoundVolume(uint32 soundFileHash, int volume) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->_soundResource->setVolume(volume);
}

void SoundMan::update() {
	// TODO Check if active
	
	for (uint i = 0; i < _soundItems.size(); ++i) {
		SoundItem *soundItem = _soundItems[i];
		if (soundItem) {
			if (soundItem->_playOnceAfterCountdown) {
				if (soundItem->_currCountdown == 0) {
					soundItem->_currCountdown = soundItem->_initialCountdown;
				} else if (--soundItem->_currCountdown == 0) {
					soundItem->_soundResource->play();
				}
			} else if (soundItem->_playOnceAfterRandomCountdown) {
				if (soundItem->_currCountdown == 0) {
					if (soundItem->_minCountdown > 0 && soundItem->_maxCountdown > 0 && soundItem->_minCountdown < soundItem->_maxCountdown)
						soundItem->_currCountdown = _vm->_rnd->getRandomNumberRng(soundItem->_minCountdown, soundItem->_maxCountdown);
				} else if (--soundItem->_currCountdown == 0) {
					soundItem->_soundResource->play();
				}
			} else if (soundItem->_playLooping && !soundItem->_soundResource->isPlaying()) {
				soundItem->_soundResource->play(); // TODO Looping parameter?
			}
		}
	}

	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem) {
			if (musicItem->_countdown) {
				--musicItem->_countdown;
			} else if (musicItem->_play && !musicItem->_musicResource->isPlaying()) {
				debug("SoundMan: play music %08X (fade %d)", musicItem->_musicFileHash, musicItem->_fadeVolumeStep);
				musicItem->_musicResource->play(musicItem->_fadeVolumeStep);
				musicItem->_fadeVolumeStep = 0;
			} else if (musicItem->_stop) {
				debug("SoundMan: stop music %08X (fade %d)", musicItem->_musicFileHash, musicItem->_fadeVolumeStep);
				musicItem->_musicResource->stop(musicItem->_fadeVolumeStep);
				musicItem->_fadeVolumeStep = 0;
				musicItem->_stop = false;
			}
		}
	}

}

void SoundMan::deleteGroup(uint32 nameHash) {
	deleteMusicGroup(nameHash);
	deleteSoundGroup(nameHash);
}

void SoundMan::deleteMusicGroup(uint32 nameHash) {
	for (uint index = 0; index < _musicItems.size(); ++index) {
		MusicItem *musicItem = _musicItems[index];
		if (musicItem && musicItem->_nameHash == nameHash) {
			delete musicItem;
			_musicItems[index] = NULL;
		}
	}
}

void SoundMan::deleteSoundGroup(uint32 nameHash) {

	SoundItem *soundItem;

	if (_soundIndex1 != -1 && _soundItems[_soundIndex1]->_nameHash == nameHash) {
		deleteSoundByIndex(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1 && _soundItems[_soundIndex2]->_nameHash == nameHash) {
		deleteSoundByIndex(_soundIndex2);
		_soundIndex2 = -1;
	}

	for (uint index = 0; index < _soundItems.size(); ++index) {
		soundItem = _soundItems[index];
		if (soundItem && soundItem->_nameHash == nameHash) {
			delete soundItem;
			_soundItems[index] = NULL;
		}
	}

}

void SoundMan::playTwoSounds(uint32 nameHash, uint32 soundFileHash1, uint32 soundFileHash2, int16 initialCountdown) {

	SoundItem *soundItem;
	int16 currCountdown1 = _initialCountdown;
	int16 currCountdown2 = _initialCountdown / 2;

	if (_soundIndex1 != -1) {
		currCountdown1 = _soundItems[_soundIndex1]->_currCountdown;
		deleteSoundByIndex(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1) {
		currCountdown2 = _soundItems[_soundIndex2]->_currCountdown;
		deleteSoundByIndex(_soundIndex2);
		_soundIndex2 = -1;
	}

	if (initialCountdown > 0)
		_initialCountdown = initialCountdown;

	if (soundFileHash1 != 0) {
		soundItem = new SoundItem(_vm, nameHash, soundFileHash1, false, 0, 0,
			_playOnceAfterCountdown, _initialCountdown, false, currCountdown1);
		soundItem->_soundResource->setVolume(80);
		_soundIndex1 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}

	if (soundFileHash2 != 0) {
		soundItem = new SoundItem(_vm, nameHash, soundFileHash2, false, 0, 0,
			_playOnceAfterCountdown, _initialCountdown, false, currCountdown2);
		soundItem->_soundResource->setVolume(80);
		_soundIndex2 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}

}

void SoundMan::playSoundThree(uint32 nameHash, uint32 soundFileHash) {

	SoundItem *soundItem;

	if (_soundIndex3 != -1) {
		deleteSoundByIndex(_soundIndex3);
		_soundIndex3 = -1;
	}

	if (soundFileHash != 0) {
		soundItem = new SoundItem(_vm, nameHash, soundFileHash, false, 0, 0,
			false, _initialCountdown3, false, 0);
		_soundIndex3 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}
	
}

void SoundMan::setTwoSoundsPlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex1 != -1)
		_soundItems[_soundIndex1]->_playOnceAfterCountdown = playOnceAfterCountdown;
	if (_soundIndex2 != -1)
		_soundItems[_soundIndex2]->_playOnceAfterCountdown = playOnceAfterCountdown;
	_playOnceAfterCountdown = playOnceAfterCountdown;
}

void SoundMan::setSoundThreePlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex3 != -1)
		_soundItems[_soundIndex3]->_playOnceAfterCountdown = playOnceAfterCountdown;
	_playOnceAfterCountdown3 = playOnceAfterCountdown;
}

MusicItem *SoundMan::getMusicItemByHash(uint32 musicFileHash) {
	for (uint i = 0; i < _musicItems.size(); ++i)
		if (_musicItems[i] && _musicItems[i]->_musicFileHash == musicFileHash)
			return _musicItems[i];
	return NULL;
}

SoundItem *SoundMan::getSoundItemByHash(uint32 soundFileHash) {
	for (uint i = 0; i < _soundItems.size(); ++i)
		if (_soundItems[i] && _soundItems[i]->_soundFileHash == soundFileHash)
			return _soundItems[i];
	return NULL;
}

int16 SoundMan::addMusicItem(MusicItem *musicItem) {
	return 0; // TODO
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
			memcpy(buffer, _buffer, bytesRead);
			buffer += bytesRead;
		}

		if (bytesRead < bytesToRead || _stream->pos() >= _stream->size() || _stream->err() || _stream->eos()) {
			if (_isLooping)
				_stream->seek(0);
			else
				_endOfData = true;
		}

	}

	return numSamples - samplesLeft;
}

AudioResourceMan::AudioResourceMan(NeverhoodEngine *vm)
	: _vm(vm) {
}

AudioResourceMan::~AudioResourceMan() {
}

int16 AudioResourceMan::addSound(uint32 fileHash) {
	AudioResourceManSoundItem *soundItem = new AudioResourceManSoundItem();
	soundItem->_resourceHandle = _vm->_res->useResource(fileHash);
	soundItem->_fileHash = fileHash;
	soundItem->_data = NULL;
	soundItem->_isLoaded = false;
	soundItem->_isPlaying = false;
	soundItem->_volume = 100;
	soundItem->_panning = 50;

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
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	if (soundItem->_data) {
		_vm->_res->unloadResource(soundItem->_resourceHandle);
		soundItem->_data = NULL;
	}
	if (soundItem->_resourceHandle != 1) {
		_vm->_res->unuseResource(soundItem->_resourceHandle);
		soundItem->_resourceHandle = -1;
	}
	if (_vm->_mixer->isSoundHandleActive(soundItem->_soundHandle))
		_vm->_mixer->stopHandle(soundItem->_soundHandle);
	delete soundItem;
	_soundItems[soundIndex] = NULL;
}

void AudioResourceMan::loadSound(int16 soundIndex) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	if (!soundItem->_data) {
		// TODO Check if it's a sound resource
		soundItem->_data = _vm->_res->loadResource(soundItem->_resourceHandle);
	}
}

void AudioResourceMan::unloadSound(int16 soundIndex) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	if (soundItem->_data) {
		_vm->_res->unloadResource(soundItem->_resourceHandle);
		soundItem->_data = NULL;
	}
}

void AudioResourceMan::setSoundVolume(int16 soundIndex, int16 volume) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	soundItem->_volume = MIN<int16>(volume, 100);
	if (soundItem->_isPlaying && _vm->_mixer->isSoundHandleActive(soundItem->_soundHandle))
		_vm->_mixer->setChannelVolume(soundItem->_soundHandle, VOLUME(soundItem->_volume));
}

void AudioResourceMan::setSoundPan(int16 soundIndex, int16 pan) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	soundItem->_panning = MIN<int16>(pan, 100);
	if (soundItem->_isPlaying && _vm->_mixer->isSoundHandleActive(soundItem->_soundHandle))
		_vm->_mixer->setChannelVolume(soundItem->_soundHandle, PANNING(soundItem->_panning));
}

void AudioResourceMan::playSound(int16 soundIndex, bool looping) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	if (!soundItem->_data)
		loadSound(soundIndex);
		
	uint32 soundSize = _vm->_res->getResourceSize(soundItem->_resourceHandle);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(soundItem->_data, soundSize, DisposeAfterUse::NO);
	byte *shiftValue = _vm->_res->getResourceExtData(soundItem->_resourceHandle);
	NeverhoodAudioStream *audioStream = new NeverhoodAudioStream(22050, *shiftValue, false, DisposeAfterUse::YES, stream);

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundItem->_soundHandle,
		audioStream, -1, VOLUME(soundItem->_volume), PANNING(soundItem->_panning));
		
	debug("playing sound %08X", soundItem->_fileHash);
	
	soundItem->_isPlaying = true;
	
}

void AudioResourceMan::stopSound(int16 soundIndex) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	if (_vm->_mixer->isSoundHandleActive(soundItem->_soundHandle))
		_vm->_mixer->stopHandle(soundItem->_soundHandle);
	soundItem->_isPlaying = false;
}

bool AudioResourceMan::isSoundPlaying(int16 soundIndex) {
	AudioResourceManSoundItem *soundItem = _soundItems[soundIndex];
	return soundItem->_isPlaying;
}

int16 AudioResourceMan::loadMusic(uint32 fileHash) {

	AudioResourceManMusicItem *musicItem;

	for (uint i = 0; i < _musicItems.size(); ++i) {
		musicItem = _musicItems[i];
		if (musicItem && musicItem->_fileHash == fileHash && musicItem->_remove) {
			musicItem->_remove = false;
			musicItem->_isFadingOut = false;
			musicItem->_isFadingIn = true;
			return i;
		}
	}
	
	musicItem = new AudioResourceManMusicItem();
	musicItem->_fileHash = fileHash;
	musicItem->_isPlaying = false;
	musicItem->_remove = false;
	musicItem->_volume = 100;
	musicItem->_panning = 50;
	musicItem->_start = false;
	musicItem->_isFadingIn = false;
	musicItem->_isFadingOut = false;

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

void AudioResourceMan::unloadMusic(int16 musicIndex) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
	if (musicItem->_isFadingOut) {
		musicItem->_remove = true;
	} else {
		if (_vm->_mixer->isSoundHandleActive(musicItem->_soundHandle))
			_vm->_mixer->stopHandle(musicItem->_soundHandle);
		musicItem->_isPlaying = false;
		_musicItems[musicIndex] = NULL;
	}
}

void AudioResourceMan::setMusicVolume(int16 musicIndex, int16 volume) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
	musicItem->_volume = MIN<int16>(volume, 100);
	if (musicItem->_isPlaying && _vm->_mixer->isSoundHandleActive(musicItem->_soundHandle))
		_vm->_mixer->setChannelVolume(musicItem->_soundHandle, VOLUME(musicItem->_volume));
}	

void AudioResourceMan::playMusic(int16 musicIndex, int16 fadeVolumeStep) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
	if (!musicItem->_isPlaying) {
		musicItem->_isFadingIn = false;
		musicItem->_isFadingOut = false;
		if (fadeVolumeStep != 0) {
			musicItem->_isFadingIn = true;
			musicItem->_fadeVolume = 0;
			musicItem->_fadeVolumeStep = fadeVolumeStep;
		}
		musicItem->_start = true;
	}
}

void AudioResourceMan::stopMusic(int16 musicIndex, int16 fadeVolumeStep) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
	if (_vm->_mixer->isSoundHandleActive(musicItem->_soundHandle)) {
		if (fadeVolumeStep != 0) {
			if (musicItem->_isFadingIn)
				musicItem->_isFadingIn = false;
			else
				musicItem->_fadeVolume = musicItem->_volume;
			musicItem->_isFadingOut = true;
			musicItem->_fadeVolumeStep = fadeVolumeStep;
		} else {
			_vm->_mixer->stopHandle(musicItem->_soundHandle);
		}
		musicItem->_isPlaying = false;
	}
}

bool AudioResourceMan::isMusicPlaying(int16 musicIndex) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];
	return musicItem->_isPlaying;
}

void AudioResourceMan::updateMusicItem(int16 musicIndex) {
	AudioResourceManMusicItem *musicItem = _musicItems[musicIndex];

	if (musicItem->_start && !_vm->_mixer->isSoundHandleActive(musicItem->_soundHandle)) {
		Common::SeekableReadStream *stream = _vm->_res->createStream(musicItem->_fileHash);
		byte *shiftValue = _vm->_res->getResourceExtDataByHash(musicItem->_fileHash);
		NeverhoodAudioStream *audioStream = new NeverhoodAudioStream(22050, *shiftValue, true, DisposeAfterUse::YES, stream);
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &musicItem->_soundHandle,
			audioStream, -1, VOLUME(musicItem->_isFadingIn ? musicItem->_fadeVolume : musicItem->_volume),
			PANNING(musicItem->_panning));
		musicItem->_start = false;
		musicItem->_isPlaying = true;
	}
	
	if (_vm->_mixer->isSoundHandleActive(musicItem->_soundHandle)) {
		if (musicItem->_isFadingIn) {
			musicItem->_fadeVolume += musicItem->_fadeVolumeStep;
			if (musicItem->_fadeVolume >= musicItem->_volume) {
				musicItem->_fadeVolume = musicItem->_volume;
				musicItem->_isFadingIn = false;
			}
			_vm->_mixer->setChannelVolume(musicItem->_soundHandle, VOLUME(musicItem->_fadeVolume));
		}
		if (musicItem->_isFadingOut) {
			musicItem->_fadeVolume -= musicItem->_fadeVolumeStep;
			if (musicItem->_fadeVolume < 0)
				musicItem->_fadeVolume = 0;
			_vm->_mixer->setChannelVolume(musicItem->_soundHandle, VOLUME(musicItem->_fadeVolume));
			if (musicItem->_fadeVolume == 0) {
				musicItem->_isFadingOut = false;
				stopMusic(musicIndex, 0);
				if (musicItem->_remove)
					unloadMusic(musicIndex);
			}
		}
	}

}

void AudioResourceMan::update() {
	for (uint i = 0; i < _musicItems.size(); ++i)
		if (_musicItems[i])
			updateMusicItem(i);
}

} // End of namespace Neverhood
