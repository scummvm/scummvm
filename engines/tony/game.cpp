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
/*  Global functions                    */
/****************************************/

uint32 mainLoadLocation(int nLoc, RMPoint pt, RMPoint start) {
	return _vm->getEngine()->loadLocation(nLoc, pt, start);
}

void mainUnloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result) {
	_vm->getEngine()->unloadLocation(coroParam, bDoOnExit, result);
}

void mainLinkGraphicTask(RMGfxTask *task) {
	_vm->getEngine()->linkGraphicTask(task);
}

void mainFreeze(void) {
	_vm->getEngine()->freeze();
}

void mainUnfreeze(void) {
	_vm->getEngine()->unfreeze();
}

void mainWaitFrame(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, _vm->_hEndOfFrame, CORO_INFINITE);
}

void mainShowMouse(void) {
	_vm->getEngine()->enableMouse();
}

void mainHideMouse(void) {
	_vm->getEngine()->disableMouse();
}

void mainPlayMusic(int nChannel, const char *filename, int nFX, bool bLoop, int nSync) {
	_vm->playMusic(nChannel, filename, nFX, bLoop, nSync);
}

void mainDisableInput(void) {
	_vm->getEngine()->disableInput();
}

void mainEnableInput(void) {
	_vm->getEngine()->enableInput();
}

void mainInitWipe(int type) {
	_vm->getEngine()->initWipe(type);
}

void mainCloseWipe(void) {
	_vm->getEngine()->closeWipe();
}

void mainWaitWipeEnd(CORO_PARAM) {
	_vm->getEngine()->waitWipeEnd(coroParam);
}

void mainEnableGUI(void) {
	_vm->getEngine()->_bGUIInterface = true;
	_vm->getEngine()->_bGUIInventory = true;
	_vm->getEngine()->_bGUIOption = true;
}

void mainDisableGUI(void) {
	_vm->getEngine()->_bGUIInterface = false;
	_vm->getEngine()->_bGUIInventory = false;
	_vm->getEngine()->_bGUIOption = false;
}

void mainSetPalesati(bool bPalesati) {
	_vm->getEngine()->setPalesati(bPalesati);
}

/****************************************************************************\
*       RMOptionButton Methods
\****************************************************************************/

RMOptionButton::RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState) {
	RMResRaw raw(dwRes);
	assert(raw.isValid());
	_buf = new RMGfxSourceBuffer16(false);
	_buf->init(raw, raw.width(), raw.height());

	_rect.setRect(pt.x, pt.y, pt.x + raw.width() - 1, pt.y + raw.height() - 1);
	_bActive = false;
	_bHasGfx = true;
	_bDoubleState = bDoubleState;
}

RMOptionButton::RMOptionButton(const RMRect &pt) {
	_rect = pt;
	_bActive = false;
	_bHasGfx = false;
	_bDoubleState = false;
	_buf = NULL;
}

RMOptionButton::~RMOptionButton() {
	if (_bHasGfx)
		delete _buf;
}

bool RMOptionButton::doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	if (!_bDoubleState) {
		if (_rect.ptInRect(mousePos)) {
			if (!_bActive) {
				_bActive = true;
				return true;
			}
		} else {
			if (_bActive) {
				_bActive = false;
				return true;
			}
		}
	} else {
		if (bLeftClick && _rect.ptInRect(mousePos)) {
			_bActive = !_bActive;
			return true;
		}
	}

	return false;
}

void RMOptionButton::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!_bActive)
		return;

	if (_bHasGfx)
		CORO_INVOKE_2(_buf->draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionButton::addToList(RMGfxTargetBuffer &bigBuf) {
	if (_bHasGfx)
		bigBuf.addPrim(new RMGfxPrimitive(this, _rect));
}

/****************************************************************************\
*       RMOptionSlide Methods
\****************************************************************************/

RMOptionSlide::RMOptionSlide(const RMPoint &pt, int nRange, int nStartValue, int slideSize) {
	RMResRaw *raw;

	_pos = pt;
	_nSlideSize = slideSize;
	_nMax = nRange;
	_nStep = 100 / _nMax;
	_nValue = nStartValue;

	_SliderCenter = NULL;
	_SliderLeft = NULL;
	_SliderRight = NULL;
	_SliderSingle = NULL;

	// Sliders
	INIT_GFX16_FROMRAW(20029, _SliderCenter);
	INIT_GFX16_FROMRAW(20030, _SliderLeft);
	INIT_GFX16_FROMRAW(20031, _SliderRight);
	INIT_GFX16_FROMRAW(20032, _SliderSingle);

	// Buttons
	_PushLeft = new RMOptionButton(RMRect(pt.x - 23, pt.y, pt.x - 23 + 22, pt.y + 26));
	_PushRight = new RMOptionButton(RMRect(pt.x + _nSlideSize, pt.y, pt.x + _nSlideSize + 5 + 22, pt.y + 26));
}


RMOptionSlide::~RMOptionSlide() {
	delete _SliderCenter;
	_SliderCenter = NULL;
	delete _SliderLeft;
	_SliderLeft = NULL;
	delete _SliderRight;
	_SliderRight = NULL;
	delete _SliderSingle;
	_SliderSingle = NULL;

	delete _PushLeft;
	_PushLeft = NULL;
	delete _PushRight;
	_PushRight = NULL;
}

bool RMOptionSlide::doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	bool bRefresh = false;

	// Do the button DoFrame's
	_PushLeft->doFrame(mousePos, bLeftClick, bRightClick);
	_PushRight->doFrame(mousePos, bLeftClick, bRightClick);

	if (_PushLeft->isActive()) {
		if (bLeftClick) {
			bRefresh = true;
			_nValue--;
		} else if (bRightClick) {
			bRefresh = true;
			_nValue -= 3;
		}
		if (_nValue < 1)
			_nValue = 1;
	} else if (_PushRight->isActive()) {
		bRefresh = true;

		if (bLeftClick) {
			bRefresh = true;
			_nValue++;
		} else if (bRightClick) {
			bRefresh = true;
			_nValue += 3;
		}
		if (_nValue > _nMax)
			_nValue = _nMax;
	}

	return bRefresh;
}

void RMOptionSlide::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int i;
	int val;
	RMPoint pos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pos = _pos;
	_ctx->pos.x += 4;
	_ctx->pos.y += 4;

	_ctx->val = _nValue * _nStep;
	if (_ctx->val < 1)
		_ctx->val = 1;
	else if (_ctx->val > 100)
		_ctx->val = 100;

	if (_ctx->val == 1) {
		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_SliderSingle->draw, bigBuf, prim);
	} else {
		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_SliderLeft->draw, bigBuf, prim);
		_ctx->pos.x += 3;

		for (_ctx->i = 1; _ctx->i < _ctx->val - 1; _ctx->i++) {
			prim->setDst(_ctx->pos);
			CORO_INVOKE_2(_SliderCenter->draw, bigBuf, prim);
			_ctx->pos.x += 3;
		}

		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_SliderRight->draw, bigBuf, prim);
		_ctx->pos.x += 3;
	}

	CORO_END_CODE;
}

void RMOptionSlide::addToList(RMGfxTargetBuffer &bigBuf) {
	bigBuf.addPrim(new RMGfxPrimitive(this));
}



/****************************************************************************\
*       RMOptionScreen Methods
\****************************************************************************/

RMOptionScreen::RMOptionScreen(void) {
	_nState = MENUNONE;
	_menu = NULL;
	_HideLoadSave = NULL;
	_QuitConfirm = NULL;
	_bQuitConfirm = false;

	create(RM_SX, RM_SY);

	_ButtonExit = NULL;
	_ButtonLoad = NULL;
	_ButtonSave = NULL;
	_ButtonGameMenu = NULL;
	_ButtonGfxMenu = NULL;
	_ButtonSoundMenu = NULL;
	_ButtonSave_ArrowLeft = NULL;
	_ButtonSave_ArrowRight = NULL;
	_bEditSaveName = false;

	int i;

	for (i = 0; i < 6; i++) {
		_curThumb[i] = NULL;
		_ButtonSave_States[i] = NULL;
	}

	_statePos = 0;
	_ButtonQuitYes = NULL;
	_ButtonQuitNo = NULL;
	_ButtonQuit = NULL;
	_SaveEasy = NULL;
	_SaveHard = NULL;
	_ButtonGfx_Tips = NULL;
	_ButtonSound_DubbingOn = NULL;
	_ButtonSound_MusicOn = NULL;
	_ButtonSound_SFXOn = NULL;
	_SlideTonySpeed = NULL;
	_SlideTextSpeed = NULL;
	_ButtonGame_Lock = NULL;
	_ButtonGfx_Anni30 = NULL;
	_SliderSound_Music = NULL;
	_ButtonGame_TimerizedText = NULL;
	_ButtonGfx_AntiAlias = NULL;
	_SliderSound_SFX = NULL;
	_ButtonGame_Scrolling = NULL;
	_ButtonGfx_Sottotitoli = NULL;
	_SliderSound_Dubbing = NULL;
	_ButtonGame_InterUp = NULL;
	_ButtonGfx_Trans = NULL;

	_FadeStep = 0;
	_FadeY = 0;
	_FadeTime = 0;
	_nEditPos = 0;
	_nLastState = MENUGAME;
}


RMOptionScreen::~RMOptionScreen(void) {
	closeState();
}

void RMOptionScreen::refreshAll(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMGfxSourceBuffer16 *thumb;
	RMText *title;
	RMText *num[6];
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	clearOT();

	addPrim(new RMGfxPrimitive(_menu));

	if (_bNoLoadSave)
		addPrim(new RMGfxPrimitive(_HideLoadSave, RMPoint(0, 401)));

	if (_bQuitConfirm) {
		addPrim(new RMGfxPrimitive(_QuitConfirm, RMPoint(270, 200)));
		_ButtonQuitYes->addToList(*this);
		_ButtonQuitNo->addToList(*this);
	}

	_ButtonExit->addToList(*this);

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		_ButtonQuit->addToList(*this);
		_ButtonLoad->addToList(*this);
		_ButtonSave->addToList(*this);
	}

	if (_nState == MENUGAME) {
		_ButtonGame_Lock->addToList(*this);
		_ButtonGame_TimerizedText->addToList(*this);
		_ButtonGame_Scrolling->addToList(*this);
		_ButtonGame_InterUp->addToList(*this);
		_SlideTextSpeed->addToList(*this);
		_SlideTonySpeed->addToList(*this);
	} else if (_nState == MENUGFX) {
		_ButtonGfx_Anni30->addToList(*this);
		_ButtonGfx_AntiAlias->addToList(*this);
		_ButtonGfx_Sottotitoli->addToList(*this);
		_ButtonGfx_Trans->addToList(*this);
		_ButtonGfx_Tips->addToList(*this);
	} else if (_nState == MENUSOUND) {
		_SliderSound_Dubbing->addToList(*this);
		_SliderSound_Music->addToList(*this);
		_SliderSound_SFX->addToList(*this);
		_ButtonSound_DubbingOn->addToList(*this);
		_ButtonSound_MusicOn->addToList(*this);
		_ButtonSound_SFXOn->addToList(*this);
	}

	_ctx->thumb = NULL;
	_ctx->title = NULL;
	Common::fill(&_ctx->num[0], &_ctx->num[6], (RMText *)NULL);

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		_ctx->title = new RMText;
		if (_nState == MENULOAD) {
			RMMessage msg(10);
			_ctx->title->writeText(msg[0], 1);
		} else {
			RMMessage msg(11);
			_ctx->title->writeText(msg[0], 1);
		}

		addPrim(new RMGfxPrimitive(_ctx->title, RMPoint(320, 10)));

		if (_curThumbDiff[0] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(48, 57)));
		else if (_curThumbDiff[0] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(48, 57)));
		if (_curThumbDiff[1] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(240, 57)));
		else if (_curThumbDiff[1] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(240, 57)));
		if (_curThumbDiff[2] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(432, 57)));
		else if (_curThumbDiff[2] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(432, 57)));
		if (_curThumbDiff[3] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(48, 239)));
		else if (_curThumbDiff[3] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(48, 239)));
		if (_curThumbDiff[4] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(240, 239)));
		else if (_curThumbDiff[4] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(240, 239)));
		if (_curThumbDiff[5] == 0)
			addPrim(new RMGfxPrimitive(_SaveHard, RMPoint(432, 239)));
		else if (_curThumbDiff[5] == 1)
			addPrim(new RMGfxPrimitive(_SaveEasy, RMPoint(432, 239)));

		if (_curThumb[0] && !(_bEditSaveName && _nEditPos == 0))
			addPrim(new RMGfxPrimitive(_curThumb[0], RMPoint(48, 57)));
		if (_curThumb[1] && !(_bEditSaveName && _nEditPos == 1))
			addPrim(new RMGfxPrimitive(_curThumb[1], RMPoint(240, 57)));
		if (_curThumb[2] && !(_bEditSaveName && _nEditPos == 2))
			addPrim(new RMGfxPrimitive(_curThumb[2], RMPoint(432, 57)));
		if (_curThumb[3] && !(_bEditSaveName && _nEditPos == 3))
			addPrim(new RMGfxPrimitive(_curThumb[3], RMPoint(48, 239)));
		if (_curThumb[4] && !(_bEditSaveName && _nEditPos == 4))
			addPrim(new RMGfxPrimitive(_curThumb[4], RMPoint(240, 239)));
		if (_curThumb[5] && !(_bEditSaveName && _nEditPos == 5))
			addPrim(new RMGfxPrimitive(_curThumb[5], RMPoint(432, 239)));

		if (_bEditSaveName) {
			_ctx->thumb = new RMGfxSourceBuffer16;
			_ctx->thumb->init((byte *)_vm->getThumbnail(), 640 / 4, 480 / 4);

			if (_nEditPos == 0)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 57)));
			else if (_nEditPos == 1)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 57)));
			else if (_nEditPos == 2)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 57)));
			else if (_nEditPos == 3)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 239)));
			else if (_nEditPos == 4)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 239)));
			else if (_nEditPos == 5)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 239)));
		}

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++) {
			RMString s;

			if (_bEditSaveName && _nEditPos == _ctx->i)
				s.format("%02d)%s*", _statePos + _ctx->i, _EditName);
			else {
				if (_statePos == 0 && _ctx->i == 0)
					s.format("Autosave");
				else
					s.format("%02d)%s", _statePos + _ctx->i, (const char *)_curThumbName[_ctx->i]);
			}

			_ctx->num[_ctx->i] = new RMText;
			_ctx->num[_ctx->i]->setAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->num[_ctx->i]->writeText(s, 2);
		}

		addPrim(new RMGfxPrimitive(_ctx->num[0], RMPoint(55 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[1], RMPoint(247 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[2], RMPoint(439 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[3], RMPoint(55 - 3, 362 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[4], RMPoint(247 - 3, 362 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[5], RMPoint(439 - 3, 362 + 14)));

		_ButtonSave_ArrowLeft->addToList(*this);
		_ButtonSave_ArrowRight->addToList(*this);
	}

	CORO_INVOKE_0(drawOT);

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_ctx->thumb) delete _ctx->thumb;
		if (_ctx->title) delete _ctx->title;

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
			if (_ctx->num[_ctx->i]) delete _ctx->num[_ctx->i];
	}

	CORO_END_CODE;
}

void RMOptionScreen::refreshThumbnails(void) {
	int i;

	for (i = 0; i < 6; i++) {
		if (_curThumb[i])
			delete _curThumb[i];

		_curThumb[i] = new RMGfxSourceBuffer16;
		_curThumb[i]->create(640 / 4, 480 / 4);
		if (!loadThumbnailFromSaveState(_statePos + i, *_curThumb[i], _curThumbName[i], _curThumbDiff[i])) {
			delete _curThumb[i];
			_curThumb[i] = NULL;
			_curThumbName[i] = "";
			_curThumbDiff[i] = 11;
		}
	}
}


void RMOptionScreen::initState(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMResRaw *raw;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND)
		_ctx->raw = new RMResRaw(20000 + _nState);
	else if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_bAlterGfx)
			_ctx->raw = new RMResRaw(20024);
		else
			_ctx->raw = new RMResRaw(20003);
	} else {
		error("Invalid state");
	}

	assert(_ctx->raw->isValid());
	assert(_menu == NULL);
	_menu = new RMGfxSourceBuffer16(false);
	_menu->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
	delete _ctx->raw;

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_bAlterGfx) {
			assert(_ButtonExit == NULL);
			_ButtonExit = new RMOptionButton(20025, RMPoint(561, 406));
		} else {
			assert(_ButtonExit == NULL);
			_ButtonExit = new RMOptionButton(20012, RMPoint(560, 404));
		}

		INIT_GFX8_FROMRAW(_ctx->raw, 20036, _SaveEasy);
		INIT_GFX8_FROMRAW(_ctx->raw, 20037, _SaveHard);

		refreshThumbnails();

		assert(_ButtonSave_States[0] == NULL);
		_ButtonSave_States[0] = new RMOptionButton(RMRect(48, 57, 48 + 160, 57 + 120));
		assert(_ButtonSave_States[1] == NULL);
		_ButtonSave_States[1] = new RMOptionButton(RMRect(240, 57, 240 + 160, 57 + 120));
		assert(_ButtonSave_States[2] == NULL);
		_ButtonSave_States[2] = new RMOptionButton(RMRect(432, 57, 432 + 160, 57 + 120));
		assert(_ButtonSave_States[3] == NULL);
		_ButtonSave_States[3] = new RMOptionButton(RMRect(48, 239, 48 + 160, 239 + 120));
		assert(_ButtonSave_States[4] == NULL);
		_ButtonSave_States[4] = new RMOptionButton(RMRect(240, 239, 240 + 160, 239 + 120));
		assert(_ButtonSave_States[5] == NULL);
		_ButtonSave_States[5] = new RMOptionButton(RMRect(432, 239, 432 + 160, 239 + 120));

		if (_bAlterGfx) {
			assert(_ButtonSave_ArrowLeft == NULL);
			_ButtonSave_ArrowLeft = new RMOptionButton(20026, RMPoint(3, 196));
			assert(_ButtonSave_ArrowRight == NULL);
			_ButtonSave_ArrowRight = new RMOptionButton(20027, RMPoint(601, 197));
		} else {
			assert(_ButtonSave_ArrowLeft == NULL);
			_ButtonSave_ArrowLeft = new RMOptionButton(20013, RMPoint(0, 197));
			assert(_ButtonSave_ArrowRight == NULL);
			_ButtonSave_ArrowRight = new RMOptionButton(20014, RMPoint(601, 197));
		}
	} else if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		assert(_ButtonExit == NULL);
		_ButtonExit = new RMOptionButton(20005, RMPoint(560, 405));
		assert(_ButtonQuit == NULL);
		_ButtonQuit = new RMOptionButton(20020, RMPoint(7, 408));
		assert(_ButtonLoad == NULL);
		_ButtonLoad = new RMOptionButton(20006, RMPoint(231, 401));
		assert(_ButtonSave == NULL);
		_ButtonSave = new RMOptionButton(20007, RMPoint(325, 401));

		assert(_ButtonGameMenu == NULL);
		_ButtonGameMenu = new RMOptionButton(RMRect(24, 32, 118, 64));
		assert(_ButtonGfxMenu == NULL);
		_ButtonGfxMenu = new RMOptionButton(RMRect(118, 32, 212, 64));
		assert(_ButtonSoundMenu == NULL);
		_ButtonSoundMenu = new RMOptionButton(RMRect(212, 32, 306, 64));

		_ctx->raw = new RMResRaw(20021);
		assert(_ctx->raw->isValid());
		assert(_QuitConfirm == NULL);
		_QuitConfirm = new RMGfxSourceBuffer16(false);
		_QuitConfirm->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
		delete _ctx->raw;

		assert(_ButtonQuitYes == NULL);
		_ButtonQuitYes = new RMOptionButton(20022, RMPoint(281, 265));
		_ButtonQuitYes->setPriority(30);
		assert(_ButtonQuitNo == NULL);
		_ButtonQuitNo = new RMOptionButton(20023, RMPoint(337, 264));
		_ButtonQuitNo->setPriority(30);

		if (_bNoLoadSave) {
			_ctx->raw = new RMResRaw(20028);
			assert(_ctx->raw->isValid());
			assert(_HideLoadSave == NULL);
			_HideLoadSave = new RMGfxSourceBuffer16(false);
			_HideLoadSave->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
			delete _ctx->raw;
		}

		// Menu GAME
		if (_nState == MENUGAME) {
			assert(_ButtonGame_Lock == NULL);
			_ButtonGame_Lock = new RMOptionButton(20008, RMPoint(176, 262), true);
			_ButtonGame_Lock->setActiveState(GLOBALS._bCfgInvLocked);
			assert(_ButtonGame_TimerizedText == NULL);
			_ButtonGame_TimerizedText = new RMOptionButton(20009, RMPoint(463, 273), true);
			_ButtonGame_TimerizedText->setActiveState(!GLOBALS._bCfgTimerizedText);
			assert(_ButtonGame_Scrolling == NULL);
			_ButtonGame_Scrolling = new RMOptionButton(20010, RMPoint(315, 263), true);
			_ButtonGame_Scrolling->setActiveState(GLOBALS._bCfgInvNoScroll);
			assert(_ButtonGame_InterUp == NULL);
			_ButtonGame_InterUp = new RMOptionButton(20011, RMPoint(36, 258), true);
			_ButtonGame_InterUp->setActiveState(GLOBALS._bCfgInvUp);

			assert(_SlideTextSpeed == NULL);
			_SlideTextSpeed = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS._nCfgTextSpeed);
			assert(_SlideTonySpeed == NULL);
			_SlideTonySpeed = new RMOptionSlide(RMPoint(165, 226), 5, GLOBALS._nCfgTonySpeed);
		}
		// Menu Graphics
		else if (_nState == MENUGFX) {
			assert(_ButtonGfx_Anni30 == NULL);
			_ButtonGfx_Anni30 = new RMOptionButton(20015, RMPoint(247, 178), true);
			_ButtonGfx_Anni30->setActiveState(GLOBALS._bCfgAnni30);
			assert(_ButtonGfx_AntiAlias == NULL);
			_ButtonGfx_AntiAlias = new RMOptionButton(20016, RMPoint(430, 83), true);
			_ButtonGfx_AntiAlias->setActiveState(!GLOBALS._bCfgAntiAlias);
			assert(_ButtonGfx_Sottotitoli == NULL);
			_ButtonGfx_Sottotitoli = new RMOptionButton(20017, RMPoint(98, 82), true);
			_ButtonGfx_Sottotitoli->setActiveState(!GLOBALS._bCfgSottotitoli);
			assert(_ButtonGfx_Tips == NULL);
			_ButtonGfx_Tips = new RMOptionButton(20018, RMPoint(431, 246), true);
			_ButtonGfx_Tips->setActiveState(GLOBALS._bCfgInterTips);
			assert(_ButtonGfx_Trans == NULL);
			_ButtonGfx_Trans = new RMOptionButton(20019, RMPoint(126, 271), true);
			_ButtonGfx_Trans->setActiveState(!GLOBALS._bCfgTransparence);

		} else if (_nState == MENUSOUND) {
			assert(_SliderSound_Dubbing == NULL);
			_SliderSound_Dubbing = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS._nCfgDubbingVolume);
			assert(_SliderSound_Music == NULL);
			_SliderSound_Music = new RMOptionSlide(RMPoint(165, 226), 10, GLOBALS._nCfgMusicVolume);
			assert(_SliderSound_SFX == NULL);
			_SliderSound_SFX = new RMOptionSlide(RMPoint(165, 330), 10, GLOBALS._nCfgSFXVolume);

			assert(_ButtonSound_DubbingOn == NULL);
			_ButtonSound_DubbingOn = new RMOptionButton(20033, RMPoint(339, 75), true);
			_ButtonSound_DubbingOn->setActiveState(GLOBALS._bCfgDubbing);
			assert(_ButtonSound_MusicOn == NULL);
			_ButtonSound_MusicOn = new RMOptionButton(20034, RMPoint(338, 179), true);
			_ButtonSound_MusicOn->setActiveState(GLOBALS._bCfgMusic);
			assert(_ButtonSound_SFXOn == NULL);
			_ButtonSound_SFXOn = new RMOptionButton(20035, RMPoint(338, 283), true);
			_ButtonSound_SFXOn->setActiveState(GLOBALS._bCfgSFX);
		}
	}

	CORO_INVOKE_0(refreshAll);

	CORO_END_CODE;
}

void RMOptionScreen::closeState(void) {
	delete _menu;
	_menu = NULL;

	delete _ButtonExit;
	_ButtonExit = NULL;

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		int i;

		for (i = 0; i < 6; i++) {
			if (_curThumb[i] != NULL) {
				delete _curThumb[i];
				_curThumb[i] = NULL;
			}

			delete _ButtonSave_States[i];
			_ButtonSave_States[i] = NULL;
		}

		delete _ButtonSave_ArrowLeft;
		_ButtonSave_ArrowLeft = NULL;
		delete _ButtonSave_ArrowRight;
		_ButtonSave_ArrowRight = NULL;

		delete _SaveEasy;
		_SaveEasy = NULL;
		delete _SaveHard;
		_SaveHard = NULL;
	}

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		delete _ButtonQuit;
		_ButtonQuit = NULL;
		delete _ButtonLoad;
		_ButtonLoad = NULL;
		delete _ButtonSave;
		_ButtonSave = NULL;
		delete _ButtonGameMenu;
		_ButtonGameMenu = NULL;
		delete _ButtonGfxMenu;
		_ButtonGfxMenu = NULL;
		delete _ButtonSoundMenu;
		_ButtonSoundMenu = NULL;
		delete _QuitConfirm;
		_QuitConfirm = NULL;
		delete _ButtonQuitYes;
		_ButtonQuitYes = NULL;
		delete _ButtonQuitNo;
		_ButtonQuitNo = NULL;

		if (_bNoLoadSave) {
			delete _HideLoadSave;
			_HideLoadSave = NULL;
		}

		if (_nState == MENUGAME) {
			GLOBALS._bCfgInvLocked = _ButtonGame_Lock->isActive();
			delete _ButtonGame_Lock;
			_ButtonGame_Lock = NULL;

			GLOBALS._bCfgTimerizedText = !_ButtonGame_TimerizedText->isActive();
			delete _ButtonGame_TimerizedText;
			_ButtonGame_TimerizedText = NULL;

			GLOBALS._bCfgInvNoScroll = _ButtonGame_Scrolling->isActive();
			delete _ButtonGame_Scrolling;
			_ButtonGame_Scrolling = NULL;

			GLOBALS._bCfgInvUp = _ButtonGame_InterUp->isActive();
			delete _ButtonGame_InterUp;
			_ButtonGame_InterUp = NULL;

			GLOBALS._nCfgTextSpeed = _SlideTextSpeed->getValue();
			delete _SlideTextSpeed;
			_SlideTextSpeed = NULL;

			GLOBALS._nCfgTonySpeed = _SlideTonySpeed->getValue();
			delete _SlideTonySpeed;
			_SlideTonySpeed = NULL;
		} else if (_nState == MENUGFX) {
			GLOBALS._bCfgAnni30 = _ButtonGfx_Anni30->isActive();
			delete _ButtonGfx_Anni30;
			_ButtonGfx_Anni30 = NULL;

			GLOBALS._bCfgAntiAlias = !_ButtonGfx_AntiAlias->isActive();
			delete _ButtonGfx_AntiAlias;
			_ButtonGfx_AntiAlias = NULL;

			GLOBALS._bCfgSottotitoli = !_ButtonGfx_Sottotitoli->isActive();
			delete _ButtonGfx_Sottotitoli;
			_ButtonGfx_Sottotitoli = NULL;

			GLOBALS._bCfgInterTips = _ButtonGfx_Tips->isActive();
			delete _ButtonGfx_Tips;
			_ButtonGfx_Tips = NULL;

			GLOBALS._bCfgTransparence = !_ButtonGfx_Trans->isActive();
			delete _ButtonGfx_Trans;
			_ButtonGfx_Trans = NULL;
		} else if (_nState == MENUSOUND) {
			GLOBALS._nCfgDubbingVolume = _SliderSound_Dubbing->getValue();
			delete _SliderSound_Dubbing;
			_SliderSound_Dubbing = NULL;

			GLOBALS._nCfgMusicVolume = _SliderSound_Music->getValue();
			delete _SliderSound_Music;
			_SliderSound_Music = NULL;

			GLOBALS._nCfgSFXVolume = _SliderSound_SFX->getValue();
			delete _SliderSound_SFX;
			_SliderSound_SFX = NULL;

			GLOBALS._bCfgDubbing = _ButtonSound_DubbingOn->isActive();
			delete _ButtonSound_DubbingOn;
			_ButtonSound_DubbingOn = NULL;

			GLOBALS._bCfgMusic = _ButtonSound_MusicOn->isActive();
			delete _ButtonSound_MusicOn;
			_ButtonSound_MusicOn = NULL;

			GLOBALS._bCfgSFX = _ButtonSound_SFXOn->isActive();
			delete _ButtonSound_SFXOn;
			_ButtonSound_SFXOn = NULL;
		}
	}

	_nState = MENUNONE;
}

void RMOptionScreen::reInit(RMGfxTargetBuffer &bigBuf) {
	bigBuf.addPrim(new RMGfxPrimitive(this));
}

void RMOptionScreen::init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_FadeStep != 0) {
		result = false;
		return;
	}

	_FadeStep = 1;
	_FadeY = -20;
	_FadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = false;
	_bNoLoadSave = false;
	_bAlterGfx = false;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	if (_nState == MENULOAD || _nState == MENUSAVE)
		_nState = MENUGAME;

	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_FadeStep != 0) {
		result = false;
		return;
	}

	_FadeStep = 1;
	_FadeY = -20;
	_FadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = true;
	_bNoLoadSave = false;
	_bAlterGfx = bAlternateGfx;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENULOAD;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_FadeStep != 0) {
		result = false;
		return;
	}

	_FadeStep = 1;
	_FadeY = -20;
	_FadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = true;
	_bNoLoadSave = false;
	_bAlterGfx = bAlternateGfx;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENUSAVE;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_FadeStep != 0) {
		result = false;
		return;
	}

	_FadeStep = 1;
	_FadeY = -20;
	_FadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = false;
	_bNoLoadSave = true;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENUGAME;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

bool RMOptionScreen::close(void) {
	if (_FadeStep != 6)
		return false;

	// Start fade out
	_FadeStep++;
	_FadeTime = _vm->getTime();
	return true;
}

bool RMOptionScreen::isClosing(void) {
	return _bExit;
}

int RMOptionScreen::priority() {
	// Just below the mouse
	return 190;
}

void RMOptionScreen::changeState(CORO_PARAM, STATE newState) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	closeState();
	_nLastState = _nState;
	_nState = newState;
	CORO_INVOKE_0(initState);

	CORO_END_CODE;
}

void RMOptionScreen::doFrame(CORO_PARAM, RMInput *input) {
	CORO_BEGIN_CONTEXT;
	bool bLeftClick, bRightClick;
	RMPoint mousePos;
	bool bRefresh;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);


	// If it is fully open, do nothing
	if (_FadeStep != 6)
		return;

	// Reads input
	_ctx->mousePos = input->mousePos();
	_ctx->bLeftClick = input->mouseLeftClicked();
	_ctx->bRightClick = input->mouseRightClicked();

	_ctx->bRefresh = false;

	if (_bQuitConfirm) {
		_ctx->bRefresh |= _ButtonQuitYes->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
		_ctx->bRefresh |= _ButtonQuitNo->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
	} else {
		_ctx->bRefresh |= _ButtonExit->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		// Check if you have clicked on the output
		if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
			// Buttons without graphics...
			_ButtonGameMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ButtonGfxMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ButtonSoundMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			// Buttons with graphics
			if (!_bNoLoadSave) {
				if (!_vm->getIsDemo()) {
					_ctx->bRefresh |= _ButtonLoad->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
					_ctx->bRefresh |= _ButtonSave->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
				}

				_ctx->bRefresh |= _ButtonQuit->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			}
		}

		if (_nState == MENUGAME) {
			_ctx->bRefresh |= _ButtonGame_Lock->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGame_TimerizedText->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGame_Scrolling->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGame_InterUp->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _SlideTextSpeed->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _SlideTonySpeed->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENUGFX) {
			_ctx->bRefresh |= _ButtonGfx_Anni30->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGfx_AntiAlias->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGfx_Sottotitoli->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGfx_Tips->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonGfx_Trans->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENUSOUND) {
			_ctx->bRefresh |= _SliderSound_Dubbing->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _SliderSound_Music->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _SliderSound_SFX->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonSound_DubbingOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonSound_MusicOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _ButtonSound_SFXOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENULOAD || _nState == MENUSAVE) {
			for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
				_ButtonSave_States[_ctx->i]->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			if (_statePos > 0)
				_ctx->bRefresh |= _ButtonSave_ArrowLeft->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			if (_statePos < 90)
				_ctx->bRefresh |= _ButtonSave_ArrowRight->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		}
	}

#define KEYPRESS(c)     (_vm->getEngine()->getInput().getAsyncKeyState(c))
#define PROCESS_CHAR(cod,c)  if (KEYPRESS(cod)) { \
		_EditName[strlen(_EditName) + 1] = '\0'; _EditName[strlen(_EditName)] = c; _ctx->bRefresh = true; }

	/**************** State Buttons **************/
	if (_bEditSaveName) {
		if (KEYPRESS(Common::KEYCODE_BACKSPACE)) {
			if (_EditName[0] != '\0') {
				_EditName[strlen(_EditName) - 1] = '\0';
				_ctx->bRefresh = true;
			}
		}

		for (_ctx->i = 0; _ctx->i < 26 && strlen(_EditName) < 12; _ctx->i++)
			if (KEYPRESS(Common::KEYCODE_LSHIFT) ||
			        KEYPRESS(Common::KEYCODE_RSHIFT)) {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'A');
			} else {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'a');
			}

		for (_ctx->i = 0; _ctx->i < 10 && strlen(_EditName) < 12; _ctx->i++)
			PROCESS_CHAR((Common::KeyCode)((int)'0' + _ctx->i), _ctx->i + '0');

		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_SPACE, ' ');

		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP0, '0');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP1, '1');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP2, '2');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP3, '3');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP4, '4');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP5, '5');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP6, '6');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP7, '7');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP8, '8');
		if (strlen(_EditName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP9, '9');

		// Cancel
		if (KEYPRESS(Common::KEYCODE_ESCAPE)) {
			_bEditSaveName = false;
			_ctx->bRefresh = true;
		}

		// OK
		if (KEYPRESS(Common::KEYCODE_RETURN)) {
			_bEditSaveName = false;
			_vm->saveState(_statePos + _nEditPos, _EditName);
			close();
		}

	} else if (_ctx->bLeftClick) {
		if (_nState == MENULOAD || _nState == MENUSAVE) {
			if (_ButtonExit->isActive()) {
				if (_bLoadMenuOnly) {
					// If only the loading menu, close
					close();
				} else {
					CORO_INVOKE_1(changeState, _nLastState);
					_ctx->bRefresh = true;
				}
			} else if (_ButtonSave_ArrowLeft->isActive()) {
				if (_statePos > 0) {
					_statePos -= 6;
					if (_statePos < 0)
						_statePos = 0;
					_ButtonSave_ArrowLeft->setActiveState(false);
					_ctx->bRefresh = true;
					refreshThumbnails();
				}
			} else if (_ButtonSave_ArrowRight->isActive()) {
				if (_statePos < 90) {
					_statePos += 6;
					if (_statePos > 90)
						_statePos = 90;
					_ButtonSave_ArrowRight->setActiveState(false);
					_ctx->bRefresh = true;
					refreshThumbnails();
				}
			} else {
				for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
					if (_ButtonSave_States[_ctx->i]->isActive()) {
						// There by saving or loading!!!
						if (_nState == MENULOAD && _curThumb[_ctx->i] != NULL) {
							// Caricamento
							CORO_INVOKE_1(_vm->loadState, _statePos + _ctx->i);
							close();
						} else if (_nState == MENUSAVE && (_statePos != 0 || _ctx->i != 0)) {
							// Turn on edit mode
							_bEditSaveName = true;
							_nEditPos = _ctx->i;
							strcpy(_EditName, _curThumbName[_ctx->i]);
							_ctx->bRefresh = true;
						}

						break;
					}
			}
		}

		if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
			if (_bQuitConfirm) {
				if (_ButtonQuitNo->isActive()) {
					_bQuitConfirm = false;
					_ctx->bRefresh = true;
				} else if (_ButtonQuitYes->isActive()) {
					_bQuitConfirm = false;
					_ctx->bRefresh = true;

					_vm->quitGame();
				}
			} else {
				if (_ButtonQuit->isActive()) {
					_bQuitConfirm = true;
					_ButtonQuitNo->setActiveState(false);
					_ButtonQuitYes->setActiveState(false);
					_ctx->bRefresh = true;
				} else if (_ButtonExit->isActive())
					close();
				else if (_ButtonLoad->isActive()) {
					CORO_INVOKE_1(changeState, MENULOAD);
					_ctx->bRefresh = true;
				} else if (_ButtonSave->isActive()) {
					CORO_INVOKE_1(changeState, MENUSAVE);
					_ctx->bRefresh = true;
				} else if (_ButtonGameMenu->isActive() && _nState != MENUGAME) {
					CORO_INVOKE_1(changeState, MENUGAME);
					_ctx->bRefresh = true;
				} else if (_ButtonGfxMenu->isActive() && _nState != MENUGFX) {
					CORO_INVOKE_1(changeState, MENUGFX);
					_ctx->bRefresh = true;
				} else if (_ButtonSoundMenu->isActive() && _nState != MENUSOUND) {
					CORO_INVOKE_1(changeState, MENUSOUND);
					_ctx->bRefresh = true;
				}

				if (_nState == MENUGFX) {
					// These options take effect immediately
					if (_ButtonGfx_Anni30->isActive())
						GLOBALS._bCfgAnni30 = true;
					else
						GLOBALS._bCfgAnni30 = false;

					if (_ButtonGfx_AntiAlias->isActive())
						GLOBALS._bCfgAntiAlias = false;
					else
						GLOBALS._bCfgAntiAlias = true;

					if (_ButtonGfx_Trans->isActive())
						GLOBALS._bCfgTransparence = false;
					else
						GLOBALS._bCfgTransparence = true;
				}
			}
		}
	}

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND)
		if (!_bQuitConfirm && KEYPRESS(Common::KEYCODE_ESCAPE))
			close();

	if (_ctx->bRefresh)
		CORO_INVOKE_0(refreshAll);

	CORO_END_CODE;
}


void RMOptionScreen::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int curTime;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = _vm->getTime();

#define FADE_SPEED 20
#define SYNC    (_ctx->curTime - _FadeTime) / 25

	if (_bExit)
		return;

	if (_FadeStep == 1) {
		// Downhill fast
		if (_FadeTime == -1)
			_FadeY += FADE_SPEED;
		else
			_FadeY += FADE_SPEED * SYNC;
		if (_FadeY > 480) {
			_FadeY = 480;
			_FadeStep++;
		}

		// Set the part to draw the scrolling
		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 2) {
		// Bounce 1
		_FadeY -= FADE_SPEED / 2 * SYNC;
		if (_FadeY < 400) {
			_FadeY = 400;
			_FadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 3) {
		_FadeY -= FADE_SPEED / 4 * SYNC;
		if (_FadeY < 380) {
			_FadeY = 380;
			_FadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 4) {
		// Bounce 1 - 2
		_FadeY += FADE_SPEED / 3 * SYNC;
		if (_FadeY > 420) {
			_FadeY = 420;
			_FadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 5) {
		_FadeY += FADE_SPEED / 2 * SYNC;
		if (_FadeY > 480) {
			_FadeY = 480;
			_FadeStep++;
			_vm->hideLocation();
		}

		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 6) {
		// Menu ON

	} else if (_FadeStep == 7) {
		// Menu OFF
		_vm->showLocation();
		_FadeStep++;

	} else if (_FadeStep == 8) {
		_FadeY -= FADE_SPEED * SYNC;
		if (_FadeY < 0) {
			_FadeY = 0;
			_FadeStep++;
		}
		prim->setSrc(RMRect(0, 480 - _FadeY, 640, 480));

	} else if (_FadeStep == 9) {
		// Hello hello!
		_bExit = true;
		_FadeStep = 0;

		// Free memory
		closeState();
		return;

	} else {
		_FadeStep = 0;
	}

	_FadeTime = _ctx->curTime;

	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionScreen::removeThis(CORO_PARAM, bool &result) {
	if (_bExit)
		result = true;
	else
		result = false;
}


bool RMOptionScreen::loadThumbnailFromSaveState(int nState, byte *lpDestBuf, RMString &name, byte &diff) {
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
	buf = _vm->getSaveStateFileName(nState);

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

		// Read in the compressed data
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
	Common::fill(_pointer, _pointer + 16, (RMGfxSourceBuffer8 *)NULL);
	Common::fill(_specialPointer, _specialPointer + 16, (RMItem *)NULL);
}

RMPointer::~RMPointer() {
	close();
}

void RMPointer::init(void) {
	int i;

	for (i = 0; i < 5; i++) {
		RMResRaw res(RES_P_GO + i);

		_pointer[i] = new RMGfxSourceBuffer8RLEByteAA;
		_pointer[i]->init(res, res.width(), res.height(), false);
		_pointer[i]->loadPaletteWA(RES_P_PAL);
	}

	for (i = 0; i < 5; i++) {
		RMRes res(RES_P_PAP1 + i);
		RMDataStream ds;

		ds.openBuffer(res);
		_specialPointer[i] = new RMItem;
		ds >> *_specialPointer[i];
	}

	//m_hotspot[0].set(19,5);
	_hotspot[0].set(5, 1);
	_hotspot[1].set(32, 28);
	_hotspot[2].set(45, 23);
	_hotspot[3].set(35, 25);
	_hotspot[4].set(32, 28);

	// Default=GO
	_nCurPointer = 0;
	_nCurSpecialPointer = 0;
}

void RMPointer::close(void) {
	int i;

	for (i = 0; i < 5; i++) {
		if (_pointer[i] != NULL) {
			delete _pointer[i];
			_pointer[i] = NULL;
		}

		if (_specialPointer[i] != NULL) {
			delete _specialPointer[i];
			_specialPointer[i] = NULL;
		}
	}
}


int RMPointer::priority() {
	// Pointer has minimum priority so it will be drawn last
	return 200;
}

void RMPointer::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int n;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Check the pointer
	_ctx->n = _nCurPointer;
	if (_ctx->n == TA_COMBINE) _ctx->n = TA_USE;

	// Copy the destination coordinates in the primitive
	prim->setDst(_pos);

	if (_pos.x >= 0 && _pos.y >= 0 && _pos.x < RM_SX && _pos.y < RM_SY) {
		// Call the Draw method of the poitner
		prim->getDst() -= _hotspot[_ctx->n];

		if (_nCurSpecialPointer == 0) {
			CORO_INVOKE_2(_pointer[_ctx->n]->draw, bigBuf, prim);
		} else {
			if (_nCurSpecialPointer == PTR_CUSTOM)
				CORO_INVOKE_2(_nCurCustomPointer->draw, bigBuf, prim);
			else
				// Call the draw on the special pointer
				CORO_INVOKE_2(_specialPointer[_nCurSpecialPointer - 1]->draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMPointer::doFrame(RMGfxTargetBuffer *bigBuf) {
	// Add it to the list of primitives
	bigBuf->addPrim(new RMGfxPrimitive(this));

	// If there is a special pointer, does DoFrame
	if (_nCurSpecialPointer != 0 && _nCurSpecialPointer != PTR_CUSTOM)
		_specialPointer[_nCurSpecialPointer - 1]->doFrame(bigBuf, false);
}

void RMPointer::removeThis(CORO_PARAM, bool &result) {
	// Always remove from the OT list, to support disabling the pointer
	result = true;
}

int RMPointer::curAction(void) {
	if (_nCurSpecialPointer != 0)
		return 0;

	return _nCurPointer;
}

} // End of namespace Tony
