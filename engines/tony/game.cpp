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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/file.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpalutils.h"
#include "tony/custom.h"
#include "tony/game.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

using namespace MPAL;

/****************************************/
/*  Global functions					*/
/****************************************/

uint32 MainLoadLocation(int nLoc, RMPoint pt, RMPoint start) {
	return _vm->GetEngine()->LoadLocation(nLoc, pt, start);
}

void MainUnloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result) {
	_vm->GetEngine()->UnloadLocation(coroParam, bDoOnExit, result);
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

void MainWaitFrame(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, _vm->m_hEndOfFrame, CORO_INFINITE);
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

void MainWaitWipeEnd(CORO_PARAM) {
	_vm->GetEngine()->WaitWipeEnd(coroParam);
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
*       RMOptionButton Methods
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

RMOptionButton::RMOptionButton(const RMRect &pt) {
	m_rect = pt;
	m_bActive = false;
	m_bHasGfx = false;
	m_bDoubleState = false;
}

RMOptionButton::~RMOptionButton() {
	if (m_bHasGfx)
		delete m_buf;
}

bool RMOptionButton::DoFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	if (!m_bDoubleState) {
		if (m_rect.PtInRect(mousePos)) {
			if (!m_bActive) {
				m_bActive = true;
				return true;
			}
		} else {
			if (m_bActive) {
				m_bActive = false;
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



void RMOptionButton::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!m_bActive)
		return;

	if (m_bHasGfx)
		CORO_INVOKE_2(m_buf->Draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionButton::AddToList(RMGfxTargetBuffer &bigBuf) {
	if (m_bHasGfx)
		bigBuf.AddPrim(new RMGfxPrimitive(this, m_rect));
}

/****************************************************************************\
*       RMOptionSlide Methods
\****************************************************************************/

RMOptionSlide::RMOptionSlide(const RMPoint &pt, int nRange, int nStartValue, int slideSize) {
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

bool RMOptionSlide::DoFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	bool bRefresh = false;

	// Do the button DoFrame's
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

void RMOptionSlide::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int i;
	int val;
	RMPoint pos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pos = m_pos;
	_ctx->pos.x += 4;
	_ctx->pos.y += 4;

	_ctx->val = m_nValue * m_nStep;
	if (_ctx->val < 1) _ctx->val = 1;
	else if (_ctx->val > 100) _ctx->val = 100;

	if (_ctx->val == 1) {
		prim->SetDst(_ctx->pos);
		CORO_INVOKE_2(m_SliderSingle->Draw, bigBuf, prim);
	} else {
		prim->SetDst(_ctx->pos);
		CORO_INVOKE_2(m_SliderLeft->Draw, bigBuf, prim);
		_ctx->pos.x += 3;

		for (_ctx->i = 1; _ctx->i < _ctx->val - 1; _ctx->i++) {
			prim->SetDst(_ctx->pos);
			CORO_INVOKE_2(m_SliderCenter->Draw, bigBuf, prim);
			_ctx->pos.x += 3;
		}

		prim->SetDst(_ctx->pos);
		CORO_INVOKE_2(m_SliderRight->Draw, bigBuf, prim);
		_ctx->pos.x += 3;
	}

	CORO_END_CODE;
}

void RMOptionSlide::AddToList(RMGfxTargetBuffer &bigBuf) {
	bigBuf.AddPrim(new RMGfxPrimitive(this));
}



/****************************************************************************\
*       RMOptionScreen Methods
\****************************************************************************/

RMOptionScreen::RMOptionScreen(void) {
	m_nState = MENUGAME;
	m_menu = NULL;
	m_HideLoadSave = NULL;
	m_QuitConfirm = NULL;
	m_bQuitConfirm = false;

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
	m_ButtonQuitYes = NULL;
	m_ButtonQuitNo = NULL;
	m_ButtonQuit = NULL;
	m_SaveEasy = NULL;
	m_SaveHard = NULL;
	m_ButtonGfx_Tips = NULL;
	m_ButtonSound_DubbingOn = NULL;
	m_ButtonSound_MusicOn = NULL;
	m_ButtonSound_SFXOn = NULL;
	m_SlideTonySpeed = NULL;
	m_SlideTextSpeed = NULL;
	m_ButtonGame_Lock = NULL;
	m_ButtonGfx_Anni30 = NULL;
	m_SliderSound_Music = NULL;
	m_ButtonGame_TimerizedText = NULL;
	m_ButtonGfx_AntiAlias = NULL;
	m_SliderSound_SFX = NULL;
	m_ButtonGame_Scrolling = NULL;
	m_ButtonGfx_Sottotitoli = NULL;
	m_SliderSound_Dubbing = NULL;
	m_ButtonGame_InterUp = NULL;
	m_ButtonGfx_Trans = NULL;

	m_FadeStep = 0;
	m_FadeY = 0;
	m_FadeTime = 0;
	m_nEditPos = 0;
	m_nLastState = MENUGAME;
}


RMOptionScreen::~RMOptionScreen(void) {
}

void RMOptionScreen::RefreshAll(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMGfxSourceBuffer16 *thumb;
	RMText *title;
	RMText *num[6];
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	ClearOT();

	AddPrim(new RMGfxPrimitive(m_menu));

	if (m_bNoLoadSave)
		AddPrim(new RMGfxPrimitive(m_HideLoadSave, RMPoint(0, 401)));

	if (m_bQuitConfirm) {
		AddPrim(new RMGfxPrimitive(m_QuitConfirm, RMPoint(270, 200)));
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

	_ctx->thumb = NULL;
	_ctx->title = NULL;
	Common::fill(&_ctx->num[0], &_ctx->num[6], (RMText *)NULL);

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		_ctx->title = new RMText;
		if (m_nState == MENULOAD) {
			RMMessage msg(10);
			_ctx->title->WriteText(msg[0], 1);
		} else {
			RMMessage msg(11);
			_ctx->title->WriteText(msg[0], 1);
		}

		AddPrim(new RMGfxPrimitive(_ctx->title, RMPoint(320, 10)));

		if (m_curThumbDiff[0] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(48, 57)));
		else if (m_curThumbDiff[0] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(48, 57)));
		if (m_curThumbDiff[1] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(240, 57)));
		else if (m_curThumbDiff[1] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(240, 57)));
		if (m_curThumbDiff[2] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(432, 57)));
		else if (m_curThumbDiff[2] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(432, 57)));
		if (m_curThumbDiff[3] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(48, 239)));
		else if (m_curThumbDiff[3] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(48, 239)));
		if (m_curThumbDiff[4] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(240, 239)));
		else if (m_curThumbDiff[4] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(240, 239)));
		if (m_curThumbDiff[5] == 0)
			AddPrim(new RMGfxPrimitive(m_SaveHard, RMPoint(432, 239)));
		else if (m_curThumbDiff[5] == 1)
			AddPrim(new RMGfxPrimitive(m_SaveEasy, RMPoint(432, 239)));

		if (m_curThumb[0] && !(m_bEditSaveName && m_nEditPos == 0))
			AddPrim(new RMGfxPrimitive(m_curThumb[0], RMPoint(48, 57)));
		if (m_curThumb[1] && !(m_bEditSaveName && m_nEditPos == 1))
			AddPrim(new RMGfxPrimitive(m_curThumb[1], RMPoint(240, 57)));
		if (m_curThumb[2] && !(m_bEditSaveName && m_nEditPos == 2))
			AddPrim(new RMGfxPrimitive(m_curThumb[2], RMPoint(432, 57)));
		if (m_curThumb[3] && !(m_bEditSaveName && m_nEditPos == 3))
			AddPrim(new RMGfxPrimitive(m_curThumb[3], RMPoint(48, 239)));
		if (m_curThumb[4] && !(m_bEditSaveName && m_nEditPos == 4))
			AddPrim(new RMGfxPrimitive(m_curThumb[4], RMPoint(240, 239)));
		if (m_curThumb[5] && !(m_bEditSaveName && m_nEditPos == 5))
			AddPrim(new RMGfxPrimitive(m_curThumb[5], RMPoint(432, 239)));

		if (m_bEditSaveName) {
			_ctx->thumb = new RMGfxSourceBuffer16;
			_ctx->thumb->Init((byte *)_vm->GetThumbnail(), 640 / 4, 480 / 4);

			if (m_nEditPos == 0)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 57)));
			else if (m_nEditPos == 1)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 57)));
			else if (m_nEditPos == 2)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 57)));
			else if (m_nEditPos == 3)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 239)));
			else if (m_nEditPos == 4)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 239)));
			else if (m_nEditPos == 5)
				AddPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 239)));
		}

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++) {
			RMString s;

			if (m_bEditSaveName && m_nEditPos == _ctx->i)
				s.Format("%02d)%s*", m_statePos + _ctx->i, m_EditName);
			else {
				if (m_statePos == 0 && _ctx->i == 0)
					s.Format("Autosave");
				else
					s.Format("%02d)%s", m_statePos + _ctx->i, (const char *)m_curThumbName[_ctx->i]);
			}

			_ctx->num[_ctx->i] = new RMText;
			_ctx->num[_ctx->i]->SetAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->num[_ctx->i]->WriteText(s, 2);
		}

		AddPrim(new RMGfxPrimitive(_ctx->num[0], RMPoint(55 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(_ctx->num[1], RMPoint(247 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(_ctx->num[2], RMPoint(439 - 3, 180 + 14)));
		AddPrim(new RMGfxPrimitive(_ctx->num[3], RMPoint(55 - 3, 362 + 14)));
		AddPrim(new RMGfxPrimitive(_ctx->num[4], RMPoint(247 - 3, 362 + 14)));
		AddPrim(new RMGfxPrimitive(_ctx->num[5], RMPoint(439 - 3, 362 + 14)));

		m_ButtonSave_ArrowLeft->AddToList(*this);
		m_ButtonSave_ArrowRight->AddToList(*this);
	}

	CORO_INVOKE_0(DrawOT);

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (_ctx->thumb) delete _ctx->thumb;
		if (_ctx->title) delete _ctx->title;

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
			if (_ctx->num[_ctx->i]) delete _ctx->num[_ctx->i];
	}

	CORO_END_CODE;
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


void RMOptionScreen::InitState(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMResRaw *raw;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND)
		_ctx->raw = new RMResRaw(20000 + m_nState);
	else if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (m_bAlterGfx)
			_ctx->raw = new RMResRaw(20024);
		else
			_ctx->raw = new RMResRaw(20003);
	} else {
		error("Invalid state");
	}

	assert(_ctx->raw->IsValid());
	assert(m_menu == NULL);
	m_menu = new RMGfxSourceBuffer16(false);
	m_menu->Init(*_ctx->raw, _ctx->raw->Width(), _ctx->raw->Height());
	delete _ctx->raw;

	if (m_nState == MENULOAD || m_nState == MENUSAVE) {
		if (m_bAlterGfx) {
			assert(m_ButtonExit == NULL);
			m_ButtonExit = new RMOptionButton(20025, RMPoint(561, 406));
		} else {
			assert(m_ButtonExit == NULL);
			m_ButtonExit = new RMOptionButton(20012, RMPoint(560, 404));
		}

		INIT_GFX8_FROMRAW(_ctx->raw, 20036, m_SaveEasy);
		INIT_GFX8_FROMRAW(_ctx->raw, 20037, m_SaveHard);

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
			m_ButtonSave_ArrowLeft = new RMOptionButton(20026, RMPoint(3, 196));
			assert(m_ButtonSave_ArrowRight == NULL);
			m_ButtonSave_ArrowRight = new RMOptionButton(20027, RMPoint(601, 197));
		} else {
			assert(m_ButtonSave_ArrowLeft == NULL);
			m_ButtonSave_ArrowLeft = new RMOptionButton(20013, RMPoint(0, 197));
			assert(m_ButtonSave_ArrowRight == NULL);
			m_ButtonSave_ArrowRight = new RMOptionButton(20014, RMPoint(601, 197));
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

		_ctx->raw = new RMResRaw(20021);
		assert(_ctx->raw->IsValid());
		assert(m_QuitConfirm == NULL);
		m_QuitConfirm = new RMGfxSourceBuffer16(false);
		m_QuitConfirm->Init(*_ctx->raw, _ctx->raw->Width(), _ctx->raw->Height());
		delete _ctx->raw;

		assert(m_ButtonQuitYes == NULL);
		m_ButtonQuitYes = new RMOptionButton(20022, RMPoint(281, 265));
		m_ButtonQuitYes->SetPriority(30);
		assert(m_ButtonQuitNo == NULL);
		m_ButtonQuitNo = new RMOptionButton(20023, RMPoint(337, 264));
		m_ButtonQuitNo->SetPriority(30);

		if (m_bNoLoadSave) {
			_ctx->raw = new RMResRaw(20028);
			assert(_ctx->raw->IsValid());
			assert(m_HideLoadSave == NULL);
			m_HideLoadSave = new RMGfxSourceBuffer16(false);
			m_HideLoadSave->Init(*_ctx->raw, _ctx->raw->Width(), _ctx->raw->Height());
			delete _ctx->raw;
		}

		// Menu GAME
		if (m_nState == MENUGAME) {
			assert(m_ButtonGame_Lock == NULL);
			m_ButtonGame_Lock = new RMOptionButton(20008, RMPoint(176, 262), true);
			m_ButtonGame_Lock->SetActiveState(GLOBALS.bCfgInvLocked);
			assert(m_ButtonGame_TimerizedText == NULL);
			m_ButtonGame_TimerizedText = new RMOptionButton(20009, RMPoint(463, 273), true);
			m_ButtonGame_TimerizedText->SetActiveState(!GLOBALS.bCfgTimerizedText);
			assert(m_ButtonGame_Scrolling == NULL);
			m_ButtonGame_Scrolling = new RMOptionButton(20010, RMPoint(315, 263), true);
			m_ButtonGame_Scrolling->SetActiveState(GLOBALS.bCfgInvNoScroll);
			assert(m_ButtonGame_InterUp == NULL);
			m_ButtonGame_InterUp = new RMOptionButton(20011, RMPoint(36, 258), true);
			m_ButtonGame_InterUp->SetActiveState(GLOBALS.bCfgInvUp);

			assert(m_SlideTextSpeed == NULL);
			m_SlideTextSpeed = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS.nCfgTextSpeed);
			assert(m_SlideTonySpeed == NULL);
			m_SlideTonySpeed = new RMOptionSlide(RMPoint(165, 226), 5, GLOBALS.nCfgTonySpeed);
		}
		// Menu Graphics
		else if (m_nState == MENUGFX) {
			assert(m_ButtonGfx_Anni30 == NULL);
			m_ButtonGfx_Anni30 = new RMOptionButton(20015, RMPoint(247, 178), true);
			m_ButtonGfx_Anni30->SetActiveState(GLOBALS.bCfgAnni30);
			assert(m_ButtonGfx_AntiAlias == NULL);
			m_ButtonGfx_AntiAlias = new RMOptionButton(20016, RMPoint(430, 83), true);
			m_ButtonGfx_AntiAlias->SetActiveState(!GLOBALS.bCfgAntiAlias);
			assert(m_ButtonGfx_Sottotitoli == NULL);
			m_ButtonGfx_Sottotitoli = new RMOptionButton(20017, RMPoint(98, 82), true);
			m_ButtonGfx_Sottotitoli->SetActiveState(!GLOBALS.bCfgSottotitoli);
			assert(m_ButtonGfx_Tips == NULL);
			m_ButtonGfx_Tips = new RMOptionButton(20018, RMPoint(431, 246), true);
			m_ButtonGfx_Tips->SetActiveState(GLOBALS.bCfgInterTips);
			assert(m_ButtonGfx_Trans == NULL);
			m_ButtonGfx_Trans = new RMOptionButton(20019, RMPoint(126, 271), true);
			m_ButtonGfx_Trans->SetActiveState(!GLOBALS.bCfgTransparence);

		} else if (m_nState == MENUSOUND) {
			assert(m_SliderSound_Dubbing == NULL);
			m_SliderSound_Dubbing = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS.nCfgDubbingVolume);
			assert(m_SliderSound_Music == NULL);
			m_SliderSound_Music = new RMOptionSlide(RMPoint(165, 226), 10, GLOBALS.nCfgMusicVolume);
			assert(m_SliderSound_SFX == NULL);
			m_SliderSound_SFX = new RMOptionSlide(RMPoint(165, 330), 10, GLOBALS.nCfgSFXVolume);

			assert(m_ButtonSound_DubbingOn == NULL);
			m_ButtonSound_DubbingOn = new RMOptionButton(20033, RMPoint(339, 75), true);
			m_ButtonSound_DubbingOn->SetActiveState(GLOBALS.bCfgDubbing);
			assert(m_ButtonSound_MusicOn == NULL);
			m_ButtonSound_MusicOn = new RMOptionButton(20034, RMPoint(338, 179), true);
			m_ButtonSound_MusicOn->SetActiveState(GLOBALS.bCfgMusic);
			assert(m_ButtonSound_SFXOn == NULL);
			m_ButtonSound_SFXOn = new RMOptionButton(20035, RMPoint(338, 283), true);
			m_ButtonSound_SFXOn->SetActiveState(GLOBALS.bCfgSFX);
		}
	}

	CORO_INVOKE_0(RefreshAll);

	CORO_END_CODE;
}

void RMOptionScreen::CloseState(void) {
	delete m_menu;
	m_menu = NULL;

	delete m_ButtonExit;
	m_ButtonExit = NULL;

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
			GLOBALS.bCfgInvLocked = m_ButtonGame_Lock->IsActive();
			delete m_ButtonGame_Lock;
			m_ButtonGame_Lock = NULL;

			GLOBALS.bCfgTimerizedText = !m_ButtonGame_TimerizedText->IsActive();
			delete m_ButtonGame_TimerizedText;
			m_ButtonGame_TimerizedText = NULL;

			GLOBALS.bCfgInvNoScroll = m_ButtonGame_Scrolling->IsActive();
			delete m_ButtonGame_Scrolling;
			m_ButtonGame_Scrolling = NULL;

			GLOBALS.bCfgInvUp = m_ButtonGame_InterUp->IsActive();
			delete m_ButtonGame_InterUp;
			m_ButtonGame_InterUp = NULL;

			GLOBALS.nCfgTextSpeed = m_SlideTextSpeed->GetValue();
			delete m_SlideTextSpeed;
			m_SlideTextSpeed = NULL;

			GLOBALS.nCfgTonySpeed = m_SlideTonySpeed->GetValue();
			delete m_SlideTonySpeed;
			m_SlideTonySpeed = NULL;
		} else if (m_nState == MENUGFX) {
			GLOBALS.bCfgAnni30 = m_ButtonGfx_Anni30->IsActive();
			delete m_ButtonGfx_Anni30;
			m_ButtonGfx_Anni30 = NULL;

			GLOBALS.bCfgAntiAlias = !m_ButtonGfx_AntiAlias->IsActive();
			delete m_ButtonGfx_AntiAlias;
			m_ButtonGfx_AntiAlias = NULL;

			GLOBALS.bCfgSottotitoli = !m_ButtonGfx_Sottotitoli->IsActive();
			delete m_ButtonGfx_Sottotitoli;
			m_ButtonGfx_Sottotitoli = NULL;

			GLOBALS.bCfgInterTips = m_ButtonGfx_Tips->IsActive();
			delete m_ButtonGfx_Tips;
			m_ButtonGfx_Tips = NULL;

			GLOBALS.bCfgTransparence = !m_ButtonGfx_Trans->IsActive();
			delete m_ButtonGfx_Trans;
			m_ButtonGfx_Trans = NULL;
		} else if (m_nState == MENUSOUND) {
			GLOBALS.nCfgDubbingVolume = m_SliderSound_Dubbing->GetValue();
			delete m_SliderSound_Dubbing;
			m_SliderSound_Dubbing = NULL;

			GLOBALS.nCfgMusicVolume = m_SliderSound_Music->GetValue();
			delete m_SliderSound_Music;
			m_SliderSound_Music = NULL;

			GLOBALS.nCfgSFXVolume = m_SliderSound_SFX->GetValue();
			delete m_SliderSound_SFX;
			m_SliderSound_SFX = NULL;

			GLOBALS.bCfgDubbing = m_ButtonSound_DubbingOn->IsActive();
			delete m_ButtonSound_DubbingOn;
			m_ButtonSound_DubbingOn = NULL;

			GLOBALS.bCfgMusic = m_ButtonSound_MusicOn->IsActive();
			delete m_ButtonSound_MusicOn;
			m_ButtonSound_MusicOn = NULL;

			GLOBALS.bCfgSFX = m_ButtonSound_SFXOn->IsActive();
			delete m_ButtonSound_SFXOn;
			m_ButtonSound_SFXOn = NULL;
		}
	}
}

void RMOptionScreen::ReInit(RMGfxTargetBuffer &bigBuf) {
	bigBuf.AddPrim(new RMGfxPrimitive(this));
}

void RMOptionScreen::Init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_FadeStep != 0) {
		result = false;
		return;
	}

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

	CORO_INVOKE_0(InitState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::InitLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_FadeStep != 0) {
		result = false;
		return;
	}

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = true;
	m_bNoLoadSave = false;
	m_bAlterGfx = bAlternateGfx;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENULOAD;
	CORO_INVOKE_0(InitState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::InitSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_FadeStep != 0) {
		result = false;
		return;
	}

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = true;
	m_bNoLoadSave = false;
	m_bAlterGfx = bAlternateGfx;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENUSAVE;
	CORO_INVOKE_0(InitState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::InitNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_FadeStep != 0) {
		result = false;
		return;
	}

	m_FadeStep = 1;
	m_FadeY = -20;
	m_FadeTime = -1;
	m_bExit = false;
	m_bLoadMenuOnly = false;
	m_bNoLoadSave = true;

	bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_nState = MENUGAME;
	CORO_INVOKE_0(InitState);

	result = true;

	CORO_END_CODE;
}

bool RMOptionScreen::Close(void) {
	if (m_FadeStep != 6)
		return false;

	// Start fade out
	m_FadeStep++;
	m_FadeTime = _vm->GetTime();
	return true;
}

bool RMOptionScreen::IsClosing(void) {
	return m_bExit;
}

int RMOptionScreen::Priority() {
	// Just below the mouse
	return 190;
}

void RMOptionScreen::ChangeState(CORO_PARAM, STATE newState) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CloseState();
	m_nLastState = m_nState;
	m_nState = newState;
	CORO_INVOKE_0(InitState);

	CORO_END_CODE;
}

void RMOptionScreen::DoFrame(CORO_PARAM, RMInput *input) {
	CORO_BEGIN_CONTEXT;
	bool bLeftClick, bRightClick;
	RMPoint mousePos;
	bool bRefresh;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);


	// If it is fully open, do nothing
	if (m_FadeStep != 6)
		return;

	// Reads input
	_ctx->mousePos = input->MousePos();
	_ctx->bLeftClick = input->MouseLeftClicked();
	_ctx->bRightClick = input->MouseRightClicked();

	_ctx->bRefresh = false;

	if (m_bQuitConfirm) {
		_ctx->bRefresh |= m_ButtonQuitYes->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
		_ctx->bRefresh |= m_ButtonQuitNo->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
	} else {
		_ctx->bRefresh |= m_ButtonExit->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		// Check if you have clicked on the output
		if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
			// Buttons without graphics...
			m_ButtonGameMenu->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			m_ButtonGfxMenu->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			m_ButtonSoundMenu->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			// Buttons with graphics
			if (!m_bNoLoadSave) {
				if (!_vm->getIsDemo()) {
					_ctx->bRefresh |= m_ButtonLoad->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
					_ctx->bRefresh |= m_ButtonSave->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
				}

				_ctx->bRefresh |= m_ButtonQuit->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			}
		}

		if (m_nState == MENUGAME) {
			_ctx->bRefresh |= m_ButtonGame_Lock->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGame_TimerizedText->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGame_Scrolling->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGame_InterUp->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_SlideTextSpeed->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_SlideTonySpeed->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (m_nState == MENUGFX) {
			_ctx->bRefresh |= m_ButtonGfx_Anni30->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGfx_AntiAlias->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGfx_Sottotitoli->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGfx_Tips->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonGfx_Trans->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (m_nState == MENUSOUND) {
			_ctx->bRefresh |= m_SliderSound_Dubbing->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_SliderSound_Music->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_SliderSound_SFX->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonSound_DubbingOn->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonSound_MusicOn->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= m_ButtonSound_SFXOn->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (m_nState == MENULOAD || m_nState == MENUSAVE) {
			for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
				m_ButtonSave_States[_ctx->i]->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			if (m_statePos > 0)
				_ctx->bRefresh |= m_ButtonSave_ArrowLeft->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			if (m_statePos < 90)
				_ctx->bRefresh |= m_ButtonSave_ArrowRight->DoFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		}
	}

#define KEYPRESS(c)     (_vm->GetEngine()->GetInput().GetAsyncKeyState(c))
#define PROCESS_CHAR(cod,c)  if (KEYPRESS(cod)) { \
		m_EditName[strlen(m_EditName) +1 ] = '\0'; m_EditName[strlen(m_EditName)] = c; _ctx->bRefresh = true; }

	/**************** State Buttons **************/
	if (m_bEditSaveName) {
		if (KEYPRESS(Common::KEYCODE_BACKSPACE)) {
			if (m_EditName[0] != '\0') {
				m_EditName[strlen(m_EditName) - 1] = '\0';
				_ctx->bRefresh = true;
			}
		}

		for (_ctx->i = 0; _ctx->i < 26 && strlen(m_EditName) < 12; _ctx->i++)
			if (KEYPRESS(Common::KEYCODE_LSHIFT) ||
			        KEYPRESS(Common::KEYCODE_RSHIFT)) {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'A');
			} else {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'a');
			}

		for (_ctx->i = 0; _ctx->i < 10 && strlen(m_EditName) < 12; _ctx->i++)
			PROCESS_CHAR((Common::KeyCode)((int)'0' + _ctx->i), _ctx->i + '0');

		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_SPACE, ' ');

		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP0, '0');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP1, '1');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP2, '2');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP3, '3');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP4, '4');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP5, '5');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP6, '6');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP7, '7');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP8, '8');
		if (strlen(m_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP9, '9');

		// Cancel
		if (KEYPRESS(Common::KEYCODE_ESCAPE)) {
			m_bEditSaveName = false;
			_ctx->bRefresh = true;
		}

		// OK
		if (KEYPRESS(Common::KEYCODE_RETURN)) {
			m_bEditSaveName = false;
			_vm->SaveState(m_statePos + m_nEditPos, m_EditName);
			Close();
		}

	} else if (_ctx->bLeftClick) {
		if (m_nState == MENULOAD || m_nState == MENUSAVE) {
			if (m_ButtonExit->IsActive()) {
				if (m_bLoadMenuOnly) {
					// If only the loading menu, close
					Close();
				} else {
					CORO_INVOKE_1(ChangeState, m_nLastState);
					_ctx->bRefresh = true;
				}
			} else if (m_ButtonSave_ArrowLeft->IsActive()) {
				if (m_statePos > 0) {
					m_statePos -= 6;
					if (m_statePos < 0) m_statePos = 0;
					m_ButtonSave_ArrowLeft->SetActiveState(false);
					_ctx->bRefresh = true;
					RefreshThumbnails();
				}
			} else if (m_ButtonSave_ArrowRight->IsActive()) {
				if (m_statePos < 90) {
					m_statePos += 6;
					if (m_statePos > 90) m_statePos = 90;
					m_ButtonSave_ArrowRight->SetActiveState(false);
					_ctx->bRefresh = true;
					RefreshThumbnails();
				}
			} else {
				for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
					if (m_ButtonSave_States[_ctx->i]->IsActive()) {
						// There by saving or loading!!!
						if (m_nState == MENULOAD && m_curThumb[_ctx->i] != NULL) {
							// Caricamento
							CORO_INVOKE_1(_vm->LoadState, m_statePos + _ctx->i);
							Close();
						} else if (m_nState == MENUSAVE && (m_statePos != 0 || _ctx->i != 0)) {
							// Turn on edit mode
							m_bEditSaveName = true;
							m_nEditPos = _ctx->i;
							strcpy(m_EditName, m_curThumbName[_ctx->i]);
							_ctx->bRefresh = true;
						}

						break;
					}
			}
		}

		if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND) {
			if (m_bQuitConfirm) {
				if (m_ButtonQuitNo->IsActive()) {
					m_bQuitConfirm = false;
					_ctx->bRefresh = true;
				} else if (m_ButtonQuitYes->IsActive()) {
					m_bQuitConfirm = false;
					_ctx->bRefresh = true;

					_vm->Quit();
				}
			} else {
				if (m_ButtonQuit->IsActive()) {
					m_bQuitConfirm = true;
					m_ButtonQuitNo->SetActiveState(false);
					m_ButtonQuitYes->SetActiveState(false);
					_ctx->bRefresh = true;
				} else if (m_ButtonExit->IsActive())
					Close();
				else if (m_ButtonLoad->IsActive()) {
					CORO_INVOKE_1(ChangeState, MENULOAD);
					_ctx->bRefresh = true;
				} else if (m_ButtonSave->IsActive()) {
					CORO_INVOKE_1(ChangeState, MENUSAVE);
					_ctx->bRefresh = true;
				} else if (m_ButtonGameMenu->IsActive() && m_nState != MENUGAME) {
					CORO_INVOKE_1(ChangeState, MENUGAME);
					_ctx->bRefresh = true;
				} else if (m_ButtonGfxMenu->IsActive() && m_nState != MENUGFX) {
					CORO_INVOKE_1(ChangeState, MENUGFX);
					_ctx->bRefresh = true;
				} else if (m_ButtonSoundMenu->IsActive() && m_nState != MENUSOUND) {
					CORO_INVOKE_1(ChangeState, MENUSOUND);
					_ctx->bRefresh = true;
				}

				if (m_nState == MENUGFX) {
					// These options take effect immediately
					if (m_ButtonGfx_Anni30->IsActive())
						GLOBALS.bCfgAnni30 = true;
					else
						GLOBALS.bCfgAnni30 = false;

					if (m_ButtonGfx_AntiAlias->IsActive())
						GLOBALS.bCfgAntiAlias = false;
					else
						GLOBALS.bCfgAntiAlias = true;

					if (m_ButtonGfx_Trans->IsActive())
						GLOBALS.bCfgTransparence = false;
					else
						GLOBALS.bCfgTransparence = true;
				}
			}
		}
	}

	if (m_nState == MENUGAME || m_nState == MENUGFX || m_nState == MENUSOUND)
		if (!m_bQuitConfirm && KEYPRESS(Common::KEYCODE_ESCAPE))
			Close();

	if (_ctx->bRefresh)
		CORO_INVOKE_0(RefreshAll);

	CORO_END_CODE;
}


void RMOptionScreen::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int curTime;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = _vm->GetTime();

#define FADE_SPEED 20
#define SYNC    (_ctx->curTime-m_FadeTime) / 25

	if (m_bExit)
		return;

	if (m_FadeStep == 1) {
		// Downhill fast
		if (m_FadeTime == -1)
			m_FadeY += FADE_SPEED;
		else
			m_FadeY += FADE_SPEED * SYNC;
		if (m_FadeY > 480) {
			m_FadeY = 480;
			m_FadeStep++;
		}

		// Set the part to draw the scrolling
		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 2) {
		// Bounce 1
		m_FadeY -= FADE_SPEED / 2 * SYNC;
		if (m_FadeY < 400) {
			m_FadeY = 400;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 3) {
		m_FadeY -= FADE_SPEED / 4 * SYNC;
		if (m_FadeY < 380) {
			m_FadeY = 380;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 4) {
		// Bounce 1 - 2
		m_FadeY += FADE_SPEED / 3 * SYNC;
		if (m_FadeY > 420) {
			m_FadeY = 420;
			m_FadeStep++;
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 5) {
		m_FadeY += FADE_SPEED / 2 * SYNC;
		if (m_FadeY > 480) {
			m_FadeY = 480;
			m_FadeStep++;
			_vm->HideLocation();
		}

		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 6) {
		// Menu ON

	} else if (m_FadeStep == 7) {
		// Menu OFF
		_vm->ShowLocation();
		m_FadeStep++;

	} else if (m_FadeStep == 8) {
		m_FadeY -= FADE_SPEED * SYNC;
		if (m_FadeY < 0) {
			m_FadeY = 0;
			m_FadeStep++;
		}
		prim->SetSrc(RMRect(0, 480 - m_FadeY, 640, 480));

	} else if (m_FadeStep == 9) {
		// Hello hello!
		m_bExit = true;
		m_FadeStep = 0;

		// Free memory
		CloseState();
		return;

	} else {
		m_FadeStep = 0;
	}

	m_FadeTime = _ctx->curTime;

	CORO_INVOKE_2(RMGfxWoodyBuffer::Draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionScreen::RemoveThis(CORO_PARAM, bool &result) {
	if (m_bExit)
		result = true;
	else
		result = false;
}


bool RMOptionScreen::LoadThumbnailFromSaveState(int nState, byte *lpDestBuf, RMString &name, byte &diff) {
	Common::String buf;
	char namebuf[256];
	int i;
	Common::InSaveFile *f;
	char id[4];

	// Cleans the destination
	Common::fill(lpDestBuf, lpDestBuf + 160 * 120 * 2, 0);
	name = "No name";
	diff = 10;

	// Get the savegame filename for the given slot
	buf = _vm->GetSaveStateFileName(nState);

	// Try and open the savegame
	f = g_system->getSavefileManager()->openForLoading(buf);
	if (f == NULL)
		return false;

	// Check to see if the file has a valid header
	f->read(id, 4);
	if (id[0] != 'R' || id[1] != 'M' || id[2] != 'S') {
		delete f;
		return false;
	}

	if (id[3] < 0x3) {
		// Very old version that doesn't have screenshots
		delete f;
		return true;
	}

	// Load the screenshot
	if ((id[3] >= 0x5) && (id[3] < 0x8)) {
		// Read it as an LZO compressed data block
		byte *cmpbuf;
		uint32 cmpsize, size;

		cmpbuf = new byte[160 * 120 * 4];

		// Se la versione >= 5, è compresso!
		cmpsize = f->readUint32LE();
		f->read(cmpbuf, cmpsize);

		lzo1x_decompress(cmpbuf, cmpsize, lpDestBuf, &size);

		delete[] cmpbuf;
	} else {
		// Read in the screenshot as an uncompressed data block
		if (id[3] >= 8)
			// Recent versions use hardcoded 160x120 uncomrpessed data, so size can be skipped
			f->skip(4);

		f->read(lpDestBuf, 160 * 120 * 2);
	}

	if (id[3] >= 0x5) {
		// Read in the difficulty level
		diff = f->readByte();
	}

	if (id[3] < 0x4) {
		// Savegame version doesn't have a stored name
		delete f;
		return true;
	}

	i = f->readByte();
	f->read(namebuf, i);
	namebuf[i] = '\0';
	name = namebuf;

	delete f;
	return true;
}


/****************************************************************************\
*       RMPointer Methods
\****************************************************************************/

RMPointer::RMPointer() {
	Common::fill(m_pointer, m_pointer + 16, (RMGfxSourceBuffer8 *)NULL);
	Common::fill(m_specialPointer, m_specialPointer + 16, (RMItem *)NULL);
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

	for (i = 0; i < 5; i++) {
		if (m_pointer[i] != NULL) {
			delete m_pointer[i];
			m_pointer[i] = NULL;
		}

		if (m_specialPointer[i] != NULL) {
			delete m_specialPointer[i];
			m_specialPointer[i] = NULL;
		}
	}
}


int RMPointer::Priority() {
	// Pointer has minimum priority so it will be drawn last
	return 200;
}

void RMPointer::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int n;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Check the pointer
	_ctx->n = m_nCurPointer;
	if (_ctx->n == TA_COMBINE) _ctx->n = TA_USE;

	// Copy the destination coordinates in the primitive
	prim->SetDst(m_pos);

	if (m_pos.x >= 0 && m_pos.y >= 0 && m_pos.x < RM_SX && m_pos.y < RM_SY) {
		// Call the Draw method of the poitner
		prim->Dst() -= m_hotspot[_ctx->n];

		if (m_nCurSpecialPointer == 0) {
			CORO_INVOKE_2(m_pointer[_ctx->n]->Draw, bigBuf, prim);
		} else {
			if (m_nCurSpecialPointer == PTR_CUSTOM)
				CORO_INVOKE_2(m_nCurCustomPointer->Draw, bigBuf, prim);
			else
				// Call the draw on the special pointer
				CORO_INVOKE_2(m_specialPointer[m_nCurSpecialPointer - 1]->Draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMPointer::DoFrame(RMGfxTargetBuffer *bigBuf) {
	// Add it to the list of primitives
	bigBuf->AddPrim(new RMGfxPrimitive(this));

	// If there is a special pointer, does DoFrame
	if (m_nCurSpecialPointer != 0 && m_nCurSpecialPointer != PTR_CUSTOM)
		m_specialPointer[m_nCurSpecialPointer - 1]->DoFrame(bigBuf, false);
}

void RMPointer::RemoveThis(CORO_PARAM, bool &result) {
	// Always remove from the OT list, to support disabling the pointer
	result = true;
}

int RMPointer::CurAction(void) {
	if (m_nCurSpecialPointer != 0)
		return 0;

	return m_nCurPointer;
}

} // End of namespace Tony
