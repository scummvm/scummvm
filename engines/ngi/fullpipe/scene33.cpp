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

const int ventsInit[9] = { 0, 0, 1, 0, 0, 1, 0, 0, 1 };

void scene33_initScene(Scene *sc) {
	g_vars->scene33_mug = sc->getStaticANIObject1ById(ANI_MUG_33, -1);
	g_vars->scene33_jettie = sc->getStaticANIObject1ById(ANI_JETTIE_FLOW, -1);
	g_vars->scene33_cube = 0;
	g_vars->scene33_cubeX = -1;
	g_vars->scene33_handleIsDown = false;

	if (g_nmi->getObjectState(sO_Cube) == g_nmi->getObjectEnumState(sO_Cube, sO_In_33)) {
		MessageQueue *mq = new MessageQueue(sc->getMessageQueueById(QU_KBK33_START), 0, 0);

		mq->sendNextCommand();
	}


	for (int i = 0; i < 9; i++) {
		g_vars->scene33_ventsX[i] = sc->getStaticANIObject1ById(ANI_VENT_33, i)->_ox;

		g_vars->scene33_ventsState[i] = ventsInit[i];
	}

	g_nmi->initArcadeKeys("SC_33");
}

void scene33_setupMusic() {
	if (g_nmi->lift_checkButton(sO_Level6))
		g_nmi->playTrack(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_33"), "MUSIC2", 1);
}

int scene33_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor == PIC_SC33_ZONES && g_nmi->_cursorId == PIC_CSR_DEFAULT)
		g_nmi->_cursorId = PIC_CSR_ITN;

	return g_nmi->_cursorId;
}

void sceneHandler33_processJettie(ExCommand *cmd) {
	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(cmd->_parId);

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

	g_vars->scene33_ventsState[ani->_odelay] = !g_vars->scene33_ventsState[ani->_odelay];
}

void sceneHandler33_processVents() {
	for (int i = 0; i < 9; i++)
		if (((g_vars->scene33_cubeX < g_vars->scene33_ventsX[i]) != (g_vars->scene33_cube->_ox < g_vars->scene33_ventsX[i]))
			&& g_vars->scene33_ventsState[i] != ventsInit[i])
				sceneHandler33_switchVent(g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, i));

	g_vars->scene33_cubeX = g_vars->scene33_cube->_ox;
}

void sceneHandler33_tryCube() {
	if (g_nmi->getObjectState(sO_Cube) == g_nmi->getObjectEnumState(sO_Cube, sO_In_32))
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

		switch (ani->_odelay) {
		case 0:
			vent1 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 2);
			vent2 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 3);
			break;

		case 1:
			vent1 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 3);
			vent2 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 4);
			break;

		case 2:
			vent1 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 4);
			vent2 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 0);
			break;

		case 3:
			vent1 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 0);
			vent2 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 1);
			break;

		case 4:
			vent1 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 1);
			vent2 = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT_33, 2);
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

	for (uint i = 0; i < g_nmi->_currentScene->_staticANIObjectList1.size(); i++) {
		StaticANIObject *ani = g_nmi->_currentScene->_staticANIObjectList1[i];

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
		g_nmi->_currentScene->getPictureObjectById(PIC_SC33_LTRUBA, 0)->_priority = 0;
		break;

	case MSG_SC32_TRUBATOBACK:
		g_nmi->_currentScene->getPictureObjectById(PIC_SC33_LTRUBA, 0)->_priority = 20;
		break;

	case MSG_SC33_TESTMUG:
		sceneHandler33_processJettie(cmd);
		break;

	case MSG_SC33_UPDATEKUBIK:
		g_vars->scene33_cube = g_nmi->_currentScene->getStaticANIObject1ById(ANI_KUBIK, -1);

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
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (pic && pic->_id == PIC_SC33_ZONES) {
					sceneHandler33_clickZones(cmd);
					break;
				}

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1) || (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0))
						g_nmi->processArcade(cmd);
				}
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();
		}

		if (g_vars->scene33_cube)
			sceneHandler33_processVents();

		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
