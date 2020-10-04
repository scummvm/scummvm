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

#include "ngi/objects.h"
#include "ngi/objectnames.h"
#include "ngi/constants.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/scene.h"
#include "ngi/floaters.h"
#include "ngi/messages.h"
#include "ngi/statics.h"
#include "ngi/behavior.h"

namespace NGI {

void scene12_initScene(Scene *sc) {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_12");
	g_nmi->_floaters->init(var);

	g_vars->scene12_fly = g_nmi->getObjectState(sO_Fly_12);

	if (g_vars->scene12_fly)
		g_vars->scene12_flyCountdown = g_nmi->_rnd.getRandomNumber(600) + 600;

	g_nmi->setObjectState(sO_Fly_12, g_nmi->_rnd.getRandomNumber(1));
}

void sceneHandler12_updateFloaters() {
	g_nmi->_floaters->genFlies(g_nmi->_currentScene, 397, -50, 100, 6);

	g_nmi->_floaters->_array2[0].countdown = g_nmi->_rnd.getRandomNumber(6) + 4;
	g_nmi->_floaters->_array2[0].val6 = 397;
	g_nmi->_floaters->_array2[0].val7 = -50;
}

int sceneHandler12(ExCommand *cmd) {
	int res = 0;

	if (cmd->_messageKind == 17 && cmd->_messageNum == 33) {
		if (g_nmi->_aniMan2) {
			if (g_nmi->_aniMan2->_ox < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.left - 300;

			if (g_nmi->_aniMan2->_ox > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.right + 300;

			res = 1;
		}

		g_vars->scene12_flyCountdown--;

		if (!g_vars->scene12_flyCountdown)
			sceneHandler12_updateFloaters();

		g_nmi->_floaters->update();

		g_nmi->_behaviorManager->updateBehaviors();
	}

	return res;
}

} // End of namespace NGI
