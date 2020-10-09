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

void scene36_initScene(Scene *sc) {
	g_vars->scene36_rotohrust = sc->getStaticANIObject1ById(ANI_ROTOHRUST, -1);
	g_vars->scene36_scissors = sc->getStaticANIObject1ById(ANI_SCISSORS_36, -1);
}

int scene36_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_cursorId != PIC_CSR_ITN || g_nmi->_objectIdAtCursor != ANI_ROTOHRUST) {
		if (g_nmi->_objectIdAtCursor == PIC_SC36_MASK && g_nmi->_cursorId == PIC_CSR_DEFAULT && (g_vars->scene36_scissors->_flags & 4))
			g_nmi->_cursorId = PIC_CSR_ITN;
	} else if (g_vars->scene36_rotohrust->_statics->_staticsId == ST_RHT_OPEN)
		g_nmi->_cursorId = PIC_CSR_GOL;

	return g_nmi->_cursorId;
}

int sceneHandler36(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case 29:
		if (g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC36_MASK)
			if (g_vars->scene36_scissors)
				if (g_vars->scene36_scissors->_flags & 4)
					if (g_nmi->_aniMan->isIdle())
						if (!(g_nmi->_aniMan->_flags & 0x100) && g_nmi->_msgObjectId2 != g_vars->scene36_scissors->_id ) {
							handleObjectInteraction(g_nmi->_aniMan, g_vars->scene36_scissors, cmd->_param);

							cmd->_messageKind = 0;
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
		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
