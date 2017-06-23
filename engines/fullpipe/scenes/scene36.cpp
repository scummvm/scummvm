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

void scene36_initScene(Scene *sc) {
	g_vars->scene36_rotohrust = sc->getStaticANIObject1ById(ANI_ROTOHRUST, -1);
	g_vars->scene36_scissors = sc->getStaticANIObject1ById(ANI_SCISSORS_36, -1);
}

int scene36_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_cursorId != PIC_CSR_ITN || g_fp->_objectIdAtCursor != ANI_ROTOHRUST) {
		if (g_fp->_objectIdAtCursor == PIC_SC36_MASK && g_fp->_cursorId == PIC_CSR_DEFAULT && (g_vars->scene36_scissors->_flags & 4))
			g_fp->_cursorId = PIC_CSR_ITN;
	} else if (g_vars->scene36_rotohrust->_statics->_staticsId == ST_RHT_OPEN)
		g_fp->_cursorId = PIC_CSR_GOL;

	return g_fp->_cursorId;
}

int sceneHandler36(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case 29:
		if (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC36_MASK)
			if (g_vars->scene36_scissors)
				if (g_vars->scene36_scissors->_flags & 4)
					if (g_fp->_aniMan->isIdle())
						if (!(g_fp->_aniMan->_flags & 0x100) && g_fp->_msgObjectId2 != g_vars->scene36_scissors->_id ) {
							handleObjectInteraction(g_fp->_aniMan, g_vars->scene36_scissors, cmd->_param);

							cmd->_messageKind = 0;
						}

		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
