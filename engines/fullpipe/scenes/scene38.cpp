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


namespace Fullpipe {

void scene38_setBottleState(Scene *sc) {
	ExCommand *ex = sc->getMessageQueueById(QU_SC38_SHOWBOTTLE_ONTABLE)->getExCommandByIndex(0);

	if (g_vars->scene38_bottle->_ox == ex->_x && g_vars->scene38_bottle->_oy == ex->_y) {
		if (g_fp->lift_checkButton(sO_Level5) ) {
			ex = sc->getMessageQueueById(QU_SC38_SHOWBOTTLE)->getExCommandByIndex(0);

			g_vars->scene38_bottle->setOXY(ex->_x, ex->_y);
			g_vars->scene38_bottle->_priority = ex->_field_14;

			g_fp->setObjectState(sO_Bottle_38, g_fp->getObjectEnumState(sO_Bottle_38, sO_Blocked));
		}
	}
}

void scene38_initScene(Scene *sc) {
	g_vars->scene38_var01 = 200;
	g_vars->scene38_var02 = 200;
	g_vars->scene38_var03 = 300;
	g_vars->scene38_var04 = 300;
	g_vars->scene38_boss = sc->getStaticANIObject1ById(ANI_GLAVAR, -1);
	g_vars->scene38_tally = sc->getStaticANIObject1ById(ANI_DYLDA, -1);
	g_vars->scene38_shorty = sc->getStaticANIObject1ById(ANI_MALYSH, -1);
	g_vars->scene38_domino0 = sc->getStaticANIObject1ById(ANI_DOMINO38, 0);
	g_vars->scene38_dominos = sc->getStaticANIObject1ById(ANI_DOMINOS, 0);
	g_vars->scene38_domino1 = sc->getStaticANIObject1ById(ANI_DOMINO38, 1);
	g_vars->scene38_bottle = sc->getStaticANIObject1ById(ANI_BOTTLE38, 0);
	g_vars->scene38_var05 = 0;
	g_vars->scene38_var06 = 0;
	g_vars->scene38_var07 = 0;
	g_vars->scene38_var08 = 15;
	g_vars->scene38_var09 = 0;
	g_vars->scene38_var10 = 0;
	g_vars->scene38_var11 = 30;
	g_vars->scene38_var12 = 0;
	g_vars->scene38_var13 = 0;

	scene38_setBottleState(sc);

	if (g_fp->getObjectState(sO_Boss) == g_fp->getObjectEnumState(sO_Boss, sO_IsSleeping)) {
		g_vars->scene38_shorty->_flags &= 0xFFFB;

		g_vars->scene38_tally->stopAnim_maybe();
		g_vars->scene38_tally->_flags &= 0xFFFB;

		g_vars->scene38_domino0->_flags &= 0xFFFB;
		g_vars->scene38_dominos->_flags &= 0xFFFB;
		g_vars->scene38_domino1->_flags &= 0xFFFB;
	}

	g_fp->lift_sub5(sc, QU_SC38_ENTERLIFT, QU_SC38_EXITLIFT);
	g_fp->lift_sub7(sc, ST_LBN_0N);
}

void sceneHandler38_tryTakeBottle() {
	g_vars->scene38_boss->changeStatics2(ST_GLV_NOHAMMER);
	g_vars->scene38_boss->startAnim(MV_GLV_LOOKMAN, 0, -1);

	g_vars->scene38_var05 = 0;
}

void sceneHandler38_postHammerKick() {
	g_vars->scene38_domino1->setOXY(g_vars->scene38_domino1->_ox, g_vars->scene38_domino1->_oy + 2);
}

void sceneHandler38_propose() {
	warning("STUB: sceneHandler38_propose()");
}

void sceneHandler38_point() {
	warning("STUB: sceneHandler38_point()");
}

void sceneHandler38_hammerKick() {
	warning("STUB: sceneHandler38_hammerKick()");
}

void sceneHandler38_drink() {
	if (!g_vars->scene38_shorty->_movement) {
		if (g_vars->scene38_shorty->_flags & 4) {
			if (!(g_vars->scene38_shorty->_flags & 2) && g_vars->scene38_var11 > 0
				&& g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_LEFT2
				&& g_fp->_rnd->getRandomNumber(32767) < 3276) {
				chainQueue(QU_MLS_TURNR, 0);
				g_vars->scene38_var11 = 0;
			}
		}
	}
}

void sceneHandler38_animateAlcoholics() {
	warning("STUB: sceneHandler38_animateAlcoholics()");
}

int sceneHandler38(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC38_TRYTAKEBOTTLE:
		sceneHandler38_tryTakeBottle();
		break;

	case MSG_SC38_POSTHMRKICK:
		sceneHandler38_postHammerKick();
		break;

	case MSG_SC38_PROPOSE:
		sceneHandler38_propose();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_animation3();
		break;

	case MSG_SC38_POINT:
		sceneHandler38_point();
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC38_HMRKICK:
		sceneHandler38_hammerKick();
		break;

	case MSG_SC38_DRINK:
		sceneHandler38_drink();
		break;

	case 64:
		g_fp->lift_sub05(cmd);
		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_sub1(ani);

				cmd->_messageKind = 0;
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene38_var01)
				g_fp->_currentScene->_x = x - g_vars->scene38_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene38_var01 )
				g_fp->_currentScene->_x = x + g_vars->scene38_var03 - g_fp->_sceneRect.right;
		}

		sceneHandler38_animateAlcoholics();

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
