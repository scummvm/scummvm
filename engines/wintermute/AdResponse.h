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

#ifndef __WmeAdResponse_H__
#define __WmeAdResponse_H__


#include "BObject.h"
#include "AdTypes.h"

namespace WinterMute {
class CBFont;
class CAdResponse : public CBObject {
public:
	DECLARE_PERSISTENT(CAdResponse, CBObject)
	HRESULT SetIcon(const char *Filename);
	HRESULT SetFont(const char *Filename);
	HRESULT SetIconHover(const char *Filename);
	HRESULT SetIconPressed(const char *Filename);
	void SetText(const char *Text);
	int _iD;
	CBSprite *_icon;
	CBSprite *_iconHover;
	CBSprite *_iconPressed;
	CBFont *_font;
	char *_text;
	char *_textOrig;
	CAdResponse(CBGame *inGame);
	virtual ~CAdResponse();
	TResponseType _responseType;

};

} // end of namespace WinterMute

#endif
