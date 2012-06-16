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
#include "commoN/memstream.h"
#include "common/str.h"

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
	_savedTimestamp = 0;
	_savedVerMajor = _savedVerMinor = _savedVerBuild = 0;
	_savedExtMajor = _savedExtMinor = 0;

	_thumbnailDataSize = 0;
	_thumbnailData = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPersistMgr::~CBPersistMgr() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::Cleanup() {
/*	if (_buffer) {
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
	_savedTimestamp = 0;
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

// TODO: This is not at all endian-safe
uint32 makeUint32(byte first, byte second, byte third, byte fourth) {
	uint32 retVal = first;
	retVal = retVal & second << 8 & third << 16 & fourth << 24;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::InitSave(const char *Desc) {
	if (!Desc) return E_FAIL;

	HRESULT res;

	Cleanup();
	_saving = true;

/*	_buffer = (byte *)malloc(SAVE_BUFFER_INIT_SIZE);
	if (_buffer) {
		_bufferSize = SAVE_BUFFER_INIT_SIZE;
		res = S_OK;
	} else res = E_FAIL;*/
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
		PutDWORD(magic);

		magic = SAVE_MAGIC_2;
		PutDWORD(magic);

		byte VerMajor, VerMinor, ExtMajor, ExtMinor;
		Game->GetVersion(&VerMajor, &VerMinor, &ExtMajor, &ExtMinor);
		//uint32 Version = MAKELONG(MAKEWORD(VerMajor, VerMinor), MAKEWORD(ExtMajor, ExtMinor));
		_saveStream->writeByte(VerMajor);
		_saveStream->writeByte(VerMinor);
		_saveStream->writeByte(ExtMajor);
		_saveStream->writeByte(ExtMinor);

		// new in ver 2
		PutDWORD((uint32)DCGF_VER_BUILD);
		PutString(Game->_name);

		// thumbnail data size
		bool ThumbnailOK = false;

		if (Game->_cachedThumbnail) {
			if (Game->_cachedThumbnail->_thumbnail) {
				uint32 Size = 0;
				byte *Buffer = Game->_cachedThumbnail->_thumbnail->CreateBMPBuffer(&Size);

				PutDWORD(Size);
				if (Size > 0) _saveStream->write(Buffer, Size);
				delete [] Buffer;
				ThumbnailOK = true;
			}
		}
		if (!ThumbnailOK) PutDWORD(0);

		// in any case, destroy the cached thumbnail once used
		delete Game->_cachedThumbnail;
		Game->_cachedThumbnail = NULL;

		uint32 DataOffset = _offset +
		                    sizeof(uint32) + // data offset
		                    sizeof(uint32) + strlen(Desc) + 1 + // description
		                    sizeof(uint32); // timestamp

		PutDWORD(DataOffset);
		PutString(Desc);
// TODO: Add usefull timestamps, we can't use ctime...
/*		time_t Timestamp;
		time(&Timestamp);
		PutDWORD((uint32)Timestamp);*/
		PutDWORD(0);
	}
	return S_OK;
}
// TODO: Do this properly, this is just a quickfix, that probably doesnt even work.
// The main point of which is ditching BASS completely.
byte getLowByte(uint16 word) {
	uint16 mask = 0xff;
	return word & mask;
}

byte getHighByte(uint16 word) {
	uint16 mask = 0xff << 8;
	word = word & mask;
	return word >> 8;
}

uint16 getLowWord(uint32 dword) {
	uint32 mask = 0xffff;
	return dword & mask;
}

uint16 getHighWord(uint32 dword) {
	uint32 mask = 0xffff << 16;
	dword = dword & mask;
	return dword >> 16;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::InitLoad(const char *Filename) {
	Cleanup();

	_saving = false;

	_loadStream = Game->_fileManager->loadSaveGame(Filename);
	//_buffer = Game->_fileManager->ReadWholeFile(Filename, &_bufferSize);
	if (_loadStream) {
		uint32 Magic;
		Magic = GetDWORD();
		if (Magic != DCGF_MAGIC) goto init_fail;

		Magic = GetDWORD();

		if (Magic == SAVE_MAGIC || Magic == SAVE_MAGIC_2) {
			_savedVerMajor = _loadStream->readByte();
			_savedVerMinor = _loadStream->readByte();
			_savedExtMajor = _loadStream->readByte();
			_savedExtMinor = _loadStream->readByte();

			if (Magic == SAVE_MAGIC_2) {
				_savedVerBuild = (byte)GetDWORD();
				char *SavedName = GetString();
				if (SavedName == NULL || scumm_stricmp(SavedName, Game->_name) != 0) {
					Game->LOG(0, "ERROR: Saved game name doesn't match current game");
					goto init_fail;
				}

				// load thumbnail
				_thumbnailDataSize = GetDWORD();
				if (_thumbnailDataSize > 0) {
					_thumbnailData = new byte[_thumbnailDataSize];
					if (_thumbnailData) {
						GetBytes(_thumbnailData, _thumbnailDataSize);
					} else _thumbnailDataSize = 0;
				}
			} else _savedVerBuild = 35; // last build with ver1 savegames


			// if save is newer version than we are, fail
			if (_savedVerMajor >  DCGF_VER_MAJOR ||
			        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor >  DCGF_VER_MINOR) ||
			        (_savedVerMajor == DCGF_VER_MAJOR && _savedVerMinor == DCGF_VER_MINOR && _savedVerBuild > DCGF_VER_BUILD)
			   ) {
				Game->LOG(0, "ERROR: Saved game version is newer than current game");
				goto init_fail;
			}

			// if save is older than the minimal version we support
			if (_savedVerMajor <  SAVEGAME_VER_MAJOR ||
			        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor <  SAVEGAME_VER_MINOR) ||
			        (_savedVerMajor == SAVEGAME_VER_MAJOR && _savedVerMinor == SAVEGAME_VER_MINOR && _savedVerBuild < SAVEGAME_VER_BUILD)
			   ) {
				Game->LOG(0, "ERROR: Saved game is too old and cannot be used by this version of game engine");
				goto init_fail;
			}

			/*
			if ( _savedVerMajor != DCGF_VER_MAJOR || _savedVerMinor != DCGF_VER_MINOR)
			{
			    Game->LOG(0, "ERROR: Saved game is created by other WME version");
			    goto init_fail;
			}
			*/
		} else goto init_fail;


		uint32 DataOffset = GetDWORD();

		_savedDescription = GetString();
		_savedTimestamp = (time_t)GetDWORD();

		_offset = DataOffset;

		return S_OK;
	}

init_fail:
	Cleanup();
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::SaveFile(const char *Filename) {
	return Game->_fileManager->SaveFile(Filename, ((Common::MemoryWriteStreamDynamic*)_saveStream)->getData(), ((Common::MemoryWriteStreamDynamic*)_saveStream)->size(), Game->_compressedSavegames, _richBuffer, _richBufferSize);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::PutBytes(byte *buffer, uint32 size) {
	_saveStream->write(buffer, size);
	if (_saveStream->err())
		return E_FAIL;
	return S_OK;
/*	while (_offset + Size > _bufferSize) {
		_bufferSize += SAVE_BUFFER_GROW_BY;
		_buffer = (byte *)realloc(_buffer, _bufferSize);
		if (!_buffer) {
			Game->LOG(0, "Error reallocating save buffer to %d bytes", _bufferSize);
			return E_FAIL;
		}
	}

	memcpy(_buffer + _offset, Buffer, Size);
	_offset += Size;

	return S_OK;*/
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::GetBytes(byte *buffer, uint32 size) {
	_loadStream->read(buffer, size);
	if (_loadStream->err())
		return E_FAIL;
	return S_OK;
/*	if (_offset + Size > _bufferSize) {
		Game->LOG(0, "Fatal: Save buffer underflow");
		return E_FAIL;
	}

	memcpy(Buffer, _buffer + _offset, Size);
	_offset += Size;

	return S_OK;*/
}

//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutDWORD(uint32 Val) {
	//PutBytes((byte *)&Val, sizeof(uint32));
	_saveStream->writeUint32LE(Val);
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPersistMgr::GetDWORD() {
	uint32 ret = _loadStream->readUint32LE();
//	GetBytes((byte *)&ret, sizeof(uint32));
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutString(const Common::String &Val) {
	if (!Val.size()) PutString("(null)");
	else {
	/*	PutDWORD(strlen(Val) + 1);
		PutBytes((byte *)Val, strlen(Val) + 1);*/
		_saveStream->writeUint32LE(Val.size());
		_saveStream->writeString(Val);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBPersistMgr::GetString() {
	uint32 len = _loadStream->readUint32LE();
	char *ret = new char[len + 1];
	_loadStream->read(ret, len);
	ret[len] = '\0';
/*	char *ret = (char *)(_buffer + _offset);
	_offset += len;*/

	if (!strcmp(ret, "(null)")) { 
		delete[] ret;
		return NULL;
	} else return ret;
}

void CBPersistMgr::putFloat(float val) {
	Common::String str = Common::String::format("F%f", val);
	_saveStream->writeUint32LE(str.size());
	_saveStream->writeString(str);
}

float CBPersistMgr::getFloat() {
	char *str = GetString();
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
	char *str = GetString();
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
HRESULT CBPersistMgr::Transfer(const char *Name, bool *Val) {
	if (_saving) { 
		//return PutBytes((byte *)Val, sizeof(bool));
		_saveStream->writeByte(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		//return GetBytes((byte *)Val, sizeof(bool));
		*Val = _loadStream->readByte();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// int
HRESULT CBPersistMgr::Transfer(const char *Name, int *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(int));
		_saveStream->writeSint32LE(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		// return GetBytes((byte *)Val, sizeof(int));
		*Val = _loadStream->readSint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// DWORD
HRESULT CBPersistMgr::Transfer(const char *Name, uint32 *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(uint32));
		_saveStream->writeUint32LE(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		// return GetBytes((byte *)Val, sizeof(uint32)); 
		*Val = _loadStream->readUint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// float
HRESULT CBPersistMgr::Transfer(const char *Name, float *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(float));
		putFloat(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		//return GetBytes((byte *)Val, sizeof(float));
		*Val = getFloat();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// double
HRESULT CBPersistMgr::Transfer(const char *Name, double *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(double));
		putDouble(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else {
		// return GetBytes((byte *)Val, sizeof(double)); 
		*Val = getDouble();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// char*
HRESULT CBPersistMgr::Transfer(const char *Name, char **Val) {
	if (_saving) {
		PutString(*Val);
		return S_OK;
	} else {
		char *str = GetString();
		if (_loadStream->err()) {
			delete[] str;
			return E_FAIL;
		}
		*Val = str;
		/*		if (str) {
		 
		 char *ret = new char[strlen(str) + 1];
		 strcpy(ret, str);
		 delete[] str;
		 } else *Val = NULL;*/
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// const char*
HRESULT CBPersistMgr::Transfer(const char *Name, const char **Val) {
	if (_saving) {
		PutString(*Val);
		return S_OK;
	} else {
		char *str = GetString();
		if (_loadStream->err()) {
			delete[] str;
			return E_FAIL;
		}
		*Val = str;
/*		if (str) {
			
			char *ret = new char[strlen(str) + 1];
			strcpy(ret, str);
			delete[] str;
		} else *Val = NULL;*/
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
// Common::String
HRESULT CBPersistMgr::Transfer(const char *Name, Common::String *val) {
	if (_saving) {
		PutString(*val);
		return S_OK;
	} else {
		char *str = GetString();
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
		/*		if (str) {
		 
		 char *ret = new char[strlen(str) + 1];
		 strcpy(ret, str);
		 delete[] str;
		 } else *Val = NULL;*/
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::Transfer(const char *Name, AnsiStringArray &Val) {
	size_t size;

	if (_saving) {
		size = Val.size();
		_saveStream->writeUint32LE(size);
		//PutBytes((byte *)&size, sizeof(size_t));

		for (AnsiStringArray::iterator it = Val.begin(); it != Val.end(); ++it) {
			PutString((*it).c_str());
		}
	} else {
		Val.clear();
		size = _loadStream->readUint32LE();
		//GetBytes((byte *)&size, sizeof(size_t));

		for (size_t i = 0; i < size; i++) {
			char *str = GetString();
			if (_loadStream->err()) {
				delete[] str;
				return E_FAIL;
			}
			if (str) Val.push_back(str);
			delete[] str;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// BYTE
HRESULT CBPersistMgr::Transfer(const char *Name, byte *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(byte));
		_saveStream->writeByte(*Val);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	} else { 
		//return GetBytes((byte *)Val, sizeof(byte)); 
		*Val = _loadStream->readByte();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// RECT
HRESULT CBPersistMgr::Transfer(const char *Name, RECT *Val) {
	if (_saving) {
		// return PutBytes((byte *)Val, sizeof(RECT));
		_saveStream->writeSint32LE(Val->left);
		_saveStream->writeSint32LE(Val->top);
		_saveStream->writeSint32LE(Val->right);
		_saveStream->writeSint32LE(Val->bottom);
		if (_saveStream->err())
			return E_FAIL;
		return S_OK;
	}
	else { 
		// return GetBytes((byte *)Val, sizeof(RECT));
		Val->left = _loadStream->readSint32LE();
		Val->top = _loadStream->readSint32LE();
		Val->right = _loadStream->readSint32LE();
		Val->bottom = _loadStream->readSint32LE();
		if (_loadStream->err())
			return E_FAIL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
// POINT
HRESULT CBPersistMgr::Transfer(const char *Name, POINT *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(POINT));
		_saveStream->writeSint32LE(Val->x);
		_saveStream->writeSint32LE(Val->y);
	} else {
		// return GetBytes((byte *)Val, sizeof(POINT));
		Val->x = _loadStream->readSint32LE();
		Val->y = _loadStream->readSint32LE();
	}
}


//////////////////////////////////////////////////////////////////////////
// Vector2
HRESULT CBPersistMgr::Transfer(const char *Name, Vector2 *Val) {
	if (_saving) {
		//return PutBytes((byte *)Val, sizeof(Vector2));
		putFloat(Val->x);
		putFloat(Val->y);
	} else {
		// return GetBytes((byte *)Val, sizeof(Vector2));
		Val->x = getFloat();
		Val->y = getFloat();
	}
}


//////////////////////////////////////////////////////////////////////////
// generic pointer
HRESULT CBPersistMgr::Transfer(const char *Name, void *Val) {
	int ClassID = -1, InstanceID = -1;

	if (_saving) {
		CSysClassRegistry::GetInstance()->GetPointerID(*(void **)Val, &ClassID, &InstanceID);
		if (*(void **)Val != NULL && (ClassID == -1 || InstanceID == -1)) {
			Game->LOG(0, "Warning: invalid instance '%s'", Name);
		}

		_saveStream->writeUint32LE(ClassID);
		_saveStream->writeUint32LE(InstanceID);
		// PutDWORD(ClassID);
		// PutDWORD(InstanceID);
	} else {
		ClassID = _loadStream->readUint32LE();
		InstanceID = _loadStream->readUint32LE();
/*		ClassID = GetDWORD();
		InstanceID = GetDWORD();*/

		*(void **)Val = CSysClassRegistry::GetInstance()->IDToPointer(ClassID, InstanceID);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBPersistMgr::CheckVersion(byte  VerMajor, byte VerMinor, byte VerBuild) {
	if (_saving) return true;

	// it's ok if we are same or newer than the saved game
	if (VerMajor >  _savedVerMajor ||
	        (VerMajor == _savedVerMajor && VerMinor >  _savedVerMinor) ||
	        (VerMajor == _savedVerMajor && VerMinor == _savedVerMinor && VerBuild > _savedVerBuild)
	   ) return false;

	return true;
}

} // end of namespace WinterMute
