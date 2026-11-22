/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

	if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_OnStool)
		|| g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_OnTheFloor))
		thingpar = 1;
	else if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_NearPipe)
			 || g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_NearPipeWithStool)) {
		getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing, true);
		getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing2, true);
		getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing3, false);

		return;
	} else {
		thingpar = 0;
	}

	getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing, thingpar);
	getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing2, false);
	getSc2MctlCompoundBySceneId(sc->_sceneId)->lockLink(sO_CloseThing3, true);
}

void scene20_initScene(Scene *sc) {
	Scene *oldsc = g_nmi->_currentScene;

	g_vars->scene20_grandma = sc->findAniObjectById(ANI_GRANDMA_20, -1);

	g_nmi->_currentScene = sc;

	if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_OnTheFloor))
		g_nmi->setState(sO_Grandma, g_nmi->getStateValue(sO_Grandma, sO_NearPipe));

	if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_OnStool)) {
		g_vars->scene20_grandma->setStatic(ST_GMA20_STOOL);
	} else if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_OnTheFloor)) {
		g_vars->scene20_grandma->setStatic(ST_GMA20_FLOOR);
	} else if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_NearPipe)
				|| g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_NearPipeWithStool)) {
		g_vars->scene20_grandma->setStatic(ST_GMA20_STAND);
	} else {
		g_vars->scene20_grandma->hide();
	}

	scene20_setExits(sc);

	g_nmi->_floaters->init(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_20"));

	for (int i = 0; i < 3; i++) {
		g_nmi->_floaters->addFly(sc, g_nmi->_rnd.getRandomNumber(101) + 70, g_nmi->_rnd.getRandomNumber(51) + 175, 100, 0);
		g_nmi->_floaters->_flies[g_nmi->_floaters->_flies.size() - 1].idleTicks = g_nmi->_rnd.getRandomNumber(9);
	}

	g_nmi->_currentScene = oldsc;

	g_vars->scene20_fliesCountdown = g_nmi->_rnd.getRandomNumber(200) + 400;
}

void sceneHandler20_updateFlies() {
	int sz = g_nmi->_floaters->_flies.size();

	if (sz < 3) {
		g_nmi->_floaters->addFly(g_nmi->_currentScene, 253, 650, 200, 0);
		g_nmi->_floaters->_flies[sz - 1].targetX = 250;
		g_nmi->_floaters->_flies[sz - 1].targetY = 200;
	} else {
		int idx = g_nmi->_rnd.getRandomNumber(sz);

		g_nmi->_floaters->_flies[idx].countdown = 0;
		g_nmi->_floaters->_flies[idx].flags |= 4u;
		g_nmi->_floaters->_flies[idx].targetX = 250;
		g_nmi->_floaters->_flies[idx].targetY = 200;
		g_nmi->_floaters->_flies[idx].exitX = 253;
		g_nmi->_floaters->_flies[idx].exitY = 650;
		g_nmi->_floaters->_flies[idx].ani->_z = 200;
	}

	g_vars->scene20_fliesCountdown = g_nmi->_rnd.getRandomNumber(200) + 400;
}

int sceneHandler20(Command *cmd) {
	if (cmd->_cmd != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC20_UPDATELOCKABLE:
		scene20_setExits(g_nmi->_currentScene);
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_x;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		--g_vars->scene20_fliesCountdown;

		if (g_vars->scene20_fliesCountdown <= 0)
			sceneHandler20_updateFlies();

		g_nmi->_floaters->onUpdate();

		g_nmi->_behaviorManager->update();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
