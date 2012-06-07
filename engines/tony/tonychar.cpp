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

#include "tony/mpal/memory.h"
#include "tony/mpal/mpalutils.h"
#include "tony/game.h"
#include "tony/tonychar.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       Metodi di RMTony
\****************************************************************************/

bool RMTony::_bAction = false;

void RMTony::initStatics() {
	_bAction = false;
}

RMTony::RMTony() {
	_bShow = false;
	_bShowOmbra = false;
	_bCorpoDavanti = false;
	_bActionPending = false;
	_ActionItem = NULL;
	_Action = 0;
	_ActionParm = 0;
	_bPastorella = false;
	_bIsStaticTalk = false;
	_bIsTalking = false;
	_nPatB4Talking = 0;
	_nTalkType = TALK_NORMAL;
	_TalkDirection = UP;
	_nTimeLastStep = 0;
	hActionThread = CORO_INVALID_PID_VALUE;
}

void RMTony::waitEndOfAction(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	_bAction = false;

	CORO_END_CODE;
}

RMGfxSourceBuffer *RMTony::newItemSpriteBuffer(int dimx, int dimy, bool bPreRLE) {
	RMGfxSourceBuffer8RLE *spr;

	assert(_cm == CM_256);
	spr = new RMGfxSourceBuffer8RLEByteAA;
	spr->setAlphaBlendColor(1);
	if (bPreRLE)
		spr->setAlreadyCompressed();
	return spr;
}

void RMTony::init(void) {
	RMRes tony(0);
	RMRes body(9999);
	RMDataStream ds;

	// Tony is shown by default
	_bShow = _bShowOmbra = true;

	// No action pending
	_bActionPending = false;
	_bAction = false;

	_bPastorella = false;
	_bIsTalking = false;
	_bIsStaticTalk = false;

	// Opens the buffer
	ds.openBuffer(tony);

	// Reads his details from the stream
	readFromStream(ds, true);

	// Closes the buffer
	ds.close();

	// Reads Tony's body
	ds.openBuffer(body);
	_body.readFromStream(ds, true);
	ds.close();
	_body.setPattern(0);

	_nTimeLastStep = _vm->getTime();
}


void RMTony::close(void) {
	// Disalloca @@@ Deallocation of missing item
	_ombra.destroy();
}

void RMTony::doFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int curLoc) {
	CORO_BEGIN_CONTEXT;
	int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!_nInList && _bShow)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	setSpeed(GLOBALS.nCfgTonySpeed);

	// Runs the normal character movement
	_ctx->time = _vm->getTime();

	do {
		_nTimeLastStep += (1000 / 40);
		CORO_INVOKE_2(RMCharacter::doFrame, bigBuf, curLoc);
	} while (_ctx->time > _nTimeLastStep + (1000 / 40));

	// Check if we are at the end of a path
	if (endOfPath() && _bActionPending) {
		// Must perform the action on which we clicked
		_bActionPending = false;
	}

	if (_bIsTalking || _bIsStaticTalk)
		_body.doFrame(bigBuf, false);

	CORO_END_CODE;
}

void RMTony::show(void) {
	_bShow = true;
	_bShowOmbra = true;
}

void RMTony::hide(bool bShowOmbra) {
	_bShow = false;
	_bShowOmbra = bShowOmbra;
}


void RMTony::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Call the Draw() of the parent class if Tony is visible
	if (_bShow && bDrawNow) {
		if (_bCorpoDavanti) {
			prim->Dst().setEmpty();
			prim->Dst().offset(-44, -134);
			if (_bPastorella)
				prim->Dst().offset(1, 4);
			CORO_INVOKE_2(RMCharacter::draw, bigBuf, prim);
		}

		if (_bIsTalking || _bIsStaticTalk) {
			// Offest direction from scrolling
			prim->Dst().setEmpty();
			prim->Dst().offset(-_curScroll);
			prim->Dst().offset(_pos);
			prim->Dst().offset(-44, -134);
			prim->Dst() += _nBodyOffset;
			CORO_INVOKE_2(_body.draw, bigBuf, prim);
		}

		if (!_bCorpoDavanti) {
			prim->Dst().setEmpty();
			prim->Dst().offset(-44, -134);
			if (_bPastorella)
				prim->Dst().offset(0, 3);
			CORO_INVOKE_2(RMCharacter::draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTony::moveAndDoAction(CORO_PARAM, RMPoint dst, RMItem *item, int nAction, int nActionParm) {
	CORO_BEGIN_CONTEXT;
	bool result;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Makes normal movement, but remember if you must then perform an action
	if (item == NULL) {
		_bActionPending = false;
		_ActionItem = NULL;
	} else {
		_ActionItem = item;
		_Action = nAction;
		_ActionParm = nActionParm;
		_bActionPending = true;
	}

	CORO_INVOKE_2(RMCharacter::move, dst, &_ctx->result);
	if (!_ctx->result) {
		_bActionPending = false;
		_ActionItem = NULL;
	}

	CORO_END_CODE;
}


void RMTony::executeAction(int nAction, int nActionItem, int nParm) {
	uint32 pid;

	if (nAction == TA_COMBINE) {
		pid = mpalQueryDoAction(TA_COMBINE, nParm, nActionItem);

		// If you failed the combine, we have RECEIVECOMBINE as a fallback
		if (pid == CORO_INVALID_PID_VALUE) {
			pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, nParm);

			// If you failed with that, go with the generic
			// @@@ CombineGive!
			if (pid == CORO_INVALID_PID_VALUE) {
				pid = mpalQueryDoAction(TA_COMBINE, nParm, 0);

				if (pid == CORO_INVALID_PID_VALUE) {
					pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, 0);
				}
			}
		}
	} else {
		// Perform the action
		pid = mpalQueryDoAction(nAction, nActionItem, 0);
	}

	if (pid != CORO_INVALID_PID_VALUE) {
		_bAction = true;
		CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
		hActionThread = pid;
	} else if (nAction != TA_GOTO) {
		if (nAction == TA_TALK) {
			pid = mpalQueryDoAction(6, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			hActionThread = pid;
		} else if (nAction == TA_PALESATI) {
			pid = mpalQueryDoAction(7, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			hActionThread = pid;
		} else {
			pid = mpalQueryDoAction(5, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			hActionThread = pid;
		}
	}
}


void RMTony::stopNoAction(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bAction)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, hActionThread, CORO_INFINITE);

	_bActionPending = false;
	_ActionItem = NULL;
	CORO_INVOKE_0(stop);

	CORO_END_CODE;
}

void RMTony::stop(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	uint32 pid;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_ActionItem != NULL) {
		// Call MPAL to choose the direction
		_ctx->pid = mpalQueryDoAction(21, _ActionItem->mpalCode(), 0);

		if (_ctx->pid == CORO_INVALID_PID_VALUE)
			CORO_INVOKE_0(RMCharacter::stop);
		else {
			bNeedToStop = false;    // If we make the OnWhichDirection, we don't need at least after the Stop().
			bMoving = false;
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->pid, CORO_INFINITE); // @@@ Put an assert after 10 seconds
		}
	} else {
		CORO_INVOKE_0(RMCharacter::stop);
	}

	if (!_bActionPending)
		return;

	_bActionPending = false;

	executeAction(_Action, _ActionItem->mpalCode(), _ActionParm);

	_ActionItem = NULL;

	CORO_END_CODE;
}


int RMTony::getCurPattern(void) {
	int nPatt = RMCharacter::getCurPattern();

	if (!_bPastorella)
		return nPatt;

	switch (nPatt) {
	case PAT_PAST_STANDUP:
		return PAT_STANDUP;
	case PAT_PAST_STANDDOWN:
		return PAT_STANDDOWN;
	case PAT_PAST_STANDLEFT:
		return PAT_STANDLEFT;
	case PAT_PAST_STANDRIGHT:
		return PAT_STANDRIGHT;
	case PAT_PAST_WALKUP:
		return PAT_WALKUP;
	case PAT_PAST_WALKDOWN:
		return PAT_WALKDOWN;
	case PAT_PAST_WALKLEFT:
		return PAT_WALKLEFT;
	case PAT_PAST_WALKRIGHT:
		return PAT_WALKRIGHT;
	}

	return nPatt;
}

void RMTony::setPattern(int nPatt, bool bPlayP0) {
	if (_bPastorella) {
		switch (nPatt) {
		case PAT_STANDUP:
			nPatt = PAT_PAST_STANDUP;
			break;
		case PAT_STANDDOWN:
			nPatt = PAT_PAST_STANDDOWN;
			break;
		case PAT_STANDLEFT:
			nPatt = PAT_PAST_STANDLEFT;
			break;
		case PAT_STANDRIGHT:
			nPatt = PAT_PAST_STANDRIGHT;
			break;
		case PAT_WALKUP:
			nPatt = PAT_PAST_WALKUP;
			break;
		case PAT_WALKDOWN:
			nPatt = PAT_PAST_WALKDOWN;
			break;
		case PAT_WALKLEFT:
			nPatt = PAT_PAST_WALKLEFT;
			break;
		case PAT_WALKRIGHT:
			nPatt = PAT_PAST_WALKRIGHT;
			break;
		}
	}

	RMCharacter::setPattern(nPatt, bPlayP0);
}


void RMTony::take(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (getCurPattern()) {
		case PAT_STANDDOWN:
			assert(0);  // Not while you're doing a StandDown
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKEUP_UP1);
				break;
			case 1:
				setPattern(PAT_TAKEUP_MID1);
				break;
			case 2:
				setPattern(PAT_TAKEUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKERIGHT_UP1);
				break;
			case 1:
				setPattern(PAT_TAKERIGHT_MID1);
				break;
			case 2:
				setPattern(PAT_TAKERIGHT_DOWN1);
				break;
			}
			break;

		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKELEFT_UP1);
				break;
			case 1:
				setPattern(PAT_TAKELEFT_MID1);
				break;
			case 2:
				setPattern(PAT_TAKELEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		setPattern(getCurPattern() + 1);
	} else if (nPart == 2) {
		switch (getCurPattern()) {
		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
			setPattern(PAT_STANDUP);
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
			setPattern(PAT_STANDLEFT);
			break;

		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
			setPattern(PAT_STANDRIGHT);
			break;
		}
	}
}


void RMTony::put(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (getCurPattern()) {
		case PAT_STANDDOWN:
			//assert(0);
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTUP_UP1);
				break;
			case 1:
				setPattern(PAT_PUTUP_MID1);
				break;
			case 2:
				setPattern(PAT_PUTUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTRIGHT_UP1);
				break;
			case 1:
				setPattern(PAT_PUTRIGHT_MID1);
				break;
			case 2:
				setPattern(PAT_PUTRIGHT_DOWN1);
				break;
			}
			break;

		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTLEFT_UP1);
				break;
			case 1:
				setPattern(PAT_PUTLEFT_MID1);
				break;
			case 2:
				setPattern(PAT_PUTLEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		setPattern(getCurPattern() + 1);
	} else if (nPart == 2) {
		switch (getCurPattern()) {
		case PAT_PUTUP_UP2:
		case PAT_PUTUP_MID2:
		case PAT_PUTUP_DOWN2:
			setPattern(PAT_STANDUP);
			break;

		case PAT_PUTLEFT_UP2:
		case PAT_PUTLEFT_MID2:
		case PAT_PUTLEFT_DOWN2:
			setPattern(PAT_STANDLEFT);
			break;

		case PAT_PUTRIGHT_UP2:
		case PAT_PUTRIGHT_MID2:
		case PAT_PUTRIGHT_DOWN2:
			setPattern(PAT_STANDRIGHT);
			break;
		}
	}
}


bool RMTony::startTalkCalculate(TALKTYPE nTalkType, int &headStartPat, int &bodyStartPat,
                                int &headLoopPat, int &bodyLoopPat) {
	assert(!_bIsTalking);

	_bIsTalking = true;
	_nPatB4Talking = getCurPattern();
	_nTalkType = nTalkType;

	// Set the direction of speech ONLY if we are not in a static animation (since it would have already been done)
	if (!_bIsStaticTalk) {
		switch (_nPatB4Talking) {
		case PAT_STANDDOWN:
			_TalkDirection = DOWN;
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
		case PAT_SIRIALZALEFT:
		case PAT_STANDLEFT:
			_TalkDirection = LEFT;
			break;

		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
		case PAT_SIRIALZARIGHT:
		case PAT_STANDRIGHT:
			_TalkDirection = RIGHT;
			break;

		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
		case PAT_STANDUP:
			_TalkDirection = UP;
			break;
		}

		// Puts the body in front by default
		_bCorpoDavanti = true;
	}

	if (_bPastorella) {
		// Talking whilst a shepherdess
		mainFreeze();
		switch (_TalkDirection) {
		case UP:
			setPattern(PAT_PAST_TALKUP);
			break;

		case DOWN:
			setPattern(PAT_PAST_TALKDOWN);
			break;

		case LEFT:
			setPattern(PAT_PAST_TALKLEFT);
			break;

		case RIGHT:
			setPattern(PAT_PAST_TALKRIGHT);
			break;
		}
		mainUnfreeze();
		return false;
	}

	headStartPat = bodyStartPat = 0;
	bodyLoopPat = 0;

	switch (nTalkType) {
	case TALK_NORMAL:
		_bCorpoDavanti = false;
		headStartPat = 0;
		bodyStartPat = 0;

		switch (_TalkDirection) {
		case DOWN:
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);
			break;

		case LEFT:
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);
			break;

		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);
			break;

		case UP:
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_STANDUP;
			_nBodyOffset.set(6, 53);
			break;
		}
		break;

	case TALK_FIANCHI:
		_bCorpoDavanti = false;
		switch (_TalkDirection) {
		case UP:
			_nBodyOffset.set(2, 42);
			headStartPat = PAT_TESTA_UP;
			bodyStartPat = BPAT_FIANCHIUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_FIANCHIUP_LOOP;
			break;

		case DOWN:
			_nBodyOffset.set(2, 48);
			headStartPat = PAT_TESTA_DOWN;
			bodyStartPat = BPAT_FIANCHIDOWN_START;
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_FIANCHIDOWN_LOOP;
			break;

		case LEFT:
			_nBodyOffset.set(-3, 53);
			headStartPat = PAT_TESTA_LEFT;
			bodyStartPat = BPAT_FIANCHILEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_FIANCHILEFT_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(2, 53);
			headStartPat = PAT_TESTA_RIGHT;
			bodyStartPat = BPAT_FIANCHIRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_FIANCHIRIGHT_LOOP;
			break;
		}
		break;


	case TALK_CANTA:
		_nBodyOffset.set(-10, 25);
		headStartPat = PAT_TESTA_LEFT;
		bodyStartPat = BPAT_CANTALEFT_START;
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_CANTALEFT_LOOP;
		break;

	case TALK_RIDE:
		_bCorpoDavanti = false;
		switch (_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_RIDELEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_RIDELEFT_LOOP;
			bodyLoopPat = BPAT_RIDELEFT;
			break;

		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_RIDERIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_RIDERIGHT_LOOP;
			bodyLoopPat = BPAT_RIDERIGHT;
			break;
		}
		break;

	case TALK_RIDE2:
		_bCorpoDavanti = false;
		switch (_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_RIDELEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_RIDELEFT_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_RIDERIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_RIDERIGHT_LOOP;
			bodyLoopPat = BPAT_RIDERIGHT;
			break;
		}
		break;


	case TALK_SIINDICA:
		switch (_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(-4, 40);
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_SIINDICALEFT;
			break;

		case RIGHT:
			_nBodyOffset.set(5, 40);
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_SIINDICARIGHT;
			break;
		}
		break;

	case TALK_SPAVENTATO:
		switch (_TalkDirection) {
		case UP:
			_nBodyOffset.set(-4, -11);
			headStartPat = PAT_TESTA_UP;
			bodyStartPat = BPAT_SPAVENTOUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_SPAVENTOUP_LOOP;
			break;

		case DOWN:
			_nBodyOffset.set(-5, 45);
			headStartPat = PAT_SPAVENTODOWN_START;
			bodyStartPat = BPAT_SPAVENTODOWN_START;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			bodyLoopPat = BPAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(-4, 41);
			headStartPat = PAT_SPAVENTORIGHT_START;
			bodyStartPat = BPAT_SPAVENTORIGHT_START;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			bodyLoopPat = BPAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			_nBodyOffset.set(-10, 41);
			headStartPat = PAT_SPAVENTOLEFT_START;
			bodyStartPat = BPAT_SPAVENTOLEFT_START;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			bodyLoopPat = BPAT_SPAVENTOLEFT_LOOP;
			break;
		}
		break;

	case TALK_SPAVENTATO2:
		_bCorpoDavanti = false;
		switch (_TalkDirection) {
		case UP:
			bodyStartPat = BPAT_STANDUP;
			bodyLoopPat = BPAT_STANDUP;
			_nBodyOffset.set(6, 53);

			headStartPat = PAT_TESTA_UP;
			headLoopPat = PAT_TALK_UP;
			break;

		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);

			headStartPat = PAT_SPAVENTODOWN_START;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SPAVENTORIGHT_START;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SPAVENTOLEFT_START;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			break;
		}
		break;

	case TALK_CONBICCHIERE:
		_nBodyOffset.set(4, 53);
		headLoopPat = PAT_TALK_DOWN;
		bodyLoopPat = BPAT_BICCHIERE;
		break;
	case TALK_CONVERME:
		_nBodyOffset.set(9, 56);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_VERME;
		break;
	case TALK_CONMARTELLO:
		_nBodyOffset.set(6, 56);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MARTELLO;
		break;
	case TALK_CONCORDA:
		_nBodyOffset.set(-3, 38);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_CORDA;
		break;
	case TALK_CONSEGRETARIA:
		_nBodyOffset.set(-17, 12);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_CONSEGRETARIA;
		break;

	case TALK_CONCONIGLIO:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-21, -5);
			bodyStartPat = BPAT_CONCONIGLIOLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONCONIGLIOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -5);
			bodyStartPat = BPAT_CONCONIGLIORIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONCONIGLIORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONRICETTA:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-61, -7);
			bodyStartPat = BPAT_CONRICETTALEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONRICETTALEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-5, -7);
			bodyStartPat = BPAT_CONRICETTARIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONRICETTARIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONCARTE:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-34, -2);
			bodyStartPat = BPAT_CONCARTELEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONCARTELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -2);
			bodyStartPat = BPAT_CONCARTERIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONCARTERIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZO:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-35, 2);
			bodyStartPat = BPAT_CONPUPAZZOLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONPUPAZZOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, 2);
			bodyStartPat = BPAT_CONPUPAZZORIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONPUPAZZORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZOSTATIC:
	case TALK_CONRICETTASTATIC:
	case TALK_CONCONIGLIOSTATIC:
	case TALK_CONCARTESTATIC:
	case TALK_CONTACCUINOSTATIC:
	case TALK_CONMEGAFONOSTATIC:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALK_LEFT;
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			break;
		}
		break;

		// The beard is the only case in which the head is animated separately while the body is the standard
	case TALK_CONBARBASTATIC:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALKBARBA_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALKBARBA_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);
			break;
		}
		break;

	case TALK_SCHIFATO:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_SCHIFATOLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SCHIFATOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_SCHIFATORIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SCHIFATORIGHT_LOOP;
			break;
		}
		break;

	case TALK_NAAH:
		switch (_TalkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_NAAHLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_NAAHLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_NAAHRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_NAAHRIGHT_LOOP;
			break;
		}
		break;

	case TALK_MACBETH1:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH1;
		break;
	case TALK_MACBETH2:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH2;
		break;
	case TALK_MACBETH3:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH3;
		break;
	case TALK_MACBETH4:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH4;
		break;
	case TALK_MACBETH5:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH5;
		break;
	case TALK_MACBETH6:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH6;
		break;
	case TALK_MACBETH7:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH7;
		break;
	case TALK_MACBETH8:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH8;
		break;
	case TALK_MACBETH9:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH9;
		break;

	case TALK_SPAVENTATOSTATIC:
		_bCorpoDavanti = false;
		switch (_TalkDirection) {
		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);

			headStartPat = PAT_SPAVENTODOWN_STAND;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SPAVENTORIGHT_STAND;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SPAVENTOLEFT_STAND;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			break;

		default:
			break;
		}
		break;
	}

	return true;
}

void RMTony::startTalk(CORO_PARAM, TALKTYPE nTalkType) {
	CORO_BEGIN_CONTEXT;
	int headStartPat, bodyStartPat;
	int headLoopPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStartPat = _ctx->bodyStartPat = 0;
	_ctx->headLoopPat = _ctx->bodyLoopPat = 0;

	if (!startTalkCalculate(nTalkType, _ctx->headStartPat, _ctx->bodyStartPat,
	                        _ctx->headLoopPat, _ctx->bodyLoopPat))
		return;

	// Perform the set pattern
	if (_ctx->headStartPat != 0 || _ctx->bodyStartPat != 0) {
		mainFreeze();
		setPattern(_ctx->headStartPat);
		_body.setPattern(_ctx->bodyStartPat);
		mainUnfreeze();

		if (_ctx->bodyStartPat != 0)
			CORO_INVOKE_0(_body.waitForEndPattern);
		if (_ctx->headStartPat != 0)
			CORO_INVOKE_0(waitForEndPattern);
	}

	mainFreeze();
	setPattern(_ctx->headLoopPat);
	if (_ctx->bodyLoopPat)
		_body.setPattern(_ctx->bodyLoopPat);
	mainUnfreeze();

	CORO_END_CODE;
}


bool RMTony::endTalkCalculate(int &headStandPat, int &headEndPat, int &bodyEndPat, int &finalPat, bool &bStatic) {
	bodyEndPat = 0;
	headEndPat = 0;

	switch (_TalkDirection) {
	case UP:
		finalPat = PAT_STANDUP;
		headStandPat = PAT_TESTA_UP;
		break;

	case DOWN:
		finalPat = PAT_STANDDOWN;
		headStandPat = PAT_TESTA_DOWN;
		break;

	case LEFT:
		finalPat = PAT_STANDLEFT;
		headStandPat = PAT_TESTA_LEFT;
		break;

	case RIGHT:
		finalPat = PAT_STANDRIGHT;
		headStandPat = PAT_TESTA_RIGHT;
		break;
	}

	if (_bPastorella) {
		mainFreeze();
		setPattern(finalPat);
		mainUnfreeze();
		_bIsTalking = false;
		return false;
	}


	bStatic = false;
	switch (_nTalkType) {
	case TALK_NORMAL:
		bodyEndPat = 0;
		break;

	case TALK_FIANCHI:
		switch (_TalkDirection) {
		case UP:
			bodyEndPat = BPAT_FIANCHIUP_END;
			break;

		case DOWN:
			bodyEndPat = BPAT_FIANCHIDOWN_END;
			break;

		case LEFT:
			bodyEndPat = BPAT_FIANCHILEFT_END;
			break;

		case RIGHT:
			bodyEndPat = BPAT_FIANCHIRIGHT_END;
			break;
		}
		break;

	case TALK_CANTA:
		bodyEndPat = BPAT_CANTALEFT_END;
		break;

	case TALK_RIDE:
	case TALK_RIDE2:
		if (_TalkDirection == LEFT)
			headEndPat = PAT_RIDELEFT_END;
		else if (_TalkDirection == RIGHT)
			headEndPat = PAT_RIDERIGHT_END;

		bodyEndPat = 0;
		break;

	case TALK_SCHIFATO:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			headEndPat = PAT_SCHIFATOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_SCHIFATORIGHT_END;
			break;
		}

		bodyEndPat = 0;
		break;

	case TALK_NAAH:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			headEndPat = PAT_NAAHLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_NAAHRIGHT_END;
			break;
		}

		bodyEndPat = 0;
		break;

	case TALK_SIINDICA:
		break;

	case TALK_SPAVENTATO:
		switch (_TalkDirection) {
		case UP:
			bodyEndPat = BPAT_SPAVENTOUP_END;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			bodyEndPat = BPAT_SPAVENTODOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			bodyEndPat = BPAT_SPAVENTORIGHT_END;
			break;

		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			bodyEndPat = BPAT_SPAVENTOLEFT_END;
			break;
		}
		break;

	case TALK_SPAVENTATO2:
		switch (_TalkDirection) {
		case UP:
			bodyEndPat = 0;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			bodyEndPat = 0;
			break;

		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			bodyEndPat = 0;
			break;
		}
		break;

	case TALK_CONCONIGLIO:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONCONIGLIOLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONCONIGLIORIGHT_END;
			break;
		}
		break;

	case TALK_CONRICETTA:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONRICETTALEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONRICETTARIGHT_END;
			break;
		}
		break;

	case TALK_CONCARTE:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONCARTELEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONCARTERIGHT_END;
			break;
		}
		break;

	case TALK_CONPUPAZZO:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONPUPAZZOLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONPUPAZZORIGHT_END;
			break;
		}
		break;

	case TALK_CONVERME:
		finalPat = PAT_CONVERME;
		break;
	case TALK_CONCORDA:
		finalPat = PAT_CONCORDA;
		break;
	case TALK_CONSEGRETARIA:
		finalPat = PAT_CONSEGRETARIA;
		break;
	case TALK_CONMARTELLO:
		finalPat = PAT_CONMARTELLO;
		break;
	case TALK_CONBICCHIERE:
		finalPat = PAT_CONBICCHIERE;
		break;

	case TALK_MACBETH1:
	case TALK_MACBETH2:
	case TALK_MACBETH3:
	case TALK_MACBETH4:
	case TALK_MACBETH5:
	case TALK_MACBETH6:
	case TALK_MACBETH7:
	case TALK_MACBETH8:
		finalPat = 0;
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (_TalkDirection) {
		case DOWN:
			headStandPat = PAT_SPAVENTODOWN_STAND;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headStandPat = PAT_SPAVENTORIGHT_STAND;
			bodyEndPat = 0;
			break;

		case LEFT:
			headStandPat = PAT_SPAVENTOLEFT_STAND;
			bodyEndPat = 0;
			break;


		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void RMTony::endTalk(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int headStandPat, headEndPat;
	int bodyEndPat, finalPat;
	bool bStatic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStandPat = _ctx->headEndPat = 0;
	_ctx->bodyEndPat = _ctx->finalPat = 0;
	_ctx->bStatic = false;

	_ctx->bodyEndPat = 0;
	_ctx->headEndPat = 0;

	if (!endTalkCalculate(_ctx->headStandPat, _ctx->headEndPat, _ctx->bodyEndPat, _ctx->finalPat, _ctx->bStatic))
		return;

	// Handles the end of an animated and static, leaving everything unchanged
	if (_bIsStaticTalk) {
		if (_nTalkType == TALK_CONBARBASTATIC) {
			mainFreeze();
			setPattern(0);
			if (_TalkDirection == UP || _TalkDirection == LEFT) {
				_body.setPattern(BPAT_CONBARBALEFT_STATIC);
				_nBodyOffset.set(-41, -14);
			} else if (_TalkDirection == DOWN || _TalkDirection == RIGHT) {
				_body.setPattern(BPAT_CONBARBARIGHT_STATIC);
				_nBodyOffset.set(-26, -14);
			}
			mainUnfreeze();
		} else {
			mainFreeze();
			setPattern(_ctx->headStandPat);
			mainUnfreeze();

			CORO_INVOKE_0(_body.waitForEndPattern);
		}

		_bIsTalking = false;
		return;
	}

	// Set the pattern
	if (_ctx->headEndPat != 0 && _ctx->bodyEndPat != 0) {
		mainFreeze();
		setPattern(_ctx->headEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(_body.waitForEndPattern);

		mainFreeze();
		_body.setPattern(_ctx->bodyEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(waitForEndPattern);
		CORO_INVOKE_0(_body.waitForEndPattern);
	} else if (_ctx->bodyEndPat != 0) {
		mainFreeze();
		setPattern(_ctx->headStandPat);
		mainUnfreeze();

		CORO_INVOKE_0(_body.waitForEndPattern);

		mainFreeze();
		_body.setPattern(_ctx->bodyEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(_body.waitForEndPattern);
	} else if (_ctx->headEndPat != 0) {
		CORO_INVOKE_0(_body.waitForEndPattern);

		mainFreeze();
		setPattern(_ctx->headEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(waitForEndPattern);
	} else {
		CORO_INVOKE_0(_body.waitForEndPattern);
	}

	if (_ctx->finalPat != 0) {
		mainFreeze();
		_body.setPattern(0);
		setPattern(_ctx->finalPat);
		mainUnfreeze();
	}

	_bIsTalking = false;

	CORO_END_CODE;
}

void RMTony::startStaticCalculate(TALKTYPE nTalk, int &headPat, int &headLoopPat,
                                  int &bodyStartPat, int &bodyLoopPat) {
	int nPat = getCurPattern();

	headLoopPat = -1;

	switch (nPat) {
	case PAT_STANDDOWN:
		_TalkDirection = DOWN;
		headPat = PAT_TESTA_RIGHT;
		break;

	case PAT_TAKELEFT_UP2:
	case PAT_TAKELEFT_MID2:
	case PAT_TAKELEFT_DOWN2:
	case PAT_SIRIALZALEFT:
	case PAT_STANDLEFT:
		_TalkDirection = LEFT;
		headPat = PAT_TESTA_LEFT;
		break;

	case PAT_TAKERIGHT_UP2:
	case PAT_TAKERIGHT_MID2:
	case PAT_TAKERIGHT_DOWN2:
	case PAT_SIRIALZARIGHT:
	case PAT_STANDRIGHT:
		_TalkDirection = RIGHT;
		headPat = PAT_TESTA_RIGHT;
		break;

	case PAT_TAKEUP_UP2:
	case PAT_TAKEUP_MID2:
	case PAT_TAKEUP_DOWN2:
	case PAT_STANDUP:
		_TalkDirection = UP;
		headPat = PAT_TESTA_LEFT;
		break;
	}

	_bCorpoDavanti = true;

	switch (nTalk) {
	case TALK_CONCONIGLIOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-21, -5);
			bodyStartPat = BPAT_CONCONIGLIOLEFT_START;
			bodyLoopPat = BPAT_CONCONIGLIOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -5);
			bodyStartPat = BPAT_CONCONIGLIORIGHT_START;
			bodyLoopPat = BPAT_CONCONIGLIORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONCARTESTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-34, -2);
			bodyStartPat = BPAT_CONCARTELEFT_START;
			bodyLoopPat = BPAT_CONCARTELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -2);
			bodyStartPat = BPAT_CONCARTERIGHT_START;
			bodyLoopPat = BPAT_CONCARTERIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONRICETTASTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-61, -7);
			bodyStartPat = BPAT_CONRICETTALEFT_START;
			bodyLoopPat = BPAT_CONRICETTALEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-5, -7);
			bodyStartPat = BPAT_CONRICETTARIGHT_START;
			bodyLoopPat = BPAT_CONRICETTARIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-35, 2);
			bodyStartPat = BPAT_CONPUPAZZOLEFT_START;
			bodyLoopPat = BPAT_CONPUPAZZOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, 2);
			bodyStartPat = BPAT_CONPUPAZZORIGHT_START;
			bodyLoopPat = BPAT_CONPUPAZZORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONTACCUINOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-16, -9);
			bodyStartPat = BPAT_CONTACCUINOLEFT_START;
			bodyLoopPat = BPAT_CONTACCUINOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-6, -9);
			bodyStartPat = BPAT_CONTACCUINORIGHT_START;
			bodyLoopPat = BPAT_CONTACCUINORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONMEGAFONOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-41, -8);
			bodyStartPat = BPAT_CONMEGAFONOLEFT_START;
			bodyLoopPat = BPAT_CONMEGAFONOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, -8);
			bodyStartPat = BPAT_CONMEGAFONORIGHT_START;
			bodyLoopPat = BPAT_CONMEGAFONORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONBARBASTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-41, -14);
			bodyStartPat = BPAT_CONBARBALEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			headLoopPat = PAT_TALKBARBA_LEFT;
			headPat = 0;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-26, -14);
			bodyStartPat = BPAT_CONBARBARIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_TALKBARBA_RIGHT;
			headPat = 0;
			break;
		}
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (_TalkDirection) {
		case DOWN:
			headPat = PAT_SPAVENTODOWN_START;
			bodyLoopPat = BPAT_STANDDOWN;
			bodyStartPat = BPAT_STANDDOWN;
			headLoopPat = PAT_SPAVENTODOWN_STAND;
			_nBodyOffset.set(4, 53);
			break;

		case LEFT:
			headPat = PAT_SPAVENTOLEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SPAVENTOLEFT_STAND;
			_nBodyOffset.set(6, 56);
			break;

		case RIGHT:
			headPat = PAT_SPAVENTORIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SPAVENTORIGHT_STAND;
			_nBodyOffset.set(6, 56);
			break;

		default:
			break;
		}

	default:
		break;
	}
}

void RMTony::startStatic(CORO_PARAM, TALKTYPE nTalk) {
	CORO_BEGIN_CONTEXT;
	int headPat, headLoopPat;
	int bodyStartPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headPat = _ctx->headLoopPat = 0;
	_ctx->bodyStartPat = _ctx->bodyLoopPat = 0;

	// e vai con i pattern
	_bIsStaticTalk = true;

	mainFreeze();
	setPattern(_ctx->headPat);
	_body.setPattern(_ctx->bodyStartPat);
	mainUnfreeze();

	CORO_INVOKE_0(_body.waitForEndPattern);
	CORO_INVOKE_0(waitForEndPattern);

	mainFreeze();
	if (_ctx->headLoopPat != -1)
		setPattern(_ctx->headLoopPat);
	_body.setPattern(_ctx->bodyLoopPat);
	mainUnfreeze();

	CORO_END_CODE;
}


void RMTony::endStaticCalculate(TALKTYPE nTalk, int &bodyEndPat, int &finalPat, int &headEndPat) {
	switch (_TalkDirection) {
	case UP:
	case LEFT:
		finalPat = PAT_STANDLEFT;
		break;

	case RIGHT:
	case DOWN:
		finalPat = PAT_STANDRIGHT;
		break;
	}

	switch (nTalk) {
	case TALK_CONPUPAZZOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONPUPAZZOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONPUPAZZORIGHT_END;
			break;
		}
		break;

	case TALK_CONRICETTASTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONRICETTALEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONRICETTARIGHT_END;
			break;
		}
		break;

	case TALK_CONCONIGLIOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONCONIGLIOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONCONIGLIORIGHT_END;
			break;
		}
		break;

	case TALK_CONCARTESTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONCARTELEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONCARTERIGHT_END;
			break;
		}
		break;

	case TALK_CONTACCUINOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONTACCUINOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONTACCUINORIGHT_END;
			break;
		}
		break;

	case TALK_CONMEGAFONOSTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONMEGAFONOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONMEGAFONORIGHT_END;
			break;
		}
		break;

	case TALK_CONBARBASTATIC:
		switch (_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONBARBALEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONBARBARIGHT_END;
			break;
		}
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (_TalkDirection) {
		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void RMTony::endStatic(CORO_PARAM, TALKTYPE nTalk) {
	CORO_BEGIN_CONTEXT;
	int bodyEndPat;
	int finalPat;
	int headEndPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bodyEndPat = 0;
	_ctx->finalPat = 0;
	_ctx->headEndPat = 0;

	endStaticCalculate(nTalk, _ctx->bodyEndPat, _ctx->finalPat, _ctx->headEndPat);

	if (_ctx->headEndPat != 0) {
		mainFreeze();
		setPattern(_ctx->headEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(waitForEndPattern);
	} else {
		// Play please
		mainFreeze();
		_body.setPattern(_ctx->bodyEndPat);
		mainUnfreeze();

		CORO_INVOKE_0(_body.waitForEndPattern);
	}

	mainFreeze();
	setPattern(_ctx->finalPat);
	_body.setPattern(0);
	mainUnfreeze();

	_bIsStaticTalk = false;

	CORO_END_CODE;
}

} // End of namespace Tony
