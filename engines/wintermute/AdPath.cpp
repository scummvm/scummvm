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

#include "dcgf.h"
#include "AdPath.h"
#include "BPoint.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdPath, false)

//////////////////////////////////////////////////////////////////////////
CAdPath::CAdPath(CBGame *inGame): CBBase(inGame) {
	_currIndex = -1;
	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
CAdPath::~CAdPath() {
	Reset();
}


//////////////////////////////////////////////////////////////////////////
void CAdPath::Reset() {
	for (int i = 0; i < _points.GetSize(); i++)
		delete _points[i];

	_points.RemoveAll();
	_currIndex = -1;
	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetFirst() {
	if (_points.GetSize() > 0) {
		_currIndex = 0;
		return _points[_currIndex];
	} else return NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetNext() {
	_currIndex++;
	if (_currIndex < _points.GetSize()) return _points[_currIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetCurrent() {
	if (_currIndex >= 0 && _currIndex < _points.GetSize()) return _points[_currIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdPath::AddPoint(CBPoint *point) {
	_points.Add(point);
}


//////////////////////////////////////////////////////////////////////////
bool CAdPath::SetReady(bool ready) {
	bool orig = _ready;
	_ready = ready;

	return orig;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdPath::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(_currIndex));
	_points.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_ready));

	return S_OK;
}

} // end of namespace WinterMute
