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

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_files.h"
#include "scumm/file.h"

namespace Scumm {

IMuseDigiFilesHandler::IMuseDigiFilesHandler(IMuseDigital *engine, ScummEngine_v7 *vm) {
	_engine = engine;
	_sound = new ImuseDigiSndMgr(vm);
	assert(_sound);
	_vm = vm;

	_ftSpeechFilename[0] = '\0';
	_ftSpeechSubFileOffset = 0;
	_ftSpeechFileSize = 0;
	_ftSpeechFileCurPos = 0;
	_ftSpeechFile = nullptr;

	for (int i = 0; i < 4; i++) {
		IMuseDigiSndBuffer *selectedSoundBuf = &_soundBuffers[i];
		selectedSoundBuf->buffer = nullptr;
		selectedSoundBuf->bufSize = 0;
		selectedSoundBuf->loadSize = 0;
		selectedSoundBuf->criticalSize = 0;
	}
}

IMuseDigiFilesHandler::~IMuseDigiFilesHandler() {
	delete _ftSpeechFile;
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
		if (_engine->isFTSoundEngine()) {
			ser.syncAsSint32LE(_ftSpeechFileCurPos, VER(103));
			ser.syncAsSint32LE(_ftSpeechFileSize, VER(103));
			ser.syncAsSint32LE(_ftSpeechSubFileOffset, VER(103));
			ser.syncArray(_ftSpeechFilename, sizeof(_ftSpeechFilename), Common::Serializer::SByte, VER(103));
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

		if (_engine->isFTSoundEngine()) {
			ser.syncAsSint32LE(_ftSpeechFileCurPos, VER(103));
			ser.syncAsSint32LE(_ftSpeechFileSize, VER(103));
			ser.syncAsSint32LE(_ftSpeechSubFileOffset, VER(103));
			ser.syncArray(_ftSpeechFilename, sizeof(_ftSpeechFilename), Common::Serializer::SByte, VER(103));
			if (strlen(_ftSpeechFilename))
				_ftSpeechFile = _vm->_sound->restoreDiMUSESpeechFile(_ftSpeechFilename);
		}
	}
}

uint8 *IMuseDigiFilesHandler::getSoundAddrData(int soundId) {
	if (_engine->isEngineDisabled())
		return nullptr;

	// This function is always used for SFX (tracks which do not
	// have a stream pointer), hence the use of the resource address
	if (soundId != 0) {
		_vm->_res->lock(rtSound, soundId);
		byte *ptr = _vm->getResourceAddress(rtSound, soundId);
		if (!ptr) {
			_vm->_res->unlock(rtSound, soundId);
			return nullptr;
		}
		return ptr;

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
		} else if (soundId == kTalkSoundID) {
			return _ftSpeechFileSize;
		}
	} else {
		return _vm->getResourceSize(rtSound, soundId);
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
	if (s || (_engine->isFTSoundEngine() && soundId == kTalkSoundID)) {
		if (soundId != 0) {
			if (_engine->isFTSoundEngine()) {
				switch (mode) {
				case SEEK_END:
					if (soundId != kTalkSoundID) {
						return s->resSize;
					} else {
						return _ftSpeechFileSize;
					}
				case SEEK_SET:
				default:
					if (soundId != kTalkSoundID) {
						if (offset <= s->resSize) {
							s->resCurOffset = offset;
							return offset;
						}
					} else {
						if (offset <= _ftSpeechFileSize) {
							_ftSpeechFileCurPos = offset;
							return _ftSpeechFileCurPos;
						}
					}
				}
			} else {
				// A soundId > 10000 is a SAN cutscene
				if ((_vm->_game.id == GID_DIG && !(_vm->_game.features & GF_DEMO)) && (soundId > kTalkSoundID))
					return 0;

				return s->bundle->seekFile(offset, mode);
			}
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

		// We don't have SoundDesc objects for FT & DIG demo speech files
		if (_engine->isFTSoundEngine() && soundId == kTalkSoundID) {
			_ftSpeechFile->seek(_ftSpeechSubFileOffset + _ftSpeechFileCurPos, SEEK_SET);
			resultingSize = size > _ftSpeechFileSize ? (_ftSpeechFileSize - _ftSpeechFileCurPos) : size;
			return _ftSpeechFile->read(buf, resultingSize);
		}

		char fileName[60] = "";
		getFilenameFromSoundId(soundId, fileName, sizeof(fileName));

		ImuseDigiSndMgr::SoundDesc *s = _sound->getSounds();
		ImuseDigiSndMgr::SoundDesc *curSnd = nullptr;
		for (int i = 0; i < MAX_IMUSE_SOUNDS; i++) {
			curSnd = &s[i];
			if (curSnd->inUse) {
				if (curSnd->soundId == soundId) {
					if (_engine->isFTSoundEngine()) { // FT & DIG demo
						resultingSize = size > (curSnd->resSize - curSnd->resCurOffset) ? (curSnd->resSize - curSnd->resCurOffset) : size;
						tmpBuf = &curSnd->resPtr[curSnd->resCurOffset];

						if (resultingSize != size)
							debug(5, "IMuseDigiFilesHandler::read(): WARNING: tried to read %d bytes, got %d instead (soundId %d (%s))", size, resultingSize, soundId, fileName);

						memcpy(buf, tmpBuf, resultingSize); // We don't free tmpBuf: it's the resource pointer
						return resultingSize;
					} else { // DIG & COMI
						resultingSize = curSnd->bundle->readFile(fileName, size, &tmpBuf, ((_vm->_game.id == GID_CMI) && !(_vm->_game.features & GF_DEMO)));

						if (resultingSize != size)
							debug(5, "IMuseDigiFilesHandler::read(): WARNING: tried to read %d bytes, got %d instead (soundId %d (%s))", size, resultingSize, soundId, fileName);

						memcpy(buf, tmpBuf, resultingSize);
						free(tmpBuf);
						return resultingSize;
					}
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
	if (!_engine->isFTSoundEngine()) {
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
	} else {
		s = _sound->findSoundById(soundId);
		if (!s)
			s = _sound->openSound(soundId, "", IMUSE_RESOURCE, -1, -1);
		if (!s)
			s = _sound->openSound(soundId, "", IMUSE_RESOURCE, -1, 1);
		if (!s)
			s = _sound->openSound(soundId, "", IMUSE_RESOURCE, -1, 2);
		if (!s) {
			debug(5, "IMuseDigiFilesHandler::openSound(): can't open sound %d", soundId);
			return 1;
		}
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
			closeSound((&s[i])->soundId);
		}
	}

	_engine->flushTracks();
}

void IMuseDigiFilesHandler::getFilenameFromSoundId(int soundId, char *fileName, size_t size) {
	if (_engine->isFTSoundEngine())
		return;

	int i = 0;

	if (soundId == kTalkSoundID) {
		Common::strlcpy(fileName, _currentSpeechFilename, size);
		return;
	}

	if (_vm->_game.id == GID_CMI) {
		if (_vm->_game.features & GF_DEMO) {
			while (_comiDemoStateMusicTable[i].soundId != -1) {
				if (_comiDemoStateMusicTable[i].soundId == soundId) {
					Common::strlcpy(fileName, _comiDemoStateMusicTable[i].filename, size);
					return;
				}
				i++;
			}
		} else {
			if (soundId < 2000) {
				while (_comiStateMusicTable[i].soundId != -1) {
					if (_comiStateMusicTable[i].soundId == soundId) {
						Common::strlcpy(fileName, _comiStateMusicTable[i].filename, size);
						return;
					}
					i++;
				}
			} else {
				while (_comiSeqMusicTable[i].soundId != -1) {
					if (_comiSeqMusicTable[i].soundId == soundId) {
						Common::strlcpy(fileName, _comiSeqMusicTable[i].filename, size);
						return;
					}
					i++;
				}
			}
		}
	} else if (_vm->_game.id == GID_DIG) {
		if (soundId < 2000) {
			while (_digStateMusicTable[i].soundId != -1) {
				if (_digStateMusicTable[i].soundId == soundId) {
					Common::strlcpy(fileName, _digStateMusicTable[i].filename, size);
					return;
				}
				i++;
			}
		} else {
			while (_digSeqMusicTable[i].soundId != -1) {
				if (_digSeqMusicTable[i].soundId == soundId) {
					Common::strlcpy(fileName, _digSeqMusicTable[i].filename, size);
					return;
				}
				i++;
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

void IMuseDigiFilesHandler::setCurrentFtSpeechFile(const char *fileName, ScummFile *file, uint32 offset, uint32 size) {
	Common::strlcpy(_ftSpeechFilename, fileName, sizeof(_ftSpeechFilename));
	delete _ftSpeechFile;
	_ftSpeechFile = file;
	_ftSpeechSubFileOffset = offset;
	_ftSpeechFileSize = size;
}

void IMuseDigiFilesHandler::closeSoundImmediatelyById(int soundId) {
	if (_engine->isEngineDisabled())
		return;

	_sound->closeSoundById(soundId);
}

} // End of namespace Scumm
