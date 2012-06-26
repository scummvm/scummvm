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

#ifndef WINTERMUTE_ADRESPONSEBOX_H
#define WINTERMUTE_ADRESPONSEBOX_H


#include "engines/wintermute/Base/BObject.h"

namespace WinterMute {

class CUIButton;
class CUIWindow;
class CUIObject;
class CAdResponse;
class CAdResponseBox : public CBObject {
public:
	CBObject *getNextAccessObject(CBObject *CurrObject);
	CBObject *getPrevAccessObject(CBObject *CurrObject);
	HRESULT getObjects(CBArray<CUIObject *, CUIObject *> &Objects, bool InteractiveOnly);

	HRESULT handleResponse(CAdResponse *Response);
	void setLastResponseText(const char *Text, const char *TextOrig);
	char *_lastResponseText;
	char *_lastResponseTextOrig;
	DECLARE_PERSISTENT(CAdResponseBox, CBObject)
	CScScript *_waitingScript;
	virtual HRESULT listen(CBScriptHolder *param1, uint32 param2);
	typedef enum {
	    EVENT_PREV, EVENT_NEXT, EVENT_RESPONSE
	} TResponseEvent;

	HRESULT weedResponses();
	HRESULT display();
	int _spacing;
	int _scrollOffset;
	CBFont *_fontHover;
	CBFont *_font;
	HRESULT createButtons();
	HRESULT invalidateButtons();
	void clearButtons();
	void clearResponses();
	CAdResponseBox(CBGame *inGame);
	virtual ~CAdResponseBox();
	CBArray<CAdResponse *, CAdResponse *> _responses;
	CBArray<CUIButton *, CUIButton *> _respButtons;
	CUIWindow *_window;
	CUIWindow *_shieldWindow;
	bool _horizontal;
	RECT _responseArea;
	int _verticalAlign;
	TTextAlign _align;
	HRESULT loadFile(const char *Filename);
	HRESULT loadBuffer(byte  *Buffer, bool Complete = true);
	virtual HRESULT saveAsText(CBDynBuffer *Buffer, int Indent);
};

} // end of namespace WinterMute

#endif
