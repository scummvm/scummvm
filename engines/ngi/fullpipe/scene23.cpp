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
#include "ngi/floaters.h"

namespace NGI {

bool sceneHandler23_testCalendar() {
	int cal0, cal1, cal2, cal3;

	if (g_vars->scene23_calend0->_movement)
		cal0 = g_vars->scene23_calend0->_movement->_staticsObj2->_staticsId;
	else
		cal0 = g_vars->scene23_calend0->_statics->_staticsId;

	if (g_vars->scene23_calend1->_movement)
		cal1 = g_vars->scene23_calend1->_movement->_staticsObj2->_staticsId;
	else
		cal1 = g_vars->scene23_calend1->_statics->_staticsId;

	if (g_vars->scene23_calend2->_movement)
		cal2 = g_vars->scene23_calend2->_movement->_staticsObj2->_staticsId;
	else
		cal2 = g_vars->scene23_calend2->_statics->_staticsId;

	if (g_vars->scene23_calend3->_movement)
		cal3 = g_vars->scene23_calend3->_movement->_staticsObj2->_staticsId;
	else
		cal3 = g_vars->scene23_calend3->_statics->_staticsId;

	return (cal0 == ST_CND_1 && cal1 == ST_CND_4 && cal2 == ST_CND_0 && cal3 == ST_CND_2 && (g_vars->scene23_giraffee->_flags & 4));
}

void scene23_initScene(Scene *sc) {
	g_vars->scene23_calend0 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 0);
	g_vars->scene23_calend1 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 1);
	g_vars->scene23_calend2 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 2);
	g_vars->scene23_calend3 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 3);
	g_vars->scene23_topReached = false;
	g_vars->scene23_isOnStool = false;
	g_vars->scene23_someVar = 0;
	g_vars->scene23_giraffeTop = sc->getStaticANIObject1ById(ANI_GIRAFFE_TOP, -1);
	g_vars->scene23_giraffee = sc->getStaticANIObject1ById(ANI_GIRAFFEE, -1);

	g_nmi->_floaters->init(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_23"));

	Scene *oldsc = g_nmi->_currentScene;
	g_nmi->_currentScene = sc;

	if (g_nmi->getObjectState(sO_UpperHatch_23) == g_nmi->getObjectEnumState(sO_UpperHatch_23, sO_Opened)) {
		sc->getPictureObjectById(PIC_SC23_BOXOPEN, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BOXCLOSED, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN1, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN2, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN3, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN4, 0)->_flags |= 4;

		if (g_vars->scene23_giraffee->_statics->_staticsId == ST_GRFG_EMPTY || !(g_vars->scene23_giraffee->_flags & 4)) {
			g_vars->scene23_giraffee->changeStatics2(ST_GRFG_BALD);
			g_vars->scene23_giraffee->_flags |= 4;
		}
		g_vars->scene23_calend0->show1(-1, -1, -1, 0);
		g_vars->scene23_calend1->show1(-1, -1, -1, 0);
		g_vars->scene23_calend2->show1(-1, -1, -1, 0);
		g_vars->scene23_calend3->show1(-1, -1, -1, 0);

		sc->getStaticANIObject1ById(ANI_LUK23_U, -1)->changeStatics2(ST_LUK23U_OPEN);
	} else {
		sc->getPictureObjectById(PIC_SC23_BOXOPEN, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BOXCLOSED, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN1, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN2, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN3, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN4, 0)->_flags &= 0xFFFB;

		g_vars->scene23_giraffee->hide();
		g_vars->scene23_calend0->hide();
		g_vars->scene23_calend1->hide();
		g_vars->scene23_calend2->hide();
		g_vars->scene23_calend3->hide();

		sc->getStaticANIObject1ById(ANI_LUK23_U, -1)->changeStatics2(ST_LUK23U_CLOSED);

		g_nmi->_floaters->genFlies(sc, 600, 90, 0, 0);
	}

	if (g_nmi->getObjectState(sO_LowerHatch_23) == g_nmi->getObjectEnumState(sO_LowerHatch_23, sO_Opened)) {
		g_vars->scene23_giraffeTop->show1(-1, -1, -1, 0);
		g_vars->scene23_giraffeTop->changeStatics2(ST_GRFU_UP);

		if (g_nmi->getObjectState(sO_LowerPipe_21) == g_nmi->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
			g_vars->scene23_giraffeTop->changeStatics2(ST_GRFU_KISS);
			g_vars->scene23_giraffee->hide();
		} else {
			if (g_nmi->getObjectState(sO_UpperHatch_23) == g_nmi->getObjectEnumState(sO_UpperHatch_23, sO_Opened)
				&& (g_vars->scene23_giraffee->_flags & 4))
				g_vars->scene23_giraffeTop->setOXY(614, 362);
			else
				g_vars->scene23_giraffeTop->setOXY(618, 350);

			if (sceneHandler23_testCalendar())
				g_vars->scene23_calend1->_statics = g_vars->scene23_calend1->getStaticsById(ST_CND_5);
		}

		sc->getStaticANIObject1ById(ANI_LUK23_D, -1)->changeStatics2(ST_LUK23_OPEN);

		if (g_nmi->getObjectState(sO_Lever_23) == g_nmi->getObjectEnumState(sO_Lever_23, sO_Taken))
			sc->getStaticANIObject1ById(ANI_INV_LEVERHANDLE, -1)->hide();

		sc->getStaticANIObject1ById(ANI_HANDLE23, -1)->hide();
	} else {
		g_vars->scene23_giraffeTop->hide();

		sc->getStaticANIObject1ById(ANI_LUK23_D, -1)->changeStatics2(ST_LUK23_WHANDLE2);

		sc->getStaticANIObject1ById(ANI_INV_LEVERHANDLE, -1)->hide();
	}

	g_nmi->_currentScene = oldsc;
}

void scene23_setGiraffeState() {
	if (g_nmi->getObjectState(sO_UpperHatch_23) == g_nmi->getObjectEnumState(sO_UpperHatch_23, sO_Opened)) {
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene23_giraffeTop, ST_GRFU_UP, QU_GRFU_TURN_UL, 0);
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene23_giraffeTop, ST_GRFU_UP, QU_GRFU_TURN_UD, 0);
	}
}

int scene23_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor == PIC_SC23_LADDERU) {
		if (g_vars->scene23_topReached)
			return g_nmi->_cursorId;

		g_nmi->_cursorId = getGameLoaderInventory()->getSelectedItemId() ? PIC_CSR_GOU : PIC_CSR_ITN; // FIXME check
	}

	if (g_nmi->_objectIdAtCursor == PIC_SC23_BTN1 || g_nmi->_objectIdAtCursor == PIC_SC23_BTN2
		|| g_nmi->_objectIdAtCursor == PIC_SC23_BTN3 || g_nmi->_objectIdAtCursor == PIC_SC23_BTN4
		|| g_nmi->_objectIdAtCursor == ANI_CALENDWHEEL)
		g_nmi->_cursorId = PIC_CSR_LIFT;

	return g_nmi->_cursorId;
}

void sceneHandler23_showStool() {
	chainQueue(QU_SC23_SHOWSTOOL, 0);
}

void sceneHandler23_hideStool() {
	g_nmi->_currentScene->getStaticANIObject1ById(ANI_TABURETTE, -1)->hide();
}

void sceneHandler23_startKiss() {
	g_vars->scene23_giraffeTop->changeStatics2(ST_GRFU_UP);
	g_vars->scene23_giraffeTop->startMQIfIdle(QU_SC23_STARTKISS, 0);
}

void sceneHandler23_spinWheel1() {
	int mv = 0;

	switch (g_vars->scene23_calend0->_statics->_staticsId) {
	case ST_CND_0:
		mv = MV_CND_0_1;
		break;

	case ST_CND_1:
		mv = MV_CND_1_2;
		break;

	case ST_CND_2:
		mv = MV_CND_2_3;
		break;

	case ST_CND_3:
		g_vars->scene23_calend0->changeStatics2(ST_CND_9);
		mv = MV_CND_9_0;
		break;

	default:
		break;
	}

	if (mv)
		g_vars->scene23_calend0->startAnim(mv, 0, -1);

	if (sceneHandler23_testCalendar())
		sceneHandler23_startKiss();
}

void sceneHandler23_spinWheel2and4(StaticANIObject *ani) {
	int mv = 0;

	switch (ani->_statics->_staticsId) {
	case ST_CND_0:
		mv = MV_CND_0_1;
		break;

	case ST_CND_1:
		mv = MV_CND_1_2;
		break;

	case ST_CND_2:
		mv = MV_CND_2_3;
		break;

	case ST_CND_3:
		mv = MV_CND_3_4;
		break;

	case ST_CND_4:
		mv = MV_CND_4_5;
		break;

	case ST_CND_5:
		mv = MV_CND_5_6;
		break;

	case ST_CND_6:
		mv = MV_CND_6_7;
		break;

	case ST_CND_7:
		mv = MV_CND_7_8;
		break;

	case ST_CND_8:
		mv = MV_CND_8_9;
		break;

	case ST_CND_9:
		mv = MV_CND_9_0;
		break;

	default:
		break;
	}

	if (mv)
		ani->startAnim(mv, 0, -1);

	if (sceneHandler23_testCalendar())
		sceneHandler23_startKiss();
}

void sceneHandler23_spinWheel3() {
	if (g_vars->scene23_calend2->_statics->_staticsId == ST_CND_0) {
		g_vars->scene23_calend2->startAnim(MV_CND_0_1, 0, -1);
	} else if (g_vars->scene23_calend2->_statics->_staticsId == ST_CND_1) {
		g_vars->scene23_calend2->changeStatics2(ST_CND_9);
		g_vars->scene23_calend2->startAnim(MV_CND_9_0, 0, -1);
	}

	if (sceneHandler23_testCalendar())
		sceneHandler23_startKiss();
}

void sceneHandler23_pushButton(ExCommand *cmd) {
	if (g_nmi->_aniMan->isIdle() || !(g_nmi->_aniMan->_flags & 0x100)) {
		if (!g_vars->scene23_topReached) {
			if (g_nmi->_aniMan->_ox != 405 || g_nmi->_aniMan->_oy != 220) {
				if (g_nmi->_aniMan->_ox != 276 || g_nmi->_aniMan->_oy != 438
					|| g_nmi->_aniMan->_movement || g_nmi->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
					if (g_nmi->_msgX == 276 && g_nmi->_msgY == 438 )
						return;

					MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 276, 438, 1, ST_MAN_RIGHT);

					if (mq) {
						mq->addExCommandToEnd(cmd->createClone());

						postExCommand(g_nmi->_aniMan->_id, 2, 276, 438, 0, -1);
					}
				} else {
					MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC23_TOCALENDAR), 0, 0);

					mq->addExCommandToEnd(cmd->createClone());
					mq->setFlags(mq->getFlags() | 1);
					mq->chain(0);
				}

				if (!g_vars->scene23_topReached)
					return;
			} else {
				g_vars->scene23_topReached = true;
			}
		}

		if (!g_nmi->_aniMan->_movement && g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER) {
			int mv = 0;

			switch (cmd->_messageNum) {
			case MSG_SC23_CLICKBTN1:
				mv = MV_MAN23_PUSH1;
				break;

			case MSG_SC23_CLICKBTN2:
				mv = MV_MAN23_PUSH2;
				break;

			case MSG_SC23_CLICKBTN3:
				mv = MV_MAN23_PUSH3;
				break;

			case MSG_SC23_CLICKBTN4:
				mv = MV_MAN23_PUSH4;
				break;

			default:
				return;
			}

			if (mv)
				g_nmi->_aniMan->startAnim(mv, 0, -1);

		}
	}
}

void sceneHandler23_sendClick(StaticANIObject *ani) {
	int msg = 0;
	switch (ani->_odelay) {
	case 0:
		msg = MSG_SC23_CLICKBTN1;
		break;
	case 1:
		msg = MSG_SC23_CLICKBTN2;
		break;
	case 2:
		msg = MSG_SC23_CLICKBTN3;
		break;
	case 3:
		msg = MSG_SC23_CLICKBTN4;
		break;
	default:
		break;
	}

	ExCommand *ex = new ExCommand(0, 17, msg, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 3;

	ex->postMessage();
}

void sceneHandler23_checkReachingTop() {
	if (g_nmi->_aniMan->_movement || g_nmi->_aniMan->_statics->_staticsId != ST_MAN_STANDLADDER
		|| g_nmi->_aniMan->_ox != 405 || g_nmi->_aniMan->_oy != 220)
		g_vars->scene23_topReached = false;
	else
		g_vars->scene23_topReached = true;
}

void sceneHandler23_exitCalendar() {
	if (!g_nmi->_aniMan->_movement && g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER
		&& !g_nmi->_aniMan->getMessageQueue() && !(g_nmi->_aniMan->_flags & 0x100)) {
		chainQueue(QU_SC23_FROMCALENDAREXIT, 1);
		g_vars->scene23_someVar = 2;
	}
}

void sceneHandler23_fromCalendar(ExCommand *cmd) {
	if (!g_nmi->_aniMan->_movement && g_nmi->_aniMan->_statics->_staticsId == ST_MAN_STANDLADDER
		&& !g_nmi->_aniMan->getMessageQueue() && !(g_nmi->_aniMan->_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC23_FROMCALENDAR), 0, 0);

		mq->addExCommandToEnd(cmd->createClone());
		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);

		g_vars->scene23_topReached = false;
		g_vars->scene23_someVar = 0;
	}
}

void sceneHandler23_fromStool(ExCommand *cmd) {
	if (!g_nmi->_aniMan->getMessageQueue() && !(g_nmi->_aniMan->_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC23_FROMSTOOL), 0, 0);

		mq->addExCommandToEnd(cmd->createClone());
		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);

		cmd->_messageKind = 0;
	}
}

int sceneHandler23(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC23_FROMSTOOL:
		g_vars->scene23_isOnStool = false;

		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enableFlag24();

		g_nmi->_behaviorManager->setFlagByStaticAniObject(g_nmi->_aniMan, 1);
		break;

	case MSG_SC23_HIDEGIRAFFEE:
		g_vars->scene23_giraffee->queueMessageQueue(0);
		g_vars->scene23_giraffee->_flags &= 0xFFFB;
		break;

	case MSG_SC23_ONSTOOL:
		g_vars->scene23_isOnStool = true;

		getCurrSceneSc2MotionController()->deactivate();
		getGameLoaderInteractionController()->disableFlag24();

		g_nmi->_behaviorManager->setFlagByStaticAniObject(g_nmi->_aniMan, 0);
		break;

	case MSG_SC22_SHOWSTOOL:
		sceneHandler23_showStool();
		break;

	case MSG_SC22_HIDESTOOL:
		sceneHandler23_hideStool();
		break;

	case MSG_SC23_SPINWHEEL1:
		sceneHandler23_spinWheel1();
		break;

	case MSG_SC23_SPINWHEEL2:
		sceneHandler23_spinWheel2and4(g_vars->scene23_calend1);
		break;

	case MSG_SC23_SPINWHEEL3:
		sceneHandler23_spinWheel3();
		break;

	case MSG_SC23_SPINWHEEL4:
		sceneHandler23_spinWheel2and4(g_vars->scene23_calend3);
		break;

	case MSG_SC23_CLICKBTN1:
	case MSG_SC23_CLICKBTN2:
	case MSG_SC23_CLICKBTN3:
	case MSG_SC23_CLICKBTN4:
		sceneHandler23_pushButton(cmd);
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		g_nmi->_floaters->update();
		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
			int picId;

			if (ani && ani->_id == ANI_CALENDWHEEL) {
				sceneHandler23_sendClick(ani);
				cmd->_messageKind = 0;
			}

			sceneHandler23_checkReachingTop();

			if (g_vars->scene23_topReached) {
				picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				if (picId == PIC_SC23_LADDER) {
					sceneHandler23_exitCalendar();

					cmd->_messageKind = 0;
					break;
				}

				if (cmd->_sceneClickY > 450) {
					sceneHandler23_fromCalendar(cmd);

					cmd->_messageKind = 0;
					break;
				}
				break;
			}

			if (!g_vars->scene23_isOnStool) {
				picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				if (picId == PIC_SC23_LADDERU && !g_vars->scene23_topReached) {
					sceneHandler23_pushButton(cmd);

					cmd->_messageKind = 0;
					break;
				}
				break;
			}

			if (ani && ani->_id == ANI_HANDLE23) {
				handleObjectInteraction(g_nmi->_aniMan, ani, cmd->_param);
				cmd->_messageKind = 0;
			} else {
				sceneHandler23_fromStool(cmd);

				cmd->_messageKind = 0;
			}

		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
