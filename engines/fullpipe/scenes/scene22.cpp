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

void scene22_initScene(Scene *sc) {
	g_vars->scene22_bag = sc->getStaticANIObject1ById(ANI_MESHOK, -1);

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	g_vars->scene22_giraffeMiddle = sc->getStaticANIObject1ById(ANI_GIRAFFE_MIDDLE, -1);
	g_vars->scene22_dudeIsOnStool = false;
	g_vars->scene22_interactionIsDisabled = false;
	g_vars->scene22_craneIsOut = true;

	if (g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_NotFallen))
		g_vars->scene22_numBagFalls = 0;
	else if (g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_FallenOnce))
		g_vars->scene22_numBagFalls = 1;
	else if ( g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_FallenTwice))
		g_vars->scene22_numBagFalls = 2;
	else {
		g_vars->scene22_numBagFalls = 3;
        g_vars->scene22_craneIsOut = false;
	}


	if ( g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened))
		g_vars->scene22_giraffeMiddle->changeStatics2(ST_GRFM_AFTER);
	else
		g_vars->scene22_giraffeMiddle->changeStatics2(ST_GRFM_NORM);

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_22");
}

int scene22_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor != ANI_HANDLE_L)
		return g_fp->_cursorId;

	int sel = g_fp->_inventory->getSelectedItemId();

	if (!sel) {
		g_fp->_cursorId = PIC_CSR_ITN;
		return g_fp->_cursorId;
	}

	if (g_vars->scene22_dudeIsOnStool || (sel != ANI_INV_STOOL && sel != ANI_INV_BOX))
		; //empty
	else
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

void scene22_setBagState() {
	if (g_vars->scene22_craneIsOut) {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 0);
	} else {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 1);
	}
}

void sceneHandler22_showStool() {
	chainQueue(QU_SC22_SHOWSTOOL, 0);
}

void sceneHandler22_hideStool() {
	g_fp->_currentScene->getStaticANIObject1ById(ANI_TABURETTE, -1)->hide();
}

void sceneHandler22_handleDown() {
	if (g_vars->scene22_bag->_statics->_staticsId == ST_MSH_SIT) {
		chainQueue(QU_MSH_CRANEOUT, 1);
		g_vars->scene22_interactionIsDisabled = false;
	} else {
		++g_vars->scene22_numBagFalls;

		int qid;

		if (g_vars->scene22_numBagFalls == 3) {
			chainQueue(QU_SC22_FALLSACK_GMA, 1);
			qid = QU_SC22_FALLBROOM;
		} else {
			qid = QU_SC22_FALLSACK;
		}

		chainQueue(qid, 1);

		int state;

		if (g_vars->scene22_numBagFalls) {
			if (g_vars->scene22_numBagFalls == 1) {
				state = g_fp->getObjectEnumState(sO_Bag_22, sO_FallenOnce);
			} else if (g_vars->scene22_numBagFalls == 2) {
				state = g_fp->getObjectEnumState(sO_Bag_22, sO_FallenTwice);
			} else {
				state = g_fp->getObjectEnumState(sO_Bag_22, sO_BrushHasFallen);
			}
		} else {
			state = g_fp->getObjectEnumState(sO_Bag_22, sO_NotFallen);
		}

		g_fp->setObjectState(sO_Bag_22, state);
	}

	g_vars->scene22_craneIsOut = true;

	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 1);
	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 0);
}

void sceneHandler22_fromStool(ExCommand *cmd) {
	if (g_fp->_aniMan->isIdle() && !(g_fp->_aniMan->_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC22_FROMSTOOL), 0, 0);

		mq->addExCommandToEnd(cmd->createClone());
		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);
	}
}

void sceneHandler22_stoolLogic(ExCommand *cmd) {
	StaticANIObject *ani;
	MessageQueue *mq;
	int xpos;
	int manId;

	if (g_fp->_aniMan->isIdle() && !(g_fp->_aniMan->_flags & 0x100)) {
		if (cmd->_keyCode == ANI_INV_STOOL) {
			if (abs(841 - g_fp->_aniMan->_ox) <= 1) {
				if (abs(449 - g_fp->_aniMan->_oy) <= 1) {
					chainQueue(QU_SC22_PUTSTOOL, 1);
					g_vars->scene22_interactionIsDisabled = true;

					return;
				}
			}
			goto LABEL_13;
		}

		if (cmd->_keyCode == ANI_INV_BOX) {
			ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_TABURETTE, -1);
			if (!ani || !(ani->_flags & 4)) {
				if (abs(841 - g_fp->_aniMan->_ox) <= 1) {
					if (abs(449 - g_fp->_aniMan->_oy) <= 1) {
						chainObjQueue(g_fp->_aniMan, QU_SC22_TRYBOX, 1);
						return;
					}
				}
			LABEL_13:
				xpos = 841;
				manId = ST_MAN_RIGHT;
			LABEL_31:
				mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, xpos, 449, 1, manId);

				if (!mq)
					return;

				mq->addExCommandToEnd(cmd->createClone());

				postExCommand(g_fp->_aniMan->_id, 2, 841, 449, 0, -1);
				return;
			}
		} else {
			if (cmd->_keyCode)
				return;

			if (g_vars->scene22_dudeIsOnStool) {
				if (g_fp->_aniMan->_movement)
					return;

				chainQueue(QU_SC22_HANDLEDOWN, 1);

				g_vars->scene22_interactionIsDisabled = true;
				return;
			}

			ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_TABURETTE, -1);
			if (ani && (ani->_flags & 4)) {
				int x = g_fp->_aniMan->_ox;
				int y = g_fp->_aniMan->_ox;

				if (sqrt((double)((841 - x) * (841 - x) + (449 - y) * (449 - y)))
					< sqrt((double)((1075 - x) * (1075 - x) + (449 - y) * (449 - y)))) {
					if (abs(841 - x) <= 1) {
						if (abs(449 - y) <= 1) {
							chainQueue(QU_SC22_TOSTOOL, 1);

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

				MGM mgm;
				MGMInfo mgminfo;

				mgm.addItem(ANI_MAN);
				mgminfo.ani = g_fp->_aniMan;
				mgminfo.staticsId2 = ST_MAN_RIGHT;
				mgminfo.x1 = 934;
				mgminfo.y1 = 391;
				mgminfo.field_1C = 10;
				mgminfo.staticsId1 = 0x4145;
				mgminfo.x2 = 981;
				mgminfo.y2 = 390;
				mgminfo.field_10 = 1;
				mgminfo.flags = 127;
				mgminfo.movementId = rMV_MAN_TURN_SRL;

				mq = mgm.genMovement(&mgminfo);

				ExCommand *ex = mq->getExCommandByIndex(0);

				mq->deleteExCommandByIndex(0, 0);

				delete mq;

				mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC22_TOSTOOL_R), 0, 0);

				mq->insertExCommandAt(2, ex);
				mq->setFlags(mq->getFlags() | 1);
				mq->chain(0);

				g_vars->scene22_interactionIsDisabled = true;
			} else {
				if (abs(1010 - g_fp->_aniMan->_ox) <= 1) {
					if (abs(443 - g_fp->_aniMan->_oy) <= 1) {
						chainQueue(QU_SC22_TRYHANDLE, 1);
						return;
					}
				}

				mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, 1010, 443, 1, ST_MAN_UP);

				if (mq) {
					mq->addExCommandToEnd(cmd->createClone());

					postExCommand(g_fp->_aniMan->_id, 2, 1010, 443, 0, -1);
					return;
				}
			}
		}
	}
}

int sceneHandler22(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC22_CRANEOUT_GMA:
		chainQueue(QU_MSH_CRANEOUT_GMA, 1);
		break;

	case MSG_SC22_CHECKGMABOOT:
		if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_15))
			g_fp->setObjectState(sO_Boot_15, g_fp->getObjectEnumState(sO_Boot_15, sO_IsPresent));

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

		getCurrSceneSc2MotionController()->setEnabled();
		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);
		break;

	case MSG_SC22_ONSTOOL:
		g_vars->scene22_dudeIsOnStool = true;
		getCurrSceneSc2MotionController()->clearEnabled();
		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);
		break;

	case MSG_SC22_HANDLEDOWN:
		sceneHandler22_handleDown();
		break;

	case 29:
		if (!g_vars->scene22_interactionIsDisabled) {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_HANDLE_L) {
				sceneHandler22_stoolLogic(cmd);
				return 0;
			}

			if (!g_vars->scene22_dudeIsOnStool) {
				if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
					int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
					PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

					if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
						if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
							|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
							g_fp->processArcade(cmd);
							return 0;
						}
					}
				}
				return 0;
			}

			if (g_fp->_aniMan->_statics->_staticsId == ST_MAN_RIGHT && !g_fp->_aniMan->_movement) {
				sceneHandler22_fromStool(cmd);

				return 0;
			}
		}

		cmd->_messageKind = 0;
		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x <= g_fp->_sceneWidth - 460) {
				if (x < g_fp->_sceneRect.left + 200)
					g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;
			} else {
				g_fp->_currentScene->_x = g_fp->_sceneWidth - x;
			}

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();
		}

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
