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

void sceneHandler34_setExits() {
	warning("STUB: sceneHandler34_setExits()");
}

void scene34_initScene(Scene *sc) {
	g_vars->scene34_var01 = 200;
	g_vars->scene34_var02 = 200;
	g_vars->scene34_var03 = 300;
	g_vars->scene34_var04 = 300;
	g_vars->scene34_cactus = sc->getStaticANIObject1ById(ANI_CACTUS_34, -1);
	g_vars->scene34_vent = sc->getStaticANIObject1ById(ANI_VENT_34, -1);
	g_vars->scene34_hatch = sc->getStaticANIObject1ById(ANI_LUK_34, -1);
	g_vars->scene34_boot = sc->getStaticANIObject1ById(ANI_BOOT_34, -1);

	if (g_fp->getObjectState(sO_Cactus) == g_fp->getObjectEnumState(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_fp->_currentScene;

		g_fp->_currentScene = sc;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_EMPTY);
		g_vars->scene34_cactus->setOXY(506, 674);
		g_vars->scene34_cactus->_priority = 30;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_GROWNEMPTY2);
		g_vars->scene34_cactus->_flags |= 4;

		g_fp->_currentScene = oldsc;
	}

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_Strolling))
		g_fp->setObjectState(sO_Grandma, g_fp->getObjectEnumState(sO_Grandma, sO_OnStool));

	sceneHandler34_setExits();

	g_vars->scene34_var05 = 0;
	g_vars->scene34_var06 = 0;
	g_vars->scene34_var07 = 0;
	g_vars->scene34_var08 = g_fp->_rnd->getRandomNumber(500) + 500;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_34"));

	g_fp->lift_setButton(sO_Level7, ST_LBN_7N);
	g_fp->lift_sub5(sc, QU_SC34_ENTERLIFT, QU_SC34_EXITLIFT);

	g_fp->initArcadeKeys("SC_34");
}

void scene34_initBeh() {
	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLEFT, 0);
	g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLRIGHT, 0);
}

int scene34_updateCursor() {
	g_fp->updateCursorCommon();

	if ((g_fp->_objectIdAtCursor != ANI_STOOL_34 || getGameLoaderInventory()->getSelectedItemId() != ANI_INV_BOX)
		 && (g_fp->_objectIdAtCursor != ANI_BOX_34 || getGameLoaderInventory()->getSelectedItemId() != ANI_INV_STOOL))
		; // emtpy
	else
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

void sceneHandler34_leaveBoard() {
	warning("STUB: sceneHandler34_leaveBoard()");
}

void sceneHandler34_onBoard() {
	warning("STUB: sceneHandler34_onBoard()");
}

void sceneHandler34_testVent() {
	warning("STUB: sceneHandler34_testVent()");
}

void sceneHandler34_hideStool() {
	warning("STUB: sceneHandler34_hideStool()");
}

void sceneHandler34_climb() {
	warning("STUB: sceneHandler34_climb()");
}

void sceneHandler34_sub04() {
	warning("STUB: sceneHandler34_sub04()");
}

void sceneHandler34_sub03(ExCommand *cmd) {
	warning("STUB: sceneHandler34_sub03(cmd)");
}

void sceneHandler34_sub02(ExCommand *cmd) {
	warning("STUB: sceneHandler34_sub02(cmd)");
}

void sceneHandler34_sub01(ExCommand *cmd) {
	warning("STUB: sceneHandler34_sub01(cmd)");
}

void sceneHandler34_showVent() {
	warning("STUB: sceneHandler34_showVent()");
}

void sceneHandler34_showBox() {
	warning("STUB: sceneHandler34_showBox()");
}

void sceneHandler34_showStool() {
	warning("STUB: sceneHandler34_showStool()");
}

void sceneHandler34_unclimb() {
	warning("STUB: sceneHandler34_unclimb()");
}

int sceneHandler34(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC4_HIDEBOOT:
		g_vars->scene34_boot->_flags &= 0xFFFB;
		break;

	case MSG_SC34_LEAVEBOARD:
		sceneHandler34_leaveBoard();
		break;

	case MSG_SC34_ONBOARD:
		sceneHandler34_onBoard();
		break;

	case MSG_SC34_TESTVENT:
		sceneHandler34_testVent();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_animation3();
		break;

	case MSG_SC34_FROMCACTUS:
		g_vars->scene34_var07 = 0;

		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();

		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);

		break;

	case MSG_SC34_RETRYVENT:
		if (!g_fp->_aniMan->isIdle())
			break;

		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);
		g_fp->_aniMan->_flags &= 0xFEFF;

		getGameLoaderInteractionController()->handleInteraction(g_fp->_aniMan, g_vars->scene34_vent, cmd->_keyCode);

		break;

	case MSG_SC34_ONBUMP:
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLEFT, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene34_cactus, ST_CTS34_GROWNEMPTY2, QU_CTS34_FALLRIGHT, 1);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC22_HIDESTOOL:
		sceneHandler34_hideStool();
		break;

	case MSG_SC34_CLIMB:
		sceneHandler34_climb();
		break;

	case MSG_SC34_UNCLIMB:
		sceneHandler34_unclimb();
		break;

	case MSG_SC22_SHOWSTOOL:
		sceneHandler34_showStool();
		break;

	case MSG_SC34_SHOWBOX:
		sceneHandler34_showBox();
		break;

	case MSG_SC34_ONCACTUS:
		g_vars->scene34_var07 = 1;

		getCurrSceneSc2MotionController()->clearEnabled();
		getGameLoaderInteractionController()->disableFlag24();

		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);
		break;

	case MSG_SC34_SHOWVENT:
		sceneHandler34_showVent();
		break;

	case 64:
		g_fp->lift_sub05(cmd);
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case 29:
		{
			if (g_vars->scene34_var05) {
				sceneHandler34_sub01(cmd);
				break;
			}

			if (g_vars->scene34_var06) {
				sceneHandler34_sub02(cmd);
				break;
			}

			if (g_vars->scene34_var07) {
				sceneHandler34_sub03(cmd);
				break;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (ani) {
				if ((ani->_id == ANI_STOOL_34 && cmd->_keyCode == ANI_INV_BOX) || (ani->_id == ANI_BOX_34 && cmd->_keyCode == ANI_INV_STOOL)) {
					getGameLoaderInteractionController()->handleInteraction(g_fp->_aniMan, g_vars->scene34_vent, cmd->_keyCode);

					cmd->_messageKind = 0;
				}

				if (ani->_id == ANI_LIFTBUTTON) {
					g_fp->lift_sub1(ani);

					cmd->_messageKind = 0;

					break;
				}
			}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1) || (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						break;
					}
				}
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene34_var01)
				g_fp->_currentScene->_x = x - g_vars->scene34_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene34_var01)
				g_fp->_currentScene->_x = x + g_vars->scene34_var03 - g_fp->_sceneRect.right;
		}

		--g_vars->scene34_var08;

		if (!g_vars->scene34_var08)
			sceneHandler34_sub04();

		g_fp->_floaters->update();

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
