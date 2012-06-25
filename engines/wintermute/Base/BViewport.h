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

#ifndef WINTERMUTE_BVIEWPORT_H
#define WINTERMUTE_BVIEWPORT_H


#include "engines/wintermute/Base/BBase.h"

namespace WinterMute {
class CBObject;
class CBViewport : public CBBase {
public:
	int getHeight();
	int GetWidth();
	RECT *GetRect();
	HRESULT SetRect(int left, int top, int right, int bottom, bool NoCheck = false);
	DECLARE_PERSISTENT(CBViewport, CBBase)
	int _offsetY;
	int _offsetX;
	CBObject *_mainObject;
	CBViewport(CBGame *inGame = NULL);
	virtual ~CBViewport();
private:
	RECT _rect;
};

} // end of namespace WinterMute

#endif
