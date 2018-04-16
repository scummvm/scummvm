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

#include "fullpipe/behavior.h"
#include "fullpipe/interaction.h"

namespace Fullpipe {

void scene10_initScene(Scene *sc) {
	g_vars->scene10_gum = sc->getStaticANIObject1ById(ANI_GUM, -1);
	g_vars->scene10_packet = sc->getStaticANIObject1ById(ANI_PACHKA, -1);
	g_vars->scene10_packet2 = sc->getStaticANIObject1ById(ANI_PACHKA2, -1);
	g_vars->scene10_inflater = sc->getStaticANIObject1ById(ANI_NADUVATEL, -1);
	g_vars->scene10_ladder = sc->getPictureObjectById(PIC_SC10_LADDER, 0);

	g_fp->lift_setButton(sO_Level1, ST_LBN_1N);
	g_fp->lift_init(sc, QU_SC10_ENTERLIFT, QU_SC10_EXITLIFT);

	if (g_fp->getObjectState(sO_Inflater) == g_fp->getObjectEnumState(sO_Inflater, sO_WithGum)) {
		g_vars->scene10_hasGum = 1;
	} else {
		g_vars->scene10_hasGum = 0;
		g_vars->scene10_gum->hide();
	}
}

bool sceneHandler10_inflaterIsBlind() {
	return g_vars->scene10_inflater->_movement
		&& g_vars->scene10_inflater->_movement->_id == MV_NDV_BLOW2
		&& g_vars->scene10_inflater->_movement->_currDynamicPhaseIndex < 42;
}

int scene10_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor == ANI_PACHKA || g_fp->_objectIdAtCursor == ANI_GUM) {
		if (g_fp->_cursorId == PIC_CSR_ITN) {
			if (g_vars->scene10_hasGum)
				g_fp->_cursorId = (sceneHandler10_inflaterIsBlind() != 0) ? PIC_CSR_ITN_GREEN : PIC_CSR_ITN_RED;
			else
				g_fp->_cursorId = PIC_CSR_DEFAULT;
		}
	}

	return g_fp->_cursorId;
}

void sceneHandler10_clickGum() {
	if (g_vars->scene10_hasGum) {
		if (sceneHandler10_inflaterIsBlind()) {
			if (g_vars->scene10_hasGum) {
				int x = g_vars->scene10_gum->_ox - 139;
				int y = g_vars->scene10_gum->_oy - 48;

				if (abs(x - g_fp->_aniMan->_ox) > 1 || abs(y - g_fp->_aniMan->_oy) > 1) {
					MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_fp->_aniMan, x, y, 1, ST_MAN_RIGHT);
					if (mq) {
						ExCommand *ex = new ExCommand(0, 17, MSG_SC10_CLICKGUM, 0, 0, 0, 1, 0, 0, 0);
						ex->_excFlags = 2;
						mq->addExCommandToEnd(ex);

						postExCommand(g_fp->_aniMan->_id, 2, x, y, 0, -1);
					}
				} else {
					g_vars->scene10_hasGum = 0;

					chainQueue(QU_SC10_TAKEGUM, 1);
				}
			}
		} else {
			g_vars->scene10_inflater->changeStatics2(ST_NDV_SIT);

			if (g_fp->getObjectState(sO_Inflater) == g_fp->getObjectEnumState(sO_Inflater, sO_WithGum))
				g_vars->scene10_inflater->startAnim(MV_NDV_DENIES, 0, -1);
			else
				g_vars->scene10_inflater->startAnim(MV_NDV_DENY_NOGUM, 0, -1);
		}
	}
}

void sceneHandler10_hideGum() {
	g_vars->scene10_gum->hide();
	g_vars->scene10_packet->hide();
	g_vars->scene10_packet2->hide();
}

void sceneHandler10_showGum() {
	if (g_vars->scene10_hasGum)
		g_vars->scene10_gum->show1(-1, -1, -1, 0);

	g_vars->scene10_packet->show1(-1, -1, -1, 0);
	g_vars->scene10_packet2->show1(-1, -1, -1, 0);
}


int sceneHandler10(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(ex);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_clickButton();
		break;

	case MSG_SC10_LADDERTOBACK:
		g_vars->scene10_ladder->_priority = 49;
		break;

	case MSG_SC10_LADDERTOFORE:
		g_vars->scene10_ladder->_priority = 0;
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC10_CLICKGUM:
		sceneHandler10_clickGum();

		ex->_messageKind = 0;
		break;

	case MSG_SC10_HIDEGUM:
		sceneHandler10_hideGum();
		break;

	case MSG_SC10_SHOWGUM:
		sceneHandler10_showGum();
		break;

	case 64:
		g_fp->lift_hoverButton(ex);
		break;

	case 29:
		{
			if (g_fp->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY) == PIC_SC10_LADDER) {
				handleObjectInteraction(g_fp->_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC10_DTRUBA, 0), ex->_param);
				ex->_messageKind = 0;

				return 0;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_animateButton(ani);
				ex->_messageKind = 0;

				return 0;
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_fp->_aniMan2) {
				if (g_fp->_aniMan2->_ox < g_fp->_sceneRect.left + 200)
					g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.left - 300;

				if (g_fp->_aniMan2->_ox > g_fp->_sceneRect.right - 200)
					g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.right + 300;

				res = 1;
			}

			g_fp->_behaviorManager->updateBehaviors();
			g_fp->startSceneTrack();

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
