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

void scene09_initScene(Scene *sc) {
	g_vars->scene09_flyingBall = 0;
	g_vars->scene09_var05 = 0;
	g_vars->scene09_glotatel = sc->getStaticANIObject1ById(ANI_GLOTATEL, -1);
	g_vars->scene09_spitter = sc->getStaticANIObject1ById(ANI_PLEVATEL, -1);
	g_vars->scene09_grit = sc->getStaticANIObject1ById(ANI_GRIT_9, -1);
	g_vars->scene09_var08 = 1;
	g_vars->scene09_var09 = 0;
	g_vars->scene09_var10 = -1;
	g_vars->scene09_var11 = -1;
	g_vars->scene09_var12 = -1000;

	while (g_vars->scene09_balls.numBalls) {
		v3 = &g_vars->scene09_balls.pHead->p0;
		v4 = g_vars->scene09_balls.pHead->p0;
		g_vars->scene09_balls.pHead = g_vars->scene09_balls.pHead->p0;
		if (g_vars->scene09_balls.pHead)
			v4->p1 = 0;
		else
			g_vars->scene09_balls.field_8 = 0;

		BallChain_init(&g_vars->scene09_balls, v3);
	}

	g_vars->scene09_var13 = 3;

	if (g_vars->scene09_hangers) {
		if (g_vars->scene09_var15 < 4) {
			v5 = g_vars->scene09_var15 + 3;

			if (g_vars->scene09_var15 + 3 <= 4)
				v5 = 4;

			v6 = operator new(16 * v5);
			memcpy(v6, g_vars->scene09_hangers, 16 * g_vars->scene09_numMovingHangers);
			Hanger_clear((Hanger *)v6 + g_vars->scene09_numMovingHangers, 4 - g_vars->scene09_numMovingHangers);

			CObjectFree(g_vars->scene09_hangers);

			g_vars->scene09_var15 = v5;
			g_vars->scene09_hangers = (Hanger *)v6;
			g_vars->scene09_numMovingHangers = 4;
		} else {
			if (g_vars->scene09_numMovingHangers < 4)
				Hanger_clear(&g_vars->scene09_hangers[g_vars->scene09_numMovingHangers], 4 - g_vars->scene09_numMovingHangers);

			g_vars->scene09_numMovingHangers = 4;
		}
	} else {
		g_vars->scene09_hangers = (Hanger *)operator new(0x40u);
		Hanger_clear(g_vars->scene09_hangers, 4);
		g_vars->scene09_var15 = 4;
		g_vars->scene09_numMovingHangers = 4;
	}

	StaticANIObject *hanger = sc->getStaticANIObject1ById(ANI_VISUNCHIK, -1);

	g_vars->scene09_hangers[0]->ani = hanger;
	g_vars->scene09_hangers[0]->phase = 0;
	g_vars->scene09_hangers[0]->field_4 = 0;
	g_vars->scene09_hangers[0]->field_8 = 0;

	int x = 75;

	for (int i = 1; x < 300; i++, x += 75)
		StaicANIObject *ani = new StaticANIObject(hanger);

		ani->show1(x + hanger->_ox, haner->_oy, MV_VSN_CYCLE2, 0);
		sc->addStaticANIObject(hanger, 1);

		g_vars->scene09_hangers[i].ani = v2;
		g_vars->scene09_hangers[i].phase = 0;
		g_vars->scene09_hangers[i].field_4 = 0;
		g_vars->scene09_hangers[i].field_8 = 0;
	}
	v11 = 0;
	if (g_vars->scene09_var07.numBalls) {
		while (1) {
			v12 = g_vars->scene09_var07.pHead;
			v13 = g_vars->scene09_var07.pHead->p0;
			g_vars->scene09_var07.pHead = g_vars->scene09_var07.pHead->p0;
			if (g_vars->scene09_var07.pHead)
				v13->p1 = 0;
			else
				g_vars->scene09_var07.field_8 = 0;
			v12->p0 = g_vars->scene09_var07.pTail;
			v14 = g_vars->scene09_var07.numBalls == 1;
			g_vars->scene09_var07.pTail = v12;
			--g_vars->scene09_var07.numBalls;
			if (v14) {
				BallChain_reset(&g_vars->scene09_var07);
				if (!g_vars->scene09_var07.numBalls)
					break;
			}
		}
	}
	v16 = Scene_getStaticANIObject1ById(sc, ANI_BALL9, -1);
	StaticANIObject_setAlpha(v16, -56);
	v17 = BallChain_sub04(&g_vars->scene09_var07, g_vars->scene09_var07.field_8, 0);
	v17->ani = v16;
	if (g_vars->scene09_var07.field_8) {
		g_vars->scene09_var07.field_8->p0 = v17;
		g_vars->scene09_var07.field_8 = v17;
		numVis = 4;
	} else {
		g_vars->scene09_var07.pHead = v17;
		g_vars->scene09_var07.field_8 = v17;
		numVis = 4;
	}
	while (1) {
		v18 = (StaticANIObject *)operator new(0xACu);
		if (v18)
			v11 = StaticANIObject_ctorCopy(v18, v16);
		StaticANIObject_setAlpha(v11, -56);
		v19 = g_vars->scene09_var07.pTail;
		v20 = g_vars->scene09_var07.field_8;
		if (!g_vars->scene09_var07.pTail) {
			v21 = (Ball_p4 *)((char *)CPlex::Create((int)&g_vars->scene09_var07.cPlex, g_vars->scene09_var07.cPlexLen, 12) + 12 * g_vars->scene09_var07.cPlexLen - 8);
			if (g_vars->scene09_var07.cPlexLen - 1 < 0) {
				v19 = g_vars->scene09_var07.pTail;
			} else {
				v22 = g_vars->scene09_var07.cPlexLen;
				v19 = g_vars->scene09_var07.pTail;
				do {
					v21->p1 = v19;
					v19 = (Ball *)v21;
					--v21;
					--v22;
				} while (v22);
				g_vars->scene09_var07.pTail = v19;
			}
		}
		g_vars->scene09_var07.pTail = v19->p0;
		v19->p1 = v20;
		v19->p0 = 0;
		++g_vars->scene09_var07.numBalls;
		v19->ani = 0;
		v19->ani = v11;
		if (g_vars->scene09_var07.field_8)
			g_vars->scene09_var07.field_8->p0 = v19;
		else
			g_vars->scene09_var07.pHead = v19;
		g_vars->scene09_var07.field_8 = v19;
		Scene_addStaticANIObject(sc, v11, 1);
		--numVis;
		if (!numVis)
			break;
		v11 = 0;
	}
	v23 = getObjectEnumState("Правая лестница_9", "Закрыта");
	setObjectState("Правая лестница_9", v23);
	v24 = getGameLoaderGameVar();
	v25 = GameVar_getSubVarByName(v24, "OBJSTATES");
	v26 = GameVar_getSubVarByName(v25, "Проглоченные яйца");
	g_vars->swallowedEgg1 = GameVar_getSubVarByName(v26, "Яйцо1");
	g_vars->swallowedEgg2 = GameVar_getSubVarByName(v26, "Яйцо2");
	g_vars->swallowedEgg3 = GameVar_getSubVarByName(v26, "Яйцо3");
	scene09_setupGrit(sc);
	initArcadeKeys("SC_9");
	lift_setButton("Этаж 1", ST_LBN_1N);
	setArcadeOverlay(PIC_CSR_ARCADE4);
}

} // End of namespace Fullpipe
