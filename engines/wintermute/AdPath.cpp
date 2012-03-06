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
	m_CurrIndex = -1;
	m_Ready = false;
}


//////////////////////////////////////////////////////////////////////////
CAdPath::~CAdPath() {
	Reset();
}


//////////////////////////////////////////////////////////////////////////
void CAdPath::Reset() {
	for (int i = 0; i < m_Points.GetSize(); i++)
		delete m_Points[i];

	m_Points.RemoveAll();
	m_CurrIndex = -1;
	m_Ready = false;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetFirst() {
	if (m_Points.GetSize() > 0) {
		m_CurrIndex = 0;
		return m_Points[m_CurrIndex];
	} else return NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetNext() {
	m_CurrIndex++;
	if (m_CurrIndex < m_Points.GetSize()) return m_Points[m_CurrIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
CBPoint *CAdPath::GetCurrent() {
	if (m_CurrIndex >= 0 && m_CurrIndex < m_Points.GetSize()) return m_Points[m_CurrIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdPath::AddPoint(CBPoint *point) {
	m_Points.Add(point);
}


//////////////////////////////////////////////////////////////////////////
bool CAdPath::SetReady(bool ready) {
	bool orig = m_Ready;
	m_Ready = ready;

	return orig;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdPath::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(m_CurrIndex));
	m_Points.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_Ready));

	return S_OK;
}

} // end of namespace WinterMute
