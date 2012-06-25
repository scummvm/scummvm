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

#ifndef WINTERMUTE_ADITEM_H
#define WINTERMUTE_ADITEM_H


#include "engines/wintermute/Ad/AdTalkHolder.h"

namespace WinterMute {

class CAdItem : public CAdTalkHolder {
public:
	bool _displayAmount;
	int _amount;
	int _amountOffsetX;
	int _amountOffsetY;
	TTextAlign _amountAlign;
	char *_amountString;


	HRESULT Update();
	DECLARE_PERSISTENT(CAdItem, CAdTalkHolder)
	HRESULT Display(int X, int Y);
	bool GetExtendedFlag(const char *FlagName);
	bool _inInventory;
	bool _cursorCombined;
	CBSprite *_spriteHover;
	CBSprite *_cursorNormal;
	CBSprite *_cursorHover;
	CAdItem(CBGame *inGame);
	virtual ~CAdItem();
	HRESULT LoadFile(const char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
