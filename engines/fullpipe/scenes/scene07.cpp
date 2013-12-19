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

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/messages.h"
#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene07_initScene(Scene *sc) {
	g_vars->scene07_lukeAnim = 0;
	g_vars->scene07_lukePercent = 0;
	g_vars->scene07_plusMinus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_fullpipe->getObjectState(sO_Guard_1) == g_fullpipe->getObjectEnumState(sO_Guard_1, sO_Off))
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_PLUS);

	if (g_fullpipe->getObjectState(sO_HareTheNooksiter) == g_fullpipe->getObjectEnumState(sO_HareTheNooksiter, sO_WithoutHandle)) {
		Scene *oldsc = g_fullpipe->_currentScene;

		g_fullpipe->_currentScene = sc;

		sc->getStaticANIObject1ById(ANI_CORNERSITTER, -1)->changeStatics2(ST_CST_HANDLELESS);

		g_fullpipe->_currentScene = oldsc;
	}
}

void sceneHandler07_openLuke() {
	warning("STUB: sceneHandler07_openLuke()");
}

void sceneHandler07_closeLuke() {
	warning("STUB: sceneHandler07_closeLuke()");
}

void sceneHandler07_hideLuke() {
	warning("STUB: sceneHandler07_hideLuke()");
}

void sceneHandler07_showBox() {
	warning("STUB: sceneHandler07_showBox()");
}

void sceneHandler07_hideBox() {
	warning("STUB: sceneHandler07_hideBox()");
}

int sceneHandler07(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_SC7_OPENLUKE:
		sceneHandler07_openLuke();
		break;

	case MSG_SC7_PULL:
		if (g_vars->scene07_plusMinus->_statics->_staticsId == ST_PMS_MINUS)
			g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_PLUS);
		else
			g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_MINUS);

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

			if (g_fullpipe->_aniMan2) {
				if (g_fullpipe->_aniMan2->_ox < g_fullpipe->_sceneRect.left + 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.left - 300;

				if (g_fullpipe->_aniMan2->_ox > g_fullpipe->_sceneRect.right - 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.right + 300;

				res = 1;
			}

			g_fullpipe->_behaviorManager->updateBehaviors();

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
