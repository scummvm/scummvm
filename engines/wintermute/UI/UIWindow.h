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

#ifndef WINTERMUTE_UIWINDOW_H
#define WINTERMUTE_UIWINDOW_H


#include "UIObject.h"
#include "common/events.h"

namespace WinterMute {

class CUIButton;
class CBViewport;
class CUIWindow : public CUIObject {
public:
	HRESULT GetWindowObjects(CBArray<CUIObject *, CUIObject *> &Objects, bool InteractiveOnly);

	bool _pauseMusic;
	void cleanup();
	virtual void makeFreezable(bool Freezable);
	CBViewport *_viewport;
	bool _clipContents;
	bool _inGame;
	bool _isMenu;
	bool _fadeBackground;
	uint32 _fadeColor;
	virtual bool handleMouseWheel(int Delta);
	CUIWindow *_shieldWindow;
	CUIButton *_shieldButton;
	HRESULT close();
	HRESULT GoSystemExclusive();
	HRESULT GoExclusive();
	TWindowMode _mode;
	HRESULT MoveFocus(bool Forward = true);
	virtual HRESULT handleMouse(TMouseEvent Event, TMouseButton Button);
	POINT _dragFrom;
	bool _dragging;
	DECLARE_PERSISTENT(CUIWindow, CUIObject)
	bool _transparent;
	HRESULT ShowWidget(const char *Name, bool Visible = true);
	HRESULT EnableWidget(const char *Name, bool Enable = true);
	RECT _titleRect;
	RECT _dragRect;
	virtual HRESULT display(int OffsetX = 0, int OffsetY = 0);
	CUIWindow(CBGame *inGame);
	virtual ~CUIWindow();
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	CBArray<CUIObject *, CUIObject *> _widgets;
	TTextAlign _titleAlign;
	HRESULT loadFile(const char *Filename);
	HRESULT loadBuffer(byte  *Buffer, bool Complete = true);
	CUITiledImage *_backInactive;
	CBFont *_fontInactive;
	CBSprite *_imageInactive;
	virtual HRESULT listen(CBScriptHolder *param1, uint32 param2);
	virtual HRESULT saveAsText(CBDynBuffer *Buffer, int Indent);

	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
