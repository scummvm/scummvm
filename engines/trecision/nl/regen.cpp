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
#include "common/scummsys.h"

#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/video.h"

namespace Trecision {

int VisualRef[50];

SDObj DObj;

void PaintScreen(uint8 flag) {
	AtFrameNext();
	PaintRegenRoom();

	g_vm->_actorLimit = 255;
	for (int a = 0; a < 20; a++)
		VisualRef[a] = 255;

	g_vm->_limitsNum = 0;
	g_vm->_flagPaintCharacter = true; // always redraws the character

	int x1 = g_vm->_actor->_lim[0];
	int y1 = g_vm->_actor->_lim[2] - TOP;
	int x2 = g_vm->_actor->_lim[1];
	int y2 = g_vm->_actor->_lim[3] - TOP;
	
	// erase character
	if (g_vm->_flagShowCharacter && x2 > x1 && y2 > y1) {                    // if a description exists
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = MAXX;
		DObj.dy   = AREA;
		DObj.l = Common::Rect(x1, y1, x2, y2);
		DObj.objIndex = -1;
		DObj.drawMask = false;
		g_vm->_graphicsMgr->DrawObj(DObj);

		g_vm->_actorLimit = g_vm->_limitsNum;
		Common::Rect l = DObj.l;
		l.translate(0, TOP);
		g_vm->_limits[g_vm->_limitsNum++] = l;		
	} else if (g_vm->_animMgr->_animMinX != MAXX) {
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = MAXX;
		DObj.dy   = AREA;
		DObj.l = Common::Rect(g_vm->_animMgr->_animMinX, g_vm->_animMgr->_animMinY, g_vm->_animMgr->_animMaxX, g_vm->_animMgr->_animMaxY);
		DObj.objIndex = -1;
		DObj.drawMask = false;
		g_vm->_graphicsMgr->DrawObj(DObj);

		g_vm->_actorLimit = g_vm->_limitsNum;
		Common::Rect l = DObj.l;
		l.translate(0, TOP);
		g_vm->_limits[g_vm->_limitsNum++] = l;
	}

// CANCELLO LA SCRITTA
	if (TextStatus & TEXT_DEL) {
		// cancello scritta
		DObj.x    = 0;
		DObj.y    = TOP;
		DObj.dx   = MAXX;
		DObj.dy   = MAXY;
		DObj.l.left = oldString.x;
		DObj.l.top = oldString.y - TOP;
		DObj.l.right = DObj.l.left + oldString.dx;
		DObj.l.bottom = DObj.l.top + oldString.dy;
		DObj.objIndex = -1;
		DObj.drawMask = false;

		if ((oldString.y >= TOP) && ((oldString.y + oldString.dy) < (AREA + TOP))) {
			g_vm->_graphicsMgr->DrawObj(DObj);
		} else {
			for (int a = (DObj.l.top + TOP); a < (DObj.l.bottom + TOP); a++)
				memset(g_vm->_screenBuffer + DObj.l.left + a * MAXX, 0x0000, (DObj.l.right - DObj.l.left) * 2);
		}
		oldString.text = nullptr;

		g_vm->_limits[g_vm->_limitsNum].left = DObj.l.left; // aggiunge rettangolo scritta
		g_vm->_limits[g_vm->_limitsNum].top = DObj.l.top + TOP;
		g_vm->_limits[g_vm->_limitsNum].right = DObj.l.right;
		g_vm->_limits[g_vm->_limitsNum].bottom = DObj.l.bottom + TOP;

		g_vm->_limitsNum++;

		if (!(TextStatus & TEXT_DRAW))        // se non c'e' nuova scritta
			TextStatus = TEXT_OFF;               // non aggiorna piu' scritta
	}

	// CANCELLA TUTTI GLI OGGETTI TOGLI
	for (int a = 0; a < g_vm->_curSortTableNum; a++) {
		if (SortTable[a]._remove) {
			DObj.x    = 0;
			DObj.y    = TOP;
			DObj.dx   = MAXX;
			DObj.dy   = MAXY;

			if (SortTable[a]._typology == TYPO_BMP) {
				DObj.l.left = g_vm->_obj[SortTable[a]._index]._px;
				DObj.l.top = g_vm->_obj[SortTable[a]._index]._py;
				DObj.l.right = DObj.l.left + g_vm->_obj[SortTable[a]._index]._dx;
				DObj.l.bottom = DObj.l.top + g_vm->_obj[SortTable[a]._index]._dy;
			}

			DObj.objIndex = -1;
			DObj.drawMask = false;
			g_vm->_graphicsMgr->DrawObj(DObj);

			if ((SortTable[a + 1]._typology == SortTable[a]._typology) &&
				(SortTable[a + 1]._roomIndex == SortTable[a]._roomIndex))
				VisualRef[a + 1] = g_vm->_limitsNum;

			Common::Rect l(DObj.l);
			l.translate(0, TOP);
			g_vm->_limits[g_vm->_limitsNum++] = l;
		}
	}

	// trova la posizione dell'omino
	actorOrder();

	// PER OGNI BOX DALL'ORIZZONTE IN AVANTI...
	// COPIA PER LIVELLO
	for (int liv = _numSortPan; liv >= 0; liv--) {
		uint16 CurBox = _sortPan[liv]._num;

		// disegna tutti gli oggetti e le animazioni che intersecano
		// i limiti e si riferiscono al box corrente
		PaintObjAnm(CurBox);

	}

	if (TextStatus & TEXT_DRAW) {
		curString.DText();
		g_vm->_limits[g_vm->_limitsNum++] = Common::Rect(curString.x, curString.y, curString.x + curString.dx, curString.y + curString.dy);
		TextStatus = TEXT_DRAW;                 // Activate text update
	}

	g_vm->_soundMgr->SoundPasso((g_vm->_actor->_lim[1] + g_vm->_actor->_lim[0]) / 2, (g_vm->_actor->_lim[5] + g_vm->_actor->_lim[4]) / 2, g_vm->_actor->_curAction, g_vm->_actor->_curFrame, g_vm->_room[g_vm->_curRoom]._sounds);

	if (!flag && !g_vm->_flagDialogActive) {
		g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
	}

	for (int a = 0; a < g_vm->_curSortTableNum; a++) {
		SortTable[a]._index = 0;
		SortTable[a]._roomIndex = 0;
		SortTable[a]._typology = TYPO_NONE;
		SortTable[a]._curFrame = 0;
	}

	g_vm->_curSortTableNum = 0;

	g_vm->_flagPaintCharacter = false;
	g_vm->_flagWaitRegen = false;

	// Handle papaverine delayed action
	if ((g_vm->_curRoom == kRoom4A) && (g_vm->_obj[oCHOCOLATES4A]._flag & kObjFlagExtra)) {
		if (g_vm->_animMgr->smkCurFrame(kSmackerBackground) > 480) {
			g_vm->playScript(s4AHELLEN);
			g_vm->_obj[oCHOCOLATES4A]._flag &= ~kObjFlagExtra;
		}
	}
	//
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
					DObj.l = Common::Rect(DObj.dx, DObj.dy);
					DObj.objIndex = SortTable[a]._roomIndex;
					DObj.drawMask = g_vm->_obj[SortTable[a]._index]._mode & OBJMODE_MASK;
					g_vm->_graphicsMgr->DrawObj(DObj);

					Common::Rect objRect(DObj.x, DObj.y, DObj.x + DObj.dx, DObj.y + DObj.dy);
					
					if (VisualRef[a] == 255) {
						g_vm->_limits[g_vm->_limitsNum++] = objRect;
					} else {
						g_vm->_limits[VisualRef[a]].extend(objRect);
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

			SObject obj = g_vm->_obj[curObject];
			
			if ((obj._mode & (OBJMODE_FULL | OBJMODE_MASK)) &&
			    (obj._mode & OBJMODE_OBJSTATUS) &&
			    (obj._nbox == CurBox)) {

				Common::Rect r = g_vm->_limits[a];
				Common::Rect r2 = Common::Rect(
					obj._px,
					obj._py + TOP,
					obj._px + obj._dx,
					obj._py + obj._dy + TOP
				);

				// Include the bottom right of the rect in the intersects() check
				r2.bottom++;
				r2.right++;
				
				if (r.intersects(r2)) {
					DObj.x = obj._px;
					DObj.y = obj._py + TOP;
					DObj.dx = obj._dx;
					DObj.dy = obj._dy;

					// Restore the bottom right of the rect
					r2.bottom--;
					r2.right--;
					
					// TODO: Simplify this?
					const int16 xr1 = (r2.left > r.left) ? 0 : r.left - r2.left;
					const int16 yr1 = (r2.top > r.top) ? 0 : r.top - r2.top;
					const int16 xr2 = MIN<int16>(r.right, r2.right) - r2.left;
					const int16 yr2 = MIN<int16>(r.bottom, r2.bottom) - r2.top;					
					DObj.l = Common::Rect(xr1, yr1, xr2, yr2);
					DObj.objIndex = b;
					DObj.drawMask = obj._mode & OBJMODE_MASK;

					g_vm->_graphicsMgr->DrawObj(DObj);
				}
			}
		}
	}

	if (_actorPos == CurBox && g_vm->_flagShowCharacter && g_vm->_flagCharacterExists) {
		drawCharacter(CALCPOINTS);

		int x1 = g_vm->_actor->_lim[0];
		int y1 = g_vm->_actor->_lim[2];
		int x2 = g_vm->_actor->_lim[1];
		int y2 = g_vm->_actor->_lim[3];
		
		if (x2 > x1 && y2 > y1) {
			// enlarge the rectangle of the character
			Common::Rect l(x1, y1, x2, y2);
			g_vm->_limits[g_vm->_actorLimit].extend(l);

			resetZBuffer(x1, y1, x2, y2);
		}

		drawCharacter(DRAWFACES);

	} else if (_actorPos == CurBox && !g_vm->_flagDialogActive) {
		g_vm->_animMgr->refreshSmkAnim(g_vm->_animMgr->_playingAnims[kSmackerAction]);
	}
}

} // End of namespace Trecision
