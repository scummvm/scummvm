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

#include "engines/grim/imuse/dimuse_engine.h"
#include "engines/grim/imuse/dimuse_files.h"

namespace Grim {

IMuseDigiFilesHandler::IMuseDigiFilesHandler(IMuseDigital *engine) {
	_engine = engine;
	_sound = new ImuseDigiSndMgr();
	assert(_sound);

	for (int i = 0; i < 4; i++) {
		IMuseDigiSndBuffer *selectedSoundBuf = &_soundBuffers[i];
		selectedSoundBuf->buffer = nullptr;
		selectedSoundBuf->bufSize = 0;
		selectedSoundBuf->loadSize = 0;
		selectedSoundBuf->criticalSize = 0;
	}
}

IMuseDigiFilesHandler::~IMuseDigiFilesHandler() {
	delete _sound;
}

void IMuseDigiFilesHandler::saveLoad(Common::Serializer &ser) {
	int curSound = 0;
	ImuseDigiSndMgr::SoundDesc *sounds = _sound->getSounds();

	ser.syncArray(_currentSpeechFilename, 60, Common::Serializer::SByte, VER(103));
	if (ser.isSaving()) {
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			ser.syncAsSint32LE(sounds[l].soundId, VER(103));
		}
	}

	if (ser.isLoading()) {
		// Close prior sounds if we're reloading (needed for edge cases like the recipe book in COMI)
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			_sound->closeSound(&sounds[l]);
		}

		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			ser.syncAsSint32LE(curSound, VER(103));
			if (curSound) {
				openSound(curSound);
				if (curSound != kTalkSoundID)
					closeSound(curSound);
			}
		}
	}
}

uint8 *IMuseDigiFilesHandler::getSoundAddrData(int soundId) {
	if (_engine->isEngineDisabled())
		return nullptr;

	// This function is always used for SFX (tracks which do not
	// have a stream pointer), hence the use of the resource address
	if (soundId != 0) {
		// TODO
		// _vm->_res->lock(rtSound, soundId);
		// byte *ptr = _vm->getResourceAddress(rtSound, soundId);
		// if (!ptr) {
		// 	_vm->_res->unlock(rtSound, soundId);
		// 	return nullptr;
		// }
		// return ptr;
	}
	debug(5, "IMuseDigiFilesHandler::getSoundAddrData(): soundId is 0 or out of range");
	return nullptr;
}

int IMuseDigiFilesHandler::getSoundAddrDataSize(int soundId, bool hasStream) {
	if (_engine->isEngineDisabled())
		return 0;

	if (hasStream) {
		ImuseDigiSndMgr::SoundDesc *s = _sound->findSoundById(soundId);
		if (s) {
			if (soundId != kTalkSoundID) {
				return s->resSize;
			}
		}
	} else {
		// TODO
		// return _vm->getResourceSize(rtSound, soundId);
	}

	return 0;
}

int IMuseDigiFilesHandler::getNextSound(int soundId) {
	int foundSoundId = 0;
	do {
		foundSoundId = _engine->diMUSEGetNextSound(foundSoundId);
		if (!foundSoundId)
			return -1;
	} while (foundSoundId != soundId);
	return 2;
}

int IMuseDigiFilesHandler::seek(int soundId, int32 offset, int mode, int bufId) {
	// This function and files_read() are used for sounds for which a stream is needed (speech
	// and music), therefore they will always refer to sounds in a bundle file for DIG and COMI
	// The seek'd position is in reference to the decompressed sound

	if (_engine->isEngineDisabled())
		return 0;

	char fileName[60] = "";
	getFilenameFromSoundId(soundId, fileName, sizeof(fileName));

	ImuseDigiSndMgr::SoundDesc *s = _sound->findSoundById(soundId);
	if (s) {
		if (soundId != 0) {
			return s->bundle->seekFile(offset, mode);
		} else {
			debug(5, "IMuseDigiFilesHandler::seek(): soundId is 0 or out of range");
		}
	} else {
		debug(5, "IMuseDigiFilesHandler::seek(): can't find sound %d (%s); did you forget to open it?", soundId, fileName);
	}

	return 0;
}

int IMuseDigiFilesHandler::read(int soundId, uint8 *buf, int32 size, int bufId) {
	// This function and files_seek() are used for sounds for which a stream is needed (speech
	// and music), therefore they will always refer to sounds in a bundle file for DIG and COMI

	if (_engine->isEngineDisabled())
		return 0;

	if (soundId != 0) {
		uint8 *tmpBuf = nullptr;
		int32 resultingSize;

		char fileName[60] = "";
		getFilenameFromSoundId(soundId, fileName, sizeof(fileName));

		ImuseDigiSndMgr::SoundDesc *s = _sound->getSounds();
		ImuseDigiSndMgr::SoundDesc *curSnd = nullptr;
		for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
			curSnd = &s[i];
			if (curSnd->inUse) {
				if (curSnd->soundId == soundId) {
					resultingSize = curSnd->bundle->readFile(fileName, size, &tmpBuf, true);

					if (resultingSize != size)
						debug(5, "IMuseDigiFilesHandler::read(): WARNING: tried to read %d bytes, got %d instead (soundId %d (%s))", size, resultingSize, soundId, fileName);

					memcpy(buf, tmpBuf, resultingSize);
					free(tmpBuf);
					return resultingSize;
				}
			}
		}

		debug(5, "IMuseDigiFilesHandler::read(): can't find sound %d (%s); did you forget to open it?", soundId, fileName);

	} else {
		debug(5, "IMuseDigiFilesHandler::read(): soundId is 0 or out of range");
	}

	return 0;
}

IMuseDigiSndBuffer *IMuseDigiFilesHandler::getBufInfo(int bufId) {
	if (bufId > 0 && bufId <= 4) {
		return &_soundBuffers[bufId];
	}

	debug(5, "IMuseDigiFilesHandler::getBufInfo(): ERROR: invalid buffer id");
	return nullptr;
}

int IMuseDigiFilesHandler::openSound(int soundId) {
	if (_engine->isEngineDisabled())
		return 1;

	ImuseDigiSndMgr::SoundDesc *s = nullptr;
	char fileName[60] = "";
	getFilenameFromSoundId(soundId, fileName, sizeof(fileName));

	int groupId = soundId == kTalkSoundID ? IMUSE_VOLGRP_VOICE : IMUSE_VOLGRP_MUSIC;
	s = _sound->findSoundById(soundId);
	if (!s)
		s = _sound->openSound(soundId, fileName, IMUSE_BUNDLE, groupId, -1);
	if (!s)
		s = _sound->openSound(soundId, fileName, IMUSE_BUNDLE, groupId, 1);
	if (!s)
		s = _sound->openSound(soundId, fileName, IMUSE_BUNDLE, groupId, 2);
	if (!s) {
		debug(5, "IMuseDigiFilesHandler::openSound(): can't open sound %d (%s)", soundId, fileName);
		return 1;
	}

	return 0;
}

void IMuseDigiFilesHandler::closeSound(int soundId) {
	if (_engine->isEngineDisabled())
		return;

	_sound->scheduleSoundForDeallocation(soundId);
}

void IMuseDigiFilesHandler::closeAllSounds() {
	ImuseDigiSndMgr::SoundDesc *s = _sound->getSounds();
	for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
		if (s[i].inUse) {
			closeSound(s[i].soundId);
		}
	}

	_engine->flushTracks();
}

void IMuseDigiFilesHandler::getFilenameFromSoundId(int soundId, char *fileName, size_t size) {
	int i = 0;

	if (soundId == kTalkSoundID) {
		Common::strlcpy(fileName, _currentSpeechFilename, size);
		return;
	}

	// TODO
	if (soundId < 2000) {
		for (i = 0; i < ARRAYSIZE(_grimStateMusicTable); i++) {
			if (_grimStateMusicTable[i].soundId == soundId) {
				Common::strlcpy(fileName, _grimStateMusicTable[i].name, size);
				Common::strlcat(fileName, ".IMC", size);
				return;
			}
		}
	} else {
		for (i = 0; i < ARRAYSIZE(_grimSeqMusicTable); i++) {
			if (_grimSeqMusicTable[i].soundId == soundId) {
				Common::strlcpy(fileName, _grimSeqMusicTable[i].name, size);
				Common::strlcat(fileName, ".IMC", size);
				return;
			}
		}
	}
}

void IMuseDigiFilesHandler::allocSoundBuffer(int bufId, int32 size, int32 loadSize, int32 criticalSize) {
	IMuseDigiSndBuffer *selectedSoundBuf;

	selectedSoundBuf = &_soundBuffers[bufId];
	selectedSoundBuf->buffer = (uint8 *)malloc(size);
	selectedSoundBuf->bufSize = size;
	selectedSoundBuf->loadSize = loadSize;
	selectedSoundBuf->criticalSize = criticalSize;
}

void IMuseDigiFilesHandler::deallocSoundBuffer(int bufId) {
	IMuseDigiSndBuffer *selectedSoundBuf;

	selectedSoundBuf = &_soundBuffers[bufId];
	free(selectedSoundBuf->buffer);
	selectedSoundBuf->buffer = nullptr;
}

void IMuseDigiFilesHandler::flushSounds() {
	if (_engine->isEngineDisabled())
		return;

	ImuseDigiSndMgr::SoundDesc *s = _sound->getSounds();
	for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
		ImuseDigiSndMgr::SoundDesc *curSnd = &s[i];
		if (curSnd && curSnd->inUse) {
			if (curSnd->scheduledForDealloc)
				if (!_engine->diMUSEGetParam(curSnd->soundId, DIMUSE_P_SND_TRACK_NUM) && !_engine->diMUSEGetParam(curSnd->soundId, DIMUSE_P_TRIGS_SNDS))
					_sound->closeSound(curSnd);
		}
	}
}

int IMuseDigiFilesHandler::setCurrentSpeechFilename(const char *fileName) {
	Common::strlcpy(_currentSpeechFilename, fileName, sizeof(_currentSpeechFilename));
	if (openSound(kTalkSoundID))
		return 1;

	return 0;
}

void IMuseDigiFilesHandler::closeSoundImmediatelyById(int soundId) {
	if (_engine->isEngineDisabled())
		return;

	_sound->closeSoundById(soundId);
}

} // End of namespace Grim
