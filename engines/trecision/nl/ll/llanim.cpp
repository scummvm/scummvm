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

#include "common/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/scummsys.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"

// locals
#define SMACKNULL	0
#define SMACKOPEN	1
#define SMACKCLOSE  2
#define FULLMOTIONANIM	620

namespace Trecision {

void CallSmackOpen(Common::SeekableReadStream *stream);
void CallSmackClose();
void CallSmackNextFrame();
void CallSmackVolumePan(int buf, int track, int vol);
void RegenSmackAnim(int num);
void RegenSmackIcon(int StartIcon, int num);

uint16 _smackPal[MAXSMACK][256];
uint8 _curSmackAction;
uint8 _curSmackBuffer;

uint16 _playingAnims[MAXSMACK];
uint16 _curAnimFrame[MAXSMACK];

uint16 _animMaxX, _animMinX, _animMaxY, _animMinY;

/*-----------------17/11/96 14.46-------------------
					StartSmackAnim
--------------------------------------------------*/
void StartSmackAnim(uint16 num) {
	extern char UStr[];
	int pos;

	_curSmackAction = SMACKOPEN;

	// choose the buffer to use
	if (AnimTab[num]._flag & SMKANIM_BKG)
		pos = 0;
	else if (AnimTab[num]._flag & SMKANIM_ICON)
		pos = 2;
	else {
		pos = 1;

		_animMaxX = 0;
		_animMinX = MAXX;
		_animMaxY = 0;
		_animMinY = MAXY;
	}

	_curSmackBuffer = pos;

	if (_playingAnims[pos] != 0) {
		_curSmackAction = SMACKNULL;
		StopSmackAnim(_playingAnims[pos]);
		_curSmackAction = SMACKOPEN;
	}

	_playingAnims[pos] = num;
	_curAnimFrame[pos] = 0;

	// sceglie come aprire
	if (AnimTab[num]._flag & SMKANIM_BKG) {
		sprintf(UStr, "Bkg\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));

		// spegne quelle che non servono
		if ((num == aBKG11) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG14) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG1C) && (g_vm->_obj[oFAX17]._flag & OBJFLAG_EXTRA)) {
			AnimTab[num]._flag |= SMKANIM_OFF1;
			CallSmackVolumePan(0, 1, 0);
		} else if ((num == aBKG1D) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG22) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG48) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG4P) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG28) && (AnimTab[num]._flag & SMKANIM_OFF4))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG37) && (!(g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA)))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG2E) && (AnimTab[num]._flag & SMKANIM_OFF2))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG2G) && (_choice[556]._flag & OBJFLAG_DONE))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG34) &&				// se sono nella 34 e
				 ((_choice[616]._flag & OBJFLAG_DONE) ||		// ho gia' fatto fmv o
		          (g_vm->_obj[oTUBOT34]._mode & OBJMODE_OBJSTATUS) || // c'e' il tubo intero
		          (g_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS) || // c'e' il tubo fuori o
		          (g_vm->_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS))) // la valovola e' chiusa
			CallSmackVolumePan(0, 2, 0);
	} else if (AnimTab[num]._flag & SMKANIM_ICON) {
		sprintf(UStr, "Icon\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));
	} else {
		extern uint32 NextRefresh;
		uint32 st = ReadTime();

		sprintf(UStr, "Anim\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));
		NextRefresh += (ReadTime() - st);    // fixup opening time
	}

	_curSmackAction = SMACKNULL;
}

/*-----------------17/11/96 14.47-------------------
					StopSmackAnim
--------------------------------------------------*/
void StopSmackAnim(uint16 num) {
	int pos;

	if (num == 0)
		return ;

	_curSmackAction = SMACKCLOSE;

	pos = 0;

	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos ++;

	if (pos >= MAXSMACK) {
		if (AnimTab[num]._flag & SMKANIM_BKG)
			pos = 0;
		else if (AnimTab[num]._flag & SMKANIM_ICON)
			pos = 2;
		else
			pos = 1;
	}

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	_curSmackBuffer = pos;
	CallSmackClose();

	_curSmackAction = SMACKNULL;

	g_vm->_lightIcon = 0xFF;
}

/* -----------------15/08/97 14.29-------------------
				StopAllSmackAnims
 --------------------------------------------------*/
void StopAllSmackAnims() {
	int a;

	for (a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a])
			StopSmackAnim(_playingAnims[a]);
	}
}

/*-----------------17/11/96 14.46-------------------
				StartFullMotion
--------------------------------------------------*/
void StartFullMotion(const char *name) {
	int pos;
	extern int _fullMotionStart, _fullMotionEnd;

	// spegne tutte le altre animazioni
	for (pos = 0; pos < MAXSMACK; pos++) {
		if (_playingAnims[pos] != 0) {
			_curSmackAction = SMACKNULL;
			StopSmackAnim(_playingAnims[pos]);
		}
	}
	_curSmackAction = SMACKOPEN;

	pos = 1;
	_curSmackBuffer = pos;

	_playingAnims[pos] = FULLMOTIONANIM;
	_curAnimFrame[pos] = 0;

	FlagShowCharacter = false;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
	TextStatus = TEXT_OFF;
	wordset(g_vm->_video2, 0, TOP * MAXX);
	g_vm->_graphicsMgr->showScreen(0, 0, MAXX, TOP);
	wordset(g_vm->_video2 + (TOP + AREA) * MAXX, 0, TOP * MAXX);
	wordset(g_vm->_video2, 0, MAXX * MAXY);
	g_vm->_graphicsMgr->showScreen(0, AREA + TOP, MAXX, TOP);
	g_vm->_graphicsMgr->unlock();

	g_vm->_gameQueue.initQueue();
	g_vm->_animQueue.initQueue();
	g_vm->_characterQueue.initQueue();
	actorStop();
	FlagMouseEnabled = false;

	CallSmackOpen(FmvFileOpen(name));

	_curSmackAction = SMACKNULL;
}

/*-----------------17/11/96 14.47-------------------
				StopFullMotion
--------------------------------------------------*/
void StopFullMotion() {
	extern int _fullMotionStart, _fullMotionEnd;

	_curSmackAction = SMACKCLOSE;
	int pos = 1;
	_curSmackBuffer = pos;

	if (_playingAnims[pos] == 0)
		return ;

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	CallSmackClose();

	_curSmackAction = SMACKNULL;

	FlagDialogActive = false;
	FlagDialogMenuActive = false;
	FlagMouseEnabled = true;
	FlagSomeOneSpeak = false;

	g_vm->_lightIcon = 0xFF;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
	if (_curDialog == dFCRED) {
		CloseSys(NULL);
		return;
	}

	if (/*( _curDialog != dF1C1 ) && */ !((_curDialog == dNEGOZIANTE1A) && (_curChoice == 185))) {
		if ((_curDialog == dF582) || (_curDialog == dFLOG) || (_curDialog == dINTRO) || (_curDialog == dF362) || (_curDialog == dC381) || (_curDialog == dF381) ||
				(_curDialog == dF491) || ((_curDialog == dC581) && !(_choice[886]._flag & OBJFLAG_DONE) && (_choice[258]._flag & OBJFLAG_DONE)) ||
		    ((_curDialog == dC5A1) && (g_vm->_room[r5A]._flag & OBJFLAG_EXTRA)))
			FlagShowCharacter = false;
		else
			RedrawRoom();

		if (_curDialog == dF582)
			SoundFadOut();
	}
}

/*-----------------26/11/96 17.58-------------------
					RegenAnim
--------------------------------------------------*/
void RegenAnim(int box) {
	for (int a = 0; a < MAXSMACK; a++) {
		if ((_playingAnims[a] != 0) && (box == BACKGROUND)) {
			if ((a == 1) && (_playingAnims[a] == FULLMOTIONANIM))
				RegenFullMotion();
			else  if (a != 1)
				RegenSmackAnim(_playingAnims[a]);
		}
	}
}

/*------------------------------------------------
					RedrawRoom
--------------------------------------------------*/
void RedrawRoom() {
	if ((_curDialog != dF321) && (_curDialog != dF431) && (_curDialog != dF4C1) && (_curDialog != dASCENSORE12) && (_curDialog != dASCENSORE13) && (_curDialog != dASCENSORE16))
		FlagShowCharacter = true;

	switch (_curDialog) {
	case dASCENSORE12:
		if (_curChoice == 3)
			StartCharacterAction(a129PARLACOMPUTERESCENDE, r13, 20, 0);
		else if (_curChoice == 4)
			StartCharacterAction(a129PARLACOMPUTERESCENDE, r16, 20, 0);
		break;

	case dASCENSORE13:
		if (_curChoice == 17)
			StartCharacterAction(a139CHIUDONOPORTESU, r12, 20, 0);
		else if (_curChoice == 18)
			StartCharacterAction(a1316CHIUDONOPORTEGIU, r16, 20, 0);
		break;

	case dASCENSORE16:
		if (_curChoice == 32)
			StartCharacterAction(a1616SALECONASCENSORE, r12, 20, 0);
		else if (_curChoice == 33)
			StartCharacterAction(a1616SALECONASCENSORE, r13, 20, 0);
		break;
	}

	g_vm->_curSortTableNum = 0;
	memset(OldObjStatus, 0, MAXOBJINROOM);
	memset(VideoObjStatus, 0, MAXOBJINROOM);

	wordset(g_vm->_video2, 0, CurRoomMaxX * MAXY);
	if (g_vm->_room[g_vm->_curRoom]._bkgAnim)
		MCopy(ImagePointer, SmackImagePointer, CurRoomMaxX * AREA);
	MCopy(g_vm->_video2 + TOP * CurRoomMaxX, ImagePointer, CurRoomMaxX * AREA);

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim)
		StartSmackAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);

	if ((g_vm->_curRoom == r4P) && (_curDialog == dF4PI)) {
		memset(_smackBuffer[0], 0, SCREENLEN * AREA);
		CallSmackGoto(0, 21);
	}

	RegenRoom();

	TextStatus = TEXT_OFF;
	FlagPaintCharacter = true;
	PaintScreen(1);
	g_vm->_graphicsMgr->showScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
