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

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/gfx.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/messages.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/behavior.h"
#include "fullpipe/motion.h"
#include "fullpipe/interaction.h"

namespace Fullpipe {

void scene06_initMumsy() {
	g_vars->scene06_mumsyJumpFw = g_fullpipe->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene06_mumsy, ST_MOM_STANDS, QU_MOM_JUMPFW);
	g_vars->scene06_mumsyJumpBk = g_fullpipe->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene06_mumsy, ST_MOM_STANDS, QU_MOM_JUMPBK);
	g_vars->scene06_mumsyJumpFwPercent = g_vars->scene06_mumsyJumpFw->_percent;
	g_vars->scene06_mumsyJumpBkPercent = g_vars->scene06_mumsyJumpBk->_percent;
}

int scene06_updateCursor() {
	g_fullpipe->updateCursorCommon();

	if (g_vars->scene06_var07) {
		if (g_vars->scene06_var08) {
			g_fullpipe->_cursorId = PIC_CSR_ARCADE2_D;

			return PIC_CSR_ARCADE2_D;
		}
		if (g_fullpipe->_aniMan == (StaticANIObject *)g_fullpipe->_objectAtCursor) {
			if (g_fullpipe->_aniMan->_statics->_staticsId == ST_MAN6_BALL && g_fullpipe->_cursorId == PIC_CSR_DEFAULT) {
				g_fullpipe->_cursorId = PIC_CSR_ITN;

				return PIC_CSR_ITN;
			}
		} else if (g_fullpipe->_objectAtCursor && (StaticANIObject *)g_fullpipe->_objectAtCursor == g_vars->scene06_var09
				   && g_fullpipe->_cursorId == PIC_CSR_DEFAULT) {
			g_fullpipe->_cursorId = PIC_CSR_ITN;
		}
	}

	return g_fullpipe->_cursorId;
}

void sceneHandler06_setExits(Scene *sc) {
	warning("STUB: sceneHandler06_setExits()");
}

void sceneHandler06_winArcade() {
	warning("STUB: sceneHandler06_winArcade()");
}

void sceneHandler06_enableDrops() {
	warning("STUB: sceneHandler06_enableDrops()");
}

void sceneHandler06_sub01() {
	warning("STUB: sceneHandler06_sub01()");
}

void sceneHandler06_spinHandle() {
	warning("STUB: sceneHandler06_spinHandle()");
}

void sceneHandler06_uPipeClick() {
	warning("STUB: sceneHandler06_uPipeClick()");
}

void sceneHandler06_buttonPush() {
	warning("STUB: sceneHandler06_buttonPush()");
}

void sceneHandler06_showNextBall() {
	warning("STUB: sceneHandler06_showNextBall()");
}

void sceneHandler06_installHandle() {
	warning("STUB: sceneHandler06_installHandle()");
}

void sceneHandler06_takeBall() {
	warning("STUB: sceneHandler06_takeBall()");
}

void sceneHandler06_sub02() {
	warning("STUB: sceneHandler06_sub02()");
}

void sceneHandler06_throwBall() {
	warning("STUB: sceneHandler06_throwBall()");
}

void sceneHandler06_sub03() {
	warning("STUB: sceneHandler06_sub03()");
}

void sceneHandler06_sub04(int par) {
	warning("STUB: sceneHandler06_sub04()");
}

void sceneHandler06_sub05() {
	warning("STUB: sceneHandler06_sub05()");
}

void sceneHandler06_sub06() {
	warning("STUB: sceneHandler06_sub06()");
}

void sceneHandler06_sub07() {
	warning("STUB: sceneHandler06_sub07()");
}

void sceneHandler06_sub08() {
	warning("STUB: sceneHandler06_sub08()");
}

void sceneHandler06_sub09() {
	warning("STUB: sceneHandler06_sub09()");
}

void sceneHandler06_sub10() {
	warning("STUB: sceneHandler06_sub10()");
}


void scene06_initScene(Scene *sc) {
	g_vars->scene06_mumsy = sc->getStaticANIObject1ById(ANI_MAMASHA, -1);
	g_vars->scene06_var06 = 0;
	g_vars->scene06_invHandle = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
	g_vars->scene06_liftButton = sc->getStaticANIObject1ById(ANI_BUTTON_6, -1);
	g_vars->scene06_ballDrop = sc->getStaticANIObject1ById(ANI_BALLDROP, -1);
	g_vars->scene06_var07 = 0;
	g_vars->scene06_var08 = 0;
	g_vars->scene06_var09 = 0;
	g_vars->scene06_var10 = 0;
	g_vars->scene06_var11 = 0;
	g_vars->scene06_balls.clear();
	g_vars->scene06_var12 = 0;
	g_vars->scene06_var13 = 0;
	g_vars->scene06_var14 = 0;
	g_vars->scene06_var15 = 1;

	StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_NEWBALL, -1);

	ball->hide();
	ball->_statics = ball->getStaticsById(ST_NBL_NORM);
	g_vars->scene06_balls.push_back(ball);

	for (int i = 0; i < 3; i++) {
		StaticANIObject *ball2 = new StaticANIObject(ball);

		ball2->hide();
		ball2->_statics = ball2->getStaticsById(ST_NBL_NORM);

		sc->addStaticANIObject(ball2, 1);

		g_vars->scene06_balls.push_back(ball2);
	}

	if (g_fullpipe->getObjectState(sO_BigMumsy) == g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsPlaying))
		g_fullpipe->setObjectState(sO_BigMumsy, g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsSleeping));

	if (g_fullpipe->getObjectState(sO_BigMumsy) != g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsSleeping))
		g_vars->scene06_mumsy->hide();

	g_fullpipe->lift_setButton(sO_Level3, ST_LBN_3N);
	g_fullpipe->lift_sub5(sc, QU_SC6_ENTERLIFT, QU_SC6_EXITLIFT);
	g_fullpipe->initArcadeKeys("SC_6");

	sceneHandler06_setExits(sc);

	g_fullpipe->setArcadeOverlay(PIC_CSR_ARCADE2);
}

int sceneHandler06(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch(ex->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_fullpipe->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fullpipe->lift_exitSeq(ex);
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler06_winArcade();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fullpipe->lift_startExitQueue();
		break;

	case MSG_SC6_RESTORESCROLL:
		g_fullpipe->_aniMan2 = g_fullpipe->_aniMan;
		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();
		sceneHandler06_setExits(g_fullpipe->_currentScene);
		break;

	case MSG_SC6_STARTDROPS:
		if (g_fullpipe->getObjectState(sO_BigMumsy) == g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsSleeping))
			sceneHandler06_enableDrops();
		break;

	case MSG_SC6_TESTNUMBALLS:
		g_vars->scene06_var16 = 0;

		if (g_vars->scene06_var13 < 5 || !g_vars->scene06_var07)
			return 0;

		sceneHandler06_sub01();
		break;

	case MSG_SC6_JUMPFW:
		++g_vars->scene06_mumsyPos;
		break;

	case MSG_SC6_JUMPBK:
		--g_vars->scene06_mumsyPos;
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fullpipe->lift_animation3();
		break;

	case MSG_SPINHANDLE:
		sceneHandler06_spinHandle();
		break;

	case MSG_LIFT_GO:
		g_fullpipe->lift_goAnimation();
		break;

	case MSG_SC6_UTRUBACLICK:
		sceneHandler06_uPipeClick();
		break;

	case MSG_SC6_BTNPUSH:
		sceneHandler06_buttonPush();
		break;

	case MSG_SC6_SHOWNEXTBALL:
		sceneHandler06_showNextBall();
		break;

	case MSG_SC6_INSTHANDLE:
		sceneHandler06_installHandle();
		break;

	case MSG_SC6_ENABLEDROPS:
		sceneHandler06_enableDrops();
		break;

	case 64:
		g_fullpipe->lift_sub05(ex);
		break;

	case MSG_SC6_TAKEBALL:
		sceneHandler06_takeBall();
		break;

	case 30:
		if (g_vars->scene06_var08) {
			sceneHandler06_sub02();
			break;
		}

		if (!g_vars->scene06_var07) {
			// Do nothing
			break;
		}
		break;

	case 29:
		{
			StaticANIObject *st = g_fullpipe->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (st) {
				if (!g_vars->scene06_var07 && st->_id == ANI_LIFTBUTTON) {
					g_fullpipe->lift_sub1(st);
					ex->_messageKind = 0;
					return 0;
				}

				if (g_vars->scene06_var09 == st) {
					if (g_vars->scene06_var12 == 1)
						sceneHandler06_takeBall();

					ex->_messageKind = 0;
				} else if (g_vars->scene06_var10 && g_fullpipe->_aniMan == st && !g_fullpipe->_aniMan->_movement && g_fullpipe->_aniMan->_statics->_staticsId == ST_MAN6_BALL) {
					g_vars->scene06_sceneClickX = ex->_sceneClickX;
					g_vars->scene06_sceneClickY = ex->_sceneClickY;

					sceneHandler06_throwBall();
				}
			}

			if (!st || !canInteractAny(g_fullpipe->_aniMan, st, ex->_keyCode)) {
				int picId = g_fullpipe->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY);
				PictureObject *pic = g_fullpipe->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fullpipe->_aniMan, pic, ex->_keyCode)) {
					if ((g_fullpipe->_sceneRect.right - ex->_sceneClickX < 47
						 && g_fullpipe->_sceneRect.right < g_fullpipe->_sceneWidth - 1) 
						|| (ex->_sceneClickX - g_fullpipe->_sceneRect.left < 47 && g_fullpipe->_sceneRect.left > 0)) {
						g_fullpipe->processArcade(ex);
						return 0;
					}
				}
			}
		}

		break;

	case 33:
		{
			int res = 0;

			if (g_fullpipe->_aniMan2) {
				int ox = g_fullpipe->_aniMan2->_ox;
				int oy = g_fullpipe->_aniMan2->_oy;

				g_vars->scene06_var01 = ox;
				g_vars->scene06_var02 = oy;

				if (g_vars->scene06_var07 && oy <= 470 && ox >= 1088) {
					if (ox < g_fullpipe->_sceneRect.left + 600) {
						g_fullpipe->_currentScene->_x = ox - g_fullpipe->_sceneRect.left - 700;
						ox = g_vars->scene06_var01;
					}

					if (ox > g_fullpipe->_sceneRect.right - 50)
						g_fullpipe->_currentScene->_x = ox - g_fullpipe->_sceneRect.right + 70;
				} else {
					if (ox < g_fullpipe->_sceneRect.left + 200) {
						g_fullpipe->_currentScene->_x = ox - g_fullpipe->_sceneRect.left - 300;
						ox = g_vars->scene06_var01;
					}

					if (ox > g_fullpipe->_sceneRect.right - 200)
						g_fullpipe->_currentScene->_x = ox - g_fullpipe->_sceneRect.right + 300;
				}

				res = 1;
			}
			if (g_vars->scene06_var07) {
				if (g_vars->scene06_mumsyPos > -3)
					g_vars->scene06_mumsyJumpBk->_percent = g_vars->scene06_mumsyJumpBkPercent;
				else
					g_vars->scene06_mumsyJumpBk->_percent = 0;

				if (g_vars->scene06_mumsyPos < 4)
					g_vars->scene06_mumsyJumpFw->_percent = g_vars->scene06_mumsyJumpFwPercent;
				else
					g_vars->scene06_mumsyJumpFw->_percent = 0;

				if (g_vars->scene06_var08) {
					g_vars->scene06_var14++;
					if (g_vars->scene06_var14 >= 600)
						sceneHandler06_sub03();
				}
			} else {
				g_vars->scene06_mumsyJumpFw->_percent = 0;
				g_vars->scene06_mumsyJumpBk->_percent = 0;
			}

			if (g_vars->scene06_var11) {
				g_vars->scene06_var03 = g_vars->scene06_var11->_ox - g_vars->scene06_var17;
				g_vars->scene06_var04 = g_vars->scene06_var11->_oy - g_vars->scene06_var18;

				g_vars->scene06_var11->setOXY(g_vars->scene06_var03, g_vars->scene06_var04);

				if (g_vars->scene06_var17 >= 2)
					g_vars->scene06_var17 -= 2;

				g_vars->scene06_var18 -= 5;

				sceneHandler06_sub04(g_vars->scene06_var17);
			}
			if (g_vars->scene06_var07
				&& !g_vars->scene06_var09
				&& !g_vars->scene06_var10
				&& !g_vars->scene06_var11
				&& g_vars->scene06_var12 >= 15
				&& !g_vars->scene06_ballDrop->_movement
				&& !g_vars->scene06_mumsy->_movement
				&& !g_vars->scene06_var16)
				sceneHandler06_sub01();
			g_fullpipe->_behaviorManager->updateBehaviors();
			g_fullpipe->startSceneTrack();

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
