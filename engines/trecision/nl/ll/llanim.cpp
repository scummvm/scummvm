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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

// locals
#define SMACKNULL	0
#define SMACKOPEN	1
#define SMACKCLOSE  2
#define FULLMOTIONANIM	620

namespace Trecision {

void CallSmackOpen(const char *name);
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
uint16 _newData[260];
uint32 _newData2[260];

/*-----------------29/04/96 16.38-------------------
					BCopy
--------------------------------------------------*/
void BCopy(uint32 Sco, uint8 *Src, uint32 Len) {
	extern bool _linearMode;
	uint32 CopyNow;

	LockVideo();
	if ((Video == NULL) || (Len == 0))
		return ;

	if (_linearMode && ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))) {
		byte2word(Video + Sco, Src, _newData, Len);
		return ;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 EndSco = Sco + Len;

	int32 y2 = EndSco / SCREENLEN;

	uint32 SrcSco = 0;

	if (Len > SCREENLEN - x1)
		CopyNow = SCREENLEN - x1;
	else
		CopyNow = Len;

	byte2word(Video + y1 * (VideoPitch / 2) + x1, Src + SrcSco, _newData, CopyNow);
	SrcSco += CopyNow;
	Len -= CopyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		CopyNow = SCREENLEN;
		byte2word(Video + i * (VideoPitch / 2), Src + SrcSco, _newData, CopyNow);
		SrcSco += CopyNow;
		Len -= CopyNow;
	}

	if (Len > 0) {
		CopyNow = Len;
		byte2word(Video + y2 * (VideoPitch / 2), Src + SrcSco, _newData, CopyNow);
		// Useless assignations, removed
		// SrcSco += CopyNow;
		// Len -= CopyNow;
	}
}
/*-----------------29/04/96 16.38-------------------
					DCopy
--------------------------------------------------*/
void DCopy(uint32 Sco, uint8 *Src, uint32 Len) {
	extern bool _linearMode;

	LockVideo();
	if ((Video == NULL) || (Len == 0))
		return ;

	if (_linearMode && ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))) {
		byte2long(Video + Sco, Src, _newData2, Len / 2);
		return ;
	}

	int32 x1 = Sco % SCREENLEN;
	int32 y1 = Sco / SCREENLEN;

	uint32 EndSco = Sco + Len;

	int32 y2 = EndSco / SCREENLEN;

	uint32 SrcSco = 0;

	uint32 CopyNow;
	if (Len > SCREENLEN - x1)
		CopyNow = SCREENLEN - x1;
	else
		CopyNow = Len;

	byte2long(Video + y1 * (VideoPitch / 2) + x1, Src + SrcSco, _newData2, CopyNow / 2);
	SrcSco += CopyNow;
	Len -= CopyNow;

	for (int32 i = (y1 + 1); i <= (y2 - 1); i++) {
		CopyNow = SCREENLEN;
		byte2long(Video + i * (VideoPitch / 2), Src + SrcSco, _newData2, CopyNow / 2);
		SrcSco += CopyNow;
		Len -= CopyNow;
	}

	if (Len > 0) {
		CopyNow = Len;
		byte2long(Video + y2 * (VideoPitch / 2), Src + SrcSco, _newData2, CopyNow / 2);
		// Useless assignment, removed
		// SrcSco += CopyNow;
		// Len -= CopyNow;
	}
}

/*-----------------17/11/96 14.46-------------------
					StartSmackAnim
--------------------------------------------------*/
void StartSmackAnim(uint16 num) {
	extern char UStr[];
	int pos;

	_curSmackAction = SMACKOPEN;

	// sceglie quale buffer usare
	if (AnimTab[num].flag & SMKANIM_BKG)
		pos = 0;
	else if (AnimTab[num].flag & SMKANIM_ICON)
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
	if (AnimTab[num].flag & SMKANIM_BKG) {
		sprintf(UStr, "Bkg\\%s", AnimTab[num].name);
		CallSmackOpen((const char *)AnimFileOpen(AnimTab[num].name));

		// spegne quelle che non servono
		if ((num == aBKG11) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG14) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG1C) && (_obj[oFAX17]._flag & OBJFLAG_EXTRA)) {
			AnimTab[num].flag |= SMKANIM_OFF1;
			CallSmackVolumePan(0, 1, 0);
		} else if ((num == aBKG1D) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG22) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG48) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG4P) && (AnimTab[num].flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG28) && (AnimTab[num].flag & SMKANIM_OFF4))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG37) && (!(Room[_curRoom]._flag & OBJFLAG_EXTRA)))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG2E) && (AnimTab[num].flag & SMKANIM_OFF2))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG2G) && (_choice[556]._flag & OBJFLAG_DONE))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG34) &&				// se sono nella 34 e
				 ((_choice[616]._flag & OBJFLAG_DONE) ||		// ho gia' fatto fmv o
				  (_obj[oTUBOT34]._mode & OBJMODE_OBJSTATUS)	||	// c'e' il tubo intero
				  (_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS) ||	// c'e' il tubo fuori o
				  (_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)))	// la valovola e' chiusa
			CallSmackVolumePan(0, 2, 0);
	} else if (AnimTab[num].flag & SMKANIM_ICON) {
		sprintf(UStr, "Icon\\%s", AnimTab[num].name);
		CallSmackOpen((const char *)AnimFileOpen(AnimTab[num].name));
	} else {
		extern uint32 NextRefresh;
		uint32 st = ReadTime();

		sprintf(UStr, "Anim\\%s", AnimTab[num].name);
		CallSmackOpen((const char *)AnimFileOpen(AnimTab[num].name));
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
		if (AnimTab[num].flag & SMKANIM_BKG)
			pos = 0;
		else if (AnimTab[num].flag & SMKANIM_ICON)
			pos = 2;
		else
			pos = 1;
	}

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	_curSmackBuffer = pos;
	CallSmackClose();

	_curSmackAction = SMACKNULL;

	LightIcon = -1;
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
	extern int FullStart, FullEnd;

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

	SemShowHomo = 0;
	FullStart = 0;
	FullEnd = 0;
	TextStatus = TEXT_OFF;
	wordset(Video2, 0, TOP * MAXX);
	ShowScreen(0, 0, MAXX, TOP);
	wordset(Video2 + (TOP + AREA)*MAXX, 0, TOP * MAXX);
	wordset(Video2, 0, MAXX * MAXY);
	ShowScreen(0, AREA + TOP, MAXX, TOP);
	UnlockVideo();

	InitQueue(&Game);
	InitQueue(&Anim);
	InitQueue(&Homo);
	actorStop();
	SemMouseEnabled = false;

	CallSmackOpen((const char *)FmvFileOpen(name));

	_curSmackAction = SMACKNULL;
}

/*-----------------17/11/96 14.47-------------------
				StopFullMotion
--------------------------------------------------*/
void StopFullMotion() {
	extern int FullStart, FullEnd;
	int pos;

	_curSmackAction = SMACKCLOSE;

	pos = 1;
	_curSmackBuffer = pos;

	if (_playingAnims[pos] == 0)
		return ;

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	CallSmackClose();

	_curSmackAction = SMACKNULL;

	SemDialogActive = false;
	SemDialogMenuActive = false;
	SemMouseEnabled = true;
	SemSomeOneSpeak = false;

	LightIcon = -1;
	FullStart = 0;
	FullEnd = 0;
	if (_curDialog == dFCRED) {
		CloseSys(NULL);
		return;
	}

	if (/*( _curDialog != dF1C1 ) && */ !((_curDialog == dNEGOZIANTE1A) && (_curChoice == 185))) {
		if ((_curDialog == dF582) || (_curDialog == dFLOG) || (_curDialog == dINTRO) || (_curDialog == dF362) || (_curDialog == dC381) || (_curDialog == dF381) ||
				(_curDialog == dF491) || ((_curDialog == dC581) && !(_choice[886]._flag & OBJFLAG_DONE) && (_choice[258]._flag & OBJFLAG_DONE)) ||
				((_curDialog == dC5A1) && (Room[r5A]._flag & OBJFLAG_EXTRA)))
			SemShowHomo = 0;
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
		SemShowHomo = true;

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

	CurSortTableNum = 0;
	memset(OldObjStatus, 0, MAXOBJINROOM);
	memset(VideoObjStatus, 0, MAXOBJINROOM);

	wordset(Video2, 0, CurRoomMaxX * MAXY);
	if (Room[_curRoom]._bkgAnim)
		MCopy(ImagePointer, SmackImagePointer, CurRoomMaxX * AREA);
	MCopy(Video2 + TOP * CurRoomMaxX, ImagePointer, CurRoomMaxX * AREA);

	if (Room[_curRoom]._bkgAnim)
		StartSmackAnim(Room[_curRoom]._bkgAnim);

	if ((_curRoom == r4P) && (_curDialog == dF4PI)) {
		memset(SmackBuffer[0], 0, SCREENLEN * AREA);
		CallSmackGoto(0, 21);
	}

	RegenRoom();

	TextStatus = TEXT_OFF;
	SemPaintHomo = 1;
	PaintScreen(1);
	ShowScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
