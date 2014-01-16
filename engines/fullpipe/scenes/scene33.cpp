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


namespace Fullpipe {

const int ventsInit[9] = { 0, 0, 1, 0, 0, 1, 0, 0, 1 };

void scene33_initScene(Scene *sc) {
	g_vars->scene33_mug = sc->getStaticANIObject1ById(ANI_MUG_33, -1);
	g_vars->scene33_jettie = sc->getStaticANIObject1ById(ANI_JETTIE_FLOW, -1);
	g_vars->scene33_cube = 0;
	g_vars->scene33_cubeX = -1;
	g_vars->scene33_handleIsDown = false;

	if (g_fp->getObjectState(sO_Cube) == g_fp->getObjectEnumState(sO_Cube, sO_In_33)) {
		MessageQueue *mq = new MessageQueue(sc->getMessageQueueById(QU_KBK33_START), 0, 0);

		mq->sendNextCommand();
	}


	for (int i = 0; i < 9; i++) {
		g_vars->scene33_ventsX[i] = sc->getStaticANIObject1ById(ANI_VENT_33, i)->_ox;

		g_vars->scene33_ventsState[i] = ventsInit[i];
	}

	g_fp->initArcadeKeys("SC_33");
}

void scene33_setupMusic() {
	if (g_fp->lift_checkButton(sO_Level6))
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_33"), "MUSIC2", 1);
}

int scene33_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor == PIC_SC33_ZONES && g_fp->_cursorId == PIC_CSR_DEFAULT)
		g_fp->_cursorId = PIC_CSR_ITN;

	return g_fp->_cursorId;
}

void sceneHandler33_processJettie(ExCommand *cmd) {
	MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

	if (mq && g_vars->scene33_jettie->_movement) {
		ExCommand *ex = mq->getExCommandByIndex(0);

		if (ex) {
			ex->_messageKind = 0;
			ex->_excFlags |= 1;
		}

		ex = mq->getExCommandByIndex(1);

		if (ex) {
			ex->_messageKind = 0;
			ex->_excFlags |= 1;
		}
	}
}

void sceneHandler33_switchVent(StaticANIObject *ani) {
	int mv = 0;

	if (ani->_statics->_staticsId == ST_VNT33_DOWN)
		mv = MV_VNT33_TURNR;

	if (ani->_statics->_staticsId == ST_VNT33_RIGHT)
		mv = MV_VNT33_TURND;

	if (mv)
		ani->startAnim(mv, 0, -1);

	g_vars->scene33_ventsState[ani->_okeyCode] = !g_vars->scene33_ventsState[ani->_okeyCode];
}

void sceneHandler33_processVents() {
	for (int i = 0; i < 9; i++)
		if (((g_vars->scene33_cubeX < g_vars->scene33_ventsX[i]) != (g_vars->scene33_cube->_ox < g_vars->scene33_ventsX[i]))
			&& g_vars->scene33_ventsState[i] != ventsInit[i])
				sceneHandler33_switchVent(g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, i));

	g_vars->scene33_cubeX = g_vars->scene33_cube->_ox;
}

void sceneHandler33_tryCube() {
	if (g_fp->getObjectState(sO_Cube) == g_fp->getObjectEnumState(sO_Cube, sO_In_32))
		chainQueue(QU_KBK33_GO, 0);
}

void sceneHandler33_pour() {
	bool solved = true;

	for (int i = 0; i < 9; i++)
		if (g_vars->scene33_ventsState[i] != ventsInit[i])
			solved = false;

	if (solved) {
		if ((g_vars->scene33_mug->_flags & 4) && g_vars->scene33_mug->_statics->_staticsId == ST_MUG33_EMPTY) {
			g_vars->scene33_jettie->startAnim(MV_JTI33_POUR, 0, -1);

			g_vars->scene33_handleIsDown = false;

			return;
		}

		if ((g_vars->scene33_mug->_flags & 4) && g_vars->scene33_mug->_statics->_staticsId == ST_MUG33_FULL) {
			g_vars->scene33_jettie->startAnim(MV_JTI33_POURFULL, 0, -1);

			g_vars->scene33_handleIsDown = false;

			return;
		}

		g_vars->scene33_jettie->startAnim(MV_JTI33_FLOW, 0, -1);
	}

	g_vars->scene33_handleIsDown = false;
}

void sceneHandler33_handleDown() {
  if (!g_vars->scene33_handleIsDown && !g_vars->scene33_jettie->_movement && !g_vars->scene33_jettie->getMessageQueue() ) {
	  chainQueue(QU_SC33_STARTWATER, 0);

	  g_vars->scene33_handleIsDown = true;
  }
}

void sceneHandler33_zoneClickProcess(StaticANIObject *ani) {
	if (!ani->_movement) {
		sceneHandler33_switchVent(ani);

		StaticANIObject *vent1 = 0;
		StaticANIObject *vent2 = 0;

		switch (ani->_okeyCode) {
		case 0:
			vent1 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 2);
			vent2 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 3);
			break;

		case 1:
			vent1 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 3);
			vent2 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 4);
			break;

		case 2:
			vent1 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 4);
			vent2 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 0);
			break;

		case 3:
			vent1 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 0);
			vent2 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 1);
			break;

		case 4:
			vent1 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 1);
			vent2 = g_fp->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 2);
			break;

		default:
			return;
		}

		if (vent1) {
			sceneHandler33_switchVent(vent1);
			sceneHandler33_switchVent(vent2);
		}
	}
}

void sceneHandler33_clickZones(ExCommand *cmd) {
	StaticANIObject *closest = 0;
	double mindist = 1e10;

	for (uint i = 0; i < g_fp->_currentScene->_staticANIObjectList1.size(); i++) {
		StaticANIObject *ani = (StaticANIObject *)g_fp->_currentScene->_staticANIObjectList1[i];

		if (ani->_id == ANI_VENT_33) {
			int dx = ani->_ox - cmd->_sceneClickX;
			int dy = ani->_oy - cmd->_sceneClickY;
			double dist = sqrt((double)(dx * dx + dy * dy));

			if (dist < mindist) {
				mindist = dist;
				closest = ani;
			}
		}
	}

	if (closest)
		sceneHandler33_zoneClickProcess(closest);
}

int sceneHandler33(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC32_TRUBATOFRONT:
		g_fp->_currentScene->getPictureObjectById(PIC_SC33_LTRUBA, 0)->_priority = 0;
		break;

	case MSG_SC32_TRUBATOBACK:
		g_fp->_currentScene->getPictureObjectById(PIC_SC33_LTRUBA, 0)->_priority = 20;
		break;

	case MSG_SC33_TESTMUG:
		sceneHandler33_processJettie(cmd);
		break;

	case MSG_SC33_UPDATEKUBIK:
		g_vars->scene33_cube = g_fp->_currentScene->getStaticANIObject1ById(ANI_KUBIK, -1);

		if (g_vars->scene33_cube)
			g_vars->scene33_cubeX = g_vars->scene33_cube->_ox;

		break;

	case MSG_SC33_TRYKUBIK:
		sceneHandler33_tryCube();
		break;

	case MSG_SC33_POUR:
		sceneHandler33_pour();
		break;

	case MSG_SC33_HANDLEDOWN:
		sceneHandler33_handleDown();
		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (pic && pic->_id == PIC_SC33_ZONES) {
					sceneHandler33_clickZones(cmd);
					break;
				}

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1) || (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
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

		if (g_vars->scene33_cube)
			sceneHandler33_processVents();

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
