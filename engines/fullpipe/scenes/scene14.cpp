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

void scene14_initScene(Scene *sc) {
	g_vars->scene14_grandma = sc->getStaticANIObject1ById(ANI_GRANDMA, -1);
	g_vars->scene14_var01 = 200;
	g_vars->scene14_var02 = 200;
	g_vars->scene14_var03 = 0;
	g_vars->scene14_var04 = 0;
	g_vars->scene14_var05 = 0;
	g_vars->scene14_var06 = 0;
	g_vars->scene14_var07 = 300;
	g_vars->scene14_var08 = 300;
	g_vars->scene14_pink = 0;
	g_vars->scene14_var10 = 0;
	g_vars->scene14_var11.clear();
	g_vars->scene14_var12.clear();

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_14)) {
		g_vars->scene14_var13 = 1;

		StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_BALL14, -1);

		if (ball) {
			ball->_flags &= 0xFFFB;
			g_vars->scene14_var11.push_back(ball);
		}
		
		for (uint i = 0; i < 3; i++) {
			ball = new StaticANIObject(ball); // create a copy

			ball->_flags &= 0xFFFB;
			g_vars->scene14_var11.push_back(ball);

			sc->addStaticANIObject(ball, 1);
		}
	} else {
		g_vars->scene14_var13 = 0;
		g_vars->scene14_grandma->hide();
	}

	g_fp->lift_setButton(sO_Level4, ST_LBN_4N);
	g_fp->lift_sub5(sc, QU_SC14_ENTERLIFT, QU_SC14_EXITLIFT);

	g_fp->initArcadeKeys("SC_14");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE6);
}

void scene14_setupMusic() {
	if (!g_vars->scene14_var13)
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_14"), "MUSIC2", 0);
}

int scene14_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene14_var03) {
		if (g_vars->scene14_var04) {
			g_fp->_cursorId = PIC_CSR_ARCADE2_D;
		} else {
			if (g_fp->_aniMan != g_fp->_objectAtCursor || g_fp->_aniMan->_movement || g_fp->_cursorId != PIC_CSR_DEFAULT) {
				if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV) {
					g_fp->_cursorId = PIC_CSR_DEFAULT;
				}
			} else {
				g_fp->_cursorId = PIC_CSR_ITN;
			}
		}
	}

	return g_fp->_cursorId;
}

void sceneHandler14_showBallGrandmaHit2() {
	warning("STUB: sceneHandler14_showBallGrandmaHit2()");
}

void sceneHandler14_showBallGrandmaDive() {
	warning("STUB: sceneHandler14_showBallGrandmaDive()");
}

void sceneHandler14_showBallGrandmaHit() {
	warning("STUB: sceneHandler14_showBallGrandmaHit()");
}

void sceneHandler14_showBallMan() {
	warning("STUB: sceneHandler14_showBallMan()");
}

void sceneHandler14_manKickBall() {
	warning("STUB: sceneHandler14_manKickBall()");
}

void sceneHandler14_showBallFly() {
	warning("STUB: sceneHandler14_showBallFly()");
}

void sceneHandler14_grandmaJump() {
	warning("STUB: sceneHandler14_grandmaJump()");
}

void sceneHandler14_winArcade() {
	warning("STUB: sceneHandler14_winArcade()");
}

void sceneHandler14_showBallLast() {
	warning("STUB: sceneHandler14_showBallLast()");
}

void sceneHandler14_hideBallLast() {
	warning("STUB: sceneHandler14_hideBallLast()");
}

void sceneHandler14_startArcade() {
	warning("STUB: sceneHandler14_startArcade()");
}

void sceneHandler14_endArcade() {
	warning("STUB: sceneHandler14_endArcade()");
}

void sceneHandler14_sub01() {
	warning("STUB: sceneHandler14_sub01()");
}

void sceneHandler14_sub02() {
	warning("STUB: sceneHandler14_sub02()");
}

void sceneHandler14_sub03() {
	warning("STUB: sceneHandler14_sub03()");
}

bool sceneHandler14_sub04(ExCommand *cmd) {
	warning("STUB: sceneHandler14_sub04()");

	return false;
}

void sceneHandler14_sub05() {
	warning("STUB: sceneHandler14_sub05()");
}

void sceneHandler14_sub06() {
	warning("STUB: sceneHandler14_sub06()");
}

void sceneHandler14_sub07() {
	warning("STUB: sceneHandler14_sub07()");
}

void sceneHandler14_sub08() {
	warning("STUB: sceneHandler14_sub08()");
}

void sceneHandler14_sub09() {
	warning("STUB: sceneHandler14_sub09()");
}

void sceneHandler14_sub10() {
	warning("STUB: sceneHandler14_sub10()");
}

void sceneHandler14_sub11() {
	warning("STUB: sceneHandler14_sub11()");
}

void sceneHandler14_sub12() {
	warning("STUB: sceneHandler14_sub12()");
}

void sceneHandler14_sub13() {
	warning("STUB: sceneHandler14_sub13()");
}

int sceneHandler14(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_SC14_SHOWBALLGMAHIT2:
		sceneHandler14_showBallGrandmaHit2();
		break;

	case MSG_SC14_SHOWBALLGMADIVE:
		sceneHandler14_showBallGrandmaDive();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_animation3();
		break;

	case MSG_SC14_SHOWBALLGMAHIT:
		sceneHandler14_showBallGrandmaHit();
		break;

	case MSG_SC14_SHOWBALLMAN:
		sceneHandler14_showBallMan();
		break;

	case MSG_SC14_MANKICKBALL:
		sceneHandler14_manKickBall();
		break;

	case MSG_SC14_SHOWBALLFLY:
		sceneHandler14_showBallFly();
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC14_GMAJUMP:
		sceneHandler14_grandmaJump();
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

	case MSG_SC14_RESTORESCROLL:
		g_fp->_aniMan2 = g_fp->_aniMan;
		g_fp->_scrollSpeed = 8;
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler14_winArcade();
		break;

	case MSG_SC14_SCROLLLEFT:
		g_fp->_aniMan2 = 0;
		g_fp->_currentScene->_x = -g_fp->_sceneRect.left;
		g_fp->_scrollSpeed = 24;
		break;

	case MSG_SC14_SHOWBALLLAST:
		sceneHandler14_showBallLast();
		break;

	case MSG_SC14_HIDEBALLLAST:
		sceneHandler14_hideBallLast();
		break;

	case MSG_SC14_HIDEPINK:
		if (!g_vars->scene14_pink)
			break;

		g_vars->scene14_pink->hide();
		break;

	case MSG_SC14_GMATOTRUBA:
		g_fp->_currentScene->_x = -g_fp->_sceneRect.left;
		break;

	case MSG_SC14_STARTARCADE:
		sceneHandler14_startArcade();
		break;

	case MSG_SC14_ENDARCADE:
		sceneHandler14_endArcade();

		g_vars->scene14_var13 = 0;
		break;

	case 64:
		g_fp->lift_sub05(cmd);
		break;

	case 33:
		{
			Movement *mov = g_fp->_aniMan->_movement;

			if (mov) {
				g_vars->scene14_var14 = mov->_ox;
				g_vars->scene14_var15 = mov->_oy;

				if (mov->_id == MV_MAN14_KICK)
					g_vars->scene14_var14 = mov->_ox + 2 * g_fp->_aniMan->_movement->_currDynamicPhaseIndex;
			} else {
				g_vars->scene14_var14 = g_fp->_aniMan->_ox;
				g_vars->scene14_var15 = g_fp->_aniMan->_oy;
			}

			mov = g_vars->scene14_grandma->_movement;
			if (mov) {
				g_vars->scene14_var16 = mov->_ox;
				g_vars->scene14_var17 = mov->_oy;
			} else {
				g_vars->scene14_var16 = g_vars->scene14_grandma->_ox;
				g_vars->scene14_var17 = g_vars->scene14_grandma->_oy;
			}

			if (g_fp->_aniMan2) {
				int x = g_fp->_aniMan2->_ox;
				g_vars->scene14_var18 = x;
				g_vars->scene14_var19 = g_fp->_aniMan2->_oy;

				if (x < g_fp->_sceneRect.left + g_vars->scene14_var01) {
					g_fp->_currentScene->_x = x - g_vars->scene14_var07 - g_fp->_sceneRect.left;
					x = g_vars->scene14_var18;
				}

				if (x > g_fp->_sceneRect.right - g_vars->scene14_var01)
					g_fp->_currentScene->_x = x + g_vars->scene14_var07 - g_fp->_sceneRect.right;
			}

			if (g_vars->scene14_var05)
				sceneHandler14_sub01();

			g_fp->_behaviorManager->updateBehaviors();
			g_fp->startSceneTrack();
			break;
		}

	case 30:
		if (g_vars->scene14_var04) {
			sceneHandler14_sub02();
			break;
		}

		if (!g_vars->scene14_var03) {
			break;
		}
		break;

	case 29:
        if (g_vars->scene14_var03) {
			int pixel;

			if (g_vars->scene14_var06 && g_fp->_aniMan->getPixelAtPos(cmd->_sceneClickX, cmd->_sceneClickY, &pixel) && !g_fp->_aniMan->_movement) {
				sceneHandler14_sub03();
				break;
			}
        } else {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_sub1(ani);
				cmd->_messageKind = 0;
				break;
			}

			if (!sceneHandler14_sub04(cmd) && (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode))) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						sceneHandler14_sub04(cmd);
						break;
					}
				}
			}
        }
		break;
    }

	return 0;
}

} // End of namespace Fullpipe
