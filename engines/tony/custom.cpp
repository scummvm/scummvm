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
/**************************************************************************
 *                                     ออออออออออออออออออออออออออออออออออ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ออออออออออออออออออออออออออออออออออ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Custom.CPP...........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c  Desc:    Custom functions.....  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c          .....................  *
 *      "#$e z$*"   .  `.   ^*Nc e$""              .....................  *
 *         "$$".  .r"   ^4.  .^$$"                 .....................  *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************
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
#include "tony/sched.h"
#include "tony/tony.h"
#include "tony/tonychar.h"
#include "tony/utils.h"

namespace Tony {

extern bool bPatIrqFreeze;

RMTony *Tony;
RMPointer *Pointer;
RMGameBoxes *Boxes;
RMLocation *Loc;
RMInventory *Inventory;
RMInput *Input;

uint32 (*LoadLocation)(int, RMPoint, RMPoint start);
void (*UnloadLocation)(CORO_PARAM, bool bDoOnExit, uint32 *result);
void (*LinkGraphicTask)(RMGfxTask *task);
void (*Freeze)(void); 
void (*Unfreeze)(void); 
void (*WaitFrame)(CORO_PARAM); 
void (*PlayMusic)(int nChannel, const char *fileName, int nFX, bool bLoop, int nSync);
void (*WaitWipeEnd)(CORO_PARAM); 
void (*CloseWipe)(void); 
void (*InitWipe)(int type); 
void (*EnableGUI)(void); 
void (*DisableGUI)(void); 
void (*SetPalesati)(bool bpal);

uint32 dwTonyNumTexts = 0;
bool bTonyInTexts = false;
bool bStaticTalk = false;
RMTony::TALKTYPE nTonyNextTalkType;

RMPoint StartLocPos[256];
OSystem::MutexRef cs[10];
OSystem::MutexRef vdb;
HANDLE mut[10];

bool bSkipIdle = false;
uint32 hSkipIdle;

int lastMusic = 0, lastTappeto = 0;

int tappeti[200];

#define T_GRILLI 1
#define T_GRILLIOV 2
#define T_GRILLIVENTO 3
#define T_GRILLIVENTO1 4
#define T_VENTO 5
#define T_MARE 6
#define T_MAREMETA 7

const char *tappetiFile[] =  {
	"None",
	"1.ADP",	// Grilli.WAV
	"2.ADP",	// Grilli-Ovattati.WAV
	"3.ADP",	// Grilli-Vento.WAV
	"3.ADP",	// Grilli-Vento1.WAV
	"5.ADP",	// Vento1.WAV
	"4.ADP",	// Mare1.WAV
	"6.ADP"		// Mare1.WAV metเ volume
};

struct CharacterStruct {
	uint32 code;
	RMItem *item;
	byte r, g, b;
	int talkpattern;
	int standpattern;
	int starttalkpattern, endtalkpattern;
	int numtexts;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(code);
		f->writeUint32LE(0);
		f->writeByte(r);
		f->writeByte(g);
		f->writeByte(b);
		f->writeUint32LE(talkpattern);
		f->writeUint32LE(standpattern);
		f->writeUint32LE(starttalkpattern);
		f->writeUint32LE(endtalkpattern);
		f->writeUint32LE(numtexts);
	}
	void Load(Common::InSaveFile *f) {
		code = f->readUint32LE();
		f->readUint32LE();
		item = NULL;
		r = f->readByte();
		g = f->readByte();
		b = f->readByte();
		talkpattern = f->readUint32LE();
		standpattern = f->readUint32LE();
		starttalkpattern = f->readUint32LE();
		endtalkpattern = f->readUint32LE();
		numtexts = f->readUint32LE();
	}
};
CharacterStruct Character[16];

struct MCharacterStruct {
	uint32 code;
	RMItem *item;
	byte r, g, b;
	int x, y;
	int numtalks[10];
	int curgroup;
	int numtexts;
	bool bInTexts;
	int curTalk;
	bool bAlwaysBack;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(code);
		f->writeUint32LE(0);
		f->writeByte(r);
		f->writeByte(g);
		f->writeByte(b);
		f->writeUint32LE(x);
		f->writeUint32LE(y);
		for (int i = 0; i < 10; ++i)
			f->writeUint32LE(numtalks[i]);
		f->writeUint32LE(curgroup);
		f->writeUint32LE(numtexts);
		f->writeByte(bInTexts);
		f->writeUint32LE(curTalk);
		f->writeByte(bAlwaysBack);
	}
	void Load(Common::InSaveFile *f) {
		code = f->readUint32LE();
		f->readUint32LE();
		item = NULL;
		r = f->readByte();
		g = f->readByte();
		b = f->readByte();
		x = f->readUint32LE();
		y = f->readUint32LE();
		for (int i = 0; i < 10; ++i)
			numtalks[i] = f->readUint32LE();
		curgroup = f->readUint32LE();
		numtexts = f->readUint32LE();
		bInTexts = f->readByte();
		curTalk = f->readUint32LE();
		bAlwaysBack = f->readByte();
	}
};
MCharacterStruct MCharacter[10];

bool IsMChar[16];

bool bAlwaysDisplay;

RMPoint saveTonyPos;
int saveTonyLoc;

RMTextDialog *curBackText = NULL;
bool bTonyIsSpeaking = false;

int curChangedHotspot = 0;

struct ChangedHotspotStruct {
	uint32 dwCode;
	uint32 nX, nY;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(dwCode);
		f->writeUint32LE(nX);
		f->writeUint32LE(nY);
	}
	void Load(Common::InSaveFile *f) {
		dwCode = f->readUint32LE();
		nX = f->readUint32LE();
		nY = f->readUint32LE();
	}
};
ChangedHotspotStruct ChangedHotspot[256];

void ReapplyChangedHotspot(void) {
	int i;
	for (i = 0; i<curChangedHotspot; i++)
		Loc->GetItemFromCode(ChangedHotspot[i].dwCode)->ChangeHotspot(RMPoint(ChangedHotspot[i].nX, ChangedHotspot[i].nY));	
}

void SaveChangedHotspot(Common::OutSaveFile *f) {
	f->writeByte(curChangedHotspot);
	if (curChangedHotspot > 0) {
		for (int i = 0; i < curChangedHotspot; ++i)
			ChangedHotspot[i].Save(f);
	}
}

void LoadChangedHotspot(Common::InSaveFile *f) {
	curChangedHotspot = f->readByte();
	
	if (curChangedHotspot > 0) {
		for (int i = 0; i < curChangedHotspot; ++i)
			ChangedHotspot[i].Load(f);
	}
}


/*
	Classi richieste per le funzioni custom!

	Tony (per muoverlo)		-> si puo' fare tramite MPAL l'animazione? Penso proprio di s์

	SendMessage -> Direi che basta theEngine.SendMessage()
	ChangeLocation -> theEngine.ChangeLocation()
	AddInventory -> theEngine.AddInventory()
*/

void MCharResetCodes(void) {
	for (int i = 0; i < 10; i++)
		MCharacter[i].item = Loc->GetItemFromCode(MCharacter[i].code);
	for (int i = 0; i < 10; i++)
		Character[i].item = Loc->GetItemFromCode(Character[i].code);
}

void CharsSaveAll(Common::OutSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		f->writeByte(IsMChar[i]);
		if (IsMChar[i]) {
			MCharacter[i].Save(f);
		} else {
			Character[i].Save(f);
		}
	}
}

void CharsLoadAll(Common::InSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		IsMChar[i] = f->readByte();
		if (IsMChar[i])
			MCharacter[i].Load(f);
		else
			Character[i].Load(f);
	}
}

DECLARE_CUSTOM_FUNCTION(FaceToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	Tony->SetPattern(Tony->PAT_STANDDOWN);
}

DECLARE_CUSTOM_FUNCTION(BackToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	Tony->SetPattern(Tony->PAT_STANDUP);
}

DECLARE_CUSTOM_FUNCTION(LeftToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	Tony->SetPattern(Tony->PAT_STANDLEFT);
}

DECLARE_CUSTOM_FUNCTION(RightToMe)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	Tony->SetPattern(Tony->PAT_STANDRIGHT);
}


DECLARE_CUSTOM_FUNCTION(TonySetPalesati)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	SetPalesati(bStatus);	
}

DECLARE_CUSTOM_FUNCTION(MySleep)(CORO_PARAM, uint32 dwTime, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!bSkipIdle)
		CORO_INVOKE_1(g_scheduler->sleep, dwTime);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetAlwaysDisplay)(CORO_PARAM, uint32 val, uint32, uint32, uint32) {
	bAlwaysDisplay = (val != 0);
}


DECLARE_CUSTOM_FUNCTION(SetPointer)(CORO_PARAM, uint32 dwPointer, uint32, uint32, uint32) {
	switch (dwPointer) {
		case 1:
			Pointer->SetSpecialPointer(Pointer->PTR_FRECCIASU);
			break;
		case 2:
			Pointer->SetSpecialPointer(Pointer->PTR_FRECCIAGIU);
			break;
		case 3:
			Pointer->SetSpecialPointer(Pointer->PTR_FRECCIASINISTRA);
			break;
		case 4:
			Pointer->SetSpecialPointer(Pointer->PTR_FRECCIADESTRA);
			break;
		case 5:
			Pointer->SetSpecialPointer(Pointer->PTR_FRECCIAMAPPA);
			break;

		default:
			Pointer->SetSpecialPointer(Pointer->PTR_NONE);
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
		if (_vm->_voices[i].code == code)
			return &_vm->_voices[i];

	return NULL;
}


DECLARE_CUSTOM_FUNCTION(SendTonyMessage)(CORO_PARAM, uint32 dwMessage, uint32 nX, uint32 nY, uint32) {
	CORO_BEGIN_CONTEXT;
		RMMessage *msg;
		int i;
		int curOffset;
		VoiceHeader *curVoc;
		FPSFX *voice;
		RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curOffset = 0;

	if (bSkipIdle) return;

	_ctx->msg = new RMMessage(dwMessage);
	if (!_ctx->msg->IsValid()) {
		delete _ctx->msg;
		return;
	}

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Si posiziona all'interno del database delle voci all'inizio della prima
		_ctx->curOffset = _ctx->curVoc->offset;

		// PRIMA VOLTA PREALLOCA
		g_system->lockMutex(vdb);
		//fseek(_vm->m_vdbFP, _ctx->curOffset, SEEK_SET);
		_vm->_vdbFP.seek(_ctx->curOffset);
		_vm->_theSound.CreateSfx(&_ctx->voice);
		_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
//		_ctx->curOffset = ftell(_vm->m_vdbFP);
		_ctx->curOffset = _vm->_vdbFP.pos();

		_ctx->voice->SetLoop(false);
		g_system->unlockMutex(vdb);
	}

	if (nTonyNextTalkType != Tony->TALK_NORMAL) {
		CORO_INVOKE_1(Tony->StartTalk, nTonyNextTalkType);

		if (!bStaticTalk)
			nTonyNextTalkType = Tony->TALK_NORMAL;
	} else {
		if (_ctx->msg->NumPeriods() > 1)
			CORO_INVOKE_1(Tony->StartTalk, Tony->TALK_FIANCHI);
		else
			CORO_INVOKE_1(Tony->StartTalk, Tony->TALK_NORMAL);
	}

	if (curBackText)
		CORO_INVOKE_0(curBackText->Hide);

	bTonyIsSpeaking = true;

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !bSkipIdle; _ctx->i++) {
		_ctx->text.SetInput(Input);
	
		// Allineamento
		_ctx->text.SetAlignType(RMText::HCENTER,RMText::VBOTTOM);
		
		// Colore
		_ctx->text.SetColor(0,255,0);

		// Scrive il testo
		_ctx->text.WriteText((*_ctx->msg)[_ctx->i],0);

		// Setta la posizione
		if (nX == 0 && nY == 0)
			_ctx->text.SetPosition(Tony->Position() - RMPoint(0, 130) - Loc->ScrollPosition());
		else
			_ctx->text.SetPosition(RMPoint(nX, nY) - Loc->ScrollPosition());

		// Setta l'always display
		if (bAlwaysDisplay) { _ctx->text.SetAlwaysDisplay(); _ctx->text.ForceTime(); }

		// Registra il testo
		LinkGraphicTask(&_ctx->text);

		if (_ctx->curVoc) {
			if (_ctx->i == 0) {
				_ctx->voice->Play();
				_ctx->text.SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			} else {
				g_system->lockMutex(vdb);
		//		fseek(_vm->m_vdbFP, _ctx->curOffset, SEEK_SET);
				_vm->_vdbFP.seek(_ctx->curOffset);
				_vm->_theSound.CreateSfx(&_ctx->voice);
				_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
		//		_ctx->curOffset = ftell(_vm->m_vdbFP);
				_ctx->curOffset = _vm->_vdbFP.pos();
				_ctx->voice->SetLoop(false);
				_ctx->voice->Play();
				_ctx->text.SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
				g_system->unlockMutex(vdb);
			}
		}

		// Aspetta la fine della visualizzazione	
		_ctx->text.SetCustomSkipHandle(hSkipIdle);
		CORO_INVOKE_0(_ctx->text.WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice=NULL;
		}
	}

	bTonyIsSpeaking = false;
	if (curBackText)
		curBackText->Show();

	CORO_INVOKE_0(Tony->EndTalk);
	delete _ctx->msg;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(ChangeBoxStatus)(CORO_PARAM, uint32 nLoc, uint32 nBox, uint32 nStatus, uint32) {
	Boxes->ChangeBoxStatus(nLoc,nBox,nStatus);
}


DECLARE_CUSTOM_FUNCTION(CustLoadLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
		uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();

	curChangedHotspot = 0;
	if (bUseStartPos != 0)
		LoadLocation(nLoc, RMPoint(tX, tY), StartLocPos[nLoc]);
	else
		LoadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	Unfreeze();
	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	// On Enter?
	if (_ctx->h != INVALID_PID_VALUE)
		CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);

	CORO_END_CODE;
}


RMPoint SFM_pt;
int SFM_nLoc;

DECLARE_CUSTOM_FUNCTION(SendFullscreenMsgStart)(CORO_PARAM, uint32 nMsg, uint32 nFont, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
		RMMessage *msg;
		RMGfxClearTask clear;
		int i;
		RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);
		
	SFM_nLoc = Loc->TEMPGetNumLoc();
	SFM_pt = Tony->Position();
	
	if (bSkipIdle) 
		return;

	CORO_INVOKE_2(UnloadLocation, false, NULL);
	Tony->Hide();
	Unfreeze();

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !bSkipIdle; _ctx->i++) {
		_ctx->text.SetInput(Input);

		// Allineamento
		_ctx->text.SetAlignType(RMText::HCENTER,RMText::VCENTER);
		
		// Forza il testo a scomparire a tempo		
		_ctx->text.ForceTime();

		// Colore
		_ctx->text.SetColor(255,255,255);

		// Scrive il testo
		if (nFont == 0)
			_ctx->text.WriteText((*_ctx->msg)[_ctx->i], 1);
		else if (nFont == 1)
			_ctx->text.WriteText((*_ctx->msg)[_ctx->i], 0);

		// Setta la posizione
		_ctx->text.SetPosition(RMPoint(320, 240));

		_ctx->text.SetAlwaysDisplay();
		_ctx->text.ForceTime();

		// Registra il testo
		LinkGraphicTask(&_ctx->clear);
		LinkGraphicTask(&_ctx->text);

		// Aspetta la fine della visualizzazione	
		_ctx->text.SetCustomSkipHandle(hSkipIdle);
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

	LinkGraphicTask(&_ctx->clear);
	
	CORO_INVOKE_0(WaitFrame);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SendFullscreenMsgEnd)(CORO_PARAM, uint32 bNotEnableTony, uint32, uint32, uint32) {
	Freeze();
	LoadLocation(SFM_nLoc,RMPoint(SFM_pt.x,SFM_pt.y),RMPoint(-1,-1));
	if (!bNotEnableTony)
		Tony->Show();
	Unfreeze();

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

bool bNoOcchioDiBue = false;

DECLARE_CUSTOM_FUNCTION(NoOcchioDiBue)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	bNoOcchioDiBue = true;
}

DECLARE_CUSTOM_FUNCTION(CloseLocation)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!bNoOcchioDiBue) {
	  InitWipe(1);
	  CORO_INVOKE_0(WaitWipeEnd);
	}

	_vm->StopMusic(4);

	// On Exit e lascia freezzato
	CORO_INVOKE_2(UnloadLocation, true, NULL);
	Unfreeze();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeLocation)(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
		uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!bNoOcchioDiBue) {
		InitWipe(1);
		CORO_INVOKE_0(WaitWipeEnd);
	}

	if (lastTappeto != tappeti[nLoc]) {
		_vm->StopMusic(4);
	}
		
	// On Exit e lascia freezzato
	CORO_INVOKE_2(UnloadLocation, true, NULL);

	curChangedHotspot = 0;
	if (bUseStartPos != 0)
		LoadLocation(nLoc, RMPoint(tX, tY), StartLocPos[nLoc]);
	else
		LoadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	if (lastTappeto != tappeti[nLoc]) {
		lastTappeto = tappeti[nLoc];
		if (lastTappeto != 0)
			_vm->PlayMusic(4, tappetiFile[lastTappeto], 0, true, 2000);
	}

	if (!bNoOcchioDiBue) {
  		InitWipe(2);
	}
	
	Unfreeze();


	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	if (!bNoOcchioDiBue) {
  		CORO_INVOKE_0(WaitWipeEnd);
		CloseWipe();
	}

	bNoOcchioDiBue = false;

	// On Enter?
	if (_ctx->h != INVALID_PID_VALUE)
		CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(SetLocStartPosition)(CORO_PARAM, uint32 nLoc, uint32 lX, uint32 lY, uint32) {
	StartLocPos[nLoc].Set(lX,lY);	
}

DECLARE_CUSTOM_FUNCTION(SaveTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	saveTonyPos = Tony->Position();
	saveTonyLoc = Loc->TEMPGetNumLoc();
}

DECLARE_CUSTOM_FUNCTION(RestoreTonyPosition)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(ChangeLocation, saveTonyLoc, saveTonyPos.x, saveTonyPos.y, 0);
	
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
	Tony->StopNoAction(coroParam);
}


DECLARE_CUSTOM_FUNCTION(CustEnableGUI)(CORO_PARAM, uint32, uint32, uint32, uint32) {
  EnableGUI();
}

DECLARE_CUSTOM_FUNCTION(CustDisableGUI)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	DisableGUI();
}



void TonyGenericTake1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	Tony->Take(nDirection, 0);
	Unfreeze();
	
	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericTake2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	Tony->Take(nDirection, 1);
	Unfreeze();

	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	Freeze();
	Tony->Take(nDirection,2);
	Unfreeze();

	CORO_END_CODE;
}

void TonyGenericPut1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	Tony->Put(nDirection, 0);
	Unfreeze();
	
	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	CORO_END_CODE;
}

void TonyGenericPut2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	Tony->Put(nDirection,1);
	Unfreeze();

	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	Freeze();
	Tony->Put(nDirection,2);
	Unfreeze();

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
		Tony->SetPattern(Tony->PAT_PERTERRALEFT);
	else
		Tony->SetPattern(Tony->PAT_PERTERRARIGHT);
}

DECLARE_CUSTOM_FUNCTION(TonySiRialza)(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (dwParte == 0)
		Tony->SetPattern(Tony->PAT_SIRIALZALEFT);
	else
		Tony->SetPattern(Tony->PAT_SIRIALZARIGHT);
	
	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyPastorella)(CORO_PARAM, uint32 bIsPast, uint32, uint32, uint32) {
  Tony->SetPastorella(bIsPast);
}

DECLARE_CUSTOM_FUNCTION(TonyFischietto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Tony->SetPattern(Tony->PAT_FISCHIETTORIGHT);
	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndPattern);

	Tony->SetPattern(Tony->PAT_STANDRIGHT);

	CORO_END_CODE;
}


void TonySetNumTexts(uint32 dwText) {
	dwTonyNumTexts=dwText;
	bTonyInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(TonyRide)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_RIDE;
}

DECLARE_CUSTOM_FUNCTION(TonyRidacchia)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_RIDE2;
}

DECLARE_CUSTOM_FUNCTION(TonyFianchi)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_FIANCHI;
}

DECLARE_CUSTOM_FUNCTION(TonyCanta)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CANTA;
}

DECLARE_CUSTOM_FUNCTION(TonySiIndica)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_SIINDICA;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoConMani)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_SPAVENTATO;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoSenzaMani)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_SPAVENTATO2;
}

DECLARE_CUSTOM_FUNCTION(TonyConMartello)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONMARTELLO;
	Tony->SetPattern(Tony->PAT_CONMARTELLO);
}

DECLARE_CUSTOM_FUNCTION(TonyConBicchiere)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONBICCHIERE;
	Tony->SetPattern(Tony->PAT_CONBICCHIERE);
}

DECLARE_CUSTOM_FUNCTION(TonyConVerme)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONVERME;
	Tony->SetPattern(Tony->PAT_CONVERME);
}

DECLARE_CUSTOM_FUNCTION(TonyConCorda)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONCORDA;
	Tony->SetPattern(Tony->PAT_CONCORDA);
}

DECLARE_CUSTOM_FUNCTION(TonyConSegretaria)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONSEGRETARIA;
	Tony->SetPattern(Tony->PAT_CONSEGRETARIA);
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONCONIGLIO;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONRICETTA;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONCARTE;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoANIM)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_CONPUPAZZO;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONPUPAZZOSTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONPUPAZZOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConPupazzoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONPUPAZZOSTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONCONIGLIOSTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONCONIGLIOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConConiglioEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONCONIGLIOSTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONRICETTASTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONRICETTASTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConRicettaEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONRICETTASTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONCARTESTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONCARTESTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConCarteEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONCARTESTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConTaccuinoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONTACCUINOSTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONTACCUINOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConTaccuinoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONTACCUINOSTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConMegafonoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONMEGAFONOSTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONMEGAFONOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConMegafonoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONMEGAFONOSTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConBarbaStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_CONBARBASTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_CONBARBASTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyConBarbaEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_CONBARBASTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoStart)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	nTonyNextTalkType = Tony->TALK_SPAVENTATOSTATIC;
	bStaticTalk = true;
	CORO_INVOKE_1(Tony->StartStatic, Tony->TALK_SPAVENTATOSTATIC);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySpaventatoEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->EndStatic, Tony->TALK_SPAVENTATOSTATIC);
	bStaticTalk = false;
	nTonyNextTalkType = Tony->TALK_NORMAL;

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(TonySchifato)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_SCHIFATO;
}

DECLARE_CUSTOM_FUNCTION(TonySniffaLeft)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Tony->SetPattern(Tony->PAT_SNIFFA_LEFT);
	CORO_INVOKE_0(Tony->WaitForEndPattern);
	CORO_INVOKE_4(LeftToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonySniffaRight)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Tony->SetPattern(Tony->PAT_SNIFFA_RIGHT);
	CORO_INVOKE_0(Tony->WaitForEndPattern);
	CORO_INVOKE_4(RightToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(TonyNaah)(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
  TonySetNumTexts(dwText);
	nTonyNextTalkType = Tony->TALK_NAAH;
}

DECLARE_CUSTOM_FUNCTION(TonyMacbeth)(CORO_PARAM, uint32 nPos, uint32, uint32, uint32) {
	switch (nPos) {
	case 1:
		nTonyNextTalkType = Tony->TALK_MACBETH1;
		break;
	case 2:
		nTonyNextTalkType = Tony->TALK_MACBETH2;
		break;
	case 3:
		nTonyNextTalkType = Tony->TALK_MACBETH3;
		break;
	case 4:
		nTonyNextTalkType = Tony->TALK_MACBETH4;
		break;
	case 5:
		nTonyNextTalkType = Tony->TALK_MACBETH5;
		break;
	case 6:
		nTonyNextTalkType = Tony->TALK_MACBETH6;
		break;
	case 7:
		nTonyNextTalkType = Tony->TALK_MACBETH7;
		break;
	case 8:
		nTonyNextTalkType = Tony->TALK_MACBETH8;
		break;
	case 9:
		nTonyNextTalkType = Tony->TALK_MACBETH9;
		break;
	}
}


DECLARE_CUSTOM_FUNCTION(EnableTony)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	Tony->Show();
}

DECLARE_CUSTOM_FUNCTION(DisableTony)(CORO_PARAM, uint32 bShowOmbra, uint32, uint32, uint32) {
	Tony->Hide(bShowOmbra);
}

DECLARE_CUSTOM_FUNCTION(WaitForPatternEnd)(CORO_PARAM, uint32 nItem, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
		RMItem *item;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->item = Loc->GetItemFromCode(nItem);
	
	if (!bSkipIdle && _ctx->item != NULL)
		CORO_INVOKE_1(_ctx->item->WaitForEndPattern, hSkipIdle);	

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(SetTonyPosition)(CORO_PARAM, uint32 nX, uint32 nY, uint32 nLoc, uint32) {
	Tony->SetPosition(RMPoint(nX, nY), nLoc);
}

DECLARE_CUSTOM_FUNCTION(MoveTonyAndWait)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(Tony->Move, RMPoint(nX, nY));
	
	if (!bSkipIdle)
		CORO_INVOKE_0(Tony->WaitForEndMovement);

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(MoveTony)(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	Tony->Move(coroParam, RMPoint(nX, nY));
}

DECLARE_CUSTOM_FUNCTION(ScrollLocation)(CORO_PARAM, uint32 nX, uint32 nY, uint32 sX, uint32 sY) {
	CORO_BEGIN_CONTEXT;
		int lx, ly;
		RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Prende le coordinate di quanto scrollare
	_ctx->lx = (int32)nX;
	_ctx->ly = (int32)nY;

	_ctx->pt = Loc->ScrollPosition();
	
	while ((_ctx->lx != 0 || _ctx->ly != 0) && !bSkipIdle) {
		if (_ctx->lx > 0) {
			_ctx->lx -= (int32)sX; if (_ctx->lx < 0) _ctx->lx = 0;
			_ctx->pt.Offset((int32)sX, 0);
		} else if (_ctx->lx < 0) {
			_ctx->lx += (int32)sX; if (_ctx->lx > 0) _ctx->lx = 0;
			_ctx->pt.Offset(-(int32)sX, 0);
		}

		if (_ctx->ly > 0) {
			_ctx->ly -= sY; if (_ctx->ly < 0) _ctx->ly = 0;
			_ctx->pt.Offset(0, sY);
		} else if (_ctx->ly < 0) {
			_ctx->ly += sY; if (_ctx->ly > 0) _ctx->ly = 0;
			_ctx->pt.Offset(0, -(int32)sY);
		}

		CORO_INVOKE_0(WaitFrame);

		Freeze();
		Loc->SetScrollPosition(_ctx->pt);
		Tony->SetScrollPosition(_ctx->pt);
		Unfreeze();
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

	// Prende le coordinate di quanto scrollare
	_ctx->lx=*((int*)&nX);
	_ctx->ly=*((int*)&nY);
	_ctx->dimx = _ctx->lx;
	_ctx->dimy = _ctx->ly;
	if (_ctx->lx < 0) _ctx->dimx = -_ctx->lx;
	if (_ctx->ly < 0) _ctx->dimy = -_ctx->ly;

	_ctx->stepX = sX;
	_ctx->stepY = sY;

	_ctx->startpt = Loc->ScrollPosition();
	
	_ctx->dwStartTime = _vm->GetTime();
	
	if (sX)
		_ctx->dwTotalTime = _ctx->dimx * (1000 / 35) / sX;
	else
		_ctx->dwTotalTime = _ctx->dimy * (1000 / 35) / sY;

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !bSkipIdle) {
		_ctx->dwCurTime = _vm->GetTime() - _ctx->dwStartTime;
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
				_ctx->pt.y += (_ctx->dimy*_ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt.y -= (_ctx->dimy*_ctx->dwCurTime) / _ctx->dwTotalTime;

		}

/*		
		sX = _ctx->stepX * (_ctx->dwCurTime-dwLastTime) / (1000 / 35);
		sY = _ctx->stepY * (_ctx->dwCurTime-dwLastTime) / (1000 / 35);

		if (_ctx->lx > 0) {
			_ctx->lx-=sX; if (_ctx->lx < 0) _ctx->lx = 0;
			_ctx->pt.Offset(sX,0);
		} else if (_ctx->lx < 0) {
			_ctx->lx+=sX; if (_ctx->lx > 0) _ctx->lx = 0;
			_ctx->pt.Offset(-sX,0);
		}

		if (_ctx->ly > 0) {
			_ctx->ly-=sY; if (_ctx->ly<0) _ctx->ly = 0;
			_ctx->pt.Offset(0,sY);
		} else if (_ctx->ly<0) {
			_ctx->ly+=sY; if (_ctx->ly > 0) _ctx->ly = 0;
			_ctx->pt.Offset(0,-sY);
		}
*/
		CORO_INVOKE_0(WaitFrame);

		Freeze();
		Loc->SetScrollPosition(_ctx->pt);
		Tony->SetScrollPosition(_ctx->pt);
		Unfreeze();

	}


	// Setta la posizione finale
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

	Freeze();
	Loc->SetScrollPosition(_ctx->pt);
	Tony->SetScrollPosition(_ctx->pt);
	Unfreeze();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(ChangeHotspot)(CORO_PARAM, uint32 dwCode, uint32 nX, uint32 nY, uint32) {
	int i;

	for (i = 0; i < curChangedHotspot; i++)
		if (ChangedHotspot[i].dwCode == dwCode) {
			ChangedHotspot[i].nX=nX;
			ChangedHotspot[i].nY=nY;
			break;
		}

	if (i == curChangedHotspot) {
		ChangedHotspot[i].dwCode=dwCode;
		ChangedHotspot[i].nX=nX;
		ChangedHotspot[i].nY=nY;
		curChangedHotspot++;
	}

	Loc->GetItemFromCode(dwCode)->ChangeHotspot(RMPoint(nX, nY));
}


DECLARE_CUSTOM_FUNCTION(AutoSave)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->AutoSave(coroParam);
}

DECLARE_CUSTOM_FUNCTION(Abort)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->Abort();
}

DECLARE_CUSTOM_FUNCTION(TremaSchermo)(CORO_PARAM, uint32 nScosse, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
		uint32 i;
		uint32 curTime;
		int dirx,diry;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = _vm->GetTime();
	
	_ctx->dirx = 1;
	_ctx->diry = 1;

	while (_vm->GetTime() < _ctx->curTime + nScosse) {
		CORO_INVOKE_0(WaitFrame);

		Freeze();
		Loc->SetFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		Tony->SetFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		Unfreeze();

		_ctx->i = _vm->_randomSource.getRandomNumber(2);

		if (_ctx->i == 0 || _ctx->i == 2)
			_ctx->dirx = -_ctx->dirx;
		else if (_ctx->i == 1 || _ctx->i == 2)
			_ctx->diry = -_ctx->diry;
	}

	Freeze();
	Loc->SetFixedScroll(RMPoint(0, 0));
	Tony->SetFixedScroll(RMPoint(0, 0));
	Unfreeze();
	
	CORO_END_CODE;
}



/*
 *	Personaggi
 */

DECLARE_CUSTOM_FUNCTION(CharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 16);
	Character[nChar].code = nCode;
	Character[nChar].item = Loc->GetItemFromCode(nCode);
	Character[nChar].r = 255;
	Character[nChar].g = 255;
 	Character[nChar].b = 255;
	Character[nChar].talkpattern = 0;
	Character[nChar].starttalkpattern = 0;
	Character[nChar].endtalkpattern = 0;
	Character[nChar].standpattern = 0;

	IsMChar[nChar] = false;
}

DECLARE_CUSTOM_FUNCTION(CharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar<16);
	Character[nChar].r = r;
	Character[nChar].g = g;
	Character[nChar].b = b;
}

DECLARE_CUSTOM_FUNCTION(CharSetTalkPattern)(CORO_PARAM, uint32 nChar, uint32 tp, uint32 sp, uint32) {
	assert(nChar<16);
	Character[nChar].talkpattern = tp;
	Character[nChar].standpattern = sp;
}

DECLARE_CUSTOM_FUNCTION(CharSetStartEndTalkPattern)(CORO_PARAM, uint32 nChar, uint32 sp, uint32 ep, uint32) {
	assert(nChar<16);
	Character[nChar].starttalkpattern=sp;
	Character[nChar].endtalkpattern=ep;
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
	_ctx->pt = Character[nChar].item->CalculatePos() - RMPoint(-60, 20) - Loc->ScrollPosition();
	
	if (Character[nChar].starttalkpattern != 0) {
		Freeze();
		Character[nChar].item->SetPattern(Character[nChar].starttalkpattern);
		Unfreeze();
		
		CORO_INVOKE_0(Character[nChar].item->WaitForEndPattern);
	}

 	Freeze();
	Character[nChar].item->SetPattern(Character[nChar].talkpattern);
	Unfreeze();

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Si posiziona all'interno del database delle voci all'inizio della prima
//		fseek(_vm->m_vdbFP, _ctx->curVoc->offset, SEEK_SET);
		g_system->lockMutex(vdb);
		_vm->_vdbFP.seek(_ctx->curVoc->offset);
		_ctx->curOffset = _ctx->curVoc->offset;
		g_system->unlockMutex(vdb);
	}

	for (_ctx->i = 0; _ctx->i<_ctx->msg->NumPeriods() && !bSkipIdle; _ctx->i++) {
		if (bIsBack) {
			curBackText = _ctx->text = new RMTextDialogScrolling(Loc);
			if (bTonyIsSpeaking)
				CORO_INVOKE_0(curBackText->Hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetInput(Input);

		// Skipping
		_ctx->text->SetSkipStatus(!bIsBack);
	
		// Allineamento
		_ctx->text->SetAlignType(RMText::HCENTER,RMText::VBOTTOM);
		
		// Colore
		_ctx->text->SetColor(Character[nChar].r,Character[nChar].g,Character[nChar].b);

		// Scrive il testo
		_ctx->text->WriteText((*_ctx->msg)[_ctx->i],0);

		// Setta la posizione
		_ctx->text->SetPosition(_ctx->pt);

		// Setta l'always display
		if (bAlwaysDisplay) { _ctx->text->SetAlwaysDisplay(); _ctx->text->ForceTime(); }

		// Registra il testo
		LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_system->lockMutex(vdb);
			_vm->_theSound.CreateSfx(&_ctx->voice);
			_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
			_ctx->voice->SetLoop(false);
			if (bIsBack) _ctx->voice->SetVolume(55);
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			_ctx->curOffset = _vm->_vdbFP.pos();
			g_system->unlockMutex(vdb);
		}

		// Aspetta la fine della visualizzazione	
		_ctx->text->SetCustomSkipHandle(hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice=NULL;
		}


		curBackText=NULL;
		delete _ctx->text;
	}

	if (Character[nChar].endtalkpattern != 0) {
		Freeze();
		Character[nChar].item->SetPattern(Character[nChar].endtalkpattern);
		Unfreeze();
		CORO_INVOKE_0(Character[nChar].item->WaitForEndPattern);
	}	

	Freeze();
	Character[nChar].item->SetPattern(Character[nChar].standpattern);
	Unfreeze();
	delete _ctx->msg;

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(AddInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	Inventory->AddItem(dwCode);	
}

DECLARE_CUSTOM_FUNCTION(RemoveInventory)(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	Inventory->RemoveItem(dwCode);
}

DECLARE_CUSTOM_FUNCTION(ChangeInventoryStatus)(CORO_PARAM, uint32 dwCode, uint32 dwStatus, uint32, uint32) {
  Inventory->ChangeItemStatus(dwCode,dwStatus);
}




/*
 *	Mastri Personaggi
 */

DECLARE_CUSTOM_FUNCTION(MCharSetCode)(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 10);
	MCharacter[nChar].code=nCode;
	if (nCode== 0)
		MCharacter[nChar].item=NULL;
	else
		MCharacter[nChar].item=Loc->GetItemFromCode(nCode);
	MCharacter[nChar].r=255;
	MCharacter[nChar].g=255;
	MCharacter[nChar].b=255;
	MCharacter[nChar].x=-1;
	MCharacter[nChar].y=-1;
	MCharacter[nChar].bAlwaysBack = 0;

	for (int i = 0; i < 10; i++)
		MCharacter[nChar].numtalks[i] = 1;

	MCharacter[nChar].curgroup = 0;
	
	IsMChar[nChar] = true;
}

DECLARE_CUSTOM_FUNCTION(MCharResetCode)(CORO_PARAM, uint32 nChar, uint32, uint32, uint32) {
	MCharacter[nChar].item=Loc->GetItemFromCode(MCharacter[nChar].code);
}


DECLARE_CUSTOM_FUNCTION(MCharSetPosition)(CORO_PARAM, uint32 nChar, uint32 nX, uint32 nY, uint32) {
	assert(nChar < 10);
	MCharacter[nChar].x=nX;
	MCharacter[nChar].y=nY;
}


DECLARE_CUSTOM_FUNCTION(MCharSetColor)(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 10);
	MCharacter[nChar].r=r;
	MCharacter[nChar].g=g;
	MCharacter[nChar].b=b;
}


DECLARE_CUSTOM_FUNCTION(MCharSetNumTalksInGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32 nTalks, uint32) {
  assert(nChar < 10);
	assert(nGroup < 10);

	MCharacter[nChar].numtalks[nGroup]=nTalks;
}


DECLARE_CUSTOM_FUNCTION(MCharSetCurrentGroup)(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32, uint32)
{
  assert(nChar < 10);
	assert(nGroup < 10);

	MCharacter[nChar].curgroup = nGroup;
}

DECLARE_CUSTOM_FUNCTION(MCharSetNumTexts)(CORO_PARAM, uint32 nChar, uint32 nTexts, uint32, uint32) {
	assert(nChar < 10);

	MCharacter[nChar].numtexts=nTexts-1;
	MCharacter[nChar].bInTexts = false;
}

DECLARE_CUSTOM_FUNCTION(MCharSetAlwaysBack)(CORO_PARAM, uint32 nChar, uint32 bAlwaysBack, uint32, uint32) {
	assert(nChar < 10);

	MCharacter[nChar].bAlwaysBack=bAlwaysBack;
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

	bIsBack |= MCharacter[nChar].bAlwaysBack ? 1 : 0;

	// Calcola la posizione del testo in base al frame corrente
	if (MCharacter[nChar].x==-1)
		_ctx->pt=MCharacter[nChar].item->CalculatePos()-RMPoint(-60,20)-Loc->ScrollPosition();
	else
		_ctx->pt=RMPoint(MCharacter[nChar].x,MCharacter[nChar].y);
	
	// Parametro per le azioni speciali: random tra le parlate
	_ctx->parm = (MCharacter[nChar].curgroup * 10) + _vm->_randomSource.getRandomNumber(
			MCharacter[nChar].numtalks[MCharacter[nChar].curgroup] - 1) + 1;

	// Cerca di eseguire la funzione custom per inizializzare la parlata
	if (MCharacter[nChar].item) {
		_ctx->h = mpalQueryDoAction(30, MCharacter[nChar].item->MpalCode(), _ctx->parm);
		if (_ctx->h != INVALID_PID_VALUE) {
			CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);
		}
	}

	_ctx->curVoc = SearchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Si posiziona all'interno del database delle voci all'inizio della prima
		g_system->lockMutex(vdb);
	//		fseek(_vm->m_vdbFP, curVoc->offset, SEEK_SET);
		_vm->_vdbFP.seek(_ctx->curVoc->offset);
		_ctx->curOffset = _ctx->curVoc->offset;
		g_system->unlockMutex(vdb);
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods() && !bSkipIdle; _ctx->i++) {
		// Crea l'oggetto diverso se ่ back o no
		if (bIsBack) {
			curBackText=_ctx->text = new RMTextDialogScrolling(Loc);
			if (bTonyIsSpeaking)
				CORO_INVOKE_0(curBackText->Hide);
		}
		else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetInput(Input);

		// Skipping
		_ctx->text->SetSkipStatus(!bIsBack);
	
		// Allineamento
		_ctx->text->SetAlignType(RMText::HCENTER,RMText::VBOTTOM);
		
		// Colore
		_ctx->text->SetColor(MCharacter[nChar].r,MCharacter[nChar].g,MCharacter[nChar].b);

		// Scrive il testo
		_ctx->text->WriteText((*_ctx->msg)[_ctx->i], nFont);

		// Setta la posizione
		_ctx->text->SetPosition(_ctx->pt);

		// Setta l'always display
		if (bAlwaysDisplay) { _ctx->text->SetAlwaysDisplay(); _ctx->text->ForceTime(); }

		// Registra il testo
		LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_system->lockMutex(vdb);
			_vm->_theSound.CreateSfx(&_ctx->voice);
			_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
			_ctx->voice->SetLoop(false);
			if (bIsBack) _ctx->voice->SetVolume(55);
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
			_ctx->curOffset = _vm->_vdbFP.pos();
			g_system->unlockMutex(vdb);
		}

		// Aspetta la fine della visualizzazione	
		_ctx->text->SetCustomSkipHandle(hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->Stop();
			_ctx->voice->Release();
			_ctx->voice = NULL;
		}

	 	curBackText = NULL;
		delete _ctx->text;
		delete _ctx->msg;
	}


	// Cerca di eseguire la funzione custom per chiudere la parlata
	if (MCharacter[nChar].item) {
		_ctx->h = mpalQueryDoAction(31, MCharacter[nChar].item->MpalCode(), _ctx->parm);
		if (_ctx->h != INVALID_PID_VALUE)
			CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);
	}

	CORO_END_CODE;
}






/*
 *	Dialoghi
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

	// La SendDialogMessage pu๒ andare in background se ่ un personaggio in MCHAR settato
	// con la SetAlwaysBack
	if (nPers != 0 && IsMChar[nPers] && MCharacter[nPers].bAlwaysBack)	
		_ctx->bIsBack = true;

	_ctx->curVoc = SearchVoiceHeader(curDialog, nMsg);
	_ctx->voice = NULL;

	if (_ctx->curVoc) {
		// Si posiziona all'interno del database delle voci all'inizio della prima
		g_system->lockMutex(vdb);
//		fseek(_vm->m_vdbFP, _ctx->curVoc->offset, SEEK_SET);
		_vm->_vdbFP.seek(_ctx->curVoc->offset);
		_vm->_theSound.CreateSfx(&_ctx->voice);
		_ctx->voice->LoadVoiceFromVDB(_vm->_vdbFP);
		_ctx->voice->SetLoop(false);
		if (_ctx->bIsBack) _ctx->voice->SetVolume(55);
		g_system->unlockMutex(vdb);
	}

	_ctx->string = mpalQueryDialogPeriod(nMsg);

	if (nPers == 0) {
		_ctx->text = new RMTextDialog;
		_ctx->text->SetColor(0,255,0);			
		_ctx->text->SetPosition(Tony->Position()-RMPoint(0,130)-Loc->ScrollPosition());
		_ctx->text->WriteText(_ctx->string,0);

		if (dwTonyNumTexts > 0) {
		  if (!bTonyInTexts) {
				if (nTonyNextTalkType != Tony->TALK_NORMAL) {
					CORO_INVOKE_1(Tony->StartTalk, nTonyNextTalkType);
					if (!bStaticTalk)
						nTonyNextTalkType = Tony->TALK_NORMAL;
				} else
					CORO_INVOKE_1(Tony->StartTalk, Tony->TALK_NORMAL);

				bTonyInTexts = true;
			}
			dwTonyNumTexts--;
		} else {
			CORO_INVOKE_1(Tony->StartTalk, nTonyNextTalkType);
			if (!bStaticTalk)
				nTonyNextTalkType = Tony->TALK_NORMAL;
		}
	} else if (!IsMChar[nPers]) {
		_ctx->text = new RMTextDialog;

		_ctx->pt = Character[nPers].item->CalculatePos() - RMPoint(-60, 20) - Loc->ScrollPosition();

		if (Character[nPers].starttalkpattern != 0) {
		  	Freeze();
  			Character[nPers].item->SetPattern(Character[nPers].starttalkpattern);
	  		Unfreeze();
			CORO_INVOKE_0(Character[nPers].item->WaitForEndPattern);
		}	

		Character[nPers].item->SetPattern(Character[nPers].talkpattern);

		_ctx->text->SetColor(Character[nPers].r, Character[nPers].g,Character[nPers].b);
		_ctx->text->WriteText(_ctx->string, 0);
		_ctx->text->SetPosition(_ctx->pt);
	} else {
		if (MCharacter[nPers].x == -1)
			_ctx->pt = MCharacter[nPers].item->CalculatePos() - RMPoint(-60, 20) - Loc->ScrollPosition();
		else
			_ctx->pt = RMPoint(MCharacter[nPers].x, MCharacter[nPers].y);

		// Parametro per le azioni speciali: random tra le parlate
		_ctx->parm = (MCharacter[nPers].curgroup * 10) + _vm->_randomSource.getRandomNumber(
			MCharacter[nPers].numtalks[MCharacter[nPers].curgroup] - 1) + 1;

		if (MCharacter[nPers].numtexts != 0 && MCharacter[nPers].bInTexts) {
			MCharacter[nPers].numtexts--;
		} else {
			// Cerca di eseguire la funzione custom per inizializzare la parlata
			_ctx->h = mpalQueryDoAction(30, MCharacter[nPers].item->MpalCode(), _ctx->parm);
			if (_ctx->h != INVALID_PID_VALUE)
				CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);

			MCharacter[nPers].curTalk = _ctx->parm;
				
			if (MCharacter[nPers].numtexts != 0) {
				MCharacter[nPers].bInTexts = true;
				MCharacter[nPers].numtexts--;
			}
		}

		if (MCharacter[nPers].bAlwaysBack) {
			_ctx->text = curBackText = new RMTextDialogScrolling(Loc);
			if (bTonyIsSpeaking)
				CORO_INVOKE_0(curBackText->Hide);

			_ctx->bIsBack = true;
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->SetSkipStatus(!MCharacter[nPers].bAlwaysBack);
		_ctx->text->SetColor(MCharacter[nPers].r,MCharacter[nPers].g,MCharacter[nPers].b);
		_ctx->text->WriteText(_ctx->string,0);
		_ctx->text->SetPosition(_ctx->pt);
	}

	if (!bSkipIdle) {
		_ctx->text->SetInput(Input);
		if (bAlwaysDisplay) { _ctx->text->SetAlwaysDisplay(); _ctx->text->ForceTime(); }
		_ctx->text->SetAlignType(RMText::HCENTER,RMText::VBOTTOM);
		LinkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_ctx->voice->Play();
			_ctx->text->SetCustomSkipHandle2(_ctx->voice->hEndOfBuffer);
		}

		// Aspetta la fine della visualizzazione	
		_ctx->text->SetCustomSkipHandle(hSkipIdle);
		CORO_INVOKE_0(_ctx->text->WaitForEndDisplay);
	}

	if (_ctx->curVoc) {
		_ctx->voice->Stop();
		_ctx->voice->Release();
		_ctx->voice=NULL;
	}

	if (nPers != 0) {
		if (!IsMChar[nPers]) {
	 		if (Character[nPers].endtalkpattern != 0) {
				Freeze();
				Character[nPers].item->SetPattern(Character[nPers].endtalkpattern);
				Unfreeze();
				CORO_INVOKE_0(Character[nPers].item->WaitForEndPattern);
			}	
			
			Character[nPers].item->SetPattern(Character[nPers].standpattern);
			delete _ctx->text;
		} else {
			if ((MCharacter[nPers].bInTexts && MCharacter[nPers].numtexts== 0) || !MCharacter[nPers].bInTexts) {
				// Cerca di eseguire la funzione custom per chiudere la parlata
				MCharacter[nPers].curTalk = (MCharacter[nPers].curTalk%10) + MCharacter[nPers].curgroup*10;
				_ctx->h = mpalQueryDoAction(31,MCharacter[nPers].item->MpalCode(),MCharacter[nPers].curTalk);
				if (_ctx->h != INVALID_PID_VALUE)
					CORO_INVOKE_2(g_scheduler->waitForSingleObject, _ctx->h, INFINITE);

				MCharacter[nPers].bInTexts = false;
				MCharacter[nPers].numtexts = 0;
			}

			curBackText = NULL;
			delete _ctx->text;
		}
	} else {
		if ((dwTonyNumTexts== 0 && bTonyInTexts) || !bTonyInTexts) {	
			CORO_INVOKE_0(Tony->EndTalk);
			dwTonyNumTexts = 0;
			bTonyInTexts = false;
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
		int i, num;
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
	while (_ctx->nChoice != -1) {
		// Si fa dare la lista di opzioni e le conta
		_ctx->sl = mpalQueryDialogSelectList(_ctx->nChoice);
		for (_ctx->num = 0; _ctx->sl[_ctx->num] != 0; _ctx->num++)
			;

		// Se c'e' una sola opzione, la fa automaticamente, e aspetta la prossima scelta
		if (_ctx->num == 1) {
			mpalQueryDialogSelection(_ctx->nChoice, _ctx->sl[0]);
			GlobalFree(_ctx->sl);
			continue;
		}
		
		// Crea una scelta per il dialogo
		_ctx->dc.Init();
		_ctx->dc.SetNumChoices(_ctx->num);

		// Scrive tutte le possibili opzioni
		for (_ctx->i = 0; _ctx->i < _ctx->num; _ctx->i++) {
			_ctx->string = mpalQueryDialogPeriod(_ctx->sl[_ctx->i]);
			assert(_ctx->string != NULL);
			_ctx->dc.AddChoice(_ctx->string);
			GlobalFree(_ctx->string);
		}

		// Attiva l'oggetto
		LinkGraphicTask(&_ctx->dc);
		CORO_INVOKE_0(_ctx->dc.Show);

		// Disegna il puntatore
		Pointer->SetSpecialPointer(Pointer->PTR_NONE);
		MainShowMouse();
			
		while (!(Input->MouseLeftClicked() && ((_ctx->sel = _ctx->dc.GetSelection()) != -1))) {
			CORO_INVOKE_0(WaitFrame);
			Freeze();
			_ctx->dc.DoFrame(Input->MousePos());
			Unfreeze();
		}	

		// Nascondi il puntatore
		MainHideMouse();
		
		CORO_INVOKE_0(_ctx->dc.Hide);
		mpalQueryDialogSelection(_ctx->nChoice, _ctx->sl[_ctx->sel]);

		// Chiude la scelta
		_ctx->dc.Close();

		GlobalFree(_ctx->sl);

		// Wait for the next choice to be made
		mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	}

	CORO_END_CODE;
}





/*
 *	Sync tra idle e mpal
 */

DECLARE_CUSTOM_FUNCTION(TakeOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
//	EnterCriticalSection(&cs[num]);
//	WaitForSingleObject(mut[num],INFINITE);
	warning("TODO");
}

DECLARE_CUSTOM_FUNCTION(ReleaseOwnership)(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
//	LeaveCriticalSection(&cs[num]);
//	g_system->unlockMutex(mut[num]);
	warning("TODO: ReleaseOwnership");
}











/*
 *	Musica
 *  ------
 *
 * Effetti di fade supportati:
 *
 *   nFX = 0   - La nuova musica si sostituisce a quella vecchia
 *   nFX=1   - La nuova musica si interfade con quella vecchia
 *   nFX=2	 - La nuova musica subentra a tempo con quella vecchia
 *
 */

int curSonoriz = 0;

bool bFadeOutStop;

void ThreadFadeInMusic(CORO_PARAM, const void *nMusic) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	int nChannel = *(const int *)nMusic;

	CORO_BEGIN_CODE(_ctx);

	debug("Start FadeIn Music\n");

	for (_ctx->i = 0; _ctx->i < 16; _ctx->i++) {
		_vm->SetMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(g_scheduler->sleep, 100);
	}
	_vm->SetMusicVolume(nChannel, 64);

	debug("End FadeIn Music\n");
	
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

	_ctx->startVolume = _vm->GetMusicVolume(nChannel);

	debug("Start FadeOut Music\n");

	for (_ctx->i = 16; _ctx->i > 0 && !bFadeOutStop; _ctx->i--) {
		if (_ctx->i * 4 < _ctx->startVolume)
			_vm->SetMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(g_scheduler->sleep, 100);
	}
	
	if (!bFadeOutStop)
		_vm->SetMusicVolume(nChannel, 0);

	// Se ่ uno stacchetto ferma tutto
	if (nChannel == 2)
		_vm->StopMusic(2);

	debug("End FadeOut Music\n");

	CORO_KILL_SELF();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(FadeInSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_scheduler->createProcess(ThreadFadeInMusic, &curSonoriz, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	bFadeOutStop = false;
	g_scheduler->createProcess(ThreadFadeOutMusic, &curSonoriz, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeOutStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	bFadeOutStop = false;
	int channel = 2;
	g_scheduler->createProcess(ThreadFadeOutMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(FadeInStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	int channel = 2;
	g_scheduler->createProcess(ThreadFadeInMusic, &channel, sizeof(int));
}

DECLARE_CUSTOM_FUNCTION(StopSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->StopMusic(curSonoriz);
}

DECLARE_CUSTOM_FUNCTION(StopStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->StopMusic(2);
}

DECLARE_CUSTOM_FUNCTION(MuteSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->SetMusicVolume(curSonoriz, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteSonoriz)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	bFadeOutStop = true;
	_vm->SetMusicVolume(curSonoriz, 64);
}

DECLARE_CUSTOM_FUNCTION(MuteStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->SetMusicVolume(2, 0);
}

DECLARE_CUSTOM_FUNCTION(DemuteStacchetto)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	_vm->SetMusicVolume(2, 64);
}

struct MusicFileEntry {
	const char *name;
	int sync;
};
const MusicFileEntry musicFiles[] =  { 
/*
	{ "PREGAME1.ADP", 0 }, { "SONO1.ADP", 0 },
	{ "SONO2.ADP", 0 }, { "SONO3.ADP", 0 },
	{ "CADUTA.ADP",0 }, { "RISVEGLIO.ADP", 0 },
	{ "BACKGROUND.ADP", 0 }, { "PREGAME3.ADP", 0 },
	{ "BARBUTA1.ADP", 2450 }, { "BARBUTA2.ADP", 0 },
*/
	{ "00.ADP", 0 }, { "01.ADP", 0 },
	{ "02.ADP", 0 }, { "03.ADP", 0 },
	{ "04.ADP",0 }, { "05.ADP", 0 },
	{ "06.ADP", 0 }, { "07.ADP", 0 },
	{ "08.ADP", 2450 }, { "09.ADP", 0 },

/*
	{ "BARBUTA3.ADP", 0 }, { "15-RUFUS.ADP", 0 },
	{ "20-MAMMA.ADP", 0 }, { "32-MAMMARADIO.ADP", 0 },
	{ "24-TUNNELOV-MIDI.ADP", 0 }, { "34-RAZZO.ADP", 0 },
	{ "27-ZUCC1.ADP", 0 }, { "BEAST.ADP", 0 },
	{ "22-MORTIMER.ADP", 0 }, { "25-PUB-OVATTATO.ADP", 0 },
*/
	{ "10.ADP", 0 }, { "11.ADP", 0 },
	{ "12.ADP", 0 }, { "13.ADP", 0 },
	{ "14.ADP", 0 }, { "15.ADP", 0 },
	{ "16.ADP", 0 }, { "17.ADP", 0 },
	{ "18.ADP", 0 }, { "19.ADP", 0 },
/*
	{ "25-PUB.ADP", 0 }, { "ALBERGO.ADP", 0 },
	{ "37.ADP", 0 }, { "26-PIRAT.ADP", 0 },
	{ "LORENZBACK.ADP", 0 }, { "LORENZ3.ADP", 0 },
	{ "CASTLE.ADP", 0 }, { "53-BACKR.ADP", 0 },
	{ "16-SMIL1.ADP", 1670 }, { "16-SMIL2.ADP", 0 },
*/
	{ "20.ADP", 0 }, { "21.ADP", 0 },
	{ "22.ADP", 0 }, { "23.ADP", 0 },
	{ "24.ADP", 0 }, { "25.ADP", 0 },
	{ "26.ADP", 0 }, { "27.ADP", 0 },
	{ "28.ADP", 1670 }, { "29.ADP", 0 },
/*	
	{ "16-SMILE.ADP", 0 }, { "16-DIALOG2.ADP", 0 },
	{ "07-SHAKE1.ADP", 2900 }, { "07-SHAKE2.ADP", 0 },
	{ "07-SHAKE3.ADP", 0 }, { "46-BEEE.ADP", 0 },
	{ "434748.ADP", 0 }, { "TORRE.ADP", 0 },
	{ "50-DRAGO.ADP", 0 }, { "56-OPERA.ADP", 0 },
*/
	{ "30.ADP", 0 }, { "31.ADP", 0 },
	{ "32.ADP", 2900 }, { "33.ADP", 0 },
	{ "34.ADP", 0 }, { "35.ADP", 0 },
	{ "36.ADP", 0 }, { "37.ADP", 0 },
	{ "38.ADP", 0 }, { "39.ADP", 0 },
/*
	{ "FLAUTARP.ADP", 0 }, { "01-2001.ADP", 1920 },
	{ "02-INTROPANORAMICA.ADP", 1560 }, { "03-PANORAMICA.ADP", 1920 },
	{ "04-INTERNI.ADP", 1920 }, { "05-CADEPIUMA.ADP", 1920 },
	{ "06-SCENDESCALE.ADP", 1920 }, { "07-THRILL.ADP", 1920 },
	{ "08-CADUTAZUCCA.ADP", 1920 }, { "09-PIUMAALEGGIA.ADP", 1920 },
*/
	{ "40.ADP", 0 }, { "41.ADP", 1920 },
	{ "42.ADP", 1560 }, { "43.ADP", 1920 },
	{ "44.ADP", 1920 }, { "45.ADP", 1920 },
	{ "46.ADP", 1920 }, { "47.ADP", 1920 },
	{ "48.ADP", 1920 }, { "49.ADP", 1920 },

/*
	{ "10-JACKZUCCONA.ADP", 1920 }, { "11-JACKIMPRECAALLAGRANDELUNA.ADP", 1920 },
	{ "12-GRANDELUNALOGO.ADP", 1920 }, { "60-BACKGROUND.ADP", 0 },
	{ "TONYSTAR.ADP", 0 }, { "FUNNY.ADP", 0 },
	{ "60-INIZIODIALOGO.ADP", 0 }, { "60-DIALOGO.ADP", 0 },
	{ "60-JACKFELICE.ADP", 0 }, { "24-TONYVIOLENTATO.ADP", 0 }
*/

	{ "50.ADP", 1920 }, { "51.ADP", 1920 },
	{ "52.ADP", 1920 }, { "53.ADP", 0 },
	{ "54.ADP", 0 }, { "55.ADP", 0 },
	{ "56.ADP", 0 }, { "57.ADP", 0 },
	{ "58.ADP", 0 }, { "59.ADP", 0 }

};


const char *staccFileNames[] = { 
/*
	"05-MUCCA.ADP", "06-HALOW.ADP", 
	"LETTERAINIZIO.ADP", "LETTERAPAURA.ADP", 
	"LETTERAFINE.ADP", "PREGAME2.ADP",
	"07-TIMOT.ADP", "15-SHOTRUFUS.ADP",
	"15-SHOTTONY.ADP", "27-ZUCC2.ADP",
*/
	"S00.ADP", "S01.ADP", 
	"S02.ADP", "S03.ADP", 
	"S04.ADP", "S05.ADP",
	"S06.ADP", "S07.ADP",
	"S08.ADP", "S09.ADP",

/*
	"53-DLOGO.ADP", "16-DIALOG1.ADP",
	"TORRE1.ADP", "GARG1.ADP",
	"LORENZ1.ADP", "24-FIGHT.ADP",
	"08-MACBETH-PREPARA.ADP", "08-MACBETH-RECITA.ADP",
	"GATTO1.ADP"
*/
	"S10.ADP", "S11.ADP",
	"S12.ADP", "S13.ADP",
	"S14.ADP", "S15.ADP",
	"S16.ADP", "S17.ADP",
	"S18.ADP"
};


void CustPlayMusic(uint32 nChannel, const char *mFN, uint32 nFX, bool bLoop, int nSync = 0) {
	if (nSync == 0)
		nSync = 2000;
	debug("Start CustPlayMusic\n");
	PlayMusic(nChannel, mFN, nFX, bLoop, nSync);
	debug("End CustPlayMusic\n");
}

DECLARE_CUSTOM_FUNCTION(PlaySonoriz)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bNoLoop, uint32) {
	if (nFX == 0 || nFX == 1 || nFX==2) {
		debug("PlaySonoriz stop fadeout\n");
		bFadeOutStop = true;
	}
	
	lastMusic = nMusic;
	CustPlayMusic(curSonoriz, musicFiles[nMusic].name, nFX, bNoLoop ? false : true, musicFiles[nMusic].sync);
}

DECLARE_CUSTOM_FUNCTION(PlayStacchetto)(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bLoop, uint32) {
	CustPlayMusic(2,staccFileNames[nMusic],nFX,bLoop);
}

DECLARE_CUSTOM_FUNCTION(PlayItemSfx)(CORO_PARAM, uint32 nItem, uint32 nSFX, uint32, uint32) {
	if (nItem== 0) {
		Tony->PlaySfx(nSFX);
	} else {
		RMItem *item = Loc->GetItemFromCode(nItem);
		if (item)
			item->PlaySfx(nSFX);
	}
}


void RestoreMusic(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(PlaySonoriz, lastMusic, 0, 0, 0);

	if (lastTappeto != 0)
		CustPlayMusic(4, tappetiFile[lastTappeto], 0, true);

	CORO_END_CODE;
}

void SaveMusic(Common::OutSaveFile *f) {
	f->writeByte(lastMusic);
	f->writeByte(lastTappeto);
}

void LoadMusic(Common::InSaveFile *f) {
	lastMusic = f->readByte();
	lastTappeto = f->readByte();
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
	bSkipIdle = true;
	g_scheduler->setEvent(hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(MustSkipIdleEnd)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	bSkipIdle = false;
	g_scheduler->resetEvent(hSkipIdle);
}

DECLARE_CUSTOM_FUNCTION(PatIrqFreeze)(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	bPatIrqFreeze = bStatus;
}

DECLARE_CUSTOM_FUNCTION(OpenInitLoadMenu)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	CORO_INVOKE_0(_vm->OpenInitLoadMenu);
	Unfreeze();

	CORO_END_CODE;
}

DECLARE_CUSTOM_FUNCTION(OpenInitOptions)(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	Freeze();
	CORO_INVOKE_0(_vm->OpenInitOptions);
	Unfreeze();

	CORO_END_CODE;
}


DECLARE_CUSTOM_FUNCTION(DoCredits)(CORO_PARAM, uint32 nMsg, uint32 dwTime, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
		RMMessage *msg;
		RMTextDialog *text;
		uint32 hDisable;
		int i;
		uint32 startTime;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);
	_ctx->hDisable = g_scheduler->createEvent(true, false);
	
	_ctx->text = new RMTextDialog[_ctx->msg->NumPeriods()];

	for (_ctx->i = 0; _ctx->i < _ctx->msg->NumPeriods(); _ctx->i++) 	{
		_ctx->text[_ctx->i].SetInput(Input);

		// Allineamento
		if ((*_ctx->msg)[_ctx->i][0] == '@') {
			_ctx->text[_ctx->i].SetAlignType(RMText::HCENTER, RMText::VTOP);
			_ctx->text[_ctx->i].WriteText(_ctx->msg[_ctx->i][1], 3);
			_ctx->text[_ctx->i].SetPosition(RMPoint(414, 70 + _ctx->i * 26));  // 70
		} else {
			_ctx->text[_ctx->i].SetAlignType(RMText::HLEFT,RMText::VTOP);
			_ctx->text[_ctx->i].WriteText((*_ctx->msg)[_ctx->i], 3);
			_ctx->text[_ctx->i].SetPosition(RMPoint(260, 70 + _ctx->i * 26));
		}
		
	
		// Setta la posizione
		_ctx->text[_ctx->i].SetAlwaysDisplay();
		_ctx->text[_ctx->i].SetForcedTime(dwTime * 1000);
		_ctx->text[_ctx->i].SetNoTab();

		// Aspetta la fine della visualizzazione	
		_ctx->text[_ctx->i].SetCustomSkipHandle(_ctx->hDisable);

		// Registra il testo
		LinkGraphicTask(&_ctx->text[_ctx->i]);
	}
	
	_ctx->startTime = _vm->GetTime();

	while (_ctx->startTime + dwTime * 1000 > _vm->GetTime()) {
		CORO_INVOKE_0(WaitFrame);
		if (Input->MouseLeftClicked() || Input->MouseRightClicked())
			break;
		if ((GetAsyncKeyState(Common::KEYCODE_TAB) & 0x8001) == 0x8001)
			break;
	}

	g_scheduler->setEvent(_ctx->hDisable);

	CORO_INVOKE_0(WaitFrame);
	CORO_INVOKE_0(WaitFrame);

	delete[] _ctx->text;
	delete _ctx->msg;

	CORO_END_CODE;
}



BEGIN_CUSTOM_FUNCTION_MAP()

	ASSIGN(  1,   CustLoadLocation)
	ASSIGN(  2,		MySleep)
	ASSIGN(  3,   SetPointer)
	ASSIGN(  5,   MoveTony)
	ASSIGN(  6,		FaceToMe)
	ASSIGN(  7,		BackToMe)
	ASSIGN(  8,		LeftToMe)
	ASSIGN(  9,		RightToMe)
	ASSIGN( 10,   SendTonyMessage)
	ASSIGN( 11,		ChangeBoxStatus)
	ASSIGN( 12,		ChangeLocation)
	ASSIGN( 13,		DisableTony)
	ASSIGN( 14,		EnableTony)
	ASSIGN( 15,		WaitForPatternEnd)
	ASSIGN( 16,   SetLocStartPosition)
	ASSIGN( 17,   ScrollLocation)
	ASSIGN( 18,   MoveTonyAndWait)
	ASSIGN( 19,		ChangeHotspot)
	ASSIGN( 20,   AddInventory)
	ASSIGN( 21,   RemoveInventory)
	ASSIGN( 22,		ChangeInventoryStatus)
	ASSIGN( 23,		SetTonyPosition)
	ASSIGN( 24,		SendFullscreenMessage)
	ASSIGN( 25,		SaveTonyPosition)
	ASSIGN( 26,		RestoreTonyPosition)
	ASSIGN( 27,		DisableInput)
	ASSIGN( 28,		EnableInput)
	ASSIGN( 29,		StopTony)

	ASSIGN( 30,		TonyTakeUp1)
	ASSIGN( 31,		TonyTakeMid1)
	ASSIGN( 32,		TonyTakeDown1)
	ASSIGN( 33,		TonyTakeUp2)
	ASSIGN( 34,		TonyTakeMid2)
	ASSIGN( 35,		TonyTakeDown2)

	ASSIGN( 72,		TonyPutUp1)
	ASSIGN( 73,		TonyPutMid1)
	ASSIGN( 74,		TonyPutDown1)
	ASSIGN( 75,		TonyPutUp2)
	ASSIGN( 76,		TonyPutMid2)
	ASSIGN( 77,		TonyPutDown2)

	ASSIGN( 36,   TonyPerTerra)
	ASSIGN( 37,   TonySiRialza)
	ASSIGN( 38,		TonyPastorella)
	ASSIGN( 39,		TonyFischietto)

	ASSIGN( 40,   TonyRide)
	ASSIGN( 41,   TonyFianchi)
	ASSIGN( 42,   TonyCanta)
	ASSIGN( 43,   TonySiIndica)
	ASSIGN( 44,		TonySpaventatoConMani)
	ASSIGN( 49,	  TonySpaventatoSenzaMani)
	ASSIGN( 45,		TonyConBicchiere)
	ASSIGN( 46,		TonyConVerme)
	ASSIGN( 47,		TonyConMartello)
	ASSIGN( 48,		TonyConCorda)
	ASSIGN( 90,		TonyConConiglioANIM)
	ASSIGN( 91,		TonyConRicettaANIM)
	ASSIGN( 92,		TonyConCarteANIM)
	ASSIGN( 93,		TonyConPupazzoANIM)
	ASSIGN( 94,		TonyConPupazzoStart)
	ASSIGN( 95,		TonyConPupazzoEnd)
	ASSIGN( 96,		TonyConConiglioStart)
	ASSIGN( 97,		TonyConConiglioEnd)
	ASSIGN( 98,		TonyConRicettaStart)
	ASSIGN( 99,		TonyConRicettaEnd)
	ASSIGN(100,		TonyConCarteStart)
	ASSIGN(101,		TonyConCarteEnd)
	ASSIGN(102,   TonyConTaccuinoStart)
	ASSIGN(103,   TonyConTaccuinoEnd)
	ASSIGN(104,   TonyConMegafonoStart)
	ASSIGN(105,   TonyConMegafonoEnd)
	ASSIGN(106,   TonyConBarbaStart)
	ASSIGN(107,   TonyConBarbaEnd)
	ASSIGN(108,   TonyRidacchia)
	ASSIGN(109,	  TonySchifato)
	ASSIGN(110,	  TonyNaah)
	ASSIGN(111,	  TonyMacbeth)
	ASSIGN(112,	  TonySniffaLeft)
	ASSIGN(113,	  TonySniffaRight)
	ASSIGN(114,	  TonySpaventatoStart)
	ASSIGN(115,	  TonySpaventatoEnd)
	ASSIGN(116,	  TonyConSegretaria)
	
	ASSIGN( 50,   CharSetCode)
	ASSIGN( 51,		CharSetColor)
	ASSIGN( 52,   CharSetTalkPattern)
	ASSIGN( 53,   CharSendMessage)
	ASSIGN( 54,   CharSetStartEndTalkPattern)

	ASSIGN( 60,   MCharSetCode)
	ASSIGN( 61,   MCharSetColor)
	ASSIGN( 62,	  MCharSetCurrentGroup)
	ASSIGN( 63,		MCharSetNumTalksInGroup)
	ASSIGN( 64,		MCharSetNumTexts)
	ASSIGN( 65,		MCharSendMessage)
	ASSIGN( 66,		MCharSetPosition)
	ASSIGN( 67,		MCharSetAlwaysBack)
	ASSIGN( 68,   MCharResetCode)

	ASSIGN( 70,		StartDialog)
	ASSIGN( 71,		SendDialogMessage)

	ASSIGN( 80,		TakeOwnership)
	ASSIGN( 81,		ReleaseOwnership)

	ASSIGN( 86,		PlaySonoriz)
	ASSIGN( 87,		PlayStacchetto)
	ASSIGN( 88,		FadeInSonoriz)
	ASSIGN( 89,		FadeOutSonoriz)
	ASSIGN(123,		FadeInStacchetto)
	ASSIGN(124,		FadeOutStacchetto)
	ASSIGN(125,		MuteSonoriz)
	ASSIGN(126,		DemuteSonoriz)
	ASSIGN(127,		MuteStacchetto)
	ASSIGN(128,		DemuteStacchetto)
	ASSIGN( 84,		StopSonoriz)
	ASSIGN( 85,		StopStacchetto)
	ASSIGN( 83,		PlayItemSfx)
	ASSIGN(129,		StacchettoFadeStart)
	ASSIGN(130,		StacchettoFadeEnd)

	ASSIGN(120,		TremaSchermo)
	ASSIGN(121,		AutoSave)
	ASSIGN(122,		Abort)
	ASSIGN(131,		NoOcchioDiBue)
	ASSIGN(132,		SendFullscreenMsgStart)
	ASSIGN(133,		SendFullscreenMsgEnd)
	ASSIGN(134,		CustEnableGUI)
	ASSIGN(135,		CustDisableGUI)
	ASSIGN(136,		ClearScreen)
	ASSIGN(137,		PatIrqFreeze)
	ASSIGN(138,		TonySetPalesati)
	ASSIGN(139,		OpenInitLoadMenu)
	ASSIGN(140,		OpenInitOptions)
	ASSIGN(141,		SyncScrollLocation)
	ASSIGN(142,		CloseLocation)
	ASSIGN(143,		SetAlwaysDisplay)
	ASSIGN(144,		DoCredits)

	ASSIGN( 200,	MustSkipIdleStart);
	ASSIGN( 201,	MustSkipIdleEnd);

END_CUSTOM_FUNCTION_MAP()

void SetupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input) {
	Tony = tony;	
	Pointer = ptr;
	Boxes = box;
	Loc = loc;
	Inventory = inv;
	Input = input;

	LoadLocation = MainLoadLocation;
	UnloadLocation = MainUnloadLocation;
	LinkGraphicTask = MainLinkGraphicTask;
	Freeze = MainFreeze;
	Unfreeze = MainUnfreeze;
	WaitFrame = MainWaitFrame;
	PlayMusic = MainPlayMusic;
	InitWipe = MainInitWipe;
	CloseWipe = MainCloseWipe;
	WaitWipeEnd = MainWaitWipeEnd;
	DisableGUI = MainDisableGUI;
	EnableGUI = MainEnableGUI;
	SetPalesati = MainSetPalesati;

	vdb = g_system->createMutex();

	bAlwaysDisplay = false;
	int i;

	for (i = 0;i < 10; i++)
		cs[i] = g_system->createMutex();
/*
	for (i = 0;i < 10; i++)
		mut[i] = CreateMutex(NULL, false, NULL);
*/
	for (i = 0; i < 200; i++)
		tappeti[i] = 0;

	tappeti[6] = T_GRILLI;
	tappeti[7] = T_GRILLI;
	tappeti[8] = T_GRILLIOV;
	tappeti[10] = T_GRILLI;
	tappeti[12] = T_GRILLI;
	tappeti[13] = T_GRILLIOV;
	tappeti[15] = T_GRILLI;
	tappeti[16] = T_GRILLIVENTO;
	tappeti[18] = T_GRILLI;
	tappeti[19] = T_GRILLIVENTO;
	tappeti[20] = T_GRILLI;
	tappeti[23] = T_GRILLI;
	tappeti[26] = T_MAREMETA;
	tappeti[27] = T_GRILLI;
	tappeti[28] = T_GRILLIVENTO;
	tappeti[31] = T_GRILLI;
	tappeti[33] = T_MARE;
	tappeti[35] = T_MARE;
	tappeti[36] = T_GRILLI;
	tappeti[37] = T_GRILLI;
	tappeti[40] = T_GRILLI;
	tappeti[41] = T_GRILLI;
	tappeti[42] = T_GRILLI;
	tappeti[45] = T_GRILLI;
	tappeti[51] = T_GRILLI;
	tappeti[52] = T_GRILLIVENTO1;
	tappeti[53] = T_GRILLI;
	tappeti[54] = T_GRILLI;
	tappeti[57] = T_VENTO;
	tappeti[58] = T_VENTO;
	tappeti[60] = T_VENTO;



	// Crea l'evento per skippare le idle
	hSkipIdle = g_scheduler->createEvent(true, false);
}

} // end of namespace Tony
