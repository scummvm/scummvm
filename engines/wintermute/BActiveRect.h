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

#ifndef WINTERMUTE_BACTIVERECT_H
#define WINTERMUTE_BACTIVERECT_H

#include "BBase.h"

namespace WinterMute {
class CBRegion;
class CBSubFrame;
class CBObject;
class CBActiveRect: CBBase {
public:
	void ClipRect();
	bool m_Precise;
	float m_ZoomX;
	float m_ZoomY;
	CBSubFrame *m_Frame;
	CBObject *m_Owner;
	CBRegion *m_Region;
	int m_OffsetX;
	int m_OffsetY;
	RECT m_Rect;
	CBActiveRect(CBGame *inGameOwner = NULL);
	CBActiveRect(CBGame *inGameOwner, CBObject *Owner, CBSubFrame *Frame, int X, int Y, int Width, int Height, float ZoomX = 100, float ZoomY = 100, bool Precise = true);
	CBActiveRect(CBGame *inGame, CBObject *Owner, CBRegion *Region, int OffsetX, int OffsetY);
	virtual ~CBActiveRect();

};

} // end of namespace WinterMute

#endif
