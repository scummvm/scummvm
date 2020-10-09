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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ngi/ngi.h"

#include "ngi/objectnames.h"
#include "ngi/constants.h"

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "ngi/interaction.h"
#include "ngi/behavior.h"
#include "ngi/floaters.h"

namespace NGI {

void scene20_setExits(Scene *sc) {
	int thingpar;

	if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_OnStool)
		|| g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_OnTheFloor))
		thingpar = 1;
	else if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_NearPipe)
			 || g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_NearPipeWithStool)) {
		getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing, 1);
		getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing2, 1);
		getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing3, 0);

		return;
	} else {
		thingpar = 0;
	}

	getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing, thingpar);
	getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing2, 0);
	getSc2MctlCompoundBySceneId(sc->_sceneId)->enableLinks(sO_CloseThing3, 1);
}

void scene20_initScene(Scene *sc) {
	Scene *oldsc = g_nmi->_currentScene;

	g_vars->scene20_grandma = sc->getStaticANIObject1ById(ANI_GRANDMA_20, -1);

	g_nmi->_currentScene = sc;

	if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_OnTheFloor))
		g_nmi->setObjectState(sO_Grandma, g_nmi->getObjectEnumState(sO_Grandma, sO_NearPipe));

	if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_OnStool)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_STOOL);
	} else if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_OnTheFloor)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_FLOOR);
	} else if (g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_NearPipe)
				|| g_nmi->getObjectState(sO_Grandma) == g_nmi->getObjectEnumState(sO_Grandma, sO_NearPipeWithStool)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_STAND);
	} else {
		g_vars->scene20_grandma->hide();
	}

	scene20_setExits(sc);

	g_nmi->_floaters->init(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_20"));

	for (int i = 0; i < 3; i++) {
		g_nmi->_floaters->genFlies(sc, g_nmi->_rnd.getRandomNumber(101) + 70, g_nmi->_rnd.getRandomNumber(51) + 175, 100, 0);
		g_nmi->_floaters->_array2[g_nmi->_floaters->_array2.size() - 1].val13 = g_nmi->_rnd.getRandomNumber(9);
	}

	g_nmi->_currentScene = oldsc;

	g_vars->scene20_fliesCountdown = g_nmi->_rnd.getRandomNumber(200) + 400;
}

void sceneHandler20_updateFlies() {
	int sz = g_nmi->_floaters->_array2.size();

	if (sz < 3) {
		g_nmi->_floaters->genFlies(g_nmi->_currentScene, 253, 650, 200, 0);
		g_nmi->_floaters->_array2[sz - 1].val2 = 250;
		g_nmi->_floaters->_array2[sz - 1].val3 = 200;
	} else {
		int idx = g_nmi->_rnd.getRandomNumber(sz);

		g_nmi->_floaters->_array2[idx].countdown = 0;
		g_nmi->_floaters->_array2[idx].fflags |= 4u;
		g_nmi->_floaters->_array2[idx].val2 = 250;
		g_nmi->_floaters->_array2[idx].val3 = 200;
		g_nmi->_floaters->_array2[idx].val6 = 253;
		g_nmi->_floaters->_array2[idx].val7 = 650;
		g_nmi->_floaters->_array2[idx].ani->_priority = 200;
	}

	g_vars->scene20_fliesCountdown = g_nmi->_rnd.getRandomNumber(200) + 400;
}

int sceneHandler20(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC20_UPDATELOCKABLE:
		scene20_setExits(g_nmi->_currentScene);
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		--g_vars->scene20_fliesCountdown;

		if (g_vars->scene20_fliesCountdown <= 0)
			sceneHandler20_updateFlies();

		g_nmi->_floaters->update();

		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
