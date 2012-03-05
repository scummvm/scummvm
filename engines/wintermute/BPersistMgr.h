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

#ifndef WINTERMUTE_BPERSISTMGR_H
#define WINTERMUTE_BPERSISTMGR_H


#include "BBase.h"

namespace WinterMute {

class Vector2;

class CBPersistMgr : public CBBase {
public:
	char *m_SavedDescription;
	time_t m_SavedTimestamp;
	byte m_SavedVerMajor;
	byte m_SavedVerMinor;
	byte m_SavedVerBuild;
	byte m_SavedExtMajor;
	byte m_SavedExtMinor;
	HRESULT SaveFile(char *Filename);
	uint32 GetDWORD();
	void PutDWORD(uint32 Val);
	char *GetString();
	void PutString(const char *Val);
	void Cleanup();
	HRESULT InitLoad(char *Filename);
	HRESULT InitSave(char *Desc);
	HRESULT GetBytes(byte  *Buffer, uint32 Size);
	HRESULT PutBytes(byte  *Buffer, uint32 Size);
	uint32 m_Offset;
	uint32 m_BufferSize;
	byte *m_Buffer;
	bool m_Saving;

	uint32 m_RichBufferSize;
	byte *m_RichBuffer;

	HRESULT Transfer(const char *Name, void *Val);
	HRESULT Transfer(const char *Name, int *Val);
	HRESULT Transfer(const char *Name, uint32 *Val);
	HRESULT Transfer(const char *Name, float *Val);
	HRESULT Transfer(const char *Name, double *Val);
	HRESULT Transfer(const char *Name, bool *Val);
	HRESULT Transfer(const char *Name, byte *Val);
	HRESULT Transfer(const char *Name, RECT *Val);
	HRESULT Transfer(const char *Name, POINT *Val);
	HRESULT Transfer(const char *Name, char **Val);
	HRESULT Transfer(const char *Name, Vector2 *Val);
	HRESULT Transfer(const char *Name, AnsiStringArray &Val);
	CBPersistMgr(CBGame *inGame = NULL);
	virtual ~CBPersistMgr();
	bool CheckVersion(byte  VerMajor, byte VerMinor, byte VerBuild);

	uint32 m_ThumbnailDataSize;
	byte *m_ThumbnailData;

};

} // end of namespace WinterMute

#endif
