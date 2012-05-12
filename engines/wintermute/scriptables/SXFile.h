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

#ifndef WINTERMUTES_SXFILE_H
#define WINTERMUTES_SXFILE_H


#include "engines/wintermute/BScriptable.h"

namespace WinterMute {

class CBFile;

class CSXFile : public CBScriptable {
public:
	DECLARE_PERSISTENT(CSXFile, CBScriptable)
	CScValue *ScGetProperty(const char *Name);
	HRESULT ScSetProperty(const char *Name, CScValue *Value);
	HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	const char *ScToString();
	CSXFile(CBGame *inGame, CScStack *Stack);
	virtual ~CSXFile();
private:
	CBFile *_readFile;
	FILE *_writeFile;
	int _mode; // 0..none, 1..read, 2..write, 3..append
	bool _textMode;
	void Close();
	void Cleanup();
	uint32 GetPos();
	uint32 GetLength();
	bool SetPos(uint32 Pos, TSeek Origin = SEEK_TO_BEGIN);
	char *_filename;
};

} // end of namespace WinterMute

#endif
