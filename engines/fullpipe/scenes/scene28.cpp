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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

void scene28_initScene(Scene *sc) {
	g_vars->scene28_fliesArePresent = true;
	g_vars->scene28_beardedDirection = true;
	g_vars->scene28_darkeningObject = 0;
	g_vars->scene28_lighteningObject = 0;
	g_vars->scene28_headDirection = false;
	g_vars->scene28_headBeardedFlipper = false;
	g_vars->scene28_lift6inside = false;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"));
	
	g_fp->initArcadeKeys("SC_28");
}

int scene28_updateCursor() {
	g_fp->updateCursorCommon();
	if (g_fp->_objectIdAtCursor == ANI_LIFT || g_fp->_objectIdAtCursor == ANI_LIFT_28)
		if (g_fp->_cursorId == PIC_CSR_DEFAULT)
			g_fp->_cursorId = PIC_CSR_ITN;

	return g_fp->_cursorId;
}

void sceneHandler28_lift1ShowAfter() {
	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_MAN_28, -1);

	g_fp->_aniMan->_statics = g_fp->_aniMan->getStaticsById(ST_MAN_SIT|0x4000);
	g_fp->_aniMan->setOXY(ani->_ox + 7, ani->_oy);
	g_fp->_aniMan->_priority = ani->_priority;
	g_fp->_aniMan->show1(-1, -1, -1, 0);
}

void sceneHandler28_makeFaces(ExCommand *cmd) {
	g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK4, 0)->_flags &= 0xFFFB;

	g_vars->scene28_lighteningObject = 0;

	MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(cmd->_parId);
	if (mq) {
		int frames[5];

		frames[0] = MV_WMN28_IN_1;
		frames[1] = MV_WMN28_IN_2;
		frames[2] = MV_WMN28_IN_3;
		frames[3] = MV_WMN28_IN_4;
		frames[4] = MV_WMN28_IN_5;

		for (int i = 0; i < 5; i++) {
			int pos;

			while (frames[pos = g_fp->_rnd->getRandomNumber(4)] == 0)
				;

			mq->getExCommandByIndex(i)->_messageNum = frames[pos];

			frames[pos] = 0;
		}
	}
}

void sceneHandler28_trySecondaryPers() {
	MessageQueue *mq;
	int x;

	if (g_vars->scene28_headBeardedFlipper) {
		if (g_vars->scene28_beardedDirection) {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOR), 0, 1);

			mq->getExCommandByIndex(0)->_x = g_fp->_sceneRect.left - 20;
			mq->getExCommandByIndex(0)->_keyCode = 1;
			mq->replaceKeyCode(-1, 1);
			mq->chain(0);

			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOR), 0, 1);

			mq->getExCommandByIndex(0)->_x = g_fp->_sceneRect.left - 40;
			mq->getExCommandByIndex(0)->_y += 20;
			mq->getExCommandByIndex(0)->_keyCode = 2;
			mq->replaceKeyCode(-1, 2);
			mq->chain(0);

			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOR), 0, 1);

			x = g_fp->_sceneRect.left - 60;
		} else {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOL), 0, 1);

			mq->getExCommandByIndex(0)->_x = g_fp->_sceneRect.right + 20;
			mq->getExCommandByIndex(0)->_keyCode = 1;
			mq->replaceKeyCode(-1, 1);
			mq->chain(0);

			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOL), 0, 1);

			mq->getExCommandByIndex(0)->_x = g_fp->_sceneRect.right + 40;
			mq->getExCommandByIndex(0)->_y += 20;
			mq->getExCommandByIndex(0)->_keyCode = 2;
			mq->replaceKeyCode(-1, 2);
			mq->chain(0);

			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_BRD28_GOL), 0, 1);

			x = g_fp->_sceneRect.right + 60;
		}

		mq->getExCommandByIndex(0)->_x = x;
		mq->getExCommandByIndex(0)->_y += 40;
		mq->getExCommandByIndex(0)->_keyCode = 3;
		mq->replaceKeyCode(-1, 3);
		mq->chain( 0);

		g_vars->scene28_beardedDirection = !g_vars->scene28_beardedDirection;
	} else {
		if (g_vars->scene28_headDirection) {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_GLV28_GOR), 0, 1);

			x = g_fp->_sceneRect.left - 40;
		} else {
			mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_GLV28_GOL), 0, 1);

			x = g_fp->_sceneRect.right + 40;
		}

		mq->getExCommandByIndex(0)->_x = x;
		mq->chain(0);

		g_vars->scene28_headDirection = !g_vars->scene28_headDirection;
	}

	g_vars->scene28_headBeardedFlipper = !g_vars->scene28_headBeardedFlipper;
}

void sceneHandler28_turnOn2() {
	if (g_vars->scene28_fliesArePresent) {
		g_fp->_floaters->genFlies(g_fp->_currentScene, 1013, 329, 60, 4);

		g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->val15 = 30;
		g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->countdown = g_fp->_rnd->getRandomNumber(12) + 12;

		g_fp->_floaters->genFlies(g_fp->_currentScene, 1074, 311, 60, 4);

		g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->val15 = 30;
		g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->countdown = g_fp->_rnd->getRandomNumber(12) + 12;
	}

	g_vars->scene28_fliesArePresent = false;
}

void sceneHandler28_startWork1() {
	g_fp->_aniMan->hide();

	StaticANIObject *man = g_fp->_currentScene->getStaticANIObject1ById(ANI_MAN_28, -1);

	man->_statics = man->getStaticsById(ST_MAN28_RIGHT);
	man->setOXY(g_fp->_aniMan->_ox, g_fp->_aniMan->_oy);
	man->_priority = g_fp->_aniMan->_priority;
	man->show1(-1, -1, -1, 0);

	chainQueue(QU_SC28_LIFT1_WORK, 1);
}

void sceneHandler28_lift0Start() {
	chainQueue(QU_SC28_LIFT0_START, 1);
}

void sceneHandler28_lift1Start() {
	g_fp->_aniMan->_flags |= 1;

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	chainQueue(QU_SC28_LIFT1_START, 1);
}

void sceneHandler28_lift2Start() {
	chainQueue(QU_SC28_LIFT2_START, 1);
}

void sceneHandler28_lift3Start() {
	chainQueue(QU_SC28_LIFT3_START, 1);
}

void sceneHandler28_lift4Start() {
	g_fp->_aniMan->_flags |= 1;

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	chainQueue(QU_SC28_WMN_START, 1);
}

void sceneHandler28_lift5Start() {
	chainQueue(QU_SC28_LIFT5_START, 1);
}

void sceneHandler28_lift6Start() {
	g_fp->_aniMan->_flags |= 1;

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	StaticANIObject *woman = g_fp->_currentScene->getStaticANIObject1ById(ANI_TIOTIA, -1);

	if (woman && (woman->_flags & 4))
		chainQueue(QU_SC28_LIFT6_START2, 1);
	else
		chainQueue(QU_SC28_LIFT6_START, 1);
}

void sceneHandler28_clickLift(int keycode) {
	int x = 0;

	switch (keycode) {
	case 0: x = 600; break;
	case 1: x = 824; break;
	case 2: x = 1055; break;
	case 3: x = 1286; break;
	case 4: x = 1517; break;
	case 5: x = 1748; break;
	case 6: x = 1979; break;
	}

	if (abs(x - g_fp->_aniMan->_ox) > 1 || abs(472 - g_fp->_aniMan->_oy) > 1
		|| g_fp->_aniMan->_movement
		|| g_fp->_aniMan->_statics->_staticsId != ST_MAN_UP) {
		MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, x, 472, 1, ST_MAN_UP);
		if (mq) {
			ExCommand *ex = new ExCommand(0, 17, MSG_SC28_CLICKLIFT, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 3;

			mq->addExCommandToEnd(ex);

			postExCommand(g_fp->_aniMan->_id, 2, x, 472, 0, -1);
		}
	} else {
		switch (keycode) {
		case 0:
			sceneHandler28_lift0Start();
			break;
		case 1:
			sceneHandler28_lift1Start();
			break;
		case 2:
			sceneHandler28_lift2Start();
			break;
		case 3:
			sceneHandler28_lift3Start();
			break;
		case 4:
			sceneHandler28_lift4Start();
			break;
		case 5:
			sceneHandler28_lift5Start();
			break;
		case 6:
			sceneHandler28_lift6Start();
			break;
		default:
			return;
		}
	}
}

int sceneHandler28(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC28_LIFT6MUSIC:
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"), "MUSIC_TIOTIA", 1);
		break;

	case MSG_SC28_LIFT6INSIDE:
		g_vars->scene28_lift6inside = true;
		break;

	case MSG_SC28_LIFT1_SHOWAFTER:
		sceneHandler28_lift1ShowAfter();
		break;

	case MSG_SC28_MAKEFACES:
		sceneHandler28_makeFaces(cmd);
		break;

	case MSG_SC28_TRYVTORPERS:
		sceneHandler28_trySecondaryPers();
		break;

	case MSG_SC28_TURNOFF_0:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK0, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_TURNON_0:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK0, 0);
		break;

	case MSG_SC28_TURNON_1:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK1, 0);
		break;

	case MSG_SC28_TURNOFF_1:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK1, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_TURNON_2:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK2, 0);
		sceneHandler28_turnOn2();
		break;

	case MSG_SC28_TURNOFF_2:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK2, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_TURNON_3:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK3, 0);
		break;

	case MSG_SC28_TURNOFF_3:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK3, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_TURNON_4:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK4, 0);
		break;

	case MSG_SC28_TURNOFF_4:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK4, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_TURNON_6:
		g_vars->scene28_darkeningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK6, 0);
		break;

	case MSG_SC28_TURNOFF_6:
		g_vars->scene28_lighteningObject = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK6, 0);
		g_vars->scene28_lighteningObject->_flags |= 4;
		break;

	case MSG_SC28_STARTWORK1:
		sceneHandler28_startWork1();
		break;

	case MSG_SC28_CLICKLIFT:
		sceneHandler28_clickLift(cmd->_keyCode);
		break;

	case MSG_SC28_ENDLIFT1:
	case MSG_SC28_ENDLIFT6:
	case MSG_SC28_ENDCABIN:
		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);
		g_fp->_aniMan->_flags &= 0xFEFF;
		break;

	case 29:
		{
			if (g_vars->scene28_lift6inside) {
				chainObjQueue(g_fp->_aniMan, QU_SC28_LIFT6_END, 1);

				g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"), "MUSIC", 1);

				g_vars->scene28_lift6inside = false;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani)
				if (ani->_id == ANI_LIFT || ani->_id == ANI_LIFT_28 ) {
					sceneHandler28_clickLift(ani->_okeyCode);

					cmd->_messageKind = 0;
					break;
				}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
						g_fp->processArcade(cmd);
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

		if (g_vars->scene28_darkeningObject) {
			if (g_vars->scene28_darkeningObject->_picture->getAlpha() > 10) {
				g_vars->scene28_darkeningObject->_picture->setAlpha(g_vars->scene28_darkeningObject->_picture->getAlpha() - 10);
			} else {
				g_vars->scene28_darkeningObject->_flags &= 0xFFFB;

				g_vars->scene28_darkeningObject = 0;
			}
		}

		if (g_vars->scene28_lighteningObject) {
			if (g_vars->scene28_lighteningObject->_picture->getAlpha() < 0xF9u) {
				g_vars->scene28_lighteningObject->_picture->setAlpha(g_vars->scene28_lighteningObject->_picture->getAlpha() + 6);
			} else {
				g_vars->scene28_lighteningObject->_picture->setAlpha(0xff);

				g_vars->scene28_lighteningObject = 0;
			}
		}

		g_fp->_floaters->update();

		for (uint i = 0; i < g_fp->_floaters->_array2.size(); i++)
			if (g_fp->_floaters->_array2[i]->val13 == 1)
				g_fp->_floaters->_array2[i]->ani->_priority = 15;

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
