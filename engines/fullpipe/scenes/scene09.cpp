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

struct Hanger {
	StaticANIObject *ani;
	int field_4;
	int field_8;
	int phase;
};

void scene09_setupGrit(Scene *sc) {
	if (g_vars->scene09_grit->_statics->_staticsId == ST_GRT9_GRIT) {
		if (!getGameLoaderInventory()->getCountItemsWithId(ANI_INV_COIN)) {
			if (g_fp->getObjectState(sO_CoinSlot_1) == g_fp->getObjectEnumState(sO_CoinSlot_1, sO_Empty)
				&& (g_vars->swallowedEgg1->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg2->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg3->_value.intValue == ANI_INV_EGGBOOT)) {
				Scene *oldsc = g_fp->_currentScene;
				g_fp->_currentScene = sc;
				g_vars->scene09_grit->changeStatics2(ST_GRT9_NORM);
				g_fp->_currentScene = oldsc;
			}
		}
	}
}

void scene09_initScene(Scene *sc) {
	g_vars->scene09_flyingBall = 0;
	g_vars->scene09_numSwallenBalls = 0;
	g_vars->scene09_gulper = sc->getStaticANIObject1ById(ANI_GLOTATEL, -1);
	g_vars->scene09_spitter = sc->getStaticANIObject1ById(ANI_PLEVATEL, -1);
	g_vars->scene09_grit = sc->getStaticANIObject1ById(ANI_GRIT_9, -1);
	g_vars->scene09_gulperIsPresent = true;
	g_vars->scene09_dudeIsOnLadder = false;
	g_vars->scene09_interactingHanger = -1;
	g_vars->scene09_intHangerPhase = -1;
	g_vars->scene09_intHangerMaxPhase = -1000;

	g_vars->scene09_balls.cPlexLen = 10;
	g_vars->scene09_flyingBalls.cPlexLen = 10;

	while (g_vars->scene09_balls.numBalls) {
		Ball *b = g_vars->scene09_balls.pHead->p0;

		g_vars->scene09_balls.pHead = g_vars->scene09_balls.pHead->p0;

		if (g_vars->scene09_balls.pHead)
			g_vars->scene09_balls.pHead->p0->p1 = 0;
		else
			g_vars->scene09_balls.field_8 = 0;

		g_vars->scene09_balls.init(&b);
	}

	g_vars->scene09_hangers.clear();
	g_vars->scene09_numMovingHangers = 4;

	StaticANIObject *hanger = sc->getStaticANIObject1ById(ANI_VISUNCHIK, -1);
	Hanger *hng = new Hanger;

	hng->ani = hanger;
	hng->phase = 0;
	hng->field_4 = 0;
	hng->field_8 = 0;

	g_vars->scene09_hangers.push_back(hng);

	int x = 75;

	for (int i = 1; x < 300; i++, x += 75) {
		StaticANIObject *ani = new StaticANIObject(hanger);

		ani->show1(x + hanger->_ox, hanger->_oy, MV_VSN_CYCLE2, 0);
		sc->addStaticANIObject(hanger, 1);

		hng = new Hanger;

		hng->ani = ani;
		hng->phase = 0;
		hng->field_4 = 0;
		hng->field_8 = 0;

		g_vars->scene09_hangers.push_back(hng);
	}

	while (g_vars->scene09_flyingBalls.numBalls) {
		Ball *ohead = g_vars->scene09_flyingBalls.pHead;

		g_vars->scene09_flyingBalls.pHead = g_vars->scene09_flyingBalls.pHead->p0;

		if (g_vars->scene09_flyingBalls.pHead)
			ohead->p0->p1 = 0;
		else
			g_vars->scene09_flyingBalls.field_8 = 0;

		ohead->p0 = g_vars->scene09_flyingBalls.pTail;

		g_vars->scene09_flyingBalls.pTail = ohead;

		g_vars->scene09_flyingBalls.numBalls--;
	}

	g_vars->scene09_flyingBalls.reset();

	Ball *b9 = g_vars->scene09_flyingBalls.sub04(g_vars->scene09_flyingBalls.field_8, 0);

	b9->ani = sc->getStaticANIObject1ById(ANI_BALL9, -1);
	b9->ani->setAlpha(0xc8);

	if (g_vars->scene09_flyingBalls.field_8) {
		g_vars->scene09_flyingBalls.field_8->p0 = b9;
		g_vars->scene09_flyingBalls.field_8 = b9;
	} else {
		g_vars->scene09_flyingBalls.pHead = b9;
		g_vars->scene09_flyingBalls.field_8 = b9;
	}

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newball = new StaticANIObject(b9->ani);

		newball->setAlpha(0xc8);

		Ball *runPtr = g_vars->scene09_flyingBalls.pTail;
		Ball *lastP = g_vars->scene09_flyingBalls.field_8;

		if (!g_vars->scene09_flyingBalls.pTail) {
			g_vars->scene09_flyingBalls.cPlex = (byte *)calloc(g_vars->scene09_flyingBalls.cPlexLen, sizeof(Ball));

			byte *p1 = g_vars->scene09_flyingBalls.cPlex + (g_vars->scene09_flyingBalls.cPlexLen - 1) * sizeof(Ball);

			if (g_vars->scene09_flyingBalls.cPlexLen - 1 < 0) {
				runPtr = g_vars->scene09_flyingBalls.pTail;
			} else {
				runPtr = g_vars->scene09_flyingBalls.pTail;

				for (int j = 0; j < g_vars->scene09_flyingBalls.cPlexLen; j++) {
					((Ball *)p1)->p1 = runPtr;
					runPtr = (Ball *)p1;

					p1 -= sizeof(Ball);
				}

				g_vars->scene09_flyingBalls.pTail = runPtr;
			}
		}

		g_vars->scene09_flyingBalls.pTail = runPtr->p0;
		runPtr->p1 = lastP;
		runPtr->p0 = 0;
		runPtr->ani = newball;

		g_vars->scene09_flyingBalls.numBalls++;

		if (g_vars->scene09_flyingBalls.field_8)
			g_vars->scene09_flyingBalls.field_8->p0 = runPtr;
		else
			g_vars->scene09_flyingBalls.pHead = runPtr;

		g_vars->scene09_flyingBalls.field_8 = runPtr;

		sc->addStaticANIObject(newball, 1);
	}

	g_fp->setObjectState(sO_RightStairs_9, g_fp->getObjectEnumState(sO_RightStairs_9, sO_IsClosed));

	GameVar *eggvar = g_fp->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = eggvar->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = eggvar->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = eggvar->getSubVarByName(sO_Egg3);

	scene09_setupGrit(sc);

	g_fp->initArcadeKeys("SC_9");

	g_fp->lift_setButton(sO_Level1, ST_LBN_1N);

	g_fp->setArcadeOverlay(PIC_CSR_ARCADE4);
}

int sceneHandler09_updateScreenCallback() {
	int res = g_fp->drawArcadeOverlay(g_fp->_objectIdAtCursor == ANI_VISUNCHIK || g_vars->scene09_interactingHanger >= 0);

	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

int scene09_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene09_interactingHanger < 0) {
		if (g_fp->_objectIdAtCursor == ANI_VISUNCHIK) {
			if (g_fp->_cursorId == PIC_CSR_ITN)
				g_fp->_updateScreenCallback = sceneHandler09_updateScreenCallback;
		} else {
			if (g_fp->_objectIdAtCursor == PIC_SC9_LADDER_R && g_fp->_cursorId == PIC_CSR_ITN)
				g_fp->_cursorId = (g_vars->scene09_dudeY < 350) ? PIC_CSR_GOD : PIC_CSR_GOU;
		}
	} else {
		g_fp->_cursorId = PIC_CSR_ITN;
	}

	return g_fp->_cursorId;
}

void sceneHandler09_winArcade() {
	if (g_vars->scene09_gulper->_flags & 4) {
		g_vars->scene09_gulper->changeStatics2(ST_GLT_SIT);
		g_vars->scene09_gulper->startAnim(MV_GLT_FLYAWAY, 0, -1);

		g_fp->setObjectState(sO_Jug, g_fp->getObjectEnumState(sO_Jug, sO_Unblocked));
		g_fp->setObjectState(sO_RightStairs_9, g_fp->getObjectEnumState(sO_RightStairs_9, sO_IsOpened));

		g_vars->scene09_gulperIsPresent = false;
	}
}

void sceneHandler09_startAuntie() {
	MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_TTA9_GOL), 0, 1);

	mq->getExCommandByIndex(0)->_x = g_fp->_sceneRect.right + 30;
	mq->chain(0);
}

void sceneHandler09_spitterClick() {
	if (g_vars->scene09_spitter->_flags & 4) {
		PicAniInfo info;

		g_vars->scene09_spitter->getPicAniInfo(&info);
		g_vars->scene09_spitter->_messageQueueId = 0;
		g_vars->scene09_spitter->changeStatics2(ST_PLV_SIT);

		int x = g_vars->scene09_spitter->_ox - 10;
		int y = g_vars->scene09_spitter->_oy + 145;

		g_vars->scene09_spitter->setPicAniInfo(&info);

		if (ABS(x - g_fp->_aniMan->_ox) > 1 || ABS(y - g_fp->_aniMan->_oy) > 1) {
			MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, x, y, 1, ST_MAN_UP);

			if (mq) {
				ExCommand *ex = new ExCommand(0, 17, MSG_SC9_PLVCLICK, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags = 2;
				mq->addExCommandToEnd(ex);

				postExCommand(g_fp->_aniMan->_id, 2, x, y, 0, -1);
			}
		} else {
			if (!g_fp->_aniMan->_movement) {
				g_vars->scene09_spitter->changeStatics2(ST_PLV_SIT);
				g_vars->scene09_spitter->hide();

				g_fp->_aniMan->startAnim(MV_MAN9_SHOOT, 0, -1);

				g_fp->stopAllSoundInstances(SND_9_006);
			}

			g_fp->_aniMan2 = 0;

			if (g_fp->_sceneRect.left < 800)
				g_fp->_currentScene->_x = 800 - g_fp->_sceneRect.left;
		}
	}
}

void sceneHandler09_eatBall() {
	if (g_vars->scene09_flyingBall) {
		g_vars->scene09_flyingBall->hide();

		Ball *ball = g_vars->scene09_balls.pHead;

		if (ball) {
			while (ball && ball->ani != g_vars->scene09_flyingBall)
				ball = ball->p0;

			if (ball) {
				if (ball == g_vars->scene09_balls.pHead)
					g_vars->scene09_balls.pHead = ball->p0;
				else
					ball->p1->p0 = ball->p0;

				if (ball == g_vars->scene09_balls.field_8)
					g_vars->scene09_balls.field_8 = ball->p1;
				else
					ball->p0->p1 = ball->p1;

				ball->p0 = g_vars->scene09_balls.pTail;
				g_vars->scene09_balls.pTail = ball;

				g_vars->scene09_balls.numBalls--;

				if (!g_vars->scene09_balls.numBalls)
					g_vars->scene09_balls.reset();
			}
		}

		ball = g_vars->scene09_flyingBalls.sub04(g_vars->scene09_flyingBalls.field_8, 0);
		ball->ani = g_vars->scene09_flyingBall;

		if (g_vars->scene09_flyingBalls.field_8)
			g_vars->scene09_flyingBalls.field_8->p0 = ball;
		else
			g_vars->scene09_flyingBalls.pHead = ball;

		g_vars->scene09_flyingBalls.field_8 = ball;

		g_vars->scene09_flyingBall = 0;
		g_vars->scene09_numSwallenBalls++;

		if (g_vars->scene09_numSwallenBalls >= 3) {
			MessageQueue *mq = g_vars->scene09_gulper->getMessageQueue();

			if (mq) {
				ExCommand *ex = new ExCommand(ANI_GLOTATEL, 1, MV_GLT_FLYAWAY, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 2;

				mq->addExCommandToEnd(ex);
			}

			g_fp->setObjectState(sO_Jug, g_fp->getObjectEnumState(sO_Jug, sO_Unblocked));
			g_fp->setObjectState(sO_RightStairs_9, g_fp->getObjectEnumState(sO_RightStairs_9, sO_IsOpened));

			g_vars->scene09_gulperIsPresent = false;
		}
	}
}

void sceneHandler09_showBall() {
	if (g_vars->scene09_flyingBalls.numBalls) {
		StaticANIObject *ani = g_vars->scene09_flyingBalls.pHead->ani;
		Ball *ph = g_vars->scene09_flyingBalls.pHead;
		g_vars->scene09_flyingBalls.pHead = ph->p0;

		if (g_vars->scene09_flyingBalls.pHead)
			ph->p0->p1 = 0;
		else
			g_vars->scene09_flyingBalls.field_8 = 0;

		ph->p0 = g_vars->scene09_flyingBalls.pTail;

		g_vars->scene09_flyingBalls.pTail = ph;
		g_vars->scene09_flyingBalls.numBalls--;

		if (!g_vars->scene09_flyingBalls.numBalls) {
			g_vars->scene09_flyingBalls.numBalls = 0;
			g_vars->scene09_flyingBalls.pTail = 0;
			g_vars->scene09_flyingBalls.field_8 = 0;
			g_vars->scene09_flyingBalls.pHead = 0;

			free(g_vars->scene09_flyingBalls.cPlex);
			g_vars->scene09_flyingBalls.cPlex = 0;
		}

		Ball *ball = g_vars->scene09_balls.sub04(g_vars->scene09_balls.field_8, 0);
		ball->ani = ani;

		if (g_vars->scene09_balls.field_8)
			g_vars->scene09_balls.field_8->p0 = ball;
		else
			g_vars->scene09_balls.pHead = ball;

		g_vars->scene09_balls.field_8 = ball;

		ani->show1(g_fp->_aniMan->_ox + 94, g_fp->_aniMan->_oy - 162, MV_BALL9_EXPLODE, 0);
	}
}

void sceneHandler09_cycleHangers() {
	for (int i = 0; i < g_vars->scene09_numMovingHangers; i++) {
		Movement *mov = g_vars->scene09_hangers[i]->ani->_movement;

		if (mov && mov->_id == MV_VSN_CYCLE2) {
			int idx;

			if (g_vars->scene09_hangers[i]->phase >= 0)
				idx = 18 - g_vars->scene09_hangers[i]->phase / 5;
			else
				idx = 18 - g_vars->scene09_hangers[i]->phase * 10 / 43;

			if (idx > 38)
				idx = 38;

			if (idx < 1)
				idx = 1;

			mov->setDynamicPhaseIndex(idx);
		}
	}
}

void sceneHandler09_limitHangerPhase() {
	for (int i = 0; i < g_vars->scene09_numMovingHangers; i++) {
		if (i != g_vars->scene09_interactingHanger) {
			g_vars->scene09_hangers[i]->phase += g_vars->scene09_hangers[i]->field_8;

			if (g_vars->scene09_hangers[i]->phase > 85)
				g_vars->scene09_hangers[i]->phase = 85;

			if (g_vars->scene09_hangers[i]->phase < -85)
				g_vars->scene09_hangers[i]->phase = -85;

			if (g_vars->scene09_hangers[i]->phase < 0)
				g_vars->scene09_hangers[i]->field_8++;

			if (g_vars->scene09_hangers[i]->phase > 0)
				g_vars->scene09_hangers[i]->field_8--;
		}
    }
}

void sceneHandler09_collideBall(Ball *ball) {
	if (g_vars->scene09_gulperIsPresent) {
		g_vars->scene09_flyingBall = ball->ani;

		if (g_vars->scene09_gulper) {
			g_vars->scene09_gulper->changeStatics2(ST_GLT_SIT);

			MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC9_EATBALL), 0, 0);

			mq->setFlags(mq->getFlags() | 1);

			if (!mq->chain(g_vars->scene09_gulper))
				delete mq;
		}
	}
}

void sceneHandler09_ballExplode(Ball *ball) {
	if (ball == g_vars->scene09_balls.pHead)
		g_vars->scene09_balls.pHead = ball->p0;
	else
		ball->p1->p0 = ball->p0;

	if (ball == g_vars->scene09_balls.field_8)
		g_vars->scene09_balls.field_8 = ball->p1;
	else
		ball->p0->p1 = ball->p1;

	ball->p0 = g_vars->scene09_balls.pTail;

	g_vars->scene09_balls.pTail = ball;
	g_vars->scene09_balls.numBalls--;

	if (!g_vars->scene09_balls.numBalls) {
		g_vars->scene09_balls.pTail = 0;
		g_vars->scene09_balls.field_8 = 0;
		g_vars->scene09_balls.pHead = 0;
		free(g_vars->scene09_balls.cPlex);
		g_vars->scene09_balls.cPlex = 0;
	}

	MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC9_BALLEXPLODE), 0, 1);

	mq->replaceKeyCode(-1, ball->ani->_okeyCode);

	if (!mq->chain(ball->ani))
		delete mq;

	Ball *runPtr = g_vars->scene09_flyingBalls.pTail;
	Ball *lastP = g_vars->scene09_flyingBalls.field_8;

	if (!g_vars->scene09_flyingBalls.pTail) {
		g_vars->scene09_flyingBalls.cPlex = (byte *)calloc(g_vars->scene09_flyingBalls.cPlexLen, sizeof(Ball));

		byte *p1 = g_vars->scene09_flyingBalls.cPlex + (g_vars->scene09_flyingBalls.cPlexLen - 1) * sizeof(Ball);

		if (g_vars->scene09_flyingBalls.cPlexLen - 1 < 0) {
			runPtr = g_vars->scene09_flyingBalls.pTail;
		} else {
			runPtr = g_vars->scene09_flyingBalls.pTail;

			for (int j = 0; j < g_vars->scene09_flyingBalls.cPlexLen; j++) {
				((Ball *)p1)->p1 = runPtr;
				runPtr = (Ball *)p1;

				p1 -= sizeof(Ball);
			}

			g_vars->scene09_flyingBalls.pTail = runPtr;
		}
	}

	g_vars->scene09_flyingBalls.pTail = runPtr->p0;
	runPtr->p1 = lastP;
	runPtr->p0 = 0;
	runPtr->ani = ball->ani;

	g_vars->scene09_flyingBalls.numBalls++;

	if (g_vars->scene09_flyingBalls.field_8) {
		g_vars->scene09_flyingBalls.field_8->p0 = runPtr;
		g_vars->scene09_flyingBalls.field_8 = runPtr;
	} else {
		g_vars->scene09_flyingBalls.pHead = runPtr;
		g_vars->scene09_flyingBalls.field_8 = runPtr;
	}
}

void sceneHandler09_checkHangerCollide() {
	for (Ball *ball = g_vars->scene09_balls.pHead; ball; ball = ball->p0) {
		int newx = ball->ani->_ox + 5;

		ball->ani->setOXY(newx, ball->ani->_oy);

		if (newx <= 1398 || g_vars->scene09_flyingBall) {
			if (g_vars->scene09_gulperIsPresent)
				goto LABEL_11;
		} else if (g_vars->scene09_gulperIsPresent) {
			sceneHandler09_collideBall(ball);
			continue;
		}

		if (newx > 1600) {
			sceneHandler09_ballExplode(ball);
			continue;
		}

	LABEL_11:
		int pixel;

		for (int i = 0; i < g_vars->scene09_numMovingHangers; i++) {
			for (int j = 0; j < 4; j++) {
				g_vars->scene09_hangers[i]->ani->getPixelAtPos(newx + g_vars->scene09_hangerOffsets[j].x, ball->ani->_oy + g_vars->scene09_hangerOffsets[j].y, &pixel);

				if (pixel) {
					sceneHandler09_ballExplode(ball);
					break;
				}
			}
		}
	}
}

void sceneHandler09_hangerStartCycle() {
	StaticANIObject *ani = g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani;

	if (ani->_movement) {
		ani->startAnim(MV_VSN_CYCLE2, 0, -1);
		g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->field_8 = 0;
		g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase = g_vars->scene09_intHangerPhase + (g_fp->_mouseScreenPos.y - g_vars->scene09_clickY) / 2;

		if (g_vars->scene09_intHangerMaxPhase != -1000 && g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase != g_vars->scene09_intHangerMaxPhase) {
			ExCommand *ex = new ExCommand(0, 35, SND_9_019, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_14 = 1;
			ex->_excFlags |= 2;
			ex->postMessage();

			g_vars->scene09_intHangerMaxPhase = -1000;
		}
	} else {
		g_vars->scene09_interactingHanger = -1;
	}
}

int sceneHandler09(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler09_winArcade();
		break;

	case MSG_SC9_STARTTIOTIA:
		sceneHandler09_startAuntie();
		break;

	case MSG_SC9_FROMLADDER:
		getCurrSceneSc2MotionController()->setEnabled();
		getGameLoaderInteractionController()->enableFlag24();

		g_vars->scene09_dudeIsOnLadder = false;
		break;

	case MSG_SC9_TOLADDER:
		getCurrSceneSc2MotionController()->clearEnabled();
		getGameLoaderInteractionController()->disableFlag24();

		g_vars->scene09_dudeIsOnLadder = true;
		break;

	case MSG_SC9_PLVCLICK:
		sceneHandler09_spitterClick();
		break;

	case MSG_SC9_FLOWN:
		g_vars->scene09_gulperIsPresent = false;
		break;

	case MSG_SC9_EATBALL:
		sceneHandler09_eatBall();
		break;

	case MSG_SC9_SHOWBALL:
		sceneHandler09_showBall();
		break;

	case 33:
		{
			int res = 0;

			if (g_fp->_aniMan2) {
				int x = g_fp->_aniMan2->_ox;

				g_vars->scene09_dudeY = g_fp->_aniMan2->_oy;

				if (x < g_fp->_sceneRect.left + 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.left - 300;

				if (x > g_fp->_sceneRect.right - 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;

				res = 1;
			} else {
				if (g_fp->_aniMan->_movement && g_fp->_aniMan->_movement->_id != MV_MAN9_SHOOT)
					g_fp->_aniMan2 = g_fp->_aniMan;
			}

			sceneHandler09_cycleHangers();
			sceneHandler09_limitHangerPhase();
			sceneHandler09_checkHangerCollide();

			if (g_vars->scene09_interactingHanger >= 0)
				sceneHandler09_hangerStartCycle();

			g_fp->_behaviorManager->updateBehaviors();

			g_fp->startSceneTrack();

			return res;
		}

	case 30:
		if (g_vars->scene09_interactingHanger >= 0)  {
			if (ABS(g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase) < 15) {
				g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani->_callback2 = 0;
				g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani->changeStatics2(ST_VSN_NORMAL);
			}
		}

		g_vars->scene09_interactingHanger = -1;

		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (ani) {
				if (ani->_id == ANI_PLEVATEL) {
					sceneHandler09_spitterClick();
					break;
				}

				if (ani->_id == ANI_VISUNCHIK) {
					if (g_vars->scene09_numMovingHangers > 0) {
						int hng = 0;

						while (g_vars->scene09_hangers[hng]->ani != ani) {
							++hng;

							if (hng >= g_vars->scene09_numMovingHangers)
								break;
						}

						g_vars->scene09_interactingHanger = hng;
						g_vars->scene09_intHangerPhase = g_vars->scene09_hangers[hng]->phase;
						g_vars->scene09_intHangerMaxPhase = g_vars->scene09_hangers[hng]->phase;

						g_vars->scene09_clickY = cmd->_y;

						if (!g_vars->scene09_hangers[hng]->ani->_movement || g_vars->scene09_hangers[hng]->ani->_movement->_id != MV_VSN_CYCLE2) {
							g_vars->scene09_hangers[hng]->ani->changeStatics2(ST_VSN_NORMAL);
							g_vars->scene09_hangers[hng]->ani->startAnim(MV_VSN_CYCLE2, 0, -1);
							g_vars->scene09_hangers[hng]->ani->_callback2 = 0;
						}

						ExCommand *ex = new ExCommand(0, 35, SND_9_018, 0, 0, 0, 1, 0, 0, 0);

						ex->_field_14 = 1;
						ex->_excFlags |= 2;
						ex->postMessage();
					}

					break;
				}
			}

			if (g_vars->scene09_dudeIsOnLadder && g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC9_LADDER_R
				&& !cmd->_keyCode && !g_fp->_aniMan->_movement) {
				handleObjectInteraction(g_fp->_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC9_LADDER_R, 0), 0);
			}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1) || (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
						g_fp->processArcade(cmd);
				}
			}

			break;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
