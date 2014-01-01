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
#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene17_initScene(Scene *sc) {
	g_vars->scene17_var01 = 200;
	g_vars->scene17_var02 = 200;
	g_vars->scene17_var03 = 300;
	g_vars->scene17_var04 = 300;
	g_vars->scene17_var05 = 1;
	g_vars->scene17_var06 = 0;
	g_vars->scene17_var07 = 0;
	g_vars->scene17_var08 = 0;
	g_vars->scene17_hand = sc->getStaticANIObject1ById(ANI_HAND17, -1);
}

void sceneHandler17_drop() {
	warning("STUB: sceneHandler17_drop()");
}

void sceneHandler17_fillBottle() {
	warning("STUB: sceneHandler17_fillBottle()");
}

void sceneHandler17_testTruba() {
	warning("STUB: sceneHandler17_testTruba()");
}

void sceneHandler17_showBottle() {
	warning("STUB: sceneHandler17_showBottle()");
}

void sceneHandler17_hideSugar() {
	warning("STUB: sceneHandler17_hideSugar()");
}

void sceneHandler17_showSugar() {
	warning("STUB: sceneHandler17_showSugar()");
}

void sceneHandler17_moonshineFill() {
	warning("STUB: sceneHandler17_moonshineFill()");
}

void sceneHandler17_updateFlies() {
	warning("STUB: sceneHandler17_updateFlies()");
}


int sceneHandler17(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC17_DROP:
		sceneHandler17_drop();
		break;

	case MSG_SC17_UPDATEHAND:
		if (g_fp->getObjectState(sO_UsherHand) == g_fp->getObjectEnumState(sO_UsherHand, sO_WithCoin)) {
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 0);

			g_vars->scene17_var09 = 0;
		} else {
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 1);

			g_vars->scene17_var09 = 1;
		}
        break;

	case MSG_SC17_FILLBOTTLE:
		sceneHandler17_fillBottle();
		break;

	case MSG_SC17_TESTTRUBA:
		sceneHandler17_testTruba();
		break;

	case MSG_SC17_SHOWBOTTLE:
		sceneHandler17_showBottle();
		break;

	case MSG_SC17_HIDESUGAR:
		sceneHandler17_hideSugar();
		break;

	case MSG_SC17_SHOWSUGAR:
		sceneHandler17_showSugar();
		break;

	case 29:
		{
			int pic = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (pic == PIC_SC17_RTRUBA2 || pic == PIC_SC17_RTRUBA) {
				if (cmd->_keyCode == ANI_INV_COIN || cmd->_keyCode == ANI_INV_BOOT || cmd->_keyCode == ANI_INV_HAMMER) {
					if (g_vars->scene17_var09) {
						if (g_fp->_aniMan->isIdle()) {
							if (!(g_fp->_aniMan->_flags & 0x100)) {
								handleObjectInteraction(g_fp->_aniMan, g_vars->scene17_hand, cmd->_keyCode);
								break;
							}
						}
					}
				}
			}
		}
		break;

	case 33:
		{
			int x = g_vars->scene17_var10;
			g_vars->scene17_var07 = g_vars->scene17_var10;

			if (g_fp->_aniMan2) {
				x = g_fp->_aniMan2->_ox;

				g_vars->scene17_var10 = x;

				if (x < g_fp->_sceneRect.left + g_vars->scene17_var01) {
					g_fp->_currentScene->_x = x - g_vars->scene17_var03 - g_fp->_sceneRect.left;

					x = g_vars->scene17_var10;
				}

				if (x > g_fp->_sceneRect.right - g_vars->scene17_var01) {
					g_fp->_currentScene->_x = x + g_vars->scene17_var03 - g_fp->_sceneRect.right;
					x = g_vars->scene17_var10;
				}
			}

			if (g_vars->scene17_var06) {
				sceneHandler17_moonshineFill();
				x = g_vars->scene17_var10;
			}

			if (g_vars->scene17_var09) {
				if (g_vars->scene17_var07 < 410 && x >= 410) {
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 0);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_ATTRACT, QU_HND17_ATTRACT, 0);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 1);
				} else if (g_vars->scene17_var07 > 410 && x <= 410) {
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_TOCYCLE, 1);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_ATTRACT, QU_HND17_ATTRACT, 1);
					g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene17_hand, ST_HND17_EMPTY, QU_HND17_ASK, 0);
				}
			}

			--g_vars->scene17_var08;

			if (!g_vars->scene17_var08)
				sceneHandler17_updateFlies();

			g_fp->_floaters->update();

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();

			break;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
