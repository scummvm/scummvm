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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BPersistMgr.h"
#include "engines/wintermute/Base/BSaveThumbHelper.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/math/Vector2.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/Base/BSound.h"
#include "graphics/decoders/bmp.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/savefile.h"

namespace WinterMute {

#define SAVE_BUFFER_INIT_SIZE 100000
#define SAVE_BUFFER_GROW_BY   50000

#define SAVE_MAGIC      0x45564153
#define SAVE_MAGIC_2    0x32564153

//////////////////////////////////////////////////////////////////////////
CBPersistMgr::CBPersistMgr(CBGame *inGame): CBBase(inGame) {
	_saving = false;
//	_buffer = NULL;
//	_bufferSize = 0;
	_offset = 0;
	_saveStream = NULL;
	_loadStream = NULL;

	_richBuffer = NULL;
	_richBufferSize = 0;

	_savedDescription = NULL;
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	_thumbnailData = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPersistMgr::~CBPersistMgr() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::cleanup() {
	/*  if (_buffer) {
	        if (_saving) free(_buffer);
	        else delete [] _buffer; // allocated by file manager
	    }
	    _buffer = NULL;

	    _bufferSize = 0;*/
	_offset = 0;

	delete[] _richBuffer;
	_richBuffer = NULL;
	_richBufferSize = 0;

	_savedDescription = NULL; // ref to buffer
//	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	if (_thumbnailData) {
		delete [] _thumbnailData;
		_thumbnailData = NULL;
	}

	delete _loadStream;
	delete _saveStream;
	_loadStream = NULL;
	_saveStream = NULL;
}

Common::String CBPersistMgr::getFilenameForSlot(int slot) {
	// TODO: Temporary solution until I have the namespacing sorted out
	return Common::String::format("save%03d.DirtySplitSav", slot);
}

void CBPersistMgr::getSaveStateDesc(int slot, SaveStateDescriptor &desc) {
	Common::String filename = getFilenameForSlot(slot);
	warning("Trying to list savegame %s in slot %d", filename.c_str(), slot);
	if (FAILED(readHeader(filename))) {
		warning("getSavedDesc(%d) - Failed for %s", slot, filename.c_str());
		return;
	}
	desc.setSaveSlot(slot);
	desc.setDescription(_savedDescription);
	desc.setDeletableFlag(true);
	desc.setWriteProtectedFlag(false);

	if (_thumbnailDataSize > 0) {
		Common::MemoryReadStream thumbStream(_thumbnailData, _thumbnailDataSize);
		Graphics::BitmapDecoder bmpDecoder;
		if (bmpDecoder.loadStream(thumbStream)) {
			Graphics::Surface *surf = new Graphics::Surface;
			surf = bmpDecoder.getSurface()->convertTo(g_system->getOverlayFormat());
			desc.setThumbnail(surf);
		}
	}

	desc.setSaveDate(_savedTimestamp.tm_year, _savedTimestamp.tm_mon, _savedTimestamp.tm_mday);
	desc.setSaveTime(_savedTimestamp.tm_hour, _savedTimestamp.tm_min);
	desc.setPlayTime(0);
}

void CBPersistMgr::deleteSaveSlot(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

uint32 CBPersistMgr::getMaxUsedSlot() {
	Common::StringArray saves = g_system->getSavefileManager()->listSavefiles("save???.DirtySplitSav");
	Common::StringArray::iterator it = saves.begin();
	int ret = -1;
	for (; it != saves.end(); it++) {
		int num = -1;
		sscanf(it->c_str(), "save%d", &num);
		ret = MAX(ret, num);
	}
	return ret;
}

bool CBPersistMgr::getSaveExists(int slot) {
	Common::String filename = getFilenameForSlot(slot);
	warning("Trying to list savegame %s in slot %d", filename.c_str(), slot);
	if (FAILED(readHeader(filename))) {
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::initSave(const char *desc) {
	if (!desc) return E_FAIL;

	cleanup();
	_saving = true;

	_saveStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);

	if (_saveStream) {
		// get thumbnails
		if (!Game->_cachedThumbnail) {
			Game->_cachedThumbnail = new CBSaveThumbHelper(Game);
			if (FAILED(Game->_cachedThumbnail->StoreThumbnail(true))) {
				delete Game->_cachedThumbnail;
				Game->_cachedThumbnail = NULL;
			}
		}

		uint32 magic = DCGF_MAGIC;
		putDWORD(magic);

		magic = SAVE_MAGIC_2;
		putDWORD(magic);

		byte VerMajor, VerMinor, ExtMajor, ExtMinor;
		Game->GetVersion(&VerMajor, &VerMinor, &ExtMajor, &ExtMinor);
		//uint32 Version = MAKELONG(MAKEWORD(VerMajor, VerMinor), MAKEWORD(ExtMajor, ExtMinor));
		_saveStream->writeByte(VerMajor);
		_saveStream->writeByte(VerMinor);
		_saveStream->writeByte(ExtMajor);
		_saveStream->writeByte(ExtMinor);

		// new in ver 2
		putDWORD((uint32)DCGF_VER_BUILD);
		putString(Game->_name);

		// thumbnail data size
		bool thumbnailOK = false;

		if (Game->_cachedThumbnail) {
			if (Game->_cachedThumbnail->_thumbnail) {
				Common::MemoryWriteStreamDynamic thumbStream(DisposeAfterUse::YES);
				if (Game->_cachedThumbnail->_thumbnail->writeBMPToStream(&thumbStream)) {
					_saveStream->writeUint32LE(thumbStream.size());
					_saveStream->write(thumbStream.getData(), thumbStream.size());
				} else {
					_saveStream->writeUint32LE(0);
				}

				thumbnailOK = true;
			}
		}
		if (!thumbnailOK) putDWORD(0);

		// in any case, destroy the cached thumbnail once used
		delete Game->_cachedThumbnail;
		Game->_cachedThumbnail = NULL;

		uint32 dataOffset = _offset +
		                    sizeof(uint32) + // data offset
		                    sizeof(uint32) + strlen(desc) + 1 + // description
		                    sizeof(uint32); // timestamp

		putDWORD(dataOffset);
		putString(desc);

		g_system->getTimeAndDate(_savedTimestamp);
		putTimeDate(_savedTimestamp);
		_savedPlayTime = g_system->getMillis();
		_saveStream->writeUint32LE(_savedPlayTime);
	}
	return S_OK;
}

HRESULT CBPersistMgr::readHeader(const Common::String &filename) {
	cleanup();

	_saving = false;

	_loadStream = g_system->getSavefileManager()->openForLoading(filename);
	//_buffer = Game->_fileManager->ReadWholeFile(Filename, &_bufferSize);
	if (_loadStream) {
		uint32 Magic;
		Magic = getDWORD();

		if (Magic != DCGF_MAGIC) {
			cleanup();
			return E_FAIL;
		}

		Magic = getDWORD();

		if (Magic == SAVE_MAGIC || Magic == SAVE_MAGIC_2) {
			_savedVerMajor = _loadStream->readByte();
			_savedVerMinor = _loadStream->readByte();
			_savedExtMajor = _loadStream->readByte();
			_savedExtMinor = _loadStream->readByte();

			if (Magic == SAVE_MAGIC_2) {
				_savedVerBuild = (byte)getDWORD();
				_savedName = getString();

				// load thumbnail
				_thumbnailDataSize = getDWORD();
				if (_thumbnailDataSize > 0) {
					_thumbnailData = new byte[_thumbnailDataSize];
					if (_thumbnailData) {
						getBytes(_thumbnailData, _thumbnailDataSize);
					} else _thumbnailDataSize = 0;
				}
			} else _savedVerBuild = 35; // last build with ver1 savegames

			uint32 dataOffset = getDWORD();

			_savedDescription = getString();
			_savedTimestamp = getTimeDate();
			_savedPlayTime = _loadStream->readUint32LE();

			_offset = dataOffset;

			return S_OK;
		}
	}

	cleanup();
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::initLoad(const char *filename) {


	if (FAILED(readHeader(filename))) {
		cleanup();
		return E_FAIL;
	}
	_saving = false;

	if (_savedName == "" || scumm_stricmp(_savedName.c_str(), Game->_name) != 0) {
		Game->LOG(0, "ERROR: Saved game name doesn't match current game");
		cleanup();
		return E_FAIL;
	}

	// if save is newer version than we are, fail
	if (_savedVerMajor >  DCGF_VER_MAJOR ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor >  DCGF_VER_MINOR) ||
	        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor == DCGF_VER_MINOR && _savedVerBuild > DCGF_VER_BUILD)
	   ) {
		Game->LOG(0, "ERROR: Saved game version is newer than current game");
		cleanup();
		return E_FAIL;
	}

	// if save is older than the minimal version we support
	if (_savedVerMajor <  SAVEGAME_VER_MAJOR ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor <  SAVEGAME_VER_MINOR) ||
	        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor == SAVEGAME_VER_MINOR && _savedVerBuild < SAVEGAME_VER_BUILD)
	   ) {
		Game->LOG(0, "ERROR: Saved game is too old and cannot be used by this version of game engine");
		cleanup();
		return E_FAIL;

	}

	/*
	 if ( _savedVerMajor != DCGF_VER_MAJOR || _savedVerMinor != DCGF_VER_MINOR)
	 {
	 Game->LOG(0, "ERROR: Saved game is created by other WME version");
	 goto init_fail;
	 }
	 */

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::saveFile(const char *filename) {
	return Game->_fileManager->SaveFile(filename, ((Common::MemoryWriteStreamDynamic *)_saveStream)->getData(), ((Common::MemoryWriteStreamDynamic *)_saveStream)->size(), Game->_compressedSavegames, _richBuffer, _richBufferSize);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::putBytes(byte *buffer, uint32 size) {
	_saveStream->write(buffer, size);
	if (_saveStream->err())
		return E_FAIL;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::getBytes(byte *buffer, uint32 size) {
	_loadStream->read(buffer, size);
	if (_loadStream->err())
		return E_FAIL;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::putDWORD(uint32 val) {
	_saveStream->writeUint32LE(val);
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPersistMgr::getDWORD() {
	uint32 ret = _loadStream->readUint32LE();
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::putString(const Common::String &val) {
	if (!val.size()) putString("(null)");
	else {
		_saveStream->writeUint32LE(val.size());
		_saveStream->writeString(val);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBPersistMgr::getString() {
	uint32 len = _loadStream->readUint32LE();
	char *ret = new char[len + 1];
	_loadStream->read(ret, len);
	ret[len] = '\0';

	if (!strcmp(ret, "(null)")) {
		delete[] ret;
		return NULL;
	} else return ret;
}

HRESULT CBPersistMgr::putTimeDate(const TimeDate &t) {
	_saveStream->writeSint32LE(t.tm_sec);
	_saveStream->writeSint32LE(t.tm_min);
	_saveStream->writeSint32LE(t.tm_hour);
	_saveStream->writeSint32LE(t.tm_mday);
	_saveStream->writeSint32LE(t.tm_mon);
	_saveStream->writeSint32LE(t.tm_year);
	// _saveStream->writeSint32LE(t.tm_wday); //TODO: Add this in when merging next

	if (_saveStream->err()) {
		return E_FAIL;
	}
	return S_OK;
}

TimeDate CBPersistMgr::getTimeDate() {
	TimeDate t;
	t.tm_sec = _loadStream->readSint32LE();
	t.tm_min = _loadStream->readSint32LE();
	t.tm_hour = _loadStream->readSint32LE();
	t.tm_mday = _loadStream->readSint32LE();
	t.tm_mon = _loadStream->readSint32LE();
	t.tm_year = _loadStream->readSint32LE();
	// t.tm_wday = _loadStream->readSint32LE(); //TODO: Add this in when merging next
	return t;
}

void CBPersistMgr::putFloat(float val) {
	Common::String str = Common::String::format("F%f", val);
	_saveStream->writeUint32LE(str.size());
	_saveStream->writeString(str);
}

float CBPersistMgr::getFloat() {
	char *str = getString();
	float value = 0.0f;
	int ret = sscanf(str, "F%f", &value);
	if (ret != 1) {
		warning("%s not parsed as float", str);
	}
	delete[] str;
	return value;
}

void CBPersistMgr::putDouble(double val) {
	Common::String str = Common::String::format("F%f", val);
	str.format("D%f", val);
	_saveStream->writeUint32LE(str.size());
	_saveStream->writeString(str);
}

double CBPersistMgr::getDouble() {
	char *str = getString();
	double value = 0.0f;
	int ret = sscanf(str, "F%f", &value);
	if (ret != 1) {
		warning("%s not parsed as float", str);
	}
	delete[] str;
	return value;
}

//////////////////////////////////////////////////////////////////////////
// bool
HRESULT CBPersistMgr::transfer(const char *name, bool *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// int
HRESULT CBPersistMgr::transfer(const char *name, int *val) {
	if (_saving) {
		_saveStream->writeSint32LE(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = _loadStream->readSint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// DWORD
HRESULT CBPersistMgr::transfer(const char *name, uint32 *val) {
	if (_saving) {
		_saveStream->writeUint32LE(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = _loadStream->readUint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// float
HRESULT CBPersistMgr::transfer(const char *name, float *val) {
	if (_saving) {
		putFloat(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = getFloat();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// double
HRESULT CBPersistMgr::transfer(const char *name, double *val) {
	if (_saving) {
		putDouble(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = getDouble();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// char*
HRESULT CBPersistMgr::transfer(const char *name, char **val) {
	if (_saving) {
		putString(*val);
		return S_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return E_FAIL;
		}
		*val = str;
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// const char*
HRESULT CBPersistMgr::transfer(const char *name, const char **val) {
	if (_saving) {
		putString(*val);
		return S_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return E_FAIL;
		}
		*val = str;
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Common::String
HRESULT CBPersistMgr::transfer(const char *name, Common::String *val) {
	if (_saving) {
		putString(*val);
		return S_OK;
	} else {
		char *str = getString();
		if (_loadStream->err()) {
			delete[] str;
			return E_FAIL;
		}
		if (str) {
			*val = str;
			delete[] str;
		} else {
			*val = "";
		}
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::transfer(const char *name, AnsiStringArray &val) {
	size_t size;

	if (_saving) {
		size = val.size();
		_saveStream->writeUint32LE(size);

		for (AnsiStringArray::iterator it = val.begin(); it != val.end(); ++it) {
			putString((*it).c_str());
		}
	} else {
		val.clear();
		size = _loadStream->readUint32LE();

		for (size_t i = 0; i < size; i++) {
			char *str = getString();
			if (_loadStream->err()) {
				delete[] str;
				return E_FAIL;
			}
			if (str) val.push_back(str);
			delete[] str;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// BYTE
HRESULT CBPersistMgr::transfer(const char *name, byte *val) {
	if (_saving) {
		_saveStream->writeByte(*val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		*val = _loadStream->readByte();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// RECT
HRESULT CBPersistMgr::transfer(const char *name, RECT *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->left);
		_saveStream->writeSint32LE(val->top);
		_saveStream->writeSint32LE(val->right);
		_saveStream->writeSint32LE(val->bottom);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		val->left = _loadStream->readSint32LE();
		val->top = _loadStream->readSint32LE();
		val->right = _loadStream->readSint32LE();
		val->bottom = _loadStream->readSint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// POINT
HRESULT CBPersistMgr::transfer(const char *name, POINT *val) {
	if (_saving) {
		_saveStream->writeSint32LE(val->x);
		_saveStream->writeSint32LE(val->y);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		val->x = _loadStream->readSint32LE();
		val->y = _loadStream->readSint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// Vector2
HRESULT CBPersistMgr::transfer(const char *name, Vector2 *val) {
	if (_saving) {
		putFloat(val->x);
		putFloat(val->y);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		val->x = getFloat();
		val->y = getFloat();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// generic pointer
HRESULT CBPersistMgr::transfer(const char *name, void *val) {
	int ClassID = -1, InstanceID = -1;

	if (_saving) {
		CSysClassRegistry::GetInstance()->GetPointerID(*(void **)val, &ClassID, &InstanceID);
		if (*(void **)val != NULL && (ClassID == -1 || InstanceID == -1)) {
			Game->LOG(0, "Warning: invalid instance '%s'", name);
		}

		_saveStream->writeUint32LE(ClassID);
		_saveStream->writeUint32LE(InstanceID);
	} else {
		ClassID = _loadStream->readUint32LE();
		InstanceID = _loadStream->readUint32LE();

		*(void **)val = CSysClassRegistry::GetInstance()->IDToPointer(ClassID, InstanceID);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBPersistMgr::checkVersion(byte verMajor, byte verMinor, byte verBuild) {
	if (_saving) return true;

	// it's ok if we are same or newer than the saved game
	if (verMajor >  _savedVerMajor ||
	        (verMajor == _savedVerMajor && verMinor >  _savedVerMinor) ||
	        (verMajor == _savedVerMajor && verMinor == _savedVerMinor && verBuild > _savedVerBuild)
	   ) return false;

	return true;
}

} // end of namespace WinterMute
