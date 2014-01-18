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

Ring::Ring() {
	ani = 0;
	x = 0;
	y = 0;
	numSubRings = 0;

	for (int i = 0; i < 10; i++)
		subRing[i] = 0;

	state = false;
}

void scene37_initScene(Scene *sc) {
	Ring *ring;
	StaticANIObject *ani;

	g_vars->scene37_var01 = 200;
	g_vars->scene37_var02 = 200;
	g_vars->scene37_var03 = 300;
	g_vars->scene37_var04 = 300;
	g_vars->scene37_var06 = -1;

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 0);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRing[0] = 1;
	ring->subRing[1] = 4;
	ring->subRing[2] = 8;
	ring->state = false;
	g_vars->scene37_var05.push_back(ring);

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 1);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRing[0] = 2;
	ring->subRing[1] = 5;
	ring->subRing[2] = 9;
	ring->state = false;
	g_vars->scene37_var05.push_back(ring);

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 2);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRing[0] = 3;
	ring->subRing[1] = 7;
	ring->subRing[2] = 11;
	ring->state = false;
	g_vars->scene37_var05.push_back(ring);

	g_fp->setObjectState(sO_LeftPipe_37, g_fp->getObjectEnumState(sO_LeftPipe_37, sO_IsClosed));
	
	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	g_vars->scene37_var07 = 0;

	g_vars->scene37_plusMinus1 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 1);

	for (int i = 0; i < g_vars->scene37_var05[0]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_var05[0]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_1) == g_fp->getObjectEnumState(sO_Guard_1, sO_On)) {
			g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_vars->scene37_plusMinus2 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 2);

	for (int i = 0; i < g_vars->scene37_var05[1]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_var05[1]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_2) == g_fp->getObjectEnumState(sO_Guard_2, sO_On)) {
			g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_vars->scene37_plusMinus3 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 3);

	for (int i = 0; i < g_vars->scene37_var05[2]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_var05[2]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_3) == g_fp->getObjectEnumState(sO_Guard_3, sO_On)) {
			g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_37");
}

} // End of namespace Fullpipe
