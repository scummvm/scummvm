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


	HRESULT update();
	DECLARE_PERSISTENT(CAdItem, CAdTalkHolder)
	HRESULT display(int x, int y);
	bool getExtendedFlag(const char *flagName);
	bool _inInventory;
	bool _cursorCombined;
	CBSprite *_spriteHover;
	CBSprite *_cursorNormal;
	CBSprite *_cursorHover;
	CAdItem(CBGame *inGame);
	virtual ~CAdItem();
	HRESULT loadFile(const char *filename);
	HRESULT loadBuffer(byte *buffer, bool complete = true);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
