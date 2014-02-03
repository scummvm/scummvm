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
	g_vars->scene27_var01 = 200;
	g_vars->scene27_var02 = 200;
	g_vars->scene27_var03 = 300;
	g_vars->scene27_var04 = 300;
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

	g_vars->scene27_var15 = 1;
	g_vars->scene27_bat = sc->getStaticANIObject1ById(ANI_BITA, -1);

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newbat = new StaticANIObject(g_vars->scene27_bat);

		Ball *runPtr = g_vars->scene27_balls.pTail;
		Ball *lastP = g_vars->scene27_balls.field_8;

		if (!g_vars->scene27_balls.pTail) {
			g_vars->scene27_balls.cPlex = (Ball *)calloc(g_vars->scene27_balls.cPlexLen, sizeof(Ball));

			Ball *p1 = g_vars->scene27_balls.cPlex + (g_vars->scene27_balls.cPlexLen - 1) * sizeof(Ball);

			if (g_vars->scene27_balls.cPlexLen - 1 < 0) {
				runPtr = g_vars->scene27_balls.pTail;
			} else {
				runPtr = g_vars->scene27_balls.pTail;

				for (int j = 0; j < g_vars->scene27_balls.cPlexLen; j++) {
					p1->p1 = runPtr;
					runPtr = p1;

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

	g_vars->scene27_var08 = 0;
	g_vars->scene27_var09 = 0;
	g_vars->scene27_var10 = 0;
	g_vars->scene27_var11 = 0;
	g_vars->scene27_var12 = 0;
	g_vars->scene27_var13 = 0;
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

	if (g_vars->scene27_var08) {
		if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV)
			g_fp->_cursorId = PIC_CSR_ARCADE7_D;

	} else if (g_fp->_objectIdAtCursor == ANI_MAN) {
		if (g_vars->scene27_var09)
			if (g_fp->_cursorId == PIC_CSR_DEFAULT)
				g_fp->_cursorId = PIC_CSR_ITN;
	}

	return g_fp->_cursorId;
}

void sceneHandler27_driverGiveVent() {
	g_vars->scene27_driver->changeStatics2(ST_DRV_VENT);
	g_vars->scene27_driver->startMQIfIdle(QU_DRV_GIVEVENT, 1);

	g_vars->scene27_var15 = 0;

	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();

	g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);
}

void sceneHandler27_winArcade() {
	if (g_fp->getObjectState(sO_Driver) == g_fp->getObjectEnumState(sO_Driver, sO_WithSteering)) {
		g_vars->scene27_var08 = 0;

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
	if (!g_vars->scene27_var09) {
		if (*phase >= 5)
			g_vars->scene27_var09 = 1;
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

	g_vars->scene27_var09 = 0;

	g_vars->scene27_bat->hide();
}

void sceneHandler27_clickBat(ExCommand *cmd) {
	int bx = g_vars->scene27_bat->_ox - 5;
	int by = g_vars->scene27_bat->_oy - 71;

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
	g_vars->scene27_var08 = 0;
	g_vars->scene27_var09 = 0;

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

void sceneHandler27_sub04(ExCommand *cmd) {
	g_vars->scene27_var16 = cmd->_x;
	g_vars->scene27_var17 = cmd->_y;
	g_vars->scene27_var08 = 1;
	g_vars->scene27_var09 = 0;
}

void sceneHandler27_aimDude() {
	int phase = (g_vars->scene27_var16 - g_fp->_mouseScreenPos.x) / 20 + 6;

	if (phase < 6)
		phase = 6;

	if (phase > 11)
		phase = 11;

	if (g_fp->_aniMan->_movement)
		g_fp->_aniMan->_movement->setDynamicPhaseIndex(phase);
}

void sceneHandler27_sub07() {
	for (uint i = 0; i < g_vars->scene27_bats.size(); i++) {
		if (g_vars->scene27_bats[i]->currX < 800.0 ) {
			g_vars->scene27_bats[i]->field_10 = atan2(800.0 - g_vars->scene27_bats[i]->currX, 520.0 - g_vars->scene27_bats[i]->currY);
			g_vars->scene27_bats[i]->power += 1.0;
		}
    }
}

bool sceneHandler27_batFallLogic(int bat) {
	warning("STUB: sceneHandler27_batFallLogic()");

	return false;
}

bool sceneHandler27_batCalcDistance(int bat1, int bat2) {
	warning("STUB: sceneHandler27_batCalcDistance()");

	return false;
}

void sceneHandler27_knockBats(int bat1, int bat2) {
	warning("STUB: sceneHandler27_knockBats()");
}

void sceneHandler27_batSetColors(int bat) {
	warning("STUB: sceneHandler27_batSetColors()");
}

void sceneHandler27_calcWinArcade() {
	warning("STUB: sceneHandler27_calcWinArcade()");
}

void sceneHandler27_sub02() {
	warning("STUB: sceneHandler27_sub02()");
}

void sceneHandler27_animateBats() {
	int oldCount = g_vars->scene27_var13;

	g_vars->scene27_var12 = 0;
	g_vars->scene27_var13 = 0;

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
			g_vars->scene27_var13++;
		else
			bat->power = 0;

		sceneHandler27_batSetColors(i);

		if (!sceneHandler27_batFallLogic(i) && !g_vars->scene27_var10) {
			for (uint j = 0; j < g_vars->scene27_bats.size(); j++) {
				if (i != j && sceneHandler27_batCalcDistance(i, j))
					sceneHandler27_knockBats(i, j);
			}
		}
	}

	for (uint i = 0; i < g_vars->scene27_var07.size(); i++) {
		Bat *bat = g_vars->scene27_var07[i];

		if (bat->currY >= 700.0) {
			g_vars->scene27_var12++;
		} else {
			bat->currX = bat->powerCos + bat->currX;
			bat->currY = bat->powerSin + bat->currY;
			bat->ani->setOXY((int)bat->currX, (int)bat->currY);
			bat->powerSin = bat->powerSin + 1.0;
		}
	}
	if (oldCount != g_vars->scene27_var13 && !g_vars->scene27_var13)
		sceneHandler27_calcWinArcade();

	if (g_vars->scene27_var10) {
		if (g_vars->scene27_var12 == 5)
			sceneHandler27_sub02();
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
		g_vars->scene27_var10 = 1;

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
		if (g_vars->scene27_var08)
			sceneHandler27_startAiming();

		break;

	case 29:
		if (g_fp->_aniMan == g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y)
			&& g_vars->scene27_var09)
			sceneHandler27_sub04(cmd);

		break;

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene27_var01)
				g_fp->_currentScene->_x = x - g_vars->scene27_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene27_var01)
				g_fp->_currentScene->_x = x + g_vars->scene27_var03 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene27_var08)
			sceneHandler27_aimDude();

		if (g_vars->scene27_var10) {
			sceneHandler27_sub07();

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
