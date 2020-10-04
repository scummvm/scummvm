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


namespace NGI {

void scene15_initScene(Scene *sc) {
	g_vars->scene15_chantingCountdown = 0;

	StaticANIObject *grandma = sc->getStaticANIObject1ById(ANI_GRANDMA_ASS, -1);

	Scene *oldsc = g_nmi->_currentScene;
	g_nmi->_currentScene = sc;

	int grandmaState = g_nmi->getObjectState(sO_Grandma);

	if (grandmaState == g_nmi->getObjectEnumState(sO_Grandma, sO_In_15)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_nmi->getObjectEnumState(sO_Grandma, sO_In_15_1)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(86, 399);
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_nmi->getObjectEnumState(sO_Grandma, sO_In_15_2)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(71, 399);
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_nmi->getObjectEnumState(sO_Grandma, sO_In_15_3)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(49, 399);
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_nmi->getObjectEnumState(sO_Grandma, sO_WithoutBoot)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		grandma->changeStatics2(ST_GMS_BOOTLESS2);
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else {
		grandma->hide();
		g_nmi->setObjectState(sO_LeftPipe_15, g_nmi->getObjectEnumState(sO_LeftPipe_15, sO_IsOpened));
	}

	g_vars->scene15_plusminus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_nmi->getObjectState(sO_Guard_2) == g_nmi->getObjectEnumState(sO_Guard_2, sO_Off))
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_PLUS);

 	g_vars->scene15_ladder = sc->getPictureObjectById(PIC_SC15_LADDER, 0);
	g_vars->scene15_boot = sc->getStaticANIObject1ById(ANI_BOOT_15, -1);

	if (g_nmi->getObjectState(sO_Boot_15) != g_nmi->getObjectEnumState(sO_Boot_15, sO_IsPresent))
		g_vars->scene15_boot->_flags &= 0xFFFB;

	g_nmi->_currentScene = oldsc;

	g_nmi->lift_setButton(sO_Level5, ST_LBN_5N);
	g_nmi->lift_init(sc, QU_SC15_ENTERLIFT, QU_SC15_EXITLIFT);
}

int scene15_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_cursorId == PIC_CSR_ITN && g_nmi->_objectIdAtCursor == PIC_SC15_LTRUBA)
		g_nmi->_cursorId = PIC_CSR_GOL;

	return g_nmi->_cursorId;
}

int sceneHandler15(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_SC4_HIDEBOOT:
		g_vars->scene15_boot->_flags &= 0xFFFB;
		break;

	case MSG_SC15_STOPCHANTING:
		g_nmi->stopAllSoundInstances(SND_15_001);

		g_vars->scene15_chantingCountdown = 120;
		break;

	case MSG_SC15_ASSDRYG:
		if (g_nmi->_rnd.getRandomNumber(1)) {
			g_nmi->playSound(SND_15_011, 0);
		} else {
			g_nmi->playSound(SND_15_006, 0);
		}
		break;

	case MSG_SC15_LADDERTOBACK:
		g_vars->scene15_ladder->_priority = 60;
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SC15_PULL:
		if (g_vars->scene15_plusminus->_statics->_staticsId == ST_PMS_MINUS) {
			g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_PLUS);
		} else {
			g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_MINUS);
		}
		break;

	case 64:
		g_nmi->lift_hoverButton(cmd);
		break;

	case 29:
		{
			if (g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC15_LADDER) {
				handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->getPictureObjectById(PIC_SC15_DTRUBA, 0), cmd->_param);
				cmd->_messageKind = 0;

				return 0;
			}

			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);

				cmd->_messageKind = 0;
			}
		}
		break;

	case 30:
		// nop
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		if (g_vars->scene15_chantingCountdown > 0) {
			g_vars->scene15_chantingCountdown--;

			if (!g_vars->scene15_chantingCountdown)
				g_nmi->playSound(SND_15_001, 1);
		}

		g_nmi->_behaviorManager->updateBehaviors();
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
