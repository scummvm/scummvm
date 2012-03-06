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

#ifndef __WmeAdResponseBox_H__
#define __WmeAdResponseBox_H__


#include "BObject.h"

namespace WinterMute {

class CUIButton;
class CUIWindow;
class CUIObject;
class CAdResponse;
class CAdResponseBox : public CBObject {
public:
	CBObject *GetNextAccessObject(CBObject *CurrObject);
	CBObject *GetPrevAccessObject(CBObject *CurrObject);
	HRESULT GetObjects(CBArray<CUIObject *, CUIObject *>& Objects, bool InteractiveOnly);

	HRESULT HandleResponse(CAdResponse *Response);
	void SetLastResponseText(char *Text, char *TextOrig);
	char *m_LastResponseText;
	char *m_LastResponseTextOrig;
	DECLARE_PERSISTENT(CAdResponseBox, CBObject)
	CScScript *m_WaitingScript;
	virtual HRESULT Listen(CBScriptHolder *param1, uint32 param2);
	typedef enum {
	    EVENT_PREV, EVENT_NEXT, EVENT_RESPONSE
	} TResponseEvent;

	HRESULT WeedResponses();
	HRESULT Display();
	int m_Spacing;
	int m_ScrollOffset;
	CBFont *m_FontHover;
	CBFont *m_Font;
	HRESULT CreateButtons();
	HRESULT InvalidateButtons();
	void ClearButtons();
	void ClearResponses();
	CAdResponseBox(CBGame *inGame);
	virtual ~CAdResponseBox();
	CBArray<CAdResponse *, CAdResponse *> m_Responses;
	CBArray<CUIButton *, CUIButton *> m_RespButtons;
	CUIWindow *m_Window;
	CUIWindow *m_ShieldWindow;
	bool m_Horizontal;
	RECT m_ResponseArea;
	int m_VerticalAlign;
	TTextAlign m_Align;
	HRESULT LoadFile(char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
};

} // end of namespace WinterMute

#endif
