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

#include "ngi/modal.h"

#include "audio/mixer.h"

namespace NGI {

void sceneFinal_initScene() {
	g_nmi->_gameLoader->loadScene(SC_FINAL2);
	g_nmi->accessScene(SC_FINAL2)->setPictureObjectsFlag4();
	g_nmi->_gameLoader->loadScene(SC_FINAL3);
	g_nmi->accessScene(SC_FINAL3)->setPictureObjectsFlag4();
	g_nmi->_gameLoader->loadScene(SC_FINAL4);
	g_nmi->accessScene(SC_FINAL4)->setPictureObjectsFlag4();

	getGameLoaderInventory()->setIsLocked(0);
	getGameLoaderInventory()->slideIn();

	g_nmi->_updateFlag = 0;
	g_nmi->_flgCanOpenMap = 0;

	g_vars->sceneFinal_var01 = 0;
	g_vars->sceneFinal_var02 = 0;
	g_vars->sceneFinal_var03 = 0;
	g_vars->sceneFinal_trackHasStarted = false;
}

int sceneFinal_updateCursor() {
	if (g_vars->sceneFinal_var01)
		g_nmi->_cursorId = 0;
	else
		g_nmi->updateCursorCommon();

	return g_nmi->_cursorId;
}

void sceneHandlerFinal_endFinal() {
	g_vars->sceneFinal_var01 = 0;
}

void sceneHandlerFinal_startMusic(const char *track) {
	g_nmi->startSoundStream1(track);
	g_vars->sceneFinal_trackHasStarted = true;
}

void sceneHandlerFinal_goto4() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_FINAL4);

	g_nmi->_gameLoader->loadScene(SC_FINAL4);

	chainQueue(QU_FN4_DOFINAL, 1);
}

void sceneHandlerFinal_goto3() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_FINAL3);

	chainQueue(QU_FN3_DOFINAL, 1);
}

void sceneHandlerFinal_goto2() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_FINAL2);

	chainQueue(QU_FN2_DOFINAL, 1);
}

void sceneHandlerFinal_startFinal() {
	g_vars->sceneFinal_var01 = 1;

	getCurrSceneSc2MotionController()->deactivate();
	getGameLoaderInteractionController()->disableFlag24();

	g_nmi->_aniMan2 = 0;

	g_nmi->_aniMan->_flags &= 0xFFFB;

	chainQueue(QU_FIN1_TAKECOIN, 1);

	g_nmi->playTrack(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_FINAL1"), "MUSIC2", 1);

	g_nmi->_modalObject = new ModalFinal;
}

void sceneHandlerFinal_fallCoin() {
	StaticANIObject *coin = g_nmi->_currentScene->getStaticANIObject1ById(ANI_FIN_COIN, -1);

	if (!coin->_movement) {
		if (!coin->_statics || coin->_statics->_staticsId != ST_FCN_NORM)
			chainQueue(QU_FIN1_FALLCOIN, 1);
	}
}

void updateMusic() {
	if (g_vars->sceneFinal_trackHasStarted && !g_nmi->_mixer->isSoundHandleActive(g_nmi->_soundStream1)) { // loop music
		sceneHandlerFinal_startMusic("track16.ogg");
	}
}

int sceneHandlerFinal(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_FIN_ENDFINAL:
		sceneHandlerFinal_endFinal();
		break;

	case MSG_FN4_STARTMUSIC:
		sceneHandlerFinal_startMusic("track16.ogg");
		break;

	case MSG_FIN_GOTO4:
		sceneHandlerFinal_goto4();

		g_nmi->playTrack(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_FINAL1"), "MUSIC3", 1);
		break;

	case MSG_FIN_GOTO3:
		sceneHandlerFinal_goto3();
		break;

	case MSG_FIN_GOTO2:
		sceneHandlerFinal_goto2();
		break;

	case MSG_FIN_STARTFINAL:
		sceneHandlerFinal_startFinal();
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			g_vars->sceneFinal_var03 = g_nmi->_aniMan2->_ox;

			if (g_vars->sceneFinal_var03 < 450 && g_vars->sceneFinal_var02 >= 450 )
				sceneHandlerFinal_fallCoin();

			g_vars->sceneFinal_var02 = g_vars->sceneFinal_var03;
		}

		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	updateMusic();

	return 0;
}

} // End of namespace NGI
