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

#include "AdPathPoint.h"
#include "BPersistMgr.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdPathPoint, false)

//////////////////////////////////////////////////////////////////////////
CAdPathPoint::CAdPathPoint() {
	x = y = 0;
	_distance = 0;

	_marked = false;
	_origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdPathPoint::CAdPathPoint(int initX, int initY, int initDistance) {
	x = initX;
	y = initY;
	_distance = initDistance;

	_marked = false;
	_origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdPathPoint::~CAdPathPoint() {
	_origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdPathPoint::Persist(CBPersistMgr *PersistMgr) {

	CBPoint::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_distance));
	PersistMgr->Transfer(TMEMBER(_marked));
	PersistMgr->Transfer(TMEMBER(_origin));

	return S_OK;
}

} // end of namespace WinterMute
