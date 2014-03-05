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

struct Swinger {
	StaticANIObject *ani;
	int sfield_4;
	double angle;
	int sx;
	int sy;
	int ix;
	int iy;
	int sflags;
	int sfield_24;
};

void scene18_preload() {
    g_fp->_scene3 = 0;

	for (SceneTagList::iterator s = g_fp->_gameProject->_sceneTagList->begin(); s != g_fp->_gameProject->_sceneTagList->end(); ++s) {
		if (s->_sceneId == SC_18) {
			g_fp->_scene3 = s->_scene;
			s->_scene = 0;

			g_fp->_scene3->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1)->freeMovementsPixelData();

			break;
		}
	}
}

void scene18_setupEntrance() {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (var->getSubVarAsInt("Entrance") == TrubaRight)
		var->setSubVarAsInt("Entrance", TrubaLeft);
}

void scene19_setSugarState(Scene *sc) {
	if (g_fp->getObjectState(sO_Sugar) != g_fp->getObjectEnumState(sO_Sugar, sO_Present)) {
		Scene *oldsc = g_fp->_currentScene;

		g_fp->_currentScene = sc;
		sc->getStaticANIObject1ById(ANI_CORDIE, -1)->changeStatics2(ST_CDI_EMPTY2);
		g_fp->_currentScene = oldsc;
	}
}

void scene19_setMovements(Scene *sc, int entranceId) {
	if (entranceId == TrubaRight) {
		g_vars->scene18_var15 = 1;
	} else {
		g_vars->scene18_var15 = 0;
		g_vars->scene19_var05 = (entranceId == PIC_SC19_RTRUBA3);
	}

	for (uint i = 0; i < g_vars->scene18_var07.size(); i++) {
		if (!g_vars->scene18_var15 && (g_vars->scene18_var07[i]->sflags & 0x20)) {
			Scene *oldsc = g_fp->_currentScene;

			g_vars->scene18_var07[i]->sflags = 1;

			g_fp->_currentScene = sc;
			g_vars->scene18_var07[i]->ani->changeStatics2(ST_KSL_NORM);
			g_vars->scene18_var07[i]->ani->_priority = 30;
			g_fp->_currentScene = oldsc;
		}

		sc->deleteStaticANIObject(g_vars->scene18_var07[i]->ani);
	}

	if (g_vars->scene18_whirlgig->_movement) {
		g_vars->scene18_var09 = g_vars->scene18_whirlgig->_movement->_currDynamicPhaseIndex + 1;

		int mx;

		if (g_vars->scene18_whirlgig->_movement->_currMovement)
			mx = g_vars->scene18_whirlgig->_movement->_currMovement->_dynamicPhases.size();
		else
			mx = g_vars->scene18_whirlgig->_movement->_dynamicPhases.size();

		if (g_vars->scene18_var09 > mx - 1)
			g_vars->scene18_var09 = -1;
	} else {
		g_vars->scene18_var09 = 0;
	}

	sc->deleteStaticANIObject(g_vars->scene18_boy);
	sc->deleteStaticANIObject(g_vars->scene18_girl);
	sc->stopAllSounds();
}

void scene19_preload() {
	for (SceneTagList::iterator s = g_fp->_gameProject->_sceneTagList->begin(); s != g_fp->_gameProject->_sceneTagList->end(); ++s) {
		if (s->_sceneId == SC_18) {
			s->_scene = g_fp->_scene3;

			break;
		}
	}
}

void scene18_setupSwingers(StaticANIObject *ani, Scene *sc) {
	Swinger *swinger;

	g_vars->scene18_var07.clear();

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	for (int i = 0; i < 8; i++) {
		swinger = new Swinger;

		swinger->angle = (double)i * M_PI / 4.0;
		swinger->sx = g_vars->scene18_var20 - (int)(cos(swinger->angle) * -575.0);
		swinger->sy = g_vars->scene18_var04 - (int)(sin(swinger->angle) * -575.0) + 87;
		swinger->ix = swinger->sx;
		swinger->iy = swinger->sy;

		if (i) {
			StaticANIObject *newani = new StaticANIObject(ani);

			ani = newani;

			swinger->ani = newani;
			swinger->sflags = 2;

			sc->addStaticANIObject(newani, 1);
		} else {
			swinger->ani = ani;
			swinger->sflags = g_vars->scene18_var03 != 0 ? 4 : 1;
		}

		ani->_statics = ani->getStaticsById(ST_KSL_NORM);
		ani->_movement = 0;
		ani->setOXY(swinger->sx, swinger->sy);
		ani->_priority = 30;
		ani->_flags |= 4;

		if (swinger->sflags & 2) {
			ani->startAnim(MV_KSL_SWINGBOY, 0, -1);
		} else {
			if (swinger->sflags & 4)
				ani->startAnim(MV_KSL_SWINGGIRL, 0, -1);
			else
				ani->startAnim(MV_KSL_SWING, 0, -1);
		}

		ani->_movement->setDynamicPhaseIndex(g_fp->_rnd->getRandomNumber(17));

		g_vars->scene18_var07.push_back(swinger);
	}

	g_fp->_currentScene = oldsc;
}

void scene18_initScene1(Scene *sc) {
	PicAniInfo info;

	int oldx = g_vars->scene18_var20;
	int oldy = g_vars->scene18_var04;

	g_vars->scene18_var03 = (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging));

	if (sc->_sceneId == SC_18) {
		g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1);
		g_vars->scene18_var20 = 1032;
		g_vars->scene18_var04 = -318;
	} else {
		g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLGIG_19, -1);
		g_vars->scene18_var20 = 1024;
		g_vars->scene18_var04 = 242;
	}

	int newx = g_vars->scene18_var20 - oldx;
	int newy = g_vars->scene18_var04 - oldy;

	g_vars->scene18_var29 += newx;
	g_vars->scene18_var30 += newy;
	g_vars->scene18_var05 += newx;
	g_vars->scene18_var06 += newy;

	for (uint i = 0; i < g_vars->scene18_var07.size(); i++) {
		g_vars->scene18_var07[i]->ani->getPicAniInfo(&info);
		sc->addStaticANIObject(g_vars->scene18_var07[i]->ani, 1);
		g_vars->scene18_var07[i]->ani->setPicAniInfo(&info);

		g_vars->scene18_var07[i]->sx += newx;
		g_vars->scene18_var07[i]->sy += newy;
		g_vars->scene18_var07[i]->ix += newx;
		g_vars->scene18_var07[i]->iy += newy;

		GameObject *go;

		if (g_vars->scene18_var07[i]->ani->_movement)
			go = g_vars->scene18_var07[i]->ani->_movement;
		else
			go = g_vars->scene18_var07[i]->ani;

		go->setOXY(newx + go->_ox, newy + go->_oy);
	}

	if (g_vars->scene18_var08 && g_vars->scene18_var09 != -1) {
		g_vars->scene18_whirlgig->startAnim(sc->_sceneId != SC_18 ? MV_WHR19_SPIN : MV_WHR18_SPIN, 0, -1);
		g_vars->scene18_whirlgig->_movement->setDynamicPhaseIndex(g_vars->scene18_var09);
	}

	int sndid;

	if (sc->_sceneId == SC_19) {
		if (g_vars->scene18_var08)
			sndid = SND_19_015;
		else
			sndid = SND_19_016;
	} else {
		if (g_vars->scene18_var08)
			sndid = SND_18_006;
		else
			sndid = SND_18_010;
	}

	g_fp->playSound(sndid, 1);

	g_vars->scene18_boy->getPicAniInfo(&info);
	sc->addStaticANIObject(g_vars->scene18_boy, 1);
	g_vars->scene18_boy->setPicAniInfo(&info);

	int x, y;

	if (g_vars->scene18_boy->_movement) {
		x = g_vars->scene18_boy->_movement->_ox;
		y = g_vars->scene18_boy->_movement->_oy;
	} else {
		x = g_vars->scene18_boy->_ox;
		y = g_vars->scene18_boy->_oy;
	}

	g_vars->scene18_boy->setOXY(newx + x, newy + y);

	g_vars->scene18_girl->getPicAniInfo(&info);
	sc->addStaticANIObject(g_vars->scene18_girl, 1);
	g_vars->scene18_girl->setPicAniInfo(&info);

	if (g_vars->scene18_girl->_movement) {
		x = g_vars->scene18_girl->_movement->_ox;
		y = g_vars->scene18_girl->_movement->_oy;
	} else {
		x = g_vars->scene18_girl->_ox;
		y = g_vars->scene18_girl->_oy;
	}

	g_vars->scene18_girl->setOXY(newx + x, newy + y);

	g_vars->scene18_var12 = 0;
	g_vars->scene18_var13 = -1;
	g_vars->scene18_var14 = -1;

	if (g_vars->scene18_var15) {
		if (sc->_sceneId == SC_19)
			g_fp->_aniMan2 = 0;
		else
			g_fp->_aniMan2 = g_vars->scene18_var07[g_vars->scene18_var27]->ani;
	} else {
		g_fp->_aniMan2 = g_fp->_aniMan;
	}
}

void scene18_initScene2(Scene *sc) {
	g_vars->scene18_var16 = 200;
	g_vars->scene18_var17 = 200;
	g_vars->scene18_var18 = 300;
	g_vars->scene18_var19 = 300;
	g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1);
	g_vars->scene18_var20 = 1032;
	g_vars->scene18_var04 = -318;

	StaticANIObject *armchair = sc->getStaticANIObject1ById(ANI_KRESLO, -1);

	armchair->loadMovementsPixelData();

	g_vars->scene18_var03 = (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging));

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene18_var08 = 1;
		g_fp->playSound(SND_18_006, 1);
	} else {
		g_vars->scene18_var08 = 0;
		g_fp->playSound(SND_18_010, 1);
	}

	scene18_setupSwingers(armchair, sc);

	g_vars->scene18_var21 = 0;
	g_vars->scene18_var12 = 0;
	g_vars->scene18_var22 = 1;
	g_vars->scene18_var23 = -1;
	g_vars->scene18_var24 = 0;
	g_vars->scene18_var25 = 0;
	g_vars->scene18_var26 = 1;
	g_vars->scene18_var27 = -1;
	g_vars->scene18_var13 = -1;
	g_vars->scene18_var14 = -1;
	g_vars->scene18_var28 = 0;
	g_vars->scene18_var15 = 0;
	g_vars->scene18_boy = sc->getStaticANIObject1ById(ANI_BOY18, -1);
	g_vars->scene18_girl = sc->getStaticANIObject1ById(ANI_GIRL18, -1);
	g_vars->scene18_domino = sc->getStaticANIObject1ById(ANI_DOMINO_18, -1);
	g_vars->scene18_var29 = 290;
	g_vars->scene18_var30 = -363;
	g_vars->scene18_var05 = 283;
	g_vars->scene18_var06 = -350;

	g_fp->initArcadeKeys("SC_18");
}

void scene19_initScene2() {
	g_fp->_aniMan2 = 0;
	g_vars->scene19_var01 = 200;
	g_vars->scene19_var02 = 200;
	g_vars->scene19_var03 = 300;
	g_vars->scene19_var04 = 300;
}

int scene18_updateCursor() {
	if (g_vars->scene18_var15) {
		g_fp->_cursorId = PIC_CSR_DEFAULT;
	} else {
		g_fp->updateCursorCommon();

		if (g_fp->_cursorId == PIC_CSR_ITN) {
			if (g_fp->_objectIdAtCursor == PIC_SC18_LADDER1) {
				g_fp->_cursorId = (g_vars->scene18_var11 <= 250) ? PIC_CSR_GOD : PIC_CSR_GOU;
			} else if (g_fp->_objectIdAtCursor == PIC_SC18_LADDER2 || g_fp->_objectIdAtCursor == PIC_SC18_LADDER3) {
				g_fp->_cursorId = PIC_CSR_GOU;
			}
		} else if (g_fp->_cursorId == PIC_CSR_DEFAULT && g_fp->_objectIdAtCursor == PIC_SC18_DOMIN && g_vars->scene18_domino && (g_vars->scene18_domino->_flags & 4)) {
			g_fp->_cursorId = PIC_CSR_ITN;
		}
	}

	return g_fp->_cursorId;
}

int scene19_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor == PIC_SC19_RTRUBA31)
		g_fp->_cursorId = g_vars->scene19_var05 != 0 ? PIC_CSR_GOR : PIC_CSR_DEFAULT;

	return g_fp->_cursorId;
}

void sceneHandler18_clickBoard() {
	warning("STUB: sceneHandler18_clickBoard()");
}

void sceneHandler18and19_showManJump() {
	warning("STUB: sceneHandler18and19_showManJump()");
}

void sceneHandler18_showManJumpTo() {
	warning("STUB: sceneHandler18_showManJumpTo()");
}

void sceneHandler18and19_showGirlJumpTo() {
	warning("STUB: sceneHandler18and19_showGirlJumpTo()");
}

void sceneHandler18and19_showGirlJump() {
	warning("STUB: sceneHandler18and19_showGirlJump()");
}

void sceneHandler18and19_showBoyJumpTo() {
	warning("STUB: sceneHandler18and19_showBoyJumpTo()");
}

void sceneHandler18and19_showBoyJump() {
	warning("STUB: sceneHandler18and19_showBoyJump()");
}

void sceneHandler18and19_drawRiders() {
	warning("STUB: sceneHandler18and19_drawRiders()");
}

void sceneHandler18and19_animateRiders() {
	warning("STUB: sceneHandler18and19_animateRiders()");
}

int sceneHandler18(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC18_CLICKBOARD:
		sceneHandler18_clickBoard();
		break;

	case MSG_SC3_HIDEDOMINO:
		g_vars->scene18_domino->_flags &= 0xFFFB;
		break;

	case MSG_SC18_SHOWMANJUMP:
		sceneHandler18and19_showManJump();
		break;

	case MSG_SC18_MANREADY:
		g_vars->scene18_var28 = 1;
		break;

	case MSG_SC18_SHOWMANJUMPTO:
		sceneHandler18_showManJumpTo();
		break;

	case MSG_SC18_SHOWGIRLJUMPTO:
		sceneHandler18and19_showGirlJumpTo();
		break;

	case MSG_SC18_SHOWGIRLJUMP:
		sceneHandler18and19_showGirlJump();
		break;

	case MSG_SC18_SHOWBOYJUMPTO:
		sceneHandler18and19_showBoyJumpTo();
		break;

	case MSG_SC18_SHOWBOYJUMP:
		sceneHandler18and19_showBoyJump();
		break;

	case 29:
		{
			if (g_vars->scene18_var15) {
				cmd->_messageKind = 0;
				break;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (pic && pic->_id == PIC_SC18_DOMIN && g_vars->scene18_domino
					&& (g_vars->scene18_domino->_flags & 4) && g_fp->_aniMan->isIdle()) {
					if (!(g_fp->_aniMan->_flags & 0x100) && g_fp->_msgObjectId2 != g_vars->scene18_domino->_id) {
						handleObjectInteraction(g_fp->_aniMan, g_vars->scene18_domino, cmd->_keyCode);
						cmd->_messageKind = 0;
						break;
					}
				}

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);

						g_vars->scene18_var28 = 0;

						break;
					}
				}
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			g_vars->scene18_var11 = g_fp->_aniMan2->_oy;

			if (x < g_fp->_sceneRect.left + g_vars->scene18_var16)
				g_fp->_currentScene->_x = x - g_vars->scene18_var18 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene18_var16)
				g_fp->_currentScene->_x = x + g_vars->scene18_var18 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene18_var28 && g_fp->_aniMan->_movement)
			g_vars->scene18_var28 = 0;

		if (g_vars->scene18_var08) {
			if (!g_vars->scene18_var12)
				sceneHandler18and19_drawRiders();

			g_vars->scene18_var12 = g_vars->scene18_var12 == 0;

			if (!g_vars->scene18_whirlgig->_movement) {
				g_vars->scene18_whirlgig->startAnim(MV_WHR18_SPIN, 0, -1);
				g_fp->_behaviorManager->updateBehaviors();
				break;
			}
		} else {
			sceneHandler18and19_animateRiders();
		}

		g_fp->_behaviorManager->updateBehaviors();
		break;
	}

	return 0;
}

void sceneHandler19_updateNumRides() {
	warning("STUB: sceneHandler19_updateNumRides()");
}

int sceneHandler19(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC18_SHOWMANJUMP:
		sceneHandler18and19_showManJump();
		break;

	case MSG_SC19_UPDATENUMRIDES:
		sceneHandler19_updateNumRides();
		break;

	case MSG_SC18_SHOWGIRLJUMPTO:
		sceneHandler18and19_showGirlJumpTo();
		break;

	case MSG_SC18_SHOWBOYJUMPTO:
		sceneHandler18and19_showBoyJumpTo();
		break;

	case MSG_SC18_SHOWGIRLJUMP:
		sceneHandler18and19_showGirlJump();
		break;

	case MSG_SC18_SHOWBOYJUMP:
		sceneHandler18and19_showBoyJump();
		break;

	case 29:
		if (g_vars->scene18_var15) {
			switch (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY)) {
			case PIC_SC19_RTRUBA1:
				g_vars->scene18_var13 = 1;
				g_vars->scene18_var14 = 331;
				cmd->_messageKind = 0;
				break;

			case PIC_SC19_RTRUBA2:
				g_vars->scene18_var13 = 2;
				g_vars->scene18_var14 = 350;
				cmd->_messageKind = 0;
				break;

			case PIC_SC19_RTRUBA3:
				g_vars->scene18_var13 = 3;
				g_vars->scene18_var14 = 9;
				cmd->_messageKind = 0;
				break;

			default:
				g_vars->scene18_var13 = -1;
				g_vars->scene18_var14 = -1;
				cmd->_messageKind = 0;
				break;
			}
			break;
		}

		if (g_vars->scene19_var05) {
			if (g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC19_RTRUBA3) {
				if (g_fp->_aniMan->isIdle()) {
					if (!(g_fp->_aniMan->_flags & 0x100)) {
						PictureObject *pic = g_fp->_currentScene->getPictureObjectById(PIC_SC19_RTRUBA31, 0);

						handleObjectInteraction(g_fp->_aniMan, pic, cmd->_keyCode);
						break;
					}
				}
			}
		}
		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			g_vars->scene18_var11 = g_fp->_aniMan2->_oy;

			if (x < g_fp->_sceneRect.left + g_vars->scene18_var16)
				g_fp->_currentScene->_x = x - g_vars->scene18_var18 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene18_var16)
				g_fp->_currentScene->_x = x + g_vars->scene18_var18 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene18_var08) {
			if (!g_vars->scene18_var12)
				sceneHandler18and19_drawRiders();

			g_vars->scene18_var12 = !g_vars->scene18_var12;

			if (!g_vars->scene18_whirlgig->_movement) {
				g_vars->scene18_whirlgig->startAnim(MV_WHR19_SPIN, 0, -1);

				g_fp->_behaviorManager->updateBehaviors();
				break;
			}
		} else {
			sceneHandler18and19_animateRiders();
		}

		g_fp->_behaviorManager->updateBehaviors();
		break;
	}

	return 0;
}

} // End of namespace Fullpipe
