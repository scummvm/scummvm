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

#include "common/math.h"

namespace NGI {

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
	g_nmi->_scene3 = 0;

	for (SceneTagList::iterator s = g_nmi->_gameProject->_sceneTagList->begin(); s != g_nmi->_gameProject->_sceneTagList->end(); ++s) {
		if (s->_sceneId == SC_18) {
			g_nmi->_scene3 = s->_scene;
			s->_scene = 0;

			g_nmi->_scene3->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1)->freeMovementsPixelData();

			break;
		}
	}
}

void scene18_setupEntrance() {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (var->getSubVarAsInt("Entrance") == TrubaRight)
		var->setSubVarAsInt("Entrance", TrubaLeft);
}

void scene19_setSugarState(Scene *sc) {
	if (g_nmi->getObjectState(sO_Sugar) != g_nmi->getObjectEnumState(sO_Sugar, sO_Present)) {
		Scene *oldsc = g_nmi->_currentScene;

		g_nmi->_currentScene = sc;
		sc->getStaticANIObject1ById(ANI_CORDIE, -1)->changeStatics2(ST_CDI_EMPTY2);
		g_nmi->_currentScene = oldsc;
	}
}

void scene19_setMovements(Scene *sc, int entranceId) {
	if (entranceId == TrubaRight) {
		g_vars->scene18_enteredTrubaRight = true;
	} else {
		g_vars->scene18_enteredTrubaRight = false;
		g_vars->scene19_enteredTruba3 = (entranceId == PIC_SC19_RTRUBA3);
	}

	for (uint i = 0; i < g_vars->scene18_swingers.size(); i++) {
		if (!g_vars->scene18_enteredTrubaRight && (g_vars->scene18_swingers[i]->sflags & 0x20)) {
			Scene *oldsc = g_nmi->_currentScene;

			g_vars->scene18_swingers[i]->sflags = 1;

			g_nmi->_currentScene = sc;
			g_vars->scene18_swingers[i]->ani->changeStatics2(ST_KSL_NORM);
			g_vars->scene18_swingers[i]->ani->_priority = 30;
			g_nmi->_currentScene = oldsc;
		}

		sc->deleteStaticANIObject(g_vars->scene18_swingers[i]->ani);
	}

	if (g_vars->scene18_whirlgig->_movement) {
		g_vars->scene18_whirlgigMovMum = g_vars->scene18_whirlgig->_movement->_currDynamicPhaseIndex + 1;

		int mx;

		if (g_vars->scene18_whirlgig->_movement->_currMovement)
			mx = g_vars->scene18_whirlgig->_movement->_currMovement->_dynamicPhases.size();
		else
			mx = g_vars->scene18_whirlgig->_movement->_dynamicPhases.size();

		if (g_vars->scene18_whirlgigMovMum > mx - 1)
			g_vars->scene18_whirlgigMovMum = -1;
	} else {
		g_vars->scene18_whirlgigMovMum = 0;
	}

	sc->deleteStaticANIObject(g_vars->scene18_boy);
	sc->deleteStaticANIObject(g_vars->scene18_girl);
	sc->stopAllSounds();
}

void scene19_preload() {
	for (SceneTagList::iterator s = g_nmi->_gameProject->_sceneTagList->begin(); s != g_nmi->_gameProject->_sceneTagList->end(); ++s) {
		if (s->_sceneId == SC_18) {
			delete s->_scene;
			s->_scene = g_nmi->_scene3;

			break;
		}
	}
}

void scene18_setupSwingers(StaticANIObject *ani, Scene *sc) {
	Swinger *swinger;

	g_vars->scene18_swingers.clear();

	Scene *oldsc = g_nmi->_currentScene;
	g_nmi->_currentScene = sc;

	for (int i = 0; i < 8; i++) {
		swinger = new Swinger;

		swinger->angle = (double)i * Common::deg2rad<double>(45.0);
		swinger->sx = g_vars->scene18_wheelCenterX - (int)(cos(swinger->angle) * -575.0);
		swinger->sy = g_vars->scene18_wheelCenterY - (int)(sin(swinger->angle) * -575.0) + 87;
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
			swinger->sflags = g_vars->scene18_girlIsSwinging ? 4 : 1;
		}

		ani->_statics = ani->getStaticsById(ST_KSL_NORM);
		//ani->_movement = 0;
		ani->setOXY(swinger->sx, swinger->sy);
		ani->_priority = 30;
		ani->_flags |= 4;

		if (swinger->sflags & 2)
			ani->startAnim(MV_KSL_SWINGBOY, 0, -1);
		else if (swinger->sflags & 4)
			ani->startAnim(MV_KSL_SWINGGIRL, 0, -1);
		else
			ani->startAnim(MV_KSL_SWING, 0, -1);

		ani->_movement->setDynamicPhaseIndex(g_nmi->_rnd.getRandomNumber(17));

		g_vars->scene18_swingers.push_back(swinger);
	}

	g_nmi->_currentScene = oldsc;
}

void scene18_initScene1(Scene *sc) {
	PicAniInfo info;

	int oldx = g_vars->scene18_wheelCenterX;
	int oldy = g_vars->scene18_wheelCenterY;

	g_vars->scene18_girlIsSwinging = (g_nmi->getObjectState(sO_Girl) == g_nmi->getObjectEnumState(sO_Girl, sO_IsSwinging));

	if (sc->_sceneId == SC_18) {
		g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1);
		g_vars->scene18_wheelCenterX = 1032;
		g_vars->scene18_wheelCenterY = -318;
	} else {
		g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLGIG_19, -1);
		g_vars->scene18_wheelCenterX = 1024;
		g_vars->scene18_wheelCenterY = 242;
	}

	int newx = g_vars->scene18_wheelCenterX - oldx;
	int newy = g_vars->scene18_wheelCenterY - oldy;

	g_vars->scene18_boyJumpX += newx;
	g_vars->scene18_boyJumpY += newy;
	g_vars->scene18_girlJumpX += newx;
	g_vars->scene18_girlJumpY += newy;

	for (uint i = 0; i < g_vars->scene18_swingers.size(); i++) {
		g_vars->scene18_swingers[i]->ani->getPicAniInfo(info);
		sc->addStaticANIObject(g_vars->scene18_swingers[i]->ani, 1);
		g_vars->scene18_swingers[i]->ani->setPicAniInfo(info);

		g_vars->scene18_swingers[i]->sx += newx;
		g_vars->scene18_swingers[i]->sy += newy;
		g_vars->scene18_swingers[i]->ix += newx;
		g_vars->scene18_swingers[i]->iy += newy;

		GameObject *go;

		if (g_vars->scene18_swingers[i]->ani->_movement)
			go = g_vars->scene18_swingers[i]->ani->_movement;
		else
			go = g_vars->scene18_swingers[i]->ani;

		go->setOXY(newx + go->_ox, newy + go->_oy);
	}

	if (g_vars->scene18_bridgeIsConvoluted && g_vars->scene18_whirlgigMovMum != -1) {
		g_vars->scene18_whirlgig->startAnim(sc->_sceneId != SC_18 ? MV_WHR19_SPIN : MV_WHR18_SPIN, 0, -1);
		g_vars->scene18_whirlgig->_movement->setDynamicPhaseIndex(g_vars->scene18_whirlgigMovMum);
	}

	int sndid;

	if (sc->_sceneId == SC_19) {
		if (g_vars->scene18_bridgeIsConvoluted)
			sndid = SND_19_015;
		else
			sndid = SND_19_016;
	} else {
		if (g_vars->scene18_bridgeIsConvoluted)
			sndid = SND_18_006;
		else
			sndid = SND_18_010;
	}

	g_nmi->playSound(sndid, 1);

	g_vars->scene18_boy->getPicAniInfo(info);
	sc->addStaticANIObject(g_vars->scene18_boy, 1);
	g_vars->scene18_boy->setPicAniInfo(info);

	int x, y;

	if (g_vars->scene18_boy->_movement) {
		x = g_vars->scene18_boy->_movement->_ox;
		y = g_vars->scene18_boy->_movement->_oy;
	} else {
		x = g_vars->scene18_boy->_ox;
		y = g_vars->scene18_boy->_oy;
	}

	g_vars->scene18_boy->setOXY(newx + x, newy + y);

	g_vars->scene18_girl->getPicAniInfo(info);
	sc->addStaticANIObject(g_vars->scene18_girl, 1);
	g_vars->scene18_girl->setPicAniInfo(info);

	if (g_vars->scene18_girl->_movement) {
		x = g_vars->scene18_girl->_movement->_ox;
		y = g_vars->scene18_girl->_movement->_oy;
	} else {
		x = g_vars->scene18_girl->_ox;
		y = g_vars->scene18_girl->_oy;
	}

	g_vars->scene18_girl->setOXY(newx + x, newy + y);

	g_vars->scene18_wheelFlipper = false;
	g_vars->scene18_jumpDistance = -1;
	g_vars->scene18_jumpAngle = -1;

	if (g_vars->scene18_enteredTrubaRight) {
		if (sc->_sceneId == SC_19)
			g_nmi->_aniMan2 = 0;
		else
			g_nmi->_aniMan2 = g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->ani;
	} else {
		g_nmi->_aniMan2 = g_nmi->_aniMan;
	}
}

void scene18_initScene2(Scene *sc) {
	g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1);
	g_vars->scene18_wheelCenterX = 1032;
	g_vars->scene18_wheelCenterY = -318;

	StaticANIObject *armchair = sc->getStaticANIObject1ById(ANI_KRESLO, -1);

	armchair->loadMovementsPixelData();

	g_vars->scene18_girlIsSwinging = (g_nmi->getObjectState(sO_Girl) == g_nmi->getObjectEnumState(sO_Girl, sO_IsSwinging));

	if (g_nmi->getObjectState(sO_Bridge) == g_nmi->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene18_bridgeIsConvoluted = true;
		g_nmi->playSound(SND_18_006, 1);
	} else {
		g_vars->scene18_bridgeIsConvoluted = false;
		g_nmi->playSound(SND_18_010, 1);
	}

	scene18_setupSwingers(armchair, sc);

	g_vars->scene18_rotationCounter = 0;
	g_vars->scene18_wheelFlipper = false;
	g_vars->scene18_wheelIsTurning = true;
	g_vars->scene18_kidIsOnWheel = -1;
	g_vars->scene18_boyIsOnWheel = 0;
	g_vars->scene18_girlIsOnWheel = 0;
	g_vars->scene18_boyJumpedOff = true;
	g_vars->scene18_manWheelPosTo = -1;
	g_vars->scene18_jumpDistance = -1;
	g_vars->scene18_jumpAngle = -1;
	g_vars->scene18_manIsReady = false;
	g_vars->scene18_enteredTrubaRight = 0;
	g_vars->scene18_boy = sc->getStaticANIObject1ById(ANI_BOY18, -1);
	g_vars->scene18_girl = sc->getStaticANIObject1ById(ANI_GIRL18, -1);
	g_vars->scene18_domino = sc->getStaticANIObject1ById(ANI_DOMINO_18, -1);
	g_vars->scene18_boyJumpX = 290;
	g_vars->scene18_boyJumpY = -363;
	g_vars->scene18_girlJumpX = 283;
	g_vars->scene18_girlJumpY = -350;

	g_nmi->initArcadeKeys("SC_18");
}

void scene19_initScene2() {
	g_nmi->_aniMan2 = 0;
}

int scene18_updateCursor() {
	if (g_vars->scene18_enteredTrubaRight) {
		g_nmi->_cursorId = PIC_CSR_DEFAULT;
	} else {
		g_nmi->updateCursorCommon();

		if (g_nmi->_cursorId == PIC_CSR_ITN) {
			if (g_nmi->_objectIdAtCursor == PIC_SC18_LADDER1) {
				g_nmi->_cursorId = (g_vars->scene18_manY <= 250) ? PIC_CSR_GOD : PIC_CSR_GOU;
			} else if (g_nmi->_objectIdAtCursor == PIC_SC18_LADDER2 || g_nmi->_objectIdAtCursor == PIC_SC18_LADDER3) {
				g_nmi->_cursorId = PIC_CSR_GOU;
			}
		} else if (g_nmi->_cursorId == PIC_CSR_DEFAULT && g_nmi->_objectIdAtCursor == PIC_SC18_DOMIN && g_vars->scene18_domino && (g_vars->scene18_domino->_flags & 4)) {
			g_nmi->_cursorId = PIC_CSR_ITN;
		}
	}

	return g_nmi->_cursorId;
}

int scene19_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor == PIC_SC19_RTRUBA31)
		g_nmi->_cursorId = g_vars->scene19_enteredTruba3 ? PIC_CSR_GOR : PIC_CSR_DEFAULT;

	return g_nmi->_cursorId;
}

void sceneHandler18_clickBoard() {
	if (ABS(967 - g_nmi->_aniMan->_ox) > 1 || ABS(379 - g_nmi->_aniMan->_oy) > 1 || g_nmi->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
		MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 967, 379, 1, ST_MAN_RIGHT);
		ExCommand *ex = new ExCommand(0, 17, MSG_SC18_MANREADY, 0, 0, 0, 1, 0, 0, 0);

		ex->_excFlags = 2;

		mq->addExCommandToEnd(ex);

		postExCommand(g_nmi->_aniMan->_id, 2, 967, 379, 0, -1);
	} else {
		g_vars->scene18_manIsReady = true;
	}
}

void sceneHandler18_showManJumpTo() {
	g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT);
	g_nmi->_aniMan->_flags &= 0xFFFB;
	g_nmi->_aniMan->_flags &= 0xFEFF;
	g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->sflags = 0x20;

	g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->ani->changeStatics2(ST_KSL_JUMPMAN);
	g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->ani->startAnim(MV_KSL_INMAN, 0, -1);
	g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->ani->_priority = 20;

	g_vars->scene18_manIsReady = false;
	g_vars->scene18_enteredTrubaRight = true;

	g_nmi->_aniMan2 = g_vars->scene18_swingers[g_vars->scene18_manWheelPosTo]->ani;
}

void sceneHandler18and19_showManJump() {
	int x, y;

	if (g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_movement) {
		x = g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_movement->_ox;
		y = g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_movement->_oy;
	} else {
		x = g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_ox;
		y = g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_oy;
	}

	g_nmi->_aniMan->show1(x + 62, y + 5, MV_MAN18_JUMPTOTRUBA, 0);
	g_nmi->_aniMan->_priority = 35;

	int mqid = 0;

	if (g_vars->scene18_jumpDistance == 1) {
		mqid = QU_SC19_MANJUMP1;
	} else if (g_vars->scene18_jumpDistance == 2) {
		mqid = QU_SC19_MANJUMP2;
	} else if (g_vars->scene18_jumpDistance == 3) {
		mqid = QU_SC19_MANJUMP3;
	}

	if (mqid) {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(mqid), 0, 0);

		g_nmi->_aniMan2 = g_nmi->_aniMan;
		g_vars->scene18_enteredTrubaRight = false;

		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);
	}

	g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->sflags = 1;

	g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->changeStatics2(ST_KSL_NORM);
	g_vars->scene18_swingers[g_vars->scene18_manWheelPos]->ani->_priority = 30;
}

void sceneHandler18and19_showGirlJumpTo() {
	g_vars->scene18_girl->stopAnim_maybe();
	g_vars->scene18_girl->hide();

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->sflags = 4;

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->ani->changeStatics2(ST_KSL_JUMPGIRL);
	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->ani->startAnim(MV_KSL_INGIRL, 0, -1);

	g_vars->scene18_kidIsOnWheel--;
	g_vars->scene18_girlIsOnWheel--;
}

void sceneHandler18and19_showGirlJump() {
	StaticANIObject *ani = g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani;
	int x, y;

	if (ani->_movement) {
		x = ani->_movement->_ox;
		y = ani->_movement->_oy;
	} else {
		x = ani->_ox;
		y = ani->_oy;
	}

	g_vars->scene18_girl->show1(x - 62, y - 10, MV_GRL18_JUMPFROM, 0);
	g_vars->scene18_girl->_priority = 50;
	g_vars->scene18_girl->startAnim(MV_GRL18_JUMPFROM, 0, -1);

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->sflags = 1;

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani->changeStatics2(ST_KSL_REACT);
	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani->startAnim(MV_KSL_CALMDOWN, 0, -1);

	g_vars->scene18_kidIsOnWheel = 1;
	g_vars->scene18_girlIsOnWheel++;
}

void sceneHandler18and19_showBoyJumpTo() {
	g_vars->scene18_boy->stopAnim_maybe();
	g_vars->scene18_boy->hide();

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->sflags = 2;
	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->ani->changeStatics2(ST_KSL_JUMPBOY);
	g_vars->scene18_swingers[g_vars->scene18_kidWheelPosTo]->ani->startAnim(MV_KSL_INBOY, 0, -1);

	g_vars->scene18_kidIsOnWheel--;
	g_vars->scene18_boyIsOnWheel--;
}

void sceneHandler18and19_showBoyJump() {
	StaticANIObject *ani = g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani;
	int x, y;

	if (ani->_movement) {
		x = ani->_movement->_ox;
		y = ani->_movement->_oy;
	} else {
		x = ani->_ox;
		y = ani->_oy;
	}

	g_vars->scene18_boy->show1(x - 48, y + 8, MV_BOY18_JUMPFROM, 0);
	g_vars->scene18_boy->_priority = 50;
	g_vars->scene18_boy->startAnim(MV_BOY18_JUMPFROM, 0, -1);

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->sflags = 1;

	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani->changeStatics2(ST_KSL_REACT);
	g_vars->scene18_swingers[g_vars->scene18_kidWheelPos]->ani->startAnim(MV_KSL_CALMDOWN, 0, -1);

	g_vars->scene18_boyJumpedOff = true;
}

void sceneHandler18and19_boyJumpTo() {
	g_vars->scene18_boy->stopAnim_maybe();
	g_vars->scene18_boy->show1(g_vars->scene18_boyJumpX, g_vars->scene18_boyJumpY, MV_BOY18_JUMPTO, 0);
	g_vars->scene18_boy->_priority = 50;
	g_vars->scene18_boy->startAnim(MV_BOY18_JUMPTO, 0, -1);
}

void sceneHandler18and19_girlJumpTo() {
	g_vars->scene18_girl->stopAnim_maybe();
	g_vars->scene18_girl->show1(g_vars->scene18_girlJumpX, g_vars->scene18_girlJumpY, MV_GRL18_JUMPTO, 0);
	g_vars->scene18_girl->_priority = 50;
	g_vars->scene18_girl->startAnim(MV_GRL18_JUMPTO, 0, -1);
}

void sceneHandler18and19_manStandArmchair() {
	g_nmi->_aniMan->changeStatics2(ST_MAN_RIGHT);
	g_nmi->_aniMan->_flags |= 0x100;
	g_nmi->_aniMan->_priority = 35;
	g_nmi->_aniMan->startAnim(MV_MAN18_STANDKRESLO, 0, -1);
}

void sceneHandler18and19_drawRiders() {
	g_vars->scene18_rotationCounter++;

	if (g_vars->scene18_rotationCounter >= 359)
		g_vars->scene18_rotationCounter = 0;

	for (uint i = 0; i < g_vars->scene18_swingers.size(); i++) {
		Swinger *swinger = g_vars->scene18_swingers[i];

		double oldangle = swinger->angle;

		swinger->angle += Common::deg2rad<double>(1.0);

		if (swinger->angle > Common::deg2rad<double>(360.0)) {
			swinger->angle -= Common::deg2rad<double>(360.0);
			oldangle -= Common::deg2rad<double>(360.0);
		}

		int ix = g_vars->scene18_wheelCenterX - (int)(cos(swinger->angle) * -575.0);
		int iy = g_vars->scene18_wheelCenterY - (int)(sin(swinger->angle) * -575.0) + 87;

		if (!g_vars->scene18_rotationCounter) {
			ix = swinger->sx;
			iy = swinger->sy;
			swinger->angle = (double)i * Common::deg2rad<double>(45.0);
		}

		if (swinger->ani->_movement)
			swinger->ani->setOXY(ix - swinger->ix + swinger->ani->_movement->_ox, iy - swinger->iy + swinger->ani->_movement->_oy);
		else
			swinger->ani->setOXY(ix - swinger->ix + swinger->ani->_ox, iy - swinger->iy + swinger->ani->_oy);

		swinger->ix = ix;
		swinger->iy = iy;

		if (!swinger->ani->_movement) {
			int mv = 0;

			if (swinger->sflags & 2) {
				mv = MV_KSL_SWINGBOY;
			} else if (swinger->sflags & 4) {
				mv = MV_KSL_SWINGGIRL;
			} else if (swinger->sflags & 0x20) {
				mv = MV_KSL_SWINGMAN;
			} else if (swinger->sflags & 1) {
				mv = MV_KSL_SWING;
			}

			if (mv)
				swinger->ani->startAnim(mv, 0, -1);

			if (swinger->ani->_movement)
				swinger->ani->_movement->_counter = 0;
		}

		if (g_vars->scene18_wheelIsTurning) {
			if ((swinger->sflags & 2) && swinger->angle >= Common::deg2rad<double>(160.0) && oldangle < Common::deg2rad<double>(160.0)) {
				swinger->sflags = 8;
				swinger->ani->changeStatics2(ST_KSL_BOY);
				swinger->ani->startAnim(MV_KSL_JUMPBOY, 0, -1);
				g_vars->scene18_kidWheelPos = i;
			} else if ((swinger->sflags & 4) && swinger->angle >= Common::deg2rad<double>(162.0) && oldangle < Common::deg2rad<double>(162.0)) {
				swinger->sflags = 16;
				swinger->ani->changeStatics2(ST_KSL_GIRL);
				swinger->ani->startAnim(MV_KSL_JUMPGIRL, 0, -1);
				g_vars->scene18_kidWheelPos = i;
			} else if (g_vars->scene18_kidIsOnWheel) {
				if (g_vars->scene18_boyIsOnWheel > 0 && (swinger->sflags & 1) && swinger->angle >= Common::deg2rad<double>(185.0) && oldangle < Common::deg2rad<double>(185.0)) {
					g_vars->scene18_kidWheelPosTo = i;
					sceneHandler18and19_boyJumpTo();
				}
			} else if (g_vars->scene18_girlIsOnWheel > 0 && (swinger->sflags & 1) && swinger->angle >= Common::deg2rad<double>(187.0) && oldangle < Common::deg2rad<double>(187.0)) {
				g_vars->scene18_kidWheelPosTo = i;
				sceneHandler18and19_girlJumpTo();
			}

			if (swinger->angle >= Common::deg2rad<double>(200.0) && oldangle < Common::deg2rad<double>(200.0)) {
				if (g_vars->scene18_boyJumpedOff)
					g_vars->scene18_boyIsOnWheel++;

				g_vars->scene18_boyJumpedOff = false;
			}
		}

		if (g_vars->scene18_manIsReady && (swinger->sflags & 1) && swinger->angle >= Common::deg2rad<double>(83.0) && oldangle < Common::deg2rad<double>(83.0)) {
			g_vars->scene18_manWheelPosTo = i;
			sceneHandler18and19_manStandArmchair();
		}

		if (!g_vars->scene18_enteredTrubaRight)
			continue;

		if ((int)i == g_vars->scene18_manWheelPosTo) {
			if (swinger->angle >= Common::deg2rad<double>(170.0) && oldangle < Common::deg2rad<double>(170.0)) {
				g_nmi->_gameLoader->preloadScene(SC_18, TrubaRight);
			} else if (swinger->angle >= Common::deg2rad<double>(25.0) && oldangle < Common::deg2rad<double>(25.0)) {
				g_nmi->_gameLoader->preloadScene(SC_19, TrubaRight);
			} else if (swinger->angle >= Common::deg2rad<double>(270.0) && oldangle < Common::deg2rad<double>(270.0)) {
				g_nmi->_sceneRect.translate(1200, 0);
			}
		}

		if (g_vars->scene18_jumpDistance > 0) {
			if (swinger->sflags & 0x20) {
				double newa = (double)g_vars->scene18_jumpAngle * Common::deg2rad<double>(1.0);

				if (newa <= swinger->angle && oldangle < newa) {
					swinger->ani->changeStatics2(ST_KSL_MAN);
					swinger->ani->startAnim(MV_KSL_JUMPMAN, 0, -1);
					swinger->ani->_priority = 35;

					g_vars->scene18_manWheelPos = i;
				}
			}
		}
	}
}

void sceneHandler18and19_animateRiders() {
	for (uint i = 0; i < g_vars->scene18_swingers.size(); i++) {
		Swinger *swinger = g_vars->scene18_swingers[i];

		if (!swinger->ani->_movement) {
			int mv = 0;

			if (swinger->sflags & 2)
				mv = MV_KSL_SWINGBOY;
			else if (swinger->sflags & 4)
				mv = MV_KSL_SWINGGIRL;
			else if (swinger->sflags & 0x20)
				mv = MV_KSL_SWINGMAN;

			if (mv)
				swinger->ani->startAnim(mv, 0, -1);

			if (swinger->ani->_movement)
				swinger->ani->_movement->_counter = 0;
		}
	}
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
		g_vars->scene18_manIsReady = true;
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
			if (g_vars->scene18_enteredTrubaRight) {
				cmd->_messageKind = 0;
				break;
			}

			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (pic && pic->_id == PIC_SC18_DOMIN && g_vars->scene18_domino
					&& (g_vars->scene18_domino->_flags & 4) && g_nmi->_aniMan->isIdle()) {
					if (!(g_nmi->_aniMan->_flags & 0x100) && g_nmi->_msgObjectId2 != g_vars->scene18_domino->_id) {
						handleObjectInteraction(g_nmi->_aniMan, g_vars->scene18_domino, cmd->_param);
						cmd->_messageKind = 0;

						break;
					}
				}

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0)) {
						g_nmi->processArcade(cmd);

						g_vars->scene18_manIsReady = false;

						break;
					}
				}
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			g_vars->scene18_manY = g_nmi->_aniMan2->_oy;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();
		}

		if (g_vars->scene18_manIsReady && g_nmi->_aniMan->_movement)
			g_vars->scene18_manIsReady = false;

		if (g_vars->scene18_bridgeIsConvoluted) {
			if (!g_vars->scene18_wheelFlipper)
				sceneHandler18and19_drawRiders();

			g_vars->scene18_wheelFlipper = !g_vars->scene18_wheelFlipper;

			if (!g_vars->scene18_whirlgig->_movement) {
				g_vars->scene18_whirlgig->startAnim(MV_WHR18_SPIN, 0, -1);
				g_nmi->_behaviorManager->updateBehaviors();

				break;
			}
		} else {
			sceneHandler18and19_animateRiders();
		}

		g_nmi->_behaviorManager->updateBehaviors();

		break;

	default:
		break;
	}

	return 0;
}

void sceneHandler19_updateNumRides() {
	int numRides = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarAsInt(sO_DudeSwinged) + 1;

	if (numRides > 1) {
		g_nmi->setObjectState(sO_Girl, g_nmi->getObjectEnumState(sO_Girl, sO_IsSwinging));

		g_vars->scene18_kidIsOnWheel = 1;
		g_vars->scene18_girlIsOnWheel++;

		numRides = 0;
	}

	g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->setSubVarAsInt(sO_DudeSwinged, numRides);
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
		if (g_vars->scene18_enteredTrubaRight) {
			switch (g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY)) {
			case PIC_SC19_RTRUBA1:
				g_vars->scene18_jumpDistance = 1;
				g_vars->scene18_jumpAngle = 331;
				cmd->_messageKind = 0;
				break;

			case PIC_SC19_RTRUBA2:
				g_vars->scene18_jumpDistance = 2;
				g_vars->scene18_jumpAngle = 350;
				cmd->_messageKind = 0;
				break;

			case PIC_SC19_RTRUBA3:
				g_vars->scene18_jumpDistance = 3;
				g_vars->scene18_jumpAngle = 9;
				cmd->_messageKind = 0;
				break;

			default:
				g_vars->scene18_jumpDistance = -1;
				g_vars->scene18_jumpAngle = -1;
				cmd->_messageKind = 0;
				break;
			}
		} else {
			if (g_vars->scene19_enteredTruba3) {
				if (g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC19_RTRUBA3) {
					if (g_nmi->_aniMan->isIdle()) {
						if (!(g_nmi->_aniMan->_flags & 0x100)) {
							PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(PIC_SC19_RTRUBA31, 0);

							handleObjectInteraction(g_nmi->_aniMan, pic, cmd->_param);
							break;
						}
					}
				}
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			g_vars->scene18_manY = g_nmi->_aniMan2->_oy;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		if (g_vars->scene18_bridgeIsConvoluted) {
			if (!g_vars->scene18_wheelFlipper)
				sceneHandler18and19_drawRiders();

			g_vars->scene18_wheelFlipper = !g_vars->scene18_wheelFlipper;

			if (!g_vars->scene18_whirlgig->_movement) {
				g_vars->scene18_whirlgig->startAnim(MV_WHR19_SPIN, 0, -1);

				g_nmi->_behaviorManager->updateBehaviors();

				break;
			}
		} else {
			sceneHandler18and19_animateRiders();
		}

		g_nmi->_behaviorManager->updateBehaviors();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
