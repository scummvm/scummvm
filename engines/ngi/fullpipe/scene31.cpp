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


namespace NGI {

void scene31_initScene(Scene *sc) {
	g_vars->scene31_chantingCountdown = 0;
	g_vars->scene31_cactus = sc->getStaticANIObject1ById(ANI_CACTUS_31, -1);

	if (g_nmi->getObjectState(sO_Cactus) == g_nmi->getObjectEnumState(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_nmi->_currentScene;
		g_nmi->_currentScene = sc;

		g_vars->scene31_cactus->changeStatics2(ST_CTS31_GROWN2);
		g_vars->scene31_cactus->_priority = 22;

		g_nmi->_currentScene = oldsc;
	} else {
		g_vars->scene31_cactus->hide();
	}

	g_vars->scene31_plusMinus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_nmi->getObjectState(sO_Guard_3) == g_nmi->getObjectEnumState(sO_Guard_3, sO_Off))
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_PLUS);
}

void sceneHandler31_testCactus(ExCommand *cmd) {
	if ((g_vars->scene31_cactus->_flags & 4) && g_vars->scene31_cactus->_statics->_staticsId == ST_CTS31_GROWN2) {
		MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

		if (mq) {
			mq->getExCommandByIndex(0)->_messageKind = 0;
			mq->getExCommandByIndex(0)->_excFlags |= 1;

			mq->getExCommandByIndex(1)->_messageKind = 0;
			mq->getExCommandByIndex(1)->_excFlags |= 1;
		}
	}
}

int sceneHandler31(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
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
			g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_PLUS);
		else
			g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_MINUS);

		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

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

		g_nmi->_behaviorManager->updateBehaviors();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
