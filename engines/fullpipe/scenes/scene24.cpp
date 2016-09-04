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

void scene24_initScene(Scene *sc) {
	g_vars->scene24_water = sc->getStaticANIObject1ById(ANI_WATER24, -1);
	g_vars->scene24_jet = sc->getStaticANIObject1ById(ANI_JET24, -1);
	g_vars->scene24_drop = sc->getStaticANIObject1ById(ANI_DROP_24, -1);

	g_vars->scene24_water->setAlpha(0xa0);
	g_vars->scene24_jet->setAlpha(0xa0);
	g_vars->scene24_drop->setAlpha(0xa0);

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Pool) == g_fp->getObjectEnumState(sO_Pool, sO_Overfull)) {
		g_vars->scene24_jetIsOn = true;
		g_vars->scene24_flowIsLow = false;
	} else {
		g_vars->scene24_jet->hide();

		g_vars->scene24_jetIsOn = false;

		g_vars->scene24_water->changeStatics2(ST_WTR24_FLOWLOWER);

		g_vars->scene24_flowIsLow = true;
	}

	if (g_fp->getObjectState(sO_Pool) < g_fp->getObjectEnumState(sO_Pool, sO_Full)) {
		g_vars->scene24_waterIsOn = false;

		g_vars->scene24_water->hide();

		g_fp->setObjectState(sO_StairsDown_24, g_fp->getObjectEnumState(sO_StairsDown_24, sO_IsOpened));
	} else {
		g_vars->scene24_waterIsOn = true;

		g_fp->setObjectState(sO_StairsDown_24, g_fp->getObjectEnumState(sO_StairsDown_24, sO_IsClosed));
	}

	g_fp->_currentScene = oldsc;
}

void scene24_setPoolState() {
	if (g_fp->getObjectState(sO_Pool) == g_fp->getObjectEnumState(sO_Pool, sO_Overfull)) {
		g_fp->_behaviorManager->setFlagByStaticAniObject(g_vars->scene24_drop, 0);

		g_fp->playSound(SND_24_007, 1);
	} else if (g_fp->getObjectState(sO_Pool) == g_fp->getObjectEnumState(sO_Pool, sO_Full)) {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOFLOOR, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER2, 0);

		g_fp->playSound(SND_24_006, 1);
	} else if (g_fp->getObjectState(sO_Pool) == g_fp->getObjectEnumState(sO_Pool, sO_HalfFull)) {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOFLOOR, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER2, 1);
	} else {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOFLOOR, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene24_drop, ST_DRP24_EMPTY, QU_DRP24_TOWATER2, 0);
	}
}

int sceneHandler24(ExCommand *cmd) {
	if (cmd->_messageKind == 17 && cmd->_messageNum == 33) {
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200) {
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;
			}
			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene24_waterIsOn && !g_vars->scene24_water->_movement) {
			if (g_vars->scene24_flowIsLow)
				g_vars->scene24_water->startAnim(MV_WTR24_FLOWLOWER, 0, -1);
			else
				g_vars->scene24_water->startAnim(MV_WTR24_FLOW, 0, -1);
		}

		if (g_vars->scene24_jetIsOn && !g_vars->scene24_jet->_movement)
			g_vars->scene24_jet->startAnim(MV_JET24_FLOW, 0, -1);

		g_fp->_behaviorManager->updateBehaviors();
	}

	return 0;
}

} // End of namespace Fullpipe
