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
#include "BFader.h"
#include "BGame.h"
#include "PlatformSDL.h"
#include "common/util.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBFader, false)

//////////////////////////////////////////////////////////////////////////
CBFader::CBFader(CBGame *inGame): CBObject(inGame) {
	m_Active = false;
	m_Red = m_Green = m_Blue = 0;
	m_CurrentAlpha = 0x00;
	m_SourceAlpha = 0;
	m_TargetAlpha = 0;
	m_Duration = 1000;
	m_StartTime = 0;
	m_System = false;
}


//////////////////////////////////////////////////////////////////////////
CBFader::~CBFader() {

}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::Update() {
	if (!m_Active) return S_OK;

	int AlphaDelta = m_TargetAlpha - m_SourceAlpha;

	uint32 time;

	if (m_System) time = CBPlatform::GetTime() - m_StartTime;
	else time = Game->m_Timer - m_StartTime;

	if (time >= m_Duration) m_CurrentAlpha = m_TargetAlpha;
	else {
		m_CurrentAlpha = m_SourceAlpha + (float)time / (float)m_Duration * AlphaDelta;
	}
	m_CurrentAlpha = MIN((unsigned char)255, MAX(m_CurrentAlpha, (byte )0)); // TODO: clean

	m_Ready = time >= m_Duration;
	if (m_Ready && m_CurrentAlpha == 0x00) m_Active = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::Display() {
	if (!m_Active) return S_OK;

	if (m_CurrentAlpha > 0x00) return Game->m_Renderer->FadeToColor(DRGBA(m_Red, m_Green, m_Blue, m_CurrentAlpha));
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::Deactivate() {
	m_Active = false;
	m_Ready = true;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::FadeIn(uint32 SourceColor, uint32 Duration, bool System) {
	m_Ready = false;
	m_Active = true;

	m_Red   = D3DCOLGetR(SourceColor);
	m_Green = D3DCOLGetG(SourceColor);
	m_Blue  = D3DCOLGetB(SourceColor);

	m_SourceAlpha = D3DCOLGetA(SourceColor);
	m_TargetAlpha = 0;

	m_Duration = Duration;
	m_System = System;

	if (m_System) m_StartTime = CBPlatform::GetTime();
	else m_StartTime = Game->m_Timer;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::FadeOut(uint32 TargetColor, uint32 Duration, bool System) {
	m_Ready = false;
	m_Active = true;

	m_Red   = D3DCOLGetR(TargetColor);
	m_Green = D3DCOLGetG(TargetColor);
	m_Blue  = D3DCOLGetB(TargetColor);

	//m_SourceAlpha = 0;
	m_SourceAlpha = m_CurrentAlpha;
	m_TargetAlpha = D3DCOLGetA(TargetColor);

	m_Duration = Duration;
	m_System = System;

	if (m_System) m_StartTime = CBPlatform::GetTime();
	else m_StartTime = Game->m_Timer;


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBFader::GetCurrentColor() {
	return DRGBA(m_Red, m_Green, m_Blue, m_CurrentAlpha);
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Active));
	PersistMgr->Transfer(TMEMBER(m_Blue));
	PersistMgr->Transfer(TMEMBER(m_CurrentAlpha));
	PersistMgr->Transfer(TMEMBER(m_Duration));
	PersistMgr->Transfer(TMEMBER(m_Green));
	PersistMgr->Transfer(TMEMBER(m_Red));
	PersistMgr->Transfer(TMEMBER(m_SourceAlpha));
	PersistMgr->Transfer(TMEMBER(m_StartTime));
	PersistMgr->Transfer(TMEMBER(m_TargetAlpha));
	PersistMgr->Transfer(TMEMBER(m_System));

	if (m_System && !PersistMgr->m_Saving) m_StartTime = 0;

	return S_OK;
}

} // end of namespace WinterMute
