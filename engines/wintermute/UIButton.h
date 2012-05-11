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

#ifndef WINTERMUTE_UIBUTTON_H
#define WINTERMUTE_UIBUTTON_H


#include "UIObject.h"
#include "dctypes.h"    // Added by ClassView

namespace WinterMute {

class CUIButton : public CUIObject {
public:
	bool _pixelPerfect;
	bool _stayPressed;
	bool _centerImage;
	bool _oneTimePress;
	uint32 _oneTimePressTime;
	DECLARE_PERSISTENT(CUIButton, CUIObject)
	void Press();
	virtual HRESULT Display(int OffsetX = 0, int OffsetY = 0);
	bool _press;
	bool _hover;
	void CorrectSize();
	TTextAlign _align;
	CBSprite *_imageHover;
	CBSprite *_imagePress;
	CBSprite *_imageDisable;
	CBSprite *_imageFocus;
	CBFont *_fontDisable;
	CBFont *_fontPress;
	CBFont *_fontHover;
	CBFont *_fontFocus;
	CUITiledImage *_backPress;
	CUITiledImage *_backHover;
	CUITiledImage *_backDisable;
	CUITiledImage *_backFocus;
	CUIButton(CBGame *inGame = NULL);
	virtual ~CUIButton();
	HRESULT LoadFile(const char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);

	// scripting interface
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual char *ScToString();
};

} // end of namespace WinterMute

#endif
