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

void scene30_enablePass(Scene *sc) {
	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_WayToPipe);

	if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_WithAll))
		lnk->_flags &= 0xDFFFFFFF;
	else
		lnk->_flags |= 0x20000000;
}

void scene30_initScene(Scene *sc, int flag) {
	Scene *oldsc = g_nmi->_currentScene;

	g_vars->scene30_leg = sc->getStaticANIObject1ById(ANI_LEG, -1);
	g_nmi->_currentScene = sc;

	if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_ShowingHeel))
		g_vars->scene30_leg->changeStatics2(ST_LEG_UP);
	else if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_WithoutJugs))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN);
	else if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_WithBig))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN1);
	else if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_WithSmall))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN2);
	else if (g_nmi->getObjectState(sO_Leg) == g_nmi->getObjectEnumState(sO_Leg, sO_WithAll))
		g_vars->scene30_leg->changeStatics2(ST_LEG_EMPTY);

	g_nmi->_currentScene = oldsc;

	scene30_enablePass(sc);

	if (flag == LiftUp || flag == LiftDown)
		g_vars->scene30_liftFlag = 0;
	else
		g_vars->scene30_liftFlag = 1;

	g_nmi->lift_setButton(sO_Level8, ST_LBN_8N);

	g_nmi->lift_init(sc, QU_SC30_ENTERLIFT, QU_SC30_EXITLIFT);
}

int scene30_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_cursorId == PIC_CSR_ITN && g_nmi->_objectIdAtCursor == PIC_SC30_LTRUBA) {
		g_nmi->_cursorId = PIC_CSR_GOL;
	}
	return g_nmi->_cursorId;
}

int sceneHandler30(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SC30_UPDATEPATH:
		scene30_enablePass(g_nmi->_currentScene);
		break;

	case 64:
		g_nmi->lift_hoverButton(cmd);
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);

				cmd->_messageKind = 0;
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
		}

		g_nmi->_behaviorManager->updateBehaviors();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
