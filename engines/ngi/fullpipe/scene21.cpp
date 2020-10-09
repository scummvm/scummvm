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

void scene21_initScene(Scene *sc) {
	Scene *oldsc = g_nmi->_currentScene;

	g_vars->scene21_giraffeBottom = sc->getStaticANIObject1ById(ANI_GIRAFFE_BOTTOM, -1);
	g_nmi->_currentScene = sc;

	if (g_nmi->getObjectState(sO_LowerPipe_21) == g_nmi->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
		g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_HANG);
		g_vars->scene21_pipeIsOpen = true;
		g_vars->scene21_wigglePos = 0.0;
		g_vars->scene21_giraffeBottomX = g_vars->scene21_giraffeBottom->_ox;
		g_vars->scene21_giraffeBottomY = g_vars->scene21_giraffeBottom->_oy;
		g_vars->scene21_wiggleTrigger = false;
	} else {
		g_vars->scene21_pipeIsOpen = false;
	}
	g_nmi->_currentScene = oldsc;
	g_nmi->initArcadeKeys("SC_21");
}

int scene21_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_cursorId == PIC_CSR_ITN && g_nmi->_objectIdAtCursor == PIC_SC21_DTRUBA)

		g_nmi->_cursorId = PIC_CSR_GOD;

	return g_nmi->_cursorId;
}

void sceneHandler21_doWiggle() {
	g_vars->scene21_giraffeBottom->setOXY((int)(cos(g_vars->scene21_wigglePos) * 4.0) + g_vars->scene21_giraffeBottom->_ox,
										  g_vars->scene21_giraffeBottom->_oy);

	g_vars->scene21_wigglePos += 0.19635;

	if (g_vars->scene21_wigglePos > 6.2831853) {
		g_vars->scene21_wigglePos = 0;

		if (!g_vars->scene21_giraffeBottom->_movement)
			g_vars->scene21_giraffeBottom->setOXY(g_vars->scene21_giraffeBottomX, g_vars->scene21_giraffeBottomY);
	}
}

int sceneHandler21(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC21_UPDATEASS:
		if (g_nmi->getObjectState(sO_LowerPipe_21) == g_nmi->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
			g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_HANG);
			g_vars->scene21_giraffeBottom->setOXY(g_vars->scene21_giraffeBottomX, g_vars->scene21_giraffeBottomY);
			g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_SIT);

			g_vars->scene21_pipeIsOpen = false;

			g_nmi->setObjectState(sO_LowerPipe_21, g_nmi->getObjectEnumState(sO_LowerPipe_21, sO_IsClosed));
		}

		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param) ) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0))
						g_nmi->processArcade(cmd);
				}
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x <= g_nmi->_sceneWidth - 460) {
				if (x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;
			} else {
				g_nmi->_currentScene->_x = g_nmi->_sceneWidth - x;
			}

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();
		}

		if (g_vars->scene21_pipeIsOpen && !g_vars->scene21_wiggleTrigger)
			sceneHandler21_doWiggle();

		g_vars->scene21_wiggleTrigger = !g_vars->scene21_wiggleTrigger;

		g_nmi->_behaviorManager->updateBehaviors();
		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
