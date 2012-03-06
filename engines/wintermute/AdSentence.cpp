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
#include "AdSentence.h"
#include "AdTalkDef.h"
#include "AdTalkNode.h"
#include "AdGame.h"
#include "PathUtil.h"
#include "BGame.h"
#include "BSound.h"
#include "AdScene.h"
#include "BFont.h"
#include "BSprite.h"
#include "BFileManager.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSentence, false)

//////////////////////////////////////////////////////////////////////////
CAdSentence::CAdSentence(CBGame *inGame): CBBase(inGame) {
	m_Text = NULL;
	m_Stances = NULL;
	m_TempStance = NULL;

	m_Duration = 0;
	m_StartTime = 0;
	m_CurrentStance = 0;

	m_Font = NULL;

	m_Pos.x = m_Pos.y = 0;
	m_Width = Game->m_Renderer->m_Width;

	m_Align = (TTextAlign)TAL_CENTER;

	m_Sound = NULL;
	m_SoundStarted = false;

	m_TalkDef = NULL;
	m_CurrentSprite = NULL;
	m_CurrentSkelAnim = NULL;
	m_FixedPos = false;
	m_Freezable = true;
}


//////////////////////////////////////////////////////////////////////////
CAdSentence::~CAdSentence() {
	delete m_Sound;
	delete[] m_Text;
	delete[] m_Stances;
	delete[] m_TempStance;
	delete m_TalkDef;
	m_Sound = NULL;
	m_Text = NULL;
	m_Stances = NULL;
	m_TempStance = NULL;
	m_TalkDef = NULL;

	m_CurrentSprite = NULL; // ref only
	m_CurrentSkelAnim = NULL;
	m_Font = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetText(char *Text) {
	if (m_Text) delete [] m_Text;
	m_Text = new char[strlen(Text) + 1];
	if (m_Text) strcpy(m_Text, Text);
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetStances(char *Stances) {
	if (m_Stances) delete [] m_Stances;
	if (Stances) {
		m_Stances = new char[strlen(Stances) + 1];
		if (m_Stances) strcpy(m_Stances, Stances);
	} else m_Stances = NULL;
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetCurrentStance() {
	return GetStance(m_CurrentStance);
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetNextStance() {
	m_CurrentStance++;
	return GetStance(m_CurrentStance);
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetStance(int Stance) {
	if (m_Stances == NULL) return NULL;

	if (m_TempStance) delete [] m_TempStance;
	m_TempStance = NULL;

	char *start;
	char *curr;
	int pos;

	if (Stance == 0) start = m_Stances;
	else {
		pos = 0;
		start = NULL;
		curr = m_Stances;
		while (pos < Stance) {
			if (*curr == '\0') break;
			if (*curr == ',') pos++;
			curr++;
		}
		if (pos == Stance) start = curr;
	}

	if (start == NULL) return NULL;

	while (*start == ' ' && *start != ',' && *start != '\0') start++;

	curr = start;
	while (*curr != '\0' && *curr != ',') curr++;

	while (curr > start && *(curr - 1) == ' ') curr--;

	m_TempStance = new char [curr - start + 1];
	if (m_TempStance) {
		m_TempStance[curr - start] = '\0';
		strncpy(m_TempStance, start, curr - start);
	}

	return m_TempStance;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Display() {
	if (!m_Font || !m_Text) return E_FAIL;

	if (m_Sound && !m_SoundStarted) {
		m_Sound->Play();
		m_SoundStarted = true;
	}

	if (Game->m_Subtitles) {
		int x = m_Pos.x;
		int y = m_Pos.y;

		if (!m_FixedPos) {
			x = x - ((CAdGame *)Game)->m_Scene->GetOffsetLeft();
			y = y - ((CAdGame *)Game)->m_Scene->GetOffsetTop();
		}


		x = std::max(x, 0);
		x = std::min(x, Game->m_Renderer->m_Width - m_Width);
		y = std::max(y, 0);

		m_Font->DrawText((byte  *)m_Text, x, y, m_Width, m_Align);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetSound(CBSound *Sound) {
	if (!Sound) return;
	delete m_Sound;
	m_Sound = Sound;
	m_SoundStarted = false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Finish() {
	if (m_Sound) m_Sound->Stop();
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER_INT(m_Align));
	PersistMgr->Transfer(TMEMBER(m_CurrentStance));
	PersistMgr->Transfer(TMEMBER(m_CurrentSprite));
	PersistMgr->Transfer(TMEMBER(m_CurrentSkelAnim));
	PersistMgr->Transfer(TMEMBER(m_Duration));
	PersistMgr->Transfer(TMEMBER(m_Font));
	PersistMgr->Transfer(TMEMBER(m_Pos));
	PersistMgr->Transfer(TMEMBER(m_Sound));
	PersistMgr->Transfer(TMEMBER(m_SoundStarted));
	PersistMgr->Transfer(TMEMBER(m_Stances));
	PersistMgr->Transfer(TMEMBER(m_StartTime));
	PersistMgr->Transfer(TMEMBER(m_TalkDef));
	PersistMgr->Transfer(TMEMBER(m_TempStance));
	PersistMgr->Transfer(TMEMBER(m_Text));
	PersistMgr->Transfer(TMEMBER(m_Width));
	PersistMgr->Transfer(TMEMBER(m_FixedPos));
	PersistMgr->Transfer(TMEMBER(m_Freezable));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::SetupTalkFile(char *SoundFilename) {
	delete m_TalkDef;
	m_TalkDef = NULL;
	m_CurrentSprite = NULL;

	if (!SoundFilename) return S_OK;


	AnsiString path = PathUtil::GetDirectoryName(SoundFilename);
	AnsiString name = PathUtil::GetFileNameWithoutExtension(SoundFilename);

	AnsiString talkDefFileName = PathUtil::Combine(path, name + ".talk");

	CBFile *file = Game->m_FileManager->OpenFile(talkDefFileName.c_str());
	if (file) {
		Game->m_FileManager->CloseFile(file);
	} else return S_OK; // no talk def file found


	m_TalkDef = new CAdTalkDef(Game);
	if (!m_TalkDef || FAILED(m_TalkDef->LoadFile(talkDefFileName.c_str()))) {
		delete m_TalkDef;
		m_TalkDef = NULL;
		return E_FAIL;
	}
	//Game->LOG(0, "Using .talk file: %s", TalkDefFile);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Update(TDirection Dir) {
	if (!m_TalkDef) return S_OK;

	uint32 CurrentTime;
	// if sound is available, synchronize with sound, otherwise use timer

	/*
	if(m_Sound) CurrentTime = m_Sound->GetPositionTime();
	else CurrentTime = Game->m_Timer - m_StartTime;
	*/
	CurrentTime = Game->m_Timer - m_StartTime;

	bool TalkNodeFound = false;
	for (int i = 0; i < m_TalkDef->m_Nodes.GetSize(); i++) {
		if (m_TalkDef->m_Nodes[i]->IsInTimeInterval(CurrentTime, Dir)) {
			TalkNodeFound = true;

			CBSprite *NewSprite = m_TalkDef->m_Nodes[i]->GetSprite(Dir);
			if (NewSprite != m_CurrentSprite) NewSprite->Reset();
			m_CurrentSprite = NewSprite;

			if (!m_TalkDef->m_Nodes[i]->m_PlayToEnd) break;
		}
	}


	// no talk node, try to use default sprite instead (if any)
	if (!TalkNodeFound) {
		CBSprite *NewSprite = m_TalkDef->GetDefaultSprite(Dir);
		if (NewSprite) {
			if (NewSprite != m_CurrentSprite) NewSprite->Reset();
			m_CurrentSprite = NewSprite;
		} else m_CurrentSprite = NULL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdSentence::CanSkip() {
	// prevent accidental sentence skipping (TODO make configurable)
	return (Game->m_Timer - m_StartTime) > 300;
}

} // end of namespace WinterMute
