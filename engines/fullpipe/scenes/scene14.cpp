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

void scene14_initScene(Scene *sc) {
	g_vars->scene14_grandma = sc->getStaticANIObject1ById(ANI_GRANDMA, -1);
	g_vars->scene14_var01 = 200;
	g_vars->scene14_var02 = 200;
	g_vars->scene14_var03 = 0;
	g_vars->scene14_var04 = 0;
	g_vars->scene14_var05 = 0;
	g_vars->scene14_var06 = 0;
	g_vars->scene14_var07 = 300;
	g_vars->scene14_var08 = 300;
	g_vars->scene14_pink = 0;
	g_vars->scene14_var10 = 0;
	g_vars->scene14_var11.clear();
	g_vars->scene14_var12.clear();

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_14)) {
		g_vars->scene14_var13 = 1;

		StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_BALL14, -1);

		if (ball) {
			ball->_flags &= 0xFFFB;
			g_vars->scene14_var11.push_back(ball);
		}
		
		for (uint i = 0; i < 3; i++) {
			ball = new StaticANIObject(ball); // create a copy

			ball->_flags &= 0xFFFB;
			g_vars->scene14_var11.push_back(ball);

			sc->addStaticANIObject(ball, 1);
		}
	} else {
		g_vars->scene14_var13 = 0;
		g_vars->scene14_grandma->hide();
	}

	g_fp->lift_setButton(sO_Level4, ST_LBN_4N);
	g_fp->lift_sub5(sc, QU_SC14_ENTERLIFT, QU_SC14_EXITLIFT);

	g_fp->initArcadeKeys("SC_14");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE6);
}

void scene14_setupMusic() {
	if (!g_vars->scene14_var13)
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_14"), "MUSIC2", 0);
}

int scene14_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene14_var03) {
		if (g_vars->scene14_var04) {
			g_fp->_cursorId = PIC_CSR_ARCADE2_D;
		} else {
			if (g_fp->_aniMan != g_fp->_objectAtCursor || g_fp->_aniMan->_movement || g_fp->_cursorId != PIC_CSR_DEFAULT) {
				if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV) {
					g_fp->_cursorId = PIC_CSR_DEFAULT;
				}
			} else {
				g_fp->_cursorId = PIC_CSR_ITN;
			}
		}
	}

	return g_fp->_cursorId;
}

} // End of namespace Fullpipe
