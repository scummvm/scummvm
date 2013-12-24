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

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/floaters.h"
#include "fullpipe/messages.h"
#include "fullpipe/statics.h"
#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene12_initScene() {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("SC_12");
	g_fp->_floaters->init(var);

	g_vars->scene12_fly = g_fp->getObjectState("Муха_12");

	if (g_vars->scene12_fly)
		g_vars->scene12_flyCountdown = 600 * g_fp->_rnd->getRandomNumber(32767) / 0x7fff + 600;

	g_fp->setObjectState("Муха_12", g_fp->_rnd->getRandomNumber(1));
}

void sceneHandler12_updateFloaters() {
	warning("STUB: sceneHandler12_updateFloaters()");
}

int sceneHandler12(ExCommand *cmd) {
	int res = 0;

	if (cmd->_messageKind == 17 && cmd->_messageNum == 33) {
		if (g_fp->_aniMan2) {
			if (g_fp->_aniMan2->_ox < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.left - 300;

			if (g_fp->_aniMan2->_ox > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.right + 300;

			res = 1;
		}

		g_vars->scene12_flyCountdown--;

		if (!g_vars->scene12_flyCountdown)
			sceneHandler12_updateFloaters();

		g_fp->_floaters->update();

		g_fp->_behaviorManager->updateBehaviors();
	}

	return res;
}

} // End of namespace Fullpipe
