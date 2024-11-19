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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/math/vector2.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/save_thumb_helper.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/wintermute.h"
#include "graphics/scaler.h"
#include "image/bmp.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/savefile.h"

namespace Wintermute {

// The original WME-Lite savegames had the following:
//#define SAVE_MAGIC      0x45564153
//#define SAVE_MAGIC_2    0x32564153
// In case anyone tries to load original savegames, or for that matter
// in case we ever want to attempt to support original savegames, we
// avoid those numbers, and use this instead:
#define SAVE_MAGIC_3    0x12564154

//////////////////////////////////////////////////////////////////////////
BasePersistenceManager::BasePersistenceManager(const Common::String &savePrefix, bool deleteSingleton) {
	_saving = false;
	_offset = 0;
	_saveStream = nullptr;
	_loadStream = nullptr;
	_deleteSingleton = deleteSingleton;
	if (BaseEngine::instance().getGameRef()) {
		_gameRef = BaseEngine::instance().getGameRef();
	} else {
		_gameRef = nullptr;
	}

	_richBuffer = nullptr;
	_richBufferSize = 0;

	_scummVMThumbnailData = nullptr;
	_scummVMThumbSize = 0;

	_savedDescription = nullptr;
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_savedTimestamp.tm_sec = 0;
	_savedTimestamp.tm_min = 0;
	_savedTimestamp.tm_hour = 0;
	_savedTimestamp.tm_mday = 0;
	_savedTimestamp.tm_mon = 0;
	_savedTimestamp.tm_year = 0;
	_savedTimestamp.tm_wday = 0;

	_savedPlayTime = 0;

	_thumbnailDataSize = 0;
	_thumbnailData = nullptr;
	if (savePrefix != "") {
		_savePrefix = savePrefix;
	} else if (_gameRef) {
		_savePrefix = _gameRef->getGameTargetName();
	} else {
		_savePrefix = "wmesav";
	}
}


//////////////////////////////////////////////////////////////////////////
BasePersistenceManager::~BasePersistenceManager() {
	cleanup();
	if (_deleteSingleton && BaseEngine::instance().getGameRef() == nullptr)
		BaseEngine::destroy();
}


//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::cleanup() {
	_offset = 0;

	delete[] _richBuffer;
	_richBuffer = nullptr;
	_richBufferSize = 0;

	delete[] _savedDescription;
	_savedDescription = nullptr; // ref to buffer
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	if (_thumbnailData) {
		delete[] _thumbnailData;
		_thumbnailData = nullptr;
	}

	_scummVMThumbSize = 0;
	if (_scummVMThumbnailData) {
		delete[] _scummVMThumbnailData;
		_scummVMThumbnailData = nullptr;
	}

	delete _loadStream;
	delete _saveStream;
	_loadStream = nullptr;
	_saveStream = nullptr;
}

Common::String BasePersistenceManager::getFilenameForSlot(int slot) const {
	// 3 Digits, to allow for one save-slot for autosave + slot 1 - 100 (which will be numbered 0-99 filename-wise)
	return Common::String::format("%s.%03d", _savePrefix.c_str(), slot);
}

void BasePersistenceManager::getSaveStateDesc(int slot, SaveStateDescriptor &desc) {
	Common::String filename = getFilenameForSlot(slot);
	debugC(kWintermuteDebugSaveGame, "Trying to list savegame %s in slot %d", filename.c_str(), slot);
	if (DID_FAIL(readHeader(filename))) {
		debugC(kWintermuteDebugSaveGame, "getSavedDesc(%d) - Failed for %s", slot, filename.c_str());
		return;
	}
	desc.setSaveSlot(slot);
	desc.setDescription(_savedDescription);
	desc.setDeletableFlag(true);
	desc.setWriteProtectedFlag(false);

	int thumbSize = 0;
	byte *thumbData = nullptr;
	if (_scummVMThumbSize > 0) {
		thumbSize = _scummVMThumbSize;
		thumbData = _scummVMThumbnailData;
	} else if (_thumbnailDataSize > 0) {
		thumbSize = _thumbnailDataSize;
		thumbData = _thumbnailData;
	}

	if (thumbSize > 0) {
		Common::MemoryReadStream thumbStream(thumbData, thumbSize, DisposeAfterUse::NO);
		Image::BitmapDecoder bmpDecoder;
		if (bmpDecoder.loadStream(thumbStream)) {
			const Graphics::Surface *bmpSurface = bmpDecoder.getSurface();
			Graphics::Surface *scaled = bmpSurface->scale(kThumbnailWidth, kThumbnailHeight2);
			Graphics::Surface *thumb = scaled->convertTo(g_system->getOverlayFormat());
			desc.setThumbnail(thumb);
			scaled->free();
			delete scaled;
		}
	}

	desc.setSaveDate(_savedTimestamp.tm_year + 1900, _savedTimestamp.tm_mon + 1, _savedTimestamp.tm_mday);
	desc.setSaveTime(_savedTimestamp.tm_hour, _savedTimestamp.tm_min);
	desc.setPlayTime(0);
}

bool BasePersistenceManager::deleteSaveSlot(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

uint32 BasePersistenceManager::getMaxUsedSlot() {
	Common::String saveMask = Common::String::format("%s.???", _savePrefix.c_str());
	Common::StringArray saves = g_system->getSavefileManager()->listSavefiles(saveMask);
	Common::StringArray::iterator it = saves.begin();
	int ret = -1;
	for (; it != saves.end(); ++it) {
		int num = -1;
		sscanf(it->c_str(), ".%d", &num);
		ret = MAX(ret, num);
	}
	return ret;
}

bool BasePersistenceManager::getSaveExists(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	if (DID_FAIL(readHeader(filename))) {
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::initSave(const Common::String &desc) {
	if (desc == "") {
		return STATUS_FAILED;
	}

	cleanup();
	_saving = true;

	_saveStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	if (_saveStream) {
		// get thumbnails
		if (!_gameRef->_cachedThumbnail) {
			_gameRef->_cachedThumbnail = new SaveThumbHelper(_gameRef);
			if (DID_FAIL(_gameRef->_cachedThumbnail->storeThumbnail(true))) {
				delete _gameRef->_cachedThumbnail;
				_gameRef->_cachedThumbnail = nullptr;
			}
		}

		uint32 magic = DCGF_MAGIC;
		putDWORD(magic);

		magic = SAVE_MAGIC_3;
		putDWORD(magic);

		byte verMajor, verMinor, extMajor, extMinor;
		_gameRef->getVersion(&verMajor, &verMinor, &extMajor, &extMinor);
		_saveStream->writeByte(verMajor);
		_saveStream->writeByte(verMinor);
		_saveStream->writeByte(extMajor);
		_saveStream->writeByte(extMinor);

		// new in ver 2
		putDWORD((uint32)DCGF_VER_BUILD);
		putString(_gameRef->getName());

		// thumbnail data size
		bool thumbnailOK = false;

		if (_gameRef->_cachedThumbnail) {
			if (_gameRef->_cachedThumbnail->_thumbnail) {
				Common::MemoryWriteStreamDynamic thumbStream(DisposeAfterUse::YES);
				if (_gameRef->_cachedThumbnail->_thumbnail->writeBMPToStream(&thumbStream)) {
					_saveStream->writeUint32LE(thumbStream.size());
					_saveStream->write(thumbStream.getData(), thumbStream.size());
				} else {
					_saveStream->writeUint32LE(0);
				}

				thumbnailOK = true;
			}
		}
		if (!thumbnailOK) {
			putDWORD(0);
		}
		thumbnailOK = false;
		// Again for the ScummVM-thumb:
		if (_gameRef->_cachedThumbnail) {
			if (_gameRef->_cachedThumbnail->_scummVMThumb) {
				Common::MemoryWriteStreamDynamic scummVMthumbStream(DisposeAfterUse::YES);
				if (_gameRef->_cachedThumbnail->_scummVMThumb->writeBMPToStream(&scummVMthumbStream)) {
					_saveStream->writeUint32LE(scummVMthumbStream.size());
					_saveStream->write(scummVMthumbStream.getData(), scummVMthumbStream.size());
				} else {
					_saveStream->writeUint32LE(0);
				}

				thumbnailOK = true;
			}
		}
		if (!thumbnailOK) {
			putDWORD(0);
		}


		// in any case, destroy the cached thumbnail once used
		delete _gameRef->_cachedThumbnail;
		_gameRef->_cachedThumbnail = nullptr;

		uint32 dataOffset = _offset +
		                    sizeof(uint32) + // data offset
		                    sizeof(uint32) + strlen(desc.c_str()) + 1 + // description
		                    sizeof(uint32); // timestamp

		putDWORD(dataOffset);
		putString(desc.c_str());

		g_system->getTimeAndDate(_savedTimestamp);
		putTimeDate(_savedTimestamp);
		_savedPlayTime = g_system->getMillis();
		_saveStream->writeUint32LE(_savedPlayTime);
	}
	return STATUS_OK;
}

bool BasePersistenceManager::readHeader(const Common::String &filename) {
	cleanup();

	_saving = false;

	_loadStream = g_system->getSavefileManager()->openForLoading(filename);

	if (_loadStream) {
		uint32 magic;
		magic = getDWORD();

		if (magic != DCGF_MAGIC) {
			cleanup();
			return STATUS_FAILED;
		}

		magic = getDWORD();

		if (magic == SAVE_MAGIC_3) {
			_savedVerMajor = _loadStream->readByte();
			_savedVerMinor = _loadStream->readByte();
			_savedExtMajor = _loadStream->readByte();
			_savedExtMinor = _loadStream->readByte();

			_savedVerBuild = (byte)getDWORD();
			_savedName = getStringObj();

			// load thumbnail
			_thumbnailDataSize = getDWORD();
			if (_thumbnailDataSize > 0) {
				_thumbnailData = new byte[_thumbnailDataSize];
				if (_thumbnailData) {
					getBytes(_thumbnailData, _thumbnailDataSize);
				} else {
					_thumbnailDataSize = 0;
				}
			}

			_scummVMThumbSize = getDWORD();
			_scummVMThumbnailData = new byte[_scummVMThumbSize];
			if (_scummVMThumbnailData) {
				getBytes(_scummVMThumbnailData, _scummVMThumbSize);
			} else {
				_scummVMThumbSize = 0;
			}

			uint32 dataOffset = getDWORD();

			_savedDescription = getString();
			_savedTimestamp = getTimeDate();
			_savedPlayTime = _loadStream->readUint32LE();

			_offset = dataOffset;

			return STATUS_OK;
		}
	}

	cleanup();
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::initLoad(const Common::String &filename) {
	if (DID_FAIL(readHeader(filename))) {
		cleanup();
		return STATUS_FAILED;
	}
	_saving = false;

	if (_savedName == "" || scumm_stricmp(_savedName.c_str(), _gameRef->getName()) != 0) {
		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game name doesn't match current game");
		cleanup();
		return STATUS_FAILED;
	}

	// if save is newer version than we are, fail
	if (_savedVerMajor >  DCGF_VER_MAJOR ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor >  DCGF_VER_MINOR) ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor == DCGF_VER_MINOR && _savedVerBuild > DCGF_VER_BUILD)
	   ) {

		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game version is newer than current game");
		debugC(kWintermuteDebugSaveGame, "ERROR: Expected %d.%d.%d got %d.%d.%d", DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, _savedVerMajor, _savedVerMinor, _savedVerBuild);
		cleanup();
		return STATUS_FAILED;
	}

	// if save is older than the minimal version we support
	if (_savedVerMajor <  SAVEGAME_VER_MAJOR ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor <  SAVEGAME_VER_MINOR) ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor == SAVEGAME_VER_MINOR && _savedVerBuild < SAVEGAME_VER_BUILD)
	   ) {
		debugC(kWintermuteDebugSaveGame, "ERROR: Saved game is too old and cannot be used by this version of game engine");
		debugC(kWintermuteDebugSaveGame, "ERROR: Expected %d.%d.%d got %d.%d.%d", DCGF_VER_MAJOR, DCGF_VER_MINOR, DCGF_VER_BUILD, _savedVerMajor, _savedVerMinor, _savedVerBuild);
		cleanup();
		return STATUS_FAILED;

	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::saveFile(const Common::String &filename) {
	byte *prefixBuffer = _richBuffer;
	uint32 prefixSize = _richBufferSize;
	byte *buffer = ((Common::MemoryWriteStreamDynamic *)_saveStream)->getData();
	uint32 bufferSize = ((Common::MemoryWriteStreamDynamic *)_saveStream)->size();

	Common::SaveFileManager *saveMan = ((WintermuteEngine *)g_engine)->getSaveFileMan();
	Common::OutSaveFile *file = saveMan->openForSaving(filename);
	file->write(prefixBuffer, prefixSize);
	file->write(buffer, bufferSize);
	bool retVal = !file->err();
	file->finalize();
	delete file;
	return retVal;
}


//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::putBytes(byte *buffer, uint32 size) {
	_saveStream->write(buffer, size);
	if (_saveStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::getBytes(byte *buffer, uint32 size) {
	_loadStream->read(buffer, size);
	if (_loadStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::putDWORD(uint32 val) {
	_saveStream->writeUint32LE(val);
}


//////////////////////////////////////////////////////////////////////////
uint32 BasePersistenceManager::getDWORD() {
	uint32 ret = _loadStream->readUint32LE();
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void BasePersistenceManager::putString(const char *val) {
	if (!val) {
		_saveStream->writeUint32LE(0);
		return;
	}

	uint32 len = strlen(val);

	_saveStream->writeUint32LE(len + 1);
	_saveStream->write(val, len);
}

Common::String BasePersistenceManager::getStringObj() {
	return getString();
}

//////////////////////////////////////////////////////////////////////////
char *BasePersistenceManager::getString() {
	uint32 len = _loadStream->readUint32LE();

	if (checkVersion(1,2,2)) {
		// Version 1.2.2 and above: len == strlen() + 1, NULL has len == 0

		if (len == 0)
			return nullptr;

		char *ret = new char[len];
		_loadStream->read(ret, len - 1);
		ret[len - 1] = '\0';

		return ret;

	} else {

		// Version 1.2.1 and older: NULL strings are represented as "(null)"
		char *ret = new char[len + 1];
		_loadStream->read(ret, len);
		ret[len] = '\0';

		if (!strcmp(ret, "(null)")) {
			delete[] ret;
			return nullptr;
		}

		return ret;
	}

}

bool BasePersistenceManager::putTimeDate(const TimeDate &t) {
	_saveStream->writeSint32LE(t.tm_sec);
	_saveStream->writeSint32LE(t.tm_min);
	_saveStream->writeSint32LE(t.tm_hour);
	_saveStream->writeSint32LE(t.tm_mday);
	_saveStream->writeSint32LE(t.tm_mon);
	_saveStream->writeSint32LE(t.tm_year);
	_saveStream->writeSint32LE(t.tm_wday);

	if (_saveStream->err()) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

TimeDate BasePersistenceManager::getTimeDate() {
	TimeDate t;
	t.tm_sec = _loadStream->readSint32LE();
	t.tm_min = _loadStream->readSint32LE();
	t.tm_hour = _loadStream->readSint32LE();
	t.tm_mday = _loadStream->readSint32LE();
	t.tm_mon = _loadStream->readSint32LE();
	t.tm_year = _loadStream->readSint32LE();
	t.tm_wday = _loadStream->readSint32LE();
	return t;
}

void BasePersistenceManager::putFloat(float val) {
	int exponent = 0;
	float significand = frexp(val, &exponent);
	Common::String str = Common::String::format("FS%f", significand);
	putString(str.c_str());
	_saveStream->writeSint32LE(exponent);
}

float BasePersistenceManager::getFloat() {
	char *str = getString();
	float value = 0.0f;
	float significand = 0.0f;
	int32 exponent = _loadStream->readSint32LE();
	int ret = sscanf(str, "FS%f", &significand);
	value = ldexp(significand, exponent);
	if (ret != 1) {
		warning("%s not parsed as float", str);
	}
	delete[] str;
	return value;
}

void BasePersistenceManager::putDouble(double val) {
	int exponent = 0;
	double significand = frexp(val, &exponent);
	Common::String str = Common::String::format("DS%f", significand);
	putString(str.c_str());
	_saveStream->writeSint32LE(exponent);
}

double BasePersistenceManager::getDouble() {
	char *str = getString();
	double value = 0.0f;
	float significand = 0.0f;
	int32 exponent = _loadStream->readSint32LE();
	int ret = sscanf(str, "DS%f", &significand);
	value = ldexp(significand, exponent);
	if (ret != 1) {
		warning("%s not parsed as double", str);
	}
	delete[] str;
	return value;
}

//////////////////////////////////////////////////////////////////////////
// bool
bool BasePersistenceManager::transferBool(const char *name, bool *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// int
bool BasePersistenceManager::transferSint32(const char *name, int32 *val) {
	if (_saving) {
		_saveStream->writeSint32LE(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// DWORD
bool BasePersistenceManager::transferUint32(const char *name, uint32 *val) {
	if (_saving) {
		_saveStream->writeUint32LE(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readUint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// float
bool BasePersistenceManager::transferFloat(const char *name, float *val) {
	if (_saving) {
		putFloat(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = getFloat();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// double
bool BasePersistenceManager::transferDouble(const char *name, double *val) {
	if (_saving) {
		putDouble(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = getDouble();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// char*
bool BasePersistenceManager::transferCharPtr(const char *name, char **val) {
	if (_saving) {
		putString(*val);
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		*val = str;
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// const char*
bool BasePersistenceManager::transferConstChar(const char *name, const char **val) {
	if (_saving) {
		putString(*val);
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		*val = str;
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Common::String
bool BasePersistenceManager::transferString(const char *name, Common::String *val) {
	if (_saving) {
		putString(val->c_str());
		return STATUS_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return STATUS_FAILED;
		}
		if (str) {
			*val = str;
			delete[] str;
		} else {
			*val = "";
		}
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// BYTE
bool BasePersistenceManager::transferByte(const char *name, byte *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// RECT
bool BasePersistenceManager::transferRect32(const char *name, Rect32 *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->left);
		_saveStream->writeSint32LE(val->top);
		_saveStream->writeSint32LE(val->right);
		_saveStream->writeSint32LE(val->bottom);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->left = _loadStream->readSint32LE();
		val->top = _loadStream->readSint32LE();
		val->right = _loadStream->readSint32LE();
		val->bottom = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// POINT
bool BasePersistenceManager::transferPoint32(const char *name, Point32 *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->x);
		_saveStream->writeSint32LE(val->y);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->x = _loadStream->readSint32LE();
		val->y = _loadStream->readSint32LE();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// Vector2
bool BasePersistenceManager::transferVector2(const char *name, Vector2 *val) {
	if (_saving) {
		putFloat(val->x);
		putFloat(val->y);
		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	} else {
		val->x = getFloat();
		val->y = getFloat();
		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Vector3
bool BasePersistenceManager::transferVector3d(const char *name, DXVector3 *val) {
	if (_saving) {
		putFloat(val->_x);
		putFloat(val->_y);
		putFloat(val->_z);

		if (_saveStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	} else {
		val->_x = getFloat();
		val->_y = getFloat();
		val->_z = getFloat();

		if (_loadStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Vector4
bool BasePersistenceManager::transferVector4d(const char *name, DXVector4 *val) {
	if (_saving) {
		putFloat(val->_x);
		putFloat(val->_y);
		putFloat(val->_z);
		putFloat(val->_w);

		if (_saveStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	} else {
		val->_x = getFloat();
		val->_y = getFloat();
		val->_z = getFloat();
		val->_w = getFloat();

		if (_loadStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Matrix4
bool BasePersistenceManager::transferMatrix4(const char *name, DXMatrix *val) {
	if (_saving) {
		for (int i = 0; i < 16; ++i) {
			putFloat(val->_m4x4[i]);
		}

		if (_saveStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	} else {
		for (int i = 0; i < 16; ++i) {
			val->_m4x4[i] = getFloat();
		}

		if (_loadStream->err()) {
			return STATUS_FAILED;
		}

		return STATUS_OK;
	}
}

bool BasePersistenceManager::transferAngle(const char *name, float *val) {
	if (_saving) {
		putFloat(*val);

		if (_saveStream->err()) {
			return STATUS_FAILED;
		}
	} else {
		*val = getFloat();

		if (_loadStream->err()) {
			return STATUS_FAILED;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// generic pointer

bool BasePersistenceManager::transferPtr(const char *name, void *val) {
	int classID = -1, instanceID = -1;

	if (_saving) {
		SystemClassRegistry::getInstance()->getPointerID(*(void **)val, &classID, &instanceID);
		if (*(void **)val != nullptr && (classID == -1 || instanceID == -1)) {
			debugC(kWintermuteDebugSaveGame, "Warning: invalid instance '%s'", name);
		}

		_saveStream->writeUint32LE(classID);
		_saveStream->writeUint32LE(instanceID);
	} else {
		classID = _loadStream->readUint32LE();
		instanceID = _loadStream->readUint32LE();

		*(void **)val = SystemClassRegistry::getInstance()->idToPointer(classID, instanceID);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BasePersistenceManager::checkVersion(byte verMajor, byte verMinor, byte verBuild) {
	if (_saving) {
		return true;
	}

	// it's ok if we are same or newer than the saved game
	if (verMajor >  _savedVerMajor ||
	        (verMajor == _savedVerMajor && verMinor >  _savedVerMinor) ||
	        (verMajor == _savedVerMajor && verMinor == _savedVerMinor && verBuild > _savedVerBuild)
	   ) {
		return false;
	}

	return true;
}

} // End of namespace Wintermute
