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

#include "dcgf.h"
#include "BFileManager.h"
#include "BGame.h"
#include "BPersistMgr.h"
#include "BSaveThumbHelper.h"
#include "PlatformSDL.h"
#include "Vector2.h"
#include "StringUtil.h"
#include "BImage.h"
#include "BSound.h"
#include "common/str.h"

namespace WinterMute {

#define SAVE_BUFFER_INIT_SIZE 100000
#define SAVE_BUFFER_GROW_BY   50000

#define SAVE_MAGIC      0x45564153
#define SAVE_MAGIC_2    0x32564153

//////////////////////////////////////////////////////////////////////////
CBPersistMgr::CBPersistMgr(CBGame *inGame): CBBase(inGame) {
	m_Saving = false;
	m_Buffer = NULL;
	m_BufferSize = 0;
	m_Offset = 0;

	m_RichBuffer = NULL;
	m_RichBufferSize = 0;

	m_SavedDescription = NULL;
	m_SavedTimestamp = 0;
	m_SavedVerMajor = m_SavedVerMinor = m_SavedVerBuild = 0;
	m_SavedExtMajor = m_SavedExtMinor = 0;

	m_ThumbnailDataSize = 0;
	m_ThumbnailData = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPersistMgr::~CBPersistMgr() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::Cleanup() {
	if (m_Buffer) {
		if (m_Saving) free(m_Buffer);
		else delete [] m_Buffer; // allocated by file manager
	}
	m_Buffer = NULL;

	m_BufferSize = 0;
	m_Offset = 0;

	delete[] m_RichBuffer;
	m_RichBuffer = NULL;
	m_RichBufferSize = 0;

	m_SavedDescription = NULL; // ref to buffer
	m_SavedTimestamp = 0;
	m_SavedVerMajor = m_SavedVerMinor = m_SavedVerBuild = 0;
	m_SavedExtMajor = m_SavedExtMinor = 0;

	m_ThumbnailDataSize = 0;
	if (m_ThumbnailData) {
		delete [] m_ThumbnailData;
		m_ThumbnailData = NULL;
	}
}

// TODO: This is not at all endian-safe
uint32 makeUint32(byte first, byte second, byte third, byte fourth) {
	uint32 retVal = first;
	retVal = retVal & second << 8 & third << 16 & fourth << 24;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::InitSave(char *Desc) {
	if (!Desc) return E_FAIL;

	HRESULT res;

	Cleanup();
	m_Saving = true;

	m_Buffer = (byte  *)malloc(SAVE_BUFFER_INIT_SIZE);
	if (m_Buffer) {
		m_BufferSize = SAVE_BUFFER_INIT_SIZE;
		res = S_OK;
	} else res = E_FAIL;


	if (SUCCEEDED(res)) {
		// get thumbnails
		if (!Game->m_CachedThumbnail) {
			Game->m_CachedThumbnail = new CBSaveThumbHelper(Game);
			if (FAILED(Game->m_CachedThumbnail->StoreThumbnail(true))) {
				delete Game->m_CachedThumbnail;
				Game->m_CachedThumbnail = NULL;
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
		PutString(Game->m_Name);

		// thumbnail data size
		bool ThumbnailOK = false;

		if (Game->m_CachedThumbnail) {
			if (Game->m_CachedThumbnail->m_Thumbnail) {
				uint32 Size = 0;
				byte *Buffer = Game->m_CachedThumbnail->m_Thumbnail->CreateBMPBuffer(&Size);

				PutDWORD(Size);
				if (Size > 0) PutBytes(Buffer, Size);

				delete [] Buffer;
				ThumbnailOK = true;
			}
		}
		if (!ThumbnailOK) PutDWORD(0);

		// in any case, destroy the cached thumbnail once used
		delete Game->m_CachedThumbnail;
		Game->m_CachedThumbnail = NULL;

		uint32 DataOffset = m_Offset +
		                   sizeof(uint32) + // data offset
		                   sizeof(uint32) + strlen(Desc) + 1 + // description
		                   sizeof(uint32); // timestamp

		PutDWORD(DataOffset);
		PutString(Desc);

		time_t Timestamp;
		time(&Timestamp);
		PutDWORD((uint32)Timestamp);
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
HRESULT CBPersistMgr::InitLoad(char *Filename) {
	Cleanup();

	m_Saving = false;

	m_Buffer = Game->m_FileManager->ReadWholeFile(Filename, &m_BufferSize);
	if (m_Buffer) {
		uint32 Magic;
		Magic = GetDWORD();
		if (Magic != DCGF_MAGIC) goto init_fail;

		Magic = GetDWORD();

		if (Magic == SAVE_MAGIC || Magic == SAVE_MAGIC_2) {
			uint32 Version = GetDWORD();
			m_SavedVerMajor = getLowByte(getLowWord(Version));
			m_SavedVerMinor = getHighByte(getLowWord(Version));
			m_SavedExtMajor = getLowByte(getHighWord(Version));
			m_SavedExtMinor = getHighByte(getHighWord(Version));

			if (Magic == SAVE_MAGIC_2) {
				m_SavedVerBuild = (byte )GetDWORD();
				char *SavedName = GetString();
				if (SavedName == NULL || scumm_stricmp(SavedName, Game->m_Name) != 0) {
					Game->LOG(0, "ERROR: Saved game name doesn't match current game");
					goto init_fail;
				}

				// load thumbnail
				m_ThumbnailDataSize = GetDWORD();
				if (m_ThumbnailDataSize > 0) {
					m_ThumbnailData = new byte[m_ThumbnailDataSize];
					if (m_ThumbnailData) {
						GetBytes(m_ThumbnailData, m_ThumbnailDataSize);
					} else m_ThumbnailDataSize = 0;
				}
			} else m_SavedVerBuild = 35; // last build with ver1 savegames


			// if save is newer version than we are, fail
			if (m_SavedVerMajor >  DCGF_VER_MAJOR ||
			        (m_SavedVerMajor == DCGF_VER_MAJOR && m_SavedVerMinor >  DCGF_VER_MINOR) ||
			        (m_SavedVerMajor == DCGF_VER_MAJOR && m_SavedVerMinor == DCGF_VER_MINOR && m_SavedVerBuild > DCGF_VER_BUILD)
			   ) {
				Game->LOG(0, "ERROR: Saved game version is newer than current game");
				goto init_fail;
			}

			// if save is older than the minimal version we support
			if (m_SavedVerMajor <  SAVEGAME_VER_MAJOR ||
			        (m_SavedVerMajor == SAVEGAME_VER_MAJOR && m_SavedVerMinor <  SAVEGAME_VER_MINOR) ||
			        (m_SavedVerMajor == SAVEGAME_VER_MAJOR && m_SavedVerMinor == SAVEGAME_VER_MINOR && m_SavedVerBuild < SAVEGAME_VER_BUILD)
			   ) {
				Game->LOG(0, "ERROR: Saved game is too old and cannot be used by this version of game engine");
				goto init_fail;
			}

			/*
			if( m_SavedVerMajor != DCGF_VER_MAJOR || m_SavedVerMinor != DCGF_VER_MINOR)
			{
			    Game->LOG(0, "ERROR: Saved game is created by other WME version");
			    goto init_fail;
			}
			*/
		} else goto init_fail;


		uint32 DataOffset = GetDWORD();

		m_SavedDescription = GetString();
		m_SavedTimestamp = (time_t)GetDWORD();

		m_Offset = DataOffset;

		return S_OK;
	}

init_fail:
	Cleanup();
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::SaveFile(char *Filename) {
	return Game->m_FileManager->SaveFile(Filename, m_Buffer, m_Offset, Game->m_CompressedSavegames, m_RichBuffer, m_RichBufferSize);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::PutBytes(byte  *Buffer, uint32 Size) {
	while (m_Offset + Size > m_BufferSize) {
		m_BufferSize += SAVE_BUFFER_GROW_BY;
		m_Buffer = (byte  *)realloc(m_Buffer, m_BufferSize);
		if (!m_Buffer) {
			Game->LOG(0, "Error reallocating save buffer to %d bytes", m_BufferSize);
			return E_FAIL;
		}
	}

	memcpy(m_Buffer + m_Offset, Buffer, Size);
	m_Offset += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPersistMgr::GetBytes(byte  *Buffer, uint32 Size) {
	if (m_Offset + Size > m_BufferSize) {
		Game->LOG(0, "Fatal: Save buffer underflow");
		return E_FAIL;
	}

	memcpy(Buffer, m_Buffer + m_Offset, Size);
	m_Offset += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutDWORD(uint32 Val) {
	PutBytes((byte  *)&Val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
uint32 CBPersistMgr::GetDWORD() {
	uint32 ret;
	GetBytes((byte  *)&ret, sizeof(uint32));
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBPersistMgr::PutString(const char *Val) {
	if (!Val) PutString("(null)");
	else {
		PutDWORD(strlen(Val) + 1);
		PutBytes((byte  *)Val, strlen(Val) + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBPersistMgr::GetString() {
	uint32 len = GetDWORD();
	char *ret = (char *)(m_Buffer + m_Offset);
	m_Offset += len;

	if (!strcmp(ret, "(null)")) return NULL;
	else return ret;
}

//////////////////////////////////////////////////////////////////////////
// bool
HRESULT CBPersistMgr::Transfer(const char *Name, bool *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(bool));
	else return GetBytes((byte  *)Val, sizeof(bool));
}


//////////////////////////////////////////////////////////////////////////
// int
HRESULT CBPersistMgr::Transfer(const char *Name, int *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(int));
	else return GetBytes((byte  *)Val, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
// DWORD
HRESULT CBPersistMgr::Transfer(const char *Name, uint32 *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(uint32));
	else return GetBytes((byte  *)Val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
// float
HRESULT CBPersistMgr::Transfer(const char *Name, float *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(float));
	else return GetBytes((byte  *)Val, sizeof(float));
}


//////////////////////////////////////////////////////////////////////////
// double
HRESULT CBPersistMgr::Transfer(const char *Name, double *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(double));
	else return GetBytes((byte  *)Val, sizeof(double));
}


//////////////////////////////////////////////////////////////////////////
// char*
HRESULT CBPersistMgr::Transfer(const char *Name, char **Val) {
	if (m_Saving) {
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

	if (m_Saving) {
		size = Val.size();
		PutBytes((byte  *)&size, sizeof(size_t));

		for (AnsiStringArray::iterator it = Val.begin(); it != Val.end(); ++it) {
			PutString((*it).c_str());
		}
	} else {
		Val.clear();
		GetBytes((byte  *)&size, sizeof(size_t));

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
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(byte ));
	else return GetBytes((byte  *)Val, sizeof(byte ));
}


//////////////////////////////////////////////////////////////////////////
// RECT
HRESULT CBPersistMgr::Transfer(const char *Name, RECT *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(RECT));
	else return GetBytes((byte  *)Val, sizeof(RECT));
}


//////////////////////////////////////////////////////////////////////////
// POINT
HRESULT CBPersistMgr::Transfer(const char *Name, POINT *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(POINT));
	else return GetBytes((byte  *)Val, sizeof(POINT));
}


//////////////////////////////////////////////////////////////////////////
// Vector2
HRESULT CBPersistMgr::Transfer(const char *Name, Vector2 *Val) {
	if (m_Saving) return PutBytes((byte  *)Val, sizeof(Vector2));
	else return GetBytes((byte  *)Val, sizeof(Vector2));
}


//////////////////////////////////////////////////////////////////////////
// generic pointer
HRESULT CBPersistMgr::Transfer(const char *Name, void *Val) {
	int ClassID = -1, InstanceID = -1;

	if (m_Saving) {
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
	if (m_Saving) return true;

	// it's ok if we are same or newer than the saved game
	if (VerMajor >  m_SavedVerMajor ||
	        (VerMajor == m_SavedVerMajor && VerMinor >  m_SavedVerMinor) ||
	        (VerMajor == m_SavedVerMajor && VerMinor == m_SavedVerMinor && VerBuild > m_SavedVerBuild)
	   ) return false;

	return true;
}

} // end of namespace WinterMute
