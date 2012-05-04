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
/**************************************************************************
 *                                     ออออออออออออออออออออออออออออออออออ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ออออออออออออออออออออออออออออออออออ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Loc.CPP..............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "common/file.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpalutils.h"
#include "tony/mpal/stubs.h"
#include "tony/custom.h"
#include "tony/game.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

using namespace MPAL;

/****************************************/
/*  Variabili globali di configurazione */
/****************************************/

// FIXME: Move into a class instance
bool bCfgInvLocked;
bool bCfgInvNoScroll;
bool bCfgTimerizedText;
bool bCfgInvUp;
bool bCfgAnni30;
bool bCfgAntiAlias;
bool bCfgSottotitoli;
bool bCfgTransparence;
bool bCfgInterTips;
bool bCfgDubbing;
bool bCfgMusic;
bool bCfgSFX;
int  nCfgTonySpeed;
int  nCfgTextSpeed;
int	 nCfgDubbingVolume;
int	 nCfgMusicVolume;
int	 nCfgSFXVolume;
bool bSkipSfxNoLoop;

bool bPatIrqFreeze;

/****************************************/
/*  Funzioni globali per la DLL Custom	*/
/****************************************/

static char path_buffer[MAX_PATH];

HANDLE MainLoadLocation(int nLoc, RMPoint pt, RMPoint start) {
	return _vm->GetEngine()->LoadLocation(nLoc, pt,start);
}

HANDLE MainUnloadLocation(bool bDoOnExit) {
	return _vm->GetEngine()->UnloadLocation(bDoOnExit);
}

void MainLinkGraphicTask(RMGfxTask *task) {
	_vm->GetEngine()->LinkGraphicTask(task);
}

void MainFreeze(void) {
	_vm->GetEngine()->Freeze();
}

void MainUnfreeze(void) {
	_vm->GetEngine()->Unfreeze();
}

void MainWaitFrame(void) {
	WaitForSingleObject(_vm->m_hEndOfFrame, INFINITE);
}

void MainShowMouse(void) {
  _vm->GetEngine()->EnableMouse();
}

void MainHideMouse(void) {
  _vm->GetEngine()->DisableMouse();
}

void MainPlayMusic(int nChannel, const char *filename, int nFX, bool bLoop, int nSync) {
	_vm->PlayMusic(nChannel, filename, nFX, bLoop, nSync);
}

void MainDisableInput(void) {
	_vm->GetEngine()->DisableInput();
}

void MainEnableInput(void) {
	_vm->GetEngine()->EnableInput();
}

void MainInitWipe(int type) {
	_vm->GetEngine()->InitWipe(type);
}

void MainCloseWipe(void) {
	_vm->GetEngine()->CloseWipe();
}

void MainWaitWipeEnd(void) {
	_vm->GetEngine()->WaitWipeEnd();
}

void MainEnableGUI(void) {
	_vm->GetEngine()->m_bGUIInterface = true;
	_vm->GetEngine()->m_bGUIInventory = true;
	_vm->GetEngine()->m_bGUIOption = true;
}

void MainDisableGUI(void) {
	_vm->GetEngine()->m_bGUIInterface = false;
	_vm->GetEngine()->m_bGUIInventory = false;
	_vm->GetEngine()->m_bGUIOption = false;
}

void MainSetPalesati(bool bPalesati) {
	_vm->GetEngine()->SetPalesati(bPalesati);
}

/****************************************************************************\
*       Metodi di RMOptionButton
\****************************************************************************/

RMOptionButton::RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState) {
	RMResRaw raw(dwRes);
	assert(raw.IsValid());
	m_buf = new RMGfxSourceBuffer16(false);
	m_buf->Init(raw, raw.Width(), raw.Height());

	m_rect.SetRect(pt.x, pt.y, pt.x + raw.Width() - 1, pt.y + raw.Height() - 1);
	m_bActive = false;
	m_bHasGfx = true;
	m_bDoubleState = bDoubleState;
}

RMOptionButton::RMOptionButton(RMRect pt) {
	m_rect = pt;
	m_bActive = false;
	m_bHasGfx = false;
	m_bDoubleState = false;
}

RMOptionButton::~RMOptionButton() {
	if (m_bHasGfx)
		delete m_buf;
}

bool RMOptionButton::DoFrame(RMPoint mousePos, bool bLeftClick, bool bRightClick) {
	if (!m_bDoubleState) {
		if (m_rect.PtInRect(mousePos)) {
			if (!m_bActive) {
				m_bActive = true;
				return true;
			}
		} else {
			if (m_bActive) {
				m_bActive=false;
				return true;
			}
		}
	} else {
		if (bLeftClick && m_rect.PtInRect(mousePos)) {	
			m_bActive = !m_bActive;
			return true;
		}
	}

	return false;
}



void RMOptionButton::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (!m_bActive)
		return;

	if (m_bHasGfx)
		m_buf->Draw(bigBuf,prim);
}

void RMOptionButton::AddToList(RMGfxTargetBuffer &bigBuf) {
	if (m_bHasGfx)
		bigBuf.AddPrim(new RMGfxPrimitive(this, m_rect));
}

/****************************************************************************\
*       Metodi di RMOptionSlide
\****************************************************************************/

RMOptionSlide::RMOptionSlide(RMPoint pt, int nRange, int nStartValue, int slideSize) {
	RMResRaw *raw;

	m_pos = pt;
	m_nSlideSize = slideSize;
	m_nMax = nRange;
	m_nStep = 100 / m_nMax;
	m_nValue = nStartValue;

	m_SliderCenter = NULL;
	m_SliderLeft = NULL;
	m_SliderRight = NULL;
	m_SliderSingle = NULL;

	// Sliders
	INIT_GFX16_FROMRAW(20029, m_SliderCenter);
	INIT_GFX16_FROMRAW(20030, m_SliderLeft);
	INIT_GFX16_FROMRAW(20031, m_SliderRight);
	INIT_GFX16_FROMRAW(20032, m_SliderSingle);

	// Buttons
	m_PushLeft = new RMOptionButton(RMRect(pt.x - 23, pt.y, pt.x - 23 + 22, pt.y + 26));
	m_PushRight = new RMOptionButton(RMRect(pt.x + m_nSlideSize, pt.y, pt.x + m_nSlideSize + 5 + 22, pt.y + 26));
}


RMOptionSlide::~RMOptionSlide() {
	delete m_SliderCenter;
	m_SliderCenter = NULL;
	delete m_SliderLeft;
	m_SliderLeft = NULL;
	delete m_SliderRight;
	m_SliderRight = NULL;
	delete m_SliderSingle;
	m_SliderSingle = NULL;

	delete m_PushLeft;
	m_PushLeft = NULL;
	delete m_PushRight;
	m_PushRight = NULL;
}

bool RMOptionSlide::DoFrame(RMPoint mousePos, bool bLeftClick, bool bRightClick) {
	bool bRefresh = false;

	// Doframe dei bottoni
	m_PushLeft->DoFrame(mousePos, bLeftClick, bRightClick);
	m_PushRight->DoFrame(mousePos, bLeftClick, bRightClick);

	if (m_PushLeft->IsActive()) {
		if (bLeftClick) {
			bRefresh = true;
			m_nValue--;
		} else if (bRightClick) {
			bRefresh = true;
			m_nValue -= 3;
		}
		if (m_nValue < 1)
			m_nValue = 1;
	} else if (m_PushRight->IsActive()) {
		bRefresh = true;

		if (bLeftClick) {
			bRefresh = true;
			m_nValue++;
		} else if (bRightClick) {
			bRefresh = true;
			m_nValue += 3;
		}
		if (m_nValue > m_nMax)
			m_nValue = m_nMax;
	}

	return bRefresh;
}

void RMOptionSlide::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int i;
	int val;

	RMPoint pos;
	pos = m_pos;
	pos.x += 4;
	pos.y += 4;

	val = m_nValue * m_nStep;
	if (val < 1) val = 1;
	else if (val > 100) val = 100;

	if (val == 1) {
		prim->SetDst(pos);
		m_SliderSingle->Draw(bigBuf, prim);
	} else {
		prim->SetDst(pos);
		m_SliderLeft->Draw(bigBuf, prim);
		pos.x += 3;

		for (i = 1; i < val - 1; i++) {
			prim->SetDst(pos);
			m_SliderCenter->Draw(bigBuf, prim);
			pos.x += 3;
		}

		prim->SetDst(pos);
		m_SliderRight->Draw(bigBuf, prim);
		pos.x += 3;
	}
}

void RMOptionSlide::AddToList(RMGfxTargetBuffer &bigBuf) {
	bigBuf.AddPrim(new RMGfxPrimitive(this));
}



/****************************************************************************\
*       Metodi di RMOptionScreen
\****************************************************************************/

RMOptionScreen::RMOptionScreen(void) {
	m_nState = MENUGAME;
	m_menu = NULL;
	m_HideLoadSave = NULL;
	m_QuitConfirm = NULL;

	Create(RM_SX, RM_SY);

	m_ButtonExit = NULL;
	m_ButtonLoad = NULL;
	m_ButtonSave = NULL;
	m_ButtonGameMenu = NULL;
	m_ButtonGfxMenu = NULL;
	m_ButtonSoundMenu = NULL;
	m_ButtonSave_ArrowLeft = NULL;
	m_ButtonSave_ArrowRight = NULL;
	m_bEditSaveName = false;

	int i;

	for (i = 0; i < 6; i++) {
		m_curThumb[i] = NULL;
		m_ButtonSave_States[i] = NULL;
	}

	m_statePos = 0;
}

RMOptionScreen::~RMOptionScreen(void) {
}

void RMOptionScreen::RefreshAll(void) {
	ClearOT();

	AddPrim(new RMGfxPrimitive(m_menu));
	
	if (m_bNoLoadSave)
		AddPrim(new RMGfxPrimitive(m_HideLoadSave,RMPoint(0, 401)));

	if (m_bQuitConfirm) {
		AddPrim(new RMGfxPrimitive(m_QuitConfirm,RMPoint(270, 200)));
		m_ButtonQuitYes->AddToList(*this);
		m_ButtonQuitNo->AddToList(*this);
	}

	m_ButtonExit->AddToList(*this);

	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
		m_ButtonQuit->AddToList(*this);
		m_ButtonLoad->AddToList(*this);
		m_ButtonSave->AddToList(*this);
	}

	if (m_nState == MENUGAME) {
		m_ButtonGame_Lock->AddToList(*this);
		m_ButtonGame_TimerizedText->AddToList(*this);
		m_ButtonGame_Scrolling->AddToList(*this);
		m_ButtonGame_InterUp->AddToList(*this);
		m_SlideTextSpeed->AddToList(*this);
		m_SlideTonySpeed->AddToList(*this);
	} else if (m_nState == MENUGFX) {
		m_ButtonGfx_Anni30->AddToList(*this);
		m_ButtonGfx_AntiAlias->AddToList(*this);
		m_ButtonGfx_Sottotitoli->AddToList(*this);
		m_ButtonGfx_Trans->AddToList(*this);
		m_ButtonGfx_Tips->AddToList(*this);
	} else if (m_nState == MENUSOUND) {
		m_SliderSound_Dubbing->AddToList(*this);
		m_SliderSound_Music->AddToList(*this);
		m_SliderSound_SFX->AddToList(*this);
		m_ButtonSound_DubbingOn->AddToList(*this);
		m_ButtonSound_MusicOn->AddToList(*this);
		m_ButtonSound_SFXOn->AddToList(*this);
	}

	RMGfxSourceBuffer16 *thumb = NULL;
	RMText* title = NULL;
	RMText* num[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
	int i;

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		title = new RMText;
		if (m_nState == MENULOAD) {
			RMMessage msg(10);
			title->WriteText(msg[0], 1);
		} else {
			RMMessage msg(11);
			title->WriteText(msg[0], 1);
		}

		AddPrim(new RMGfxPrimitive(title, RMPoint(320, 10)));

		if (m_curThumbDiff[0] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(48, 57)));
		else if (m_curThumbDiff[0] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(48, 57)));
		if (m_curThumbDiff[1] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(240, 57)));
		else if (m_curThumbDiff[1] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy,RMPoint(240, 57)));
		if (m_curThumbDiff[2] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard,RMPoint(432, 57)));
		else if (m_curThumbDiff[2] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy,RMPoint(432, 57)));
		if (m_curThumbDiff[3] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard,RMPoint(48, 239)));
		else if (m_curThumbDiff[3] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy,RMPoint(48, 239)));
		if (m_curThumbDiff[4] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard,RMPoint(240, 239)));
		else if (m_curThumbDiff[4] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy,RMPoint(240, 239)));
		if (m_curThumbDiff[5] == 0) AddPrim(new RMGfxPrimitive(m_SaveHard,RMPoint(432, 239)));
		else if (m_curThumbDiff[5] == 1) AddPrim(new RMGfxPrimitive(m_SaveEasy,RMPoint(432, 239)));

		if (m_curThumb[0] && !(m_bEditSaveName && m_nEditPos == 0)) AddPrim(new RMGfxPrimitive(m_curThumb[0], RMPoint(48, 57)));
		if (m_curThumb[1] && !(m_bEditSaveName && m_nEditPos == 1)) AddPrim(new RMGfxPrimitive(m_curThumb[1], RMPoint(240, 57)));
		if (m_curThumb[2] && !(m_bEditSaveName && m_nEditPos == 2)) AddPrim(new RMGfxPrimitive(m_curThumb[2], RMPoint(432, 57)));
		if (m_curThumb[3] && !(m_bEditSaveName && m_nEditPos == 3)) AddPrim(new RMGfxPrimitive(m_curThumb[3], RMPoint(48, 239)));
		if (m_curThumb[4] && !(m_bEditSaveName && m_nEditPos == 4)) AddPrim(new RMGfxPrimitive(m_curThumb[4], RMPoint(240, 239)));
		if (m_curThumb[5] && !(m_bEditSaveName && m_nEditPos == 5)) AddPrim(new RMGfxPrimitive(m_curThumb[5], RMPoint(432, 239)));

		if (m_bEditSaveName) {
			thumb = new RMGfxSourceBuffer16;
			thumb->Init((byte *)_vm->GetThumbnail(), 640 / 4, 480 / 4);

			switch (m_nEditPos) {
			case 0:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(48, 57))); break;
			case 1:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(240, 57))); break;
			case 2:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(432, 57))); break;
			case 3:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(48, 239))); break;
			case 4:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(240, 239))); break;
			case 5:	AddPrim(new RMGfxPrimitive(thumb,RMPoint(432, 239))); break;
			}
		}

		for (i = 0; i < 6; i++) {
			RMString s;
			
			if (m_bEditSaveName && m_nEditPos == i)
				s.Format("%02d)%s*", m_statePos + i, m_EditName);
			else {
				if (m_statePos == 0 && i == 0)
					s.Format("Autosave");
				else
					s.Format("%02d)%s", m_statePos + i, (const char *)m_curThumbName[i]);
			}
			
			num[i] = new RMText;
			num[i]->SetAlignType(RMText::HLEFT, RMText::VTOP);
			num[i]->WriteText(s, 2);
		}
		
		AddPrim(new RMGfxPrimitive(num[0], RMPoint(55 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(num[1], RMPoint(247 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(num[2],RMPoint(439 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(num[3],RMPoint(55 - 3, 362 + 14)));
		AddPrim(new RMGfxPrimitive(num[4],RMPoint(247 - 3, 362 + 14)));
		AddPrim(new RMGfxPrimitive(num[5],RMPoint(439 - 3, 362 + 14)));
		
		m_ButtonSave_ArrowLeft->AddToList(*this);
		m_ButtonSave_ArrowRight->AddToList(*this);
	}
		
	DrawOT();

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (thumb) delete thumb;
		if (title) delete title;

		for (i = 0; i < 6; i++)
			if (num[i]) delete num[i];
	}
}

void RMOptionScreen::RefreshThumbnails(void) {
	int i;

	for (i = 0; i < 6; i++) {
		if (m_curThumb[i])
			delete m_curThumb[i];
		
		m_curThumb[i] = new RMGfxSourceBuffer16;
		m_curThumb[i]->Create(640 / 4, 480 / 4);
		if (!LoadThumbnailFromSaveState(m_statePos + i, *m_curThumb[i], m_curThumbName[i], m_curThumbDiff[i])) {
			delete m_curThumb[i];
			m_curThumb[i] = NULL;
			m_curThumbName[i] = "";
			m_curThumbDiff[i] = 11;
		}
	}
}


void RMOptionScreen::InitState(void) {
	RMResRaw *raw;
	
	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND)
		raw = new RMResRaw(20000 + m_nState);
	else if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (m_bAlterGfx)
			raw = new RMResRaw(20024);
		else
			raw = new RMResRaw(20003);
	} else {
		error("Invalid state");
	}

	assert(raw->IsValid());
	assert(m_menu == NULL);
	m_menu = new RMGfxSourceBuffer16(false);
	m_menu->Init(*raw, raw->Width(), raw->Height());
	delete raw;

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (m_bAlterGfx) {
			assert(m_ButtonExit == NULL);
			m_ButtonExit = new RMOptionButton(20025, RMPoint(561, 406));
		} else {
			assert(m_ButtonExit == NULL);
			m_ButtonExit = new RMOptionButton(20012, RMPoint(560, 404));
		}

		INIT_GFX8_FROMRAW(20036, m_SaveEasy);
		INIT_GFX8_FROMRAW(20037, m_SaveHard);

		RefreshThumbnails();

		assert(m_ButtonSave_States[0] == NULL);
		m_ButtonSave_States[0] = new RMOptionButton(RMRect(48, 57, 48 + 160, 57 + 120));
		assert(m_ButtonSave_States[1] == NULL);
		m_ButtonSave_States[1] = new RMOptionButton(RMRect(240, 57, 240 + 160, 57 + 120));
		assert(m_ButtonSave_States[2] == NULL);
		m_ButtonSave_States[2] = new RMOptionButton(RMRect(432, 57, 432 + 160, 57 + 120));
		assert(m_ButtonSave_States[3] == NULL);
		m_ButtonSave_States[3] = new RMOptionButton(RMRect(48, 239, 48 + 160, 239 + 120));
		assert(m_ButtonSave_States[4] == NULL);
		m_ButtonSave_States[4] = new RMOptionButton(RMRect(240, 239, 240 + 160, 239 + 120));
		assert(m_ButtonSave_States[5] == NULL);
		m_ButtonSave_States[5] = new RMOptionButton(RMRect(432, 239, 432 + 160, 239 + 120));

		if (m_bAlterGfx) {
			assert(m_ButtonSave_ArrowLeft == NULL);
			m_ButtonSave_ArrowLeft = new RMOptionButton(20026,RMPoint(3,196));
			assert(m_ButtonSave_ArrowRight == NULL);
			m_ButtonSave_ArrowRight = new RMOptionButton(20027, RMPoint(601, 197));
		} else {
			assert(m_ButtonSave_ArrowLeft == NULL);
			m_ButtonSave_ArrowLeft = new RMOptionButton(20013, RMPoint(0, 197));
			assert(m_ButtonSave_ArrowRight == NULL);
			m_ButtonSave_ArrowRight = new RMOptionButton(20014,RMPoint(601, 197));
		}
	} else if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
		assert(m_ButtonExit == NULL);
		m_ButtonExit = new RMOptionButton(20005, RMPoint(560, 405));
		assert(m_ButtonQuit == NULL);
		m_ButtonQuit = new RMOptionButton(20020, RMPoint(7, 408));
		assert(m_ButtonLoad == NULL);
		m_ButtonLoad = new RMOptionButton(20006, RMPoint(231, 401));
		assert(m_ButtonSave == NULL);
		m_ButtonSave = new RMOptionButton(20007, RMPoint(325, 401));

		assert(m_ButtonGameMenu == NULL);
		m_ButtonGameMenu = new RMOptionButton(RMRect(24, 32, 118, 64));
		assert(m_ButtonGfxMenu == NULL);
		m_ButtonGfxMenu = new RMOptionButton(RMRect(118, 32, 212, 64));
		assert(m_ButtonSoundMenu == NULL);
		m_ButtonSoundMenu = new RMOptionButton(RMRect(212, 32, 306, 64));

		raw = new RMResRaw(20021);
		assert(raw->IsValid());
		assert(m_QuitConfirm == NULL);
		m_QuitConfirm = new RMGfxSourceBuffer16(false);
		m_QuitConfirm->Init(*raw, raw->Width(), raw->Height());
		delete raw;

		assert(m_ButtonQuitYes == NULL);
		m_ButtonQuitYes = new RMOptionButton(20022, RMPoint(281, 265));
		m_ButtonQuitYes->SetPriority(30);
		assert(m_ButtonQuitNo == NULL);
		m_ButtonQuitNo = new RMOptionButton(20023,RMPoint(337, 264));
		m_ButtonQuitNo->SetPriority(30);

		if (m_bNoLoadSave) {
			raw = new RMResRaw(20028);
			assert(raw->IsValid());
			assert(m_HideLoadSave == NULL);
			m_HideLoadSave = new RMGfxSourceBuffer16(false);
			m_HideLoadSave->Init(*raw, raw->Width(), raw->Height());
			delete raw;
		}

		// Menu GAME
		if (m_nState == MENUGAME) {
			assert(m_ButtonGame_Lock == NULL);
			m_ButtonGame_Lock = new RMOptionButton(20008,RMPoint(176, 262), true);
			m_ButtonGame_Lock->SetActiveState(bCfgInvLocked);
			assert(m_ButtonGame_TimerizedText == NULL);
			m_ButtonGame_TimerizedText = new RMOptionButton(20009,RMPoint(463, 273), true);
			m_ButtonGame_TimerizedText->SetActiveState(!bCfgTimerizedText);
			assert(m_ButtonGame_Scrolling == NULL);
			m_ButtonGame_Scrolling = new RMOptionButton(20010, RMPoint(315, 263), true);
			m_ButtonGame_Scrolling->SetActiveState(bCfgInvNoScroll);
			assert(m_ButtonGame_InterUp == NULL);
			m_ButtonGame_InterUp = new RMOptionButton(20011,RMPoint(36, 258), true);
			m_ButtonGame_InterUp->SetActiveState(bCfgInvUp);

			assert(m_SlideTextSpeed == NULL);
			m_SlideTextSpeed = new RMOptionSlide(RMPoint(165, 122), 10, nCfgTextSpeed);
			assert(m_SlideTonySpeed == NULL);
			m_SlideTonySpeed = new RMOptionSlide(RMPoint(165, 226), 5, nCfgTonySpeed);
		}
		// Menu Graphics
		else if (m_nState == MENUGFX) {
			assert(m_ButtonGfx_Anni30 == NULL);
			m_ButtonGfx_Anni30 = new RMOptionButton(20015,RMPoint(247, 178), true);
			m_ButtonGfx_Anni30->SetActiveState(bCfgAnni30);
			assert(m_ButtonGfx_AntiAlias == NULL);
			m_ButtonGfx_AntiAlias = new RMOptionButton(20016, RMPoint(430, 83), true);
			m_ButtonGfx_AntiAlias->SetActiveState(!bCfgAntiAlias);
			assert(m_ButtonGfx_Sottotitoli == NULL);
			m_ButtonGfx_Sottotitoli = new RMOptionButton(20017,RMPoint(98, 82), true);
			m_ButtonGfx_Sottotitoli->SetActiveState(!bCfgSottotitoli);
			assert(m_ButtonGfx_Tips == NULL);
			m_ButtonGfx_Tips = new RMOptionButton(20018,RMPoint(431, 246), true);
			m_ButtonGfx_Tips->SetActiveState(bCfgInterTips);
			assert(m_ButtonGfx_Trans == NULL);
			m_ButtonGfx_Trans = new RMOptionButton(20019,RMPoint(126, 271), true);
			m_ButtonGfx_Trans->SetActiveState(!bCfgTransparence);

		} else if (m_nState == MENUSOUND) {
			assert(m_SliderSound_Dubbing == NULL);
			m_SliderSound_Dubbing = new RMOptionSlide(RMPoint(165, 122), 10, nCfgDubbingVolume);
			assert(m_SliderSound_Music == NULL);
			m_SliderSound_Music = new RMOptionSlide(RMPoint(165, 226), 10, nCfgMusicVolume);
			assert(m_SliderSound_SFX == NULL);
			m_SliderSound_SFX = new RMOptionSlide(RMPoint(165, 330), 10, nCfgSFXVolume);

			assert(m_ButtonSound_DubbingOn == NULL);
			m_ButtonSound_DubbingOn = new RMOptionButton(20033, RMPoint(339, 75), true);
			m_ButtonSound_DubbingOn->SetActiveState(bCfgDubbing);
			assert(m_ButtonSound_MusicOn == NULL);
			m_ButtonSound_MusicOn = new RMOptionButton(20034,RMPoint(338, 179), true);
			m_ButtonSound_MusicOn->SetActiveState(bCfgMusic);
			assert(m_ButtonSound_SFXOn == NULL);
			m_ButtonSound_SFXOn = new RMOptionButton(20035,RMPoint(338, 283), true);
			m_ButtonSound_SFXOn->SetActiveState(bCfgSFX);
		}
	}

	RefreshAll();
}

void RMOptionScreen::CloseState(void) {
	delete m_menu;
	m_menu = NULL;

	delete m_ButtonExit;
	m_ButtonExit = false;

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		int i;

		for (i = 0; i < 6; i++) {
			if (m_curThumb[i] != NULL) {
				delete m_curThumb[i];
				m_curThumb[i] = NULL;
			}
			
			delete m_ButtonSave_States[i];
			m_ButtonSave_States[i] = NULL;
		}

		delete m_ButtonSave_ArrowLeft;
		m_ButtonSave_ArrowLeft = NULL;
		delete m_ButtonSave_ArrowRight;
		m_ButtonSave_ArrowRight = NULL;

		delete m_SaveEasy;
		m_SaveEasy = NULL;
		delete m_SaveHard;
		m_SaveHard = NULL;
	}

	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
		delete m_ButtonQuit;
		m_ButtonQuit = NULL;
		delete m_ButtonLoad;
		m_ButtonLoad = NULL;
		delete m_ButtonSave;
		m_ButtonSave = NULL;
		delete m_ButtonGameMenu;
		m_ButtonGameMenu = NULL;
		delete m_ButtonGfxMenu;
		m_ButtonGfxMenu = NULL;
		delete m_ButtonSoundMenu;
		m_ButtonSoundMenu = NULL;
		delete m_QuitConfirm;
		m_QuitConfirm = NULL;
		delete m_ButtonQuitYes;
		m_ButtonQuitYes = NULL;
		delete m_ButtonQuitNo;
		m_ButtonQuitNo = NULL;

		if (m_bNoLoadSave) {
			delete m_HideLoadSave;
			m_HideLoadSave = NULL;
		}

		if (m_nState == MENUGAME) {
			bCfgInvLocked = m_ButtonGame_Lock->IsActive();
			delete m_ButtonGame_Lock;
			m_ButtonGame_Lock = NULL;

			bCfgTimerizedText = !m_ButtonGame_TimerizedText->IsActive();
			delete m_ButtonGame_TimerizedText;
			m_ButtonGame_TimerizedText = NULL;

			bCfgInvNoScroll = m_ButtonGame_Scrolling->IsActive();
			delete m_ButtonGame_Scrolling;
			m_ButtonGame_Scrolling = NULL;

			bCfgInvUp = m_ButtonGame_InterUp->IsActive();
			delete m_ButtonGame_InterUp;
			m_ButtonGame_InterUp = NULL;

			nCfgTextSpeed = m_SlideTextSpeed->GetValue();
			delete m_SlideTextSpeed;
			m_SlideTextSpeed = NULL;

			nCfgTonySpeed = m_SlideTonySpeed->GetValue();
			delete m_SlideTonySpeed;
			m_SlideTonySpeed = NULL;
		} else if (m_nState == MENUGFX) {
			bCfgAnni30 = m_ButtonGfx_Anni30->IsActive();
			delete m_ButtonGfx_Anni30;
			m_ButtonGfx_Anni30 = NULL;

			bCfgAntiAlias = !m_ButtonGfx_AntiAlias->IsActive();
			delete m_ButtonGfx_AntiAlias;
			m_ButtonGfx_AntiAlias = NULL;

			bCfgSottotitoli = !m_ButtonGfx_Sottotitoli->IsActive();
			delete m_ButtonGfx_Sottotitoli;
			m_ButtonGfx_Sottotitoli = NULL;

			bCfgInterTips = m_ButtonGfx_Tips->IsActive();
			delete m_ButtonGfx_Tips;
			m_ButtonGfx_Tips = NULL;

			bCfgTransparence = !m_ButtonGfx_Trans->IsActive();
			delete m_ButtonGfx_Trans;
			m_ButtonGfx_Trans = NULL;
		} else if (m_nState == MENUSOUND) {
			nCfgDubbingVolume = m_SliderSound_Dubbing->GetValue();
			delete m_SliderSound_Dubbing;
			m_SliderSound_Dubbing = NULL;
			
			nCfgMusicVolume = m_SliderSound_Music->GetValue();
			delete m_SliderSound_Music;
			m_SliderSound_Music = NULL;
			
			nCfgSFXVolume = m_SliderSound_SFX->GetValue();
			delete m_SliderSound_SFX;
			m_SliderSound_SFX = NULL;

			bCfgDubbing = m_ButtonSound_DubbingOn->IsActive();
			delete m_ButtonSound_DubbingOn;
			m_ButtonSound_DubbingOn = NULL;

			bCfgMusic = m_ButtonSound_MusicOn->IsActive();
			delete m_ButtonSound_MusicOn;
			m_ButtonSound_MusicOn = NULL;

			bCfgSFX = m_ButtonSound_SFXOn->IsActive();
			delete m_ButtonSound_SFXOn;
			m_ButtonSound_SFXOn = NULL;
		}
	}
}

void RMOptionScreen::ReInit(RMGfxTargetBuffer &bigBuf) {
	bigBuf.AddPrim(new RMGfxPrimitive(this));
}

bool RMOptionScreen::Init(RMGfxTargetBuffer &bigBuf) {
	if (m_FadeStep != 0)
		return false;

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = false;
	m_bNoLoadSave = false;
	m_bAlterGfx = false;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	if (m_nState == MENULOAD || m_nState == MENUSAVE)
		m_nState = MENUGAME;
	InitState();
   
	return true;
}

bool RMOptionScreen::InitLoadMenuOnly(RMGfxTargetBuffer &bigBuf, bool bAlternateGfx) {
	if (m_FadeStep != 0)
		return false;

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = true;
	m_bNoLoadSave = false;
	m_bAlterGfx = bAlternateGfx;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENULOAD;
	InitState();
   
	return true;
}

bool RMOptionScreen::InitSaveMenuOnly(RMGfxTargetBuffer &bigBuf, bool bAlternateGfx) {
	if (m_FadeStep != 0)
		return false;

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = true;
	m_bNoLoadSave = false;
	m_bAlterGfx = bAlternateGfx;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENUSAVE;
	InitState();
   
	return true;
}

bool RMOptionScreen::InitNoLoadSave(RMGfxTargetBuffer &bigBuf) {
	if (m_FadeStep != 0)
		return false;

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = false;
	m_bNoLoadSave = true;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENUGAME;
	InitState();
   
	return true;
}

bool RMOptionScreen::Close(void) {
	if (m_FadeStep != 6)
		return false;

	// Inizia il fade out
	m_FadeStep++;
	m_FadeTime = _vm->GetTime();
	return true;
}

bool RMOptionScreen::IsClosing(void) {
	return m_bExit;
}

int RMOptionScreen::Priority() {
	// Appena sotto il mouse
	return 190;
}

void RMOptionScreen::ChangeState(STATE newState) {
	CloseState();
	m_nLastState = m_nState;
	m_nState = newState;
	InitState();	
}

void RMOptionScreen::DoFrame(RMInput *input) {	
	bool bLeftClick, bRightClick;
	RMPoint mousePos;
	bool bRefresh;
	int i;

	// Se non ่ completamente aperto, non fare nulla
	if (m_FadeStep != 6)
		return;

	// Legge l'input
	mousePos = input->MousePos();
	bLeftClick = input->MouseLeftClicked();
	bRightClick = input->MouseRightClicked();

	bRefresh = false;

	if (m_bQuitConfirm) {
		bRefresh |= m_ButtonQuitYes->DoFrame(mousePos, bLeftClick, bRightClick);
		bRefresh |= m_ButtonQuitNo->DoFrame(mousePos, bLeftClick, bRightClick);
	} else {
		bRefresh |= m_ButtonExit->DoFrame(mousePos, bLeftClick, bRightClick);

		// Controlla se ha clickato sull'uscita
		if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
			// bottoni senza grafica...
			m_ButtonGameMenu->DoFrame(mousePos, bLeftClick, bRightClick);
			m_ButtonGfxMenu->DoFrame(mousePos, bLeftClick, bRightClick);
			m_ButtonSoundMenu->DoFrame(mousePos, bLeftClick, bRightClick);

			// bottoni con grafica
			if (!m_bNoLoadSave) {
				if (!_vm->getIsDemo()) {
					bRefresh |= m_ButtonLoad->DoFrame(mousePos, bLeftClick, bRightClick);
					bRefresh |= m_ButtonSave->DoFrame(mousePos, bLeftClick, bRightClick);
				}

				bRefresh |= m_ButtonQuit->DoFrame(mousePos, bLeftClick, bRightClick);
			}
		}

		switch (m_nState) {
		case MENUGAME:
			bRefresh |= m_ButtonGame_Lock->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGame_TimerizedText->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGame_Scrolling->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGame_InterUp->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_SlideTextSpeed->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_SlideTonySpeed->DoFrame(mousePos, bLeftClick, bRightClick);
			break;

		case MENUGFX:
			bRefresh |= m_ButtonGfx_Anni30->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGfx_AntiAlias->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGfx_Sottotitoli->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGfx_Tips->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonGfx_Trans->DoFrame(mousePos, bLeftClick, bRightClick);
			break;

		case MENUSOUND:
			bRefresh |= m_SliderSound_Dubbing->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_SliderSound_Music->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_SliderSound_SFX->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonSound_DubbingOn->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonSound_MusicOn->DoFrame(mousePos, bLeftClick, bRightClick);
			bRefresh |= m_ButtonSound_SFXOn->DoFrame(mousePos, bLeftClick, bRightClick);
			break;

		case MENULOAD:
		case MENUSAVE:
			for (i=0;i<6;i++)
				m_ButtonSave_States[i]->DoFrame(mousePos, bLeftClick, bRightClick);
	
			if (m_statePos > 0)
				bRefresh |= m_ButtonSave_ArrowLeft->DoFrame(mousePos, bLeftClick, bRightClick);
			if (m_statePos < 90)
				bRefresh |= m_ButtonSave_ArrowRight->DoFrame(mousePos, bLeftClick, bRightClick);
			break;			
		}
	}
		
#define KEYPRESS(c)		((GetAsyncKeyState(c)&0x8001)==0x8001)
#define PROCESS_CHAR(cod,c)  if (KEYPRESS(cod)) { \
	m_EditName[strlen(m_EditName) +1 ] = '\0'; m_EditName[strlen(m_EditName)] = c; bRefresh = true; }

	/**************** STATO BOTTONI **************/
	if (m_bEditSaveName) {
		if (KEYPRESS(Common::KEYCODE_BACKSPACE)) {
			if (m_EditName[0] != '\0') {
				m_EditName[strlen(m_EditName) - 1] = '\0';
				bRefresh = true;
			}
		}

		for (i=0;i<26 && strlen(m_EditName)<12;i++)
			if ((GetAsyncKeyState(Common::KEYCODE_LSHIFT) & 0x8000) ||
					(GetAsyncKeyState(Common::KEYCODE_RSHIFT) & 0x8000)) {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + i), i + 'A');
			} else {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + i), i + 'a');
			}

		for (i = 0; i < 10 && strlen(m_EditName) < 12; i++)
			PROCESS_CHAR((Common::KeyCode)((int)'0' + i), i + '0');
		
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_SPACE,' ');
		
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP0, '0');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP1, '1');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP2, '2');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP3, '3');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP4, '4');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP5, '5');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP6, '6');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP7, '7');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP8, '8');
		if (strlen(m_EditName)<12) PROCESS_CHAR(Common::KEYCODE_KP9, '9');

		// ANNULLA
		if (KEYPRESS(Common::KEYCODE_ESCAPE)) {	
			m_bEditSaveName = false;
			bRefresh = true;
		}

		// OK
		if (KEYPRESS(Common::KEYCODE_RETURN)) {
			m_bEditSaveName = false;
			_vm->SaveState(m_statePos + m_nEditPos, m_EditName);
			Close();
		}

	} else if (bLeftClick) {
		if (m_nState == MENULOAD || m_nState == MENUSAVE) {
			if (m_ButtonExit->IsActive()) {
				if (m_bLoadMenuOnly) {
					// Se ่ solo il menu di loading, chiudiamo
					Close();
				} else {
					ChangeState(m_nLastState);
					bRefresh = true;
				}
			} else if (m_ButtonSave_ArrowLeft->IsActive()) {
				if (m_statePos > 0) {
					m_statePos -= 6;
					if (m_statePos < 0) m_statePos = 0;
					m_ButtonSave_ArrowLeft->SetActiveState(false);
					bRefresh = true;
					RefreshThumbnails();
				}
			} else if (m_ButtonSave_ArrowRight->IsActive()) {
				if (m_statePos < 90) {
					m_statePos += 6;
					if (m_statePos > 90) m_statePos = 90;
					m_ButtonSave_ArrowRight->SetActiveState(false);
					bRefresh = true;
					RefreshThumbnails();
				}
			} else {
				for (i = 0; i < 6; i++)
					if (m_ButtonSave_States[i]->IsActive()) {		
						// C'่ da effettuare il salvataggio o il caricamento!!!!
						if (m_nState == MENULOAD && m_curThumb[i] != NULL) {
							// Caricamento
							_vm->LoadState(m_statePos+i);
							Close();
						} else if (m_nState == MENUSAVE && (m_statePos != 0 || i != 0)) {
							// Attiva la modalitเ di editing
							m_bEditSaveName = true;
							m_nEditPos = i;
							strcpy(m_EditName, m_curThumbName[i]);
							bRefresh = true;

							//_vm->SaveState(m_statePos+i,"No name");
							//Close();
						}

						break;
					}
			}
		}

		if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
			if (m_bQuitConfirm) {
				if (m_ButtonQuitNo->IsActive()) {
					m_bQuitConfirm = false;
					bRefresh = true;
				} else if (m_ButtonQuitYes->IsActive()) {
					m_bQuitConfirm = false;
					bRefresh = true;

					_vm->Quit();
				}
			} else {
				if (m_ButtonQuit->IsActive()) {
					m_bQuitConfirm = true;
					m_ButtonQuitNo->SetActiveState(false);
					m_ButtonQuitYes->SetActiveState(false);
					bRefresh = true;
				} else if (m_ButtonExit->IsActive())
					Close();
				else if (m_ButtonLoad->IsActive()) {
					ChangeState(MENULOAD);
					bRefresh = true;
				} else if (m_ButtonSave->IsActive()) {
					ChangeState(MENUSAVE);
					bRefresh = true;
				} else if (m_ButtonGameMenu->IsActive() && m_nState != MENUGAME) {
					ChangeState(MENUGAME);
					bRefresh = true;
				} else if (m_ButtonGfxMenu->IsActive() && m_nState != MENUGFX) {
					ChangeState(MENUGFX);
					bRefresh = true;
				} else if (m_ButtonSoundMenu->IsActive() && m_nState != MENUSOUND) {
					ChangeState(MENUSOUND);
					bRefresh = true;
				}

				if (m_nState == MENUGFX) {
					// Queste opzioni hanno effetto immediato
					if (m_ButtonGfx_Anni30->IsActive())
						bCfgAnni30 = true;
					else
						bCfgAnni30 = false;

					if (m_ButtonGfx_AntiAlias->IsActive())
						bCfgAntiAlias = false;
					else
						bCfgAntiAlias = true;

					if (m_ButtonGfx_Trans->IsActive())
						bCfgTransparence = false;
					else
						bCfgTransparence = true;
				}
			}
		}
	}

	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND)
		if (!m_bQuitConfirm && KEYPRESS(Common::KEYCODE_ESCAPE))
			Close();

	if (bRefresh)
		RefreshAll();
}


void RMOptionScreen::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int curTime = _vm->GetTime();

#define FADE_SPEED 20
#define SYNC	(curTime-m_FadeTime) / 25
	
	if (m_bExit)
		return;

	switch (m_FadeStep) {
	// Discesa veloce
	case 1:
		if (m_FadeTime == -1)
			m_FadeY += FADE_SPEED;
		else
			m_FadeY += FADE_SPEED*SYNC;
		if (m_FadeY > 480) {
			m_FadeY = 480;
			m_FadeStep++;
		}

		// Setta la parte da disegnare per lo scrolling
		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;
	
	// Rimbalzo 1
	case 2:
		m_FadeY -= FADE_SPEED / 2 * SYNC;
		if (m_FadeY < 400) {
			m_FadeY = 400;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;

	case 3:
		m_FadeY -= FADE_SPEED / 4 * SYNC;
		if (m_FadeY < 380) {
			m_FadeY = 380;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;

	// Rimbalzo 1 - 2
	case 4:
		m_FadeY += FADE_SPEED / 3 * SYNC;
		if (m_FadeY > 420) {
			m_FadeY = 420;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;

	case 5:
		m_FadeY += FADE_SPEED / 2 * SYNC;
		if (m_FadeY > 480) {
			m_FadeY = 480;
			m_FadeStep++;
			_vm->HideLocation();
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;

	// Menu ON
	case 6:
		break;

	// Menu OFF
	case 7:
		_vm->ShowLocation();
		m_FadeStep++;
		break;

	case 8:
		m_FadeY -= FADE_SPEED * SYNC;
		if (m_FadeY < 0) {
			m_FadeY = 0;
			m_FadeStep++;
		}
		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));
		break;

	// Ciao ciao!
	case 9:
		m_bExit = true;
		m_FadeStep = 0;

		// Libera la memoria			
		CloseState();
		return;

	default:
		m_FadeStep = 0;
		break;
	}

	m_FadeTime = curTime;

	RMGfxWoodyBuffer::Draw(bigBuf,prim);
}

bool RMOptionScreen::RemoveThis() {
	if (m_bExit)
		return true;

	return false;
}


bool RMOptionScreen::LoadThumbnailFromSaveState(int nState, byte *lpDestBuf, RMString& name, byte &diff) {	
	char buf[256];
	char namebuf[256]; int i;
	Common::File f;
	char id[4];
	
	// Pulisce la destinazione
	Common::fill(lpDestBuf, lpDestBuf + 160 * 120 * 2, 0);
	name = "No name";
	diff = 10;

	// Si fa dare il nome del salvataggio
	_vm->GetSaveStateFileName(nState, buf);
	
	// Guarda se esiste
	if (f.open(buf))
		return false;

	// Controlla se ่ giusto l'header
	f.read(id, 4);
	if (id[0] != 'R' || id[1] != 'M' || id[2] != 'S') {
		f.close();
		return false;
	}

	if (id[3] < 0x3) {
		// Versione vecchia, niente screenshot
		f.close();
		return true;
	}

	// legge lo screenshot
	if (id[3] >= 0x5) {
		byte *cmpbuf;
		uint32 cmpsize, size;

		cmpbuf = new byte[160 * 120 * 4];
		
		// Se la versione >= 5, ่ compresso!
		cmpsize = f.readUint32LE();
		f.read(cmpbuf, cmpsize);

		lzo1x_decompress(cmpbuf,cmpsize,lpDestBuf,&size);

		delete[] cmpbuf;
	} else
		f.read(lpDestBuf, 160 * 120 * 2);

	if (id[3] >= 0x5) {
		// Legge il livello di difficoltเ
		diff = f.readByte();
	}

	if (id[3] < 0x4) {
		// Versione vecchia, niente nome
		f.close();
		return true;
	}

	i = f.readByte();
	f.read(namebuf, i);
	namebuf[i] = '\0';
	name = namebuf;

	f.close();
	return true;
}


/****************************************************************************\
*       Metodi di RMPointer
\****************************************************************************/

RMPointer::RMPointer() {
	Common::fill(m_pointer, m_pointer + 16, (RMGfxSourceBuffer8 *)NULL);
}

RMPointer::~RMPointer() {
	Close();
}

void RMPointer::Init(void) {
	int i;

	for (i = 0; i < 5; i++) {
		RMResRaw res(RES_P_GO + i);

		m_pointer[i] = new RMGfxSourceBuffer8RLEByteAA;
		m_pointer[i]->Init(res, res.Width(), res.Height(), false);
		m_pointer[i]->LoadPaletteWA(RES_P_PAL);
	}

	for (i = 0; i < 5; i++) {
		RMRes res(RES_P_PAP1 + i);
		RMDataStream ds;

		ds.OpenBuffer(res);
		m_specialPointer[i] = new RMItem;
		ds >> *m_specialPointer[i];
	}
	
	//m_hotspot[0].Set(19,5);
	m_hotspot[0].Set(5, 1);
	m_hotspot[1].Set(32, 28);
	m_hotspot[2].Set(45, 23);
	m_hotspot[3].Set(35, 25);
	m_hotspot[4].Set(32, 28);

	// Default=GO
	m_nCurPointer = 0;
	m_nCurSpecialPointer = 0;
}

void RMPointer::Close(void) {
	int i;

	for (i = 0; i < 1; i++)
		if (m_pointer[i] != NULL)
			delete m_pointer[i], m_pointer[i] = NULL;
}


int RMPointer::Priority() {
	// Priorita' minima: davanti a tutto
	return 200;
}

void RMPointer::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int n;

	// Controlla il pointer
	n = m_nCurPointer;
	if (n == TA_COMBINE) n = TA_USE;

	// Copia le coordinate di destinazione nella primitiva
	prim->SetDst(m_pos);

	if (m_pos.x >= 0 && m_pos.y >= 0 && m_pos.x < RM_SX && m_pos.y < RM_SY) {
		// Richiama il draw del puntatore
		prim->Dst()-=m_hotspot[n];

		if (m_nCurSpecialPointer == 0) {
			m_pointer[n]->Draw(bigBuf,prim);
		} else {
			if (m_nCurSpecialPointer == PTR_CUSTOM)
				m_nCurCustomPointer->Draw(bigBuf, prim);
			else
				// Richiama il draw sul puntatore speciale
				m_specialPointer[m_nCurSpecialPointer-1]->Draw(bigBuf, prim);
		}
	}
}

void RMPointer::DoFrame(RMGfxTargetBuffer *bigBuf) {
	// Si aggiunge alla lista delle primitive
	bigBuf->AddPrim(new RMGfxPrimitive(this));

	// Se c'e' un puntatore speciale, fa la DoFrame
	if (m_nCurSpecialPointer!=0 && m_nCurSpecialPointer!=PTR_CUSTOM)
		m_specialPointer[m_nCurSpecialPointer-1]->DoFrame(bigBuf,false);
}

bool RMPointer::RemoveThis() {
	// Si leva sempre dalla lista di OT, per supportare la DisableInput
	return true;
}

int RMPointer::CurAction(void) {
	if (m_nCurSpecialPointer != 0)
		return 0;

	return m_nCurPointer;
}

} // End of namespace Tony
