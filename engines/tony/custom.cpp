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
 *
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/system.h"
#include "common/savefile.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/memory.h"
#include "tony/custom.h"
#include "tony/font.h"
#include "tony/game.h"
#include "tony/gfxcore.h"
#include "tony/tony.h"
#include "tony/tonychar.h"
#include "tony/utils.h"

namespace Tony {

const char *ambianceFile[] =  {
	"None",
	"1.ADP",    // Grilli.WAV
	"2.ADP",    // Grilli-Ovattati.WAV
	"3.ADP",    // Grilli-Vento.WAV
	"3.ADP",    // Grilli-Vento1.WAV
	"5.ADP",    // Vento1.WAV
	"4.ADP",    // Mare1.WAV
	"6.ADP"     // Mare1.WAV half volume
};

struct MusicFileEntry {
	const char *name;
	int sync;
};

const MusicFileEntry musicFiles[] =  {
	{"00.ADP", 0},    {"01.ADP", 0},
	{"02.ADP", 0},    {"03.ADP", 0},
	{"04.ADP", 0},    {"05.ADP", 0},
	{"06.ADP", 0},    {"07.ADP", 0},
	{"08.ADP", 2450}, {"09.ADP", 0},
	{"10.ADP", 0},    {"11.ADP", 0},
	{"12.ADP", 0},    {"13.ADP", 0},
	{"14.ADP", 0},    {"15.ADP", 0},
	{"16.ADP", 0},    {"17.ADP", 0},
	{"18.ADP", 0},    {"19.ADP", 0},
	{"20.ADP", 0},    {"21.ADP", 0},
	{"22.ADP", 0},    {"23.ADP", 0},
	{"24.ADP", 0},    {"25.ADP", 0},
	{"26.ADP", 0},    {"27.ADP", 0},
	{"28.ADP", 1670}, {"29.ADP", 0},
	{"30.ADP", 0},    {"31.ADP", 0},
	{"32.ADP", 2900}, {"33.ADP", 0},
	{"34.ADP", 0},    {"35.ADP", 0},
	{"36.ADP", 0},    {"37.ADP", 0},
	{"38.ADP", 0},    {"39.ADP", 0},
	{"40.ADP", 0},    {"41.ADP", 1920},
	{"42.ADP", 1560}, {"43.ADP", 1920},
	{"44.ADP", 1920}, {"45.ADP", 1920},
	{"46.ADP", 1920}, {"47.ADP", 1920},
	{"48.ADP", 1920}, {"49.ADP", 1920},
	{"50.ADP", 1920}, {"51.ADP", 1920},
	{"52.ADP", 1920}, {"53.ADP", 0},
	{"54.ADP", 0},    {"55.ADP", 0},
	{"56.ADP", 0},    {"57.ADP", 0},
	{"58.ADP", 0},    {"59.ADP", 0}
};


const char *jingleFileNames[] = {
	"S00.ADP", "S01.ADP",
	"S02.ADP", "S03.ADP",
	"S04.ADP", "S05.ADP",
	"S06.ADP", "S07.ADP",
	"S08.ADP", "S09.ADP",
	"S10.ADP", "S11.ADP",
	"S12.ADP", "S13.ADP",
	"S14.ADP", "S15.ADP",
	"S16.ADP", "S17.ADP",
	"S18.ADP"
};


void ReapplyChangedHotspot() {
	int i;
	for (i = 0; i < GLOBALS._curChangedHotspot; i++)
		GLOBALS._loc->getItemFromCode(GLOBALS._changedHotspot[i]._dwCode)->changeHotspot(RMPoint(GLOBALS._changedHotspot[i]._nX, GLOBALS._changedHotspot[i]._nY));
}

void SaveChangedHotspot(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS._curChangedHotspot);
	if (GLOBALS._curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS._curChangedHotspot; ++i)
			GLOBALS._changedHotspot[i].save(f);
	}
}

void LoadChangedHotspot(Common::InSaveFile *f) {
	GLOBALS._curChangedHotspot = f->readByte();

	if (GLOBALS._curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS._curChangedHotspot; ++i)
			GLOBALS._changedHotspot[i].load(f);
	}
}


/**
 * Classes required for custom functions
 *
 * Tony (To Move him)     -> You can do MPAL through the animation? I really think so
 *
 * SendMessage -> I'd say just theEngine.SendMessage()
 * ChangeLocation -> theEngine.ChangeLocation()
 * AddInventory -> theEngine.AddInventory()
*/

void MCharResetCodes() {
	for (int i = 0; i < 10; i++)
		GLOBALS._mCharacter[i]._item = GLOBALS._loc->getItemFromCode(GLOBALS._mCharacter[i]._code);
	for (int i = 0; i < 10; i++)
		GLOBALS._character[i]._item = GLOBALS._loc->getItemFromCode(GLOBALS._character[i]._code);
}

void CharsSaveAll(Common::OutSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		f->writeByte(GLOBALS._isMChar[i]);
		if (GLOBALS._isMChar[i]) {
			GLOBALS._mCharacter[i].save(f);
		} else {
			GLOBALS._character[i].save(f);
		}
	}
}

void CharsLoadAll(Common::InSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		GLOBALS._isMChar[i] = f->readByte();
		if (GLOBALS._isMChar[i])
			GLOBALS._mCharacter[i].load(f);
		else
			GLOBALS._character[i].load(f);
	}
}

DECLARE_CUSTOM_FUNCTION(FaceToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDDOWN);
}

DECLARE_CUSTOM_FUNCTION(BackToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDUP);
}

DECLARE_CUSTOM_FUNCTION(LeftToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDLEFT);
}

DECLARE_CUSTOM_FUNCTION(RightToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDRIGHT);
}


DECLARE_CUSTOM_FUNCTION(TonySetPerorate)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	g_vm->getEngine()->setPerorate(bStatus);
}

DECLARE_CUSTOM_FUNCTION(MySleep)(CORO_PARAM, uint32 dwTime, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_1(CoroScheduler.sleep, dwTime);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetAlwaysDisplay)(CORO_PARAM, uint32 val, uint32, uint32, uint32) {
	GLOBALS._bAlwaysDisplay = (val != 0);
}


DECLARE_CUSTOM_FUNCTION(SetPointer)(CORO_PARAM, uint32 dwPointer, uint32, uint32, uint32) {
	switch (dwPointer) {
	case 1:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWUP);
		break;
	case 2:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWDOWN);
		break;
	case 3:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWLEFT);
		break;
	case 4:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWRIGHT);
		break;
	case 5:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWMAP);
		break;

	default:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_NONE);
		break;
	}
}

VoiceHeader *SearchVoiceHeader(uint32 codehi, uint32 codelo) {
	int code = (codehi << 16) | codelo;

	if (g_vm->_voices.size() == 0)
		return NULL;

	for (uint i = 0; i < g_vm->_voices.size(); i++) {
		if (g_vm->_voices[i]._code == code)
			return &g_vm->_voices[i];
	}

	return NULL;
}


DECLARE_CUSTOM_FUNCTION(SendTonyMessage)(CORO_PARAM, uint32 dwMessage, uint32 nX, uint32 nY, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage msg;
	int i;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curOffset = 0;

	if (GLOBALS._bSkipIdle)
		return;

	_ctx->msg.load(dwMessage);
	if (!_ctx->msg.isValid())
		return;

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Is positioned within the database of entries beginning at the first
		_ctx->curOffset = _ctx->curVoc->_offset;

		// First time allocation
		g_vm->_vdbFP.seek(_ctx->curOffset);
		g_vm->_theSound.createSfx(&_ctx->voice);

		_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
		_ctx->curOffset = g_vm->_vdbFP.pos();

		_ctx->voice->setLoop(false);
	}

	if (GLOBALS._nTonyNextTalkType != GLOBALS._tony->TALK_NORMAL) {
		CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);

		if (!GLOBALS._bStaticTalk)
			GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
	} else {
		if (_ctx->msg.numPeriods() > 1)
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_HIPS);
		else
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_NORMAL);
	}

	if (GLOBALS._curBackText)
		CORO_INVOKE_0(GLOBALS._curBackText->hide);

	GLOBALS._bTonyIsSpeaking = true;

	for (_ctx->i = 0; _ctx->i < _ctx->msg.numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		_ctx->text.setInput(GLOBALS._input);

		// Alignment
		_ctx->text.setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text.setColor(0, 255, 0);

		// Writes the text
		_ctx->text.writeText(_ctx->msg[_ctx->i], 0);

		// Set the position
		if (nX == 0 && nY == 0)
			_ctx->text.setPosition(GLOBALS._tony->position() - RMPoint(0, 130) - GLOBALS._loc->scrollPosition());
		else
			_ctx->text.setPosition(RMPoint(nX, nY) - GLOBALS._loc->scrollPosition());

		// Handling for always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text.setAlwaysDisplay();
			_ctx->text.forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->text);

		if (_ctx->curVoc) {
			if (_ctx->i == 0) {
				_ctx->voice->play();
				_ctx->text.setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			} else {
				g_vm->_vdbFP.seek(_ctx->curOffset);
				g_vm->_theSound.createSfx(&_ctx->voice);
				_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);

				_ctx->curOffset = g_vm->_vdbFP.pos();
				_ctx->voice->setLoop(false);
				_ctx->voice->play();
				_ctx->text.setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			}
		}

		// Wait for the end of the display
		_ctx->text.setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text.waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}
	}

	GLOBALS._bTonyIsSpeaking = false;
	if (GLOBALS._curBackText)
		GLOBALS._curBackText->show();

	CORO_INVOKE_0(GLOBALS._tony->endTalk);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(ChangeBoxStatus)(CORO_PARAM, uint32 nLoc, uint32 nBox, uint32 nStatus, uint32) {
	GLOBALS._boxes->changeBoxStatus(nLoc, nBox, nStatus);
}


DECLARE_CUSTOM_FUNCTION(CustLoadLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._curChangedHotspot = 0;
	if (bUseStartPos != 0)
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), GLOBALS._startLocPos[nLoc]);
	else
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	// On Enter?
	if (_ctx->h != CORO_INVALID_PID_VALUE)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(SendFullscreenMsgStart)(CORO_PARAM, uint32 nMsg, uint32 nFont, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	RMGfxClearTask clear;
	int i;
	RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);

	GLOBALS._fullScreenMessageLoc = GLOBALS._loc->TEMPGetNumLoc();
	GLOBALS._fullScreenMessagePt = GLOBALS._tony->position();

	if (GLOBALS._bSkipIdle)
		return;

	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, false, NULL);
	GLOBALS._tony->hide();

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		_ctx->text.setInput(GLOBALS._input);

		// Alignment
		_ctx->text.setAlignType(RMText::HCENTER, RMText::VCENTER);

		// Forces the text to disappear in time
		_ctx->text.forceTime();

		// Color
		_ctx->text.setColor(255, 255, 255);

		// Write the text
		if (nFont == 0)
			_ctx->text.writeText((*_ctx->msg)[_ctx->i], 1);
		else if (nFont == 1)
			_ctx->text.writeText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text.setPosition(RMPoint(320, 240));

		_ctx->text.setAlwaysDisplay();
		_ctx->text.forceTime();

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->clear);
		g_vm->getEngine()->linkGraphicTask(&_ctx->text);

		// Wait for the end of display
		_ctx->text.setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text.waitForEndDisplay);
	}

	delete _ctx->msg;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(ClearScreen)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	char buf[256];
	RMGfxClearTask clear;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	g_vm->getEngine()->linkGraphicTask(&_ctx->clear);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	// WORKAROUND: This fixes a bug in the original source where the linked clear task
	// didn't have time to be drawn and removed from the draw list before the method
	// ended, thus remaining in the draw list and causing a later crash
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SendFullscreenMsgEnd)(CORO_PARAM, uint32 bNotEnableTony, uint32, uint32, uint32) {
	g_vm->getEngine()->loadLocation(GLOBALS._fullScreenMessageLoc, RMPoint(GLOBALS._fullScreenMessagePt._x, GLOBALS._fullScreenMessagePt._y), RMPoint(-1, -1));
	if (!bNotEnableTony)
		GLOBALS._tony->show();

	MCharResetCodes();
	ReapplyChangedHotspot();
}


DECLARE_CUSTOM_FUNCTION(SendFullscreenMessage)(CORO_PARAM, uint32 nMsg, uint32 nFont, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(SendFullscreenMsgStart, nMsg, nFont, 0, 0);
	CORO_INVOKE_4(SendFullscreenMsgEnd, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(NoBullsEye)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bNoBullsEye = true;
}

DECLARE_CUSTOM_FUNCTION(CloseLocation)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(1);
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
	}

	g_vm->stopMusic(4);

	// On exit, unload
	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, true, NULL);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(1);
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
	}

	if (GLOBALS._lastTappeto != GLOBALS._ambiance[nLoc]) {
		g_vm->stopMusic(4);
	}

	// On exit, unfreeze
	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, true, NULL);

	GLOBALS._curChangedHotspot = 0;
	if (bUseStartPos != 0)
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), GLOBALS._startLocPos[nLoc]);
	else
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	if (GLOBALS._lastTappeto != GLOBALS._ambiance[nLoc]) {
		GLOBALS._lastTappeto = GLOBALS._ambiance[nLoc];
		if (GLOBALS._lastTappeto != 0)
			g_vm->playMusic(4, ambianceFile[GLOBALS._lastTappeto], 0, true, 2000);
	}

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(2);
	}

	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	if (!GLOBALS._bNoBullsEye) {
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
		g_vm->getEngine()->closeWipe();
	}

	GLOBALS._bNoBullsEye = false;

	// On Enter?
	if (_ctx->h != CORO_INVALID_PID_VALUE)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetLocStartPosition)(CORO_PARAM, uint32 nLoc, uint32 lX, uint32 lY, uint32) {
	GLOBALS._startLocPos[nLoc].set(lX, lY);
}

DECLARE_CUSTOM_FUNCTION(SaveTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._saveTonyPos = GLOBALS._tony->position();
	GLOBALS._saveTonyLoc = GLOBALS._loc->TEMPGetNumLoc();
}

DECLARE_CUSTOM_FUNCTION(RestoreTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(ChangeLocation, GLOBALS._saveTonyLoc, GLOBALS._saveTonyPos._x, GLOBALS._saveTonyPos._y, 0);

	MCharResetCodes();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(DisableInput)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->getEngine()->disableInput();
}

DECLARE_CUSTOM_FUNCTION(EnableInput)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->getEngine()->enableInput();
}

DECLARE_CUSTOM_FUNCTION(StopTony)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->stopNoAction(coroParam);
}

DECLARE_CUSTOM_FUNCTION(CustEnableGUI)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.EnableGUI();
}

DECLARE_CUSTOM_FUNCTION(CustDisableGUI)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.DisableGUI();
}



void TonyGenericTake1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->take(nDirection, 0);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericTake2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->take(nDirection, 1);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->take(nDirection, 2);

	CORO_END_CODE;
}

void TonyGenericPut1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->put(nDirection, 0);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericPut2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->put(nDirection, 1);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->put(nDirection, 2);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(TonyTakeUp1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake1(coroParam, 0);
}


DECLARE_CUSTOM_FUNCTION(TonyTakeMid1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake1(coroParam, 1);
}

DECLARE_CUSTOM_FUNCTION(TonyTakeDown1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake1(coroParam, 2);
}



DECLARE_CUSTOM_FUNCTION(TonyTakeUp2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake2(coroParam, 0);
}


DECLARE_CUSTOM_FUNCTION(TonyTakeMid2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake2(coroParam, 1);
}

DECLARE_CUSTOM_FUNCTION(TonyTakeDown2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericTake2(coroParam, 2);
}



DECLARE_CUSTOM_FUNCTION(TonyPutUp1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut1(coroParam, 0);
}


DECLARE_CUSTOM_FUNCTION(TonyPutMid1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut1(coroParam, 1);
}

DECLARE_CUSTOM_FUNCTION(TonyPutDown1)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut1(coroParam, 2);
}

DECLARE_CUSTOM_FUNCTION(TonyPutUp2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut2(coroParam, 0);
}


DECLARE_CUSTOM_FUNCTION(TonyPutMid2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut2(coroParam, 1);
}

DECLARE_CUSTOM_FUNCTION(TonyPutDown2)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	TonyGenericPut2(coroParam, 2);
}


DECLARE_CUSTOM_FUNCTION(TonyOnTheFloor)(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	if (dwParte == 0)
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_ONTHEFLOORLEFT);
	else
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_ONTHEFLOORRIGHT);
}

DECLARE_CUSTOM_FUNCTION(TonyGetUp)(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (dwParte == 0)
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_GETUPLEFT);
	else
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_GETUPRIGHT);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyShepherdess)(CORO_PARAM, uint32 bIsPast, uint32, uint32, uint32) {
	GLOBALS._tony->setShepherdess(bIsPast);
}

DECLARE_CUSTOM_FUNCTION(TonyWhistle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WHISTLERIGHT);
	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDRIGHT);

	CORO_END_CODE;
}


void TonySetNumTexts(uint32 dwText) {
	GLOBALS._dwTonyNumTexts = dwText;
	GLOBALS._bTonyInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(TonyLaugh)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_LAUGH;
}

DECLARE_CUSTOM_FUNCTION(TonyGiggle)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_LAUGH2;
}

DECLARE_CUSTOM_FUNCTION(TonyHips)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_HIPS;
}

DECLARE_CUSTOM_FUNCTION(TonySing)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SING;
}

DECLARE_CUSTOM_FUNCTION(TonyIndicate)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_INDICATE;
}

DECLARE_CUSTOM_FUNCTION(TonyScaredWithHands)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCARED;
}

DECLARE_CUSTOM_FUNCTION(TonyScaredWithoutHands)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCARED2;
}

DECLARE_CUSTOM_FUNCTION(TonyWithHammer)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHHAMMER;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHHAMMER);
}

DECLARE_CUSTOM_FUNCTION(TonyWithGlasses)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHGLASSES;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHGLASSES);
}

DECLARE_CUSTOM_FUNCTION(TonyWithWorm)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHWORM;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHWORM);
}

DECLARE_CUSTOM_FUNCTION(TonyWithRope)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHROPE;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHROPE);
}

DECLARE_CUSTOM_FUNCTION(TonyWithSecretary)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSECRETARY;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHSECRETARY);
}

DECLARE_CUSTOM_FUNCTION(TonyWithRabbitANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRABBIT;
}

DECLARE_CUSTOM_FUNCTION(TonyWithRecipeANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRECIPE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithCardsANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHCARDS;
}

DECLARE_CUSTOM_FUNCTION(TonyWithSnowmanANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSNOWMAN;
}

DECLARE_CUSTOM_FUNCTION(TonyWithSnowmanStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSNOWMANSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHSNOWMANSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithSnowmanEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHSNOWMANSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithRabbitStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRABBITSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHRABBITSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithRabbitEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHRABBITSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithRecipeStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRECIPESTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHRECIPESTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithRecipeEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHRECIPESTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithCardsStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHCARDSSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHCARDSSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithCardsEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHCARDSSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithNotebookStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITH_NOTEBOOK;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITH_NOTEBOOK);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithNotebookEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITH_NOTEBOOK);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithMegaphoneStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHMEGAPHONESTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHMEGAPHONESTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithMegaphoneEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHMEGAPHONESTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithBeardStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHBEARDSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHBEARDSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyWithBeardEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHBEARDSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyScaredStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCAREDSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_SCAREDSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyScaredEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_SCAREDSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(TonyDisgusted)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_DISGUSTED;
}

DECLARE_CUSTOM_FUNCTION(TonySniffLeft)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_SNIFF_LEFT);
	CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);
	CORO_INVOKE_4(LeftToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySniffRight)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_SNIFF_RIGHT);
	CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);
	CORO_INVOKE_4(RightToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySarcastic)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SARCASTIC;
}

DECLARE_CUSTOM_FUNCTION(TonyMacbeth)(CORO_PARAM, uint32 nPos, uint32, uint32, uint32) {
	switch (nPos) {
	case 1:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH1;
		break;
	case 2:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH2;
		break;
	case 3:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH3;
		break;
	case 4:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH4;
		break;
	case 5:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH5;
		break;
	case 6:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH6;
		break;
	case 7:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH7;
		break;
	case 8:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH8;
		break;
	case 9:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH9;
		break;
	}
}


DECLARE_CUSTOM_FUNCTION(EnableTony)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->show();
}

DECLARE_CUSTOM_FUNCTION(DisableTony)(CORO_PARAM, uint32 bShowShadow, uint32, uint32, uint32) {
	GLOBALS._tony->hide(bShowShadow);
}

DECLARE_CUSTOM_FUNCTION(WaitForPatternEnd)(CORO_PARAM, uint32 nItem, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMItem *item;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->item = GLOBALS._loc->getItemFromCode(nItem);

	if (!GLOBALS._bSkipIdle && _ctx->item != NULL)
		CORO_INVOKE_1(_ctx->item->waitForEndPattern, GLOBALS._hSkipIdle);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(SetTonyPosition)(CORO_PARAM, uint32 nX, uint32 nY, uint32 nLoc, uint32) {
	GLOBALS._tony->setPosition(RMPoint(nX, nY), nLoc);
}

DECLARE_CUSTOM_FUNCTION(MoveTonyAndWait)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// WORKAROUND: Delay for a frame before starting the move to give any previous move time to finish.
	// This fixes a bug in the first scene where if you immediately 'Use Door', Tony moves to the door,
	// and then floats to the right rather than properly walking.
	CORO_SLEEP(1);

	CORO_INVOKE_1(GLOBALS._tony->move, RMPoint(nX, nY));

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndMovement);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(MoveTony)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	GLOBALS._tony->move(coroParam, RMPoint(nX, nY));
}

DECLARE_CUSTOM_FUNCTION(ScrollLocation)(CORO_PARAM, uint32 nX, uint32 nY, uint32 sX, uint32 sY) {
	CORO_BEGIN_CONTEXT;
	int lx, ly;
	RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Take the scroll coordinates
	_ctx->lx = (int32)nX;
	_ctx->ly = (int32)nY;

	_ctx->pt = GLOBALS._loc->scrollPosition();

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS._bSkipIdle) {
		if (_ctx->lx > 0) {
			_ctx->lx -= (int32)sX;
			if (_ctx->lx < 0)
				_ctx->lx = 0;
			_ctx->pt.offset((int32)sX, 0);
		} else if (_ctx->lx < 0) {
			_ctx->lx += (int32)sX;
			if (_ctx->lx > 0)
				_ctx->lx = 0;
			_ctx->pt.offset(-(int32)sX, 0);
		}

		if (_ctx->ly > 0) {
			_ctx->ly -= sY;
			if (_ctx->ly < 0)
				_ctx->ly = 0;
			_ctx->pt.offset(0, sY);
		} else if (_ctx->ly < 0) {
			_ctx->ly += sY;
			if (_ctx->ly > 0)
				_ctx->ly = 0;
			_ctx->pt.offset(0, -(int32)sY);
		}

		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setScrollPosition(_ctx->pt);
		GLOBALS._tony->setScrollPosition(_ctx->pt);
	}

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SyncScrollLocation)(CORO_PARAM, uint32 nX, uint32 nY, uint32 sX, uint32 sY) {
	CORO_BEGIN_CONTEXT;
	int lx, ly;
	RMPoint pt, startpt;
	uint32 dwStartTime, dwCurTime, dwTotalTime;
	uint32 stepX, stepY;
	int dimx, dimy;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Take the scroll coordinates
	_ctx->lx = (int32)nX;
	_ctx->ly = (int32)nY;
	_ctx->dimx = _ctx->lx;
	_ctx->dimy = _ctx->ly;
	if (_ctx->lx < 0)
		_ctx->dimx = -_ctx->lx;

	if (_ctx->ly < 0)
		_ctx->dimy = -_ctx->ly;

	_ctx->stepX = sX;
	_ctx->stepY = sY;

	_ctx->startpt = GLOBALS._loc->scrollPosition();

	_ctx->dwStartTime = g_vm->getTime();

	if (sX)
		_ctx->dwTotalTime = _ctx->dimx * (1000 / 35) / sX;
	else
		_ctx->dwTotalTime = _ctx->dimy * (1000 / 35) / sY;

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS._bSkipIdle) {
		_ctx->dwCurTime = g_vm->getTime() - _ctx->dwStartTime;
		if (_ctx->dwCurTime > _ctx->dwTotalTime)
			break;

		_ctx->pt = _ctx->startpt;

		if (sX) {
			if (_ctx->lx > 0)
				_ctx->pt._x += (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt._x -= (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
		} else {
			if (_ctx->ly > 0)
				_ctx->pt._y += (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt._y -= (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;

		}

		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setScrollPosition(_ctx->pt);
		GLOBALS._tony->setScrollPosition(_ctx->pt);

	}


	// Set the position finale
	if (sX) {
		if (_ctx->lx > 0)
			_ctx->pt._x = _ctx->startpt._x + _ctx->dimx;
		else
			_ctx->pt._x = _ctx->startpt._x - _ctx->dimx;
	} else {
		if (_ctx->ly > 0)
			_ctx->pt._y = _ctx->startpt._y + _ctx->dimy;
		else
			_ctx->pt._y = _ctx->startpt._y - _ctx->dimy;
	}

	GLOBALS._loc->setScrollPosition(_ctx->pt);
	GLOBALS._tony->setScrollPosition(_ctx->pt);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeHotspot)(CORO_PARAM, uint32 dwCode, uint32 nX, uint32 nY, uint32) {
	int i;

	for (i = 0; i < GLOBALS._curChangedHotspot; i++) {
		if (GLOBALS._changedHotspot[i]._dwCode == dwCode) {
			GLOBALS._changedHotspot[i]._nX = nX;
			GLOBALS._changedHotspot[i]._nY = nY;
			break;
		}
	}

	if (i == GLOBALS._curChangedHotspot) {
		GLOBALS._changedHotspot[i]._dwCode = dwCode;
		GLOBALS._changedHotspot[i]._nX = nX;
		GLOBALS._changedHotspot[i]._nY = nY;
		GLOBALS._curChangedHotspot++;
	}

	GLOBALS._loc->getItemFromCode(dwCode)->changeHotspot(RMPoint(nX, nY));
}


DECLARE_CUSTOM_FUNCTION(AutoSave)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->autoSave(coroParam);
}

DECLARE_CUSTOM_FUNCTION(AbortGame)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->abortGame();
}

DECLARE_CUSTOM_FUNCTION(ShakeScreen)(CORO_PARAM, uint32 nScosse, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	uint32 i;
	uint32 curTime;
	int dirx, diry;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = g_vm->getTime();

	_ctx->dirx = 1;
	_ctx->diry = 1;

	while (g_vm->getTime() < _ctx->curTime + nScosse) {
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		GLOBALS._tony->setFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));

		_ctx->i = g_vm->_randomSource.getRandomNumber(2);

		if (_ctx->i == 0 || _ctx->i == 2)
			_ctx->dirx = -_ctx->dirx;
		else if (_ctx->i == 1 || _ctx->i == 2)
			_ctx->diry = -_ctx->diry;
	}

	GLOBALS._loc->setFixedScroll(RMPoint(0, 0));
	GLOBALS._tony->setFixedScroll(RMPoint(0, 0));

	CORO_END_CODE;
}

/*
 *  Characters
 */

DECLARE_CUSTOM_FUNCTION(CharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._code = nCode;
	GLOBALS._character[nChar]._item = GLOBALS._loc->getItemFromCode(nCode);
	GLOBALS._character[nChar]._r = 255;
	GLOBALS._character[nChar]._g = 255;
	GLOBALS._character[nChar]._b = 255;
	GLOBALS._character[nChar]._talkPattern = 0;
	GLOBALS._character[nChar]._startTalkPattern = 0;
	GLOBALS._character[nChar]._endTalkPattern = 0;
	GLOBALS._character[nChar]._standPattern = 0;

	GLOBALS._isMChar[nChar] = false;
}

DECLARE_CUSTOM_FUNCTION(CharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._r = r;
	GLOBALS._character[nChar]._g = g;
	GLOBALS._character[nChar]._b = b;
}

DECLARE_CUSTOM_FUNCTION(CharSetTalkPattern)(CORO_PARAM, uint32 nChar, uint32 tp, uint32 sp, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._talkPattern = tp;
	GLOBALS._character[nChar]._standPattern = sp;
}

DECLARE_CUSTOM_FUNCTION(CharSetStartEndTalkPattern)(CORO_PARAM, uint32 nChar, uint32 sp, uint32 ep, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._startTalkPattern = sp;
	GLOBALS._character[nChar]._endTalkPattern = ep;
}

DECLARE_CUSTOM_FUNCTION(CharSendMessage)(CORO_PARAM, uint32 nChar, uint32 dwMessage, uint32 bIsBack, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	int i;
	RMPoint pt;
	RMTextDialog *text;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 16);
	_ctx->pt = GLOBALS._character[nChar]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();

	if (GLOBALS._character[nChar]._startTalkPattern != 0) {
		GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._startTalkPattern);

		CORO_INVOKE_0(GLOBALS._character[nChar]._item->waitForEndPattern);
	}

	GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._talkPattern);

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		if (bIsBack) {
			GLOBALS._curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setInput(GLOBALS._input);

		// Skipping
		_ctx->text->setSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text->setColor(GLOBALS._character[nChar]._r, GLOBALS._character[nChar]._g, GLOBALS._character[nChar]._b);

		// Write the text
		_ctx->text->writeText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text->setPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_vm->_theSound.createSfx(&_ctx->voice);
			g_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
			_ctx->voice->setLoop(false);
			if (bIsBack)
				_ctx->voice->setVolume(55);
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			_ctx->curOffset = g_vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}


		GLOBALS._curBackText = NULL;
		delete _ctx->text;
	}

	if (GLOBALS._character[nChar]._endTalkPattern != 0) {
		GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._endTalkPattern);
		CORO_INVOKE_0(GLOBALS._character[nChar]._item->waitForEndPattern);
	}

	GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._standPattern);
	delete _ctx->msg;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(AddInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS._inventory->addItem(dwCode);
}

DECLARE_CUSTOM_FUNCTION(RemoveInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS._inventory->removeItem(dwCode);
}

DECLARE_CUSTOM_FUNCTION(ChangeInventoryStatus)(CORO_PARAM, uint32 dwCode, uint32 dwStatus, uint32, uint32) {
	GLOBALS._inventory->changeItemStatus(dwCode, dwStatus);
}


/*
 * Master Characters
 */

DECLARE_CUSTOM_FUNCTION(MCharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._code = nCode;
	if (nCode == 0)
		GLOBALS._mCharacter[nChar]._item = NULL;
	else
		GLOBALS._mCharacter[nChar]._item = GLOBALS._loc->getItemFromCode(nCode);
	GLOBALS._mCharacter[nChar]._r = 255;
	GLOBALS._mCharacter[nChar]._g = 255;
	GLOBALS._mCharacter[nChar]._b = 255;
	GLOBALS._mCharacter[nChar]._x = -1;
	GLOBALS._mCharacter[nChar]._y = -1;
	GLOBALS._mCharacter[nChar]._bAlwaysBack = 0;

	for (int i = 0; i < 10; i++)
		GLOBALS._mCharacter[nChar]._numTalks[i] = 1;

	GLOBALS._mCharacter[nChar]._curGroup = 0;

	GLOBALS._isMChar[nChar] = true;
}

DECLARE_CUSTOM_FUNCTION(MCharResetCode)(CORO_PARAM, uint32 nChar, uint32, uint32, uint32) {
	GLOBALS._mCharacter[nChar]._item = GLOBALS._loc->getItemFromCode(GLOBALS._mCharacter[nChar]._code);
}


DECLARE_CUSTOM_FUNCTION(MCharSetPosition)(CORO_PARAM, uint32 nChar, uint32 nX, uint32 nY, uint32) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._x = nX;
	GLOBALS._mCharacter[nChar]._y = nY;
}


DECLARE_CUSTOM_FUNCTION(MCharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._r = r;
	GLOBALS._mCharacter[nChar]._g = g;
	GLOBALS._mCharacter[nChar]._b = b;
}


DECLARE_CUSTOM_FUNCTION(MCharSetNumTalksInGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32 nTalks, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS._mCharacter[nChar]._numTalks[nGroup] = nTalks;
}


DECLARE_CUSTOM_FUNCTION(MCharSetCurrentGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS._mCharacter[nChar]._curGroup = nGroup;
}

DECLARE_CUSTOM_FUNCTION(MCharSetNumTexts)(CORO_PARAM, uint32 nChar, uint32 nTexts, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS._mCharacter[nChar]._numTexts = nTexts - 1;
	GLOBALS._mCharacter[nChar]._bInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(MCharSetAlwaysBack)(CORO_PARAM, uint32 nChar, uint32 bAlwaysBack, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS._mCharacter[nChar]._bAlwaysBack = bAlwaysBack;
}


DECLARE_CUSTOM_FUNCTION(MCharSendMessage)(CORO_PARAM, uint32 nChar, uint32 dwMessage, uint32 bIsBack, uint32 nFont) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	int i;
	int parm;
	RMPoint pt;
	uint32 h;
	RMTextDialog *text;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 10);

	bIsBack |= GLOBALS._mCharacter[nChar]._bAlwaysBack ? 1 : 0;

	// Calculates the position of the text according to the current frame
	if (GLOBALS._mCharacter[nChar]._x == -1)
		_ctx->pt = GLOBALS._mCharacter[nChar]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();
	else
		_ctx->pt = RMPoint(GLOBALS._mCharacter[nChar]._x, GLOBALS._mCharacter[nChar]._y);

	// Parameter for special actions: random between the spoken
	_ctx->parm = (GLOBALS._mCharacter[nChar]._curGroup * 10) + g_vm->_randomSource.getRandomNumber(
	                 GLOBALS._mCharacter[nChar]._numTalks[GLOBALS._mCharacter[nChar]._curGroup] - 1) + 1;

	// Try to run the custom function to initialize the speech
	if (GLOBALS._mCharacter[nChar]._item) {
		_ctx->h = mpalQueryDoAction(30, GLOBALS._mCharacter[nChar]._item->mpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
		}
	}

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		// fseek(g_vm->m_vdbFP, curVoc->offset, SEEK_SET);
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		// Create a different object depending on whether it's background or not
		if (bIsBack) {
			GLOBALS._curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setInput(GLOBALS._input);

		// Skipping
		_ctx->text->setSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text->setColor(GLOBALS._mCharacter[nChar]._r, GLOBALS._mCharacter[nChar]._g, GLOBALS._mCharacter[nChar]._b);

		// Write the text
		_ctx->text->writeText((*_ctx->msg)[_ctx->i], nFont);

		// Set the position
		_ctx->text->setPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_vm->_theSound.createSfx(&_ctx->voice);
			g_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
			_ctx->voice->setLoop(false);
			if (bIsBack)
				_ctx->voice->setVolume(55);
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			_ctx->curOffset = g_vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}

		GLOBALS._curBackText = NULL;
		delete _ctx->text;
	}

	delete _ctx->msg;

	// Try to run the custom function to close the speech
	if (GLOBALS._mCharacter[nChar]._item) {
		_ctx->h = mpalQueryDoAction(31, GLOBALS._mCharacter[nChar]._item->mpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
	}

	CORO_END_CODE;
}

/*
 *  Dialogs
 */

int g_curDialog;

DECLARE_CUSTOM_FUNCTION(SendDialogMessage)(CORO_PARAM, uint32 nPers, uint32 nMsg, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	char *string;
	RMTextDialog *text;
	int parm;
	uint32 h;
	bool bIsBack;
	VoiceHeader *curVoc;
	FPSfx *voice;
	RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bIsBack = false;

	// The SendDialogMessage can go in the background if it is a character
	if (nPers != 0 && GLOBALS._isMChar[nPers] && GLOBALS._mCharacter[nPers]._bAlwaysBack)
		_ctx->bIsBack = true;

	_ctx->curVoc = SearchVoiceHeader(g_curDialog, nMsg);
	_ctx->voice = NULL;

	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		g_vm->_theSound.createSfx(&_ctx->voice);
		_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
		_ctx->voice->setLoop(false);
		if (_ctx->bIsBack)
			_ctx->voice->setVolume(55);
	}

	_ctx->string = mpalQueryDialogPeriod(nMsg);

	if (nPers == 0) {
		_ctx->text = new RMTextDialog;
		_ctx->text->setColor(0, 255, 0);
		_ctx->text->setPosition(GLOBALS._tony->position() - RMPoint(0, 130) - GLOBALS._loc->scrollPosition());
		_ctx->text->writeText(_ctx->string, 0);

		if (GLOBALS._dwTonyNumTexts > 0) {
			if (!GLOBALS._bTonyInTexts) {
				if (GLOBALS._nTonyNextTalkType != GLOBALS._tony->TALK_NORMAL) {
					CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);
					if (!GLOBALS._bStaticTalk)
						GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
				} else
					CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_NORMAL);

				GLOBALS._bTonyInTexts = true;
			}
			GLOBALS._dwTonyNumTexts--;
		} else {
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);
			if (!GLOBALS._bStaticTalk)
				GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
		}
	} else if (!GLOBALS._isMChar[nPers]) {
		_ctx->text = new RMTextDialog;

		_ctx->pt = GLOBALS._character[nPers]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();

		if (GLOBALS._character[nPers]._startTalkPattern != 0) {
			GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._startTalkPattern);
			CORO_INVOKE_0(GLOBALS._character[nPers]._item->waitForEndPattern);
		}

		GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._talkPattern);

		_ctx->text->setColor(GLOBALS._character[nPers]._r, GLOBALS._character[nPers]._g, GLOBALS._character[nPers]._b);
		_ctx->text->writeText(_ctx->string, 0);
		_ctx->text->setPosition(_ctx->pt);
	} else {
		if (GLOBALS._mCharacter[nPers]._x == -1)
			_ctx->pt = GLOBALS._mCharacter[nPers]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();
		else
			_ctx->pt = RMPoint(GLOBALS._mCharacter[nPers]._x, GLOBALS._mCharacter[nPers]._y);

		// Parameter for special actions. Random between the spoken.
		_ctx->parm = (GLOBALS._mCharacter[nPers]._curGroup * 10) + g_vm->_randomSource.getRandomNumber(
		                 GLOBALS._mCharacter[nPers]._numTalks[GLOBALS._mCharacter[nPers]._curGroup] - 1) + 1;

		if (GLOBALS._mCharacter[nPers]._numTexts != 0 && GLOBALS._mCharacter[nPers]._bInTexts) {
			GLOBALS._mCharacter[nPers]._numTexts--;
		} else {
			// Try to run the custom function to initialize the speech
			_ctx->h = mpalQueryDoAction(30, GLOBALS._mCharacter[nPers]._item->mpalCode(), _ctx->parm);
			if (_ctx->h != CORO_INVALID_PID_VALUE)
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

			GLOBALS._mCharacter[nPers]._curTalk = _ctx->parm;

			if (GLOBALS._mCharacter[nPers]._numTexts != 0) {
				GLOBALS._mCharacter[nPers]._bInTexts = true;
				GLOBALS._mCharacter[nPers]._numTexts--;
			}
		}

		if (GLOBALS._mCharacter[nPers]._bAlwaysBack) {
			_ctx->text = GLOBALS._curBackText = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);

			_ctx->bIsBack = true;
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setSkipStatus(!GLOBALS._mCharacter[nPers]._bAlwaysBack);
		_ctx->text->setColor(GLOBALS._mCharacter[nPers]._r, GLOBALS._mCharacter[nPers]._g, GLOBALS._mCharacter[nPers]._b);
		_ctx->text->writeText(_ctx->string, 0);
		_ctx->text->setPosition(_ctx->pt);
	}

	if (!GLOBALS._bSkipIdle) {
		_ctx->text->setInput(GLOBALS._input);
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);
	}

	if (_ctx->curVoc) {
		_ctx->voice->stop();
		_ctx->voice->release();
		_ctx->voice = NULL;
	}

	if (nPers != 0) {
		if (!GLOBALS._isMChar[nPers]) {
			if (GLOBALS._character[nPers]._endTalkPattern != 0) {
				GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._endTalkPattern);
				CORO_INVOKE_0(GLOBALS._character[nPers]._item->waitForEndPattern);
			}

			GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._standPattern);
			delete _ctx->text;
		} else {
			if ((GLOBALS._mCharacter[nPers]._bInTexts && GLOBALS._mCharacter[nPers]._numTexts == 0) || !GLOBALS._mCharacter[nPers]._bInTexts) {
				// Try to run the custom function to close the speech
				GLOBALS._mCharacter[nPers]._curTalk = (GLOBALS._mCharacter[nPers]._curTalk % 10) + GLOBALS._mCharacter[nPers]._curGroup * 10;
				_ctx->h = mpalQueryDoAction(31, GLOBALS._mCharacter[nPers]._item->mpalCode(), GLOBALS._mCharacter[nPers]._curTalk);
				if (_ctx->h != CORO_INVALID_PID_VALUE)
					CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

				GLOBALS._mCharacter[nPers]._bInTexts = false;
				GLOBALS._mCharacter[nPers]._numTexts = 0;
			}

			GLOBALS._curBackText = NULL;
			delete _ctx->text;
		}
	} else {
		if ((GLOBALS._dwTonyNumTexts == 0 && GLOBALS._bTonyInTexts) || !GLOBALS._bTonyInTexts) {
			CORO_INVOKE_0(GLOBALS._tony->endTalk);
			GLOBALS._dwTonyNumTexts = 0;
			GLOBALS._bTonyInTexts = false;
		}

		delete _ctx->text;
	}

	globalDestroy(_ctx->string);

	CORO_END_CODE;
}


// @@@@ This cannot be skipped!!!!!!!!!!!!!!!!!!!

DECLARE_CUSTOM_FUNCTION(StartDialog)(CORO_PARAM, uint32 nDialog, uint32 nStartGroup, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	uint32 nChoice;
	uint32 *sl;
	uint32 i, num;
	char *string;
	RMDialogChoice dc;
	int sel;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	g_curDialog = nDialog;

	// Call MPAL to start the dialog
	mpalQueryDoDialog(nDialog, nStartGroup);

	// Wait until a choice is selected
	mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	while (_ctx->nChoice != (uint32) - 1) {
		// Get the list of options
		_ctx->sl = mpalQueryDialogSelectList(_ctx->nChoice);
		for (_ctx->num = 0; _ctx->sl[_ctx->num] != 0; _ctx->num++)
			;

		// If there is only one option, do it automatically, and wait for the next choice
		if (_ctx->num == 1) {
			mpalQueryDialogSelectionDWORD(_ctx->nChoice, _ctx->sl[0]);
			globalDestroy(_ctx->sl);

			// Wait for the next choice to be made
			mpalQueryDialogWaitForChoice(&_ctx->nChoice);
			continue;
		}

		// Making a choice for dialog
		_ctx->dc.init();
		_ctx->dc.setNumChoices(_ctx->num);

		// Writeall the possible options
		for (_ctx->i = 0; _ctx->i < _ctx->num; _ctx->i++) {
			_ctx->string = mpalQueryDialogPeriod(_ctx->sl[_ctx->i]);
			assert(_ctx->string != NULL);
			_ctx->dc.addChoice(_ctx->string);
			globalDestroy(_ctx->string);
		}

		// Activate the object
		g_vm->getEngine()->linkGraphicTask(&_ctx->dc);
		CORO_INVOKE_0(_ctx->dc.show);

		// Draw the pointer
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_NONE);
		g_vm->getEngine()->enableMouse();

		while (!(GLOBALS._input->mouseLeftClicked() && ((_ctx->sel = _ctx->dc.getSelection()) != -1))) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
			CORO_INVOKE_1(_ctx->dc.doFrame, GLOBALS._input->mousePos());
		}

		// Hide the pointer
		g_vm->getEngine()->disableMouse();

		CORO_INVOKE_0(_ctx->dc.hide);
		mpalQueryDialogSelectionDWORD(_ctx->nChoice, _ctx->sl[_ctx->sel]);

		// Closes the choice
		_ctx->dc.close();

		globalDestroy(_ctx->sl);

		// Wait for the next choice to be made
		mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	}

	CORO_END_CODE;
}


/*
 *  Sync between idle and mpal
 */

DECLARE_CUSTOM_FUNCTION(TakeOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (GLOBALS._mut[num]._ownerPid != (uint32)CoroScheduler.getCurrentPID()) {
		// The mutex is currently owned by a different process.
		// Wait for the event to be signalled, which means the mutex is free.
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._mut[num]._eventId, CORO_INFINITE);
		GLOBALS._mut[num]._ownerPid = (uint32)CoroScheduler.getCurrentPID();
	}

	GLOBALS._mut[num]._lockCount++;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(ReleaseOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	if (!GLOBALS._mut[num]._lockCount) {
		warning("ReleaseOwnership tried to release mutex %d, which isn't held", num);
		return;
	}

	if (GLOBALS._mut[num]._ownerPid != (uint32)CoroScheduler.getCurrentPID())
		error("ReleaseOwnership tried to release mutex %d, which is held by a different process", num);

	GLOBALS._mut[num]._lockCount--;
	if (!GLOBALS._mut[num]._lockCount) {
		GLOBALS._mut[num]._ownerPid = 0;

		// Signal the event, to wake up processes waiting for the lock.
		CoroScheduler.setEvent(GLOBALS._mut[num]._eventId);
	}
}

/*
 *  Music
 *  -----
 *
 * Fadeout effects supposed:
 *
 *   nFX = 0 - The new music replaces the old one
 *   nFX=1   - The new music interfades with the old one
 *   nFX=2   - The new music takes over in time from the old
 *
 */

void ThreadFadeInMusic(CORO_PARAM, const void *nMusic) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	int nChannel = *(const int *)nMusic;

	CORO_BEGIN_CODE(_ctx);

	debug("Start FadeIn Music");

	for (_ctx->i = 0; _ctx->i < 16; _ctx->i++) {
		g_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}
	g_vm->setMusicVolume(nChannel, 64);

	debug("End FadeIn Music");

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void ThreadFadeOutMusic(CORO_PARAM, const void *nMusic) {
	CORO_BEGIN_CONTEXT;
	int i;
	int startVolume;
	CORO_END_CONTEXT(_ctx);

	int nChannel = *(const int *)nMusic;

	CORO_BEGIN_CODE(_ctx);

	_ctx->startVolume = g_vm->getMusicVolume(nChannel);

	for (_ctx->i = 16; _ctx->i > 0 && !GLOBALS._bFadeOutStop; _ctx->i--) {
		if (_ctx->i * 4 < _ctx->startVolume)
			g_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}

	if (!GLOBALS._bFadeOutStop)
		g_vm->setMusicVolume(nChannel, 0);

	// If a jingle is played, stop it
	if (nChannel == 2)
		g_vm->stopMusic(2);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(FadeInSoundEffect)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CoroScheduler.createProcess(ThreadFadeInMusic, &GLOBALS._curSoundEffect, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutSoundEffect)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = false;
	CoroScheduler.createProcess(ThreadFadeOutMusic, &GLOBALS._curSoundEffect, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutJingle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = false;
	int channel = 2;
	CoroScheduler.createProcess(ThreadFadeOutMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeInJingle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	int channel = 2;
	CoroScheduler.createProcess(ThreadFadeInMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(StopSoundEffect)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->stopMusic(GLOBALS._curSoundEffect);
}

DECLARE_CUSTOM_FUNCTION(StopJingle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->stopMusic(2);
}

DECLARE_CUSTOM_FUNCTION(MuteSoundEffect)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(GLOBALS._curSoundEffect, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteSoundEffect)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = true;
	g_vm->setMusicVolume(GLOBALS._curSoundEffect, 64);
}

DECLARE_CUSTOM_FUNCTION(MuteJingle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(2, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteJingle)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(2, 64);
}

void CustPlayMusic(uint32 nChannel, const char *mFN, uint32 nFX, bool bLoop, int nSync = 0) {
	if (nSync == 0)
		nSync = 2000;
	debugC(DEBUG_INTERMEDIATE, kTonyDebugMusic, "Start CustPlayMusic");
	g_vm->playMusic(nChannel, mFN, nFX, bLoop, nSync);
	debugC(DEBUG_INTERMEDIATE, kTonyDebugMusic, "End CustPlayMusic");
}

DECLARE_CUSTOM_FUNCTION(PlaySoundEffect)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bNoLoop, uint32) {
	if (nFX == 0 || nFX == 1 || nFX == 2) {
		debugC(DEBUG_INTERMEDIATE, kTonyDebugSound, "PlaySoundEffect stop fadeout");
		GLOBALS._bFadeOutStop = true;
	}

	GLOBALS._lastMusic = nMusic;
	CustPlayMusic(GLOBALS._curSoundEffect, musicFiles[nMusic].name, nFX, bNoLoop ? false : true, musicFiles[nMusic].sync);
}

DECLARE_CUSTOM_FUNCTION(PlayJingle)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bLoop, uint32) {
	CustPlayMusic(2, jingleFileNames[nMusic], nFX, bLoop);
}

DECLARE_CUSTOM_FUNCTION(PlayItemSfx)(CORO_PARAM, uint32 nItem, uint32 nSFX, uint32, uint32) {
	if (nItem == 0) {
		GLOBALS._tony->playSfx(nSFX);
	} else {
		RMItem *item = GLOBALS._loc->getItemFromCode(nItem);
		if (item)
			item->playSfx(nSFX);
	}
}


void RestoreMusic(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(PlaySoundEffect, GLOBALS._lastMusic, 0, 0, 0);

	if (GLOBALS._lastTappeto != 0)
		CustPlayMusic(4, ambianceFile[GLOBALS._lastTappeto], 0, true);

	CORO_END_CODE;
}

void SaveMusic(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS._lastMusic);
	f->writeByte(GLOBALS._lastTappeto);
}

void LoadMusic(Common::InSaveFile *f) {
	GLOBALS._lastMusic = f->readByte();
	GLOBALS._lastTappeto = f->readByte();
}


DECLARE_CUSTOM_FUNCTION(JingleFadeStart)(CORO_PARAM, uint32 nJingle, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(FadeOutSoundEffect, 0, 0, 0, 0);
	CORO_INVOKE_4(MuteJingle, 0, 0, 0, 0);
	CORO_INVOKE_4(PlayJingle, nJingle, 0, bLoop, 0);
	CORO_INVOKE_4(FadeInJingle, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(JingleFadeEnd)(CORO_PARAM, uint32 nJingle, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(FadeOutJingle, 0, 0, 0, 0);
	CORO_INVOKE_4(FadeInSoundEffect, 0, 0, 0, 0);

	CORO_END_CODE;
}




DECLARE_CUSTOM_FUNCTION(MustSkipIdleStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bSkipIdle = true;
	CoroScheduler.setEvent(GLOBALS._hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(MustSkipIdleEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bSkipIdle = false;
	CoroScheduler.resetEvent(GLOBALS._hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(PatIrqFreeze)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	// Unused in ScummVM.
}

DECLARE_CUSTOM_FUNCTION(OpenInitLoadMenu)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(g_vm->openInitLoadMenu);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(OpenInitOptions)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(g_vm->openInitOptions);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(DoCredits)(CORO_PARAM, uint32 nMsg, uint32 dwTime, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	RMTextDialog *text;
	uint32 hDisable;
	int i;
	uint32 startTime;

	~CoroContextTag() {
		delete msg;
		delete[] text;
	}

	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);
	_ctx->hDisable = CoroScheduler.createEvent(true, false);

	_ctx->text = new RMTextDialog[_ctx->msg->numPeriods()];

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods(); _ctx->i++)     {
		_ctx->text[_ctx->i].setInput(GLOBALS._input);

		// Alignment
		if ((*_ctx->msg)[_ctx->i][0] == '@') {
			_ctx->text[_ctx->i].setAlignType(RMText::HCENTER, RMText::VTOP);
			_ctx->text[_ctx->i].writeText(&(*_ctx->msg)[_ctx->i][1], 3);
			_ctx->text[_ctx->i].setPosition(RMPoint(414, 70 + _ctx->i * 26));  // 70
		} else {
			_ctx->text[_ctx->i].setAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->text[_ctx->i].writeText((*_ctx->msg)[_ctx->i], 3);
			_ctx->text[_ctx->i].setPosition(RMPoint(260, 70 + _ctx->i * 26));
		}


		// Set the position
		_ctx->text[_ctx->i].setAlwaysDisplay();
		_ctx->text[_ctx->i].setForcedTime(dwTime * 1000);
		_ctx->text[_ctx->i].setNoTab();

		// Wait for the end of display
		_ctx->text[_ctx->i].setCustomSkipHandle(_ctx->hDisable);

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->text[_ctx->i]);
	}

	_ctx->startTime = g_vm->getTime();

	while (_ctx->startTime + dwTime * 1000 > g_vm->getTime()) {
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
		if (GLOBALS._input->mouseLeftClicked() || GLOBALS._input->mouseRightClicked())
			break;
		if (g_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
			break;
	}

	CoroScheduler.setEvent(_ctx->hDisable);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	delete[] _ctx->text;
	delete _ctx->msg;
	_ctx->text = NULL;
	_ctx->msg = NULL;

	CORO_END_CODE;
}



BEGIN_CUSTOM_FUNCTION_MAP()

ASSIGN(1,   CustLoadLocation)
ASSIGN(2,   MySleep)
ASSIGN(3,   SetPointer)
ASSIGN(5,   MoveTony)
ASSIGN(6,   FaceToMe)
ASSIGN(7,   BackToMe)
ASSIGN(8,   LeftToMe)
ASSIGN(9,   RightToMe)
ASSIGN(10,  SendTonyMessage)
ASSIGN(11,  ChangeBoxStatus)
ASSIGN(12,  ChangeLocation)
ASSIGN(13,  DisableTony)
ASSIGN(14,  EnableTony)
ASSIGN(15,  WaitForPatternEnd)
ASSIGN(16,  SetLocStartPosition)
ASSIGN(17,  ScrollLocation)
ASSIGN(18,  MoveTonyAndWait)
ASSIGN(19,  ChangeHotspot)
ASSIGN(20,  AddInventory)
ASSIGN(21,  RemoveInventory)
ASSIGN(22,  ChangeInventoryStatus)
ASSIGN(23,  SetTonyPosition)
ASSIGN(24,  SendFullscreenMessage)
ASSIGN(25,  SaveTonyPosition)
ASSIGN(26,  RestoreTonyPosition)
ASSIGN(27,  DisableInput)
ASSIGN(28,  EnableInput)
ASSIGN(29,  StopTony)

ASSIGN(30,  TonyTakeUp1)
ASSIGN(31,  TonyTakeMid1)
ASSIGN(32,  TonyTakeDown1)
ASSIGN(33,  TonyTakeUp2)
ASSIGN(34,  TonyTakeMid2)
ASSIGN(35,  TonyTakeDown2)

ASSIGN(72,  TonyPutUp1)
ASSIGN(73,  TonyPutMid1)
ASSIGN(74,  TonyPutDown1)
ASSIGN(75,  TonyPutUp2)
ASSIGN(76,  TonyPutMid2)
ASSIGN(77,  TonyPutDown2)

ASSIGN(36,  TonyOnTheFloor)
ASSIGN(37,  TonyGetUp)
ASSIGN(38,  TonyShepherdess)
ASSIGN(39,  TonyWhistle)

ASSIGN(40,  TonyLaugh)
ASSIGN(41,  TonyHips)
ASSIGN(42,  TonySing)
ASSIGN(43,  TonyIndicate)
ASSIGN(44,  TonyScaredWithHands)
ASSIGN(49,  TonyScaredWithoutHands)
ASSIGN(45,  TonyWithGlasses)
ASSIGN(46,  TonyWithWorm)
ASSIGN(47,  TonyWithHammer)
ASSIGN(48,  TonyWithRope)
ASSIGN(90,  TonyWithRabbitANIM)
ASSIGN(91,  TonyWithRecipeANIM)
ASSIGN(92,  TonyWithCardsANIM)
ASSIGN(93,  TonyWithSnowmanANIM)
ASSIGN(94,  TonyWithSnowmanStart)
ASSIGN(95,  TonyWithSnowmanEnd)
ASSIGN(96,  TonyWithRabbitStart)
ASSIGN(97,  TonyWithRabbitEnd)
ASSIGN(98,  TonyWithRecipeStart)
ASSIGN(99,  TonyWithRecipeEnd)
ASSIGN(100, TonyWithCardsStart)
ASSIGN(101, TonyWithCardsEnd)
ASSIGN(102, TonyWithNotebookStart)
ASSIGN(103, TonyWithNotebookEnd)
ASSIGN(104, TonyWithMegaphoneStart)
ASSIGN(105, TonyWithMegaphoneEnd)
ASSIGN(106, TonyWithBeardStart)
ASSIGN(107, TonyWithBeardEnd)
ASSIGN(108, TonyGiggle)
ASSIGN(109, TonyDisgusted)
ASSIGN(110, TonySarcastic)
ASSIGN(111, TonyMacbeth)
ASSIGN(112, TonySniffLeft)
ASSIGN(113, TonySniffRight)
ASSIGN(114, TonyScaredStart)
ASSIGN(115, TonyScaredEnd)
ASSIGN(116, TonyWithSecretary)

ASSIGN(50,  CharSetCode)
ASSIGN(51,  CharSetColor)
ASSIGN(52,  CharSetTalkPattern)
ASSIGN(53,  CharSendMessage)
ASSIGN(54,  CharSetStartEndTalkPattern)

ASSIGN(60,  MCharSetCode)
ASSIGN(61,  MCharSetColor)
ASSIGN(62,  MCharSetCurrentGroup)
ASSIGN(63,  MCharSetNumTalksInGroup)
ASSIGN(64,  MCharSetNumTexts)
ASSIGN(65,  MCharSendMessage)
ASSIGN(66,  MCharSetPosition)
ASSIGN(67,  MCharSetAlwaysBack)
ASSIGN(68,  MCharResetCode)

ASSIGN(70,  StartDialog)
ASSIGN(71,  SendDialogMessage)

ASSIGN(80,  TakeOwnership)
ASSIGN(81,  ReleaseOwnership)

ASSIGN(86,  PlaySoundEffect)
ASSIGN(87,  PlayJingle)
ASSIGN(88,  FadeInSoundEffect)
ASSIGN(89,  FadeOutSoundEffect)
ASSIGN(123, FadeInJingle)
ASSIGN(124, FadeOutJingle)
ASSIGN(125, MuteSoundEffect)
ASSIGN(126, DemuteSoundEffect)
ASSIGN(127, MuteJingle)
ASSIGN(128, DemuteJingle)
ASSIGN(84,  StopSoundEffect)
ASSIGN(85,  StopJingle)
ASSIGN(83,  PlayItemSfx)
ASSIGN(129, JingleFadeStart)
ASSIGN(130, JingleFadeEnd)

ASSIGN(120, ShakeScreen)
ASSIGN(121, AutoSave)
ASSIGN(122, AbortGame)
ASSIGN(131, NoBullsEye)
ASSIGN(132, SendFullscreenMsgStart)
ASSIGN(133, SendFullscreenMsgEnd)
ASSIGN(134, CustEnableGUI)
ASSIGN(135, CustDisableGUI)
ASSIGN(136, ClearScreen)
ASSIGN(137, PatIrqFreeze)
ASSIGN(138, TonySetPerorate)
ASSIGN(139, OpenInitLoadMenu)
ASSIGN(140, OpenInitOptions)
ASSIGN(141, SyncScrollLocation)
ASSIGN(142, CloseLocation)
ASSIGN(143, SetAlwaysDisplay)
ASSIGN(144, DoCredits)

ASSIGN(200, MustSkipIdleStart);
ASSIGN(201, MustSkipIdleEnd);

END_CUSTOM_FUNCTION_MAP()

void processKilledCallback(Common::PROCESS *p) {
	for (uint i = 0; i < 10; i++)
		if (GLOBALS._mut[i]._ownerPid == p->pid) {
			// Handle scripts which don't call ReleaseOwnership, such as
			// the one in loc37's vEnter when Tony is chasing the mouse.
			debug(DEBUG_BASIC, "Force-releasing mutex %d after process died", i);

			GLOBALS._mut[i]._ownerPid = 0;
			GLOBALS._mut[i]._lockCount = 0;
			CoroScheduler.setEvent(GLOBALS._mut[i]._eventId);
		}
}

void setupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input) {
	GLOBALS._tony = tony;
	GLOBALS._pointer = ptr;
	GLOBALS._boxes = box;
	GLOBALS._loc = loc;
	GLOBALS._inventory = inv;
	GLOBALS._input = input;

	GLOBALS.DisableGUI = mainDisableGUI;
	GLOBALS.EnableGUI = mainEnableGUI;

	GLOBALS._bAlwaysDisplay = false;
	int i;

	CoroScheduler.setResourceCallback(processKilledCallback);
	for (i = 0; i < 10; i++)
		GLOBALS._mut[i]._eventId = CoroScheduler.createEvent(false, true);

	for (i = 0; i < 200; i++)
		GLOBALS._ambiance[i] = 0;

	GLOBALS._ambiance[6] =  AMBIANCE_CRICKETS;
	GLOBALS._ambiance[7] =  AMBIANCE_CRICKETS;
	GLOBALS._ambiance[8] =  AMBIANCE_CRICKETSMUFFLED;
	GLOBALS._ambiance[10] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[12] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[13] = AMBIANCE_CRICKETSMUFFLED;
	GLOBALS._ambiance[15] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[16] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[18] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[19] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[20] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[23] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[26] = AMBIANCE_SEAHALFVOLUME;
	GLOBALS._ambiance[27] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[28] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[31] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[33] = AMBIANCE_SEA;
	GLOBALS._ambiance[35] = AMBIANCE_SEA;
	GLOBALS._ambiance[36] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[37] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[40] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[41] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[42] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[45] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[51] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[52] = AMBIANCE_CRICKETSWIND1;
	GLOBALS._ambiance[53] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[54] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[57] = AMBIANCE_WIND;
	GLOBALS._ambiance[58] = AMBIANCE_WIND;
	GLOBALS._ambiance[60] = AMBIANCE_WIND;



	// Create an event for the idle skipping
	GLOBALS._hSkipIdle = CoroScheduler.createEvent(true, false);
}

} // end of namespace Tony
