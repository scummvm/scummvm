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

void scene01_fixEntrance() {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");
	if (var->getSubVarAsInt("Entrance") == TrubaLeft)
		var->setSubVarAsInt("Entrance", TrubaRight);
}

void scene01_initScene(Scene *sc, int entrance) {
	g_vars->scene01_picSc01Osk = sc->getPictureObjectById(PIC_SC1_OSK, 0);
	g_vars->scene01_picSc01Osk->_flags &= 0xFFFB;

	g_vars->scene01_picSc01Osk2 = sc->getPictureObjectById(PIC_SC1_OSK2, 0);
	g_vars->scene01_picSc01Osk2->_flags &= 0xFFFB;

	if (g_fp->getObjectState(sO_EggCracker) == g_fp->getObjectEnumState(sO_EggCracker, sO_DidNotCrackEgg)) {
		PictureObject *pic = sc->getPictureObjectById(PIC_SC1_KUCHKA, 0);
		if (pic)
			pic->_flags &= 0xFFFB;
	}

	if (entrance != TrubaLeft) {
		StaticANIObject *bootAnim = sc->getStaticANIObject1ById(ANI_BOOT_1, -1);
		if (bootAnim)
			bootAnim->_flags &= ~0x04;
	}

	g_fp->lift_setButton(sO_Level2, ST_LBN_2N);
}

int sceneHandler01(ExCommand *cmd) {
	int res = 0;

	if (cmd->_messageKind != 17)
		return 0;

	if (cmd->_messageNum > MSG_SC1_SHOWOSK) {
		if (cmd->_messageNum == MSG_SC1_UTRUBACLICK)
			handleObjectInteraction(g_fp->_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC1_LADDER, 0), 0);

		return 0;
	}

	if (cmd->_messageNum == MSG_SC1_SHOWOSK) {
		g_vars->scene01_picSc01Osk->_flags |= 4;

		g_vars->scene01_picSc01Osk->_priority = 20;
		g_vars->scene01_picSc01Osk2->_priority = 21;

		return 0;
	}

	if (cmd->_messageNum != 0x21) {
		if (cmd->_messageNum == MSG_SC1_SHOWOSK2) {
			g_vars->scene01_picSc01Osk2->_flags |= 4;
			g_vars->scene01_picSc01Osk2->_priority = 20;
			g_vars->scene01_picSc01Osk->_priority = 21;

			return 0;
		}

		return 0;
	}

	if (g_fp->_aniMan2) {
		if (g_fp->_aniMan2->_ox < g_fp->_sceneRect.left + 200) {
			g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.left - 300;
		}

		if (g_fp->_aniMan2->_ox > g_fp->_sceneRect.right - 200)
			g_fp->_currentScene->_x = g_fp->_aniMan2->_ox - g_fp->_sceneRect.right + 300;

		res = 1;
	}
	g_fp->_behaviorManager->updateBehaviors();

	g_fp->startSceneTrack();

	return res;
}

} // End of namespace Fullpipe
