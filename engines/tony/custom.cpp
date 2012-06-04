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
#include "tony/adv.h"
#include "tony/custom.h"
#include "tony/font.h"
#include "tony/game.h"
#include "tony/gfxcore.h"
#include "tony/tony.h"
#include "tony/tonychar.h"
#include "tony/utils.h"

namespace Tony {

const char *tappetiFile[] =  {
	"None",
	"1.ADP",    // Grilli.WAV
	"2.ADP",    // Grilli-Ovattati.WAV
	"3.ADP",    // Grilli-Vento.WAV
	"3.ADP",    // Grilli-Vento1.WAV
	"5.ADP",    // Vento1.WAV
	"4.ADP",    // Mare1.WAV
	"6.ADP"     // Mare1.WAV metà volume
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


const char *staccFileNames[] = {
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


void ReapplyChangedHotspot(void) {
	int i;
	for (i = 0; i < GLOBALS.curChangedHotspot; i++)
		GLOBALS.Loc->GetItemFromCode(GLOBALS.ChangedHotspot[i].dwCode)->ChangeHotspot(RMPoint(GLOBALS.ChangedHotspot[i].nX, GLOBALS.ChangedHotspot[i].nY));
}

void SaveChangedHotspot(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS.curChangedHotspot);
	if (GLOBALS.curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS.curChangedHotspot; ++i)
			GLOBALS.ChangedHotspot[i].Save(f);
	}
}

void LoadChangedHotspot(Common::InSaveFile *f) {
	GLOBALS.curChangedHotspot = f->readByte();

	if (GLOBALS.curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS.curChangedHotspot; ++i)
			GLOBALS.ChangedHotspot[i].Load(f);
	}
}


/**
 * @defgroup Classes required for custom functions
 *
 * Tony (To Move him)     -> You can do MPAL through the animation? I really think so
 *
 * SendMessage -> I'd say just theEngine.SendMessage()
 * ChangeLocation -> theEngine.ChangeLocation()
 * AddInventory -> theEngine.AddInventory()
*/
void MCharResetCodes(void) {
	for (int i = 0; i < 10; i++)
		GLOBALS.MCharacter[i].item = GLOBALS.Loc->GetItemFromCode(GLOBALS.MCharacter[i].code);
	for (int i = 0; i < 10; i++)
		GLOBALS.Character[i].item = GLOBALS.Loc->GetItemFromCode(GLOBALS.Character[i].code);
}

void CharsSaveAll(Common::OutSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		f->writeByte(GLOBALS.IsMChar[i]);
		if (GLOBALS.IsMChar[i]) {
			GLOBALS.MCharacter[i].Save(f);
		} else {
			GLOBALS.Character[i].Save(f);
		}
	}
}

void CharsLoadAll(Common::InSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		GLOBALS.IsMChar[i] = f->readByte();
		if (GLOBALS.IsMChar[i])
			GLOBALS.MCharacter[i].Load(f);
		else
			GLOBALS.Character[i].Load(f);
	}
}

DECLARE_CUSTOM_FUNCTION(FaceToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_STANDDOWN);
}

DECLARE_CUSTOM_FUNCTION(BackToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_STANDUP);
}

DECLARE_CUSTOM_FUNCTION(LeftToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_STANDLEFT);
}

DECLARE_CUSTOM_FUNCTION(RightToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_STANDRIGHT);
}


DECLARE_CUSTOM_FUNCTION(TonySetPalesati)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	GLOBALS.SetPalesati(bStatus);
}

DECLARE_CUSTOM_FUNCTION(MySleep)(CORO_PARAM, uint32 dwTime, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_1(CoroScheduler.sleep, dwTime);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetAlwaysDisplay)(CORO_PARAM, uint32 val, uint32, uint32, uint32) {
	GLOBALS.bAlwaysDisplay = (val != 0);
}


DECLARE_CUSTOM_FUNCTION(SetPointer)(CORO_PARAM, uint32 dwPointer, uint32, uint32, uint32) {
	switch (dwPointer) {
	case 1:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_FRECCIASU);
		break;
	case 2:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_FRECCIAGIU);
		break;
	case 3:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_FRECCIASINISTRA);
		break;
	case 4:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_FRECCIADESTRA);
		break;
	case 5:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_FRECCIAMAPPA);
		break;

	default:
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_NONE);
		break;
	}
}

VoiceHeader *SearchVoiceHeader(uint32 codehi, uint32 codelo) {
	uint i;
	int code;

	code = (codehi << 16) | codelo;

	if (_vm->_voices.size() == 0)
		return NULL;

	for (i = 0; i < _vm->_voices.size(); i++)
		if (_vm->_voices[i]._code == code)
			return &_vm->_voices[i];

	return NULL;
}


DECLARE_CUSTOM_FUNCTION(SendTonyMessage)(CORO_PARAM, uint32 dwMessage, uint32 nX, uint32 nY, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage msg;
	int i;
	int curOffset;
	VoiceHeader *curVoc;
	FPSFX *voice;
	RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curOffset = 0;

	if (GLOBALS.bSkipIdle) return;

	_ctx->msg.Load(dwMessage);
	if (!_ctx->msg.IsValid()) {
		return;
	}

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Is positioned within the database of entries beginning at the first
		_ctx->curOffset = _ctx->curVoc->_offset;

		// First time allocation
		_vm->_vdbFP.seek(_ctx->curOffset);
		_vm->_theSound.CreateSfx(&_ctx->voice);

		_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
		_ctx->curOffset = _vm->_vdbFP.pos();

		_ctx->voice->SetLoop(false);
	}

	if (GLOBALS.nTonyNextTalkType != GLOBALS.Tony->TALK_NORMAL) {
		CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.nTonyNextTalkType);

		if (!GLOBALS.bStaticTalk)
			GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;
	} else {
		if (_ctx->msg.NumPeriods() > 1)
			CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.Tony->TALK_FIANCHI);
		else
			CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.Tony->TALK_NORMAL);
	}

	if (GLOBALS.curBackText)
		CORO_INVOKE_0(GLOBALS.curBackText->Hide);

	GLOBALS.bTonyIsSpeaking = true;

	for (_ctx->i = 0; _ctx->i < _ctx->msg.NumPeriods() && !GLOBALS.bSkipIdle; _ctx->i++) {
		_ctx->text.SetInput(GLOBALS.Input);

		// Alignment
		_ctx->text.SetAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Colour
		_ctx->text.SetColor(0, 255, 0);

		// Writes the text
		_ctx->text.WriteText(_ctx->msg[_ctx->i], 0);

		// Set the position
		if (nX == 0 && nY == 0)
			_ctx->text.SetPosition(GLOBALS.Tony->Position() - RMPoint(0, 130) - GLOBALS.Loc->ScrollPosition());
		else
			_ctx->text.SetPosition(RMPoint(nX, nY) - GLOBALS.Loc->ScrollPosition());

		// Handling for always display
		if (GLOBALS.bAlwaysDisplay) {
			_ctx->text.SetAlwaysDisplay();
			_ctx->text.ForceTime();
		}

		// Record the text
		GLOBALS.LinkGraphicTask(&_ctx->text);

		if (_ctx->curVoc) {
			if (_ctx->i == 0) {
				_ctx->voice->Play();
				_ctx->text.SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			} else {
				_vm->_vdbFP.seek(_ctx->curOffset);
				_vm->_theSound.CreateSfx(&_ctx->voice);
				_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);

				_ctx->curOffset = _vm->_vdbFP.pos();
				_ctx->voice->SetLoop(false);
				_ctx->voice->Play();
				_ctx->text.SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			}
		}

		// Wait for the end of the display
		_ctx->text.SetCustomSkipHandle(GLOBALS.hSkipIdle);
		CORO_INVOKE_0(_ctx->text.WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice = NULL;
		}
	}

	GLOBALS.bTonyIsSpeaking = false;
	if (GLOBALS.curBackText)
		GLOBALS.curBackText->Show();

	CORO_INVOKE_0(GLOBALS.Tony->EndTalk);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(ChangeBoxStatus)(CORO_PARAM, uint32 nLoc, uint32 nBox, uint32 nStatus, uint32) {
	GLOBALS.Boxes->ChangeBoxStatus(nLoc, nBox, nStatus);
}


DECLARE_CUSTOM_FUNCTION(CustLoadLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();

	GLOBALS.curChangedHotspot = 0;
	if (bUseStartPos != 0)
		GLOBALS.LoadLocation(nLoc, RMPoint(tX, tY), GLOBALS.StartLocPos[nLoc]);
	else
		GLOBALS.LoadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	GLOBALS.Unfreeze();
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

	GLOBALS.SFM_nLoc = GLOBALS.Loc->TEMPGetNumLoc();
	GLOBALS.SFM_pt = GLOBALS.Tony->Position();

	if (GLOBALS.bSkipIdle)
		return;

	CORO_INVOKE_2(GLOBALS.UnloadLocation, false, NULL);
	GLOBALS.Tony->Hide();
	GLOBALS.Unfreeze();

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !GLOBALS.bSkipIdle; _ctx->i++) {
		_ctx->text.SetInput(GLOBALS.Input);

		// Alignment
		_ctx->text.SetAlignType(RMText::HCENTER, RMText::VCENTER);

		// Forces the text to disappear in time
		_ctx->text.ForceTime();

		// Colour
		_ctx->text.SetColor(255, 255, 255);

		// Write the text
		if (nFont == 0)
			_ctx->text.WriteText((*_ctx->msg)[_ctx->i], 1);
		else if (nFont == 1)
			_ctx->text.WriteText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text.SetPosition(RMPoint(320, 240));

		_ctx->text.SetAlwaysDisplay();
		_ctx->text.ForceTime();

		// Record the text
		GLOBALS.LinkGraphicTask(&_ctx->clear);
		GLOBALS.LinkGraphicTask(&_ctx->text);

		// Wait for the end of display
		_ctx->text.SetCustomSkipHandle(GLOBALS.hSkipIdle);
		CORO_INVOKE_0(_ctx->text.WaitForEndDisplay);
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

	GLOBALS.LinkGraphicTask(&_ctx->clear);

	CORO_INVOKE_0(GLOBALS.WaitFrame);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SendFullscreenMsgEnd)(CORO_PARAM, uint32 bNotEnableTony, uint32, uint32, uint32) {
	GLOBALS.Freeze();
	GLOBALS.LoadLocation(GLOBALS.SFM_nLoc, RMPoint(GLOBALS.SFM_pt.x, GLOBALS.SFM_pt.y), RMPoint(-1, -1));
	if (!bNotEnableTony)
		GLOBALS.Tony->Show();
	GLOBALS.Unfreeze();

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

DECLARE_CUSTOM_FUNCTION(NoOcchioDiBue)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bNoOcchioDiBue = true;
}

DECLARE_CUSTOM_FUNCTION(CloseLocation)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS.bNoOcchioDiBue) {
		GLOBALS.InitWipe(1);
		CORO_INVOKE_0(GLOBALS.WaitWipeEnd);
	}

	_vm->stopMusic(4);

	// On exit, unload and unfreeze
	CORO_INVOKE_2(GLOBALS.UnloadLocation, true, NULL);
	GLOBALS.Unfreeze();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS.bNoOcchioDiBue) {
		GLOBALS.InitWipe(1);
		CORO_INVOKE_0(GLOBALS.WaitWipeEnd);
	}

	if (GLOBALS.lastTappeto != GLOBALS.tappeti[nLoc]) {
		_vm->stopMusic(4);
	}

	// On exit, unfreeze
	CORO_INVOKE_2(GLOBALS.UnloadLocation, true, NULL);

	GLOBALS.curChangedHotspot = 0;
	if (bUseStartPos != 0)
		GLOBALS.LoadLocation(nLoc, RMPoint(tX, tY), GLOBALS.StartLocPos[nLoc]);
	else
		GLOBALS.LoadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	if (GLOBALS.lastTappeto != GLOBALS.tappeti[nLoc]) {
		GLOBALS.lastTappeto = GLOBALS.tappeti[nLoc];
		if (GLOBALS.lastTappeto != 0)
			_vm->playMusic(4, tappetiFile[GLOBALS.lastTappeto], 0, true, 2000);
	}

	if (!GLOBALS.bNoOcchioDiBue) {
		GLOBALS.InitWipe(2);
	}

	GLOBALS.Unfreeze();


	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	if (!GLOBALS.bNoOcchioDiBue) {
		CORO_INVOKE_0(GLOBALS.WaitWipeEnd);
		GLOBALS.CloseWipe();
	}

	GLOBALS.bNoOcchioDiBue = false;

	// On Enter?
	if (_ctx->h != CORO_INVALID_PID_VALUE)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetLocStartPosition)(CORO_PARAM, uint32 nLoc, uint32 lX, uint32 lY, uint32) {
	GLOBALS.StartLocPos[nLoc].Set(lX, lY);
}

DECLARE_CUSTOM_FUNCTION(SaveTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.saveTonyPos = GLOBALS.Tony->Position();
	GLOBALS.saveTonyLoc = GLOBALS.Loc->TEMPGetNumLoc();
}

DECLARE_CUSTOM_FUNCTION(RestoreTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(ChangeLocation, GLOBALS.saveTonyLoc, GLOBALS.saveTonyPos.x, GLOBALS.saveTonyPos.y, 0);

	MCharResetCodes();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(DisableInput)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	MainDisableInput();
}


DECLARE_CUSTOM_FUNCTION(EnableInput)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	MainEnableInput();
}

DECLARE_CUSTOM_FUNCTION(StopTony)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->StopNoAction(coroParam);
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

	GLOBALS.Freeze();
	GLOBALS.Tony->Take(nDirection, 0);
	GLOBALS.Unfreeze();

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericTake2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();
	GLOBALS.Tony->Take(nDirection, 1);
	GLOBALS.Unfreeze();

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	GLOBALS.Freeze();
	GLOBALS.Tony->Take(nDirection, 2);
	GLOBALS.Unfreeze();

	CORO_END_CODE;
}

void TonyGenericPut1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();
	GLOBALS.Tony->Put(nDirection, 0);
	GLOBALS.Unfreeze();

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericPut2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();
	GLOBALS.Tony->Put(nDirection, 1);
	GLOBALS.Unfreeze();

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	GLOBALS.Freeze();
	GLOBALS.Tony->Put(nDirection, 2);
	GLOBALS.Unfreeze();

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


DECLARE_CUSTOM_FUNCTION(TonyPerTerra)(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	if (dwParte == 0)
		GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_PERTERRALEFT);
	else
		GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_PERTERRARIGHT);
}

DECLARE_CUSTOM_FUNCTION(TonySiRialza)(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (dwParte == 0)
		GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_SIRIALZALEFT);
	else
		GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_SIRIALZARIGHT);

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyPastorella)(CORO_PARAM, uint32 bIsPast, uint32, uint32, uint32) {
	GLOBALS.Tony->SetPastorella(bIsPast);
}

DECLARE_CUSTOM_FUNCTION(TonyFischietto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_FISCHIETTORIGHT);
	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);

	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_STANDRIGHT);

	CORO_END_CODE;
}


void TonySetNumTexts(uint32 dwText) {
	GLOBALS.dwTonyNumTexts = dwText;
	GLOBALS.bTonyInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(TonyRide)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_RIDE;
}

DECLARE_CUSTOM_FUNCTION(TonyRidacchia)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_RIDE2;
}

DECLARE_CUSTOM_FUNCTION(TonyFianchi)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_FIANCHI;
}

DECLARE_CUSTOM_FUNCTION(TonyCanta)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CANTA;
}

DECLARE_CUSTOM_FUNCTION(TonySiIndica)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_SIINDICA;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoConMani)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_SPAVENTATO;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoSenzaMani)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_SPAVENTATO2;
}

DECLARE_CUSTOM_FUNCTION(TonyConMartello)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONMARTELLO;
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_CONMARTELLO);
}

DECLARE_CUSTOM_FUNCTION(TonyConBicchiere)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONBICCHIERE;
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_CONBICCHIERE);
}

DECLARE_CUSTOM_FUNCTION(TonyConVerme)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONVERME;
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_CONVERME);
}

DECLARE_CUSTOM_FUNCTION(TonyConCorda)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONCORDA;
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_CONCORDA);
}

DECLARE_CUSTOM_FUNCTION(TonyConSegretaria)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONSEGRETARIA;
	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_CONSEGRETARIA);
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONCONIGLIO;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONRICETTA;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONCARTE;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONPUPAZZO;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONPUPAZZOSTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONPUPAZZOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONPUPAZZOSTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONCONIGLIOSTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONCONIGLIOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONCONIGLIOSTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONRICETTASTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONRICETTASTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONRICETTASTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONCARTESTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONCARTESTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONCARTESTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConTaccuinoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONTACCUINOSTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONTACCUINOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConTaccuinoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONTACCUINOSTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConMegafonoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONMEGAFONOSTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONMEGAFONOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConMegafonoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONMEGAFONOSTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConBarbaStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_CONBARBASTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_CONBARBASTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConBarbaEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_CONBARBASTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_SPAVENTATOSTATIC;
	GLOBALS.bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS.Tony->StartStatic, GLOBALS.Tony->TALK_SPAVENTATOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->EndStatic, GLOBALS.Tony->TALK_SPAVENTATOSTATIC);
	GLOBALS.bStaticTalk = false;
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(TonySchifato)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_SCHIFATO;
}

DECLARE_CUSTOM_FUNCTION(TonySniffaLeft)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_SNIFFA_LEFT);
	CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);
	CORO_INVOKE_4(LeftToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySniffaRight)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Tony->SetPattern(GLOBALS.Tony->PAT_SNIFFA_RIGHT);
	CORO_INVOKE_0(GLOBALS.Tony->WaitForEndPattern);
	CORO_INVOKE_4(RightToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyNaah)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NAAH;
}

DECLARE_CUSTOM_FUNCTION(TonyMacbeth)(CORO_PARAM, uint32 nPos, uint32, uint32, uint32) {
	switch (nPos) {
	case 1:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH1;
		break;
	case 2:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH2;
		break;
	case 3:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH3;
		break;
	case 4:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH4;
		break;
	case 5:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH5;
		break;
	case 6:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH6;
		break;
	case 7:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH7;
		break;
	case 8:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH8;
		break;
	case 9:
		GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_MACBETH9;
		break;
	}
}


DECLARE_CUSTOM_FUNCTION(EnableTony)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.Tony->Show();
}

DECLARE_CUSTOM_FUNCTION(DisableTony)(CORO_PARAM, uint32 bShowOmbra, uint32, uint32, uint32) {
	GLOBALS.Tony->Hide(bShowOmbra);
}

DECLARE_CUSTOM_FUNCTION(WaitForPatternEnd)(CORO_PARAM, uint32 nItem, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMItem *item;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->item = GLOBALS.Loc->GetItemFromCode(nItem);

	if (!GLOBALS.bSkipIdle && _ctx->item != NULL)
		CORO_INVOKE_1(_ctx->item->WaitForEndPattern, GLOBALS.hSkipIdle);

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(SetTonyPosition)(CORO_PARAM, uint32 nX, uint32 nY, uint32 nLoc, uint32) {
	GLOBALS.Tony->SetPosition(RMPoint(nX, nY), nLoc);
}

DECLARE_CUSTOM_FUNCTION(MoveTonyAndWait)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS.Tony->Move, RMPoint(nX, nY));

	if (!GLOBALS.bSkipIdle)
		CORO_INVOKE_0(GLOBALS.Tony->WaitForEndMovement);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(MoveTony)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	GLOBALS.Tony->Move(coroParam, RMPoint(nX, nY));
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

	_ctx->pt = GLOBALS.Loc->ScrollPosition();

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS.bSkipIdle) {
		if (_ctx->lx > 0) {
			_ctx->lx -= (int32)sX;
			if (_ctx->lx < 0) _ctx->lx = 0;
			_ctx->pt.Offset((int32)sX, 0);
		} else if (_ctx->lx < 0) {
			_ctx->lx += (int32)sX;
			if (_ctx->lx > 0) _ctx->lx = 0;
			_ctx->pt.Offset(-(int32)sX, 0);
		}

		if (_ctx->ly > 0) {
			_ctx->ly -= sY;
			if (_ctx->ly < 0) _ctx->ly = 0;
			_ctx->pt.Offset(0, sY);
		} else if (_ctx->ly < 0) {
			_ctx->ly += sY;
			if (_ctx->ly > 0) _ctx->ly = 0;
			_ctx->pt.Offset(0, -(int32)sY);
		}

		CORO_INVOKE_0(GLOBALS.WaitFrame);

		GLOBALS.Freeze();
		GLOBALS.Loc->SetScrollPosition(_ctx->pt);
		GLOBALS.Tony->SetScrollPosition(_ctx->pt);
		GLOBALS.Unfreeze();
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

	_ctx->startpt = GLOBALS.Loc->ScrollPosition();

	_ctx->dwStartTime = _vm->getTime();

	if (sX)
		_ctx->dwTotalTime = _ctx->dimx * (1000 / 35) / sX;
	else
		_ctx->dwTotalTime = _ctx->dimy * (1000 / 35) / sY;

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS.bSkipIdle) {
		_ctx->dwCurTime = _vm->getTime() - _ctx->dwStartTime;
		if (_ctx->dwCurTime > _ctx->dwTotalTime)
			break;

		_ctx->pt = _ctx->startpt;

		if (sX) {
			if (_ctx->lx > 0)
				_ctx->pt.x += (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt.x -= (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
		} else {
			if (_ctx->ly > 0)
				_ctx->pt.y += (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt.y -= (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;

		}

		CORO_INVOKE_0(GLOBALS.WaitFrame);

		GLOBALS.Freeze();
		GLOBALS.Loc->SetScrollPosition(_ctx->pt);
		GLOBALS.Tony->SetScrollPosition(_ctx->pt);
		GLOBALS.Unfreeze();

	}


	// Set the position finale
	if (sX) {
		if (_ctx->lx > 0)
			_ctx->pt.x = _ctx->startpt.x + _ctx->dimx;
		else
			_ctx->pt.x = _ctx->startpt.x - _ctx->dimx;
	} else {
		if (_ctx->ly > 0)
			_ctx->pt.y = _ctx->startpt.y + _ctx->dimy;
		else
			_ctx->pt.y = _ctx->startpt.y - _ctx->dimy;
	}

	GLOBALS.Freeze();
	GLOBALS.Loc->SetScrollPosition(_ctx->pt);
	GLOBALS.Tony->SetScrollPosition(_ctx->pt);
	GLOBALS.Unfreeze();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeHotspot)(CORO_PARAM, uint32 dwCode, uint32 nX, uint32 nY, uint32) {
	int i;

	for (i = 0; i < GLOBALS.curChangedHotspot; i++)
		if (GLOBALS.ChangedHotspot[i].dwCode == dwCode) {
			GLOBALS.ChangedHotspot[i].nX = nX;
			GLOBALS.ChangedHotspot[i].nY = nY;
			break;
		}

	if (i == GLOBALS.curChangedHotspot) {
		GLOBALS.ChangedHotspot[i].dwCode = dwCode;
		GLOBALS.ChangedHotspot[i].nX = nX;
		GLOBALS.ChangedHotspot[i].nY = nY;
		GLOBALS.curChangedHotspot++;
	}

	GLOBALS.Loc->GetItemFromCode(dwCode)->ChangeHotspot(RMPoint(nX, nY));
}


DECLARE_CUSTOM_FUNCTION(AutoSave)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->autoSave(coroParam);
}

DECLARE_CUSTOM_FUNCTION(AbortGame)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->abortGame();
}

DECLARE_CUSTOM_FUNCTION(TremaSchermo)(CORO_PARAM, uint32 nScosse, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	uint32 i;
	uint32 curTime;
	int dirx, diry;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = _vm->getTime();

	_ctx->dirx = 1;
	_ctx->diry = 1;

	while (_vm->getTime() < _ctx->curTime + nScosse) {
		CORO_INVOKE_0(GLOBALS.WaitFrame);

		GLOBALS.Freeze();
		GLOBALS.Loc->SetFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		GLOBALS.Tony->SetFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		GLOBALS.Unfreeze();

		_ctx->i = _vm->_randomSource.getRandomNumber(2);

		if (_ctx->i == 0 || _ctx->i == 2)
			_ctx->dirx = -_ctx->dirx;
		else if (_ctx->i == 1 || _ctx->i == 2)
			_ctx->diry = -_ctx->diry;
	}

	GLOBALS.Freeze();
	GLOBALS.Loc->SetFixedScroll(RMPoint(0, 0));
	GLOBALS.Tony->SetFixedScroll(RMPoint(0, 0));
	GLOBALS.Unfreeze();

	CORO_END_CODE;
}



/*
 *  Personaggi
 */

DECLARE_CUSTOM_FUNCTION(CharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 16);
	GLOBALS.Character[nChar].code = nCode;
	GLOBALS.Character[nChar].item = GLOBALS.Loc->GetItemFromCode(nCode);
	GLOBALS.Character[nChar].r = 255;
	GLOBALS.Character[nChar].g = 255;
	GLOBALS.Character[nChar].b = 255;
	GLOBALS.Character[nChar].talkpattern = 0;
	GLOBALS.Character[nChar].starttalkpattern = 0;
	GLOBALS.Character[nChar].endtalkpattern = 0;
	GLOBALS.Character[nChar].standpattern = 0;

	GLOBALS.IsMChar[nChar] = false;
}

DECLARE_CUSTOM_FUNCTION(CharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 16);
	GLOBALS.Character[nChar].r = r;
	GLOBALS.Character[nChar].g = g;
	GLOBALS.Character[nChar].b = b;
}

DECLARE_CUSTOM_FUNCTION(CharSetTalkPattern)(CORO_PARAM, uint32 nChar, uint32 tp, uint32 sp, uint32) {
	assert(nChar < 16);
	GLOBALS.Character[nChar].talkpattern = tp;
	GLOBALS.Character[nChar].standpattern = sp;
}

DECLARE_CUSTOM_FUNCTION(CharSetStartEndTalkPattern)(CORO_PARAM, uint32 nChar, uint32 sp, uint32 ep, uint32) {
	assert(nChar < 16);
	GLOBALS.Character[nChar].starttalkpattern = sp;
	GLOBALS.Character[nChar].endtalkpattern = ep;
}

DECLARE_CUSTOM_FUNCTION(CharSendMessage)(CORO_PARAM, uint32 nChar, uint32 dwMessage, uint32 bIsBack, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	int i;
	RMPoint pt;
	RMTextDialog *text;
	int curOffset;
	VoiceHeader *curVoc;
	FPSFX *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 16);
	_ctx->pt = GLOBALS.Character[nChar].item->CalculatePos() - RMPoint(-60, 20) - GLOBALS.Loc->ScrollPosition();

	if (GLOBALS.Character[nChar].starttalkpattern != 0) {
		GLOBALS.Freeze();
		GLOBALS.Character[nChar].item->SetPattern(GLOBALS.Character[nChar].starttalkpattern);
		GLOBALS.Unfreeze();

		CORO_INVOKE_0(GLOBALS.Character[nChar].item->WaitForEndPattern);
	}

	GLOBALS.Freeze();
	GLOBALS.Character[nChar].item->SetPattern(GLOBALS.Character[nChar].talkpattern);
	GLOBALS.Unfreeze();

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !GLOBALS.bSkipIdle; _ctx->i++) {
		if (bIsBack) {
			GLOBALS.curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS.Loc);
			if (GLOBALS.bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS.curBackText->Hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetInput(GLOBALS.Input);

		// Skipping
		_ctx->text->SetSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->SetAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Colour
		_ctx->text->SetColor(GLOBALS.Character[nChar].r, GLOBALS.Character[nChar].g, GLOBALS.Character[nChar].b);

		// Write the text
		_ctx->text->WriteText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text->SetPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS.bAlwaysDisplay) {
			_ctx->text->SetAlwaysDisplay();
			_ctx->text->ForceTime();
		}

		// Record the text
		GLOBALS.LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_vm->_theSound.CreateSfx(&_ctx->voice);
			_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
			_ctx->voice->SetLoop(false);
			if (bIsBack) _ctx->voice->SetVolume(55);
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			_ctx->curOffset = _vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->SetCustomSkipHandle(GLOBALS.hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice = NULL;
		}


		GLOBALS.curBackText = NULL;
		delete _ctx->text;
	}

	if (GLOBALS.Character[nChar].endtalkpattern != 0) {
		GLOBALS.Freeze();
		GLOBALS.Character[nChar].item->SetPattern(GLOBALS.Character[nChar].endtalkpattern);
		GLOBALS.Unfreeze();
		CORO_INVOKE_0(GLOBALS.Character[nChar].item->WaitForEndPattern);
	}

	GLOBALS.Freeze();
	GLOBALS.Character[nChar].item->SetPattern(GLOBALS.Character[nChar].standpattern);
	GLOBALS.Unfreeze();
	delete _ctx->msg;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(AddInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS.Inventory->AddItem(dwCode);
}

DECLARE_CUSTOM_FUNCTION(RemoveInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS.Inventory->RemoveItem(dwCode);
}

DECLARE_CUSTOM_FUNCTION(ChangeInventoryStatus)(CORO_PARAM, uint32 dwCode, uint32 dwStatus, uint32, uint32) {
	GLOBALS.Inventory->ChangeItemStatus(dwCode, dwStatus);
}


/*
 * Master Characters
 */

DECLARE_CUSTOM_FUNCTION(MCharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 10);
	GLOBALS.MCharacter[nChar].code = nCode;
	if (nCode == 0)
		GLOBALS.MCharacter[nChar].item = NULL;
	else
		GLOBALS.MCharacter[nChar].item = GLOBALS.Loc->GetItemFromCode(nCode);
	GLOBALS.MCharacter[nChar].r = 255;
	GLOBALS.MCharacter[nChar].g = 255;
	GLOBALS.MCharacter[nChar].b = 255;
	GLOBALS.MCharacter[nChar].x = -1;
	GLOBALS.MCharacter[nChar].y = -1;
	GLOBALS.MCharacter[nChar].bAlwaysBack = 0;

	for (int i = 0; i < 10; i++)
		GLOBALS.MCharacter[nChar].numtalks[i] = 1;

	GLOBALS.MCharacter[nChar].curgroup = 0;

	GLOBALS.IsMChar[nChar] = true;
}

DECLARE_CUSTOM_FUNCTION(MCharResetCode)(CORO_PARAM, uint32 nChar, uint32, uint32, uint32) {
	GLOBALS.MCharacter[nChar].item = GLOBALS.Loc->GetItemFromCode(GLOBALS.MCharacter[nChar].code);
}


DECLARE_CUSTOM_FUNCTION(MCharSetPosition)(CORO_PARAM, uint32 nChar, uint32 nX, uint32 nY, uint32) {
	assert(nChar < 10);
	GLOBALS.MCharacter[nChar].x = nX;
	GLOBALS.MCharacter[nChar].y = nY;
}


DECLARE_CUSTOM_FUNCTION(MCharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 10);
	GLOBALS.MCharacter[nChar].r = r;
	GLOBALS.MCharacter[nChar].g = g;
	GLOBALS.MCharacter[nChar].b = b;
}


DECLARE_CUSTOM_FUNCTION(MCharSetNumTalksInGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32 nTalks, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS.MCharacter[nChar].numtalks[nGroup] = nTalks;
}


DECLARE_CUSTOM_FUNCTION(MCharSetCurrentGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS.MCharacter[nChar].curgroup = nGroup;
}

DECLARE_CUSTOM_FUNCTION(MCharSetNumTexts)(CORO_PARAM, uint32 nChar, uint32 nTexts, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS.MCharacter[nChar].numtexts = nTexts - 1;
	GLOBALS.MCharacter[nChar].bInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(MCharSetAlwaysBack)(CORO_PARAM, uint32 nChar, uint32 bAlwaysBack, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS.MCharacter[nChar].bAlwaysBack = bAlwaysBack;
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
	FPSFX *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 10);

	bIsBack |= GLOBALS.MCharacter[nChar].bAlwaysBack ? 1 : 0;

	// Calculates the position of the text according to the current frame
	if (GLOBALS.MCharacter[nChar].x == -1)
		_ctx->pt = GLOBALS.MCharacter[nChar].item->CalculatePos() - RMPoint(-60, 20) - GLOBALS.Loc->ScrollPosition();
	else
		_ctx->pt = RMPoint(GLOBALS.MCharacter[nChar].x, GLOBALS.MCharacter[nChar].y);

	// Parameter for special actions: random between the spoken
	_ctx->parm = (GLOBALS.MCharacter[nChar].curgroup * 10) + _vm->_randomSource.getRandomNumber(
	                 GLOBALS.MCharacter[nChar].numtalks[GLOBALS.MCharacter[nChar].curgroup] - 1) + 1;

	// Try to run the custom function to initialise the speech
	if (GLOBALS.MCharacter[nChar].item) {
		_ctx->h = mpalQueryDoAction(30, GLOBALS.MCharacter[nChar].item->MpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
		}
	}

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		// fseek(_vm->m_vdbFP, curVoc->offset, SEEK_SET);
		_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !GLOBALS.bSkipIdle; _ctx->i++) {
		// Create a different object depending on whether it's background or not
		if (bIsBack) {
			GLOBALS.curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS.Loc);
			if (GLOBALS.bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS.curBackText->Hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetInput(GLOBALS.Input);

		// Skipping
		_ctx->text->SetSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->SetAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Colour
		_ctx->text->SetColor(GLOBALS.MCharacter[nChar].r, GLOBALS.MCharacter[nChar].g, GLOBALS.MCharacter[nChar].b);

		// Write the text
		_ctx->text->WriteText((*_ctx->msg)[_ctx->i], nFont);

		// Set the position
		_ctx->text->SetPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS.bAlwaysDisplay) {
			_ctx->text->SetAlwaysDisplay();
			_ctx->text->ForceTime();
		}

		// Record the text
		GLOBALS.LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_vm->_theSound.CreateSfx(&_ctx->voice);
			_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
			_ctx->voice->SetLoop(false);
			if (bIsBack) _ctx->voice->SetVolume(55);
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			_ctx->curOffset = _vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->SetCustomSkipHandle(GLOBALS.hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice = NULL;
		}

		GLOBALS.curBackText = NULL;
		delete _ctx->text;
		delete _ctx->msg;
	}


	// Try to run the custom function to close the speech
	if (GLOBALS.MCharacter[nChar].item) {
		_ctx->h = mpalQueryDoAction(31, GLOBALS.MCharacter[nChar].item->MpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
	}

	CORO_END_CODE;
}






/*
 *  Dialoghi
 */

int curDialog;

DECLARE_CUSTOM_FUNCTION(SendDialogMessage)(CORO_PARAM, uint32 nPers, uint32 nMsg, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	LPSTR string;
	RMTextDialog *text;
	int parm;
	uint32 h;
	bool bIsBack;
	VoiceHeader *curVoc;
	FPSFX *voice;
	RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bIsBack = false;

	// The SendDialogMessage can go in the background if it is a character
	if (nPers != 0 && GLOBALS.IsMChar[nPers] && GLOBALS.MCharacter[nPers].bAlwaysBack)
		_ctx->bIsBack = true;

	_ctx->curVoc = SearchVoiceHeader(curDialog, nMsg);
	_ctx->voice = NULL;

	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_vm->_theSound.CreateSfx(&_ctx->voice);
		_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
		_ctx->voice->SetLoop(false);
		if (_ctx->bIsBack) _ctx->voice->SetVolume(55);
	}

	_ctx->string = mpalQueryDialogPeriod(nMsg);

	if (nPers == 0) {
		_ctx->text = new RMTextDialog;
		_ctx->text->SetColor(0, 255, 0);
		_ctx->text->SetPosition(GLOBALS.Tony->Position() - RMPoint(0, 130) - GLOBALS.Loc->ScrollPosition());
		_ctx->text->WriteText(_ctx->string, 0);

		if (GLOBALS.dwTonyNumTexts > 0) {
			if (!GLOBALS.bTonyInTexts) {
				if (GLOBALS.nTonyNextTalkType != GLOBALS.Tony->TALK_NORMAL) {
					CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.nTonyNextTalkType);
					if (!GLOBALS.bStaticTalk)
						GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;
				} else
					CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.Tony->TALK_NORMAL);

				GLOBALS.bTonyInTexts = true;
			}
			GLOBALS.dwTonyNumTexts--;
		} else {
			CORO_INVOKE_1(GLOBALS.Tony->StartTalk, GLOBALS.nTonyNextTalkType);
			if (!GLOBALS.bStaticTalk)
				GLOBALS.nTonyNextTalkType = GLOBALS.Tony->TALK_NORMAL;
		}
	} else if (!GLOBALS.IsMChar[nPers]) {
		_ctx->text = new RMTextDialog;

		_ctx->pt = GLOBALS.Character[nPers].item->CalculatePos() - RMPoint(-60, 20) - GLOBALS.Loc->ScrollPosition();

		if (GLOBALS.Character[nPers].starttalkpattern != 0) {
			GLOBALS.Freeze();
			GLOBALS.Character[nPers].item->SetPattern(GLOBALS.Character[nPers].starttalkpattern);
			GLOBALS.Unfreeze();
			CORO_INVOKE_0(GLOBALS.Character[nPers].item->WaitForEndPattern);
		}

		GLOBALS.Character[nPers].item->SetPattern(GLOBALS.Character[nPers].talkpattern);

		_ctx->text->SetColor(GLOBALS.Character[nPers].r, GLOBALS.Character[nPers].g, GLOBALS.Character[nPers].b);
		_ctx->text->WriteText(_ctx->string, 0);
		_ctx->text->SetPosition(_ctx->pt);
	} else {
		if (GLOBALS.MCharacter[nPers].x == -1)
			_ctx->pt = GLOBALS.MCharacter[nPers].item->CalculatePos() - RMPoint(-60, 20) - GLOBALS.Loc->ScrollPosition();
		else
			_ctx->pt = RMPoint(GLOBALS.MCharacter[nPers].x, GLOBALS.MCharacter[nPers].y);

		// Parameter for special actions. Random between the spoken.
		_ctx->parm = (GLOBALS.MCharacter[nPers].curgroup * 10) + _vm->_randomSource.getRandomNumber(
		                 GLOBALS.MCharacter[nPers].numtalks[GLOBALS.MCharacter[nPers].curgroup] - 1) + 1;

		if (GLOBALS.MCharacter[nPers].numtexts != 0 && GLOBALS.MCharacter[nPers].bInTexts) {
			GLOBALS.MCharacter[nPers].numtexts--;
		} else {
			// Try to run the custom function to initialise the speech
			_ctx->h = mpalQueryDoAction(30, GLOBALS.MCharacter[nPers].item->MpalCode(), _ctx->parm);
			if (_ctx->h != CORO_INVALID_PID_VALUE)
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

			GLOBALS.MCharacter[nPers].curTalk = _ctx->parm;

			if (GLOBALS.MCharacter[nPers].numtexts != 0) {
				GLOBALS.MCharacter[nPers].bInTexts = true;
				GLOBALS.MCharacter[nPers].numtexts--;
			}
		}

		if (GLOBALS.MCharacter[nPers].bAlwaysBack) {
			_ctx->text = GLOBALS.curBackText = new RMTextDialogScrolling(GLOBALS.Loc);
			if (GLOBALS.bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS.curBackText->Hide);

			_ctx->bIsBack = true;
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetSkipStatus(!GLOBALS.MCharacter[nPers].bAlwaysBack);
		_ctx->text->SetColor(GLOBALS.MCharacter[nPers].r, GLOBALS.MCharacter[nPers].g, GLOBALS.MCharacter[nPers].b);
		_ctx->text->WriteText(_ctx->string, 0);
		_ctx->text->SetPosition(_ctx->pt);
	}

	if (!GLOBALS.bSkipIdle) {
		_ctx->text->SetInput(GLOBALS.Input);
		if (GLOBALS.bAlwaysDisplay) {
			_ctx->text->SetAlwaysDisplay();
			_ctx->text->ForceTime();
		}
		_ctx->text->SetAlignType(RMText::HCENTER, RMText::VBOTTOM);
		GLOBALS.LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
		}

		// Wait for the end of display
		_ctx->text->SetCustomSkipHandle(GLOBALS.hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);
	}

	if (_ctx->curVoc) {
		_ctx->voice->Stop();
		_ctx->voice->Release();
		_ctx->voice = NULL;
	}

	if (nPers != 0) {
		if (!GLOBALS.IsMChar[nPers]) {
			if (GLOBALS.Character[nPers].endtalkpattern != 0) {
				GLOBALS.Freeze();
				GLOBALS.Character[nPers].item->SetPattern(GLOBALS.Character[nPers].endtalkpattern);
				GLOBALS.Unfreeze();
				CORO_INVOKE_0(GLOBALS.Character[nPers].item->WaitForEndPattern);
			}

			GLOBALS.Character[nPers].item->SetPattern(GLOBALS.Character[nPers].standpattern);
			delete _ctx->text;
		} else {
			if ((GLOBALS.MCharacter[nPers].bInTexts && GLOBALS.MCharacter[nPers].numtexts == 0) || !GLOBALS.MCharacter[nPers].bInTexts) {
				// Try to run the custom function to close the speech
				GLOBALS.MCharacter[nPers].curTalk = (GLOBALS.MCharacter[nPers].curTalk % 10) + GLOBALS.MCharacter[nPers].curgroup * 10;
				_ctx->h = mpalQueryDoAction(31, GLOBALS.MCharacter[nPers].item->MpalCode(), GLOBALS.MCharacter[nPers].curTalk);
				if (_ctx->h != CORO_INVALID_PID_VALUE)
					CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

				GLOBALS.MCharacter[nPers].bInTexts = false;
				GLOBALS.MCharacter[nPers].numtexts = 0;
			}

			GLOBALS.curBackText = NULL;
			delete _ctx->text;
		}
	} else {
		if ((GLOBALS.dwTonyNumTexts == 0 && GLOBALS.bTonyInTexts) || !GLOBALS.bTonyInTexts) {
			CORO_INVOKE_0(GLOBALS.Tony->EndTalk);
			GLOBALS.dwTonyNumTexts = 0;
			GLOBALS.bTonyInTexts = false;
		}

		delete _ctx->text;
	}

	GlobalFree(_ctx->string);

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

	curDialog = nDialog;

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
			GlobalFree(_ctx->sl);

			// Wait for the next choice to be made
			mpalQueryDialogWaitForChoice(&_ctx->nChoice);
			continue;
		}

		// Making a choice for dialog
		_ctx->dc.Init();
		_ctx->dc.SetNumChoices(_ctx->num);

		// Writeall the possible options
		for (_ctx->i = 0; _ctx->i < _ctx->num; _ctx->i++) {
			_ctx->string = mpalQueryDialogPeriod(_ctx->sl[_ctx->i]);
			assert(_ctx->string != NULL);
			_ctx->dc.AddChoice(_ctx->string);
			GlobalFree(_ctx->string);
		}

		// Activate the object
		GLOBALS.LinkGraphicTask(&_ctx->dc);
		CORO_INVOKE_0(_ctx->dc.Show);

		// Draw the pointer
		GLOBALS.Pointer->SetSpecialPointer(GLOBALS.Pointer->PTR_NONE);
		MainShowMouse();

		while (!(GLOBALS.Input->mouseLeftClicked() && ((_ctx->sel = _ctx->dc.GetSelection()) != -1))) {
			CORO_INVOKE_0(GLOBALS.WaitFrame);
			GLOBALS.Freeze();
			CORO_INVOKE_1(_ctx->dc.DoFrame, GLOBALS.Input->mousePos());
			GLOBALS.Unfreeze();
		}

		// Hide the pointer
		MainHideMouse();

		CORO_INVOKE_0(_ctx->dc.Hide);
		mpalQueryDialogSelectionDWORD(_ctx->nChoice, _ctx->sl[_ctx->sel]);

		// Closes the choice
		_ctx->dc.Close();

		GlobalFree(_ctx->sl);

		// Wait for the next choice to be made
		mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	}

	CORO_END_CODE;
}


/*
 *  Sync between idle and mpal
 */

DECLARE_CUSTOM_FUNCTION(TakeOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	CoroScheduler.waitForSingleObject(coroParam, GLOBALS.mut[num], CORO_INFINITE);
}

DECLARE_CUSTOM_FUNCTION(ReleaseOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	CoroScheduler.pulseEvent(GLOBALS.mut[num]);
	warning("TODO: Validate that the use of events in TakeOwnership/ReleaseOwnership match original");
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
		_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}
	_vm->setMusicVolume(nChannel, 64);

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

	_ctx->startVolume = _vm->getMusicVolume(nChannel);

	debug("Start FadeOut Music");

	for (_ctx->i = 16; _ctx->i > 0 && !GLOBALS.bFadeOutStop; _ctx->i--) {
		if (_ctx->i * 4 < _ctx->startVolume)
			_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}

	if (!GLOBALS.bFadeOutStop)
		_vm->setMusicVolume(nChannel, 0);

	// If there is a stacchetto, stop all
	if (nChannel == 2)
		_vm->stopMusic(2);

	debug("End FadeOut Music");

	CORO_KILL_SELF();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(FadeInSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CoroScheduler.createProcess(ThreadFadeInMusic, &GLOBALS.curSonoriz, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bFadeOutStop = false;
	CoroScheduler.createProcess(ThreadFadeOutMusic, &GLOBALS.curSonoriz, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bFadeOutStop = false;
	int channel = 2;
	CoroScheduler.createProcess(ThreadFadeOutMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeInStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	int channel = 2;
	CoroScheduler.createProcess(ThreadFadeInMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(StopSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->stopMusic(GLOBALS.curSonoriz);
}

DECLARE_CUSTOM_FUNCTION(StopStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->stopMusic(2);
}

DECLARE_CUSTOM_FUNCTION(MuteSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->setMusicVolume(GLOBALS.curSonoriz, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bFadeOutStop = true;
	_vm->setMusicVolume(GLOBALS.curSonoriz, 64);
}

DECLARE_CUSTOM_FUNCTION(MuteStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->setMusicVolume(2, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->setMusicVolume(2, 64);
}





void CustPlayMusic(uint32 nChannel, const char *mFN, uint32 nFX, bool bLoop, int nSync = 0) {
	if (nSync == 0)
		nSync = 2000;
	debug("Start CustPlayMusic");
	GLOBALS.PlayMusic(nChannel, mFN, nFX, bLoop, nSync);
	debug("End CustPlayMusic");
}

DECLARE_CUSTOM_FUNCTION(PlaySonoriz)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bNoLoop, uint32) {
	if (nFX == 0 || nFX == 1 || nFX == 2) {
		debug("PlaySonoriz stop fadeout");
		GLOBALS.bFadeOutStop = true;
	}

	GLOBALS.lastMusic = nMusic;
	CustPlayMusic(GLOBALS.curSonoriz, musicFiles[nMusic].name, nFX, bNoLoop ? false : true, musicFiles[nMusic].sync);
}

DECLARE_CUSTOM_FUNCTION(PlayStacchetto)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bLoop, uint32) {
	CustPlayMusic(2, staccFileNames[nMusic], nFX, bLoop);
}

DECLARE_CUSTOM_FUNCTION(PlayItemSfx)(CORO_PARAM, uint32 nItem, uint32 nSFX, uint32, uint32) {
	if (nItem == 0) {
		GLOBALS.Tony->PlaySfx(nSFX);
	} else {
		RMItem *item = GLOBALS.Loc->GetItemFromCode(nItem);
		if (item)
			item->PlaySfx(nSFX);
	}
}


void RestoreMusic(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(PlaySonoriz, GLOBALS.lastMusic, 0, 0, 0);

	if (GLOBALS.lastTappeto != 0)
		CustPlayMusic(4, tappetiFile[GLOBALS.lastTappeto], 0, true);

	CORO_END_CODE;
}

void SaveMusic(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS.lastMusic);
	f->writeByte(GLOBALS.lastTappeto);
}

void LoadMusic(Common::InSaveFile *f) {
	GLOBALS.lastMusic = f->readByte();
	GLOBALS.lastTappeto = f->readByte();
}


DECLARE_CUSTOM_FUNCTION(StacchettoFadeStart)(CORO_PARAM, uint32 nStacc, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(FadeOutSonoriz, 0, 0, 0, 0);
	CORO_INVOKE_4(MuteStacchetto, 0, 0, 0, 0);
	CORO_INVOKE_4(PlayStacchetto, nStacc, 0, bLoop, 0);
	CORO_INVOKE_4(FadeInStacchetto, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(StacchettoFadeEnd)(CORO_PARAM, uint32 nStacc, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(FadeOutStacchetto, 0, 0, 0, 0);
	CORO_INVOKE_4(FadeInSonoriz, 0, 0, 0, 0);

	CORO_END_CODE;
}




DECLARE_CUSTOM_FUNCTION(MustSkipIdleStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bSkipIdle = true;
	CoroScheduler.setEvent(GLOBALS.hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(MustSkipIdleEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.bSkipIdle = false;
	CoroScheduler.resetEvent(GLOBALS.hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(PatIrqFreeze)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	GLOBALS.bPatIrqFreeze = bStatus;
}

DECLARE_CUSTOM_FUNCTION(OpenInitLoadMenu)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();
	CORO_INVOKE_0(_vm->openInitLoadMenu);
	GLOBALS.Unfreeze();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(OpenInitOptions)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS.Freeze();
	CORO_INVOKE_0(_vm->openInitOptions);
	GLOBALS.Unfreeze();

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

	_ctx->text = new RMTextDialog[_ctx->msg->NumPeriods()];

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods(); _ctx->i++)     {
		_ctx->text[_ctx->i].SetInput(GLOBALS.Input);

		// Alignment
		if ((*_ctx->msg)[_ctx->i][0] == '@') {
			_ctx->text[_ctx->i].SetAlignType(RMText::HCENTER, RMText::VTOP);
			_ctx->text[_ctx->i].WriteText(&(*_ctx->msg)[_ctx->i][1], 3);
			_ctx->text[_ctx->i].SetPosition(RMPoint(414, 70 + _ctx->i * 26));  // 70
		} else {
			_ctx->text[_ctx->i].SetAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->text[_ctx->i].WriteText((*_ctx->msg)[_ctx->i], 3);
			_ctx->text[_ctx->i].SetPosition(RMPoint(260, 70 + _ctx->i * 26));
		}


		// Set the position
		_ctx->text[_ctx->i].SetAlwaysDisplay();
		_ctx->text[_ctx->i].SetForcedTime(dwTime * 1000);
		_ctx->text[_ctx->i].SetNoTab();

		// Wait for the end of display
		_ctx->text[_ctx->i].SetCustomSkipHandle(_ctx->hDisable);

		// Record the text
		GLOBALS.LinkGraphicTask(&_ctx->text[_ctx->i]);
	}

	_ctx->startTime = _vm->getTime();

	while (_ctx->startTime + dwTime * 1000 > _vm->getTime()) {
		CORO_INVOKE_0(GLOBALS.WaitFrame);
		if (GLOBALS.Input->mouseLeftClicked() || GLOBALS.Input->mouseRightClicked())
			break;
		if (_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
			break;
	}

	CoroScheduler.setEvent(_ctx->hDisable);

	CORO_INVOKE_0(GLOBALS.WaitFrame);
	CORO_INVOKE_0(GLOBALS.WaitFrame);

	delete[] _ctx->text;
	delete _ctx->msg;
	_ctx->text = NULL;
	_ctx->msg = NULL;

	CORO_END_CODE;
}



BEGIN_CUSTOM_FUNCTION_MAP()

ASSIGN(1,   CustLoadLocation)
ASSIGN(2,       MySleep)
ASSIGN(3,   SetPointer)
ASSIGN(5,   MoveTony)
ASSIGN(6,       FaceToMe)
ASSIGN(7,       BackToMe)
ASSIGN(8,       LeftToMe)
ASSIGN(9,       RightToMe)
ASSIGN(10,   SendTonyMessage)
ASSIGN(11,      ChangeBoxStatus)
ASSIGN(12,      ChangeLocation)
ASSIGN(13,      DisableTony)
ASSIGN(14,      EnableTony)
ASSIGN(15,      WaitForPatternEnd)
ASSIGN(16,   SetLocStartPosition)
ASSIGN(17,   ScrollLocation)
ASSIGN(18,   MoveTonyAndWait)
ASSIGN(19,      ChangeHotspot)
ASSIGN(20,   AddInventory)
ASSIGN(21,   RemoveInventory)
ASSIGN(22,      ChangeInventoryStatus)
ASSIGN(23,      SetTonyPosition)
ASSIGN(24,      SendFullscreenMessage)
ASSIGN(25,      SaveTonyPosition)
ASSIGN(26,      RestoreTonyPosition)
ASSIGN(27,      DisableInput)
ASSIGN(28,      EnableInput)
ASSIGN(29,      StopTony)

ASSIGN(30,      TonyTakeUp1)
ASSIGN(31,      TonyTakeMid1)
ASSIGN(32,      TonyTakeDown1)
ASSIGN(33,      TonyTakeUp2)
ASSIGN(34,      TonyTakeMid2)
ASSIGN(35,      TonyTakeDown2)

ASSIGN(72,      TonyPutUp1)
ASSIGN(73,      TonyPutMid1)
ASSIGN(74,      TonyPutDown1)
ASSIGN(75,      TonyPutUp2)
ASSIGN(76,      TonyPutMid2)
ASSIGN(77,      TonyPutDown2)

ASSIGN(36,   TonyPerTerra)
ASSIGN(37,   TonySiRialza)
ASSIGN(38,      TonyPastorella)
ASSIGN(39,      TonyFischietto)

ASSIGN(40,   TonyRide)
ASSIGN(41,   TonyFianchi)
ASSIGN(42,   TonyCanta)
ASSIGN(43,   TonySiIndica)
ASSIGN(44,      TonySpaventatoConMani)
ASSIGN(49,    TonySpaventatoSenzaMani)
ASSIGN(45,      TonyConBicchiere)
ASSIGN(46,      TonyConVerme)
ASSIGN(47,      TonyConMartello)
ASSIGN(48,      TonyConCorda)
ASSIGN(90,      TonyConConiglioANIM)
ASSIGN(91,      TonyConRicettaANIM)
ASSIGN(92,      TonyConCarteANIM)
ASSIGN(93,      TonyConPupazzoANIM)
ASSIGN(94,      TonyConPupazzoStart)
ASSIGN(95,      TonyConPupazzoEnd)
ASSIGN(96,      TonyConConiglioStart)
ASSIGN(97,      TonyConConiglioEnd)
ASSIGN(98,      TonyConRicettaStart)
ASSIGN(99,      TonyConRicettaEnd)
ASSIGN(100,     TonyConCarteStart)
ASSIGN(101,     TonyConCarteEnd)
ASSIGN(102,   TonyConTaccuinoStart)
ASSIGN(103,   TonyConTaccuinoEnd)
ASSIGN(104,   TonyConMegafonoStart)
ASSIGN(105,   TonyConMegafonoEnd)
ASSIGN(106,   TonyConBarbaStart)
ASSIGN(107,   TonyConBarbaEnd)
ASSIGN(108,   TonyRidacchia)
ASSIGN(109,   TonySchifato)
ASSIGN(110,   TonyNaah)
ASSIGN(111,   TonyMacbeth)
ASSIGN(112,   TonySniffaLeft)
ASSIGN(113,   TonySniffaRight)
ASSIGN(114,   TonySpaventatoStart)
ASSIGN(115,   TonySpaventatoEnd)
ASSIGN(116,   TonyConSegretaria)

ASSIGN(50,   CharSetCode)
ASSIGN(51,      CharSetColor)
ASSIGN(52,   CharSetTalkPattern)
ASSIGN(53,   CharSendMessage)
ASSIGN(54,   CharSetStartEndTalkPattern)

ASSIGN(60,   MCharSetCode)
ASSIGN(61,   MCharSetColor)
ASSIGN(62,    MCharSetCurrentGroup)
ASSIGN(63,      MCharSetNumTalksInGroup)
ASSIGN(64,      MCharSetNumTexts)
ASSIGN(65,      MCharSendMessage)
ASSIGN(66,      MCharSetPosition)
ASSIGN(67,      MCharSetAlwaysBack)
ASSIGN(68,   MCharResetCode)

ASSIGN(70,      StartDialog)
ASSIGN(71,      SendDialogMessage)

ASSIGN(80,      TakeOwnership)
ASSIGN(81,      ReleaseOwnership)

ASSIGN(86,      PlaySonoriz)
ASSIGN(87,      PlayStacchetto)
ASSIGN(88,      FadeInSonoriz)
ASSIGN(89,      FadeOutSonoriz)
ASSIGN(123,     FadeInStacchetto)
ASSIGN(124,     FadeOutStacchetto)
ASSIGN(125,     MuteSonoriz)
ASSIGN(126,     DemuteSonoriz)
ASSIGN(127,     MuteStacchetto)
ASSIGN(128,     DemuteStacchetto)
ASSIGN(84,      StopSonoriz)
ASSIGN(85,      StopStacchetto)
ASSIGN(83,      PlayItemSfx)
ASSIGN(129,     StacchettoFadeStart)
ASSIGN(130,     StacchettoFadeEnd)

ASSIGN(120,     TremaSchermo)
ASSIGN(121,     AutoSave)
ASSIGN(122,     AbortGame)
ASSIGN(131,     NoOcchioDiBue)
ASSIGN(132,     SendFullscreenMsgStart)
ASSIGN(133,     SendFullscreenMsgEnd)
ASSIGN(134,     CustEnableGUI)
ASSIGN(135,     CustDisableGUI)
ASSIGN(136,     ClearScreen)
ASSIGN(137,     PatIrqFreeze)
ASSIGN(138,     TonySetPalesati)
ASSIGN(139,     OpenInitLoadMenu)
ASSIGN(140,     OpenInitOptions)
ASSIGN(141,     SyncScrollLocation)
ASSIGN(142,     CloseLocation)
ASSIGN(143,     SetAlwaysDisplay)
ASSIGN(144,     DoCredits)

ASSIGN(200, MustSkipIdleStart);
ASSIGN(201, MustSkipIdleEnd);

END_CUSTOM_FUNCTION_MAP()

void setupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input) {
	GLOBALS.Tony = tony;
	GLOBALS.Pointer = ptr;
	GLOBALS.Boxes = box;
	GLOBALS.Loc = loc;
	GLOBALS.Inventory = inv;
	GLOBALS.Input = input;

	GLOBALS.LoadLocation = MainLoadLocation;
	GLOBALS.UnloadLocation = MainUnloadLocation;
	GLOBALS.LinkGraphicTask = MainLinkGraphicTask;
	GLOBALS.Freeze = MainFreeze;
	GLOBALS.Unfreeze = MainUnfreeze;
	GLOBALS.WaitFrame = MainWaitFrame;
	GLOBALS.PlayMusic = MainPlayMusic;
	GLOBALS.InitWipe = MainInitWipe;
	GLOBALS.CloseWipe = MainCloseWipe;
	GLOBALS.WaitWipeEnd = MainWaitWipeEnd;
	GLOBALS.DisableGUI = MainDisableGUI;
	GLOBALS.EnableGUI = MainEnableGUI;
	GLOBALS.SetPalesati = MainSetPalesati;

	GLOBALS.bAlwaysDisplay = false;
	int i;

	for (i = 0; i < 10; i++)
		GLOBALS.mut[i] = CoroScheduler.createEvent(false, false);

	for (i = 0; i < 200; i++)
		GLOBALS.tappeti[i] = 0;

	GLOBALS.tappeti[6] = T_GRILLI;
	GLOBALS.tappeti[7] = T_GRILLI;
	GLOBALS.tappeti[8] = T_GRILLIOV;
	GLOBALS.tappeti[10] = T_GRILLI;
	GLOBALS.tappeti[12] = T_GRILLI;
	GLOBALS.tappeti[13] = T_GRILLIOV;
	GLOBALS.tappeti[15] = T_GRILLI;
	GLOBALS.tappeti[16] = T_GRILLIVENTO;
	GLOBALS.tappeti[18] = T_GRILLI;
	GLOBALS.tappeti[19] = T_GRILLIVENTO;
	GLOBALS.tappeti[20] = T_GRILLI;
	GLOBALS.tappeti[23] = T_GRILLI;
	GLOBALS.tappeti[26] = T_MAREMETA;
	GLOBALS.tappeti[27] = T_GRILLI;
	GLOBALS.tappeti[28] = T_GRILLIVENTO;
	GLOBALS.tappeti[31] = T_GRILLI;
	GLOBALS.tappeti[33] = T_MARE;
	GLOBALS.tappeti[35] = T_MARE;
	GLOBALS.tappeti[36] = T_GRILLI;
	GLOBALS.tappeti[37] = T_GRILLI;
	GLOBALS.tappeti[40] = T_GRILLI;
	GLOBALS.tappeti[41] = T_GRILLI;
	GLOBALS.tappeti[42] = T_GRILLI;
	GLOBALS.tappeti[45] = T_GRILLI;
	GLOBALS.tappeti[51] = T_GRILLI;
	GLOBALS.tappeti[52] = T_GRILLIVENTO1;
	GLOBALS.tappeti[53] = T_GRILLI;
	GLOBALS.tappeti[54] = T_GRILLI;
	GLOBALS.tappeti[57] = T_VENTO;
	GLOBALS.tappeti[58] = T_VENTO;
	GLOBALS.tappeti[60] = T_VENTO;



	// Create an event for the idle skipping
	GLOBALS.hSkipIdle = CoroScheduler.createEvent(true, false);
}

} // end of namespace Tony
