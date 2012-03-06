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

#ifndef __WmeAdItem_H__
#define __WmeAdItem_H__


#include "AdTalkHolder.h"

namespace WinterMute {

class CAdItem : public CAdTalkHolder {
public:
	bool m_DisplayAmount;
	int m_Amount;
	int m_AmountOffsetX;
	int m_AmountOffsetY;
	TTextAlign m_AmountAlign;
	char *m_AmountString;


	HRESULT Update();
	DECLARE_PERSISTENT(CAdItem, CAdTalkHolder)
	HRESULT Display(int X, int Y);
	bool GetExtendedFlag(char *FlagName);
	bool m_InInventory;
	bool m_CursorCombined;
	CBSprite *m_SpriteHover;
	CBSprite *m_CursorNormal;
	CBSprite *m_CursorHover;
	CAdItem(CBGame *inGame);
	virtual ~CAdItem();
	HRESULT LoadFile(char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();
};

} // end of namespace WinterMute

#endif
