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

#include "ngi/constants.h"
#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/modal.h"
#include "ngi/statics.h"

namespace NGI {

int sceneIntro_updateCursor() {
	g_nmi->_cursorId = 0;

	return 0;
}

void sceneIntro_initScene(Scene *sc) {
	g_nmi->_gameLoader->loadScene(SC_INTRO2);

	g_vars->sceneIntro_aniin1man = sc->getStaticANIObject1ById(ANI_IN1MAN, -1);
	g_vars->sceneIntro_needSleep = true;
	g_vars->sceneIntro_needGetup = false;
	g_vars->sceneIntro_playing = true;
	g_vars->sceneIntro_needBlackout = false;

	if (g_nmi->_recordEvents || g_nmi->_inputArFlag)
		g_vars->sceneIntro_skipIntro = false;

	g_nmi->_modalObject = new ModalIntro;
}

void sceneHandlerIntro_part1() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_INTRO1);
	chainQueue(QU_INTR_FINISH, 0);
}

void sceneHandlerIntro_part2() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_INTRO2);
	chainQueue(QU_IN2_DO, 0);
}

int sceneHandlerIntro(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_INTR_ENDINTRO:
		g_vars->sceneIntro_playing = 0;
		return 0;

	case MSG_INTR_SWITCHTO1:
		sceneHandlerIntro_part1();
		return 0;

	case MSG_INTR_GETUPMAN:
		g_vars->sceneIntro_needSleep = 0;
		g_vars->sceneIntro_needGetup = 1;
		return 0;

	case MSG_INTR_SWITCHTO2:
		sceneHandlerIntro_part2();
		return 0;

	case 33:
		// fall through
		break;

	default:
		return 0;
	}

	if (g_vars->sceneIntro_needSleep) {
		if (!g_vars->sceneIntro_aniin1man->_movement && g_vars->sceneIntro_aniin1man->_statics->_staticsId == ST_IN1MAN_SLEEP)
			g_vars->sceneIntro_aniin1man->startAnim(MV_IN1MAN_SLEEP, 0, -1);
	} else if (g_vars->sceneIntro_needGetup && !g_vars->sceneIntro_aniin1man->_movement &&
				g_vars->sceneIntro_aniin1man->_statics->_staticsId == ST_IN1MAN_SLEEP) {
		g_vars->sceneIntro_needGetup = 0;

		chainQueue(QU_INTR_GETUPMAN, 0);
	}

	g_nmi->startSceneTrack();

	return 0;
}

} // End of namespace NGI
