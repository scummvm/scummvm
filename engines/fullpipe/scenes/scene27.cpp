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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"

#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/interaction.h"
#include "fullpipe/behavior.h"


namespace Fullpipe {

void scene27_initScene(Scene *sc) {
	g_vars->scene27_var01 = 200;
	g_vars->scene27_var02 = 200;
	g_vars->scene27_var03 = 300;
	g_vars->scene27_var04 = 300;
	g_vars->scene27_hitZone = sc->getPictureObjectById(PIC_SC27_HITZONE2, 0);
	g_vars->scene27_driver = sc->getStaticANIObject1ById(ANI_VODILLA, -1);
	g_vars->scene27_maid = sc->getStaticANIObject1ById(ANI_MAID, -1);
	g_vars->scene27_batHandler = sc->getStaticANIObject1ById(ANI_BITAHANDLER, -1);

	g_vars->scene27_balls.numBalls = 0;
	g_vars->scene27_balls.pTail = 0;
	g_vars->scene27_balls.field_8 = 0;
	g_vars->scene27_balls.pHead = 0;
	CPlex::FreeDataChain(g_vars->scene27_balls.cPlex);
	g_vars->scene27_balls.cPlex = 0;
	scene27_bats.clear();
	scene27_var07.clear();

	g_vars->scene27_var15 = 1;
	g_vars->scene27_bat = sc->getStaticANIObject1ById(ANI_BITA, -1);

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newbat = new StaticANIObject(g_vars->scene27_bat);

		v5 = g_vars->scene27_balls.pTail;
		v6 = g_vars->scene27_balls.field_8;

		if (!g_vars->scene27_balls.pTail) {
			v7 = CPlex::Create(&g_vars->scene27_balls.cPlex, g_vars->scene27_balls.cPlexLen, 12) + 4 + 12 * g_vars->scene27_balls.cPlexLen - 12;
			if (g_vars->scene27_balls.cPlexLen - 1 < 0) {
				v5 = g_vars->scene27_balls.pTail;
			} else {
				v8 = g_vars->scene27_balls.cPlexLen;
				v5 = g_vars->scene27_balls.pTail;
				do {
					*(_DWORD *)v7 = v5;
					v5 = (Ball *)v7;
					v7 -= 12;
					--v8;
				} while (v8);

				g_vars->scene27_balls.pTail = v5;
			}
		}

		g_vars->scene27_balls.pTail = v5->pNext;
		v5->pPrev = v6;
		v5->pNext = 0;
		++g_vars->scene27_balls.numBalls;
		v5->ani = 0;
		v5->ani = newbat;

		if (g_vars->scene27_balls.field_8)
			g_vars->scene27_balls.field_8->pNext = v5;
		else
			g_vars->scene27_balls.pHead = v5;
		g_vars->scene27_balls.field_8 = v5;

		sc->addStaticANIObject(newbat, 1);
	}

	g_vars->scene27_var08 = 0;
	g_vars->scene27_var09 = 0;
	g_vars->scene27_var10 = 0;
	g_vars->scene27_var11 = 0;
	g_vars->scene27_var12 = 0;
	g_vars->scene27_var13 = 0;
	g_vars->scene27_launchPhase = 0;

	oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSwab)) {
		StaticANIObject_changeStatics2(g_vars->scene27_maid, ST_MID_SWAB2);
	} else if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithBroom)) {
		StaticANIObject_changeStatics2(g_vars->scene27_maid, ST_MID_BROOM);
	} else if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSpade)) {
		StaticANIObject_changeStatics2(g_vars->scene27_maid, ST_MID_SPADE);
	}

	g_fp->_currentScene = oldsc;

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE7);
}

int scene27_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene27_var08) {
		if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV)
			g_fp->_cursorId = PIC_CSR_ARCADE7_D;

	} else if (g_fp->_objectIdAtCursor == ANI_MAN) {
		if (g_vars->scene27_var09)
			if (g_fp->_cursorId == PIC_CSR_DEFAULT)
				g_fp->_cursorId = PIC_CSR_ITN;
	}

	return g_fp->_cursorId;
}

} // End of namespace Fullpipe
