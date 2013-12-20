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
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/floaters.h"
#include "fullpipe/motion.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene08_initScene(Scene *sc) {
	g_vars->scene08_var06 = 0;
	g_vars->scene08_var01 = 0;
	g_vars->scene08_var02 = 0;
	g_vars->scene08_var03 = 0;
	g_vars->scene08_batuta = sc->getStaticANIObject1ById(ANI_BATUTA, -1);
	g_vars->scene08_vmyats = sc->getStaticANIObject1ById(ANI_VMYATS, -1);
	g_vars->scene08_clock = sc->getStaticANIObject1ById(ANI_CLOCK_8, -1);
	g_vars->scene08_var04 = -37;
	g_vars->scene08_var05 = -1;

	Scene *oldsc = g_fullpipe->_currentScene;
	g_fullpipe->_currentScene = sc;

	int batuta = g_fullpipe->getObjectState(sO_TummyTrampie);
	MovGraphLink *lock = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_CloseThing);

	if (lock)
		lock->_flags &= 0xDFFFFFFF;

	if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsEating)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SPOON);
	} else if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsDrinking)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_NOSPOON);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(3);
	} else if (batuta== g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsScratchingBelly)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_CHESHET);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(8);
	} else if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SLEEPS);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(2);

		if (lock)
			lock->_flags |= 0x20000000;

		g_vars->scene08_var05 = 71;
	}

	g_vars->scene08_clock->_callback2 = 0;

	if (g_fullpipe->getObjectState(sO_StairsUp_8) == g_fullpipe->getObjectEnumState(sO_StairsUp_8, sO_Broken)) {
		g_vars->scene08_var07 = 0;

		sc->getPictureObjectById(PIC_SC8_LADDER, 0)->_flags &= 0xFFFB;

		g_vars->scene08_var04 = -39;
	} else {
		g_vars->scene08_var07 = 1;
	}

	sc->getPictureObjectById(PIC_SC8_ARCADENOW, 0)->_flags &= 0xFFFB;

	g_fullpipe->_currentScene = oldsc;

	g_fullpipe->_floaters->init(g_fullpipe->getGameLoaderGameVar()->getSubVarByName("SC_8"));
	g_fullpipe->_floaters->genFlies(sc, 100, 100, 0, 0);

	g_fullpipe->setArcadeOverlay(PIC_CSR_ARCADE3);
}

void sceneHandler08_winArcade() {
	warning("STUB: sceneHandler08_winArcade()");
}

void sceneHandler08_enterUp() {
	warning("STUB: sceneHandler08_enterUp()");
}

void sceneHandler08_hideLadder() {
	warning("STUB: sceneHandler08_hideLadder()");
}

void sceneHandler08_arcadeNow() {
	warning("STUB: sceneHandler08_arcadeNow()");
}

void sceneHandler08_resumeFlight() {
	warning("STUB: sceneHandler08_resumeFlight()");
}

void sceneHandler08_startArcade() {
	warning("STUB: sceneHandler08_startArcade()");
}

void sceneHandler08_airMoves() {
	warning("STUB: sceneHandler08_airMoves()");
}

void sceneHandler08_jumpLogic(ExCommand *cmd) {
	warning("STUB: sceneHandler08_jumpLogic(cmd)");
}

void sceneHandler08_calcFlight() {
	warning("STUB: sceneHandler08_calcFlight()");
}

void sceneHandler08_checkEndArcade() {
	warning("STUB: sceneHandler08_checkEndArcade()");
}

void sceneHandler08_badLuck() {
	warning("STUB: sceneHandler08_badLuck()");
}

void sceneHandler08_calcOffset() {
	warning("STUB: sceneHandler08_calcOffset()");
}

void sceneHandler08_finishArcade() {
	warning("STUB: sceneHandler08_finishArcade()");
}

void sceneHandler08_jumpOff() {
	warning("STUB: sceneHandler08_jumpOff()");
}

void sceneHandler08_pushCallback(int *par) {
	warning("STUB: sceneHandler08_pushCallback()");
}

void sceneHandler08_sitDown() {
	warning("STUB: sceneHandler08_sitDown()");
}

void sceneHandler08_standUp() {
	warning("STUB: sceneHandler08_standUp()");
}

void sceneHandler08_updateScreenCallback() {
	warning("STUB: sceneHandler08_updateScreenCallback()");
}

int sceneHandler08(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler08_winArcade();
		break;

	case MSG_SC8_ENTERUP:
		sceneHandler08_enterUp();
		break;

	case MSG_SC8_HIDELADDER_D:
		sceneHandler08_hideLadder();
		break;

	case MSG_SC8_STANDUP:
		g_vars->scene08_var08 = -10;
		g_vars->scene08_vmyats->changeStatics2(ST_VMT_MIN);
		g_vars->scene08_vmyats->setOXY(382, 703);
		g_vars->scene08_vmyats->_priority = 29;
		g_vars->scene08_vmyats->_callback2 = sceneHandler08_pushCallback;
		g_vars->scene08_var01 = 1;
		break;

	case MSG_SC8_ARCADENOW:
		sceneHandler08_arcadeNow();
		break;

	case MSG_SC8_RESUMEFLIGHT:
		sceneHandler08_resumeFlight();
		break;

	case MSG_SC8_GETHIMUP:
		g_vars->scene08_var08 = 0;
		g_vars->scene08_var02 = 1;
		break;

	case MSG_STARTARCADE:
		sceneHandler08_startArcade();
		break;

	case 29:
		if (g_vars->scene08_var06) {
			if (g_vars->scene08_var01) {
				sceneHandler08_airMoves();
				break;
			}
			if (g_vars->scene08_var03) {
				sceneHandler08_jumpLogic(cmd);
				break;
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_fullpipe->_aniMan2) {
				if (g_vars->scene08_var06) {
					int scHeight = g_fullpipe->_sceneRect.bottom - g_fullpipe->_sceneRect.top;

					if (g_fullpipe->_aniMan2->_oy < g_fullpipe->_sceneRect.top + 200) {
						g_fullpipe->_sceneRect.top = g_fullpipe->_aniMan2->_oy - 200;

						if (g_fullpipe->_sceneRect.top < 0)
							g_fullpipe->_sceneRect.top = 0;

						g_fullpipe->_sceneRect.bottom = scHeight + g_fullpipe->_sceneRect.top;
					}

					if (g_fullpipe->_aniMan2->_oy > g_fullpipe->_sceneRect.bottom - 350) {
						g_fullpipe->_sceneRect.bottom = g_fullpipe->_aniMan2->_oy + 350;
						g_fullpipe->_sceneRect.top = g_fullpipe->_aniMan2->_oy + 350 - scHeight;
					}
				} else {
					if (g_fullpipe->_aniMan2->_ox < g_fullpipe->_sceneRect.left + 200)
						g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.left - 300;

					if (g_fullpipe->_aniMan2->_ox > g_fullpipe->_sceneRect.right - 200)
						g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.right + 300;

					res = 1;
				}
			}

			g_fullpipe->_floaters->update();

			if (g_vars->scene08_var06) {
				if (g_vars->scene08_var01)
					sceneHandler08_calcFlight();
			} else {
				Movement *mov = g_fullpipe->_aniMan->_movement;

				if (mov) {
					if (mov->_id == MV_MAN_TOLADDERD && mov->_currDynamicPhaseIndex == 8)
						g_fullpipe->_aniMan->_priority = 2;

					if (mov && mov->_id == MV_MAN_FROMLADDERUP && mov->_currDynamicPhaseIndex == 13)
						g_fullpipe->_aniMan->_priority = 20;
				}

				g_fullpipe->_behaviorManager->updateBehaviors();
				g_fullpipe->startSceneTrack();
			}

			if (g_vars->scene08_var02)
				sceneHandler08_checkEndArcade();

			if (g_vars->scene08_var05 > 0) {
				g_vars->scene08_var05--;

				if (!g_vars->scene08_var05) {
					g_fullpipe->playSound(SND_8_014, 0);

					g_vars->scene08_var05 = 71;
				}
			}

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
