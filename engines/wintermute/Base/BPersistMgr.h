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


#include "engines/wintermute/Base/BBase.h"
#include "engines/savestate.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/system.h"

namespace WinterMute {

class Vector2;

class CBPersistMgr : public CBBase {
public:
	char *_savedDescription;
	TimeDate _savedTimestamp;
	uint32 _savedPlayTime;
	byte _savedVerMajor;
	byte _savedVerMinor;
	byte _savedVerBuild;
	byte _savedExtMajor;
	byte _savedExtMinor;
	Common::String _savedName;
	HRESULT saveFile(const char *filename);
	uint32 getDWORD();
	void putDWORD(uint32 val);
	char *getString();
	Common::String getStringObj();
	void putString(const Common::String &val);
	float getFloat();
	void putFloat(float val);
	double getDouble();
	void putDouble(double val);
	void cleanup();
	void getSaveStateDesc(int slot, SaveStateDescriptor &desc);
	void deleteSaveSlot(int slot);
	uint32 getMaxUsedSlot();
	bool getSaveExists(int slot);
	HRESULT initLoad(const char *filename);
	HRESULT initSave(const char *desc);
	HRESULT getBytes(byte *buffer, uint32 size);
	HRESULT putBytes(byte *buffer, uint32 size);
	uint32 _offset;

	bool _saving;

	uint32 _richBufferSize;
	byte *_richBuffer;

	HRESULT transfer(const char *name, void *val);
	HRESULT transfer(const char *name, int *val);
	HRESULT transfer(const char *name, uint32 *val);
	HRESULT transfer(const char *name, float *val);
	HRESULT transfer(const char *name, double *val);
	HRESULT transfer(const char *name, bool *val);
	HRESULT transfer(const char *name, byte *val);
	HRESULT transfer(const char *name, RECT *val);
	HRESULT transfer(const char *name, POINT *val);
	HRESULT transfer(const char *name, const char **val);
	HRESULT transfer(const char *name, char **val);
	HRESULT transfer(const char *name, Common::String *val);
	HRESULT transfer(const char *name, Vector2 *val);
	HRESULT transfer(const char *name, AnsiStringArray &Val);
	CBPersistMgr(CBGame *inGame = NULL);
	virtual ~CBPersistMgr();
	bool checkVersion(byte  verMajor, byte verMinor, byte verBuild);

	uint32 _thumbnailDataSize;
	byte *_thumbnailData;
private:
	Common::String getFilenameForSlot(int slot);
	HRESULT readHeader(const Common::String &filename);
	TimeDate getTimeDate();
	HRESULT putTimeDate(const TimeDate &t);
	Common::WriteStream *_saveStream;
	Common::SeekableReadStream *_loadStream;
};

} // end of namespace WinterMute

#endif
