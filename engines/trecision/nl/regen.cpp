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
#include "common/scummsys.h"

#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

uint16 _curSortTableIndex = 0;

#define MAXBLOCK 5000
uint32 PaintBlock[MAXBLOCK];
int16 BlockCount;

int xr1, xr2, yr1, yr2;
int Tlim;
int VisualRef[50];

SDObj DObj;
int sflag;
int fillviola = 0;

uint32 DislVar = 0;

void PaintScreen(uint8 flag) {
	int a, liv;
	static int framenum;

	AtFrameNext();

	ContinueTalk();

	sflag = flag;

	PaintRegenRoom();

//	if( ( !FlagPaintCharacter) && ( _curSortTableNum == 0 ) &&  ( curString.sign == NULL ) && ( oldString.sign == NULL ) )
//		return;

	g_vm->_actorLimit = 255;
	Tlim = 255;
	for (a = 0; a < 20; a++)
		VisualRef[a] = 255;

	g_vm->_limitsNum = 0;
	FlagPaintCharacter = true;                      // ridisegna sempre l'omino
	AddLine(0, 0, 0);

// CANCELLA L'OMINO
	if (FlagShowCharacter) {                    // se c'era una scritta
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

		g_vm->_limits[g_vm->_limitsNum][0] = DObj.l[0]; // aggiunge rettangolo omino
		g_vm->_limits[g_vm->_limitsNum][1] = DObj.l[1] + TOP;
		g_vm->_limits[g_vm->_limitsNum][2] = DObj.l[2];
		g_vm->_limits[g_vm->_limitsNum][3] = DObj.l[3] + TOP;

		g_vm->_actorLimit = g_vm->_limitsNum;
		g_vm->_limitsNum++;
	} else if (g_vm->_animMgr->_animMinX != MAXX) {
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = CurRoomMaxX;
		DObj.dy   = AREA;
		DObj.l[0] = g_vm->_animMgr->_animMinX;
		DObj.l[1] = g_vm->_animMgr->_animMinY;
		DObj.l[2] = g_vm->_animMgr->_animMaxX;
		DObj.l[3] = g_vm->_animMgr->_animMaxY;
		DObj.buf  = ImagePointer;
		DObj.flag = COPYTORAM;
		DrawObj(DObj);

		g_vm->_limits[g_vm->_limitsNum][0] = DObj.l[0]; // aggiunge rettangolo omino
		g_vm->_limits[g_vm->_limitsNum][1] = DObj.l[1] + TOP;
		g_vm->_limits[g_vm->_limitsNum][2] = DObj.l[2];
		g_vm->_limits[g_vm->_limitsNum][3] = DObj.l[3] + TOP;

		g_vm->_actorLimit = g_vm->_limitsNum;
		g_vm->_limitsNum++;
	}

// CANCELLO LA SCRITTA
	if (TextStatus & TEXT_DEL) {
		// cancello scritta
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = CurRoomMaxX;
		DObj.dy   = 480;
		DObj.l[0] = oldString.x;
		DObj.l[1] = oldString.y - TOP;
		DObj.l[2] = DObj.l[0] + oldString.dx;
		DObj.l[3] = DObj.l[1] + oldString.dy;

		if ((oldString.y >= TOP) && ((oldString.y + oldString.dy) < (AREA + TOP))) {
			DObj.buf  = ImagePointer;
			DObj.flag = COPYTORAM;
			DrawObj(DObj);
		} else {
			for (a = (DObj.l[1] + TOP); a < (DObj.l[3] + TOP); a++)
				wordset(g_vm->_video2 + DObj.l[0] + a * CurRoomMaxX, 0x0000, (DObj.l[2] - DObj.l[0]));
		}
		oldString.sign = NULL;

		g_vm->_limits[g_vm->_limitsNum][0] = DObj.l[0]; // aggiunge rettangolo scritta
		g_vm->_limits[g_vm->_limitsNum][1] = DObj.l[1] + TOP;
		g_vm->_limits[g_vm->_limitsNum][2] = DObj.l[2];
		g_vm->_limits[g_vm->_limitsNum][3] = DObj.l[3] + TOP;

		Tlim = g_vm->_limitsNum;
		g_vm->_limitsNum++;

		if (!(TextStatus & TEXT_DRAW))        // se non c'e' nuova scritta
			TextStatus = TEXT_OFF;               // non aggiorna piu' scritta
	}

//	if( ( TextStatus & TEXT_DRAW ) && ( ( curString.sign == NULL ) || ( curString.sign[0] == '\0' ) ) )
//		TextStatus = TEXT_OFF;

// CANCELLA TUTTI GLI OGGETTI TOGLI
	for (a = 0; a < g_vm->_curSortTableNum; a++) {
		if (SortTable[a]._remove) {
			DObj.x    = 0;
			DObj.y    = TOP;
			DObj.dx   = CurRoomMaxX;
			DObj.dy   = 480;

			if (SortTable[a]._typology == TYPO_BMP) {
				DObj.l[0] = g_vm->_obj[SortTable[a]._index]._px;
				DObj.l[1] = g_vm->_obj[SortTable[a]._index]._py;
				DObj.l[2] = DObj.l[0] + g_vm->_obj[SortTable[a]._index]._dx;
				DObj.l[3] = DObj.l[1] + g_vm->_obj[SortTable[a]._index]._dy;
			}

			DObj.buf  = ImagePointer;
			DObj.flag = COPYTORAM;
			DrawObj(DObj);

			g_vm->_limits[g_vm->_limitsNum][0] = DObj.l[0]; // aggiunge rettangolo
			g_vm->_limits[g_vm->_limitsNum][1] = DObj.l[1] + TOP;
			g_vm->_limits[g_vm->_limitsNum][2] = DObj.l[2];
			g_vm->_limits[g_vm->_limitsNum][3] = DObj.l[3] + TOP;

			if ((SortTable[a + 1]._typology  == SortTable[a]._typology) &&
					(SortTable[a + 1]._roomIndex == SortTable[a]._roomIndex))
				VisualRef[a + 1] = g_vm->_limitsNum;

			g_vm->_limitsNum++;
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
		for (a = 0; a < g_vm->_limitsNum; a++) {
			if (IntersecateRect(g_vm->_limits[a][0], g_vm->_limits[a][1],
			                    g_vm->_limits[a][2], g_vm->_limits[a][3],
								curString.x, curString.y,
								curString.x + curString.dx,
								curString.y + curString.dy)) {
				curString.l[0] = xr1;
				curString.l[1] = yr1;
				curString.l[2] = xr2;
				curString.l[3] = yr2;

				curString.DText();
			}
		}

	} else if (TextStatus & TEXT_DRAW) {
		curString.DText();
		g_vm->_limits[g_vm->_limitsNum][0] = curString.x; // aggiunge rettangolo
		g_vm->_limits[g_vm->_limitsNum][1] = curString.y;
		g_vm->_limits[g_vm->_limitsNum][2] = curString.x + curString.dx;
		g_vm->_limits[g_vm->_limitsNum][3] = curString.y + curString.dy;

		Tlim = g_vm->_limitsNum;
		g_vm->_limitsNum++;

		TextStatus = TEXT_DRAW;                 // attiva aggiornamento scritta
	}

	SoundPasso((_actor._lim[1] + _actor._lim[0]) / 2, (_actor._lim[5] + _actor._lim[4]) / 2, _actor._curAction, _actor._curFrame, g_vm->_room[g_vm->_curRoom]._sounds);

	for (liv = 0; liv < g_vm->_limitsNum; liv++) {
		for (int b = g_vm->_limits[liv][1]; b < g_vm->_limits[liv][3]; b++) {
			AddLine(g_vm->_limits[liv][0], g_vm->_limits[liv][2], b);
		}
	}

	if (!flag && !FlagDialogActive) {
		// sorta i blocchi da copiare ed elimina quelli inutili
		SortBlock();
		VMouseCopy();
		for (a = 0; a < BlockCount; a++) {
//			if ( fillviola )
//				wordset( _video2+(PaintBlock[a]&0xFFFFF), 0x0e0e, ((PaintBlock[a]&0xFFF00000)>>20)&0xFFF );

			g_vm->_graphicsMgr->vCopy((PaintBlock[a] & 0xFFFFF), g_vm->_video2 + (PaintBlock[a] & 0xFFFFF), ((PaintBlock[a] & 0xFFF00000) >> 20) & 0xFFF);
		}
		VMouseRestore();
		g_vm->_graphicsMgr->unlock();

	}

	framenum ++;

	for (a = 0; a < g_vm->_curSortTableNum; a++) {
		SortTable[a]._index = 0;
		SortTable[a]._roomIndex = 0;
		SortTable[a]._typology = 0;
		SortTable[a]._curFrame = 0;
	}

	g_vm->_curSortTableNum = 0;

	FlagPaintCharacter = false;
	FlagWaitRegen = false;

	// gestione papaverina ritardata
	if ((g_vm->_curRoom == r4A) && (g_vm->_obj[oCIOCCOLATINI4A]._flag & OBJFLAG_EXTRA)) {
		if (g_vm->_animMgr->_curAnimFrame[0] > 480) {
			PlayScript(s4AHELLEN);
			g_vm->_obj[oCIOCCOLATINI4A]._flag &= ~OBJFLAG_EXTRA;
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
		SWAP(x1, x2);
	}

	PaintBlock[BlockCount++] = (uint32)((uint32)(y * CurRoomMaxX + x1) & 0xFFFFF) + ((((uint32)(x2 - x1) & 0xFFF) << 20) & 0xFFF00000);

	if (BlockCount >= MAXBLOCK) {
		warning(g_vm->_sysText[19]);
		CloseSys(g_vm->_sysText[19]);
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
	g_vm->_animMgr->refreshAnim(CurBox);

	// disegna nuove schede appartenenti al box corrente
	for (int a = 0; a < g_vm->_curSortTableNum; a++) {
		if (!SortTable[a]._remove) {
			if (SortTable[a]._typology == TYPO_BMP) {
				if (g_vm->_obj[SortTable[a]._index]._nbox == CurBox) {
					// l'oggetto bitmap al livello desiderato
					DObj.x = g_vm->_obj[SortTable[a]._index]._px;
					DObj.y = g_vm->_obj[SortTable[a]._index]._py + TOP;
					DObj.dx = g_vm->_obj[SortTable[a]._index]._dx;
					DObj.dy = g_vm->_obj[SortTable[a]._index]._dy;
					DObj.l[0] = 0;
					DObj.l[1] = 0;
					DObj.l[2] = DObj.dx;
					DObj.l[3] = DObj.dy;
					DObj.buf  = ObjPointers[SortTable[a]._roomIndex];
					DObj.mask = MaskPointers[SortTable[a]._roomIndex];
					DObj.flag = COPYTORAM;
					if (g_vm->_obj[SortTable[a]._index]._mode & OBJMODE_MASK)
						DObj.flag += DRAWMASK;
					DrawObj(DObj);

					if (VisualRef[a] == 255) {
						g_vm->_limits[g_vm->_limitsNum][0] = DObj.x; // aggiunge rettangolo
						g_vm->_limits[g_vm->_limitsNum][1] = DObj.y;
						g_vm->_limits[g_vm->_limitsNum][2] = DObj.x + DObj.dx;
						g_vm->_limits[g_vm->_limitsNum][3] = DObj.y + DObj.dy;
						g_vm->_limitsNum++;
					} else {
						if (g_vm->_limits[VisualRef[a]][0] > DObj.x)
							g_vm->_limits[VisualRef[a]][0] = DObj.x;

						if (g_vm->_limits[VisualRef[a]][1] > DObj.y)
							g_vm->_limits[VisualRef[a]][1] = DObj.y;

						if (g_vm->_limits[VisualRef[a]][2] < (DObj.x + DObj.dx))
							g_vm->_limits[VisualRef[a]][2] = DObj.x + DObj.dx;

						if (g_vm->_limits[VisualRef[a]][3] < (DObj.y + DObj.dy))
							g_vm->_limits[VisualRef[a]][3] = DObj.y + DObj.dy;
					}
				}
			}
		}
	}
	for (int a = 0; a < g_vm->_limitsNum; a++) {
		for (int b = 0; b < MAXOBJINROOM; b++) {
			uint16 curObject = g_vm->_room[g_vm->_curRoom]._object[b];

			if (!curObject)
				break;

			if ((g_vm->_obj[curObject]._mode & (OBJMODE_FULL | OBJMODE_MASK)) &&
			    (g_vm->_obj[curObject]._mode & OBJMODE_OBJSTATUS) &&
			    (g_vm->_obj[curObject]._nbox == CurBox)) {

				if (IntersecateRect(g_vm->_limits[a][0], g_vm->_limits[a][1],
				                    g_vm->_limits[a][2], g_vm->_limits[a][3],
				                    g_vm->_obj[curObject]._px, g_vm->_obj[curObject]._py + TOP,
				                    g_vm->_obj[curObject]._px + g_vm->_obj[curObject]._dx,
				                    g_vm->_obj[curObject]._py + g_vm->_obj[curObject]._dy + TOP)) {
					DObj.x = g_vm->_obj[curObject]._px;
					DObj.y = g_vm->_obj[curObject]._py + TOP;
					DObj.dx = g_vm->_obj[curObject]._dx;
					DObj.dy = g_vm->_obj[curObject]._dy;
					DObj.l[0] = xr1;
					DObj.l[1] = yr1;
					DObj.l[2] = xr2;
					DObj.l[3] = yr2;

					DObj.buf  = ObjPointers[b];
					DObj.mask = MaskPointers[b];
					DObj.flag = COPYTORAM;

					if (g_vm->_obj[curObject]._mode & OBJMODE_MASK)
						DObj.flag = COPYTORAM + DRAWMASK;

					DrawObj(DObj);
				}
			}
		}
	}

	if ((_actorPos == CurBox) &&
			((FlagShowCharacter) && (FlagCharacterExist))) {
		drawCharacter(CALCPOINTS);

		// enlarge the rectangle of the character
		if (g_vm->_limits[g_vm->_actorLimit][0] > _actor._lim[0])
			g_vm->_limits[g_vm->_actorLimit][0] = _actor._lim[0];

		if (g_vm->_limits[g_vm->_actorLimit][1] > _actor._lim[2])
			g_vm->_limits[g_vm->_actorLimit][1] = _actor._lim[2];

		if (g_vm->_limits[g_vm->_actorLimit][2] < _actor._lim[1])
			g_vm->_limits[g_vm->_actorLimit][2] = _actor._lim[1];

		if (g_vm->_limits[g_vm->_actorLimit][3] < _actor._lim[3])
			g_vm->_limits[g_vm->_actorLimit][3] = _actor._lim[3];

		ResetZB(_actor._lim[0], _actor._lim[2], _actor._lim[1], _actor._lim[3]);
		drawCharacter(DRAWFACES);

		//FlagPaintCharacter = false;
	} else if ((_actorPos == CurBox) && !FlagDialogActive) {
		g_vm->_animMgr->refreshSmkAnim(g_vm->_animMgr->_playingAnims[1]);
	}
}

int IntersecateRect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	if ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3)) {
		xr1 = (x3 > x1) ? 0 : x1 - x3;
		xr2 = MIN(x2, x4) - x3;

		yr1 = (y3 > y1) ? 0 : y1 - y3;
		yr2 = MIN(y2, y4) - y3;

		return true;
	}
	
	return false;
}

} // End of namespace Trecision
