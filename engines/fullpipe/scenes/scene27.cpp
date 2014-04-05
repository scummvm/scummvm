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

#define DBG 0

namespace Fullpipe {

struct Bat {
	StaticANIObject *ani;
	int field_4;
	double power;
	double field_10;
	double currX;
	double currY;
	double powerCos;
	double powerSin;
};

void scene27_initScene(Scene *sc) {
	g_vars->scene27_hitZone = sc->getPictureObjectById(PIC_SC27_HITZONE2, 0);
	g_vars->scene27_driver = sc->getStaticANIObject1ById(ANI_VODILLA, -1);
	g_vars->scene27_maid = sc->getStaticANIObject1ById(ANI_MAID, -1);
	g_vars->scene27_batHandler = sc->getStaticANIObject1ById(ANI_BITAHANDLER, -1);

	g_vars->scene27_balls.numBalls = 0;
	g_vars->scene27_balls.pTail = 0;
	g_vars->scene27_balls.field_8 = 0;
	g_vars->scene27_balls.pHead = 0;
	g_vars->scene27_balls.cPlexLen = 10;

	free(g_vars->scene27_balls.cPlex);
	g_vars->scene27_balls.cPlex = 0;

	g_vars->scene27_bats.clear();
	g_vars->scene27_var07.clear();

	g_vars->scene27_driverHasVent = true;
	g_vars->scene27_bat = sc->getStaticANIObject1ById(ANI_BITA, -1);

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newbat = new StaticANIObject(g_vars->scene27_bat);

		Ball *runPtr = g_vars->scene27_balls.pTail;
		Ball *lastP = g_vars->scene27_balls.field_8;

		if (!g_vars->scene27_balls.pTail) {
			g_vars->scene27_balls.cPlex = (byte *)calloc(g_vars->scene27_balls.cPlexLen, sizeof(Ball));

			byte *p1 = g_vars->scene27_balls.cPlex + (g_vars->scene27_balls.cPlexLen - 1) * sizeof(Ball);

			if (g_vars->scene27_balls.cPlexLen - 1 < 0) {
				runPtr = g_vars->scene27_balls.pTail;
			} else {
				runPtr = g_vars->scene27_balls.pTail;

				for (int j = 0; j < g_vars->scene27_balls.cPlexLen; j++) {
					((Ball *)p1)->p1 = runPtr;
					runPtr = (Ball *)p1;

					p1 -= sizeof(Ball);
				}

				g_vars->scene27_balls.pTail = runPtr;
			}
		}

		g_vars->scene27_balls.pTail = runPtr->p0;
		runPtr->p1 = lastP;
		runPtr->p0 = 0;
		runPtr->ani = newbat;

		g_vars->scene27_balls.numBalls++;

		if (g_vars->scene27_balls.field_8)
			g_vars->scene27_balls.field_8->p0 = runPtr;
		else
			g_vars->scene27_balls.pHead = runPtr;

		g_vars->scene27_balls.field_8 = runPtr;

		sc->addStaticANIObject(newbat, 1);
	}

	g_vars->scene27_dudeIsAiming = false;
	g_vars->scene27_maxPhaseReached = false;
	g_vars->scene27_wipeIsNeeded = false;
	g_vars->scene27_driverPushedButton = false;
	g_vars->scene27_numLostBats = 0;
	g_vars->scene27_knockCount = 0;
	g_vars->scene27_launchPhase = 0;

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSwab)) {
		g_vars->scene27_maid->changeStatics2(ST_MID_SWAB2);
	} else if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithBroom)) {
		g_vars->scene27_maid->changeStatics2(ST_MID_BROOM);
	} else if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSpade)) {
		g_vars->scene27_maid->changeStatics2(ST_MID_SPADE);
	}

	g_fp->_currentScene = oldsc;

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE7);
}

int scene27_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene27_dudeIsAiming) {
		if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV)
			g_fp->_cursorId = PIC_CSR_ARCADE7_D;

	} else if (g_fp->_objectIdAtCursor == ANI_MAN) {
		if (g_vars->scene27_maxPhaseReached)
			if (g_fp->_cursorId == PIC_CSR_DEFAULT)
				g_fp->_cursorId = PIC_CSR_ITN;
	}

	return g_fp->_cursorId;
}

void sceneHandler27_driverGiveVent() {
	g_vars->scene27_driver->changeStatics2(ST_DRV_VENT);
	g_vars->scene27_driver->startMQIfIdle(QU_DRV_GIVEVENT, 1);

	g_vars->scene27_driverHasVent = false;

	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);
}

void sceneHandler27_winArcade() {
	if (g_fp->getObjectState(sO_Driver) == g_fp->getObjectEnumState(sO_Driver, sO_WithSteering)) {
		g_vars->scene27_dudeIsAiming = false;

		g_fp->_aniMan->_callback2 = 0;
		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);

		sceneHandler27_driverGiveVent();
	}
}

void sceneHandler27_takeVent() {
	if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSwab)) {
		if (g_vars->scene27_maid->isIdle()) {
			if (g_vars->scene27_maid->_flags & 4) {
				g_vars->scene27_maid->changeStatics2(ST_MID_SWAB2);
				g_vars->scene27_maid->startMQIfIdle(QU_MID_CLEANVENT, 1);
			}
		}
	}
}

void sceneHandler27_showNextBat() {
	if (g_vars->scene27_bat) {
		MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC27_SHOWBET), 0, 1);

		mq->replaceKeyCode(-1, g_vars->scene27_bat->_okeyCode);
		mq->chain(0);
	}

	g_vars->scene27_batHandler->_priority = 1045;
}

int sceneHandler27_updateScreenCallback() {
	int res;

	res = g_fp->drawArcadeOverlay(getGameLoaderInteractionController()->_flag24 == 0);

	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

void sceneHandler27_aniManCallback(int *phase) {
	if (!g_vars->scene27_maxPhaseReached) {
		if (*phase >= 5)
			g_vars->scene27_maxPhaseReached = true;
		else
			++*phase;
	}
}

void sceneHandler27_throwBat() {
	if (getGameLoaderInteractionController()->_flag24)
		g_fp->_updateScreenCallback = sceneHandler27_updateScreenCallback;

	g_fp->_aniMan->_callback2 = sceneHandler27_aniManCallback;

	g_fp->_aniMan->startAnim(MV_MAN27_THROWBET, 0, -1);

	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 0);

	g_vars->scene27_maxPhaseReached = false;

	g_vars->scene27_bat->hide();
}

void sceneHandler27_clickBat(ExCommand *cmd) {
	int bx = g_vars->scene27_bat->_ox - 5;
	int by = g_vars->scene27_bat->_oy - 71;

#if DBG
	sceneHandler27_throwBat();
	return;
#endif

	if (ABS(bx - g_fp->_aniMan->_ox) > 1 || ABS(by - g_fp->_aniMan->_oy) > 1
		|| g_fp->_aniMan->_movement || g_fp->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
		MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, bx, by, 1, ST_MAN_RIGHT);

		if (mq) {
			mq->addExCommandToEnd(cmd->createClone());

			postExCommand(g_fp->_aniMan->_id, 2, bx, by, 0, -1);
		}
	} else {
		sceneHandler27_throwBat();
	}
}

void sceneHandler27_maidSwab() {
#if DBG
	return;
#endif
	if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSwab))
		g_vars->scene27_maid->changeStatics2(ST_MID_SWAB);
}

void sceneHandler27_startBat(StaticANIObject *bat) {
	Bat *newbat = new Bat;

	newbat->power = g_vars->scene27_launchPhase * 2.5 + 8.0;
	newbat->field_10 = 0;
	newbat->ani = bat;
	newbat->powerCos = newbat->power * cos(0.0);
	newbat->powerSin = newbat->power * sin(0.0);
	newbat->currX = newbat->powerCos + (double)g_fp->_aniMan->_ox + 42.0;
	newbat->currY = newbat->powerSin + (double)g_fp->_aniMan->_oy + 58.0;

	bat->_statics = (Statics *)bat->_staticsList[0];
	bat->setOXY((int)newbat->currX, (int)newbat->currY);
	bat->_flags |= 4;

	g_vars->scene27_bats.push_back(newbat);

	sceneHandler27_maidSwab();
}

void sceneHandler27_startAiming() {
	g_vars->scene27_dudeIsAiming = false;
	g_vars->scene27_maxPhaseReached = false;

	g_fp->_aniMan->_callback2 = 0;

	g_vars->scene27_launchPhase = g_fp->_aniMan->_movement->_currDynamicPhaseIndex - 6;

	int phase = 21 - g_vars->scene27_launchPhase;

    if (phase < 14)
		phase = 14;

    if (phase > 20)
		phase = 20;

	g_fp->playSound(SND_27_044, 0);

	g_fp->_aniMan->_movement->setDynamicPhaseIndex(phase);
}

void sceneHandler27_initAiming(ExCommand *cmd) {
	g_vars->scene27_aimStartX = cmd->_x;
	g_vars->scene27_aimStartY = cmd->_y;
	g_vars->scene27_dudeIsAiming = true;
	g_vars->scene27_maxPhaseReached = false;
}

void sceneHandler27_aimDude() {
	int phase = (g_vars->scene27_aimStartX - g_fp->_mouseScreenPos.x) / 20 + 6;

	if (phase < 6)
		phase = 6;

	if (phase > 11)
		phase = 11;

	if (g_fp->_aniMan->_movement)
		g_fp->_aniMan->_movement->setDynamicPhaseIndex(phase);
}

void sceneHandler27_wipeDo() {
	for (uint i = 0; i < g_vars->scene27_bats.size(); i++) {
		if (g_vars->scene27_bats[i]->currX < 800.0) {
			g_vars->scene27_bats[i]->field_10 = atan2(800.0 - g_vars->scene27_bats[i]->currX, 520.0 - g_vars->scene27_bats[i]->currY);
			g_vars->scene27_bats[i]->power += 1.0;
		}
    }
}

bool sceneHandler27_batFallLogic(uint batn) {
	Bat *bat = g_vars->scene27_bats[batn];

	int y = (bat->currY - 458.0) * 0.4848484848484849 + 734.0;

	if (y >= bat->currX)
		return false;

	if (bat->currX - y > 15.0 || bat->ani->_statics->_staticsId == ST_BTA_FALL) {
		bat->ani->_priority = 2020;

		g_vars->scene27_var07.push_back(bat);

		if (batn != g_vars->scene27_var07.size() - 1)
			g_vars->scene27_var07.remove_at(batn);
    } else if (!bat->ani->_movement) {
		bat->ani->startAnim(MV_BTA_FALL, 0, -1);
    }

	return true;
}

bool sceneHandler27_batCalcDistance(int bat1, int bat2) {
	double at = atan2(g_vars->scene27_bats[bat1]->currX - g_vars->scene27_bats[bat2]->currX, g_vars->scene27_bats[bat1]->currY - g_vars->scene27_bats[bat2]->currY);
	double dy = g_vars->scene27_bats[bat1]->currY - g_vars->scene27_bats[bat2]->currY;
	double dx = g_vars->scene27_bats[bat1]->currX - g_vars->scene27_bats[bat2]->currX;
	double ay = cos(at);
	double ax = sin(at);

	return sqrt(ax * ax * 0.25 + ay * ay) * 54.0 > sqrt(dx * dx + dy * dy);
}

void sceneHandler27_knockBats(int bat1n, int bat2n) {
	Bat *bat1 = g_vars->scene27_bats[bat1n];
	Bat *bat2 = g_vars->scene27_bats[bat2n];

	if (0.0 != bat1->power) {
		double rndF = (double)g_fp->_rnd->getRandomNumber(32767) * 0.0000009155552842799158 - 0.015
			+ atan2(bat2->currX - bat1->currX, bat2->currY - bat1->currY);
		double rndCos = cos(rndF);
		double rndSin = sin(rndF);

		double pow1x = cos(bat1->field_10 - rndF) * (double)((int)(bat2->currX - bat1->currX) >= 0 ? 1 : -1) * bat1->power;
		double pow1y = sin(bat1->field_10 - rndF) * (double)((int)(bat2->currY - bat1->currY) >= 0 ? 1 : -1) * bat1->power;

		bat1->powerCos -= pow1x * 1.1;
		bat1->powerSin -= pow1y * 1.1;

		rndF = ((double)g_fp->_rnd->getRandomNumber(32767) * 0.0000009155552842799158 - 0.015
							   + atan2(bat1->currX - bat2->currX, bat1->currY - bat2->currY));
		double pow2x = cos(bat2->field_10 - rndF) * (double)((int)(bat1->currX - bat2->currX) >= 0 ? 1 : -1) * bat2->power;
		double pow2y = sin(bat2->field_10 - rndF) * (double)((int)(bat1->currY - bat2->currY) >= 0 ? 1 : -1) * bat2->power;

		bat2->powerCos -= pow2x * 1.1;
		bat2->powerSin -= pow2y * 1.1;

		double dy = bat1->currY - bat2->currY;
	    double dx = bat1->currX - bat2->currX;
		double dist = (sqrt(rndSin * rndSin * 0.25 + rndCos * rndCos) * 54.0 - sqrt(dx * dx + dy * dy)) / cos(rndF - bat1->field_10);
		bat1->currX -= cos(bat1->field_10) * (dist + 1.0);
		bat1->currY -= sin(bat1->field_10) * (dist + 1.0);
		bat1->powerCos += pow2x * 0.64;

		if (bat1->currX <= 500.0)
			bat1->powerSin = 0.0;
		else
			bat1->powerSin += pow2y * 0.64;

		bat1->field_10 = atan2(bat1->powerCos, bat1->powerSin);
		bat1->power = sqrt(bat1->powerCos * bat1->powerCos + bat1->powerSin * bat1->powerSin);
		bat2->powerCos += pow1x * 0.64;

		if (bat2->currX <= 500.0)
			bat2->powerSin = 0;
		else
			bat2->powerSin += pow1y * 0.64;

		bat2->field_10 = atan2(bat2->powerCos, bat2->powerSin);
		bat2->power = sqrt(bat2->powerCos * bat2->powerCos + bat2->powerSin * bat2->powerSin);

		g_fp->playSound(SND_27_026, 0);
	}
}

void sceneHandler27_batSetColors(int batn) {
	Bat *bat = g_vars->scene27_bats[batn];

	if (g_vars->scene27_hitZone->isPixelHitAtPos((int)bat->currX, (int)bat->currY) ) {
		if (bat->ani->_statics->_staticsId == ST_BTA_NORM) {
			if (!bat->ani->_movement)
				bat->ani->_statics = bat->ani->getStaticsById(ST_BTA_HILITE);
		}
	} else {
		if (bat->ani->_statics->_staticsId == ST_BTA_HILITE) {
			if (!bat->ani->_movement)
				bat->ani->_statics = bat->ani->getStaticsById(ST_BTA_NORM);
		}
	}
}

void sceneHandler27_driverPushButton() {
	if (g_fp->getObjectState(sO_Driver) == g_fp->getObjectEnumState(sO_Driver, sO_WithSteering)) {
		g_vars->scene27_driver->changeStatics2(ST_DRV_VENT);
		chainQueue(QU_DRV_PUSHBUTTON, 1);

		g_vars->scene27_driverPushedButton = true;
	} else {
		g_vars->scene27_driver->changeStatics2(ST_DRV_SITNOVENT);


		chainQueue(QU_DRV_PUSHBUTTON_NOVENT, 1);

		g_vars->scene27_driverPushedButton = true;
	}
}

void sceneHandler27_maidSwitchback() {
#ifndef DBG
	if (g_fp->getObjectState(sO_Maid) == g_fp->getObjectEnumState(sO_Maid, sO_WithSwab)) {
		g_vars->scene27_maid->changeStatics2(ST_MID_SWAB);
		g_vars->scene27_maid->startMQIfIdle(QU_MID_SWITCHBACK, 1);
	}
#endif
}

void sceneHandler27_batLogic() {
	if (g_vars->scene27_balls.numBalls) {
		g_vars->scene27_bat = g_vars->scene27_balls.pHead->ani;

		g_vars->scene27_balls.pHead = g_vars->scene27_balls.pHead->p0;

		if (g_vars->scene27_balls.pHead)
			g_vars->scene27_balls.pHead->p0->p1 = 0;
		else
			g_vars->scene27_balls.field_8 = 0;

		g_vars->scene27_balls.init(&g_vars->scene27_balls.pHead->p0);

		int mv;

		switch (g_vars->scene27_batHandler->_statics->_staticsId) {
		case ST_BTH_5:
			mv = MV_BTH_5_4;
			break;

		case ST_BTH_4:
			mv = MV_BTH_4_3;
			break;

		case ST_BTH_3:
			mv = MV_BTH_3_2;
			break;

		case ST_BTH_2:
			mv = MV_BTH_2_1;
			break;

		case ST_BTH_1:
			mv = MV_BTH_1_0;
			break;

		default:
			chainQueue(QU_SC27_RESTARTBETS, 1);

			getCurrSceneSc2MotionController()->setEnabled();
			getGameLoaderInteractionController()->enableFlag24();

			g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);

			return;
		}

		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

		mq->setFlags(mq->getFlags() | 1);

		ExCommand *ex = new ExCommand(g_vars->scene27_batHandler->_id, 1, mv, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(0, 17, MSG_SC27_CLICKBET, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);

		mq->chain(0);
	} else {
		g_vars->scene27_bat = 0;
	}
}

void sceneHandler27_calcWinArcade() {
	if (!g_vars->scene27_wipeIsNeeded && !g_vars->scene27_driverPushedButton) {
		int numHilite = 0;

		for (uint i = 0; i < g_vars->scene27_bats.size(); i++) {
			if (g_vars->scene27_bats[i]->ani->_statics->_staticsId == ST_BTA_HILITE)
				numHilite++;
		}

		if (numHilite >= 3) {
			if (g_fp->getObjectState(sO_Driver) == g_fp->getObjectEnumState(sO_Driver, sO_WithSteering)) {
				sceneHandler27_driverGiveVent();
				sceneHandler27_maidSwitchback();

				return;
			}
		}

		if (!g_vars->scene27_balls.numBalls) {
			sceneHandler27_driverPushButton();
			sceneHandler27_maidSwitchback();
			return;
		}

		sceneHandler27_batLogic();
	}

	sceneHandler27_maidSwitchback();
}

void sceneHandler27_regenBats() {
	g_vars->scene27_wipeIsNeeded = false;

	for (uint i = 0; i < g_vars->scene27_var07.size(); i++) {
		g_vars->scene27_var07[i]->ani->hide();

		Ball *runPtr = g_vars->scene27_balls.pTail;
		Ball *lastP = g_vars->scene27_balls.field_8;
		StaticANIObject *newbat = g_vars->scene27_var07[i]->ani;

		if (!g_vars->scene27_balls.pTail) {
			g_vars->scene27_balls.cPlex = (byte *)calloc(g_vars->scene27_balls.cPlexLen, sizeof(Ball));

			byte *p1 = g_vars->scene27_balls.cPlex + (g_vars->scene27_balls.cPlexLen - 1) * sizeof(Ball);

			if (g_vars->scene27_balls.cPlexLen - 1 < 0) {
				runPtr = g_vars->scene27_balls.pTail;
			} else {
				runPtr = g_vars->scene27_balls.pTail;

				for (int j = 0; j < g_vars->scene27_balls.cPlexLen; j++) {
					((Ball *)p1)->p1 = runPtr;
					runPtr = (Ball *)p1;

					p1 -= sizeof(Ball);
				}

				g_vars->scene27_balls.pTail = runPtr;
			}
		}

		g_vars->scene27_balls.pTail = runPtr->p0;
		runPtr->p1 = lastP;
		runPtr->p0 = 0;
		runPtr->ani = newbat;

		g_vars->scene27_balls.numBalls++;

		if (g_vars->scene27_balls.field_8)
			g_vars->scene27_balls.field_8->p0 = runPtr;
		else
			g_vars->scene27_balls.pHead = runPtr;

		g_vars->scene27_balls.field_8 = runPtr;
	}

	g_vars->scene27_var07.clear();

	sceneHandler27_batLogic();

	g_vars->scene27_driverPushedButton = false;
}

void sceneHandler27_animateBats() {
	int oldCount = g_vars->scene27_knockCount;

	g_vars->scene27_numLostBats = 0;
	g_vars->scene27_knockCount = 0;

	for (uint i = 0; i < g_vars->scene27_bats.size(); i++) {
		Bat *bat = g_vars->scene27_bats[i];

		bat->currX = cos(bat->field_10) * bat->power + bat->currX;
		bat->currY = sin(bat->field_10) * bat->power + bat->currY;

		bat->ani->setOXY((int)bat->currX, (int)bat->currY);
		bat->ani->_priority = (int)(600.0 - bat->currY);

		double powerDelta;

		if (cos(bat->field_10) >= 0.0 || bat->currX >= 362.0)
			powerDelta = bat->power * 0.035;
		else
			powerDelta = bat->power * 0.4;

		bat->power -= powerDelta;
		bat->powerCos = cos(bat->field_10) * bat->power;
		bat->powerSin = sin(bat->field_10) * bat->power;

		if (bat->power >= 0.5)
			g_vars->scene27_knockCount++;
		else
			bat->power = 0;

		sceneHandler27_batSetColors(i);

		if (!sceneHandler27_batFallLogic(i) && !g_vars->scene27_wipeIsNeeded) {
			for (uint j = 0; j < g_vars->scene27_bats.size(); j++) {
				if (i != j && sceneHandler27_batCalcDistance(i, j))
					sceneHandler27_knockBats(i, j);
			}
		}
	}

	for (uint i = 0; i < g_vars->scene27_var07.size(); i++) {
		Bat *bat = g_vars->scene27_var07[i];

		if (bat->currY >= 700.0) {
			g_vars->scene27_numLostBats++;
		} else {
			bat->currX = bat->powerCos + bat->currX;
			bat->currY = bat->powerSin + bat->currY;
			bat->ani->setOXY((int)bat->currX, (int)bat->currY);
			bat->powerSin = bat->powerSin + 1.0;
		}
	}

	if (oldCount != g_vars->scene27_knockCount && !g_vars->scene27_knockCount)
		sceneHandler27_calcWinArcade();

	if (g_vars->scene27_wipeIsNeeded) {
		if (g_vars->scene27_numLostBats == 5)
			sceneHandler27_regenBats();
	}
}

int sceneHandler27(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler27_winArcade();
		break;

	case MSG_SC27_TAKEVENT:
		sceneHandler27_takeVent();
		break;

	case MSG_SC27_SHOWNEXTBET:
		sceneHandler27_showNextBat();
		break;

	case MSG_SC27_HANDLERTOFRONT:
		g_vars->scene27_batHandler->_priority = 1005;
		break;

	case MSG_SC27_STARTWIPE:
		g_vars->scene27_wipeIsNeeded = true;

		g_fp->playSound(SND_27_027, 0);

		break;

	case MSG_SC27_CLICKBET:
		sceneHandler27_clickBat(cmd);
		break;

	case MSG_SC27_STARTBET:
		if (g_vars->scene27_bat)
			sceneHandler27_startBat(g_vars->scene27_bat);

		break;

	case 30:
		if (g_vars->scene27_dudeIsAiming)
			sceneHandler27_startAiming();

		break;

	case 29:
		if (g_fp->_aniMan == g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y)
			&& g_vars->scene27_maxPhaseReached)
			sceneHandler27_initAiming(cmd);

		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene27_dudeIsAiming)
			sceneHandler27_aimDude();

		if (g_vars->scene27_wipeIsNeeded) {
			sceneHandler27_wipeDo();

			if (!g_fp->_aniMan->_movement && g_fp->_aniMan->_statics->_staticsId == ST_MAN_RIGHT)
				g_fp->_aniMan->startAnim(MV_MAN27_FLOW, 0, -1);
		}

		sceneHandler27_animateBats();

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
