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
#include "engines/wintermute/StringUtil.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/Base/BSound.h"
#include "common/str.h"

namespace WinterMute {

#define SAVE_BUFFER_INIT_SIZE 100000
#define SAVE_BUFFER_GROW_BY   50000

#define SAVE_MAGIC      0x45564153
#define SAVE_MAGIC_2    0x32564153

//////////////////////////////////////////////////////////////////////////
CBPersistMgr::CBPersistMgr(CBGame *inGame): CBBase(inGame) {
	_saving = false;
	_buffer = NULL;
	_bufferSize = 0;
	_offset = 0;

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
	if (_buffer) {
		if (_saving) free(_buffer);
		else delete [] _buffer; // allocated by file manager
	}
	_buffer = NULL;

	_bufferSize = 0;
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

	_buffer = (byte *)malloc(SAVE_BUFFER_INIT_SIZE);
	if (_buffer) {
		_bufferSize = SAVE_BUFFER_INIT_SIZE;
		res = S_OK;
	} else res = E_FAIL;


	if (SUCCEEDED(res)) {
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
		uint32 Version = makeUint32(VerMajor, VerMinor, ExtMajor, ExtMinor);
		PutDWORD(Version);

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
				if (Size > 0) PutBytes(Buffer, Size);

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
	}
	return res;
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

	_buffer = Game->_fileManager->ReadWholeFile(Filename, &_bufferSize);
	if (_buffer) {
		uint32 Magic;
		Magic = GetDWORD();
		if (Magic != DCGF_MAGIC) goto init_fail;

		Magic = GetDWORD();

		if (Magic == SAVE_MAGIC || Magic == SAVE_MAGIC_2) {
			uint32 Version = GetDWORD();
			_savedVerMajor = getLowByte(getLowWord(Version));
			_savedVerMinor = getHighByte(getLowWord(Version));
			_savedExtMajor = getLowByte(getHighWord(Version));
			_savedExtMinor = getHighByte(getHighWord(Version));

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
			if( _savedVerMajor != DCGF_VER_MAJOR || _savedVerMinor != DCGF_VER_MINOR)
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
	return Game->_fileManager->SaveFile(Filename, _buffer, _offset, Game->_compressedSavegames, _richBuffer, _richBufferSize);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::PutBytes(byte  *Buffer, uint32 Size) {
	while (_offset + Size > _bufferSize) {
		_bufferSize += SAVE_BUFFER_GROW_BY;
		_buffer = (byte *)realloc(_buffer, _bufferSize);
		if (!_buffer) {
			Game->LOG(0, "Error reallocating save buffer to %d bytes", _bufferSize);
			return E_FAIL;
		}
	}

	memcpy(_buffer + _offset, Buffer, Size);
	_offset += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::GetBytes(byte  *Buffer, uint32 Size) {
	if (_offset + Size > _bufferSize) {
		Game->LOG(0, "Fatal: Save buffer underflow");
		return E_FAIL;
	}

	memcpy(Buffer, _buffer + _offset, Size);
	_offset += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutDWORD(uint32 Val) {
	PutBytes((byte *)&Val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPersistMgr::GetDWORD() {
	uint32 ret;
	GetBytes((byte *)&ret, sizeof(uint32));
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutString(const char *Val) {
	if (!Val) PutString("(null)");
	else {
		PutDWORD(strlen(Val) + 1);
		PutBytes((byte *)Val, strlen(Val) + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBPersistMgr::GetString() {
	uint32 len = GetDWORD();
	char *ret = (char *)(_buffer + _offset);
	_offset += len;

	if (!strcmp(ret, "(null)")) return NULL;
	else return ret;
}

//////////////////////////////////////////////////////////////////////////
// bool
HRESULT CBPersistMgr::Transfer(const char *Name, bool *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(bool));
	else return GetBytes((byte *)Val, sizeof(bool));
}


//////////////////////////////////////////////////////////////////////////
// int
HRESULT CBPersistMgr::Transfer(const char *Name, int *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(int));
	else return GetBytes((byte *)Val, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
// DWORD
HRESULT CBPersistMgr::Transfer(const char *Name, uint32 *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(uint32));
	else return GetBytes((byte *)Val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
// float
HRESULT CBPersistMgr::Transfer(const char *Name, float *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(float));
	else return GetBytes((byte *)Val, sizeof(float));
}


//////////////////////////////////////////////////////////////////////////
// double
HRESULT CBPersistMgr::Transfer(const char *Name, double *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(double));
	else return GetBytes((byte *)Val, sizeof(double));
}


//////////////////////////////////////////////////////////////////////////
// char*
HRESULT CBPersistMgr::Transfer(const char *Name, char **Val) {
	if (_saving) {
		PutString(*Val);
		return S_OK;
	} else {
		char *str = GetString();
		if (str) {

			*Val = new char[strlen(str) + 1];
			strcpy(*Val, str);
		} else *Val = NULL;
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::Transfer(const char *Name, AnsiStringArray &Val) {
	size_t size;

	if (_saving) {
		size = Val.size();
		PutBytes((byte *)&size, sizeof(size_t));

		for (AnsiStringArray::iterator it = Val.begin(); it != Val.end(); ++it) {
			PutString((*it).c_str());
		}
	} else {
		Val.clear();
		GetBytes((byte *)&size, sizeof(size_t));

		for (size_t i = 0; i < size; i++) {
			char *str = GetString();
			if (str) Val.push_back(str);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// BYTE
HRESULT CBPersistMgr::Transfer(const char *Name, byte *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(byte));
	else return GetBytes((byte *)Val, sizeof(byte));
}


//////////////////////////////////////////////////////////////////////////
// RECT
HRESULT CBPersistMgr::Transfer(const char *Name, RECT *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(RECT));
	else return GetBytes((byte *)Val, sizeof(RECT));
}


//////////////////////////////////////////////////////////////////////////
// POINT
HRESULT CBPersistMgr::Transfer(const char *Name, POINT *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(POINT));
	else return GetBytes((byte *)Val, sizeof(POINT));
}


//////////////////////////////////////////////////////////////////////////
// Vector2
HRESULT CBPersistMgr::Transfer(const char *Name, Vector2 *Val) {
	if (_saving) return PutBytes((byte *)Val, sizeof(Vector2));
	else return GetBytes((byte *)Val, sizeof(Vector2));
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

		PutDWORD(ClassID);
		PutDWORD(InstanceID);
	} else {
		ClassID = GetDWORD();
		InstanceID = GetDWORD();

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
