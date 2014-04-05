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

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/floaters.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/behavior.h"
#include "fullpipe/interaction.h"

namespace Fullpipe {

void scene08_initScene(Scene *sc) {
	g_vars->scene08_inArcade = false;
	g_vars->scene08_inAir = false;
	g_vars->scene08_flyingUp = false;
	g_vars->scene08_onBelly = false;
	g_vars->scene08_batuta = sc->getStaticANIObject1ById(ANI_BATUTA, -1);
	g_vars->scene08_vmyats = sc->getStaticANIObject1ById(ANI_VMYATS, -1);
	g_vars->scene08_clock = sc->getStaticANIObject1ById(ANI_CLOCK_8, -1);
	g_vars->scene08_stairsOffset = -37;
	g_vars->scene08_snoringCountdown = -1;

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	int batuta = g_fp->getObjectState(sO_TummyTrampie);
	MovGraphLink *lock = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_CloseThing);

	if (lock)
		lock->_flags &= 0xDFFFFFFF;

	if (batuta == g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsEating)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SPOON);
	} else if (batuta == g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsDrinking)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_NOSPOON);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(3);
	} else if (batuta== g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsScratchingBelly)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_CHESHET);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(8);
	} else if (batuta == g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SLEEPS);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(2);

		if (lock)
			lock->_flags |= 0x20000000;

		g_vars->scene08_snoringCountdown = 71;
	}

	g_vars->scene08_clock->_callback2 = 0;

	if (g_fp->getObjectState(sO_StairsUp_8) == g_fp->getObjectEnumState(sO_StairsUp_8, sO_Broken)) {
		g_vars->scene08_stairsVisible = false;

		sc->getPictureObjectById(PIC_SC8_LADDER, 0)->_flags &= 0xFFFB;

		g_vars->scene08_stairsOffset = -39;
	} else {
		g_vars->scene08_stairsVisible = true;
	}

	sc->getPictureObjectById(PIC_SC8_ARCADENOW, 0)->_flags &= 0xFFFB;

	g_fp->_currentScene = oldsc;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_8"));
	g_fp->_floaters->genFlies(sc, 100, 100, 0, 0);

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE3);
}

void scene08_setupMusic() {
	if (g_fp->getObjectState(sO_TummyTrampie) == g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping))
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_8"), "MUSIC_ARCADE", 1);
}

int scene08_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene08_inArcade) {
		if (g_vars->scene08_onBelly) {
			if (g_fp->_objectIdAtCursor == PIC_SC8_LADDERD && g_fp->_cursorId == PIC_CSR_ITN)
				g_fp->_cursorId = PIC_CSR_GOU;
		} else {
			g_fp->_cursorId = -1;
		}
	} else {
		if (g_fp->_objectIdAtCursor == PIC_SC8_LADDERD && g_fp->_cursorId == PIC_CSR_ITN) {
			if (g_fp->_aniMan2->_oy >= 520) {
				if (g_fp->_aniMan2->_oy <= 750)
					g_fp->_cursorId = PIC_CSR_GOU;
			} else {
				g_fp->_cursorId = PIC_CSR_GOD;
			}
		}
	}

	return g_fp->_cursorId;
}

void sceneHandler08_enterUp() {
	g_fp->_currentScene->getPictureObjectById(PIC_SC8_LADDER, 0)->_flags &= 0xFFFB;

	g_fp->_aniMan->changeStatics2(ST_MAN8_HANDSUP);
	g_fp->_aniMan->setOXY(386, 236);
	g_fp->_aniMan->_priority = 10;
	g_fp->_aniMan->_flags = 4;

	chainObjQueue(g_fp->_aniMan, QU_SC8_FINISH, 1);

	g_vars->scene08_inAir = false;
}

void sceneHandler08_winArcade() {
	if (g_vars->scene08_inArcade) {
		g_vars->scene08_inArcade = false;
		g_fp->_sceneRect.top = 0;
		g_fp->_sceneRect.bottom = 600;

		sceneHandler08_enterUp();
	}
}

void sceneHandler08_hideLadder() {
	g_fp->_currentScene->getPictureObjectById(PIC_SC8_LADDER_D, 0)->_flags &= 0xFFFB;
}

void sceneHandler08_arcadeNow() {
	MovGraphLink *lnk = getCurrSceneSc2MotionController()->getLinkByName(sO_Stairway);

	g_fp->setObjectState(sO_TummyTrampie, g_fp->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping));

	g_vars->scene08_batuta->changeStatics2(ST_BTT_SLEEPS);

	if (lnk)
		lnk->_flags |= 0x20000000;
}

void sceneHandler08_resumeFlight() {
	g_vars->scene08_manOffsetY = 3;
	g_vars->scene08_stairsOffset = -39;
	g_vars->scene08_inAir = true;
	g_vars->scene08_stairsVisible = false;
}

int sceneHandler08_calcOffset(int off, int flag) {
	if (off > 0) {
		if (flag)
			return off * 31 / 10; // off * 3.1
		else
			return 5 * off;
	} else {
		return 5 * off;
	}
}

void sceneHandler08_pushCallback(int *par) {
	Common::Point point;

	int y = g_fp->_aniMan->_oy + g_fp->_aniMan->getSomeXY(point)->y;

	if (g_fp->_aniMan->_statics && g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYDOWN)
		y -= 25;

	*par = (y - 703) / 10;
	if (*par > 11) {
		*par = 11;
		g_vars->scene08_manOffsetY = 0;
	}
	if (*par >= 0)
		g_vars->scene08_manOffsetY -= sceneHandler08_calcOffset(*par, g_vars->scene08_manOffsetY < 0);
	else
		*par = 0;
}

int sceneHandler08_updateScreenCallback() {
	int res;

	res = g_fp->drawArcadeOverlay(g_vars->scene08_inArcade);

	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

void sceneHandler08_startArcade() {
	g_vars->scene08_inArcade = true;
	g_vars->scene08_inAir = true;
	g_vars->scene08_flyingUp = false;
	g_vars->scene08_onBelly = false;

	getGameLoaderInteractionController()->disableFlag24();
	getCurrSceneSc2MotionController()->clearEnabled();

	g_vars->scene08_batuta->stopAnim_maybe();

	g_vars->scene08_vmyats->_statics = g_vars->scene08_vmyats->getStaticsById(ST_VMT_MIN);
	g_vars->scene08_vmyats->setOXY(382, 703);
	g_vars->scene08_vmyats->_priority = 29;
	g_vars->scene08_vmyats->_callback2 = sceneHandler08_pushCallback;

	g_fp->_aniMan = g_fp->_currentScene->getStaticANIObject1ById(ANI_MAN, -1);

	g_vars->scene08_manOffsetY = 15;

	g_fp->_currentScene->_y = 0;

	g_fp->_updateScreenCallback = sceneHandler08_updateScreenCallback;
}

void sceneHandler08_airMoves() {
	if (g_fp->_aniMan->isIdle() && !(g_fp->_aniMan->_flags & 0x100)) {
		int x = g_fp->_aniMan->_ox;
		int y = g_fp->_aniMan->_oy;
		Common::Point point;

		if (703 - g_fp->_aniMan->getSomeXY(point)->y - y < 150) {
			if (g_fp->_aniMan->_statics) {
				if (g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYDOWN) {
					y -= 25;

					g_fp->_aniMan->setOXY(x, y);
				}
			}

			g_fp->_aniMan->changeStatics2(ST_MAN8_STAND);
			g_fp->_aniMan->setOXY(380, y);
			g_fp->_aniMan->startAnim(MV_MAN8_JUMP, 0, -1);

		} else if (g_fp->_aniMan->_statics) {
			if (g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYUP) {
				g_fp->_aniMan->startAnim(MV_MAN8_DRYGUP, 0, -1);

			} else if (g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYDOWN) {
				g_fp->_aniMan->startAnim(MV_MAN8_DRYGDOWN, 0, -1);
			}
		}
	}
}

void sceneHandler08_finishArcade() {
	g_vars->scene08_inArcade = false;

	getGameLoaderInteractionController()->enableFlag24();
	getCurrSceneSc2MotionController()->setEnabled();
}

void sceneHandler08_jumpOff(ExCommand *cmd) {
	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

	mq->addExCommandToEnd(cmd->createClone());
	mq->setFlags(mq->getFlags() | 1);

	g_fp->_globalMessageQueueList->addMessageQueue(mq);

	g_fp->_aniMan->startAnim(MV_MAN8_JUMPOFF, mq->_id, -1);

	sceneHandler08_finishArcade();
}

void sceneHandler08_standUp() {
	chainQueue(QU_SC8_STANDUP, 1);
	g_vars->scene08_onBelly = false;
}

void sceneHandler08_jumpLogic(ExCommand *cmd) {
  if (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC8_LADDERD) {
    sceneHandler08_jumpOff(cmd);

    cmd->_messageKind = 0;
  } else {
    sceneHandler08_standUp();
  }
}

void sceneHandler08_badLuck() {
	g_fp->_currentScene->getPictureObjectById(PIC_SC8_LADDER, 0)->_flags &= 0xFFFB;

	g_fp->_aniMan->changeStatics2(ST_MAN8_HANDSUP);
	g_fp->_aniMan->setOXY(376, 280);
	g_fp->_aniMan->_priority = 10;

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

	ExCommand *ex = new ExCommand(g_fp->_aniMan->_id, 1, MV_MAN8_BADLUCK, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 2;
	ex->_keyCode = g_fp->_aniMan->_okeyCode;
	mq->addExCommandToEnd(ex);

	mq->setFlags(mq->getFlags() | 1);
	mq->chain(0);

	g_fp->setObjectState(sO_StairsUp_8, g_fp->getObjectEnumState(sO_StairsUp_8, sO_NotBroken));

	g_vars->scene08_inAir = false;
}

void sceneHandler08_sitDown() {
	g_fp->_aniMan->setOXY(380, g_fp->_aniMan->_oy);

	g_fp->_aniMan->changeStatics2(ST_MAN8_FLYDOWN);
	g_fp->_aniMan->startAnim(MV_MAN8_SITDOWN, 0, -1);

	g_vars->scene08_vmyats->changeStatics2(ST_VMT_MIN);
	g_vars->scene08_vmyats->hide();

	g_vars->scene08_inAir = false;
	g_vars->scene08_onBelly = true;
}

void sceneHandler08_calcFlight() {
	Common::Point point;
	int y = g_vars->scene08_manOffsetY + g_fp->_aniMan->_oy;

	g_fp->_aniMan->setOXY(g_fp->_aniMan->_ox, y);

	g_vars->scene08_manOffsetY += 2;

	if (g_vars->scene08_manOffsetY < g_vars->scene08_stairsOffset)
		g_vars->scene08_manOffsetY = g_vars->scene08_stairsOffset;

	y = y + g_fp->_aniMan->getSomeXY(point)->y;

	if (g_fp->_aniMan->_statics && g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYDOWN)
		y -= 25;

	if (y <= g_vars->scene08_vmyats->_oy) {
		g_vars->scene08_vmyats->hide();
	} else {
		g_vars->scene08_vmyats->show1(-1, -1, -1, 0);

		if (!g_vars->scene08_vmyats->_movement)
			g_vars->scene08_vmyats->startAnim(MV_VMT_DEF, 0, -1);
	}

	if (g_fp->_aniMan->_oy <= 280 && g_vars->scene08_stairsVisible
		&& g_fp->_aniMan->_statics && g_fp->_aniMan->_statics->_staticsId == ST_MAN8_HANDSUP) {
		sceneHandler08_badLuck();
	} else if (g_fp->_aniMan->_oy > 236 || g_vars->scene08_stairsVisible
			   || !g_fp->_aniMan->_statics || g_fp->_aniMan->_statics->_staticsId != ST_MAN8_HANDSUP) {
		if (g_fp->_aniMan->_movement || g_fp->_aniMan->_oy < 660
			 || (g_vars->scene08_vmyats->_movement && g_vars->scene08_vmyats->_movement->_currDynamicPhaseIndex > 0)
			|| abs(g_vars->scene08_manOffsetY) > 2) {
			if (g_vars->scene08_manOffsetY >= 0 && !g_fp->_aniMan->_movement) {
				if (g_fp->_aniMan->_statics->_staticsId == ST_MAN8_HANDSUP)
					g_fp->_aniMan->startAnim(MV_MAN8_HANDSDOWN, 0, -1);
				else
					g_fp->_aniMan->changeStatics2(ST_MAN8_FLYDOWN);
			}

			if (g_fp->_aniMan->_oy < 500 && !g_fp->_aniMan->_movement && g_fp->_aniMan->_statics->_staticsId == ST_MAN8_FLYUP && g_vars->scene08_manOffsetY < 0)
				g_fp->_aniMan->startAnim(MV_MAN8_HANDSUP, 0, -1);
		} else {
			sceneHandler08_sitDown();
		}
	} else {
		sceneHandler08_enterUp();
	}
}

void sceneHandler08_checkEndArcade() {
	if (g_vars->scene08_flyingUp) {
		int x = g_fp->_aniMan->_ox;
		int y = g_vars->scene08_manOffsetY + g_fp->_aniMan->_oy;

		if (!((g_vars->scene08_manOffsetY + g_fp->_aniMan->_oy) % 3))
			g_vars->scene08_manOffsetY--;

		g_fp->_aniMan->setOXY(x, y);

		if (y < 80) {
			sceneHandler08_finishArcade();

			ExCommand *ex = new ExCommand(SC_8, 17, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_messageNum = 61;
			ex->_excFlags |= 2;
			ex->_keyCode = TrubaUp;

			ex->postMessage();
		}
	}
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
		g_vars->scene08_manOffsetY = -10;
		g_vars->scene08_vmyats->changeStatics2(ST_VMT_MIN);
		g_vars->scene08_vmyats->setOXY(382, 703);
		g_vars->scene08_vmyats->_priority = 29;
		g_vars->scene08_vmyats->_callback2 = sceneHandler08_pushCallback;
		g_vars->scene08_inAir = true;
		break;

	case MSG_SC8_ARCADENOW:
		sceneHandler08_arcadeNow();
		break;

	case MSG_SC8_RESUMEFLIGHT:
		sceneHandler08_resumeFlight();
		break;

	case MSG_SC8_GETHIMUP:
		g_vars->scene08_manOffsetY = 0;
		g_vars->scene08_flyingUp = true;
		break;

	case MSG_STARTARCADE:
		sceneHandler08_startArcade();
		break;

	case 29:
		if (g_vars->scene08_inArcade) {
			if (g_vars->scene08_inAir) {
				sceneHandler08_airMoves();
				break;
			}
			if (g_vars->scene08_onBelly) {
				sceneHandler08_jumpLogic(cmd);
				break;
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_fp->_aniMan2) {
				if (g_vars->scene08_inArcade) {
					int scHeight = g_fp->_sceneRect.bottom - g_fp->_sceneRect.top;

					if (g_fp->_aniMan2->_oy < g_fp->_sceneRect.top + 200) {
						g_fp->_sceneRect.top = g_fp->_aniMan2->_oy - 200;

						if (g_fp->_sceneRect.top < 0)
							g_fp->_sceneRect.top = 0;

						g_fp->_sceneRect.bottom = scHeight + g_fp->_sceneRect.top;
					}

					if (g_fp->_aniMan2->_oy > g_fp->_sceneRect.bottom - 350) {
						g_fp->_sceneRect.bottom = g_fp->_aniMan2->_oy + 350;
						g_fp->_sceneRect.top = g_fp->_aniMan2->_oy + 350 - scHeight;
					}
				} else {
					if (g_fp->_aniMan2->_ox < g_fp->_sceneRect.left + 200)
						g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.left - 300;

					if (g_fp->_aniMan2->_ox > g_fp->_sceneRect.right - 200)
						g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.right + 300;

					res = 1;
				}
			}

			g_fp->_floaters->update();

			if (g_vars->scene08_inArcade) {
				if (g_vars->scene08_inAir)
					sceneHandler08_calcFlight();
			} else {
				Movement *mov = g_fp->_aniMan->_movement;

				if (mov) {
					if (mov->_id == MV_MAN_TOLADDERD && mov->_currDynamicPhaseIndex == 8)
						g_fp->_aniMan->_priority = 2;

					if (mov && mov->_id == MV_MAN_FROMLADDERUP && mov->_currDynamicPhaseIndex == 13)
						g_fp->_aniMan->_priority = 20;
				}

				g_fp->_behaviorManager->updateBehaviors();
				g_fp->startSceneTrack();
			}

			if (g_vars->scene08_flyingUp)
				sceneHandler08_checkEndArcade();

			if (g_vars->scene08_snoringCountdown > 0) {
				g_vars->scene08_snoringCountdown--;

				if (!g_vars->scene08_snoringCountdown) {
					g_fp->playSound(SND_8_014, 0);

					g_vars->scene08_snoringCountdown = 71;
				}
			}

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
