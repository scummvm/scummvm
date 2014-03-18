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

int sceneHandler29(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler29_winArcade();
		break;

	case MSG_SC29_LAUGH:
        if (g_vars->scene29_var18 == ANI_SHELL_GREEN) {
			playSound(SND_29_028, 0);
			break;
        }
        playSound(SND_29_029, 0);

		break;

	case MSG_SC29_SHOWLASTRED:
		if (g_vars->scene29_var05.numBalls) {
			g_vars->scene29_var06.field_8->ani->show1(-1, -1, -1, 0);
			g_vars->scene29_var06.field_8->ani->startAnim(MV_SHR_HITASS, 0, -1);
		}

		break;

	case MSG_SC29_SHOOTGREEN:
        sceneHandler29_shootGreen();
        break;

	case MSG_SC29_SHOOTRED:
        sceneHandler29_shootRed();
        break;

	case MSG_SC29_SHOWLASTGREEN:
		if (g_vars->scene29_var05.numBalls) {
			g_vars->scene29_var05.field_8->ani->show1(-1, -1, (Objects)-1, 0);
			g_vars->scene29_var05.field_8->ani->startAnim(MV_SHG_HITASS, 0, -1);
		}

		break;

	case MSG_SC29_STOPRIDE:
		g_vars->scene29_var09 = 0;
		g_vars->scene29_var10 = 0;
		g_vars->scene29_var11 = 0;
		g_vars->scene29_var12 = 0;

		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();
		break;

	case MSG_SC29_STOPRIDE:
		g_vars->scene29_var12 = 0;
		break;

	case MSG_SC29_DISABLERIDEBACK:
		g_vars->scene29_var12 = 1;
		// fall through

	case MSG_SC29_DISABLEPORTER:
		g_vars->scene29_var11 = 0;
		break;

	case MSG_SC29_ENABLEPORTER:
		g_vars->scene29_var11 = 1;
		g_vars->scene29_var12 = 0;
		g_vars->scene29_var15 = 0;
		break;

	case 29:
		if (!g_vars->scene29_var09 || g_vars->scene29_var10) {
			if (!g_vars->scene29_var10) {
				v6 = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);
				if (v6 && v6 == g_vars->scene29_porter) {
					sceneHandler29_clickPorter(cmd);

					cmd->_messageKind = 0;
					break;
				}
				break;
			}

			sceneHandler29_manJump();

			cmd->_messageKind = 0;
			break;
		}
		break;

	case 107:
		if (g_vars->scene29_var10)
			sceneHandler29_manBend();

		break;

	case 33:
		if (g_vars->scene29_var10) {
			v5 = g_vars->scene29_var20;
			if (g_vars->scene29_var20 > g_fp->_sceneRect.right - 500) {
				g_fp->_currentScene->_x = g_fp->_sceneRect.right - g_vars->scene29_var20 - 350;
				v5 = g_vars->scene29_var20;
			}

			if (v5 < g_sceneRect.left + 100)
				g_fp->_currentScene->_x = v5 - g_fp->_sceneRect.left - 100;

		} else if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene29_var01)
				g_fp->_currentScene->_x = x - g_vars->scene29_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene29_var01)
				g_fp->_currentScene->_x = x + g_vars->scene29_var03 - g_fp->_sceneRect.right;
		}

		g_vars->scene29_var20 = g_fp->_aniMan->_ox;
		g_vars->scene29_var21 = g_fp->_aniMan->_oy;

		sceneHandler29_sub03();

		if (!g_vars->scene29_porter->_movement)
			g_vars->scene29_porter->startAnim(MV_PTR_MOVEFAST, 0, -1);

		if (g_vars->scene29_var09) {
			sceneHandler29_manFromL();

		} else if (g_vars->scene29_var10 && !g_aniMan->_movement) {
			sceneHandler29_sub05();
		}

		if (g_vars->scene29_var11)
			sceneHandler29_shootersEscape();
		else if (g_vars->scene29_var12)
			sceneHandler29_sub07();

		g_vars->scene29_var13++;

		if (g_vars->scene29_var13 > g_vars->scene29_var14)
			sceneHandler29_shoot();

		sceneHandler29_animBearded();

		g_fp->_behaviorManager->updateBehaviors();
		startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
