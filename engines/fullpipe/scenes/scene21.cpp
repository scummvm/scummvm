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
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/interaction.h"
#include "fullpipe/behavior.h"


namespace Fullpipe {

void scene21_initScene(Scene *sc) {
	g_vars->scene21_var01 = 200;
	g_vars->scene21_var02 = 200;
	g_vars->scene21_var03 = 300;
	g_vars->scene21_var04 = 300;

	Scene *oldsc = g_fp->_currentScene;

	g_vars->scene21_giraffeBottom = sc->getStaticANIObject1ById(ANI_GIRAFFE_BOTTOM, -1);
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
		g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_HANG);
		g_vars->scene21_var05 = 1;
		g_vars->scene21_var06 = 0.0;
		g_vars->scene21_giraffeBottomX = g_vars->scene21_giraffeBottom->_ox;
		g_vars->scene21_giraffeBottomY = g_vars->scene21_giraffeBottom->_oy;
		g_vars->scene21_var07 = 0;
	} else {
		g_vars->scene21_var05 = 0;
	}
	g_fp->_currentScene = oldsc;
	g_fp->initArcadeKeys("SC_21");
}

void sceneHandler21_doWiggle() {
	g_vars->scene21_giraffeBottom->setOXY((int)(cos(g_vars->scene21_var06) * 4.0) + g_vars->scene21_giraffeBottom->_ox,
										  g_vars->scene21_giraffeBottom->_oy);

	g_vars->scene21_var06 += 0.19635;

	if (g_vars->scene21_var06 > 6.2831853) {
		g_vars->scene21_var06 = 0;

		if (!g_vars->scene21_giraffeBottom->_movement)
			g_vars->scene21_giraffeBottom->setOXY(g_vars->scene21_giraffeBottomX, g_vars->scene21_giraffeBottomY);
	}
}

int sceneHandler21(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC21_UPDATEASS:
		if (g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
			g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_HANG);
			g_vars->scene21_giraffeBottom->setOXY(g_vars->scene21_giraffeBottomX, g_vars->scene21_giraffeBottomY);
			g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_SIT);

			g_vars->scene21_var05 = 0;

			g_fp->setObjectState(sO_LowerPipe_21, g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsClosed));
		}

		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode) ) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
						g_fp->processArcade(cmd);
				}
			}

			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x <= g_fp->_sceneWidth - 460) {
				if (x < g_fp->_sceneRect.left + g_vars->scene21_var01)
					g_fp->_currentScene->_x = x - g_vars->scene21_var03 - g_fp->_sceneRect.left;
			} else {
				g_fp->_currentScene->_x = g_fp->_sceneWidth - x;
			}

			if (x > g_fp->_sceneRect.right - g_vars->scene21_var01)
				g_fp->_currentScene->_x = x + g_vars->scene21_var03 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene21_var05 && !g_vars->scene21_var07)
			sceneHandler21_doWiggle();

		g_vars->scene21_var07 = g_vars->scene21_var07 == 0;

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
