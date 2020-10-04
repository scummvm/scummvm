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

#include "ngi/objectnames.h"
#include "ngi/constants.h"

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "ngi/interaction.h"
#include "ngi/behavior.h"

namespace NGI {

void scene25_showBoardOnRightFar() {
	g_vars->scene25_board->show1(453, 557, MV_BRD25_RIGHT, 0);
	g_vars->scene25_board->_priority = 28;
}

void scene25_showBoardOnRightClose() {
	g_vars->scene25_board->show1(632, 557, rMV_BRD25_RIGHT, 0);
	g_vars->scene25_board->_priority = 28;
}

void scene25_initScene(Scene *sc, int entranceId) {
	g_vars->scene25_water = sc->getStaticANIObject1ById(ANI_WATER25, -1);
	g_vars->scene25_board = sc->getStaticANIObject1ById(ANI_BOARD25, -1);
	g_vars->scene25_drop = sc->getStaticANIObject1ById(ANI_DROP_25, -1);
	g_vars->scene25_water->setAlpha(0xa0);
	g_vars->scene25_drop->setAlpha(0xa0);
	g_vars->scene25_dudeIsOnBoard = false;

	if (g_nmi->getObjectState(sO_Pool) < g_nmi->getObjectEnumState(sO_Pool, sO_HalfFull)) {
		g_vars->scene25_waterIsPresent = false;

		g_vars->scene25_water->hide();
	} else {
		g_vars->scene25_waterIsPresent = true;

		g_nmi->playSound(SND_25_006, 1);
	}

	int boardState = g_nmi->getObjectState(sO_Board_25);

	if (entranceId == TrubaRight) {
		if (boardState == g_nmi->getObjectEnumState(sO_Board_25, sO_FarAway)) {
			scene25_showBoardOnRightFar();

			g_nmi->playSound(SND_25_029, 0);

			g_vars->scene25_boardIsSelectable = false;
		} else {
			if (boardState == g_nmi->getObjectEnumState(sO_Board_25, sO_Nearby)
				|| boardState == g_nmi->getObjectEnumState(sO_Board_25, sO_WithDudeOnRight))
				scene25_showBoardOnRightClose();
			g_vars->scene25_boardIsSelectable = false;
		}
	} else {
		if (boardState == g_nmi->getObjectEnumState(sO_Board_25, sO_WithDudeOnLeft)) {
			if (!getGameLoaderInventory()->getCountItemsWithId(ANI_INV_BOARD)) {
				getGameLoaderInventory()->addItem(ANI_INV_BOARD, 1);
				getGameLoaderInventory()->rebuildItemRects();
			}
		} else {
			g_vars->scene25_boardIsSelectable = true;
		}
	}

	g_vars->scene25_beardersAreThere = false;
	g_vars->scene25_beardersCounter = 0;
}

int scene25_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_vars->scene25_waterIsPresent) {
		int inv = getGameLoaderInventory()->getSelectedItemId();

		if (g_nmi->_objectIdAtCursor == ANI_WATER25) {
			if ((g_vars->scene25_boardIsSelectable && (!inv || inv == ANI_INV_BOARD)) || (g_vars->scene25_dudeIsOnBoard && (inv == ANI_INV_LOPAT || !inv)))
				g_nmi->_cursorId = (g_nmi->_cursorId != PIC_CSR_DEFAULT) ? PIC_CSR_ITN_INV : PIC_CSR_ITN;
		} else if (g_nmi->_objectIdAtCursor == ANI_BOARD25 && (!inv || inv == ANI_INV_SWAB || inv == ANI_INV_BROOM || inv == ANI_INV_LOPAT)) {
			g_nmi->_cursorId = (g_nmi->_cursorId != PIC_CSR_DEFAULT) ? PIC_CSR_ITN_INV : PIC_CSR_ITN;
		}
	}

	return g_nmi->_cursorId;
}

void scene25_setupWater(Scene *a1, int entranceId) {
	if (g_vars->scene25_waterIsPresent) {
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene25_drop, ST_DRP25_EMPTY, QU_DRP25_TOFLOOR, 0);
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene25_drop, ST_DRP25_EMPTY, QU_DRP25_TOWATER, 1);

		if (entranceId != TrubaRight)
			g_nmi->playTrack(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_25"), "MUSIC2", 0);
	} else {
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene25_drop, ST_DRP25_EMPTY, QU_DRP25_TOFLOOR, 1);
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene25_drop, ST_DRP25_EMPTY, QU_DRP25_TOWATER, 0);
	}
}

void sceneHandler25_stopBearders() {
	g_vars->scene25_beardersAreThere = false;

	g_vars->scene25_bearders.clear();
}

void sceneHandler25_startBearders() {
	g_vars->scene25_bearders.clear();
	g_vars->scene25_beardersCounter = 0;

	StaticANIObject *bearded = g_nmi->accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_BEARDED_CMN, -1);

	for (int i = 0; i < 3; i++) {
		StaticANIObject *ani = new StaticANIObject(bearded);

		g_vars->scene25_bearders.push_back(ani);

		ani->_statics = ani->getStaticsById(ST_BRDCMN_EMPTY);

		g_nmi->_currentScene->addStaticANIObject(ani, 1);
	}

	g_vars->scene25_beardersAreThere = true;
}

void sceneHandler25_enterMan() {
	if (g_vars->scene25_waterIsPresent) {
		chainQueue(QU_SC25_ENTERUP_WATER, 1);

		getCurrSceneSc2MotionController()->deactivate();
	} else {
		chainQueue(QU_SC25_ENTERUP_FLOOR, 1);
	}
}

void sceneHandler25_enterTruba() {
	PicAniInfo info;

	g_nmi->_aniMan->getPicAniInfo(info);
	g_nmi->_aniMan->_messageQueueId = 0;
	g_nmi->_aniMan->changeStatics2(g_nmi->_aniMan->_statics->_staticsId);

	int x = g_nmi->_aniMan->_ox;
	int y = g_nmi->_aniMan->_oy;

	g_nmi->_aniMan->setPicAniInfo(info);

	int id = g_nmi->_aniMan->_statics->_staticsId;
	int qid = 0;

	if (id == ST_MAN25_ONBOARD && x == 634 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD);

		qid = QU_SC25_MANTOTRUBA;
	} else if (id == (ST_MAN25_ONBOARD|0x4000) && x == 632 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD|0x4000);

		qid = QU_SC25_MANTOTRUBA_R;
	}

	if (qid) {
		chainQueue(qid, 1);
		g_vars->scene25_sneezeFlipper = false;
	}
}

void sceneHandler25_saveEntrance(int value) {
	g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME")->setSubVarAsInt("Entrance", value);
}

void sceneHandler25_toLadder() {
	PicAniInfo info;

	g_nmi->_aniMan->getPicAniInfo(info);
	g_nmi->_aniMan->_messageQueueId = 0;
	g_nmi->_aniMan->changeStatics2(g_nmi->_aniMan->_statics->_staticsId);

	int x = g_nmi->_aniMan->_ox;
	int y = g_nmi->_aniMan->_oy;

	g_nmi->_aniMan->setPicAniInfo(info);

	int id = g_nmi->_aniMan->_statics->_staticsId;
	int qid = 0;

	if (id == ST_MAN25_ONBOARD && x == 307 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD);

		qid = QU_SC25_BACKTOLADDER;
	} else if (id == (ST_MAN25_ONBOARD|0x4000) && x == 192 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD|0x4000);

		qid = QU_SC25_BOARDTOLADDER;
	}

	if (qid) {
		chainQueue(qid, 1);
		g_vars->scene25_dudeIsOnBoard = false;
		g_vars->scene25_boardIsSelectable = true;
		g_vars->scene25_sneezeFlipper = false;

		sceneHandler25_saveEntrance(TrubaUp);
	}
}

void sceneHandler25_animateBearders() {
	if (g_nmi->_rnd.getRandomNumber(32767) < 218) {
		MessageQueue *mq;

		mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC25_BEARDED), 0, 1);

		mq->setParamInt(-1, g_vars->scene25_bearders[0]->_odelay);
		mq->getExCommandByIndex(0)->_x = g_nmi->_rnd.getRandomNumber(650) + 100;
		mq->chain(0);

		g_vars->scene25_beardersCounter = 0;

		if (g_nmi->_rnd.getRandomNumber(32767) < 0x1FFF) {
			mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC25_BEARDED2), 0, 1);

			mq->setParamInt(-1, g_vars->scene25_bearders[1]->_odelay);
			mq->getExCommandByIndex(0)->_x = g_nmi->_rnd.getRandomNumber(650) + 100;
			mq->chain(0);

			if (g_nmi->_rnd.getRandomNumber(32767) < 8191) {
				mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC25_BEARDED3), 0, 1);

				mq->setParamInt(-1, g_vars->scene25_bearders[2]->_odelay);
				mq->getExCommandByIndex(0)->_x = g_nmi->_rnd.getRandomNumber(650) + 100;
				mq->chain(0);
			}
		}
	}
}

void sceneHandler25_sneeze() {
	if (g_nmi->_rnd.getRandomNumber(32767) % 10) {
		if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN25_ONBOARD) {
			g_nmi->_aniMan->startAnim(MV_MAN25_ONBOARD, 0, -1);
		} else if (g_nmi->_aniMan->_statics->_staticsId == (ST_MAN25_ONBOARD|0x4000)) {
			g_nmi->_aniMan->startAnim(rMV_MAN25_ONBOARD, 0, -1);
		}
	} else if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN25_ONBOARD) {
		g_nmi->_aniMan->startAnim(MV_MAN25_CHIH, 0, -1);
	} else if (g_nmi->_aniMan->_statics->_staticsId == (ST_MAN25_ONBOARD|0x4000)) {
		g_nmi->_aniMan->startAnim(rMV_MAN25_CHIH, 0, -1);
	}
}

void sceneHandler25_rowShovel() {
	PicAniInfo info;

	g_nmi->_aniMan->getPicAniInfo(info);
	g_nmi->_aniMan->_messageQueueId = 0;
	g_nmi->_aniMan->changeStatics2(g_nmi->_aniMan->_statics->_staticsId);

	int x = g_nmi->_aniMan->_ox;
	int y = g_nmi->_aniMan->_oy;

	g_nmi->_aniMan->setPicAniInfo(info);

	int id = g_nmi->_aniMan->_statics->_staticsId;
	int qid = 0;

	if (id == ST_MAN25_ONBOARD && x == 370 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD);

		qid = QU_SC25_ROWTOTRUBA;
	} else if (id == (ST_MAN25_ONBOARD|0x4000) && x == 632 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD|0x4000);

		qid = QU_SC25_ROWTOLADDER;

		g_nmi->playSound(SND_25_030, 0);
	}

	if (qid) {
		chainQueue(qid, 1);

		g_vars->scene25_sneezeFlipper = false;
	}
}

void sceneHandler25_rowHand() {
	PicAniInfo info;

	g_nmi->_aniMan->getPicAniInfo(info);
	g_nmi->_aniMan->_messageQueueId = 0;
	g_nmi->_aniMan->changeStatics2(g_nmi->_aniMan->_statics->_staticsId);

	int x = g_nmi->_aniMan->_ox;
	int y = g_nmi->_aniMan->_oy;

	g_nmi->_aniMan->setPicAniInfo(info);

	int id = g_nmi->_aniMan->_statics->_staticsId;
	int qid = 0;

	if (id == ST_MAN25_ONBOARD && x == 370 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD);

		qid = QU_SC25_TRYROWHAND;
	} else if (id == (ST_MAN25_ONBOARD|0x4000) && x == 632 && y == 502) {
		g_nmi->_aniMan->changeStatics2(ST_MAN25_ONBOARD|0x4000);

		qid = QU_SC25_TRYROWHAND_R;
	}

	if (qid) {
		chainObjQueue(g_nmi->_aniMan, qid, 1);
		g_vars->scene25_sneezeFlipper = false;
	}
}

void sceneHandler25_putBoard() {
	if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER
		|| g_nmi->_aniMan->_statics->_staticsId == ST_MAN_LADDERDOWN_R) {
		g_nmi->_aniMan->changeStatics2(ST_MAN_STANDLADDER);
		g_nmi->_aniMan->setOXY(281, 481);

		chainQueue(QU_SC25_PUTBOARD, 1);

		g_vars->scene25_dudeIsOnBoard = true;
		g_vars->scene25_sneezeFlipper = false;
		g_vars->scene25_boardIsSelectable = false;
	}
}

void sceneHandler25_tryWater() {
	if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER
		|| g_nmi->_aniMan->_statics->_staticsId == ST_MAN_LADDERDOWN_R) {
		g_nmi->_aniMan->changeStatics2(ST_MAN_STANDLADDER);

		chainQueue(QU_SC25_TRYWATER, 1);
	}
}

void sceneHandler25_tryRow(int obj) {
	PicAniInfo info;

	g_nmi->_aniMan->getPicAniInfo(info);
	g_nmi->_aniMan->_messageQueueId = 0;
	g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

	int x = g_nmi->_aniMan->_ox;
	int y = g_nmi->_aniMan->_oy;

	g_nmi->_aniMan->setPicAniInfo(info);

	int qid = 0;

	if (x == 788 && y == 468) {
		if (g_vars->scene25_board->_statics->_staticsId == ST_BRD25_RIGHT2) {
			if (obj == ANI_INV_BROOM) {
				g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

				qid = QU_SC25_TRYBROOM;
			}
			if (obj == ANI_INV_LOPAT) {
				g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

				qid = QU_SC25_TRYSPADE;
			}

			if (qid) {
				chainQueue(qid, 1);

				g_nmi->playSound(SND_25_028, 0);

				return;
			}

			if (obj == ANI_INV_SWAB) {
				g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

				chainQueue(QU_SC25_TRYSWAB, 1);
			} else if (!obj) {
				g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

				chainObjQueue(g_nmi->_aniMan, QU_SC25_TRYHAND, 1);

				g_nmi->playSound(SND_25_028, 0);
			}
		} else if (g_vars->scene25_board->_statics->_staticsId == (ST_BRD25_RIGHT2 | 0x4000) && !obj) {
			g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);

			chainQueue(QU_SC25_TRUBATOBOARD, 1);

			g_vars->scene25_dudeIsOnBoard = true;
		}
	}
}

void sceneHandler25_ladderUp() {
	if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER
		|| g_nmi->_aniMan->_statics->_staticsId == ST_MAN_LADDERDOWN_R) {
		g_nmi->_aniMan->changeStatics2(ST_MAN_STANDLADDER);

		chainQueue(QU_SC25_LADDERUP, 1);
	}
}

void sceneHandler25_backToPipe() {
	if (!g_nmi->_aniMan->_movement && g_nmi->_aniMan->_statics->_staticsId == (ST_MAN_RIGHT|0x4000)) {
		g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);

		chainQueue(QU_SC25_BACKTOTRUBA, 1);
	}
}

void sceneHandler25_walkOnLadder(StaticANIObject *ani, Common::Point *pnt, MessageQueue *mq, int flag) {
	int aniY = ani->_oy;
	int newx = 0, newy = 0;
	Common::Point point;
	ExCommand *ex;

	if (flag) {
		if (ani->_movement) {
			point = ani->_movement->calcSomeXY(0, ani->_movement->_currDynamicPhaseIndex);
			newx = point.x;
			aniY = ani->_oy - point.y;
		}
	}

	int pntx = pnt->x;
	int pnty = pnt->y;
	int numObsolete = -1;
	int minDistance = 20000;
	ExCommand *lastEx = 0;

	for (uint i = 0; i < mq->getCount(); i++) {
		int curDistance = abs(pnty - aniY);

		ex = mq->getExCommandByIndex(i);

		if (ex->_messageKind == 1 && ani->_id == ex->_parentId) {
			if (ex->_excFlags & 0x10000) {
				if (ex->_messageNum == MV_MAN_TOLADDER)
					ex->_messageNum = MV_MAN_TOLADDER2;
				if (ex->_messageNum == MV_MAN_STARTLADDER)
					ex->_messageNum = MV_MAN_STARTLADDER2;
				if (ex->_messageNum == MV_MAN_GOLADDER)
					ex->_messageNum = MV_MAN_GOLADDER2;
				if (ex->_messageNum == MV_MAN_STOPLADDER)
					ex->_messageNum = MV_MAN_STOPLADDER2;
			}

			if (curDistance < minDistance || numObsolete < 0) {
				numObsolete = i;
				minDistance = curDistance;
				lastEx = ex;
				newx = pntx;
				newy = pnty;
			}

			point = ani->getMovementById(ex->_messageNum)->calcSomeXY(0, -1);
			pntx += point.x;
			pnty += point.y;
		}
	}

	for (int i = 0; i < numObsolete; i++)
		mq->deleteExCommandByIndex(0, 1);

	ex = new ExCommand(ani->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

	ex->_z = 256;
	ex->_messageNum = 0;
	ex->_excFlags |= 3;

	mq->addExCommandToEnd(ex);

	if (flag && ani->_movement && ani->_movement->_id == mq->getExCommandByIndex(0)->_messageNum) {
		mq->deleteExCommandByIndex(0, 1);

		int movId = ani->_movement->_id;
		int idx = ani->_movement->_currDynamicPhaseIndex;

		ani->changeStatics2(ani->_movement->_staticsObj1->_staticsId);
		ani->setOXY(newx, newy);

		ani->startAnim(movId, mq->_id, -1);

		ani->_movement->setDynamicPhaseIndex(idx);
	} else {
		if (!lastEx)
			error("sceneHandler25_walkOnLadder(): Incorrect state. Please report this to sev");

		ani->changeStatics2(ani->getMovementById(lastEx->_messageNum)->_staticsObj1->_staticsId);
		ani->setOXY(newx, newy);
		ani->restartMessageQueue(mq);
	}

	ani->_flags |= 0x100;
}

bool sceneHandler25_isOnLadder(ExCommand *cmd) {
	if ((g_nmi->_aniMan->_movement && g_nmi->_aniMan->_movement->_id == MV_MAN_GOLADDERDOWN)
		|| g_nmi->_aniMan->_statics->_staticsId == ST_MAN_GOLADDERD) {
		Interaction *inter = getGameLoaderInteractionController()->getInteractionByObjectIds(PIC_SC25_LADDERDOWN, ANI_MAN, cmd->_param);

		if (!inter)
			return 0;

		MessageQueue *mq = new MessageQueue(inter->_messageQueue, 0, 1);
		PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(PIC_SC25_LADDERDOWN, 0);
		Common::Point point;

		point.x = inter->_xOffs + pic->_ox;
		point.y = inter->_yOffs + pic->_oy;

		mq->setFlags(mq->getFlags() | 1);

		sceneHandler25_walkOnLadder(g_nmi->_aniMan, &point, mq, 0);

		return true;
	} else {
		return false;
	}
}

int sceneHandler25(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC25_STOPBEARDEDS:
		sceneHandler25_stopBearders();
		break;

	case MSG_SC25_STARTBEARDEDS:
		sceneHandler25_startBearders();
		break;

	case MSG_SC25_ENTERMAN:
		sceneHandler25_enterMan();
		break;

	case MSG_SC25_ENTERTRUBA:
		sceneHandler25_enterTruba();
		break;

	case MSG_SC25_TOLADDER:
		sceneHandler25_toLadder();
		break;

	case MSG_BRD_TURN:
		switch (g_nmi->_rnd.getRandomNumber(3)) {
		case 0:
			g_nmi->playSound(SND_25_025, 0);
			break;

		case 1:
			g_nmi->playSound(SND_25_026, 0);
			break;

		case 2:
		default:
			g_nmi->playSound(SND_25_027, 0);
			break;
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;
			int y = g_nmi->_aniMan2->_oy;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			if (!g_vars->scene25_waterIsPresent) {
				if (y < g_nmi->_sceneRect.top + 200)
					g_nmi->_currentScene->_y = y - 300 - g_nmi->_sceneRect.top;

				if (y > g_nmi->_sceneRect.bottom - 200)
					g_nmi->_currentScene->_y = y + 300 - g_nmi->_sceneRect.bottom;
			}
		}

		if (g_vars->scene25_beardersAreThere) {
			g_vars->scene25_beardersCounter++;

			if (g_vars->scene25_beardersCounter >= 120)
				sceneHandler25_animateBearders();
		}

		g_nmi->_behaviorManager->updateBehaviors();
		g_nmi->startSceneTrack();

		if (g_vars->scene25_waterIsPresent && !g_vars->scene25_water->_movement)
			g_vars->scene25_water->startAnim(MV_WTR25_FLOW, 0, -1);

		if (g_vars->scene25_dudeIsOnBoard && !g_nmi->_aniMan->_movement && g_vars->scene25_sneezeFlipper)
			sceneHandler25_sneeze();

		g_vars->scene25_sneezeFlipper = true;

		if (g_vars->scene25_board->_flags & 4) {
			if (!g_vars->scene25_board->_movement) {
				if (g_vars->scene25_board->_statics->_staticsId & 0x4000)
					g_vars->scene25_board->startAnim(rMV_BRD25_RIGHT, 0, -1);
				else
					g_vars->scene25_board->startAnim(MV_BRD25_RIGHT, 0, -1);
			}
		}
		break;

	case 29:
		{
			int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!g_vars->scene25_waterIsPresent) {
				if ((picId == PIC_SC25_LADDERUP || picId == PIC_SC25_LADDERDOWN) && sceneHandler25_isOnLadder(cmd))
					cmd->_messageKind = 0;

				return 0;
			}

			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani) {
				if (g_nmi->_aniMan != ani) {
					if (g_nmi->_aniMan->isIdle()) {
						if (!(g_nmi->_aniMan->_flags & 0x100)) {
							if (ani->_id == ANI_WATER25) {
								if (g_vars->scene25_dudeIsOnBoard) {
									if (cmd->_param == ANI_INV_LOPAT) {
										sceneHandler25_rowShovel();
										return 0;
									}

									if (!cmd->_param) {
										sceneHandler25_rowHand();
										return 0;
									}
								} else {
									if (cmd->_param == ANI_INV_BOARD) {
										sceneHandler25_putBoard();
										return 0;
									}

									if (!cmd->_param) {
										sceneHandler25_tryWater();
										return 0;
									}
								}
							} else if (ani->_id == ANI_BOARD25) {
								sceneHandler25_tryRow(cmd->_param);
								return 0;
							}
							return 0;
						}
					}
				}
			}

			if (picId == PIC_SC25_LADDERUP && sceneHandler25_isOnLadder(cmd))
				cmd->_messageKind = 0;

			if (!g_nmi->_aniMan->isIdle() || (g_nmi->_aniMan->_flags & 0x100))
				return 0;

			if (g_vars->scene25_dudeIsOnBoard) {
				if (picId == PIC_SC25_RTRUBA && !cmd->_param) {
					sceneHandler25_enterTruba();
					return 0;
				}
			} else {
				if (picId != PIC_SC25_RTRUBA) {
					if (picId == PIC_SC25_LADDERUP && !cmd->_param)
						sceneHandler25_ladderUp();
					return 0;
				}

				if (!cmd->_param) {
					sceneHandler25_backToPipe();
					return 0;
				}
			}
			if (g_vars->scene25_dudeIsOnBoard) {
				if (picId != PIC_SC25_LADDERUP || cmd->_param)
					return 0;

				sceneHandler25_toLadder();
				return 0;
			}

			if (picId == PIC_SC25_LADDERUP && !cmd->_param) {
				sceneHandler25_ladderUp();
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
