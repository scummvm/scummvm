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

void sceneHandler34_setExits() {
	int state;

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_NearPipeWithStool)) {
		if (g_fp->getObjectState(sO_Hatch_34) == g_fp->getObjectEnumState(sO_Hatch_34, sO_Closed))
			state = g_fp->getObjectEnumState(sO_Plank_34, sO_ClosedWithBoot);
		else
			state = g_fp->getObjectEnumState(sO_Plank_34, sO_OpenedWithBoot);
	} else {
		if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_OnStool)) {
			if (g_fp->getObjectState(sO_Hatch_34) == g_fp->getObjectEnumState(sO_Hatch_34, sO_Closed))
				state = g_fp->getObjectEnumState(sO_Plank_34, sO_IsClosed);
			else
				state = g_fp->getObjectEnumState(sO_Plank_34, sO_IsOpened);
		} else {
			state = g_fp->getObjectEnumState(sO_Plank_34, sO_Passive);
		}
	}

	g_fp->setObjectState(sO_Plank_34, state);
}

void scene34_initScene(Scene *sc) {
	g_vars->scene34_cactus = sc->getStaticANIObject1ById(ANI_CACTUS_34, -1);
	g_vars->scene34_vent = sc->getStaticANIObject1ById(ANI_VENT_34, -1);
	g_vars->scene34_hatch = sc->getStaticANIObject1ById(ANI_LUK_34, -1);
	g_vars->scene34_boot = sc->getStaticANIObject1ById(ANI_BOOT_34, -1);

	if (g_fp->getObjectState(sO_Cactus) == g_fp->getObjectEnumState(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_fp->_currentScene;

		g_fp->_currentScene = sc;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_EMPTY);
		g_vars->scene34_cactus->setOXY(506, 674);
		g_vars->scene34_cactus->_priority = 30;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_GROWNEMPTY2);
		g_vars->scene34_cactus->_flags |= 4;

		g_fp->_currentScene = oldsc;
	}

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_Strolling))
		g_fp->setObjectState(sO_Grandma, g_fp->getObjectEnumState(sO_Grandma, sO_OnStool));

	sceneHandler34_setExits();

	g_vars->scene34_dudeClimbed = false;
	g_vars->scene34_dudeOnBoard = false;
	g_vars->scene34_dudeOnCactus = false;
	g_vars->scene34_fliesCountdown = g_fp->_rnd->getRandomNumber(500) + 500;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_34"));

	g_fp->lift_setButton(sO_Level7, ST_LBN_7N);
	g_fp->lift_init(sc, QU_SC34_ENTERLIFT, QU_SC34_EXITLIFT);

	g_fp->initArcadeKeys("SC_34");
}

void scene34_initBeh() {
	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLEFT, 0);
	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLRIGHT, 0);
}

int scene34_updateCursor() {
	g_fp->updateCursorCommon();

	if ((g_fp->_objectIdAtCursor != ANI_STOOL_34 || getGameLoaderInventory()->getSelectedItemId() != ANI_INV_BOX)
		 && (g_fp->_objectIdAtCursor != ANI_BOX_34 || getGameLoaderInventory()->getSelectedItemId() != ANI_INV_STOOL))
		; // emtpy
	else
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

void sceneHandler34_leaveBoard() {
	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);

	g_vars->scene34_dudeOnBoard = false;
}

void sceneHandler34_onBoard() {
	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	g_vars->scene34_dudeOnBoard = true;
}

void sceneHandler34_testVent() {
	if (g_fp->_aniMan->_movement->_id == MV_MAN34_TURNVENT_R) {
		g_vars->scene34_hatch->changeStatics2(ST_LUK34_CLOSED);

		chainQueue(QU_LUK34_OPEN, 0);
	} else if (g_fp->_aniMan->_movement->_id == MV_MAN34_TURNVENT_L) {
		g_vars->scene34_hatch->changeStatics2(ST_LUK34_OPEN);

		chainQueue(QU_LUK34_CLOSE, 0);
	}
}

void sceneHandler34_hideStool() {
	g_fp->_currentScene->getStaticANIObject1ById(ANI_STOOL_34, -1)->hide();
}

void sceneHandler34_climb() {
	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	g_vars->scene34_dudeClimbed = true;
}

void sceneHandler34_genFlies() {
	g_fp->_floaters->genFlies(g_fp->_currentScene, 1072, -50, 100, 4);

	g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->countdown = 1;
	g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->val6 = 1072;
	g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->val7 = -50;

	g_vars->scene34_fliesCountdown = g_fp->_rnd->getRandomNumber(500) + 500;
}

void sceneHandler34_fromCactus(ExCommand *cmd) {
	if (g_fp->_aniMan->_movement || g_vars->scene34_cactus->_movement || (g_fp->_aniMan->_flags & 0x100)) {
		cmd->_messageKind = 0;

		return;
	}

	MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC34_FROMCACTUS), 0, 0);

	ExCommand *ex = new ExCommand(ANI_MAN, 34, 256, 0, 0, 0, 1, 0, 0, 0);

	ex->_messageNum = 0;
	ex->_excFlags |= 3;
	ex->_field_14 = 256;
	mq->addExCommandToEnd(ex);

	ex = cmd->createClone();
	mq->addExCommandToEnd(ex);

	mq->setFlags(mq->getFlags() | 1);
	mq->chain(0);

	g_fp->_aniMan->_flags |= 1;
}

void sceneHandler34_animateLeaveBoard(ExCommand *cmd) {
	if (!g_fp->_aniMan->_movement) {
		MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC34_LEAVEBOARD), 0, 0);

		mq->addExCommandToEnd(cmd->createClone());
		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);
	}

	cmd->_messageKind = 0;
}

void sceneHandler34_animateAction(ExCommand *cmd) {
	if (g_fp->_aniMan->_movement)
		return;

	int ox = g_fp->_aniMan->_ox;
	int oy = g_fp->_aniMan->_oy;
	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

	if (!ani || ani->_id != ANI_VENT_34) {
		int qId = 0;

		if (ox == 887) {
			if (oy != 370)
				return;

			qId = QU_SC34_FROMSTOOL;
		} else {
			if (ox != 916)
				return;

			if (oy == 286) {
				MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC34_FROMBOX), 0, 0);

				mq->addExCommandToEnd(cmd->createClone());
				mq->chain(0);

				sceneHandler34_setExits();

				return;
			}

			if (oy != 345)
				return;

			qId = QU_SC34_FROMBOX_FLOOR;
		}

		if (qId) {
			MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(qId), 0, 0);

			mq->addExCommandToEnd(cmd->createClone());
			mq->chain(0);
		}

		return;
	}

	if (ox == 887) {
		if (oy == 370)
			g_fp->_aniMan->startAnim(MV_MAN34_TRYTABUR, 0, -1);

	} else if (ox == 916) {
		if (oy == 286) {
			int id = g_vars->scene34_vent->_statics->_staticsId;
			if (id == ST_VNT34_UP2) {
				g_fp->_aniMan->startAnim(MV_MAN34_TURNVENT_R, 0, -1);
			} else if (id == ST_VNT34_RIGHT3) {
				g_fp->_aniMan->startAnim(MV_MAN34_TURNVENT_L, 0, -1);
			}
		} else if (oy == 345) {
			g_fp->_aniMan->startAnim(MV_MAN34_TRY, 0, -1);
		}
	}
}

void sceneHandler34_showVent() {
	if (g_vars->scene34_vent->_statics->_staticsId == ST_VNT34_UP2)
		g_vars->scene34_vent->changeStatics2(ST_VNT34_RIGHT3);
	else if (g_vars->scene34_vent->_statics->_staticsId == ST_VNT34_RIGHT3)
		g_vars->scene34_vent->changeStatics2(ST_VNT34_UP2);

	g_vars->scene34_vent->show1(-1, -1, -1, 0);
}

void sceneHandler34_showBox() {
	g_fp->_currentScene->getStaticANIObject1ById(ANI_STOOL_34, -1)->changeStatics2(ST_STL34_BOX2);
}

void sceneHandler34_showStool() {
	chainQueue(QU_SC34_SHOWSTOOL, 0);
}

void sceneHandler34_unclimb() {
	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);

	g_vars->scene34_dudeClimbed = false;
}

int sceneHandler34(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC4_HIDEBOOT:
		g_vars->scene34_boot->_flags &= 0xFFFB;
		break;

	case MSG_SC34_LEAVEBOARD:
		sceneHandler34_leaveBoard();
		break;

	case MSG_SC34_ONBOARD:
		sceneHandler34_onBoard();
		break;

	case MSG_SC34_TESTVENT:
		sceneHandler34_testVent();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_clickButton();
		break;

	case MSG_SC34_FROMCACTUS:
		g_vars->scene34_dudeOnCactus = false;

		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();

		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);

		break;

	case MSG_SC34_RETRYVENT:
		if (!g_fp->_aniMan->isIdle())
			break;

		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);
		g_fp->_aniMan->_flags &= 0xFEFF;

		getGameLoaderInteractionController()->handleInteraction(g_fp->_aniMan, g_vars->scene34_vent, cmd->_keyCode);

		break;

	case MSG_SC34_ONBUMP:
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLEFT, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLRIGHT, 1);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC22_HIDESTOOL:
		sceneHandler34_hideStool();
		break;

	case MSG_SC34_CLIMB:
		sceneHandler34_climb();
		break;

	case MSG_SC34_UNCLIMB:
		sceneHandler34_unclimb();
		break;

	case MSG_SC22_SHOWSTOOL:
		sceneHandler34_showStool();
		break;

	case MSG_SC34_SHOWBOX:
		sceneHandler34_showBox();
		break;

	case MSG_SC34_ONCACTUS:
		g_vars->scene34_dudeOnCactus = true;

		getCurrSceneSc2MotionController()->clearEnabled();
		getGameLoaderInteractionController()->disableFlag24();

		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);
		break;

	case MSG_SC34_SHOWVENT:
		sceneHandler34_showVent();
		break;

	case 64:
		g_fp->lift_hoverButton(cmd);
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case 29:
		{
			if (g_vars->scene34_dudeClimbed) {
				sceneHandler34_animateAction(cmd);
				break;
			}

			if (g_vars->scene34_dudeOnBoard) {
				sceneHandler34_animateLeaveBoard(cmd);
				break;
			}

			if (g_vars->scene34_dudeOnCactus) {
				sceneHandler34_fromCactus(cmd);
				break;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (ani) {
				if ((ani->_id == ANI_STOOL_34 && cmd->_keyCode == ANI_INV_BOX) || (ani->_id == ANI_BOX_34 && cmd->_keyCode == ANI_INV_STOOL)) {
					getGameLoaderInteractionController()->handleInteraction(g_fp->_aniMan, g_vars->scene34_vent, cmd->_keyCode);

					cmd->_messageKind = 0;
				}

				if (ani->_id == ANI_LIFTBUTTON) {
					g_fp->lift_animateButton(ani);

					cmd->_messageKind = 0;

					break;
				}
			}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1) || (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						break;
					}
				}
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		--g_vars->scene34_fliesCountdown;

		if (!g_vars->scene34_fliesCountdown)
			sceneHandler34_genFlies();

		g_fp->_floaters->update();

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
