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
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"

namespace Trecision {

uint16 CurSortTableIndex = 0;
int16  limiti[50][4];
uint16 limitinum;

#define MAXBLOCK 5000
uint32 PaintBlock[MAXBLOCK];
int16 BlockCount;

int xr1, xr2, yr1, yr2;
int Hlim, Tlim;
int VisualRef[50];

struct SDObj DObj;
int sflag;
extern uint16 _animMaxX, _animMinX, _animMaxY, _animMinY;
int fillviola = 0;

uint32 DislVar = 0;

void PaintScreen(uint8 flag) {
	extern FILE *fhi;
	int a, liv;
	static int framenum;

	AtFrameNext();

	ContinueTalk();

	sflag = flag;

	PaintRegenRoom();

//	if( ( !SemPaintHomo) && ( CurSortTableNum == 0 ) &&  ( TheString.sign == NULL ) && ( OldString.sign == NULL ) )
//		return;

	Hlim = 255;
	Tlim = 255;
	for (a = 0; a < 20; a++)
		VisualRef[a] = 255;

	limitinum = 0;
	SemPaintHomo = 1;                      // ridisegna sempre l'omino
	AddLine(0, 0, 0);

// CANCELLA L'OMINO
	if (SemShowHomo) {                    // se c'era una scritta
		// cancello omino
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = CurRoomMaxX;
		DObj.dy   = AREA;
		DObj.l[0] = _actor._lim[0];
		DObj.l[1] = _actor._lim[2] - TOP;
		DObj.l[2] = _actor._lim[1];
		DObj.l[3] = _actor._lim[3] - TOP;
		DObj.buf  = ImagePointer;
		DObj.flag = COPYTORAM;
		DrawObj(DObj);

		limiti[limitinum][0] = DObj.l[0];     // aggiunge rettangolo omino
		limiti[limitinum][1] = DObj.l[1] + TOP;
		limiti[limitinum][2] = DObj.l[2];
		limiti[limitinum][3] = DObj.l[3] + TOP;

		Hlim = limitinum;
		limitinum ++;
	} else if (_animMinX != MAXX) {
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = CurRoomMaxX;
		DObj.dy   = AREA;
		DObj.l[0] = _animMinX;
		DObj.l[1] = _animMinY;
		DObj.l[2] = _animMaxX;
		DObj.l[3] = _animMaxY;
		DObj.buf  = ImagePointer;
		DObj.flag = COPYTORAM;
		DrawObj(DObj);

		limiti[limitinum][0] = DObj.l[0];     // aggiunge rettangolo omino
		limiti[limitinum][1] = DObj.l[1] + TOP;
		limiti[limitinum][2] = DObj.l[2];
		limiti[limitinum][3] = DObj.l[3] + TOP;

		Hlim = limitinum;
		limitinum ++;
	}

// CANCELLO LA SCRITTA
	if (TextStatus & TEXT_DEL) {
		// cancello scritta
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = CurRoomMaxX;
		DObj.dy   = 480;
		DObj.l[0] = OldString.x;
		DObj.l[1] = OldString.y - TOP;
		DObj.l[2] = DObj.l[0] + OldString.dx;
		DObj.l[3] = DObj.l[1] + OldString.dy;

		if ((OldString.y >= TOP) && ((OldString.y + OldString.dy) < (AREA + TOP))) {
			DObj.buf  = ImagePointer;
			DObj.flag = COPYTORAM;
			DrawObj(DObj);
		} else {
			for (a = (DObj.l[1] + TOP); a < (DObj.l[3] + TOP); a++)
				wordset(Video2 + DObj.l[0] + a * CurRoomMaxX, 0x0000, (DObj.l[2] - DObj.l[0]));
		}
		OldString.sign = NULL;

		limiti[limitinum][0] = DObj.l[0];     // aggiunge rettangolo scritta
		limiti[limitinum][1] = DObj.l[1] + TOP;
		limiti[limitinum][2] = DObj.l[2];
		limiti[limitinum][3] = DObj.l[3] + TOP;

		Tlim = limitinum;
		limitinum ++;

		if (!(TextStatus & TEXT_DRAW))        // se non c'e' nuova scritta
			TextStatus = TEXT_OFF;               // non aggiorna piu' scritta
	}

//	if( ( TextStatus & TEXT_DRAW ) && ( ( TheString.sign == NULL ) || ( TheString.sign[0] == '\0' ) ) )
//		TextStatus = TEXT_OFF;

// CANCELLA TUTTI GLI OGGETTI TOGLI
	for (a = 0; a < CurSortTableNum; a++) {
		if (SortTable[a].togli == true) {
			DObj.x    = 0;
			DObj.y    = TOP;
			DObj.dx   = CurRoomMaxX;
			DObj.dy   = 480;

			if (SortTable[a].typology == TYPO_BMP) {
				DObj.l[0] = _obj[SortTable[a].index]._px;
				DObj.l[1] = _obj[SortTable[a].index]._py;
				DObj.l[2] = DObj.l[0] + _obj[SortTable[a].index]._dx;
				DObj.l[3] = DObj.l[1] + _obj[SortTable[a].index]._dy;
			}

			DObj.buf  = ImagePointer;
			DObj.flag = COPYTORAM;
			DrawObj(DObj);

			limiti[limitinum][0] = DObj.l[0];    // aggiunge rettangolo
			limiti[limitinum][1] = DObj.l[1] + TOP;
			limiti[limitinum][2] = DObj.l[2];
			limiti[limitinum][3] = DObj.l[3] + TOP;

			if ((SortTable[a + 1].typology  == SortTable[a].typology) &&
					(SortTable[a + 1].roomindex == SortTable[a].roomindex))
				VisualRef[a + 1] = limitinum;

			limitinum ++;
		}
	}

	// azzera variabili
	BlockCount = 0;
	// trova la posizione dell'omino
	actorOrder();
	// disegna gli oggetti che intersecano nel box di sfondo
	//PaintObjAnm( BACKGROUND );

	// PER OGNI BOX DALL'ORIZZONTE IN AVANTI...
	// COPIA PER LIVELLO
	for (liv = _numSortPan; liv >= 0; liv--) {
		uint16 CurBox = _sortPan[liv]._num;

		// disegna tutti gli oggetti e le animazioni che intersecano
		// i limiti e si riferiscono al box corrente
		PaintObjAnm(CurBox);

	}

	// disegna gli oggetti che intersecano nel foreground
	//PaintObjAnm( FOREGROUND );

	if (TextStatus == TEXT_ON) {
		for (a = 0; a < limitinum; a++) {
			if (IntersecateRect(limiti[a][0], limiti[a][1],
								limiti[a][2], limiti[a][3],
								TheString.x, TheString.y,
								TheString.x + TheString.dx,
								TheString.y + TheString.dy)) {
				TheString.l[0] = xr1;
				TheString.l[1] = yr1;
				TheString.l[2] = xr2;
				TheString.l[3] = yr2;

				DText(TheString);
			}
		}

	} else if (TextStatus & TEXT_DRAW) {
		DText(TheString);
		limiti[limitinum][0] = TheString.x;   // aggiunge rettangolo
		limiti[limitinum][1] = TheString.y;
		limiti[limitinum][2] = TheString.x + TheString.dx;
		limiti[limitinum][3] = TheString.y + TheString.dy;

		Tlim = limitinum;
		limitinum ++;

		TextStatus = TEXT_DRAW;                 // attiva aggiornamento scritta
	}

	SoundPasso((_actor._lim[1] + _actor._lim[0]) / 2, (_actor._lim[5] + _actor._lim[4]) / 2, _actor._curAction, _actor._curFrame, Room[_curRoom]._sounds);

	for (liv = 0; liv < limitinum; liv++) {
		for (int b = limiti[liv][1]; b < limiti[liv][3]; b++) {
			AddLine(limiti[liv][0], limiti[liv][2], b);
		}
	}

	if (!flag && !SemDialogActive) {
		// sorta i blocchi da copiare ed elimina quelli inutili
		SortBlock();
		VMouseCopy();
		for (a = 0; a < BlockCount; a++) {
//			if ( fillviola )
//				wordset( Video2+(PaintBlock[a]&0xFFFFF), 0x0e0e, ((PaintBlock[a]&0xFFF00000)>>20)&0xFFF );

			VCopy((PaintBlock[a] & 0xFFFFF), Video2 + (PaintBlock[a] & 0xFFFFF), ((PaintBlock[a] & 0xFFF00000) >> 20) & 0xFFF);
		}
		VMouseRestore();
		UnlockVideo();

//		ShowScreen(0,0,640,480);
	}

	framenum ++;

	for (a = 0; a < CurSortTableNum; a++) {
		SortTable[a].index = 0;
		SortTable[a].roomindex = 0;
		SortTable[a].typology = 0;
		SortTable[a].framecur = 0;
	}

	CurSortTableNum = 0;

	SemPaintHomo = 0;
	SemWaitRegen = false;

	// gestione papaverina ritardata
	if ((_curRoom == r4A) && (_obj[oCIOCCOLATINI4A]._flag & OBJFLAG_EXTRA)) {
		extern uint16 _curAnimFrame[];
		if (_curAnimFrame[0] > 480) {
			PlayScript(s4AHELLEN);
			_obj[oCIOCCOLATINI4A]._flag &= ~OBJFLAG_EXTRA;
		}
		return;
	}
	//
}

/* -----------------12/06/97 21.35-------------------
			Aggiunge linea a buffer da copiare
 --------------------------------------------------*/
void AddLine(int16 x1, int16 x2, int16 y) {
	if (x1 > x2) {
		int16 t = x1;
		x1 = x2;
		x2 = t;
	}

	PaintBlock[BlockCount++] = (uint32)((uint32)(y * CurRoomMaxX + x1) & 0xFFFFF) + ((((uint32)(x2 - x1) & 0xFFF) << 20) & 0xFFF00000);

	if (BlockCount >= MAXBLOCK) {
		warning(_sysSent[19]);
		CloseSys(_sysSent[19]);
	}
}

/*-----------------============-------------------
		Compare panel distance (qsort)
--------------------------------------------------*/
int BlockCompare(const void *arg1, const void *arg2) {
	uint32 p1 = *(uint32 *)arg1;
	uint32 p2 = *(uint32 *)arg2;

	if ((p1 & 0xFFFFF) > (p2 & 0xFFFFF))
		return 1;

	if ((p1 & 0xFFFFF) < (p2 & 0xFFFFF))
		return -1;

	if ((p1 & 0xFFF00000) > (p2 & 0xFFF00000))
		return -1;

	if ((p1 & 0xFFF00000) < (p2 & 0xFFF00000))
		return 1;

	return 0;
}

/*-----------------15/10/96 10.34-------------------
			Sorta i blocchi da visualizzare
--------------------------------------------------*/
void SortBlock() {
// si possono verificare 4 casi
// 1) x1 < a1 e a1 < x2 < a2 => a1 = x1
// 2) a1 < x1 < a2 e x2 > a2 => a2 = x2
// 3) a1 < x1 < x2 < a2 => break
// 4) x1 < a1 e x2 > a2 => a1 = x1 e a2 = x2
	uint32 x1, x2, c;

	qsort(&PaintBlock[0], BlockCount, sizeof(uint32), BlockCompare);

	for (c = 0; c < BlockCount;) {
		uint32 a1 = (PaintBlock[c] & 0xFFFFF);
		uint32 a2 = a1 + (((PaintBlock[c] & 0xFFF00000) >> 20) & 0xFFF);
		uint32 oc = c;
		c ++;

		// finche' il blocco seguente non e' completamente staccato dal blocco attuale
		while ((c < BlockCount) && ((x1 = (PaintBlock[c] & 0xFFFFF)) < a2)) {
			// se allarga il blocco precedente aggiorna
			if ((x2 = x1 + (((PaintBlock[c] & 0xFFF00000) >> 20) & 0xFFF)) > a2)
				a2 = x2;
			// altrimenti cancella il blocco
			else
				PaintBlock[c] = 0xFFFFFFFF;

			c ++;
		}
		PaintBlock[oc] = (uint32)((((uint32)a1) & 0xFFFFF) + ((((uint32)(a2 - a1) & 0xFFF) << 20) & 0xFFF00000));
	}

	qsort(&PaintBlock[0], BlockCount, sizeof(uint32), BlockCompare);

	// leva i blocchi che non servono
	for (c = 0; c < BlockCount; c++)
		if (PaintBlock[c] == 0xFFFFFFFF)
			break;

	BlockCount = c;
}

/* -----------------12/06/97 21.35-------------------
 Disegna tutti gli oggetti e le animazioni che intersecano i limiti
 appartenenti a curbox
 --------------------------------------------------*/
void PaintObjAnm(uint16 CurBox) {
	int a, b;
	uint16 TheObject;
	extern uint16 _playingAnims[];

	RegenAnim(CurBox);

	// disegna nuove schede appartenenti al box corrente
	for (a = 0; a < CurSortTableNum; a++) {
		if (!SortTable[a].togli) {
			if (SortTable[a].typology == TYPO_BMP) {
				if (_obj[SortTable[a].index]._nbox == CurBox) {
					// l'oggetto bitmap Š al livello desiderato
					DObj.x    = _obj[SortTable[a].index]._px;
					DObj.y    = _obj[SortTable[a].index]._py + TOP;
					DObj.dx   = _obj[SortTable[a].index]._dx;
					DObj.dy   = _obj[SortTable[a].index]._dy;
					DObj.l[0] = 0;
					DObj.l[1] = 0;
					DObj.l[2] = DObj.dx;
					DObj.l[3] = DObj.dy;
					DObj.buf  = ObjPointers[SortTable[a].roomindex];
					DObj.mask = MaskPointers[SortTable[a].roomindex];
					DObj.flag = COPYTORAM;
					if (_obj[SortTable[a].index]._mode & OBJMODE_MASK)
						DObj.flag += DRAWMASK;
					DrawObj(DObj);

					if (VisualRef[a] == 255) {
						limiti[limitinum][0] = DObj.x;    // aggiunge rettangolo
						limiti[limitinum][1] = DObj.y;
						limiti[limitinum][2] = DObj.x + DObj.dx;
						limiti[limitinum][3] = DObj.y + DObj.dy;
						limitinum ++;
					} else {
						if (limiti[VisualRef[a]][0] > DObj.x)
							limiti[VisualRef[a]][0] = DObj.x;

						if (limiti[VisualRef[a]][1] > DObj.y)
							limiti[VisualRef[a]][1] = DObj.y;

						if (limiti[VisualRef[a]][2] < (DObj.x + DObj.dx))
							limiti[VisualRef[a]][2] = DObj.x + DObj.dx;

						if (limiti[VisualRef[a]][3] < (DObj.y + DObj.dy))
							limiti[VisualRef[a]][3] = DObj.y + DObj.dy;
					}
				}
			}
		}
	}
	for (a = 0; a < limitinum; a++) {
		for (b = 0; b < MAXOBJINROOM; b++) {
			TheObject = Room[_curRoom]._object[b];

			if (!TheObject)
				break;

			if ((_obj[TheObject]._mode & (OBJMODE_FULL | OBJMODE_MASK)) &&
					(_obj[TheObject]._mode & OBJMODE_OBJSTATUS) &&
					(_obj[TheObject]._nbox == CurBox)) {

				if (IntersecateRect(limiti[a][0], limiti[a][1],
									limiti[a][2], limiti[a][3],
									_obj[TheObject]._px, _obj[TheObject]._py + TOP,
									_obj[TheObject]._px + _obj[TheObject]._dx,
									_obj[TheObject]._py + _obj[TheObject]._dy + TOP)) {
					DObj.x    = _obj[TheObject]._px;
					DObj.y    = _obj[TheObject]._py + TOP;
					DObj.dx   = _obj[TheObject]._dx;
					DObj.dy   = _obj[TheObject]._dy;
					DObj.l[0] = xr1;
					DObj.l[1] = yr1;
					DObj.l[2] = xr2;
					DObj.l[3] = yr2;

					DObj.buf  = ObjPointers[b];
					DObj.mask = MaskPointers[b];
					DObj.flag = COPYTORAM;

					if (_obj[TheObject]._mode & OBJMODE_MASK)
						DObj.flag = COPYTORAM + DRAWMASK;

					DrawObj(DObj);
				}
			}
		}
	}

	if ((_actorPos == CurBox) &&
			((SemShowHomo) && (SemCharacterExist))) {
		drawCharacter(CALCPOINTS);

		// allarga il rettangolo dell'omino
		if (limiti[Hlim][0] > _actor._lim[0])
			limiti[Hlim][0] = _actor._lim[0];

		if (limiti[Hlim][1] > _actor._lim[2])
			limiti[Hlim][1] = _actor._lim[2];

		if (limiti[Hlim][2] < _actor._lim[1])
			limiti[Hlim][2] = _actor._lim[1];

		if (limiti[Hlim][3] < _actor._lim[3])
			limiti[Hlim][3] = _actor._lim[3];

		ResetZB(_actor._lim[0], _actor._lim[2], _actor._lim[1], _actor._lim[3]);
		drawCharacter(DRAWFACES);

		//SemPaintHomo = 0;
	} else if ((_actorPos == CurBox) && !(SemDialogActive)) {
		RegenSmackAnim(_playingAnims[1]);
	}
}

int IntersecateRect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	if ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3)) {
		if (x3 > x1)
			xr1 = 0;                             // 0
		else
			xr1 = x1 - x3;

		if (x2 < x4)
			xr2 = x2 - x3;
		else
			xr2 = x4 - x3;                       // _dx

		if (y3 > y1)
			yr1 = 0;                             // 0
		else
			yr1 = y1 - y3;

		if (y2 < y4)
			yr2 = y2 - y3;
		else
			yr2 = y4 - y3;                       // _dy

		return true;
	} else
		return false;
}

} // End of namespace Trecision
