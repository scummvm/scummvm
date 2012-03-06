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

#ifndef __WmeAdInventoryBox_H__
#define __WmeAdInventoryBox_H__

#include "BObject.h"

namespace WinterMute {
class CUIButton;
class CUIWindow;

class CAdInventoryBox : public CBObject {
public:
	bool m_HideSelected;
	DECLARE_PERSISTENT(CAdInventoryBox, CBObject)
	bool m_Exclusive;
	int m_ScrollBy;
	int m_ItemHeight;
	int m_ItemWidth;
	bool m_Visible;
	virtual HRESULT Display();
	CUIButton *m_CloseButton;
	int m_Spacing;
	int m_ScrollOffset;
	RECT m_ItemsArea;
	HRESULT Listen(CBScriptHolder *param1, uint32 param2);
	CUIWindow *m_Window;
	CAdInventoryBox(CBGame *inGame);
	virtual ~CAdInventoryBox();
	HRESULT LoadFile(char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
};

} // end of namespace WinterMute

#endif
