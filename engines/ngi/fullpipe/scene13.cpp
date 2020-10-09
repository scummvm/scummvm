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

void scene13_whirlgigCallback(int *phase) {
	// Do nothing
}

void scene13_initScene(Scene *sc) {
	g_vars->scene13_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLGIG_13, -1);
	g_vars->scene13_guard = sc->getStaticANIObject1ById(ANI_STOROZH, -1);
	g_vars->scene13_handleR = sc->getStaticANIObject1ById(ANI_HANDLE_R, -1);
	g_vars->scene13_handleL = sc->getStaticANIObject1ById(ANI_HANDLE_L, -1);
	g_vars->scene13_bridge = sc->getStaticANIObject1ById(ANI_BRIDGE, -1);
	g_vars->scene13_guardDirection = true;

	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_Bridge);
	Scene *oldsc = g_nmi->_currentScene;

	g_nmi->_currentScene = sc;

	if (g_nmi->getObjectState(sO_Bridge) == g_nmi->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);

		lnk->_flags |= 0x20000000u;

		g_nmi->playSound(SND_13_018, 1);

		g_vars->scene13_whirlgig->_callback2 = 0; // Really NULL
	} else {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);

		lnk->_flags &= 0xDFFFFFFF;

		g_vars->scene13_whirlgig->stopAnim_maybe();
		g_vars->scene13_whirlgig->_callback2 = scene13_whirlgigCallback;
		g_vars->scene13_whirlgig->startAnim(MV_WHR13_SPIN, 0, -1);

		if (g_vars->scene13_whirlgig->_movement)
			g_vars->scene13_whirlgig->_movement->setDynamicPhaseIndex(30);

		g_nmi->playSound(SND_13_037, 1);
	}

	g_vars->scene13_bridge->_flags &= 0xFFFD;

	g_nmi->_currentScene = oldsc;

	g_nmi->initArcadeKeys("SC_13");
}

void sceneHandler13_openBridge() {
	Movement *mov = g_vars->scene13_bridge->_movement;

	if (mov && mov->_id == MV_BDG_CLOSE) {
		int sz;

		if (mov->_currMovement)
			sz = mov->_currMovement->_dynamicPhases.size();
		else
			sz = mov->_dynamicPhases.size();

		g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);
		g_vars->scene13_bridge->startAnim(MV_BDG_OPEN, 0, -1);

		mov->setDynamicPhaseIndex(sz - mov->_currDynamicPhaseIndex);
	} else {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);
		g_vars->scene13_bridge->startAnim(MV_BDG_OPEN, 0, -1);
	}
}

void sceneHandler13_testClose() {
	int id = g_vars->scene13_handleL->_statics->_staticsId;

	if (id == ST_HDLL_UP)
		chainQueue(QU_SC13_CLOSEFAIL, 1);
	else if (id == ST_HDLL_FIRECAN || id == ST_HDLL_HAMMER)
		chainQueue(QU_SC13_CLOSESUCCESS, 1);
}

void sceneHandler13_testOpen() {
	switch (g_vars->scene13_handleR->_statics->_staticsId) {
	case ST_HDLR_DOWN:
		chainQueue(QU_SC13_OPENFAIL, 1);
		break;

	case ST_HDLR_DOWN_GUM:
		chainQueue(QU_SC13_OPENSUCCESS, 1);
		break;

	case ST_HDLR_GUM:
		g_vars->scene13_handleR->changeStatics2(ST_HDLR_DOWN_GUM);

		chainQueue(QU_SC13_OPENSUCCESS, 1);
		break;

	default:
		break;
	}
}

void sceneHandler13_closeBridge() {
	Movement *mov = g_vars->scene13_bridge->_movement;

	if (mov && mov->_id == MV_BDG_OPEN) {
		int sz;

		if (mov->_currMovement)
			sz = mov->_currMovement->_dynamicPhases.size();
		else
			sz = mov->_dynamicPhases.size();

		g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);
		g_vars->scene13_bridge->startAnim(MV_BDG_CLOSE, 0, -1);

		mov->setDynamicPhaseIndex(sz - mov->_currDynamicPhaseIndex);
	} else {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);
		g_vars->scene13_bridge->startAnim(MV_BDG_CLOSE, 0, -1);
	}
}

void sceneHandler13_closeFast() {
	g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);
	g_vars->scene13_bridge->startAnim(MV_BDG_CLOSE, 0, -1);
	g_vars->scene13_bridge->_movement->setDynamicPhaseIndex(21);
}

void sceneHandler13_stopWhirlgig() {
	g_vars->scene13_whirlgig->_callback2 = scene13_whirlgigCallback;

	g_nmi->stopAllSoundInstances(SND_13_018);
	g_nmi->playSound(SND_13_033, 0);
	g_nmi->playSound(SND_13_037, 1);
}

void sceneHandler13_startWhirlgig() {
	g_vars->scene13_whirlgig->_callback2 = 0; // Really NULL

	g_nmi->playSound(SND_13_018, 1);
	g_nmi->playSound(SND_13_034, 0);

	g_nmi->stopAllSoundInstances(SND_13_037);
}

void sceneHandler13_openFast() {
	g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);
	g_vars->scene13_bridge->startAnim(MV_BDG_OPEN, 0, -1);
	g_vars->scene13_bridge->_movement->setDynamicPhaseIndex(15);
}

void sceneHandler13_uneatGum() {
	BehaviorMove *beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT, QU_STR_CHEW);

	if (beh) {
		beh->_percent = 0;
		beh->_delay = 36;
	}

	beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT, QU_STR_PLUU);
	if (beh) {
		beh->_percent = 0;
		beh->_delay = 36;
	}
}

void sceneHandler13_eatGum() {
	BehaviorMove *beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT, QU_STR_CHEW);

	if (beh) {
		beh->_percent = 10922;
		beh->_delay = 0;
	}
}

void sceneHandler13_updateBridge() {
	MovGraphLink *lnk = getCurrSceneSc2MotionController()->getLinkByName(sO_Bridge);

	if (lnk) {
		if (g_nmi->getObjectState(sO_Bridge) == g_nmi->getObjectEnumState(sO_Bridge, sO_Convoluted))
			lnk->_flags |= 0x20000000;
		else
			lnk->_flags &= 0xDFFFFFFF;
	}
}

void sceneHandler13_showGum() {
	chainQueue(QU_SC13_SHOWGUM, 0);
}

void sceneHandler13_setBehFlag(BehaviorMove *beh, bool flag) {
	if (!flag) {
		beh->_percent = 327;
		beh->_flags |= 1;
		beh->_delay = 36;
	} else {
		beh->_percent = 0x7FFF;
		beh->_flags &= 0xFFFFFFFE;
		beh->_delay = 0;
	}
}

void sceneHandler13_walkForward(bool flag) {
	BehaviorMove *beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT, QU_STR_RTOL);

	sceneHandler13_setBehFlag(beh, flag);

	beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_LEFT, QU_STR_TURNR);

	sceneHandler13_setBehFlag(beh, flag);

	beh->_flags &= 0xFE;
}

void sceneHandler13_walkBackward(bool flag) {
	BehaviorMove *beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT|0x4000, QU_STR_LTOR);

	sceneHandler13_setBehFlag(beh, flag);

	beh = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene13_guard, ST_STR_LEFT|0x4000, QU_STR_TURNR_L);

	sceneHandler13_setBehFlag(beh, flag);

	beh->_flags &= 0xFE;
}

int sceneHandler13(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_SC13_OPENBRIDGE:
		sceneHandler13_openBridge();
		break;

	case MSG_SC13_TESTCLOSE:
		sceneHandler13_testClose();
		break;

	case MSG_SC13_TESTOPEN:
		sceneHandler13_testOpen();
		break;

	case MSG_SC13_CLOSEBRIDGE:
		sceneHandler13_closeBridge();
		break;

	case MSG_SC13_CLOSEFAST:
		sceneHandler13_closeFast();
		break;

	case MSG_SC13_STOPWHIRLGIG:
		sceneHandler13_stopWhirlgig();
		break;

	case MSG_SC13_STARTWHIRLGIG:
		sceneHandler13_startWhirlgig();
		break;

	case MSG_SC13_OPENFAST:
		sceneHandler13_openFast();
		break;

	case MSG_SC13_UNEATGUM:
		sceneHandler13_uneatGum();
		break;

	case MSG_SC13_EATGUM:
		sceneHandler13_eatGum();
		break;

	case MSG_SC13_CHEW:
		g_vars->scene13_guard->_flags &= 0xFF7Fu;
		break;

	case MSG_SC13_UPDATEBRIDGE:
		sceneHandler13_updateBridge();
		break;

	case MSG_SC13_SHOWGUM:
		sceneHandler13_showGum();
		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47
						 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0)) {
						g_nmi->processArcade(cmd);
					}
				}
			}
		}
		break;

	case 33:
		{
			int res = 0;
			int x;

			if (g_nmi->_aniMan2) {
				x = g_nmi->_aniMan2->_ox;
				g_vars->scene13_dudeX = x;

				if (x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = x - g_nmi->_sceneRect.left - 300;

				if (x > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = x - g_nmi->_sceneRect.right + 300;

				res = 1;

				g_nmi->sceneAutoScrolling();
			} else {
				x = g_vars->scene13_dudeX;
			}

			if (g_vars->scene13_guardDirection) {
				if (x < 1022) {
					sceneHandler13_walkForward(1);
					sceneHandler13_walkBackward(0);

					g_vars->scene13_guardDirection = false;

					g_nmi->_behaviorManager->updateBehaviors();
					g_nmi->startSceneTrack();

					return res;
				}
			} else if (x > 1022) {
				sceneHandler13_walkForward(0);
				sceneHandler13_walkBackward(1);

				g_vars->scene13_guardDirection = true;
			}

			g_nmi->_behaviorManager->updateBehaviors();
			g_nmi->startSceneTrack();

			return res;
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
