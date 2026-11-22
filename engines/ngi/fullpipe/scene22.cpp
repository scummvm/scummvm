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

void scene22_initScene(Scene *sc) {
	g_vars->scene22_bag = sc->findAniObjectById(ANI_MESHOK, -1);

	Scene *oldsc = g_nmi->_currentScene;
	g_nmi->_currentScene = sc;

	g_vars->scene22_giraffeMiddle = sc->findAniObjectById(ANI_GIRAFFE_MIDDLE, -1);
	g_vars->scene22_dudeIsOnStool = false;
	g_vars->scene22_interactionIsDisabled = false;
	g_vars->scene22_craneIsOut = true;

	if (g_nmi->getStateId(sO_Bag_22) == g_nmi->getStateValue(sO_Bag_22, sO_NotFallen))
		g_vars->scene22_numBagFalls = 0;
	else if (g_nmi->getStateId(sO_Bag_22) == g_nmi->getStateValue(sO_Bag_22, sO_FallenOnce))
		g_vars->scene22_numBagFalls = 1;
	else if ( g_nmi->getStateId(sO_Bag_22) == g_nmi->getStateValue(sO_Bag_22, sO_FallenTwice))
		g_vars->scene22_numBagFalls = 2;
	else {
		g_vars->scene22_numBagFalls = 3;
		g_vars->scene22_craneIsOut = false;
	}


	if ( g_nmi->getStateId(sO_LowerPipe_21) == g_nmi->getStateValue(sO_LowerPipe_21, sO_IsOpened))
		g_vars->scene22_giraffeMiddle->setStatic(ST_GRFM_AFTER);
	else
		g_vars->scene22_giraffeMiddle->setStatic(ST_GRFM_NORM);

	g_nmi->_currentScene = oldsc;

	g_nmi->initArcadeKeys("SC_22");
}

int scene22_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor != ANI_HANDLE_L)
		return g_nmi->_cursorId;

	int sel = g_nmi->_inventory->getSelectedItemId();

	if (!sel) {
		g_nmi->_cursorId = PIC_CSR_ITN;
		return g_nmi->_cursorId;
	}

	if (g_vars->scene22_dudeIsOnStool || (sel != ANI_INV_STOOL && sel != ANI_INV_BOX))
		; //empty
	else
		g_nmi->_cursorId = PIC_CSR_ITN_INV;

	return g_nmi->_cursorId;
}

void scene22_setBagState() {
	if (g_vars->scene22_craneIsOut) {
		g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 1);
		g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 0);
	} else {
		g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 0);
		g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 1);
	}
}

void sceneHandler22_showStool() {
	runQueue(QU_SC22_SHOWSTOOL, 0);
}

void sceneHandler22_hideStool() {
	g_nmi->_currentScene->findAniObjectById(ANI_TABURETTE, -1)->hide();
}

void sceneHandler22_onHandleDown() {
	if (g_vars->scene22_bag->_statics->_staticsId == ST_MSH_SIT) {
		runQueue(QU_MSH_CRANEOUT, 1);
		g_vars->scene22_interactionIsDisabled = false;
	} else {
		++g_vars->scene22_numBagFalls;

		int qid;

		if (g_vars->scene22_numBagFalls == 3) {
			runQueue(QU_SC22_FALLSACK_GMA, 1);
			qid = QU_SC22_FALLBROOM;
		} else {
			qid = QU_SC22_FALLSACK;
		}

		runQueue(qid, 1);

		int state;

		if (g_vars->scene22_numBagFalls) {
			if (g_vars->scene22_numBagFalls == 1) {
				state = g_nmi->getStateValue(sO_Bag_22, sO_FallenOnce);
			} else if (g_vars->scene22_numBagFalls == 2) {
				state = g_nmi->getStateValue(sO_Bag_22, sO_FallenTwice);
			} else {
				state = g_nmi->getStateValue(sO_Bag_22, sO_BrushHasFallen);
			}
		} else {
			state = g_nmi->getStateValue(sO_Bag_22, sO_NotFallen);
		}

		g_nmi->setState(sO_Bag_22, state);
	}

	g_vars->scene22_craneIsOut = true;

	g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 1);
	g_nmi->_behaviorManager->enableAni(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 0);
}

void sceneHandler22_fromStool(Command *cmd) {
	if (g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC22_FROMSTOOL), 0, 0);

		mq->addCommandToEnd(cmd->createClone());
		mq->setFlags(mq->getFlags() | 1);
		mq->run(nullptr);
	}
}

void sceneHandler22_stoolLogic(Command *cmd) {
	AniObject *ani;
	MessageQueue *mq;
	int xpos;
	int manId;

	if (g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
		if (cmd->_param == ANI_INV_STOOL) {
			if (abs(841 - g_nmi->_aniMan->_x) <= 1) {
				if (abs(449 - g_nmi->_aniMan->_y) <= 1) {
					runQueue(QU_SC22_PUTSTOOL, 1);
					g_vars->scene22_interactionIsDisabled = true;

					return;
				}
			}
			goto LABEL_13;
		}

		if (cmd->_param == ANI_INV_BOX) {
			ani = g_nmi->_currentScene->findAniObjectById(ANI_TABURETTE, -1);
			if (!ani || !(ani->_flags & 4)) {
				if (abs(841 - g_nmi->_aniMan->_x) <= 1) {
					if (abs(449 - g_nmi->_aniMan->_y) <= 1) {
						runQueueByIdWithAni(g_nmi->_aniMan, QU_SC22_TRYBOX, 1);
						return;
					}
				}
			LABEL_13:
				xpos = 841;
				manId = ST_MAN_RIGHT;
			LABEL_31:
				mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, xpos, 449, 1, manId);

				if (!mq)
					return;

				mq->addCommandToEnd(cmd->createClone());

				postExCommand(g_nmi->_aniMan->_id, 2, 841, 449, 0, -1);
				return;
			}
		} else {
			if (cmd->_param)
				return;

			if (g_vars->scene22_dudeIsOnStool) {
				if (g_nmi->_aniMan->_movement)
					return;

				runQueue(QU_SC22_HANDLEDOWN, 1);

				g_vars->scene22_interactionIsDisabled = true;
				return;
			}

			ani = g_nmi->_currentScene->findAniObjectById(ANI_TABURETTE, -1);
			if (ani && (ani->_flags & 4)) {
				int x = g_nmi->_aniMan->_x;
				int y = g_nmi->_aniMan->_y;

				if (sqrt((double)((841 - x) * (841 - x) + (449 - y) * (449 - y)))
					< sqrt((double)((1075 - x) * (1075 - x) + (449 - y) * (449 - y)))) {
					if (abs(841 - x) <= 1) {
						if (abs(449 - y) <= 1) {
							runQueue(QU_SC22_TOSTOOL, 1);

							g_vars->scene22_interactionIsDisabled = true;
							return;
						}
					}
					goto LABEL_13;
				}

				if (abs(1075 - x) > 1 || abs(449 - y) > 1) {
					xpos = 1075;
					manId = ST_MAN_RIGHT | 0x4000;
					goto LABEL_31;
				}

				AniHandler mgm;
				MakeQueueStruct mkQueue;

				mgm.attachObject(ANI_MAN);
				mkQueue.ani = g_nmi->_aniMan;
				mkQueue.staticsId2 = ST_MAN_RIGHT;
				mkQueue.x1 = 934;
				mkQueue.y1 = 391;
				mkQueue.destZ = 10;
				mkQueue.staticsId1 = 0x4145;
				mkQueue.x2 = 981;
				mkQueue.y2 = 390;
				mkQueue.cycleCount = 1;
				mkQueue.flags = 127;
				mkQueue.movementId = rMV_MAN_TURN_SRL;

				mq = mgm.makeRunQueue(&mkQueue);

				Command *ex = mq->getCommandByIndex(0);

				mq->removeCommand(0, 0);

				delete mq;

				mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC22_TOSTOOL_R), 0, 0);

				mq->addCommand(2, ex);
				mq->setFlags(mq->getFlags() | 1);
				mq->run(nullptr);

				g_vars->scene22_interactionIsDisabled = true;
			} else {
				if (abs(1010 - g_nmi->_aniMan->_x) <= 1) {
					if (abs(443 - g_nmi->_aniMan->_y) <= 1) {
						runQueue(QU_SC22_TRYHANDLE, 1);
						return;
					}
				}

				mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 1010, 443, 1, ST_MAN_UP);

				if (mq) {
					mq->addCommandToEnd(cmd->createClone());

					postExCommand(g_nmi->_aniMan->_id, 2, 1010, 443, 0, -1);
					return;
				}
			}
		}
	}
}

int sceneHandler22(Command *cmd) {
	if (cmd->_cmd != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC22_CRANEOUT_GMA:
		runQueue(QU_MSH_CRANEOUT_GMA, 1);
		break;

	case MSG_SC22_CHECKGMABOOT:
		if (g_nmi->getStateId(sO_Grandma) == g_nmi->getStateValue(sO_Grandma, sO_In_15)) {
			g_nmi->setState(sO_Boot_15, g_nmi->getStateValue(sO_Boot_15, sO_IsPresent));
		}
		break;

	case MSG_SC22_SHOWSTOOL:
		sceneHandler22_showStool();
		break;

	case MSG_SC22_HIDESTOOL:
		sceneHandler22_hideStool();
		break;

	case MSG_SC22_FROMSTOOL:
		g_vars->scene22_dudeIsOnStool = false;
		g_vars->scene22_interactionIsDisabled = false;

		getCurrSceneSc2MotionController()->activate();
		g_nmi->_behaviorManager->enableAllQueuesForAni(g_nmi->_aniMan, 1);
		break;

	case MSG_SC22_ONSTOOL:
		g_vars->scene22_dudeIsOnStool = true;
		getCurrSceneSc2MotionController()->deactivate();
		g_nmi->_behaviorManager->enableAllQueuesForAni(g_nmi->_aniMan, 0);
		break;

	case MSG_SC22_HANDLEDOWN:
		sceneHandler22_onHandleDown();
		break;

	case 29:
		if (!g_vars->scene22_interactionIsDisabled) {
			AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_HANDLE_L) {
				sceneHandler22_stoolLogic(cmd);
				return 0;
			}

			if (!g_vars->scene22_dudeIsOnStool) {
				if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
					int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
					PictureObject *pic = g_nmi->_currentScene->findPictureObjectById(picId, 0);

					if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
						if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
							|| (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0)) {
							g_nmi->processArcade(cmd);
							return 0;
						}
					}
				}
				return 0;
			}

			if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_RIGHT && !g_nmi->_aniMan->_movement) {
				sceneHandler22_fromStool(cmd);

				return 0;
			}
		}

		cmd->_cmd = 0;
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_x;

			if (x <= g_nmi->_sceneWidth - 460) {
				if (x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;
			} else {
				g_nmi->_currentScene->_x = g_nmi->_sceneWidth - x;
			}

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();

			g_nmi->_behaviorManager->update();

			g_nmi->startSceneTrack();
		}

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
