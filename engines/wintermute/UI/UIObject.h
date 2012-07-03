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

#ifndef WINTERMUTE_UIOBJECT_H
#define WINTERMUTE_UIOBJECT_H


#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView

namespace WinterMute {

class CUITiledImage;
class CBFont;
class CUIObject : public CBObject {
public:

	HRESULT getTotalOffset(int *offsetX, int *offsetY);
	bool _canFocus;
	HRESULT focus();
	virtual HRESULT handleMouse(TMouseEvent Event, TMouseButton Button);
	bool isFocused();
	bool _parentNotify;
	DECLARE_PERSISTENT(CUIObject, CBObject)
	CUIObject *_parent;
	virtual HRESULT display(int offsetX = 0, int offsetY = 0);
	virtual void correctSize();
	bool _sharedFonts;
	bool _sharedImages;
	void setText(const char *text);
	char *_text;
	CBFont *_font;
	bool _visible;
	CUITiledImage *_back;
	bool _disable;
	CUIObject(CBGame *inGame = NULL);
	virtual ~CUIObject();
	int _width;
	int _height;
	TUIObjectType _type;
	CBSprite *_image;
	void setListener(CBScriptHolder *Object, CBScriptHolder *ListenerObject, uint32 ListenerParam);
	CBScriptHolder *_listenerParamObject;
	uint32 _listenerParamDWORD;
	CBScriptHolder *_listenerObject;
	CUIObject *_focusedWidget;
	virtual HRESULT saveAsText(CBDynBuffer *Buffer, int Indent);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
