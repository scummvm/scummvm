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

void scene15_initScene(Scene *sc) {
	g_vars->scene15_chantingCountdown = 0;

	StaticANIObject *grandma = sc->getStaticANIObject1ById(ANI_GRANDMA_ASS, -1);

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	int grandmaState = g_fp->getObjectState(sO_Grandma);

	if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_1)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(86, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_2)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(71, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_3)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(49, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_WithoutBoot)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		grandma->changeStatics2(ST_GMS_BOOTLESS2);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else {
		grandma->hide();
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsOpened));
	}

	g_vars->scene15_plusminus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_fp->getObjectState(sO_Guard_2) == g_fp->getObjectEnumState(sO_Guard_2, sO_Off))
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_PLUS);

 	g_vars->scene15_ladder = sc->getPictureObjectById(PIC_SC15_LADDER, 0);
	g_vars->scene15_boot = sc->getStaticANIObject1ById(ANI_BOOT_15, -1);

	if (g_fp->getObjectState(sO_Boot_15) != g_fp->getObjectEnumState(sO_Boot_15, sO_IsPresent))
		g_vars->scene15_boot->_flags &= 0xFFFB;

	g_fp->_currentScene = oldsc;

	g_fp->lift_setButton(sO_Level5, ST_LBN_5N);
	g_fp->lift_init(sc, QU_SC15_ENTERLIFT, QU_SC15_EXITLIFT);
}

int scene15_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_cursorId == PIC_CSR_ITN && g_fp->_objectIdAtCursor == PIC_SC15_LTRUBA)
		g_fp->_cursorId = PIC_CSR_GOL;

	return g_fp->_cursorId;
}

int sceneHandler15(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC4_HIDEBOOT:
		g_vars->scene15_boot->_flags &= 0xFFFB;
		break;

	case MSG_SC15_STOPCHANTING:
		g_fp->stopAllSoundInstances(SND_15_001);

		g_vars->scene15_chantingCountdown = 120;
		break;

	case MSG_SC15_ASSDRYG:
		if (g_fp->_rnd.getRandomNumber(1))
			g_fp->playSound(SND_15_011, 0);
		else
			g_fp->playSound(SND_15_006, 0);

		break;

	case MSG_SC15_LADDERTOBACK:
		g_vars->scene15_ladder->_priority = 60;
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_clickButton();
		break;

	case MSG_SC15_PULL:
		if (g_vars->scene15_plusminus->_statics->_staticsId == ST_PMS_MINUS)
			g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_PLUS);
		else
			g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_MINUS);

		break;

	case 64:
		g_fp->lift_hoverButton(cmd);
		break;

	case 29:
		{
			if (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC15_LADDER) {
				handleObjectInteraction(g_fp->_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC15_DTRUBA, 0), cmd->_param);
				cmd->_messageKind = 0;

				return 0;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_animateButton(ani);

				cmd->_messageKind = 0;
			}
			break;
		}

	case 30:
		// nop
		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene15_chantingCountdown > 0) {
			g_vars->scene15_chantingCountdown--;

			if (!g_vars->scene15_chantingCountdown)
				g_fp->playSound(SND_15_001, 1);
		}

		g_fp->_behaviorManager->updateBehaviors();
	}

	return 0;
}

} // End of namespace Fullpipe
