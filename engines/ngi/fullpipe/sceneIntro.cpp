/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ngi/ngi.h"

#include "ngi/constants.h"
#include "ngi/behavior.h"
#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/modal.h"
#include "ngi/statics.h"

namespace NGI {

void sceneIntro_initMan(Scene &scene) {
	g_vars->sceneIntro_aniin1man = scene.findAniObjectById(ANI_IN1MAN, -1);
	g_vars->sceneIntro_needSleep = true;
	g_vars->sceneIntro_needGetup = false;
	g_vars->sceneIntro_playing = true;
	g_vars->sceneIntro_fader.start(1025, 0, 0);
}

static bool sceneIntro_updateMan() {
	AniObject *man = g_vars->sceneIntro_aniin1man;
	if (!man || man->_movement || !man->_statics || man->_statics->_staticsId != ST_IN1MAN_SLEEP)
		return false;

	if (g_vars->sceneIntro_needSleep)
		return man->startMovement(MV_IN1MAN_SLEEP, 0, -1);
	if (g_vars->sceneIntro_needGetup) {
		g_vars->sceneIntro_needGetup = false;
		return runQueue(QU_INTR_GETUPMAN, 0);
	}
	return false;
}

bool sceneIntro_updateAndroid(uint32 elapsed, byte &brightness) {
	if (!g_vars->sceneIntro_playing)
		return false;

	Fader &fader = g_vars->sceneIntro_fader;
	if (fader.active()) {
		fader.update(elapsed);
		const bool changed = brightness != fader.brightness();
		brightness = fader.brightness();
		if (!fader.active())
			runQueue(QU_INTR_STARTINTRO, 0);
		return changed;
	}

	return sceneIntro_updateMan();
}

int sceneIntro_updateCursor() {
	g_nmi->_cursorId = 0;

	return 0;
}

void sceneIntro_initScene(Scene *sc) {
	g_nmi->_gameLoader->loadScene(SC_INTRO2);

	sceneIntro_initMan(*sc);
	g_vars->sceneIntro_needBlackout = false;

	if (g_nmi->_recordEvents || g_nmi->_inputArFlag)
		g_vars->sceneIntro_skipIntro = false;

	g_nmi->_modalObject = new ModalIntro;
}

void sceneHandlerIntro_part1() {
	g_nmi->_currentScene = g_nmi->findScene(SC_INTRO1);
	runQueue(QU_INTR_FINISH, 0);
}

void sceneHandlerIntro_part2() {
	g_nmi->_currentScene = g_nmi->findScene(SC_INTRO2);
	runQueue(QU_IN2_DO, 0);
}

int sceneHandlerIntro(Command *ex) {
	if (ex->_cmd != 17)
		return 0;

	if (g_nmi->getPlatform() == Common::kPlatformAndroid && !ex->_messageName.empty()) {
		debugC(3, kDebugEvents, "Android intro message %s", ex->_messageName.c_str());
		if (ex->_messageName == "MSG_ENABLE_BHV_SOUND") {
			if (g_nmi->_currentScene->_behaviorController)
				g_nmi->_currentScene->_behaviorController->enableAllMusic();
			return 0;
		}
	}

	switch (ex->_messageNum) {
	case MSG_INTR_ENDINTRO:
		if (g_vars->sceneIntro_playing && g_nmi->getPlatform() == Common::kPlatformAndroid)
			sceneChapter_show("CHAPTER_1");
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

	sceneIntro_updateMan();

	g_nmi->startSceneTrack();

	return 0;
}

} // End of namespace NGI
