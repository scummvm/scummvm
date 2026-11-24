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

#include "ngi/objectnames.h"
#include "ngi/constants.h"

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "ngi/interaction.h"
#include "ngi/behavior.h"


namespace NGI {

void scene01_fixEntrance() {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");
	if (var->getSubVarAsInt("Entrance") == TrubaLeft)
		var->setSubVarAsInt("Entrance", TrubaRight);
}

void scene01_initObjects(Scene *sc, int entrance, bool eggUncracked) {
	g_vars->scene01_picSc01Osk = sc->findPictureObjectById(PIC_SC1_OSK, 0);
	g_vars->scene01_picSc01Osk->_flags &= 0xFFFB;

	g_vars->scene01_picSc01Osk2 = sc->findPictureObjectById(PIC_SC1_OSK2, 0);
	g_vars->scene01_picSc01Osk2->_flags &= 0xFFFB;

	if (eggUncracked) {
		PictureObject *pic = sc->findPictureObjectById(PIC_SC1_KUCHKA, 0);
		if (pic)
			pic->_flags &= 0xFFFB;
	}

	if (entrance != TrubaLeft) {
		AniObject *bootAnim = sc->findAniObjectById(ANI_BOOT_1, -1);
		if (bootAnim)
			bootAnim->_flags &= ~0x04;
	}
}

void scene01_initScene(Scene *sc, int entrance) {
	scene01_initObjects(sc, entrance, g_nmi->getStateId(sO_EggCracker) == g_nmi->getStateValue(sO_EggCracker, sO_DidNotCrackEgg));
	g_nmi->lift_setButton(sO_Level2, ST_LBN_2N);
}

int sceneHandler01(Command *cmd) {
	int res = 0;

	if (cmd->_cmd != 17)
		return 0;

	if (!cmd->_messageNum && !cmd->_messageName.empty()) {
		if (cmd->_messageName == "MSG_ENABLE_BHV_SOUND") {
			if (g_nmi->_currentScene->_behaviorController)
				g_nmi->_currentScene->_behaviorController->enableAllMusic();
			return 0;
		}
		warning("Android scene 1 message %s not implemented", cmd->_messageName.c_str());
		return 0;
	}

	if (cmd->_messageNum > MSG_SC1_SHOWOSK) {
		if (cmd->_messageNum == MSG_SC1_UTRUBACLICK)
			handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->findPictureObjectById(PIC_SC1_LADDER, 0), 0);

		return 0;
	}

	if (cmd->_messageNum == MSG_SC1_SHOWOSK) {
		g_vars->scene01_picSc01Osk->_flags |= 4;

		g_vars->scene01_picSc01Osk->_z = 20;
		g_vars->scene01_picSc01Osk2->_z = 21;

		return 0;
	}

	if (cmd->_messageNum != 0x21) {
		if (cmd->_messageNum == MSG_SC1_SHOWOSK2) {
			g_vars->scene01_picSc01Osk2->_flags |= 4;
			g_vars->scene01_picSc01Osk2->_z = 20;
			g_vars->scene01_picSc01Osk->_z = 21;

			return 0;
		}

		return 0;
	}

	if (g_nmi->_aniMan2) {
		if (g_nmi->_aniMan2->_x < g_nmi->_sceneRect.left + 200) {
			g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.left - 300;
		}

		if (g_nmi->_aniMan2->_x > g_nmi->_sceneRect.right - 200)
			g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.right + 300;

		res = 1;
	}
	g_nmi->_behaviorManager->update();

	g_nmi->startSceneTrack();

	return res;
}

} // End of namespace NGI
