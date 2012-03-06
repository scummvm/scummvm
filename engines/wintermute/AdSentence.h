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

#ifndef __WmeAdSentence_H__
#define __WmeAdSentence_H__


#include "BBase.h"
#include "persistent.h"
#include "dctypes.h"    // Added by ClassView

namespace WinterMute {
class CAdTalkDef;
class CBFont;
class CBSprite;
class CBSound;
class CAdSentence : public CBBase {
public:
	bool m_Freezable;
	bool m_FixedPos;
	CBSprite *m_CurrentSprite;
	char *m_CurrentSkelAnim;
	HRESULT Update(TDirection Dir = DI_DOWN);
	HRESULT SetupTalkFile(char *SoundFilename);
	DECLARE_PERSISTENT(CAdSentence, CBBase)
	HRESULT Finish();
	void SetSound(CBSound *Sound);
	bool m_SoundStarted;
	CBSound *m_Sound;
	TTextAlign m_Align;
	HRESULT Display();
	int m_Width;
	POINT m_Pos;
	CBFont *m_Font;
	char *GetNextStance();
	char *GetCurrentStance();
	void SetStances(char *Stances);
	void SetText(char *Text);
	int m_CurrentStance;
	uint32 m_StartTime;
	char *m_Stances;
	char *m_Text;
	uint32 m_Duration;
	CAdSentence(CBGame *inGame);
	virtual ~CAdSentence();
	CAdTalkDef *m_TalkDef;

	bool CanSkip();

private:
	char *m_TempStance;
	char *GetStance(int Stance);

};

} // end of namespace WinterMute

#endif
