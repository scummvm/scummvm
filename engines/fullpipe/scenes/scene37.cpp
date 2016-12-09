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

Ring::Ring() {
	ani = 0;
	x = 0;
	y = 0;
	numSubRings = 0;

	for (int i = 0; i < 10; i++)
		subRings[i] = 0;

	state = false;
}

void scene37_initScene(Scene *sc) {
	Ring *ring;
	StaticANIObject *ani;

	for (uint i = 0; i < g_vars->scene37_rings.size(); i++)
		delete g_vars->scene37_rings[i];

	g_vars->scene37_rings.clear();

	g_vars->scene37_lastDudeX = -1;

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 0);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRings[0] = 1;
	ring->subRings[1] = 4;
	ring->subRings[2] = 8;
	ring->state = false;
	g_vars->scene37_rings.push_back(ring);

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 1);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRings[0] = 2;
	ring->subRings[1] = 5;
	ring->subRings[2] = 9;
	ring->state = false;
	g_vars->scene37_rings.push_back(ring);

	ring = new Ring();
	ani = sc->getStaticANIObject1ById(ANI_GUARD_37, 2);
	ring->ani = ani;
	ring->x = ani->_ox - 40;
	ring->y = ani->_ox + 40;
	ring->numSubRings = 3;
	ring->subRings[0] = 3;
	ring->subRings[1] = 7;
	ring->subRings[2] = 11;
	ring->state = false;
	g_vars->scene37_rings.push_back(ring);

	g_fp->setObjectState(sO_LeftPipe_37, g_fp->getObjectEnumState(sO_LeftPipe_37, sO_IsClosed));

	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	g_vars->scene37_pipeIsOpen = false;

	g_vars->scene37_plusMinus1 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 1);

	for (int i = 0; i < g_vars->scene37_rings[0]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_rings[0]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_1) == g_fp->getObjectEnumState(sO_Guard_1, sO_On)) {
			g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_vars->scene37_plusMinus2 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 2);

	for (int i = 0; i < g_vars->scene37_rings[1]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_rings[1]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_2) == g_fp->getObjectEnumState(sO_Guard_2, sO_On)) {
			g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_vars->scene37_plusMinus3 = sc->getStaticANIObject1ById(ANI_PLUSMINUS, 3);

	for (int i = 0; i < g_vars->scene37_rings[2]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_rings[2]->subRings[i]);

		if (g_fp->getObjectState(sO_Guard_3) == g_fp->getObjectEnumState(sO_Guard_3, sO_On)) {
			g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_PLUS);
			ani->changeStatics2(ST_RNG_OPEN);
		} else {
			g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_MINUS);
			ani->changeStatics2(ST_RNG_CLOSED2);
		}
	}

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_37");
}

int scene37_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_cursorId == PIC_CSR_ITN && g_fp->_objectIdAtCursor == PIC_SC37_MASK) {
		if (g_vars->scene37_pipeIsOpen)
			g_fp->_cursorId = PIC_CSR_GOL;
	}

	return g_fp->_cursorId;
}

void sceneHandler37_updateRing(int ringNum) {
	g_vars->scene37_rings[ringNum]->ani->changeStatics2(ST_GRD37_STAND);
	g_vars->scene37_rings[ringNum]->ani->startAnim(MV_GRD37_PULL, 0, -1);
	g_vars->scene37_rings[ringNum]->state = !g_vars->scene37_rings[ringNum]->state;

	StaticANIObject *ani;

	for (int i = 0; i < g_vars->scene37_rings[ringNum]->numSubRings; i++) {
		ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_rings[ringNum]->subRings[i]);

		if ((ani->_movement && ani->_movement->_id != MV_RNG_CLOSE) || ani->_statics->_staticsId != ST_RNG_CLOSED2) {
			ani->changeStatics2(ST_RNG_OPEN);
			ani->startAnim(MV_RNG_CLOSE, 0, -1);
		} else {
			ani->changeStatics2(ST_RNG_CLOSED2);
			ani->startAnim(MV_RNG_OPEN, 0, -1);
		}
	}

	g_vars->scene37_pipeIsOpen = true;

	for (uint i = 0; i < g_vars->scene37_rings.size(); i++) {
		for (int j = 0; j < g_vars->scene37_rings[i]->numSubRings; j++) {
			ani = g_fp->_currentScene->getStaticANIObject1ById(ANI_RING, g_vars->scene37_rings[i]->subRings[j]);

			debugC(2, kDebugSceneLogic, "ring[%d][%d]: mov: %d st: %d", i, j, (ani->_movement ? ani->_movement->_id : 0), ani->_statics->_staticsId);

			if (ani->_movement) {
				if (ani->_movement->_id == MV_RNG_CLOSE)
					g_vars->scene37_pipeIsOpen = false;
			} else {
				if (ani->_statics->_staticsId == ST_RNG_CLOSED2)
					g_vars->scene37_pipeIsOpen = false;
			}
		}
	}

	debugC(1, kDebugSceneLogic, "Pipe is: %s", g_vars->scene37_pipeIsOpen ? "open" : "closed");

	int state;

	if (g_vars->scene37_pipeIsOpen)
		state = g_fp->getObjectEnumState(sO_LeftPipe_37, sO_IsOpened);
	else
		state = g_fp->getObjectEnumState(sO_LeftPipe_37, sO_IsClosed);

	g_fp->setObjectState(sO_LeftPipe_37, state);
}

void sceneHandler37_setRingsState() {
	if (g_vars->scene37_lastDudeX == -1) {
		g_vars->scene37_lastDudeX = g_vars->scene37_dudeX;
	} else {
		for (uint i = 0; i < g_vars->scene37_rings.size(); i++) {
			int x = g_vars->scene37_rings[i]->x;

			if (g_vars->scene37_lastDudeX > x && g_vars->scene37_dudeX <= x && !g_vars->scene37_rings[i]->state)
				sceneHandler37_updateRing(i);

			x = g_vars->scene37_rings[i]->y;

			if (g_vars->scene37_lastDudeX < x && g_vars->scene37_dudeX >= x) {
				if (g_vars->scene37_rings[i]->state)
					sceneHandler37_updateRing(i);
			}
		}

		g_vars->scene37_lastDudeX = g_vars->scene37_dudeX;
	}
}

int sceneHandler37(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_SC37_EXITLEFT:
		sceneHandler37_updateRing(0);
		sceneHandler37_updateRing(1);
		sceneHandler37_updateRing(2);

		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_param)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_param)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						break;
					}
				}
			}
		}

		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			g_vars->scene37_dudeX = x;

			if (x >= 500) {
				if (x < g_fp->_sceneRect.left + 200)
					g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;
			} else {
				g_fp->_currentScene->_x = -g_fp->_sceneRect.left;
			}
			x = g_vars->scene37_dudeX;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;

			g_fp->sceneAutoScrolling();
		}

		sceneHandler37_setRingsState();

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

		++g_vars->scene37_soundFlipper;

		break;

	case MSG_SC37_PULL:
		if (g_vars->scene37_rings[0]->ani->_movement && g_vars->scene37_rings[0]->ani->_movement->_id == MV_GRD37_PULL) {
			if ((g_fp->getObjectState(sO_Guard_1) == g_fp->getObjectEnumState(sO_Guard_1, sO_On) && !g_vars->scene37_rings[0]->state)
				|| (g_fp->getObjectState(sO_Guard_1) == g_fp->getObjectEnumState(sO_Guard_1, sO_Off) && g_vars->scene37_rings[0]->state)) {
				g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_PLUS);
			} else {
				g_vars->scene37_plusMinus1->_statics = g_vars->scene37_plusMinus1->getStaticsById(ST_PMS_MINUS);
			}
		} else if (g_vars->scene37_rings[1]->ani->_movement && g_vars->scene37_rings[1]->ani->_movement->_id == MV_GRD37_PULL) {
			if ((g_fp->getObjectState(sO_Guard_2) == g_fp->getObjectEnumState(sO_Guard_2, sO_On) && !g_vars->scene37_rings[1]->state)
				|| (g_fp->getObjectState(sO_Guard_2) == g_fp->getObjectEnumState(sO_Guard_2, sO_Off) && g_vars->scene37_rings[1]->state)) {
				g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_PLUS);
			} else {
				g_vars->scene37_plusMinus2->_statics = g_vars->scene37_plusMinus2->getStaticsById(ST_PMS_MINUS);
			}
		} else if (g_vars->scene37_rings[2]->ani->_movement && g_vars->scene37_rings[2]->ani->_movement->_id == MV_GRD37_PULL) {
			if ((g_fp->getObjectState(sO_Guard_3) == g_fp->getObjectEnumState(sO_Guard_3, sO_On) && !g_vars->scene37_rings[2]->state)
				|| (g_fp->getObjectState(sO_Guard_3) == g_fp->getObjectEnumState(sO_Guard_3, sO_Off) && g_vars->scene37_rings[2]->state)) {
				g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_PLUS);
			} else {
				g_vars->scene37_plusMinus3->_statics = g_vars->scene37_plusMinus3->getStaticsById(ST_PMS_MINUS);
			}
		}

		if (g_vars->scene37_soundFlipper) {
			g_fp->playSound(SND_37_007, 0);

			g_vars->scene37_soundFlipper = 0;
		}

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
