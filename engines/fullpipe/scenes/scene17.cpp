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
#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene17_initScene(Scene *sc) {
	g_vars->scene17_flyState = 1;
	g_vars->scene17_sugarIsShown = false;
	g_vars->scene17_sceneOldEdgeX = 0;
	g_vars->scene17_flyCountdown = 0;
	g_vars->scene17_hand = sc->getStaticANIObject1ById(ANI_HAND17, -1);
}

void scene17_restoreState() {
	if (g_fp->getObjectState(sO_UsherHand) == g_fp->getObjectEnumState(sO_UsherHand, sO_WithCoin)) {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 0);

		g_vars->scene17_handPhase = false;
	} else {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 1);

		g_vars->scene17_handPhase = true;
	}

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_17"));

	g_vars->scene17_flyState = g_fp->getObjectState(sO_Fly_17);

	if (g_vars->scene17_flyState <= 0 ) {
		g_vars->scene17_flyCountdown = g_fp->_rnd.getRandomNumber(600) + 600;

		g_vars->scene17_flyState = g_fp->_rnd.getRandomNumber(4) + 1;
	}

	g_fp->setObjectState(sO_Fly_17, g_vars->scene17_flyState - 1);
}

int scene17_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor != PIC_SC17_RTRUBA2 && g_fp->_objectIdAtCursor != PIC_SC17_RTRUBA)
		return g_fp->_cursorId;

	if (!g_vars->scene17_handPhase)
		return g_fp->_cursorId;

	int item = g_fp->_inventory->getSelectedItemId();

	if ((g_fp->_cursorId != PIC_CSR_DEFAULT_INV || item != ANI_INV_COIN) && item != ANI_INV_BOOT && item != ANI_INV_HAMMER)
		; // empty
	else
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

void sceneHandler17_drop() {
	StaticANIObject *mug = g_fp->_currentScene->getStaticANIObject1ById(ANI_MUG_17, -1);
	StaticANIObject *jet = g_fp->_currentScene->getStaticANIObject1ById(ANI_JET_17, -1);

	if (mug && mug->_flags & 4) {
		mug->changeStatics2(ST_MUG17_EMPTY);
		chainQueue(QU_SC17_FILLMUG_DROP, 0);
	} else if (jet) {
		jet->queueMessageQueue(0);
		chainQueue(QU_JET17_DROP, 0);
	}
}

void sceneHandler17_fillBottle() {
	StaticANIObject *bottle = g_fp->_currentScene->getStaticANIObject1ById(ANI_INV_BOTTLE, -1);
	StaticANIObject *mug = g_fp->_currentScene->getStaticANIObject1ById(ANI_MUG_17, -1);
	StaticANIObject *boot = g_fp->_currentScene->getStaticANIObject1ById(ANI_BOOT_17, -1);

	if (bottle && (bottle->_flags & 4))
		chainQueue(QU_SC17_FILLBOTTLE, 1);
	else if (mug && (mug->_flags & 4) && mug->_statics->_staticsId == ST_MUG17_EMPTY)
		chainQueue(QU_SC17_FILLMUG, 1);
	else if (boot && (boot->_flags & 4))
		chainQueue(QU_SC17_FILLBOOT, 1);
	else
		chainQueue(QU_JET17_FLOW, 1);
}

void sceneHandler17_testTruba() {
	if (g_vars->scene17_hand->isIdle()) {
		if (!g_vars->scene17_hand->_movement || g_vars->scene17_hand->_movement->_id != MV_HND17_FIGA) {
			g_vars->scene17_hand->changeStatics2(ST_HND17_EMPTY);
			g_vars->scene17_hand->startAnim(MV_HND17_FIGA, 0, -1);
		}
	}
}

void sceneHandler17_showBottle() {
	chainQueue(QU_SC17_SHOWBOTTLE, 0);
}

void sceneHandler17_hideSugar() {
	StaticANIObject *sugar = g_fp->_currentScene->getStaticANIObject1ById(ANI_INV_SUGAR, -1);

	if (sugar)
		sugar->hide();
}

void sceneHandler17_showSugar() {
	chainQueue(QU_SC17_SHOWSUGAR, 0);

	g_vars->scene17_sugarIsShown = true;
}

void sceneHandler17_moonshineFill() {
	StaticANIObject *moonshiner = g_fp->_currentScene->getStaticANIObject1ById(ANI_SAMOGONSHCHIK, -1);

	if (!(moonshiner->_flags & 0x80)) {
		moonshiner->changeStatics2(ST_SMG_SIT);
		chainObjQueue(moonshiner, QU_SMG_FILLBOTTLE, 1);

		g_vars->scene17_sugarIsShown = false;
	}
}

void sceneHandler17_updateFlies() {
	g_fp->_floaters->genFlies(g_fp->_currentScene, 239, -50, 20, 4);

	g_fp->_floaters->_array2[0].countdown = g_fp->_rnd.getRandomNumber(5) + 6;
	g_fp->_floaters->_array2[0].val6 = 239;
	g_fp->_floaters->_array2[0].val7 = -50;
}


int sceneHandler17(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC17_DROP:
		sceneHandler17_drop();
		break;

	case MSG_SC17_UPDATEHAND:
		if (g_fp->getObjectState(sO_UsherHand) == g_fp->getObjectEnumState(sO_UsherHand, sO_WithCoin)) {
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 0);

			g_vars->scene17_handPhase = false;
		} else {
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 1);

			g_vars->scene17_handPhase = true;
		}
		break;

	case MSG_SC17_FILLBOTTLE:
		sceneHandler17_fillBottle();
		break;

	case MSG_SC17_TESTTRUBA:
		sceneHandler17_testTruba();
		break;

	case MSG_SC17_SHOWBOTTLE:
		sceneHandler17_showBottle();
		break;

	case MSG_SC17_HIDESUGAR:
		sceneHandler17_hideSugar();
		break;

	case MSG_SC17_SHOWSUGAR:
		sceneHandler17_showSugar();
		break;

	case 29:
		{
			int pic = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (pic == PIC_SC17_RTRUBA2 || pic == PIC_SC17_RTRUBA) {
				if (cmd->_param == ANI_INV_COIN || cmd->_param == ANI_INV_BOOT || cmd->_param == ANI_INV_HAMMER) {
					if (g_vars->scene17_handPhase) {
						if (g_fp->_aniMan->isIdle()) {
							if (!(g_fp->_aniMan->_flags & 0x100)) {
								handleObjectInteraction(g_fp->_aniMan, g_vars->scene17_hand, cmd->_param);
								break;
							}
						}
					}
				}
			}
		}
		break;

	case 33:
		{
			int x = g_vars->scene17_sceneEdgeX;
			g_vars->scene17_sceneOldEdgeX = g_vars->scene17_sceneEdgeX;

			if (g_fp->_aniMan2) {
				x = g_fp->_aniMan2->_ox;

				g_vars->scene17_sceneEdgeX = x;

				if (x < g_fp->_sceneRect.left + 200) {
					g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

					x = g_vars->scene17_sceneEdgeX;
				}

				if (x > g_fp->_sceneRect.right - 200) {
					g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
					x = g_vars->scene17_sceneEdgeX;
				}
			}

			if (g_vars->scene17_sugarIsShown) {
				sceneHandler17_moonshineFill();
				x = g_vars->scene17_sceneEdgeX;
			}

			if (g_vars->scene17_handPhase) {
				if (g_vars->scene17_sceneOldEdgeX < 410 && x >= 410) {
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 0);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_ATTRACT, QU_HND17_ATTRACT, 0);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 1);
				} else if (g_vars->scene17_sceneOldEdgeX > 410 && x <= 410) {
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 1);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_ATTRACT, QU_HND17_ATTRACT, 1);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
				}
			}

			--g_vars->scene17_flyCountdown;

			if (!g_vars->scene17_flyCountdown)
				sceneHandler17_updateFlies();

			g_fp->_floaters->update();

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();

			break;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
