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

#ifndef WINTERMUTE_BBASE_H
#define WINTERMUTE_BBASE_H

#include "wintypes.h"
#include "dctypes.h"
#include <map>
#include <string>

namespace WinterMute {

class CBGame;
class CBDynBuffer;

class CBBase {
public:
	bool m_Persistable;
	HRESULT SetEditorProp(const char *PropName, const char *PropValue);
	const char *GetEditorProp(const char *PropName, const char *InitVal = NULL);
	CBBase(TDynamicConstructor, TDynamicConstructor) {};
	HRESULT ParseEditorProperty(byte  *Buffer, bool Complete = true);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent = 0);
	CBBase();
	CBGame *Game;
	CBBase(CBGame *GameOwner);
	virtual ~CBBase();

	std::map<std::string, std::string> m_EditorProps;
	std::map<std::string, std::string>::iterator m_EditorPropsIter;
};

} // end of namespace WinterMute

#endif
