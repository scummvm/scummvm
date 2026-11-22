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

void scene32_initScene(Scene *sc) {
	g_vars->scene32_flagIsWaving = false;
	g_vars->scene32_flagNeedsStopping = false;
	g_vars->scene32_dudeIsSitting = false;
	g_vars->scene32_cactusCounter = -1;
	g_vars->scene32_dudeOnLadder = false;
	g_vars->scene32_cactusIsGrowing = false;
	g_vars->scene32_flag = sc->findAniObjectById(ANI_FLAG, -1);
	g_vars->scene32_cactus = sc->findAniObjectById(ANI_CACTUS, -1);
	g_vars->scene32_massOrange = sc->findAniObjectById(ANI_TESTO_ORANGE, -1);
	g_vars->scene32_massBlue = sc->findAniObjectById(ANI_TESTO_BLUE, -1);
	g_vars->scene32_massGreen = sc->findAniObjectById(ANI_TESTO_GREEN, -1);
	g_vars->scene32_button = sc->findAniObjectById(ANI_BUTTON_32, -1);

	g_vars->scene32_massOrange->startMovement(MV_TSTO_FLOW, 0, -1);
	g_vars->scene32_massOrange->_movement->setCurFrame(15);

	g_vars->scene32_massGreen->startMovement(MV_TSTG_FLOW, 0, -1);
	g_vars->scene32_massGreen->_movement->setCurFrame(26);

	Scene *oldsc = g_nmi->_currentScene;
	AniObject *ani;

	if (g_nmi->getStateId(sO_ClockHandle) == g_nmi->getStateValue(sO_ClockHandle, sO_In_32_Lies)) {
		ani = sc->findAniObjectById(ANI_INV_HANDLE, -1);
		if (ani) {
			g_nmi->_currentScene = sc;

			ani->setStatic(ST_HDL_LAID);
		}
	} else {
		if (g_nmi->getStateId(sO_ClockHandle) == g_nmi->getStateValue(sO_ClockHandle, sO_In_32_Sticks)) {
			ani = sc->findAniObjectById(ANI_INV_HANDLE, -1);

			g_nmi->_currentScene = sc;

			if (ani)
				ani->setStatic(ST_HDL_PLUGGED);

			g_vars->scene32_button->setStatic(ST_BTN32_ON);
		}
	}

	g_nmi->_currentScene = oldsc;

	if (g_nmi->getStateId(sO_Cube) == g_nmi->getStateValue(sO_Cube, sO_In_32)) {
		MessageQueue *mq = new MessageQueue(sc->findMessageQueue(QU_KBK32_START), 0, 0);

		mq->next();
	}

	g_nmi->lift_setButton(sO_Level9, ST_LBN_9N);
	g_nmi->lift_init(sc, QU_SC32_ENTERLIFT, QU_SC32_EXITLIFT);

	g_nmi->initArcadeKeys("SC_32");

	if (g_nmi->getStateId(sO_Cactus) == g_nmi->getStateValue(sO_Cactus, sO_HasGrown)) {
		g_nmi->_currentScene = sc;

		g_vars->scene32_cactus->setStatic(ST_CTS_GROWUP);
		g_vars->scene32_cactus->_z = 22;

		g_nmi->_currentScene = oldsc;
	}
}

void scene32_setupMusic() {
	if (g_nmi->lift_checkButton(sO_Level6))
		g_nmi->playTrack(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_32"), "MUSIC2", 1);
}

int scene32_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor == PIC_SC32_LADDER && g_nmi->_cursorId == PIC_CSR_ITN)
		g_nmi->_cursorId = g_vars->scene32_dudeOnLadder ? PIC_CSR_GOD : PIC_CSR_GOU;

	return g_nmi->_cursorId;
}

void sceneHandler32_tryCube() {
	if (g_nmi->getStateId(sO_Cube) == g_nmi->getStateValue(sO_Cube, sO_In_33))
		runQueue(QU_KBK32_GO, 0);
}

void sceneHandler32_startCactus() {
	g_vars->scene32_cactusCounter = 48;
	g_vars->scene32_cactusIsGrowing = false;
}

void sceneHandler32_spin(Command *cmd) {
	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

	if (!mq || mq->getCount() == 0)
		return;

	Command *ex = mq->getCommandByIndex(0);
	Command *newex;

	if ((g_vars->scene32_cactus->_movement && g_vars->scene32_cactus->_movement->_id == MV_CTS_DEFAULT)
		|| g_vars->scene32_cactus->_statics->_staticsId == ST_CTS_GROWUP) {
		for (int i = 0; i < 12; i++) {
			newex = ex->createClone();
			newex->_excFlags |= 2;
			mq->addCommand(1, newex);
		}

		g_vars->scene32_cactus->setStatic(ST_CTS_GROWUP);

		runQueue(QU_CTS_BACK, 1);

		g_vars->scene32_cactusIsGrowing = false;

		return;
	}

	if (g_vars->scene32_cactus->_statics->_staticsId == ST_CTS_EMPTY && g_vars->scene32_cactusCounter < 0) {
		for (int i = 0; i < 2; i++) {
			newex = ex->createClone();
			newex->_excFlags |= 2;
			mq->addCommand(1, newex);
		}

		runQueue(QU_KDK_DRIZZLE, 0);
	}
}

void sceneHandler32_startFlagLeft() {
	g_vars->scene32_flag->setStatic(ST_FLG_NORM);
	g_vars->scene32_flag->startMovement(MV_FLG_STARTL, 0, -1);

	g_vars->scene32_flagIsWaving = true;
}

void sceneHandler32_startFlagRight() {
	g_vars->scene32_flag->setStatic(ST_FLG_NORM);
	g_vars->scene32_flag->startMovement(MV_FLG_STARTR, 0, -1);

	g_vars->scene32_flagIsWaving = true;
}

void sceneHandler32_trySit(Command *cmd) {
	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

	if (!mq || mq->getCount() == 0)
		return;

	Command *ex = mq->getCommandByIndex(0);

	if (g_vars->scene32_cactusIsGrowing || g_vars->scene32_cactus->_movement
		|| g_vars->scene32_cactus->_statics->_staticsId != ST_CTS_EMPTY
		|| (g_vars->scene32_cactusCounter >= 0 && g_vars->scene32_cactusCounter <= 20)) {
		ex->_cmd = 0;
		ex->_excFlags |= 1;
	} else {
		ex->_parentId = ANI_MAN;
		ex->_cmd = 1;
		ex->_messageNum = MV_MAN32_SITDOWN;
		ex->_param = g_nmi->_aniMan->_copyId;

		g_vars->scene32_dudeIsSitting = true;

		getCurrSceneSc2MotionController()->deactivate();
		getGameLoaderInteractionController()->disable();
	}
}

void sceneHandler32_buttonPush() {
	if (g_nmi->getStateId(sO_ClockHandle) == g_nmi->getStateValue(sO_ClockHandle, sO_In_32_Sticks)) {
		AniObject *ani = g_nmi->_currentScene->findAniObjectById(ANI_INV_HANDLE, -1);
		if (ani)
			ani->setStatic(ST_HDL_PLUGGED);

		runQueue(QU_SC32_FALLHANDLE, 1);

		g_vars->scene32_button->setStatic(ST_BTN32_OFF);
	}
}

void sceneHandler32_installHandle() {
	runQueue(QU_SC32_SHOWHANDLE, 0);

	g_vars->scene32_button->setStatic(ST_BTN32_ON);
}

void sceneHandler32_animateCactus() {
	if (g_nmi->_aniMan->_statics->_staticsId != ST_MAN32_SIT)
		runQueue(QU_CTS_GROW, 1);
	else
		runQueue(QU_CTS_GROWMAN, 1);

	g_vars->scene32_cactusCounter = -1;
	g_vars->scene32_cactusIsGrowing = true;
}

void sceneHandler32_ladderLogic(Command *cmd) {
	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC32_FROMLADDER), 0, 0);

	if (g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) != PIC_SC32_LADDER)
		mq->addCommandToEnd(cmd->createClone());

	mq->setFlags(mq->getFlags() | 1);

	g_nmi->_aniMan->setStatic(ST_MAN_STANDLADDER);
	if (!mq->run(g_nmi->_aniMan))
		delete mq;

	g_vars->scene32_dudeOnLadder = false;

	getCurrSceneSc2MotionController()->activate();
	getGameLoaderInteractionController()->enable();
}

void sceneHandler32_potLogic(Command *cmd) {
	if (g_vars->scene32_cactusCounter < 0 || g_vars->scene32_cactusCounter > 20) {
		MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

		Command *ex = new Command(ANI_MAN, 1, MV_MAN32_STANDUP, 0, 0, 0, 1, 0, 0, 0);

		ex->_excFlags |= 2;

		mq->addCommandToEnd(ex);

		AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

		if (!ani || ani->_id != ANI_KADKA)
			mq->addCommandToEnd(cmd->createClone());

		mq->setFlags(mq->getFlags() | 1);
		mq->run(nullptr);

		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enable();

		g_vars->scene32_dudeIsSitting = false;
	}
}

int sceneHandler32(Command *cmd) {
	if (cmd->_cmd != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_SC32_TRUBATOBACK:
		g_nmi->_currentScene->findPictureObjectById(PIC_SC32_RTRUBA, 0)->_z = 20;
		break;

	case MSG_SC32_TRUBATOFRONT:
		g_nmi->_currentScene->findPictureObjectById(PIC_SC32_RTRUBA, 0)->_z = 0;
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
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
		g_nmi->lift_goAnimation();
		break;

	case MSG_SC32_ONLADDER:
		g_vars->scene32_dudeOnLadder = true;

		getCurrSceneSc2MotionController()->deactivate();
		getGameLoaderInteractionController()->disable();
		break;

	case MSG_SC6_BTNPUSH:
		sceneHandler32_buttonPush();
		break;

	case 64:
		g_nmi->lift_hoverButton(cmd);
		break;

	case MSG_SC6_INSTHANDLE:
		sceneHandler32_installHandle();
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_x;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();
		}

		if (!g_vars->scene32_flag->_movement) {
			if (g_vars->scene32_flagIsWaving) {
				if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_RIGHT)
					g_vars->scene32_flag->startMovement(MV_FLG_CYCLER, 0, -1);
				else if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_LEFT)
					g_vars->scene32_flag->startMovement(MV_FLG_CYCLEL, 0, -1);
			}

			if (g_vars->scene32_flagNeedsStopping && !g_vars->scene32_flagIsWaving) {
				if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_RIGHT)
					g_vars->scene32_flag->startMovement(MV_FLG_STOPR, 0, -1);
				else if (g_vars->scene32_flag->_statics->_staticsId == ST_FLG_LEFT)
					g_vars->scene32_flag->startMovement(MV_FLG_STOPL, 0, -1);

				g_vars->scene32_flagNeedsStopping = false;
			}
		}

		if (g_vars->scene32_cactusCounter) {
			if (g_vars->scene32_cactusCounter > 0)
				--g_vars->scene32_cactusCounter;

			g_nmi->_behaviorManager->update();

			g_nmi->startSceneTrack();
		} else {
			sceneHandler32_animateCactus();

			g_nmi->_behaviorManager->update();

			g_nmi->startSceneTrack();

		}
		break;

	case 29:
		if (g_vars->scene32_dudeOnLadder) {
			sceneHandler32_ladderLogic(cmd);
			cmd->_cmd = 0;
			break;
		}

		if (!g_vars->scene32_dudeIsSitting || g_nmi->_aniMan->_movement) {
			AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);

				cmd->_cmd = 0;
				break;
			}

			if (g_nmi->_cursorId == PIC_CSR_GOFAR_R || g_nmi->_cursorId == PIC_CSR_GOFAR_L) {
				if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
					|| (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0))
					g_nmi->processArcade(cmd);
			}
			break;
		}

		if (!g_vars->scene32_cactusIsGrowing)
			sceneHandler32_potLogic(cmd);

		cmd->_cmd = 0;

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
