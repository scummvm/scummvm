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
#include "ngi/gfx.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"
#include "ngi/scene.h"
#include "ngi/messages.h"
#include "ngi/gameloader.h"
#include "ngi/behavior.h"
#include "ngi/interaction.h"

namespace NGI {

void scene06_initMumsy() {
	g_vars->scene06_mumsyJumpFw = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene06_mumsy, ST_MOM_STANDS, QU_MOM_JUMPFW);
	g_vars->scene06_mumsyJumpBk = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(g_vars->scene06_mumsy, ST_MOM_STANDS, QU_MOM_JUMPBK);
	g_vars->scene06_mumsyJumpFwPercent = g_vars->scene06_mumsyJumpFw->_percent;
	g_vars->scene06_mumsyJumpBkPercent = g_vars->scene06_mumsyJumpBk->_percent;
}

int scene06_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_vars->scene06_arcadeEnabled) {
		if (g_vars->scene06_aimingBall) {
			g_nmi->_cursorId = PIC_CSR_ARCADE2_D;

			return PIC_CSR_ARCADE2_D;
		}
		if (g_nmi->_aniMan == g_nmi->_objectAtCursor) {
			if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN6_BALL && g_nmi->_cursorId == PIC_CSR_DEFAULT) {
				g_nmi->_cursorId = PIC_CSR_ITN;

				return PIC_CSR_ITN;
			}
		} else if (g_nmi->_objectAtCursor && g_nmi->_objectAtCursor == g_vars->scene06_currentBall
					&& g_nmi->_cursorId == PIC_CSR_DEFAULT) {
			g_nmi->_cursorId = PIC_CSR_ITN;
		}
	}

	return g_nmi->_cursorId;
}

void sceneHandler06_setExits(Scene *sc) {
	MotionController *mc = getSc2MctlCompoundBySceneId(sc->_sceneId);

	mc->enableLinks(sO_CloseThing, (g_nmi->getObjectState(sO_BigMumsy) != g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsGone)));
	mc->enableLinks(sO_CloseThing2, g_vars->scene06_arcadeEnabled);
}

void sceneHandler06_winArcade() {
	g_nmi->setObjectState(sO_BigMumsy, g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsGone));

	if (g_nmi->getObjectState(sO_ClockAxis) == g_nmi->getObjectEnumState(sO_ClockAxis, sO_IsNotAvailable))
		g_nmi->setObjectState(sO_ClockAxis, g_nmi->getObjectEnumState(sO_ClockAxis, sO_WithoutHandle));

	if (g_vars->scene06_arcadeEnabled) {
		g_nmi->_aniMan->_callback2 = 0; // Really NULL

		g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

		if (g_vars->scene06_someBall) {
			g_vars->scene06_someBall->_flags &= 0xFFFB;

			g_vars->scene06_balls.push_back(g_vars->scene06_someBall);

			g_vars->scene06_someBall = 0;
		}

		if (g_vars->scene06_flyingBall) {
			g_vars->scene06_flyingBall->_flags &= 0xFFFB;

			g_vars->scene06_balls.push_back(g_vars->scene06_flyingBall);

			g_vars->scene06_flyingBall = 0;
		}

		if (g_vars->scene06_ballInHands) {
			g_vars->scene06_ballInHands->_flags &= 0xFFFB;

			g_vars->scene06_balls.push_back(g_vars->scene06_ballInHands);

			g_vars->scene06_ballInHands = 0;
		}

		g_vars->scene06_arcadeEnabled = false;
		g_vars->scene06_aimingBall = false;
	}

	g_vars->scene06_mumsy->_flags &= 0xFFFB;

	sceneHandler06_setExits(g_nmi->_currentScene);

	getCurrSceneSc2MotionController()->activate();
	getGameLoaderInteractionController()->enableFlag24();
}

void sceneHandler06_enableDrops() {
	chainQueue(QU_SC6_DROPS, 0);

	g_vars->scene06_mumsy->changeStatics2(ST_MOM_SITS);
	g_nmi->setObjectState(sO_BigMumsy, g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsPlaying));

	chainQueue(QU_MOM_STANDUP, 1);

	g_vars->scene06_arcadeEnabled = true;
	g_vars->scene06_numBallsGiven = 0;
	g_vars->scene06_mumsyPos = 0;
	g_vars->scene06_mumsyNumBalls = 0;
	g_vars->scene06_mumsyGotBall = false;

	sceneHandler06_setExits(g_nmi->_currentScene);
}

void sceneHandler06_mumsyBallTake() {
	int momAni = 0;

	switch (g_vars->scene06_mumsyNumBalls) {
	case 1:
		momAni = MV_MOM_TAKE1;
		break;
	case 2:
		momAni = MV_MOM_TAKE2;
		break;
	case 3:
		momAni = MV_MOM_TAKE3;
		break;
	case 4:
		momAni = MV_MOM_TAKE4;
		break;
	case 5:
		momAni = MV_MOM_TAKE5;
		break;
	default:
		break;
	}

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	ExCommand *ex = new ExCommand(ANI_MAMASHA, 2, 50, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags = 2u;
	mq->addExCommandToEnd(ex);

	if (g_vars->scene06_mumsyNumBalls >= 5) {
		g_nmi->setObjectState(sO_BigMumsy, g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsGone));

		if (g_nmi->getObjectState(sO_ClockAxis) == g_nmi->getObjectEnumState(sO_ClockAxis, sO_IsNotAvailable))
			g_nmi->setObjectState(sO_ClockAxis, g_nmi->getObjectEnumState(sO_ClockAxis, sO_WithoutHandle));

		ex = new ExCommand(ANI_MAMASHA, 1, momAni, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);

		if (g_vars->scene06_mumsyPos + 3 >= 0) {
			ex = new ExCommand(ANI_MAMASHA, 1, MV_MOM_STARTBK, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2u;
			mq->addExCommandToEnd(ex);

			for (int i = 0; i < g_vars->scene06_mumsyPos + 3; i++) {
				ex = new ExCommand(ANI_MAMASHA, 1, MV_MOM_CYCLEBK, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 2;
				mq->addExCommandToEnd(ex);
			}

			ex = new ExCommand(ANI_MAMASHA, 1, MV_MOM_STOPBK, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(0, 18, QU_MOM_TOLIFT, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);
	} else {
		if (momAni) {
			ex = new ExCommand(ANI_MAMASHA, 1, momAni, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);
		}

		if (g_vars->scene06_mumsyPos < 0) {
			for (int i = 0; i > g_vars->scene06_mumsyPos; i--) {
				ex = new ExCommand(ANI_MAMASHA, 1, MV_MOM_JUMPFW, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 2;
				mq->addExCommandToEnd(ex);
			}
		} else if (g_vars->scene06_mumsyPos > 0) {
			for (int i = 0; i < g_vars->scene06_mumsyPos; i++) {
				ex = new ExCommand(ANI_MAMASHA, 1, MV_MOM_JUMPBK, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 2;
				mq->addExCommandToEnd(ex);
			}
		}

		ex = new ExCommand(0, 18, QU_MOM_SITDOWN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3u;
		mq->addExCommandToEnd(ex);
	}

	mq->setFlags(mq->getFlags() | 1);
	mq->chain(0);

	g_vars->scene06_mumsyNumBalls = 0;
	g_vars->scene06_arcadeEnabled = false;

	g_nmi->_aniMan2 = 0;
}

void sceneHandler06_spinHandle() {
	int tummy = g_nmi->getObjectState(sO_TummyTrampie);

	if (tummy == g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsEating))
		g_nmi->setObjectState(sO_TummyTrampie, g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping));
	else if (tummy == g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping))
		g_nmi->setObjectState(sO_TummyTrampie, g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsDrinking));
	else if (tummy == g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsDrinking))
		g_nmi->setObjectState(sO_TummyTrampie, g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsScratchingBelly));
	else if (tummy == g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsScratchingBelly))
		g_nmi->setObjectState(sO_TummyTrampie, g_nmi->getObjectEnumState(sO_TummyTrampie, sO_IsEating));
}

void sceneHandler06_uPipeClick() {
	if (getGameLoaderInteractionController()->_flag24)
		handleObjectInteraction(g_nmi->_aniMan2, g_nmi->_currentScene->getPictureObjectById(PIC_SC6_LADDER, 0), 0);
}

void sceneHandler06_buttonPush() {
	g_vars->scene06_invHandle = g_nmi->_currentScene->getStaticANIObject1ById(ANI_INV_HANDLE, -1);

	if (g_vars->scene06_invHandle)
		if (g_vars->scene06_invHandle->_flags & 4)
			if (g_vars->scene06_invHandle->_statics)
				if (g_vars->scene06_invHandle->_statics->_staticsId == ST_HDL_PLUGGED)
					chainQueue(QU_SC6_FALLHANDLE, 1);
}

void sceneHandler06_showNextBall() {
	if (g_vars->scene06_balls.size()) {
		g_vars->scene06_currentBall = g_vars->scene06_balls.remove_at(0);

		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC6_SHOWNEXTBALL), 0, 1);

		mq->setParamInt(-1, g_vars->scene06_currentBall->_odelay);
		mq->chain(0);

		++g_vars->scene06_numBallsGiven;
	}
}

void sceneHandler06_installHandle() {
	chainQueue(QU_SC6_SHOWHANDLE, 0);
}

int sceneHandler06_updateScreenCallback() {
	int res;

	res = g_nmi->drawArcadeOverlay(g_vars->scene06_arcadeEnabled);

	if (!res)
		g_nmi->_updateScreenCallback = 0;

	return res;
}

void sceneHandler06_startAiming() {
	if (g_vars->scene06_currentBall) {
		g_vars->scene06_currentBall->hide();

		g_nmi->_aniMan->startAnim(MV_MAN6_TAKEBALL, 0, -1);

		g_vars->scene06_ballInHands = g_vars->scene06_currentBall;
		g_vars->scene06_currentBall = 0;

		if (getCurrSceneSc2MotionController()->_isEnabled)
			g_nmi->_updateScreenCallback = sceneHandler06_updateScreenCallback;

		getCurrSceneSc2MotionController()->deactivate();
		getGameLoaderInteractionController()->disableFlag24();

		g_vars->scene06_ballDrop->queueMessageQueue(0);
	}
}

void sceneHandler06_takeBall() {
	if (g_vars->scene06_currentBall && !g_vars->scene06_currentBall->_movement && g_vars->scene06_currentBall->_statics->_staticsId == ST_NBL_NORM) {
		if (abs(1158 - g_nmi->_aniMan->_ox) > 1
			|| abs(452 - g_nmi->_aniMan->_oy) > 1
			|| g_nmi->_aniMan->_movement
			|| g_nmi->_aniMan->_statics->_staticsId != (0x4000 | ST_MAN_RIGHT)) {
			MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 1158, 452, 1, (0x4000 | ST_MAN_RIGHT));

			if (mq) {
				ExCommand *ex = new ExCommand(0, 17, MSG_SC6_TAKEBALL, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				mq->addExCommandToEnd(ex);

				postExCommand(g_nmi->_aniMan->_id, 2, 1158, 452, 0, -1);
			}
		} else {
			sceneHandler06_startAiming();
		}
	}
}

void sceneHandler06_aiming() {
	if (g_vars->scene06_ballInHands) {
		g_vars->scene06_ballDeltaX = 4 * g_nmi->_aniMan->_movement->_currDynamicPhaseIndex + 16;
		g_vars->scene06_ballDeltaY = 5 * (g_nmi->_aniMan->_movement->_currDynamicPhaseIndex + 4);

		if (g_nmi->_aniMan->_movement->_currDynamicPhaseIndex < 4) {
			g_nmi->_aniMan->_movement->setDynamicPhaseIndex(11);

			g_vars->scene06_aimingBall = false;

			return;
		}

		g_nmi->_aniMan->_movement->setDynamicPhaseIndex(9);
	}

	g_vars->scene06_aimingBall = false;
}

void sceneHandler06_ballStartFly() {
	if (g_vars->scene06_ballInHands) {
		g_vars->scene06_flyingBall = g_vars->scene06_ballInHands;
		g_vars->scene06_ballInHands = 0;
		g_vars->scene06_flyingBall->show1(g_nmi->_aniMan->_ox - 60, g_nmi->_aniMan->_oy - 60, -1, 0);

		g_vars->scene06_flyingBall->_priority = 27;
	}
}

void sceneHandler06_throwCallback(int *arg) {
	if (g_vars->scene06_aimingBall) {
		int dist = (g_nmi->_mouseVirtY - g_vars->scene06_sceneClickY)
			* (g_nmi->_mouseVirtY - g_vars->scene06_sceneClickY)
			+ (g_nmi->_mouseVirtX - g_vars->scene06_sceneClickX)
			* (g_nmi->_mouseVirtX - g_vars->scene06_sceneClickX);

		*arg = (int)(sqrt((double)dist) * 0.1);

		if (*arg > 8)
			*arg = 8;
	} else {
		*arg = *arg + 1;
		if (*arg == 12)
			sceneHandler06_ballStartFly();
	}
}

void sceneHandler06_throwBall() {
	g_nmi->_aniMan->_callback2 = sceneHandler06_throwCallback;
	g_nmi->_aniMan->startAnim(MV_MAN6_THROWBALL, 0, -1);

	g_vars->scene06_aimingBall = true;
}

void sceneHandler06_eggieWalk() {
	if (15 - g_vars->scene06_numBallsGiven >= 4 && !g_nmi->_rnd.getRandomNumber(9)) {
		StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_EGGIE, -1);

		if (!ani || !(ani->_flags & 4)) {
			if (g_vars->scene06_eggieDirection)
				chainQueue(QU_EGG6_GOR, 0);
			else
				chainQueue(QU_EGG6_GOL, 0);

			g_vars->scene06_eggieTimeout = 0;
			g_vars->scene06_eggieDirection = !g_vars->scene06_eggieDirection;
		}
	}
}

void sceneHandler06_dropBall() {
	if (g_vars->scene06_numBallsGiven >= 15 || g_vars->scene06_mumsyNumBalls >= 5)
		g_vars->scene06_ballDrop->hide();
	else
		chainQueue(QU_SC6_DROPS3, 0);
}

void sceneHandler06_fallBall() {
	g_vars->scene06_ballY = 475;

	g_vars->scene06_flyingBall->setOXY(g_vars->scene06_ballX, g_vars->scene06_ballY);

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC6_FALLBALL), 0, 1);

	mq->setParamInt(-1, g_vars->scene06_flyingBall->_odelay);
	mq->chain(0);

	g_vars->scene06_balls.push_back(g_vars->scene06_flyingBall);

	g_vars->scene06_flyingBall = 0;

	sceneHandler06_dropBall();
	sceneHandler06_eggieWalk();
}

void sceneHandler06_catchBall() {
	if (g_vars->scene06_flyingBall) {
		g_vars->scene06_flyingBall->hide();

		g_vars->scene06_balls.push_back(g_vars->scene06_flyingBall);

		g_vars->scene06_flyingBall = 0;

		g_vars->scene06_mumsyNumBalls++;

		if (g_vars->scene06_mumsy->_movement) {
			Common::Point point;

			if (g_vars->scene06_mumsy->_movement->_id == MV_MOM_JUMPFW) {
				if (g_vars->scene06_mumsy->_movement->_currDynamicPhaseIndex <= 5) {
					point = g_vars->scene06_mumsy->_movement->calcSomeXY(0, g_vars->scene06_mumsy->_movement->_currDynamicPhaseIndex);

					point.x = -point.x;
					point.y = -point.y;
				} else {
					point = g_vars->scene06_mumsy->_movement->calcSomeXY(1, -1);

					g_vars->scene06_mumsyPos++;
				}
			} else if (g_vars->scene06_mumsy->_movement->_id == MV_MOM_JUMPBK) {
				if (g_vars->scene06_mumsy->_movement->_currDynamicPhaseIndex <= 4) {
					point = g_vars->scene06_mumsy->_movement->calcSomeXY(0, g_vars->scene06_mumsy->_movement->_currDynamicPhaseIndex);

					point.x = -point.x;
					point.y = -point.y;
				} else {
					point = g_vars->scene06_mumsy->_movement->calcSomeXY(1, -1);

					g_vars->scene06_mumsyPos--;
				}
			}

			g_vars->scene06_mumsy->changeStatics2(ST_MOM_STANDS);
			g_vars->scene06_mumsy->setOXY(point.x + g_vars->scene06_mumsy->_ox,
										  point.y + g_vars->scene06_mumsy->_oy);
		} else {
			g_vars->scene06_mumsy->changeStatics2(ST_MOM_STANDS);
		}

		chainQueue(QU_MOM_PUTBALL, 1);
		g_vars->scene06_mumsyGotBall = true;

		sceneHandler06_dropBall();
	}
}

void sceneHandler06_checkBallTarget(int par) {
	if (g_vars->scene06_ballY <= 475) {
		if (g_vars->scene06_mumsy->isPixelHitAtPos(g_vars->scene06_ballX, g_vars->scene06_ballY)) {
			chainObjQueue(g_vars->scene06_mumsy, QU_MOM_JUMPBK, 0);

			sceneHandler06_catchBall();
		}
	} else {
		sceneHandler06_fallBall();
	}
}

void scene06_initScene(Scene *sc) {
	g_vars->scene06_mumsy = sc->getStaticANIObject1ById(ANI_MAMASHA, -1);
	g_vars->scene06_someBall = 0;
	g_vars->scene06_invHandle = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
	g_vars->scene06_liftButton = sc->getStaticANIObject1ById(ANI_BUTTON_6, -1);
	g_vars->scene06_ballDrop = sc->getStaticANIObject1ById(ANI_BALLDROP, -1);
	g_vars->scene06_arcadeEnabled = false;
	g_vars->scene06_aimingBall = false;
	g_vars->scene06_currentBall = 0;
	g_vars->scene06_ballInHands = 0;
	g_vars->scene06_flyingBall = 0;
	g_vars->scene06_balls.clear();
	g_vars->scene06_numBallsGiven = 0;
	g_vars->scene06_mumsyNumBalls = 0;
	g_vars->scene06_eggieTimeout = 0;
	g_vars->scene06_eggieDirection = true;

	StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_NEWBALL, -1);

	ball->hide();
	ball->_statics = ball->getStaticsById(ST_NBL_NORM);
	g_vars->scene06_balls.push_back(ball);

	for (int i = 0; i < 3; i++) {
		StaticANIObject *ball2 = new StaticANIObject(ball);

		ball2->hide();
		ball2->_statics = ball2->getStaticsById(ST_NBL_NORM);

		sc->addStaticANIObject(ball2, 1);

		g_vars->scene06_balls.push_back(ball2);
	}

	if (g_nmi->getObjectState(sO_BigMumsy) == g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsPlaying))
		g_nmi->setObjectState(sO_BigMumsy, g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsSleeping));

	if (g_nmi->getObjectState(sO_BigMumsy) != g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsSleeping))
		g_vars->scene06_mumsy->hide();

	g_nmi->lift_setButton(sO_Level3, ST_LBN_3N);
	g_nmi->lift_init(sc, QU_SC6_ENTERLIFT, QU_SC6_EXITLIFT);
	g_nmi->initArcadeKeys("SC_6");

	sceneHandler06_setExits(sc);

	g_nmi->setArcadeOverlay(PIC_CSR_ARCADE2);
}

int sceneHandler06(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(ex);
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler06_winArcade();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_SC6_RESTORESCROLL:
		g_nmi->_aniMan2 = g_nmi->_aniMan;
		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enableFlag24();
		sceneHandler06_setExits(g_nmi->_currentScene);
		break;

	case MSG_SC6_STARTDROPS:
		if (g_nmi->getObjectState(sO_BigMumsy) == g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsSleeping))
			sceneHandler06_enableDrops();
		break;

	case MSG_SC6_TESTNUMBALLS:
		g_vars->scene06_mumsyGotBall = false;

		if (g_vars->scene06_mumsyNumBalls < 5 || !g_vars->scene06_arcadeEnabled)
			return 0;

		sceneHandler06_mumsyBallTake();
		break;

	case MSG_SC6_JUMPFW:
		++g_vars->scene06_mumsyPos;
		break;

	case MSG_SC6_JUMPBK:
		--g_vars->scene06_mumsyPos;
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SPINHANDLE:
		sceneHandler06_spinHandle();
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case MSG_SC6_UTRUBACLICK:
		sceneHandler06_uPipeClick();
		break;

	case MSG_SC6_BTNPUSH:
		sceneHandler06_buttonPush();
		break;

	case MSG_SC6_SHOWNEXTBALL:
		sceneHandler06_showNextBall();
		break;

	case MSG_SC6_INSTHANDLE:
		sceneHandler06_installHandle();
		break;

	case MSG_SC6_ENABLEDROPS:
		sceneHandler06_enableDrops();
		break;

	case 64:
		g_nmi->lift_hoverButton(ex);
		break;

	case MSG_SC6_TAKEBALL:
		sceneHandler06_takeBall();
		break;

	case 30:
		if (g_vars->scene06_aimingBall) {
			sceneHandler06_aiming();
			break;
		}

		if (!g_vars->scene06_arcadeEnabled) {
			// Do nothing
			break;
		}
		break;

	case 29:
		{
			StaticANIObject *st = g_nmi->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (st) {
				if (!g_vars->scene06_arcadeEnabled && st->_id == ANI_LIFTBUTTON) {
					g_nmi->lift_animateButton(st);
					ex->_messageKind = 0;
					return 0;
				}

				if (g_vars->scene06_currentBall == st) {
					if (g_vars->scene06_numBallsGiven == 1)
						sceneHandler06_takeBall();

					ex->_messageKind = 0;
				} else if (g_vars->scene06_ballInHands && g_nmi->_aniMan == st && !g_nmi->_aniMan->_movement && g_nmi->_aniMan->_statics->_staticsId == ST_MAN6_BALL) {
					g_vars->scene06_sceneClickX = ex->_sceneClickX;
					g_vars->scene06_sceneClickY = ex->_sceneClickY;

					sceneHandler06_throwBall();
				}
			}

			if (!st || !canInteractAny(g_nmi->_aniMan, st, ex->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, ex->_param)) {
					if ((g_nmi->_sceneRect.right - ex->_sceneClickX < 47
						 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
						|| (ex->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0)) {
						g_nmi->processArcade(ex);
						return 0;
					}
				}
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_nmi->_aniMan2) {
				int ox = g_nmi->_aniMan2->_ox;
				int oy = g_nmi->_aniMan2->_oy;

				g_vars->scene06_manX = ox;
				g_vars->scene06_manY = oy;

				if (g_vars->scene06_arcadeEnabled && oy <= 470 && ox >= 1088) {
					if (ox < g_nmi->_sceneRect.left + 600) {
						g_nmi->_currentScene->_x = ox - g_nmi->_sceneRect.left - 700;
						ox = g_vars->scene06_manX;
					}

					if (ox > g_nmi->_sceneRect.right - 50)
						g_nmi->_currentScene->_x = ox - g_nmi->_sceneRect.right + 70;
				} else {
					if (ox < g_nmi->_sceneRect.left + 200) {
						g_nmi->_currentScene->_x = ox - g_nmi->_sceneRect.left - 300;
						ox = g_vars->scene06_manX;
					}

					if (ox > g_nmi->_sceneRect.right - 200)
						g_nmi->_currentScene->_x = ox - g_nmi->_sceneRect.right + 300;
				}

				res = 1;

				g_nmi->sceneAutoScrolling();
			}
			if (g_vars->scene06_arcadeEnabled) {
				if (g_vars->scene06_mumsyPos > -3)
					g_vars->scene06_mumsyJumpBk->_percent = g_vars->scene06_mumsyJumpBkPercent;
				else
					g_vars->scene06_mumsyJumpBk->_percent = 0;

				if (g_vars->scene06_mumsyPos < 4)
					g_vars->scene06_mumsyJumpFw->_percent = g_vars->scene06_mumsyJumpFwPercent;
				else
					g_vars->scene06_mumsyJumpFw->_percent = 0;

				if (g_vars->scene06_aimingBall) {
					g_vars->scene06_eggieTimeout++;

					if (g_vars->scene06_eggieTimeout >= 600)
						sceneHandler06_eggieWalk();
				}
			} else {
				g_vars->scene06_mumsyJumpFw->_percent = 0;
				g_vars->scene06_mumsyJumpBk->_percent = 0;
			}

			if (g_vars->scene06_flyingBall) {
				g_vars->scene06_ballX = g_vars->scene06_flyingBall->_ox - g_vars->scene06_ballDeltaX;
				g_vars->scene06_ballY = g_vars->scene06_flyingBall->_oy - g_vars->scene06_ballDeltaY;

				g_vars->scene06_flyingBall->setOXY(g_vars->scene06_ballX, g_vars->scene06_ballY);

				if (g_vars->scene06_ballDeltaX >= 2)
					g_vars->scene06_ballDeltaX -= 2;

				g_vars->scene06_ballDeltaY -= 5;

				sceneHandler06_checkBallTarget(g_vars->scene06_ballDeltaX);
			}
			if (g_vars->scene06_arcadeEnabled
				&& !g_vars->scene06_currentBall
				&& !g_vars->scene06_ballInHands
				&& !g_vars->scene06_flyingBall
				&& g_vars->scene06_numBallsGiven >= 15
				&& !g_vars->scene06_ballDrop->_movement
				&& !g_vars->scene06_mumsy->_movement
				&& !g_vars->scene06_mumsyGotBall)
				sceneHandler06_mumsyBallTake();
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
