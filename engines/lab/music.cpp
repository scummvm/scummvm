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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "audio/decoders/raw.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/resource.h"

namespace Lab {

#define MUSICBUFSIZE   (2 * 65536L)
#define SAMPLESPEED    15000L

#define CLOWNROOM           123
#define DIMROOM              80

Music::Music(LabEngine *vm) : _vm(vm) {
	_file = 0;
	_tFile = 0;
	_musicPaused = false;

	_oldMusicOn = false;
	_tLeftInFile = 0;

	_leftInFile = 0;

	_musicOn = false;
	_loopSoundEffect = false;
	_queuingAudioStream = nullptr;
	_lastMusicRoom = 1;
	_doReset = true;
}

void Music::updateMusic() {
	if (!_musicOn || (getPlayingBufferCount() >= MAXBUFFERS))
		return;

	// NOTE: We need to use malloc(), cause this will be freed with free()
	// by the music code
	byte *musicBuffer = (byte *)malloc(MUSICBUFSIZE);
	fillbuffer(musicBuffer);

	// Queue a music block, and start the music, if needed
	bool startMusicFlag = false;

	if (!_queuingAudioStream) {
		_queuingAudioStream = Audio::makeQueuingAudioStream(SAMPLESPEED, false);
		startMusicFlag = true;
	}

	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else if (_vm->getPlatform() == Common::kPlatformDOS)
		soundFlags |= Audio::FLAG_UNSIGNED;

	_queuingAudioStream->queueBuffer(musicBuffer, MUSICBUFSIZE, DisposeAfterUse::YES, soundFlags);

	if (startMusicFlag)
		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _queuingAudioStream);
}

uint16 Music::getPlayingBufferCount() {
	return (_queuingAudioStream) ? _queuingAudioStream->numQueuedStreams() : 0;
}

void Music::playSoundEffect(uint16 sampleSpeed, uint32 length, Common::File *dataFile) {
	pauseBackMusic();
	stopSoundEffect();

	if (sampleSpeed < 4000)
		sampleSpeed = 4000;

	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else
		soundFlags |= Audio::FLAG_UNSIGNED;

	// NOTE: We need to use malloc(), cause this will be freed with free()
	// by the music code
	byte *soundData = (byte *)malloc(length);
	dataFile->read(soundData, length);

	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream((const byte *)soundData, length, sampleSpeed, soundFlags);
	uint loops = (_loopSoundEffect) ? 0 : 1;
	Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, loops);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, loopingAudioStream);
}

void Music::stopSoundEffect() {
	if (isSoundEffectActive())
		_vm->_mixer->stopHandle(_sfxHandle);
}

bool Music::isSoundEffectActive() const {
	return _vm->_mixer->isSoundHandleActive(_sfxHandle);
}

void Music::fillbuffer(byte *musicBuffer) {
	if (MUSICBUFSIZE < _leftInFile) {
		_file->read(musicBuffer, MUSICBUFSIZE);
		_leftInFile -= MUSICBUFSIZE;
	} else {
		_file->read(musicBuffer, _leftInFile);

		memset((char *)musicBuffer + _leftInFile, 0, MUSICBUFSIZE - _leftInFile);

		_file->seek(0);
		_leftInFile = _file->size();
	}
}

void Music::startMusic(bool restartFl) {
	if (!_musicOn)
		return;

	stopSoundEffect();

	if (restartFl) {
		_file->seek(0);
		_leftInFile  = _file->size();
	}

	_musicOn = true;
	_vm->updateMusicAndEvents();
}

bool Music::initMusic(const Common::String filename) {
	_musicOn = true;
	_musicPaused = false;
	_file = _vm->_resource->openDataFile(filename);
	startMusic(true);
	return true;
}

void Music::freeMusic() {
	_musicOn = false;

	_vm->_mixer->stopHandle(_musicHandle);
	_queuingAudioStream = nullptr;
	_vm->_mixer->stopHandle(_sfxHandle);

	delete _file;
	_file = nullptr;
}

void Music::pauseBackMusic() {
	if (!_musicPaused && _musicOn) {
		_vm->updateMusicAndEvents();
		_musicOn = false;
		stopSoundEffect();

		_vm->_mixer->pauseHandle(_musicHandle, true);

		_musicPaused = true;
	}
}

void Music::resumeBackMusic() {
	if (_musicPaused) {
		stopSoundEffect();
		_musicOn = true;

		_vm->_mixer->pauseHandle(_musicHandle, false);

		_vm->updateMusicAndEvents();
		_musicPaused = false;
	}
}

void Music::setMusic(bool on) {
	stopSoundEffect();

	if (on && !_musicOn) {
		_musicOn = true;
		startMusic(true);
	} else if (!on && _musicOn) {
		_musicOn = false;
		_vm->updateMusicAndEvents();
	} else
		_musicOn = on;
}

void Music::checkRoomMusic() {
	if ((_lastMusicRoom == _vm->_roomNum) || !_musicOn)
		return;

	if (_vm->_roomNum == CLOWNROOM)
		changeMusic("Music:Laugh");
	else if (_vm->_roomNum == DIMROOM)
		changeMusic("Music:Rm81");
	else if (_doReset)
		resetMusic();

	_lastMusicRoom = _vm->_roomNum;
}

void Music::changeMusic(const Common::String filename) {
	if (!_tFile) {
		_tFile = _file;
		_oldMusicOn = _musicOn;
		_tLeftInFile = _leftInFile + 65536L;

		if (_tLeftInFile > (uint32)_tFile->size())
			_tLeftInFile = _leftInFile;
	}

	_file = _vm->_resource->openDataFile(filename);
	// turn music off
	_musicOn = true;
	setMusic(false);

	// turn it back on
	_musicOn = false;
	setMusic(true);
}

void Music::resetMusic() {
	if (!_tFile)
		return;

	if (_file->isOpen())
		_file->close();

	_file      = _tFile;
	_leftInFile = _tLeftInFile;

	_file->seek(_file->size() - _leftInFile);

	_musicOn = true;
	setMusic(false);
	_vm->updateMusicAndEvents();

	if (!_oldMusicOn) {
		_tFile = 0;
		return;
	}

	_musicOn = _oldMusicOn;
	startMusic(false);

	_tFile = 0;
}

bool Music::readMusic(const Common::String filename, bool waitTillFinished) {
	Common::File *file = _vm->_resource->openDataFile(filename, MKTAG('D', 'I', 'F', 'F'));
	_vm->updateMusicAndEvents();
	if (!_loopSoundEffect)
		stopSoundEffect();

	if (!file)
		return false;

	_vm->_anim->_doBlack = false;
	readSound(waitTillFinished, file);

	return true;
}

void Music::readSound(bool waitTillFinished, Common::File *file) {
	uint32 magicBytes = file->readUint32LE();
	if (magicBytes != 1219009121L) {
		warning("readSound: Bad signature, skipping");
		return;
	}
	uint32 soundTag = file->readUint32LE();
	uint32 soundSize = file->readUint32LE();

	if (soundTag == 0)
		file->skip(soundSize);	// skip the header
	else
		return;

	while (soundTag != 65535) {
		_vm->updateMusicAndEvents();
		soundTag = file->readUint32LE();
		soundSize = file->readUint32LE() - 8;

		if ((soundTag == 30) || (soundTag == 31)) {
			if (waitTillFinished) {
				while (isSoundEffectActive()) {
					_vm->updateMusicAndEvents();
					_vm->waitTOF();
				}
			}

			file->skip(4);

			uint16 sampleRate = file->readUint16LE();
			file->skip(2);
			playSoundEffect(sampleRate, soundSize, file);
		} else if (soundTag == 65535L) {
			if (waitTillFinished) {
				while (isSoundEffectActive()) {
					_vm->updateMusicAndEvents();
					_vm->waitTOF();
				}
			}
		} else
			file->skip(soundSize);
	}
}

} // End of namespace Lab
