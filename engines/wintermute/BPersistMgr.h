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
	char *_savedDescription;
	time_t _savedTimestamp;
	byte _savedVerMajor;
	byte _savedVerMinor;
	byte _savedVerBuild;
	byte _savedExtMajor;
	byte _savedExtMinor;
	HRESULT SaveFile(const char *Filename);
	uint32 GetDWORD();
	void PutDWORD(uint32 Val);
	char *GetString();
	void PutString(const char *Val);
	void Cleanup();
	HRESULT InitLoad(const char *Filename);
	HRESULT InitSave(const char *Desc);
	HRESULT GetBytes(byte  *Buffer, uint32 Size);
	HRESULT PutBytes(byte  *Buffer, uint32 Size);
	uint32 _offset;
	uint32 _bufferSize;
	byte *_buffer;
	bool _saving;

	uint32 _richBufferSize;
	byte *_richBuffer;

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

	uint32 _thumbnailDataSize;
	byte *_thumbnailData;

};

} // end of namespace WinterMute

#endif
