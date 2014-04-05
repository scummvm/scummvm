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

void scene32_initScene(Scene *sc) {
	g_vars->scene32_flagIsWaving = false;
	g_vars->scene32_flagNeedsStopping = false;
	g_vars->scene32_dudeIsSitting = false;
	g_vars->scene32_cactusCounter = -1;
	g_vars->scene32_dudeOnLadder = false;
	g_vars->scene32_cactusIsGrowing = false;
	g_vars->scene32_flag = sc->getStaticANIObject1ById(ANI_FLAG, -1);
	g_vars->scene32_cactus = sc->getStaticANIObject1ById(ANI_CACTUS, -1);
	g_vars->scene32_massOrange = sc->getStaticANIObject1ById(ANI_TESTO_ORANGE, -1);
	g_vars->scene32_massBlue = sc->getStaticANIObject1ById(ANI_TESTO_BLUE, -1);
	g_vars->scene32_massGreen = sc->getStaticANIObject1ById(ANI_TESTO_GREEN, -1);
	g_vars->scene32_button = sc->getStaticANIObject1ById(ANI_BUTTON_32, -1);

	g_vars->scene32_massOrange->startAnim(MV_TSTO_FLOW, 0, -1);
	g_vars->scene32_massOrange->_movement->setDynamicPhaseIndex(15);

	g_vars->scene32_massGreen->startAnim(MV_TSTG_FLOW, 0, -1);
	g_vars->scene32_massGreen->_movement->setDynamicPhaseIndex(26);

	Scene *oldsc = g_fp->_currentScene;
	StaticANIObject *ani;

	if (g_fp->getObjectState(sO_ClockHandle) == g_fp->getObjectEnumState(sO_ClockHandle, sO_In_32_Lies)) {
		ani = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
		if (ani) {
			g_fp->_currentScene = sc;

			ani->changeStatics2(ST_HDL_LAID);
		}
	} else {
		if (g_fp->getObjectState(sO_ClockHandle) == g_fp->getObjectEnumState(sO_ClockHandle, sO_In_32_Sticks)) {
			ani = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);

			g_fp->_currentScene = sc;

			if (ani)
				ani->changeStatics2(ST_HDL_PLUGGED);

			g_vars->scene32_button->changeStatics2(ST_BTN32_ON);
		}
	}

	g_fp->_currentScene = oldsc;

	if (g_fp->getObjectState(sO_Cube) == g_fp->getObjectEnumState(sO_Cube, sO_In_32)) {
		MessageQueue *mq = new MessageQueue(sc->getMessageQueueById(QU_KBK32_START), 0, 0);

		mq->sendNextCommand();
	}

	g_fp->lift_setButton(sO_Level9, ST_LBN_9N);
	g_fp->lift_init(sc, QU_SC32_ENTERLIFT, QU_SC32_EXITLIFT);

	g_fp->initArcadeKeys("SC_32");
}

void scene32_setupMusic() {
	if (g_fp->lift_checkButton(sO_Level6))
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_32"), "MUSIC2", 1);
}

int scene32_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor == PIC_SC32_LADDER && g_fp->_cursorId == PIC_CSR_ITN)
		g_fp->_cursorId = g_vars->scene32_dudeOnLadder ? PIC_CSR_GOD : PIC_CSR_GOU; // TODO FIXME doublecheck

	return g_fp->_cursorId;
}

void sceneHandler32_tryCube() {
	if (g_fp->getObjectState(sO_Cube) == g_fp->getObjectEnumState(sO_Cube, sO_In_33))
		chainQueue(QU_KBK32_GO, 0);
}

void sceneHandler32_startCactus() {
	g_vars->scene32_cactusCounter = 48;
	g_vars->scene32_cactusIsGrowing = false;
}

void sceneHandler32_spin(ExCommand *cmd) {
	MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

	if (!mq || mq->getCount() == 0)
		return;

	ExCommand *ex = mq->getExCommandByIndex(0);
	ExCommand *newex;

	if ((g_vars->scene32_cactus->_movement && g_vars->scene32_cactus->_movement->_id == MV_CTS_DEFAULT)
		|| g_vars->scene32_cactus->_statics->_staticsId == ST_CTS_GROWUP) {
		for (int i = 0; i < 12; i++) {
			newex = ex->createClone();
			newex->_excFlags |= 2;
			mq->insertExCommandAt(1, newex);
		}

		g_vars->scene32_cactus->changeStatics2(ST_CTS_GROWUP);

		chainQueue(QU_CTS_BACK, 1);

		g_vars->scene32_cactusIsGrowing = false;

		return;
	}

	if (g_vars->scene32_cactus->_statics->_staticsId == ST_CTS_EMPTY && g_vars->scene32_cactusCounter < 0) {
		for (int i = 0; i < 2; i++) {
			newex = ex->createClone();
			newex->_excFlags |= 2;
			mq->insertExCommandAt(1, newex);
		}

		chainQueue(QU_KDK_DRIZZLE, 0);
	}
}

void sceneHandler32_startFlagLeft() {
	g_vars->scene32_flag->changeStatics2(ST_FLG_NORM);
	g_vars->scene32_flag->startAnim(MV_FLG_STARTL, 0, -1);

	g_vars->scene32_flagIsWaving = true;
}

void sceneHandler32_startFlagRight() {
	g_vars->scene32_flag->changeStatics2(ST_FLG_NORM);
	g_vars->scene32_flag->startAnim(MV_FLG_STARTR, 0, -1);

	g_vars->scene32_flagIsWaving = true;
}

void sceneHandler32_trySit(ExCommand *cmd) {
	MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

	if (!mq || mq->getCount() == 0)
		return;

	ExCommand *ex = mq->getExCommandByIndex(0);

	if (g_vars->scene32_cactusIsGrowing || g_vars->scene32_cactus->_movement
		|| g_vars->scene32_cactus->_statics->_staticsId != ST_CTS_EMPTY
		|| (g_vars->scene32_cactusCounter >= 0 && g_vars->scene32_cactusCounter <= 20)) {
		ex->_messageKind = 0;
		ex->_excFlags |= 1;
	} else {
		ex->_parentId = ANI_MAN;
		ex->_messageKind = 1;
		ex->_messageNum = MV_MAN32_SITDOWN;
		ex->_keyCode = g_fp->_aniMan->_okeyCode;

		g_vars->scene32_dudeIsSitting = true;

		getCurrSceneSc2MotionController()->clearEnabled();
		getGameLoaderInteractionController()->disableFlag24();
	}
}

void sceneHandler32_buttonPush() {
	if (g_fp->getObjectState(sO_ClockHandle) == g_fp->getObjectEnumState(sO_ClockHandle, sO_In_32_Sticks)) {
		StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
		if (ani)
			ani->changeStatics2(ST_HDL_PLUGGED);

		chainQueue(QU_SC32_FALLHANDLE, 1);

		g_vars->scene32_button->changeStatics2(ST_BTN32_OFF);
	}
}

void sceneHandler32_installHandle() {
	chainQueue(QU_SC32_SHOWHANDLE, 0);

	g_vars->scene32_button->changeStatics2(ST_BTN32_ON);
}

void sceneHandler32_animateCactus() {
	if (g_fp->_aniMan->_statics->_staticsId != ST_MAN32_SIT)
		chainQueue(QU_CTS_GROW, 1);
	else
		chainQueue(QU_CTS_GROWMAN, 1);

	g_vars->scene32_cactusCounter = -1;
	g_vars->scene32_cactusIsGrowing = true;
}

void sceneHandler32_ladderLogic(ExCommand *cmd) {
	MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC32_FROMLADDER), 0, 0);

	if (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) != PIC_SC32_LADDER)
		mq->addExCommandToEnd(cmd->createClone());

	mq->setFlags(mq->getFlags() | 1);

	g_fp->_aniMan->changeStatics2(ST_MAN_STANDLADDER);
	if (!mq->chain(g_fp->_aniMan))
		delete mq;

	g_vars->scene32_dudeOnLadder = false;

	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();
}

void sceneHandler32_potLogic(ExCommand *cmd) {
	if (g_vars->scene32_cactusCounter < 0 || g_vars->scene32_cactusCounter > 20) {
		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

		ExCommand *ex = new ExCommand(ANI_MAN, 1, MV_MAN32_STANDUP, 0, 0, 0, 1, 0, 0, 0);

		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);

		StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

		if (!ani || ani->_id != ANI_KADKA)
			mq->addExCommandToEnd(cmd->createClone());

		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);

		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();

		g_vars->scene32_dudeIsSitting = false;
	}
}

int sceneHandler32(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC32_TRUBATOBACK:
		g_fp->_currentScene->getPictureObjectById(PIC_SC32_RTRUBA, 0)->_priority = 20;
		break;

	case MSG_SC32_TRUBATOFRONT:
		g_fp->_currentScene->getPictureObjectById(PIC_SC32_RTRUBA, 0)->_priority = 0;
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_clickButton();
		break;

	case MSG_SC33_TRYKUBIK:
		sceneHandler32_tryCube();
		break;

	case MSG_SC32_STARTCACTUS:
		sceneHandler32_startCactus();
		break;

	case MSG_SC32_STOPFLAG:
		g_vars->scene32_flagIsWaving = false;
		g_vars->scene32_flagNeedsStopping = true;
		break;

	case MSG_SC32_SPIN:
		sceneHandler32_spin(cmd);
		break;

	case MSG_SC32_STARTFLAGLEFT :
		sceneHandler32_startFlagLeft();
		break;

	case MSG_SC32_STARTFLAGRIGHT:
		sceneHandler32_startFlagRight();
		break;

	case MSG_SC32_TRYSIT:
		sceneHandler32_trySit(cmd);
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC32_ONLADDER:
		g_vars->scene32_dudeOnLadder = true;

		getCurrSceneSc2MotionController()->clearEnabled();
		getGameLoaderInteractionController()->disableFlag24();
		break;

	case MSG_SC6_BTNPUSH:
		sceneHandler32_buttonPush();
		break;

	case 64:
		g_fp->lift_hoverButton(cmd);
		break;

	case MSG_SC6_INSTHANDLE:
		sceneHandler32_installHandle();
		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		if (!g_vars->scene32_flag->_movement) {
			if (g_vars->scene32_flagIsWaving) {
				if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_RIGHT)
					g_vars->scene32_flag->startAnim(MV_FLG_CYCLER, 0, -1);
				else if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_LEFT)
					g_vars->scene32_flag->startAnim(MV_FLG_CYCLEL, 0, -1);
			}

			if (g_vars->scene32_flagNeedsStopping && !g_vars->scene32_flagIsWaving) {
				if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_RIGHT)
					g_vars->scene32_flag->startAnim(MV_FLG_STOPR, 0, -1);
				else if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_LEFT)
					g_vars->scene32_flag->startAnim(MV_FLG_STOPL, 0, -1);

				g_vars->scene32_flagNeedsStopping = false;
			}
		}

		if (g_vars->scene32_cactusCounter) {
			if (g_vars->scene32_cactusCounter > 0)
				--g_vars->scene32_cactusCounter;

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();
		} else {
			sceneHandler32_animateCactus();

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();

		}
		break;

	case 29:
		if (g_vars->scene32_dudeOnLadder) {
			sceneHandler32_ladderLogic(cmd);
			cmd->_messageKind = 0;
			break;
		}

		if (!g_vars->scene32_dudeIsSitting || g_fp->_aniMan->_movement) {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_animateButton(ani);

				cmd->_messageKind = 0;
				break;
			}

			if (g_fp->_cursorId == PIC_CSR_GOFAR_R || g_fp->_cursorId == PIC_CSR_GOFAR_L) {
				if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
					|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
					g_fp->processArcade(cmd);
			}
			break;
		}

		if (!g_vars->scene32_cactusIsGrowing)
			sceneHandler32_potLogic(cmd);

		cmd->_messageKind = 0;

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
