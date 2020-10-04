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

int sceneIntroDemo_updateCursor() {
	g_nmi->_cursorId = 0;

	return 0;
}

void sceneIntroDemo_initScene(Scene *sc) {
	g_vars->sceneIntro_needSleep = true;
	g_vars->sceneIntro_needGetup = false;
	g_vars->sceneIntro_playing = true;
	g_vars->sceneIntro_needBlackout = false;

	if (g_nmi->_recordEvents || g_nmi->_inputArFlag)
		g_vars->sceneIntro_skipIntro = false;

	g_nmi->_modalObject = new ModalIntroDemo;
}

void sceneHandlerIntroDemo_part1() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_INTRO1);
	chainQueue(QU_INTR_FINISH, 0);
}

void sceneHandlerIntroDemo_part2() {
	chainQueue(QU_IN2_DO, 0);
}

int sceneHandlerIntroDemo(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_INTR_ENDINTRO:
		g_vars->sceneIntro_playing = 0;
		return 0;

	case MSG_INTR_SWITCHTO1:
		sceneHandlerIntroDemo_part1();
		return 0;

	case MSG_INTR_GETUPMAN:
		g_vars->sceneIntro_needSleep = 0;
		g_vars->sceneIntro_needGetup = 1;
		return 0;

	case MSG_INTR_SWITCHTO2:
		sceneHandlerIntroDemo_part2();
		return 0;

	case 33:
		g_nmi->startSceneTrack();
		break;

	default:
		return 0;
	}

	return 0;
}

} // End of namespace NGI
