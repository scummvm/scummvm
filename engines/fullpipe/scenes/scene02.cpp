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
#include "fullpipe/floaters.h"


namespace Fullpipe {

void scene02_initScene(Scene *sc) {
	g_vars->scene02_guvTheDrawer = sc->getStaticANIObject1ById(ANI_DADAYASHIK, -1);

	if (g_fp->getObjectState(sO_GuvTheDrawer) == g_fp->getObjectEnumState(sO_GuvTheDrawer, sO_IsSleeping)) {
		Scene *s = g_fp->_currentScene;

		g_fp->_currentScene = sc;
		g_vars->scene02_guvTheDrawer->changeStatics2(ST_DYAS_LIES);
		g_fp->_currentScene = s;
	}

	g_vars->scene02_boxDelay = 0;

	StaticANIObject *box = sc->getStaticANIObject1ById(ANI_SC2_BOX, -1);

	if (box && box->_flags & 4) {
		g_vars->scene02_boxOpen = false;
	} else {
		g_vars->scene02_boxOpen = true;
		g_vars->scene02_boxDelay = 100 * g_fp->_rnd.getRandomNumber(32767) + 150;
	}

	g_fp->_floaters->init(g_fp->_gameLoader->_gameVar->getSubVarByName("SC_2"));
}

void sceneHandler02_ladderClick() {
	handleObjectInteraction(g_fp->_aniMan2, g_fp->_currentScene->getPictureObjectById(PIC_SC2_DTRUBA, 0), 0);
}

void sceneHandler02_showLadder() {
	g_fp->_currentScene->getPictureObjectById(PIC_SC2_LADDER, 0)->_flags |= 4;
}

void sceneHandler02_hideLadder() {
	g_fp->_currentScene->getPictureObjectById(PIC_SC2_LADDER, 0)->_flags &= 0xfffb;
	g_fp->_aniMan2->_priority = 25;
}

int sceneHandler02(ExCommand *ex) {
	int res = 0;

	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_SC2_LADDERCLICK:
		sceneHandler02_ladderClick();
		return 0;

	case MSG_SC2_SHOWLADDER:
		sceneHandler02_showLadder();
		return 0;

	case MSG_SC2_PUTMANUP:
		g_fp->_aniMan2->_priority = 0;
		return 0;

	case MSG_SC2_HIDELADDER:
		sceneHandler02_hideLadder();
		return 0;

	case 33:
		if (g_fp->_aniMan2) {
			if (g_fp->_aniMan2->_ox < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.left - 300;

			if (g_fp->_aniMan2->_ox > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.right + 300;

			res = 1;
		}

		if (g_vars->scene02_boxOpen) {
			if (g_vars->scene02_boxDelay >= 1) {
				--g_vars->scene02_boxDelay;
			} else if (g_fp->_floaters->_array2.size() >= 1) {
				if (g_fp->_floaters->_array2[0].val5 == -50) {
					g_fp->_floaters->stopAll();
					g_vars->scene02_boxOpen = false;
					g_vars->scene02_boxDelay = 100 * g_fp->_rnd.getRandomNumber(32767) + 150;
				} else {
					g_fp->_floaters->_array2[0].val3 = -50;
				}
			} else {
				g_fp->_floaters->genFlies(g_fp->_currentScene, g_fp->_rnd.getRandomNumber(700) + 100, -50, 0, 0);
				g_vars->scene02_boxDelay = 500 * g_fp->_rnd.getRandomNumber(32767) + 1000;
			}
		}

		g_fp->_floaters->update();
		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();
	}

	return res;
}

} // End of namespace Fullpipe
