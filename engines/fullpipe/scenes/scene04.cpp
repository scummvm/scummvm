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
#include "fullpipe/utils.h"
#include "fullpipe/gfx.h"
#include "fullpipe/scenes.h"
#include "fullpipe/messages.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/behavior.h"
#include "fullpipe/motion.h"

namespace Fullpipe {

static const int scene04_speakerPhases[] = {
	0, 1,  2,  3, -1, -1,
	0, 2,  3, -1, -1, -1,
	0, 2, -1, -1, -1, -1
};

void scene04_speakerCallback(int *phase) {
	if (g_vars->scene04_soundPlaying) {
		if (g_vars->scene04_speakerPhase >= 0) {
			*phase = scene04_speakerPhases[g_vars->scene04_speakerPhase + 6 * g_vars->scene04_speakerVariant];

			g_vars->scene04_speakerPhase++;

			if (scene04_speakerPhases[g_vars->scene04_speakerPhase + 6 * g_vars->scene04_speakerVariant] < 0) {
				g_vars->scene04_speakerPhase = 0;
				g_vars->scene04_speakerVariant = g_fullpipe->_rnd->getRandomNumber(2);
			}
		} else {
			++g_vars->scene04_speakerPhase;
		}
	}
}

void scene04_initScene(Scene *sc) {
	g_vars->scene04_dudeOnLadder = false;
	g_vars->scene04_bottle = sc->getPictureObjectById(PIC_SC4_BOTTLE, 0);
	g_vars->scene04_hand = sc->getStaticANIObject1ById(ANI_HAND, -1);
	g_vars->scene04_plank = sc->getStaticANIObject1ById(ANI_PLANK, -1);
	g_vars->scene04_clock = sc->getStaticANIObject1ById(ANI_CLOCK, -1);
	g_vars->scene04_spring = sc->getStaticANIObject1ById(ANI_SPRING, -1);
	g_vars->scene04_mamasha = sc->getStaticANIObject1ById(ANI_MAMASHA_4, -1);
	g_vars->scene04_boot = sc->getStaticANIObject1ById(ANI_SC4_BOOT, -1);
	g_vars->scene04_ladder = 0;

	StaticANIObject *koz = sc->getStaticANIObject1ById(ANI_KOZAWKA, -1);

	if (koz) {
		Movement *kozmov = koz->getMovementById(MV_KZW_JUMP);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_dynamicPhases.size() : kozmov->_dynamicPhases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setDynamicPhaseIndex(i);

				if (kozmov->_framePosOffsets) {
					g_vars->scene04_jumpingKozyawki[i] = *kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpingKozyawki[i] = kozmov->_somePoint;
				}
			}
		}

		kozmov = koz->getMovementById(MV_KZW_JUMPROTATE);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_dynamicPhases.size() : kozmov->_dynamicPhases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setDynamicPhaseIndex(i);

				if (kozmov->_framePosOffsets) {
					g_vars->scene04_jumpRotateKozyawki[i] = *kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpRotateKozyawki[i] = kozmov->_somePoint;
				}
			}
		}
	}

	Interaction *plank = getGameLoaderInteractionController()->getInteractionByObjectIds(ANI_PLANK, 0, 0);
	if (plank)
		plank->_flags |= 8;

	if (g_fullpipe->getObjectState(sO_Jar_4) == g_fullpipe->getObjectEnumState(sO_Jar_4, sO_UpsideDown)) {
		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_kozyawkiObjList.clear();

		sc->getPictureObjectById(PIC_SC4_BOTTLE, 0)->_flags &= 0xfffb;
		sc->getPictureObjectById(PIC_SC4_MASK, 0)->_flags &= 0xfffb;
		sc->getStaticANIObject1ById(ANI_SPRING, 0)->_flags &= 0xfffb;

		g_vars->scene04_var18 = 0;
		g_vars->scene04_var19 = 0;
	} else {
		StaticANIObject *spring = sc->getStaticANIObject1ById(ANI_SPRING, -1);

		if (spring)
			spring->_callback2 = 0;

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_BOTTLE, 0));
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_MASK, 0));

		g_vars->scene04_kozyawkiObjList.clear();

		if (koz) {
			koz->loadMovementsPixelData();

			koz->_statics = koz->getStaticsById(ST_KZW_EMPTY);
			koz->setOXY(0, 0);
			koz->hide();

			g_vars->scene04_kozyawkiObjList.push_back(koz);

			for (int i = 0; i < 6; i++) {
				StaticANIObject *koz1 = new StaticANIObject(koz);

				sc->addStaticANIObject(koz1, 1);
				koz1->_statics = koz->getStaticsById(ST_KZW_EMPTY);
				koz1->setOXY(0, 0);
				koz1->hide();
				g_vars->scene04_kozyawkiObjList.push_back(koz1);
			}
		}
		sc->getPictureObjectById(PIC_SC4_BOTTLE2, 0)->_flags &= 0xfffb;

		g_vars->scene04_var18 = 1;
		g_vars->scene04_var19 = 1;
	}

	g_vars->scene04_var02 = 0;
	g_vars->scene04_soundPlaying = false;
	g_vars->scene04_var04 = 0;
	g_vars->scene04_walkingKozyawka = 0;
	g_vars->scene04_bottleWeight = 2;
	g_vars->scene04_dynamicPhaseIndex = 0;

	g_vars->scene04_kozyawkiAni.clear();

	g_fullpipe->setObjectState(sO_LowerPipe, g_fullpipe->getObjectEnumState(sO_LowerPipe, sO_IsClosed));

	g_vars->scene04_var07 = 0;
	g_vars->scene04_var08 = 0;
	g_vars->scene04_coinPut = 0;
	g_vars->scene04_var09 = 0;
	g_vars->scene04_var10 = 0;
	g_vars->scene04_var11 = 0;
	g_vars->scene04_var12 = 0;
	g_vars->scene04_var13 = 1;
	g_vars->scene04_var14 = 0;
	g_vars->scene04_var15 = 1;

	if (g_fullpipe->getObjectState(sO_BigMumsy) != g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_Gone))
		g_vars->scene04_mamasha->hide();

	g_vars->scene04_speaker = sc->getStaticANIObject1ById(ANI_SPEAKER_4, -1);
	g_vars->scene04_speaker->_callback2 = scene04_speakerCallback;
	g_vars->scene04_speaker->startAnim(MV_SPK4_PLAY, 0, -1);

	g_vars->scene04_speakerVariant = 0;
	g_vars->scene04_speakerPhase = 0;

	g_fullpipe->initArcadeKeys("SC_4");
}

bool sceneHandler04_friesAreWalking() {
	if (g_vars->scene04_dudeOnLadder && g_fullpipe->_aniMan->isIdle() && !(g_fullpipe->_aniMan->_flags & 0x100)) {
		int col = g_vars->scene04_ladder->collisionDetection(g_fullpipe->_aniMan);
		if (col >= 3 && col <= 6 ) {
			Movement *koz;

			if (!g_vars->scene04_walkingKozyawka
				 || (koz = g_vars->scene04_walkingKozyawka->_movement) == 0
				 || koz->_id != MV_KZW_WALKPLANK
				 || koz->_currDynamicPhaseIndex < 10
				 || koz->_currDynamicPhaseIndex > 41)
				return true;
		}
	}

	return false;
}

int scene04_updateCursor() {
	g_fullpipe->updateCursorCommon();

	if (g_fullpipe->_objectIdAtCursor == PIC_SC4_LRTRUBA) {
		if (!g_vars->scene04_var19) {
			g_fullpipe->_cursorId = PIC_CSR_DEFAULT;

			return g_fullpipe->_cursorId;
		}
	} else if (g_fullpipe->_objectIdAtCursor == ANI_PLANK || g_fullpipe->_objectIdAtCursor == PIC_SC4_PLANK) {
		if (g_fullpipe->_objectIdAtCursor == ANI_PLANK && g_fullpipe->_cursorId != PIC_CSR_ITN)
			return g_fullpipe->_cursorId;

		if (g_fullpipe->_objectIdAtCursor == ANI_PLANK || (g_fullpipe->_objectIdAtCursor == PIC_SC4_PLANK && g_fullpipe->_cursorId == PIC_CSR_DEFAULT)) {
			if (sceneHandler04_friesAreWalking()) {
				g_fullpipe->_cursorId = PIC_CSR_ARCADE1;
				return g_fullpipe->_cursorId;
			}
			if (g_vars->scene04_soundPlaying) {
				g_fullpipe->_cursorId = PIC_CSR_DEFAULT;
				return g_fullpipe->_cursorId;
			}
		}
	}

	if (g_fullpipe->_objectIdAtCursor == PIC_CSR_ITN && g_fullpipe->_objectIdAtCursor == PIC_SC4_DOWNTRUBA)
		g_fullpipe->_cursorId = PIC_CSR_GOD;

	return g_fullpipe->_cursorId;
}

void sceneHandler04_checkBigBallClick() {
	StaticANIObject *ball = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (ball)
		for (uint i = 0; i < ball->_movements.size(); i++)
			((Movement *)ball->_movements[i])->_counterMax = 73;

	g_vars->scene04_var13 = 1;
}

void sceneHandler04_clickBottle() {
	if (!g_vars->scene04_var02)
		g_vars->scene04_var20 += 5;
}

void sceneHandler04_clickButton() {
	StaticANIObject *but = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BUTTON, -1);

	if (but) {
		if (!g_vars->scene04_clock->_movement || 
			(g_vars->scene04_clock->_movement->_id == MV_CLK_GO && g_vars->scene04_clock->_movement->_currDynamicPhaseIndex > 3 && 
			 g_vars->scene04_clock->_movement->_currDynamicPhaseIndex < 105)) {
			if (!g_vars->scene04_hand->_movement && !g_vars->scene04_var02) {
				but->startAnim(MV_BTN_CLICK, 0, -1);
				g_vars->scene04_hand->startAnim(MV_HND_POINT, 0, -1);
			}
		}
	}
}

void sceneHandler04_downLadder(int x, int y) {
	g_vars->scene04_ladder->method34(g_fullpipe->_aniMan, x + g_vars->scene04_ladder->_ladder_field_20, y + g_vars->scene04_ladder->_ladder_field_24, 0, 0);
}

void sceneHandler04_walkClimbLadder(ExCommand *ex) {
	MessageQueue *mq = new MessageQueue(g_fullpipe->_globalMessageQueueList->compact());

	ExCommand *ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex1->_keyCode = g_fullpipe->_aniMan->_okeyCode;
	ex1->_excFlags |= 2;

	mq->addExCommandToEnd(ex1);

	ExCommand *ex2 = new ExCommand(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex2->_keyCode = g_fullpipe->_aniMan->_okeyCode;
	ex2->_excFlags |= 2;

	mq->addExCommandToEnd(ex2);

	ExCommand *ex3;

	if (ex) {
		ex3 = new ExCommand(ex);
	} else {
		ex3 = new ExCommand(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex3->_excFlags |= 3;
	}

	mq->addExCommandToEnd(ex3);

	mq->setFlags(mq->getFlags() | 1);

	mq->chain(0);

	g_vars->scene04_dudeOnLadder = 1;

	g_vars->scene04_ladder = new MctlLadder;
	g_vars->scene04_ladder->_objId = MV_MAN_TURN_SUD;
	g_vars->scene04_ladder->_ladderY = 406;
	g_vars->scene04_ladder->_ladder_field_14 = 12;
	g_vars->scene04_ladder->_ladder_field_18 = 0;
	g_vars->scene04_ladder->_height = -40;
	g_vars->scene04_ladder->_ladder_field_20 = 0;
	g_vars->scene04_ladder->_ladder_field_24 = -60;

	g_vars->scene04_ladder->addObject(g_fullpipe->_aniMan);

	if (g_vars->scene04_soundPlaying) {
		g_vars->scene04_ladder->_movements.front()->movVars->varUpStart = MV_MAN_STARTLADDER2;
		g_vars->scene04_ladder->_movements.front()->movVars->varUpGo = MV_MAN_GOLADDER2;
		g_vars->scene04_ladder->_movements.front()->movVars->varUpStop = MV_MAN_STOPLADDER2;
		g_vars->scene04_ladder->_movements.front()->staticIds[2] = ST_MAN_GOLADDER2;
	} else {
		g_vars->scene04_ladder->_movements.front()->movVars->varUpStart = MV_MAN_STARTLADDER;
		g_vars->scene04_ladder->_movements.front()->movVars->varUpGo = MV_MAN_GOLADDER;
		g_vars->scene04_ladder->_movements.front()->movVars->varUpStop = MV_MAN_STOPLADDER;
		g_vars->scene04_ladder->_movements.front()->staticIds[2] = ST_MAN_GOLADDER;
	}

	g_fullpipe->_aniMan->_priority = 12;

	getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();
}

void sceneHandler04_clickLadder() {
	g_vars->scene04_dudePosX = g_fullpipe->_aniMan->_ox;
	g_vars->scene04_dudePosY = g_fullpipe->_aniMan->_oy;

	if (g_vars->scene04_dudeOnLadder) {
		if (!g_fullpipe->_aniMan->isIdle() || (g_fullpipe->_aniMan->_flags & 0x100)) {
			g_vars->scene04_var08 = 1;
		} else {
			int h3 = 3 * g_vars->scene04_ladder->_height;
			int half = abs(g_vars->scene04_ladder->_height) / 2;
			int start = g_vars->scene04_ladder->_ladderY - g_vars->scene04_ladder->_ladder_field_24;
			int min = 2 * h3 + start + half + 1;
			int max =     h3 + start - half - 1;

			if (g_vars->scene04_sceneClickY > max)
				g_vars->scene04_sceneClickY = max;

			if (g_vars->scene04_sceneClickY < min)
				g_vars->scene04_sceneClickY = min;

			sceneHandler04_downLadder(g_vars->scene04_sceneClickX, g_vars->scene04_sceneClickY);

			g_vars->scene04_var08 = 0;
		}
	} else {
		if (g_fullpipe->_aniMan->isIdle() && !(g_fullpipe->_aniMan->_flags & 0x100)) {
			if (abs(1095 - g_vars->scene04_dudePosX) > 1 || abs(434 - g_vars->scene04_dudePosY) > 1) {
				MessageQueue *mq = getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->method34(g_fullpipe->_aniMan, 1095, 434, 1, ST_MAN_UP);
				if (mq) {
					ExCommand *ex = new ExCommand(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);

					ex->_excFlags = 3;
					mq->addExCommandToEnd(ex);

					postExCommand(g_fullpipe->_aniMan->_id, 2, 1095, 434, 0, -1);
				}
			} else {
				sceneHandler04_walkClimbLadder(0);
			}
		}
	}
}

void sceneHandler04_jumpOnLadder() {
	if (g_fullpipe->_aniMan->_movement && g_fullpipe->_aniMan->_movement->_id != MV_MAN_LOOKLADDER)
		return;

	if (g_fullpipe->_aniMan->_statics->_staticsId != ST_MAN_STANDLADDER && g_fullpipe->_aniMan->_statics->_staticsId != ST_MAN_LADDERDOWN)
		return;

	g_fullpipe->_aniMan->changeStatics2(ST_MAN_LADDERDOWN);

	g_fullpipe->_aniMan->_flags |= 1;

	MGM mgm;
	MGMInfo mgminfo;

	mgm.addItem(ANI_MAN);

	mgminfo.ani = g_fullpipe->_aniMan;
	mgminfo.staticsId2 = ST_MAN_ONPLANK;
	mgminfo.x1 = 938;
	mgminfo.y1 = 442;
	mgminfo.field_1C = 10;
	mgminfo.field_10 = 1;
	mgminfo.flags = 78;
	mgminfo.movementId = MV_MAN_JUMPONPLANK;

	MessageQueue *mq = mgm.genMovement(&mgminfo);

	if (mq) {
		mq->_flags |= 1;

		if (!mq->chain(g_fullpipe->_aniMan))
			delete mq;

		g_fullpipe->_aniMan->_priority = 10;
	}

	g_vars->scene04_ladderOffset = g_vars->scene04_ladder->collisionDetection(g_fullpipe->_aniMan);
}

void sceneHandler04_clickPlank() {
	if (sceneHandler04_friesAreWalking())
		sceneHandler04_jumpOnLadder();
	else if (g_vars->scene04_dudeOnLadder)
		g_fullpipe->playSound(SND_4_033, 0);
	else if (!g_vars->scene04_soundPlaying)
		chainQueue(QU_PNK_CLICK, 0);
}

void sceneHandler04_dropBottle() {
	g_vars->scene04_var12 = 1;
	g_vars->scene04_bottleY = 10;
	g_vars->scene04_bottleWeight = 0;

	while (g_vars->scene04_kozyawkiAni.size()) {
		StaticANIObject *koz = g_vars->scene04_kozyawkiAni.front();
		g_vars->scene04_kozyawkiAni.pop_front();

		for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it)
			if (*it == koz) {
				g_vars->scene04_bottleObjList.erase(it);
				break;
			}

		koz->queueMessageQueue(0);
		koz->hide();

		g_vars->scene04_kozyawkiObjList.push_back(koz);
	}

	g_vars->scene04_hand->changeStatics2(ST_HND_EMPTY);

	g_vars->scene04_hand->setOXY(429, 21);
	g_vars->scene04_hand->_priority = 15;
}

void sceneHandler04_gotoLadder(ExCommand *ex) {
	MGM mgm;
	MGMInfo mgminfo;

	mgm.addItem(ANI_MAN);

	mgminfo.ani = g_fullpipe->_aniMan;
	mgminfo.staticsId2 = ST_MAN_UP;
	mgminfo.x1 = 1095;
	mgminfo.y1 = 434;
	mgminfo.field_1C = 12;
	mgminfo.field_10 = 1;
	mgminfo.flags = 78;
	mgminfo.movementId = MV_MAN_PLANKTOLADDER;

	MessageQueue *mq = mgm.genMovement(&mgminfo);

	if (mq) {
		mq->deleteExCommandByIndex(mq->getCount() - 1, 1);

		ExCommand *ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex1->_excFlags = 2;
		ex1->_field_24 = 1;
		ex1->_keyCode = -1;
		mq->addExCommandToEnd(ex1);

		ExCommand *ex2 = new ExCommand(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex2->_excFlags = 2;
		ex2->_field_24 = 1;
		ex2->_keyCode = -1;
		mq->addExCommandToEnd(ex2);

		ExCommand *ex3 = new ExCommand(g_fullpipe->_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
		ex3->_field_14 = 256;
		ex3->_messageNum = 0;
		ex3->_excFlags |= 3;
		mq->addExCommandToEnd(ex3);

		if (ex) {
			ExCommand *ex4 = new ExCommand(ex);

			mq->addExCommandToEnd(ex4);
		}

		mq->setFlags(mq->getFlags() | 1);

		if (mq->chain(g_fullpipe->_aniMan)) {
			g_fullpipe->_aniMan->_priority = 12;
			g_fullpipe->_aniMan->_flags |= 1;
		} else {
			delete mq;
		}
	}

	g_vars->scene04_var04 = 0;
}

void sceneHandler04_lowerPlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTRIGHT, 0, -1);
}

void sceneHandler04_manFromBottle() {
	for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it)
		if (*it == g_fullpipe->_aniMan) {
			g_vars->scene04_bottleObjList.erase(it);
			g_vars->scene04_bottleWeight -= 9;
			break;
		}

	if (g_vars->scene04_ladder)
		delete g_vars->scene04_ladder;

	g_vars->scene04_ladder = 0;

	getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();
}

void sceneHandler04_manToBottle() {
	g_vars->scene04_bottleObjList.push_back(g_fullpipe->_aniMan);
	g_vars->scene04_var20 = 5;
	g_vars->scene04_bottleWeight += 9;
	g_fullpipe->_aniMan2 = g_fullpipe->_aniMan;
	g_vars->scene04_var10 = 1;
}

void sceneHandler04_raisePlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTLEFT, 0, -1);
}

MessageQueue *sceneHandler04_kozFly3(StaticANIObject *ani, double phase) {
	warning("STUB: sceneHandler04_kozFly3()");

	return 0;
}

MessageQueue *sceneHandler04_kozFly5(StaticANIObject *ani, double phase) {
	warning("STUB: sceneHandler04_kozFly5()");

	return 0;
}

MessageQueue *sceneHandler04_kozFly6(StaticANIObject *ani) {
	warning("STUB: sceneHandler04_kozFly6()");

	return 0;
}

MessageQueue *sceneHandler04_kozFly7(StaticANIObject *ani, double phase) {
	warning("STUB: sceneHandler04_kozFly7()");

	return 0;
}

static const int kozTrajectory3[] = {
	3, 2, 0,
	3, 2, 0,
	3, 2, 0
};

static const int kozTrajectory4[] = {
	5, 3, 1,
	5, 4, 1,
	5, 3, 1
};

static const int kozTrajectory5[] = {
	6, 5, 4,
	6, 5, 4,
	6, 5, 4
};

static const int kozTrajectory6[] = {
	7, 6, 5,
	7, 6, 5,
	7, 6, 5
};

void sceneHandler04_shootKozyawka() {
	g_vars->scene04_plank->changeStatics2(ST_PNK_WEIGHTRIGHT);

	if (!g_vars->scene04_walkingKozyawka)
		return;

	if (g_vars->scene04_walkingKozyawka->_movement) {
		if (g_vars->scene04_walkingKozyawka->_movement->_id == MV_KZW_WALKPLANK) {
			int dphase = g_vars->scene04_walkingKozyawka->_movement->_currDynamicPhaseIndex;

			if (dphase < 41) {
				int col = 3 * dphase / 15;
				if (col > 2)
					col = 2;

				int row = g_vars->scene04_kozyawkiAni.size();
				if (row > 2)
					row = 2;

				int idx = 3 * row + col;
				int phase;

				if (g_vars->scene04_ladderOffset == 3) {
					phase = kozTrajectory3[idx];
				} else if (g_vars->scene04_ladderOffset == 4) {
					phase = kozTrajectory4[idx];
				} else {
					if (g_vars->scene04_ladderOffset == 5)
						phase = kozTrajectory5[idx];
					else
						phase = kozTrajectory6[idx];
				}

				g_vars->scene04_walkingKozyawka->queueMessageQueue(0);
				g_vars->scene04_walkingKozyawka->_movement = 0;
				g_vars->scene04_walkingKozyawka->_statics = g_vars->scene04_walkingKozyawka->getStaticsById(ST_KZW_RIGHT);

				MessageQueue *mq;

				if (phase > 2) {
					if (phase > 5) {
						if (phase == 6)
							mq = sceneHandler04_kozFly6(g_vars->scene04_walkingKozyawka);
						else
							mq = sceneHandler04_kozFly7(g_vars->scene04_walkingKozyawka, (double)(phase - 6) * 0.3333333333333333);
					} else {
						mq = sceneHandler04_kozFly5(g_vars->scene04_walkingKozyawka, (double)(phase - 2) * 0.3333333333333333);
					}
				} else {
					mq = sceneHandler04_kozFly3(g_vars->scene04_walkingKozyawka, (double)phase * 0.5);
				}

				if (mq) {
					g_vars->scene04_var24 = g_vars->scene04_walkingKozyawka;

					if (!mq->chain(g_vars->scene04_walkingKozyawka) )
						delete mq;
				}
			}
		}
	}

	if (g_vars->scene04_ladderOffset > 3)
		g_fullpipe->_aniMan->changeStatics1(ST_MAN_LOOKPLANK);

	g_vars->scene04_var04 = 1;
}

void sceneHandler04_showCoin() {
	StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_SC4_COIN, -1);

	if (ani) {
		ani->show1(MV_BDG_OPEN, MV_MAN_GOU, MV_SC4_COIN_default, 0);

		ani->_priority = 40;
	}
}

void sceneHandler04_stopSound() {
	warning("STUB: sceneHandler04_stopSound()");
}

void sceneHandler04_sub1(ExCommand *ex) {
	g_fullpipe->_aniMan->changeStatics2(ST_MAN_SIT);

	MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(QU_SC4_MANFROMBOTTLE), 0, 0);

	if (ex) {
		ExCommand *newex = new ExCommand(ex);

		mq->addExCommandToEnd(newex);
	  }

	mq->_flags |= 1;
	mq->chain(0);

	g_vars->scene04_var10 = 0;
	g_fullpipe->_behaviorManager->setFlagByStaticAniObject(g_fullpipe->_aniMan, 1);
}

void sceneHandler04_walkKozyawka() {
	if (g_vars->scene04_kozyawkiObjList.size()) {
		g_vars->scene04_walkingKozyawka = g_vars->scene04_kozyawkiObjList.front();
		g_vars->scene04_kozyawkiObjList.pop_front();

		MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(QU_KOZAW_WALK), 0, 1);
		mq->replaceKeyCode(-1, g_vars->scene04_walkingKozyawka->_okeyCode);
		mq->chain(0);
	}
}

void sceneHandler04_bottleUpdateObjects(int off) {
	for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it) {
		GameObject *obj = *it;

		obj->setOXY(obj->_ox, off + obj->_oy);
	}
}

void sceneHandler04_springWobble() {
	int oldDynIndex = g_vars->scene04_dynamicPhaseIndex;
	int newdelta = g_vars->scene04_var20 + g_vars->scene04_dynamicPhaseIndex;

	g_vars->scene04_dynamicPhaseIndex += g_vars->scene04_var20;

	if (newdelta < 0) {
		newdelta = 0;
		g_vars->scene04_dynamicPhaseIndex = 0;
		g_vars->scene04_var20 = 0;
	}

	if (newdelta > 14) {
		newdelta = 14;
		g_vars->scene04_dynamicPhaseIndex = 14;
		g_vars->scene04_var20 = 0;
	}

	if (g_vars->scene04_bottleWeight > newdelta)
		g_vars->scene04_var20++;

	if (g_vars->scene04_bottleWeight < newdelta)
		g_vars->scene04_var20--;

	if ((oldDynIndex > g_vars->scene04_bottleWeight && newdelta > g_vars->scene04_bottleWeight) || newdelta <= g_vars->scene04_bottleWeight) {
		g_vars->scene04_var25++;

		if (g_vars->scene04_var20 && g_vars->scene04_var25 > 1) {
			g_vars->scene04_var25 = 0;
			g_vars->scene04_var20 = g_vars->scene04_var20 - g_vars->scene04_var20 / abs(g_vars->scene04_var20);
		}
	}

	Common::Point point;

	if (g_vars->scene04_dynamicPhaseIndex) {
		if (!g_vars->scene04_spring->_movement)
			g_vars->scene04_spring->startAnim(MV_SPR_LOWER, 0, -1);

		g_vars->scene04_spring->_movement->setDynamicPhaseIndex(g_vars->scene04_dynamicPhaseIndex);
	} else {
		g_vars->scene04_spring->changeStatics2(ST_SPR_UP);
	}

	if (g_vars->scene04_dynamicPhaseIndex != oldDynIndex)
		sceneHandler04_bottleUpdateObjects(oldDynIndex - g_vars->scene04_dynamicPhaseIndex);
}

void sceneHandler04_leaveScene() {
	g_fullpipe->_aniMan2 = 0;

    MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(QU_SC4_MANTOBOTTLE), 0, 0);
	ExCommand *ex = 0;

	for (uint i = 0; i < mq->getCount(); i++) {
		if (mq->getExCommandByIndex(i)->_messageKind == 27) {
			ex = mq->getExCommandByIndex(i);
			break;
		}
	}

	ex->_y = g_vars->scene04_bottle->_oy - 304;

	mq->chain(0);

	g_vars->scene04_var07 = 0;
	g_vars->scene04_dudeOnLadder = 0;

	g_fullpipe->_behaviorManager->setFlagByStaticAniObject(g_fullpipe->_aniMan, 0);

	g_fullpipe->updateMapPiece(PIC_MAP_P03, 1);
}

void sceneHandler04_liftBottle() {
	int newy = g_vars->scene04_bottleY + g_vars->scene04_spring->_oy;

	g_vars->scene04_bottleY += 5;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_oy);

	g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, newy);

	if (g_vars->scene04_bottle->_oy >= 226) {
		sceneHandler04_bottleUpdateObjects(226 - g_vars->scene04_bottle->_oy);

		g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, 437);
		g_vars->scene04_var12 = 0;
		g_vars->scene04_var09 = 0;
		g_vars->scene04_var19 = 1;
		g_vars->scene04_bottleWeight = 2;
		g_vars->scene04_var20 = 10;
		g_vars->scene04_var02 = 0;

		g_fullpipe->setObjectState(sO_LowerPipe, g_fullpipe->getObjectEnumState(sO_LowerPipe, sO_IsClosed));
	}
}

void sceneHandler04_startSounds(const char *snd1, const char *snd2, const char *snd3) {
	warning("STUB: sceneHandler04_startSounds()");
}

void sceneHandler04_goClock() {
	sceneHandler04_walkKozyawka();
	chainQueue(QU_SC4_GOCLOCK, 0);
	g_vars->scene04_soundPlaying = 1;
	g_vars->scene04_coinPut = 0;

	g_fullpipe->stopAllSoundStreams();

	sceneHandler04_startSounds("sc4_start.ogg", "sc4_loop.ogg", "sc4_stop2.ogg");

	g_vars->scene04_var14 = 0;
}

void sceneHandler04_sub8(ExCommand *ex) {
	warning("STUB: sceneHandler04_sub8()");
}

void sceneHandler04_sub12() {
	StaticANIObject *ball =  g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (ball && ball->_flags & 4)
		for (uint i = 0; i < ball->_movements.size(); i++)
			((Movement *)ball->_movements[i])->_counterMax = 0;

	g_vars->scene04_var13 = 0;
}

void sceneHandler04_handTake() {
	g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);

	if (g_vars->scene04_kozyawkiAni.size()) {
		if (g_vars->scene04_kozyawkiAni.size() == 1) {
			chainQueue(QU_HND_TAKE1, 0);
			g_vars->scene04_var19 = 0;
		} else {
			chainQueue((g_vars->scene04_kozyawkiAni.size() != 2) ? QU_HND_TAKEBOTTLE : QU_HND_TAKE2, 0);
			g_vars->scene04_var19 = 0;
		}
	} else {
		chainQueue(QU_HND_TAKE0, 0);
		g_vars->scene04_var19 = 0;
	}
}

void sceneHandler04_sub9(StaticANIObject *ani) {
	g_vars->scene04_bottleObjList.push_back(ani);
	g_vars->scene04_kozyawkiAni.push_back(ani);

	g_vars->scene04_bottleWeight += 2;
	g_vars->scene04_walkingKozyawka = 0;
	g_vars->scene04_var24 = 0;

	if (g_vars->scene04_kozyawkiAni.size() > 1 )
		g_vars->scene04_var19 = 0;

	if (g_vars->scene04_kozyawkiAni.size() <= 2 || g_vars->scene04_hand->_movement) {
		sceneHandler04_walkKozyawka();
	} else {
		sceneHandler04_handTake();
		sceneHandler04_stopSound();
	}
}

void sceneHandler04_sub17() {
	StaticANIObject *ball =  g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (g_vars->scene04_dudeOnLadder
		 && (!ball || !(ball->_flags & 4))
		 && g_vars->scene04_ladder->collisionDetection(g_fullpipe->_aniMan) > 3) {

		if (!g_fullpipe->_rnd->getRandomNumber(49)) {
			if (g_vars->scene04_var15)
				chainQueue(QU_BALL_WALKR, 0);
			else
				chainQueue(QU_BALL_WALKL, 0);

			g_vars->scene04_var15 = !g_vars->scene04_var15;

			sceneHandler04_checkBigBallClick();

			g_vars->scene04_var14 = 0;
		}
	}
}

void sceneHandler04_takeBottle() {
	g_vars->scene04_var02 = 1;
	g_vars->scene04_hand->_priority = 5;

	g_fullpipe->setObjectState(sO_LowerPipe, g_fullpipe->getObjectEnumState(sO_LowerPipe, sO_IsOpened));
}

void sceneHandler04_takeKozyawka() {
	if (g_vars->scene04_kozyawkiAni.size() > 0) {
		if (g_vars->scene04_kozyawkiAni.size() == 1) 
			g_vars->scene04_var19 = 1;

		StaticANIObject *koz = g_vars->scene04_kozyawkiAni.front();
		g_vars->scene04_kozyawkiAni.pop_front();

		if (koz) {
			koz->queueMessageQueue(0);
			koz->hide();

			g_vars->scene04_kozyawkiObjList.push_back(koz);

			for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it)
				if (*it == koz) {
					g_vars->scene04_bottleObjList.erase(it);
					break;
				}

			g_vars->scene04_bottleWeight -= 2;
		}
	}
}

void sceneHandler04_testPlank(ExCommand *ex) {
	MessageQueue *mq = g_fullpipe->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (!mq)
		return;

	if (g_vars->scene04_plank->_movement || !g_vars->scene04_plank->_statics || g_vars->scene04_plank->_statics->_staticsId != ST_PNK_WEIGHTLEFT) {
		mq->getExCommandByIndex(0)->_messageNum = MV_KZW_TOHOLERV;
	} else {
		mq->getExCommandByIndex(0)->_messageNum = MV_KZW_WALKPLANK;
	}
}

void sceneHandler04_updateBottle() {
	Common::Point point;

	int yoff;

	if (g_vars->scene04_hand->_movement)
		yoff = g_vars->scene04_hand->_movement->_oy;
	else
		yoff = g_vars->scene04_hand->_oy;

	int newy = g_vars->scene04_hand->getSomeXY(point)->y + yoff + 140;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_oy);

	g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, newy);
}

void sceneHandler04_winArcade() {
	if (g_fullpipe->getObjectState(sO_LowerPipe) == g_fullpipe->getObjectEnumState(sO_LowerPipe, sO_IsClosed)
		&& g_vars->scene04_soundPlaying) {
		g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);
		g_vars->scene04_hand->changeStatics2(ST_HND_EMPTY);

		chainQueue(QU_HND_TAKEBOTTLE, 1);

		if (g_vars->scene04_walkingKozyawka) {
			g_vars->scene04_kozyawkiObjList.push_back(g_vars->scene04_walkingKozyawka);

			g_vars->scene04_walkingKozyawka->changeStatics2(ST_KZW_EMPTY);
			g_vars->scene04_walkingKozyawka->hide();
			g_vars->scene04_walkingKozyawka = 0;
		}

		g_vars->scene04_var19 = 0;
		g_vars->scene04_soundPlaying = 0;

		getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->setEnabled();

		getGameLoaderInteractionController()->enableFlag24();

		g_fullpipe->stopSoundStream2();
	}
}

int sceneHandler04(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_UPDATEBOTTLE:
		sceneHandler04_updateBottle();
		break;
		
	case MSG_CLICKBOTTLE:
		sceneHandler04_clickBottle();
		break;

	case MSG_SHOOTKOZAW:
		sceneHandler04_shootKozyawka();
		break;

	case MSG_SHAKEBOTTLE:
		if (!g_vars->scene04_var02)
			++g_vars->scene04_var20;
		break;

	case MSG_STARTHAND:
		g_vars->scene04_var09 = 1;
		g_vars->scene04_coinPut = 0;

		if (g_vars->scene04_var10)
			sceneHandler04_sub1(0);

		sceneHandler04_handTake();
		sceneHandler04_stopSound();
		break;

	case MSG_TAKEKOZAW:
		sceneHandler04_takeKozyawka();
		break;

	case MSG_CLICKBUTTON:
		sceneHandler04_clickButton();
		break;

	case MSG_CLICKPLANK:
		sceneHandler04_clickPlank();
		break;

	case MSG_RAISEPLANK:
		sceneHandler04_raisePlank();
		break;

	case MSG_KOZAWRESTART:
		if (g_vars->scene04_walkingKozyawka) {
			g_vars->scene04_kozyawkiObjList.push_back(g_vars->scene04_walkingKozyawka);
			g_vars->scene04_walkingKozyawka->hide();
			g_vars->scene04_walkingKozyawka = 0;
		}

		if (g_vars->scene04_soundPlaying)
			sceneHandler04_walkKozyawka();

		break;

	case MSG_LOWERPLANK:
		sceneHandler04_lowerPlank();
		break;

	case MSG_TESTPLANK:
		sceneHandler04_testPlank(ex);
		break;

	case 33:
		{
			g_vars->scene04_dudePosX = g_fullpipe->_aniMan->_ox;
			g_vars->scene04_dudePosY = g_fullpipe->_aniMan->_oy;

			int res = 0;

			if (g_fullpipe->_aniMan2) {
				if (g_fullpipe->_aniMan->_ox < g_fullpipe->_sceneRect.left + 200) {
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.left - 300;
					g_fullpipe->_aniMan->_ox = g_vars->scene04_dudePosX;
				}
				if (g_fullpipe->_aniMan->_ox > g_fullpipe->_sceneRect.right - 200) {
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan->_ox - g_fullpipe->_sceneRect.right + 300;
				}

				res = 1;

				if (g_vars->scene04_soundPlaying) {
					if (g_fullpipe->_aniMan->_movement) {
						if (g_fullpipe->_aniMan->_movement->_id == MV_MAN_TOLADDER) {
							g_fullpipe->_aniMan2 = 0;

							if (g_fullpipe->_sceneRect.left > 380)
								g_fullpipe->_currentScene->_x = 380 - g_fullpipe->_sceneRect.left;
						}
					}
				}
			} else {
				if (g_fullpipe->_aniMan->_movement && g_fullpipe->_aniMan->_movement->_id == MV_MAN_GOD)
					g_fullpipe->_aniMan2 = g_fullpipe->_aniMan;
			}

			sceneHandler04_springWobble();

			if (g_vars->scene04_var07 && !g_vars->scene04_var09)
				sceneHandler04_leaveScene();

			if (g_vars->scene04_var12)
				sceneHandler04_liftBottle();

			if (g_vars->scene04_var08)
				sceneHandler04_clickLadder();

			if (g_vars->scene04_var10 && g_vars->scene04_hand->_movement)
				sceneHandler04_sub1(0);

			if (g_vars->scene04_coinPut && g_vars->scene04_var18 && !g_vars->scene04_var09 && !g_vars->scene04_soundPlaying)
				sceneHandler04_goClock();

			if (g_vars->scene04_dudeOnLadder) {
				if (!g_vars->scene04_soundPlaying) {
					g_fullpipe->startSceneTrack();

					g_fullpipe->_behaviorManager->updateBehaviors();
					return res;
				}

				g_vars->scene04_var14++;

				if (g_vars->scene04_var14 > 600)
					sceneHandler04_sub17();
			}

			if (g_vars->scene04_soundPlaying) {
				g_fullpipe->_behaviorManager->updateBehaviors();

				return res;
			}

			g_fullpipe->startSceneTrack();

			g_fullpipe->_behaviorManager->updateBehaviors();

			return res;
		}

	case 29:
		{
			int picid = g_fullpipe->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (g_vars->scene04_var10) {
				sceneHandler04_sub1(ex);

				break;
			}

			if (picid == PIC_SC4_LADDER) {
				if (!g_vars->scene04_var04) {
					g_vars->scene04_sceneClickX = ex->_sceneClickX;
					g_vars->scene04_sceneClickY = ex->_sceneClickY;

					sceneHandler04_clickLadder();

					ex->_messageKind = 0;

					break;
				}

				sceneHandler04_gotoLadder(0);

				break;
			}

			StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if ((ani && ani->_id == ANI_PLANK) || picid == PIC_SC4_PLANK) {
				sceneHandler04_clickPlank();

				ex->_messageKind = 0;
			} else if (g_vars->scene04_dudeOnLadder) {
				sceneHandler04_sub8(ex);
			} else if (!ani || !canInteractAny(g_fullpipe->_aniMan, ani, ex->_keyCode)) {
				PictureObject *pic = g_fullpipe->_currentScene->getPictureObjectById(picid, 0);

				if (!pic || !canInteractAny(g_fullpipe->_aniMan, pic,ex->_keyCode)) {
					if ((g_fullpipe->_sceneRect.right - ex->_sceneClickX < 47 && g_fullpipe->_sceneRect.right < g_fullpipe->_sceneWidth - 1)
						|| (ex->_sceneClickX - g_fullpipe->_sceneRect.left < 47 && g_fullpipe->_sceneRect.left > 0))
						g_fullpipe->processArcade(ex);
				}
			}
		}

		break;

	case MSG_SC4_HIDEBOOT:
		g_vars->scene04_boot->_flags &= 0xfffb;
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler04_winArcade();
		break;

	case MSG_SC4_HANDOVER:
		g_vars->scene04_var09 = 0;
		g_vars->scene04_var19 = 1;
		break;

	case MSG_SC4_DROPBOTTLE:
		sceneHandler04_dropBottle();
		break;
		
	case MSG_SC4_COINOUT:
		g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);
		g_vars->scene04_coinPut = 0;
		sceneHandler04_stopSound();

		if (g_vars->scene04_kozyawkiAni.size() && !g_vars->scene04_var02) {
			g_vars->scene04_var09 = 1;

			if (g_vars->scene04_var10)
				sceneHandler04_sub1(0);

			sceneHandler04_handTake();
		}

		break;

	case MSG_SC4_KOZAWFALL:
		{
			ExCommand *exnew;

			if (g_vars->scene04_var11) {
				sceneHandler04_sub9(g_vars->scene04_var24);

				g_vars->scene04_var11 = 0;

				exnew = new ExCommand(0, 35, SND_4_010, 0, 0, 0, 1, 0, 0, 0);
			} else {
				exnew = new ExCommand(0, 35, SND_4_012, 0, 0, 0, 1, 0, 0, 0);
			}

			exnew->_field_14 = 5;
			exnew->_excFlags |= 2;
			exnew->postMessage();
			break;
		}

	case MSG_SC4_MANFROMBOTTLE:
		sceneHandler04_manFromBottle();
		break;

	case MSG_SC4_CLICKLADDER:
		sceneHandler04_clickLadder();
		break;

	case MSG_SC4_MANTOBOTTLE:
		sceneHandler04_manToBottle();
		break;

	case MSG_SHOWCOIN:
		sceneHandler04_showCoin();
		break;

	case MSG_TAKEBOTTLE:
		sceneHandler04_takeBottle();
		break;

	case MSG_GOTOLADDER:
		sceneHandler04_gotoLadder(0);
		break;

	case MSG_SC4_COINPUT:
		g_vars->scene04_coinPut = 1;
		break;
	}

	return 0;
}

} // End of namespace Fullpipe
