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

struct Hanger {
	StaticANIObject *ani;
	int field_4;
	int field_8;
	int phase;
};

void scene09_setupGrit(Scene *sc) {
	if (g_vars->scene09_grit->_statics->_staticsId == ST_GRT9_GRIT) {
		if (!getGameLoaderInventory()->getCountItemsWithId(ANI_INV_COIN)) {
			if (g_fp->getObjectState(sO_CoinSlot_1) == g_fp->getObjectEnumState(sO_CoinSlot_1, sO_Empty)
				&& (g_vars->swallowedEgg1->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg2->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg3->_value.intValue == ANI_INV_EGGBOOT)) {
				Scene *oldsc = g_fp->_currentScene;
				g_fp->_currentScene = sc;
				g_vars->scene09_grit->changeStatics2(ST_GRT9_NORM);
				g_fp->_currentScene = oldsc;
			}
		}
	}
}

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
		Ball *b = g_vars->scene09_balls.pHead->p0;

		g_vars->scene09_balls.pHead = g_vars->scene09_balls.pHead->p0;

		if (g_vars->scene09_balls.pHead)
			g_vars->scene09_balls.pHead->p0->p1 = 0;
		else
			g_vars->scene09_balls.field_8 = 0;

		g_vars->scene09_balls.init(&b);
	}

	g_vars->scene09_var13 = 3;

	g_vars->scene09_hangers.clear();
	g_vars->scene09_var15 = 4;
	g_vars->scene09_numMovingHangers = 4;

	StaticANIObject *hanger = sc->getStaticANIObject1ById(ANI_VISUNCHIK, -1);
	Hanger *hng = new Hanger;

	hng->ani = hanger;
	hng->phase = 0;
	hng->field_4 = 0;
	hng->field_8 = 0;

	g_vars->scene09_hangers.push_back(hng);

	int x = 75;

	for (int i = 1; x < 300; i++, x += 75) {
		StaticANIObject *ani = new StaticANIObject(hanger);

		ani->show1(x + hanger->_ox, hanger->_oy, MV_VSN_CYCLE2, 0);
		sc->addStaticANIObject(hanger, 1);

		hng = new Hanger;

		hng->ani = ani;
		hng->phase = 0;
		hng->field_4 = 0;
		hng->field_8 = 0;

		g_vars->scene09_hangers.push_back(hng);
	}

	while (g_vars->scene09_var07.numBalls) {
		Ball *ohead = g_vars->scene09_var07.pHead;

		g_vars->scene09_var07.pHead = g_vars->scene09_var07.pHead->p0;

		if (g_vars->scene09_var07.pHead)
			ohead->p0->p1 = 0;
		else
			g_vars->scene09_var07.field_8 = 0;

		ohead->p0 = g_vars->scene09_var07.pTail;

		g_vars->scene09_var07.pTail = ohead;

		g_vars->scene09_var07.numBalls--;
	}

	g_vars->scene09_var07.reset();

	Ball *b9 = g_vars->scene09_var07.sub04(g_vars->scene09_var07.field_8, 0);
	b9->ani = sc->getStaticANIObject1ById(ANI_BALL9, -1);
	b9->ani->setAlpha(0xc8);

	if (g_vars->scene09_var07.field_8) {
		g_vars->scene09_var07.field_8->p0 = b9;
		g_vars->scene09_var07.field_8 = b9;
	} else {
		g_vars->scene09_var07.pHead = b9;
		g_vars->scene09_var07.field_8 = b9;
	}

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newball = new StaticANIObject(b9->ani);

		newball->setAlpha(0xc8);

		Ball *runPtr = g_vars->scene09_var07.pTail;
		Ball *lastP = g_vars->scene09_var07.field_8;

		if (!g_vars->scene09_var07.pTail) {
			g_vars->scene09_var07.cPlex = (byte *)calloc(g_vars->scene09_var07.cPlexLen, sizeof(Ball));

			byte *p1 = g_vars->scene09_var07.cPlex + (g_vars->scene09_var07.cPlexLen - 1) * sizeof(Ball);

			if (g_vars->scene09_var07.cPlexLen - 1 < 0) {
				runPtr = g_vars->scene09_var07.pTail;
			} else {
				runPtr = g_vars->scene09_var07.pTail;

				for (int j = 0; j < g_vars->scene09_var07.cPlexLen; j++) {
					((Ball *)p1)->p1 = runPtr;
					runPtr = (Ball *)p1;

					p1 -= sizeof(Ball);
				}

				g_vars->scene09_var07.pTail = runPtr;
			}
		}

		g_vars->scene09_var07.pTail = runPtr->p0;
		runPtr->p1 = lastP;
		runPtr->p0 = 0;
		runPtr->ani = newball;

		g_vars->scene09_var07.numBalls++;

		if (g_vars->scene09_var07.field_8)
			g_vars->scene09_var07.field_8->p0 = runPtr;
		else
			g_vars->scene09_var07.pHead = runPtr;

		g_vars->scene09_var07.field_8 = runPtr;

		sc->addStaticANIObject(newball, 1);
	}

	g_fp->setObjectState(sO_RightStairs_9, g_fp->getObjectEnumState(sO_RightStairs_9, sO_IsClosed));

	GameVar *eggvar = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = eggvar->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = eggvar->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = eggvar->getSubVarByName(sO_Egg3);

	scene09_setupGrit(sc);

	g_fp->initArcadeKeys("SC_9");

	g_fp->lift_setButton(sO_Level1, ST_LBN_1N);

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE4);
}

} // End of namespace Fullpipe
