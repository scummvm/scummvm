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

#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/resource.h"

namespace Lab {

#define CLOWNROOM           123
#define DIMROOM              80

Music::Music(LabEngine *vm) : _vm(vm) {
	_musicFile = nullptr;
	_storedPos = 0;
}

byte Music::getSoundFlags() {
	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else if (_vm->getPlatform() == Common::kPlatformDOS)
		soundFlags |= Audio::FLAG_UNSIGNED;

	return soundFlags;
}

void Music::loadSoundEffect(const Common::String filename, bool loop, bool waitTillFinished) {
	stopSoundEffect();

	Common::File *file = _vm->_resource->openDataFile(filename, MKTAG('D', 'I', 'F', 'F'));
	if (!file)
		return;

	_vm->_anim->_doBlack = false;

	uint32 magicBytes = file->readUint32LE();
	if (magicBytes != 1219009121) {
		warning("readSound: Bad signature, skipping");
		return;
	}
	uint32 soundTag = file->readUint32LE();
	uint32 soundSize = file->readUint32LE();

	if (soundTag != 0)
		return;

	file->skip(soundSize);	// skip the header

	while (soundTag != 65535) {
		_vm->updateEvents();
		soundTag = file->readUint32LE();
		soundSize = file->readUint32LE() - 8;

		if ((soundTag == 30) || (soundTag == 31)) {
			if (waitTillFinished) {
				while (isSoundEffectActive()) {
					_vm->updateEvents();
					_vm->waitTOF();
				}
			}

			file->skip(4);

			uint16 sampleRate = file->readUint16LE();
			file->skip(2);
			playSoundEffect(sampleRate, soundSize, loop, file);
		} else if (soundTag == 65535) {
			if (waitTillFinished) {
				while (isSoundEffectActive()) {
					_vm->updateEvents();
					_vm->waitTOF();
				}
			}
		} else
			file->skip(soundSize);
	}
}

void Music::playSoundEffect(uint16 sampleSpeed, uint32 length, bool loop, Common::File *dataFile) {
	stopSoundEffect();

	// NOTE: We need to use malloc(), cause this will be freed with free()
	// by the music code
	byte *soundData = (byte *)malloc(length);
	dataFile->read(soundData, length);

	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream((const byte *)soundData, length, MAX<uint16>(sampleSpeed, 4000), getSoundFlags());
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, new Audio::LoopingAudioStream(audioStream, (loop) ? 0 : 1));
}

void Music::stopSoundEffect() {
	if (isSoundEffectActive())
		_vm->_mixer->stopHandle(_sfxHandle);
}

bool Music::isSoundEffectActive() const {
	return _vm->_mixer->isSoundHandleActive(_sfxHandle);
}

void Music::changeMusic(const Common::String filename, bool storeCurPos, bool seektoStoredPos) {
	if (storeCurPos)
		_storedPos = _musicFile->pos();

	stopSoundEffect();
	freeMusic();
	_musicFile = _vm->_resource->openDataFile(filename);
	if (seektoStoredPos)
		_musicFile->seek(_storedPos);

	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(_musicFile, 15000, getSoundFlags());
	_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES, false));
}

void Music::resetMusic(bool seektoStoredPos) {
	if (_vm->getPlatform() != Common::kPlatformAmiga)
		changeMusic("Music:BackGrou", false, seektoStoredPos);
	else
		changeMusic("Music:BackGround", false, seektoStoredPos);
}

void Music::checkRoomMusic(uint16 prevRoom, uint16 newRoom) {
	if (newRoom == CLOWNROOM)
		changeMusic("Music:Laugh", true, false);
	else if (newRoom == DIMROOM)
		changeMusic("Music:Rm81", true, false);
	else if (prevRoom == CLOWNROOM || prevRoom == DIMROOM)
		resetMusic(true);
}

void Music::freeMusic() {
	_vm->_mixer->stopHandle(_musicHandle);
	_vm->_mixer->stopHandle(_sfxHandle);
	_musicFile = nullptr;
}

} // End of namespace Lab
