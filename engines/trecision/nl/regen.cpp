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

#include "trecision/3d.h"
#include "trecision/actor.h"
#include "trecision/anim.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"

namespace Trecision {

SDObj DObj;

void PaintScreen(bool flag) {
	g_vm->_animTypeMgr->next();

	g_vm->_actorRect = nullptr;
	g_vm->_dirtyRects.clear();
	g_vm->_flagPaintCharacter = true; // always redraws the character

	int x1 = g_vm->_actor->_lim[0];
	int y1 = g_vm->_actor->_lim[2] - TOP;
	int x2 = g_vm->_actor->_lim[1];
	int y2 = g_vm->_actor->_lim[3] - TOP;

	// erase character
	if (g_vm->_flagShowCharacter && x2 > x1 && y2 > y1) {                    // if a description exists
		DObj.rect = Common::Rect(0, TOP, MAXX, AREA + TOP);
		DObj.l = Common::Rect(x1, y1, x2, y2);
		DObj.objIndex = -1;
		DObj.drawMask = false;
		g_vm->_graphicsMgr->DrawObj(DObj);

		g_vm->addDirtyRect(DObj.l);
		g_vm->_actorRect = &g_vm->_dirtyRects.back();
	} else if (g_vm->_animMgr->_animRect.left != MAXX) {
		DObj.rect = Common::Rect(0, TOP, MAXX, AREA + TOP);
		DObj.l = g_vm->_animMgr->_animRect;
		DObj.objIndex = -1;
		DObj.drawMask = false;
		g_vm->_graphicsMgr->DrawObj(DObj);

		g_vm->addDirtyRect(DObj.l);
		g_vm->_actorRect = &g_vm->_dirtyRects.back();
	}

	// CANCELLO LA SCRITTA
	if (TextStatus & TEXT_DEL) {
		// cancello scritta
		DObj.rect = Common::Rect(0, TOP, MAXX, MAXY + TOP);
		DObj.l = g_vm->_textMgr->getOldTextRect();
		DObj.l.translate(0, -TOP);
		DObj.objIndex = -1;
		DObj.drawMask = false;

		if (DObj.l.top >= 0 && DObj.l.bottom < AREA) {
			g_vm->_graphicsMgr->DrawObj(DObj);
		} else {
			g_vm->_graphicsMgr->EraseObj(DObj);
		}
		g_vm->_textMgr->clearOldText();
		g_vm->addDirtyRect(DObj.l);

		if (!(TextStatus & TEXT_DRAW))        // se non c'e' nuova scritta
			TextStatus = TEXT_OFF;               // non aggiorna piu' scritta
	}

	// CANCELLA TUTTI GLI OGGETTI TOGLI
	for (Common::List<SSortTable>::iterator i = g_vm->_sortTable.begin(); i != g_vm->_sortTable.end(); ++i) {
		if (i->_remove) {
			DObj.rect = Common::Rect(0, TOP, MAXX, AREA + TOP);

			DObj.l = g_vm->_obj[i->_objectId]._rect;
			DObj.objIndex = -1;
			DObj.drawMask = false;
			g_vm->_graphicsMgr->DrawObj(DObj);
			g_vm->addDirtyRect(DObj.l);
		}
	}

	// trova la posizione dell'omino
	g_vm->_pathFind->actorOrder();

	// PER OGNI BOX DALL'ORIZZONTE IN AVANTI...
	// COPIA PER LIVELLO
	for (int liv = _numSortPan; liv >= 0; liv--) {
		uint16 CurBox = g_vm->_pathFind->_sortPan[liv]._num;

		// disegna tutti gli oggetti e le animazioni che intersecano
		// i limiti e si riferiscono al box corrente
		PaintObjAnm(CurBox);

	}

	if (TextStatus & TEXT_DRAW) {
		g_vm->_textMgr->drawCurString();
		TextStatus = TEXT_DRAW;                 // Activate text update
	}

	g_vm->_soundMgr->SoundPasso((g_vm->_actor->_lim[1] + g_vm->_actor->_lim[0]) / 2, (g_vm->_actor->_lim[5] + g_vm->_actor->_lim[4]) / 2, g_vm->_actor->_curAction, g_vm->_actor->_curFrame, g_vm->_room[g_vm->_curRoom]._sounds);

	if (!flag && !g_vm->_flagDialogActive) {
		g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
	}

	g_vm->_sortTable.clear();

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

int getRoomObjectIndex(uint16 objectId) {
	for (uint16 index = 0; index < MAXOBJINROOM; index++) {
		const uint16 curObjId = g_vm->_room[g_vm->_curRoom]._object[index];
		if (curObjId == 0)
			return -1;
		if (curObjId == objectId)
			return index;
	}

	return -1;
}

/* -----------------12/06/97 21.35-------------------
 Disegna tutti gli oggetti e le animazioni che intersecano i limiti
 appartenenti a curbox
 --------------------------------------------------*/
void PaintObjAnm(uint16 CurBox) {
	g_vm->_animMgr->refreshAnim(CurBox);

	// disegna nuove schede appartenenti al box corrente
	for (Common::List<SSortTable>::iterator i = g_vm->_sortTable.begin(); i != g_vm->_sortTable.end(); ++i) {
		if (!i->_remove) {
			if (g_vm->_obj[i->_objectId]._nbox == CurBox) {
				// l'oggetto bitmap al livello desiderato
				SObject o = g_vm->_obj[i->_objectId];
				DObj.rect = o._rect;
				DObj.rect.translate(0, TOP);
				DObj.l = Common::Rect(DObj.rect.width(), DObj.rect.height());
				DObj.objIndex = getRoomObjectIndex(i->_objectId);
				DObj.drawMask = o._mode & OBJMODE_MASK;
				g_vm->_graphicsMgr->DrawObj(DObj);
				g_vm->_dirtyRects.push_back(DObj.rect);
			}
		}
	}

	for (DirtyRectsIterator d = g_vm->_dirtyRects.begin(); d != g_vm->_dirtyRects.end(); ++d) {
		for (int b = 0; b < MAXOBJINROOM; b++) {
			const uint16 curObject = g_vm->_room[g_vm->_curRoom]._object[b];
			if (!curObject)
				break;

			SObject obj = g_vm->_obj[curObject];

			if ((obj._mode & (OBJMODE_FULL | OBJMODE_MASK)) &&
				g_vm->isObjectVisible(curObject) &&
			    (obj._nbox == CurBox)) {

				Common::Rect r = *d;
				Common::Rect r2 = obj._rect;

				r2.translate(0, TOP);

				// Include the bottom right of the rect in the intersects() check
				r2.bottom++;
				r2.right++;

				if (r.intersects(r2)) {
					DObj.rect = obj._rect;
					DObj.rect.translate(0, TOP);

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
		g_vm->_renderer->drawCharacter(CALCPOINTS);

		int x1 = g_vm->_actor->_lim[0];
		int y1 = g_vm->_actor->_lim[2];
		int x2 = g_vm->_actor->_lim[1];
		int y2 = g_vm->_actor->_lim[3];

		if (x2 > x1 && y2 > y1) {
			// enlarge the rectangle of the character
			Common::Rect l(x1, y1, x2, y2);
			if (g_vm->_actorRect)
				g_vm->_actorRect->extend(l);

			g_vm->_renderer->resetZBuffer(x1, y1, x2, y2);
		}

		g_vm->_renderer->drawCharacter(DRAWFACES);

	} else if (_actorPos == CurBox && !g_vm->_flagDialogActive) {
		g_vm->_animMgr->refreshSmkAnim(g_vm->_animMgr->_playingAnims[kSmackerAction]);
	}
}

} // End of namespace Trecision
