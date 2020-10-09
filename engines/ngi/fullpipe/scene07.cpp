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

#include "ngi/objects.h"
#include "ngi/objectnames.h"
#include "ngi/constants.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/scene.h"
#include "ngi/statics.h"
#include "ngi/messages.h"
#include "ngi/behavior.h"

namespace NGI {

void scene07_initScene(Scene *sc) {
	g_vars->scene07_lukeAnim = 0;
	g_vars->scene07_lukePercent = 0;
	g_vars->scene07_plusMinus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_nmi->getObjectState(sO_Guard_1) == g_nmi->getObjectEnumState(sO_Guard_1, sO_Off))
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_PLUS);

	if (g_nmi->getObjectState(sO_HareTheNooksiter) == g_nmi->getObjectEnumState(sO_HareTheNooksiter, sO_WithoutHandle)) {
		Scene *oldsc = g_nmi->_currentScene;

		g_nmi->_currentScene = sc;

		sc->getStaticANIObject1ById(ANI_CORNERSITTER, -1)->changeStatics2(ST_CST_HANDLELESS);

		g_nmi->_currentScene = oldsc;
	}
}

void sceneHandler07_openLuke() {
	StaticANIObject *luke = g_nmi->_currentScene->getStaticANIObject1ById(ANI_LUKE, -1);

	luke->changeStatics2(ST_LUK_OPEN);
	luke->show1(-1, -1, -1, 0);

	if (g_vars->scene07_lukeAnim) {
		g_vars->scene07_lukeAnim->_percent = g_vars->scene07_lukePercent;
	} else {
		StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_CORNERSITTER, -1);

		g_vars->scene07_lukeAnim = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(ani, ST_CST_HANDLELESS, QU_CST_CLOSELUKE);

		g_vars->scene07_lukeAnim->_percent = g_vars->scene07_lukePercent;
	}
}

void sceneHandler07_closeLuke() {
	g_nmi->_currentScene->getStaticANIObject1ById(ANI_LUKE, -1)->changeStatics2(ST_LUK_CLOSED);

	if (!g_vars->scene07_lukeAnim) {
		StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_CORNERSITTER, -1);

		g_vars->scene07_lukeAnim = g_nmi->_behaviorManager->getBehaviorMoveByMessageQueueDataId(ani, ST_CST_HANDLELESS, QU_CST_CLOSELUKE);
	}

	g_vars->scene07_lukePercent = g_vars->scene07_lukeAnim->_percent;
	g_vars->scene07_lukeAnim->_percent = 0;

	StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_HOOLIGAN, -1);

	ani->changeStatics2(ST_HGN_LUKE);
	ani->show1(-1, -1, -1, 0);
}

void sceneHandler07_hideLuke() {
	g_nmi->_currentScene->getStaticANIObject1ById(ANI_LUKE, -1)->hide();

	Movement *mov = g_nmi->_currentScene->getStaticANIObject1ById(ANI_CORNERSITTER, -1)->_movement;

	if (mov) {
		if (mov->_id == MV_CST_CLOSELUKE) {
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_HOOLIGAN, -1);

			ani->changeStatics2(ST_HGN_LOOK);
			ani->_flags &= 0xFFFB;
		}
	}
}

void sceneHandler07_showBox() {
	StaticANIObject *box = g_nmi->_currentScene->getStaticANIObject1ById(ANI_SC7_BOX, -1);

	box->show1(492, 474, MV_SC7_BOX_default, 0);
	box->_priority = 25;
}

void sceneHandler07_hideBox() {
	g_nmi->_currentScene->getStaticANIObject1ById(ANI_SC7_BOX, -1)->hide();
}

int sceneHandler07(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_SC7_OPENLUKE:
		sceneHandler07_openLuke();
		break;

	case MSG_SC7_PULL:
		if (g_vars->scene07_plusMinus->_statics->_staticsId == ST_PMS_MINUS) {
			g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_PLUS);
		} else {
			g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_MINUS);
		}
		break;

	case MSG_SC7_CLOSELUKE:
		sceneHandler07_closeLuke();
		break;

	case MSG_SC7_HIDELUKE:
		sceneHandler07_hideLuke();
		break;

	case MSG_SC7_SHOWBOX:
		sceneHandler07_showBox();
		break;

	case MSG_SC7_HIDEBOX:
		sceneHandler07_hideBox();
		break;

	case 33:
		{
			int res = 0;

			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan2->_ox < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.left - 300;

				if (g_nmi->_aniMan2->_ox > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.right + 300;

				res = 1;
			}

			g_nmi->_behaviorManager->updateBehaviors();

			return res;
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
