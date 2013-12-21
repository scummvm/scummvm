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
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene10_initScene(Scene *sc) {
	g_vars->scene10_gum = sc->getStaticANIObject1ById(sc, ANI_GUM, -1);
	g_vars->scene10_packet = sc->getStaticANIObject1ById(sc, ANI_PACHKA, -1);
	g_vars->scene10_packet2 = sc->getStaticANIObject1ById(sc, ANI_PACHKA2, -1);
	g_vars->scene10_inflater = sc->getStaticANIObject1ById(sc, ANI_NADUVATEL, -1);
	g_vars->scene10_ladder = sc->getPictureObjectById(sc, PIC_SC10_LADDER, 0);

	g_fp->lift_setButton(sO_Level1, ST_LBN_1N);
	g_fp->lift_sub5(sc, QU_SC10_ENTERLIFT, QU_SC10_EXITLIFT);

	if (g_fp->getObjectState(sO_Inflater) == g_fp->getObjectEnumState(sO_Inflater, sO_WithGum)) {
		g_vars->scene10_hasGum = 1;
	} else {
		g_vars->scene10_hasGum = 0;
		g_vars->scene10_gum->hide();
	}
}

int sceneHandler10(ExCommand *ex) {
	if (ex->msg._messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		lift_closedoorSeq();
        break;

	case MSG_LIFT_EXITLIFT:
		lift_exitSeq(ex);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		lift_startExitQueue();
		break;

	case MSG_LIFT_CLICKBUTTON:
		lift_animation3();
		break;

	case MSG_SC10_LADDERTOBACK:
        g_vars->scene10_ladder->_priority = 49;
		break;

	case MSG_SC10_LADDERTOFORE:
		g_vars->scene10_ladder->_priority = 0;
		break;

	case MSG_LIFT_GO:
		lift_goAnimation();
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
		lift_sub05(ex);
		break;

	case 29:
		if (g_fp->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY) == PIC_SC10_LADDER) {
			handleObjectInteraction(g_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC10_DTRUBA, 0), ex->_keyCode);
			ex->_messageKind = 0;

			return 0;
		}

		StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

		if (ani && ani->_id == ANI_LIFTBUTTON) {
			lift_sub1(ani);
			ex->_messageKind = 0;

			return 0;
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
