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

void scene13_initScene(Scene *sc) {
	g_vars->scene13_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLGIG_13, -1);
	g_vars->scene13_guard = sc->getStaticANIObject1ById(ANI_STOROZH, -1);
	g_vars->scene13_handleR = sc->getStaticANIObject1ById(ANI_HANDLE_R, -1);
	g_vars->scene13_handleL = sc->getStaticANIObject1ById(ANI_HANDLE_L, -1);
	g_vars->scene13_bridge = sc->getStaticANIObject1ById(ANI_BRIDGE, -1);
	g_vars->scene13_var01 = 0;
	g_vars->scene13_var02 = 1;

	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_Bridge);
	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);

		lnk->_flags |= 0x20000000u;

		g_fp->playSound(SND_13_018, 1);

		g_vars->scene13_whirlgig->_callback2 = 0;
	} else {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);

		lnk->_flags &= 0xDFFFFFFF;

		g_vars->scene13_whirlgig->stopAnim_maybe();
		g_vars->scene13_whirlgig->_callback2 = 0;
		g_vars->scene13_whirlgig->startAnim(MV_WHR13_SPIN, 0, -1);

		if (g_vars->scene13_whirlgig->_movement)
			g_vars->scene13_whirlgig->_movement->setDynamicPhaseIndex(30);

		g_fp->playSound(SND_13_037, 1);
	}

	g_vars->scene13_bridge->_flags &= 0xFFFD;

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_13");
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
	warning("STUB: sceneHandler13_testClose()");
}

void sceneHandler13_testOpen() {
	warning("STUB: sceneHandler13_testOpen()");
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
	g_vars->scene13_whirlgig->_callback2 = 0;

	g_fp->stopAllSoundInstances(SND_13_018);
	g_fp->playSound(SND_13_033, 0);
	g_fp->playSound(SND_13_037, 1);
}

void sceneHandler13_startWhirlgig() {
	g_vars->scene13_whirlgig->_callback2 = 0;

	g_fp->playSound(SND_13_018, 1);
	g_fp->playSound(SND_13_034, 0);

	g_fp->stopAllSoundInstances(SND_13_037);
}

void sceneHandler13_openFast() {
	g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);
	g_vars->scene13_bridge->startAnim(MV_BDG_OPEN, 0, -1);
	g_vars->scene13_bridge->_movement->setDynamicPhaseIndex(15);
}

void sceneHandler13_uneatGum() {
	warning("STUB: sceneHandler13_uneatGum()");
}

void sceneHandler13_eatGum() {
	BehaviorEntryInfo *beh = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene13_guard, ST_STR_RIGHT, QU_STR_CHEW);

	if (beh) {
		beh->_percent = 10922;
		beh->_delay = 0;
	}
}

void sceneHandler13_updateBridge() {
	warning("STUB: sceneHandler13_updateBridge()");
}

void sceneHandler13_showGum() {
	chainQueue(QU_SC13_SHOWGUM, 0);
}

void sceneHandler13_sub01(bool flag) {
	warning("STUB: sceneHandler13_sub01()");
}

void sceneHandler13_sub02(bool flag) {
	warning("STUB: sceneHandler13_sub02()");
}

void sceneHandler13_sub03(bool flag) {
	warning("STUB: sceneHandler13_sub03()");
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
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47
						 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
					}
				}
			}
			break;
		}

	case 33:
		{
			int res = 0;
			int x;

			if (g_fp->_aniMan2) {
				x = g_fp->_aniMan2->_ox;
				g_vars->scene13_var03 = x;

				if (x < g_fp->_sceneRect.left + 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.left - 300;

				if (x > g_fp->_sceneRect.right - 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;

				res = 1;
			} else {
				x = g_vars->scene13_var03;
			}

			if (g_vars->scene13_var02) {
				if (x < 1022) {
					sceneHandler13_sub01(1);
					sceneHandler13_sub02(0);

					g_vars->scene13_var02 = 0;

					g_fp->_behaviorManager->updateBehaviors();
					g_fp->startSceneTrack();

					return res;
				}
			} else if (x > 1022) {
				sceneHandler13_sub01(0);
				sceneHandler13_sub02(1);

				g_vars->scene13_var02 = 1;
			}

			g_fp->_behaviorManager->updateBehaviors();
			g_fp->startSceneTrack();

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
