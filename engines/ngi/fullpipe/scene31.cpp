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


namespace NGI {

void scene31_initScene(Scene *sc) {
	g_vars->scene31_chantingCountdown = 0;
	g_vars->scene31_cactus = sc->findAniObjectById(ANI_CACTUS_31, -1);

	if (g_nmi->getStateId(sO_Cactus) == g_nmi->getStateValue(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_nmi->_currentScene;
		g_nmi->_currentScene = sc;

		g_vars->scene31_cactus->setStatic(ST_CTS31_GROWN2);
		g_vars->scene31_cactus->_z = 22;

		g_nmi->_currentScene = oldsc;
	} else {
		g_vars->scene31_cactus->hide();
	}

	g_vars->scene31_plusMinus = sc->findAniObjectById(ANI_PLUSMINUS, -1);

	if (g_nmi->getStateId(sO_Guard_3) == g_nmi->getStateValue(sO_Guard_3, sO_Off))
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStatic(ST_PMS_MINUS);
	else
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStatic(ST_PMS_PLUS);
}

void sceneHandler31_testCactus(Command *cmd) {
	if ((g_vars->scene31_cactus->_flags & 4) && g_vars->scene31_cactus->_statics->_staticsId == ST_CTS31_GROWN2) {
		MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

		if (mq) {
			mq->getCommandByIndex(0)->_cmd = 0;
			mq->getCommandByIndex(0)->_excFlags |= 1;

			mq->getCommandByIndex(1)->_cmd = 0;
			mq->getCommandByIndex(1)->_excFlags |= 1;
		}
	}
}

int sceneHandler31(Command *cmd) {
	if (cmd->_cmd != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC31_TESTCACTUS:
		sceneHandler31_testCactus(cmd);
		break;

	case MSG_SC15_STOPCHANTING:
		g_nmi->stopAllSoundInstances(SND_31_001);

		g_vars->scene31_chantingCountdown = 120;
		break;

	case MSG_SC31_PULL:
		if ( g_vars->scene31_plusMinus->_statics->_staticsId == ST_PMS_MINUS)
			g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStatic(ST_PMS_PLUS);
		else
			g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStatic(ST_PMS_MINUS);

		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_x;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		if (g_vars->scene31_chantingCountdown > 0) {
			--g_vars->scene31_chantingCountdown;

			if (!g_vars->scene31_chantingCountdown)
				g_nmi->playSound(SND_31_001, 1);
		}

		g_nmi->_behaviorManager->update();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
