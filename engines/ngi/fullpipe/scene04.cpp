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
#include "ngi/utils.h"
#include "ngi/gfx.h"
#include "ngi/messages.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"
#include "ngi/scene.h"
#include "ngi/interaction.h"
#include "ngi/gameloader.h"
#include "ngi/behavior.h"

#include "audio/mixer.h"

namespace NGI {

static const int scene04_speakerPhases[] = {
	0, 1,  2,  3, -1, -1,
	0, 2,  3, -1, -1, -1,
	0, 2, -1, -1, -1, -1
};

void sceneHandler04_putKozyawkaBack(StaticANIObject *ani);

void scene04_speakerCallback(int *phase) {
	if (g_vars->scene04_soundPlaying) {
		if (g_vars->scene04_speakerPhase >= 0) {
			*phase = scene04_speakerPhases[g_vars->scene04_speakerPhase + 6 * g_vars->scene04_speakerVariant];

			g_vars->scene04_speakerPhase++;

			if (scene04_speakerPhases[g_vars->scene04_speakerPhase + 6 * g_vars->scene04_speakerVariant] < 0) {
				g_vars->scene04_speakerPhase = 0;
				g_vars->scene04_speakerVariant = g_nmi->_rnd.getRandomNumber(2);
			}
		} else {
			++g_vars->scene04_speakerPhase;
		}
	}
}

void scene04_springCallback(int *phase) {
	// do nothing
}

void scene04_initScene(Scene *sc) {
	debugC(1, kDebugSceneLogic, "scene04_initScene()");

#if 0
	Inventory2 *inv = getGameLoaderInventory();
	inv->addItem(ANI_INV_COIN, 1);
	inv->rebuildItemRects();
#endif

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

				if (kozmov->_framePosOffsets.size()) {
					g_vars->scene04_jumpingKozyawki[i] = kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
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

				if (kozmov->_framePosOffsets.size()) {
					g_vars->scene04_jumpRotateKozyawki[i] = kozmov->_framePosOffsets[kozmov->_currDynamicPhaseIndex];
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

	if (g_nmi->getObjectState(sO_Jar_4) == g_nmi->getObjectEnumState(sO_Jar_4, sO_UpsideDown)) {
		debugC(2, kDebugSceneLogic, "scene04: Jar is upside down");

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_kozyawkiObjList.clear();

		sc->getPictureObjectById(PIC_SC4_BOTTLE, 0)->_flags &= 0xfffb;
		sc->getPictureObjectById(PIC_SC4_MASK, 0)->_flags &= 0xfffb;
		sc->getStaticANIObject1ById(ANI_SPRING, 0)->_flags &= 0xfffb;

		g_vars->scene04_clockCanGo = false;
		g_vars->scene04_objectIsTaken = false;
	} else {
		debugC(2, kDebugSceneLogic, "scene04: Jar is NOT upside down");

		StaticANIObject *spring = sc->getStaticANIObject1ById(ANI_SPRING, -1);

		if (spring)
			spring->_callback2 = scene04_springCallback;

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_BOTTLE, 0));
		g_vars->scene04_bottleObjList.push_back(sc->getPictureObjectById(PIC_SC4_MASK, 0));

		g_vars->scene04_kozyawkiObjList.clear();

		if (koz) {
			debugC(2, kDebugSceneLogic, "scene04: We're in koz");

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

		g_vars->scene04_clockCanGo = true;
		g_vars->scene04_objectIsTaken = true;

		debugC(2, kDebugSceneLogic, "scene04: kozObjList size: %d", g_vars->scene04_kozyawkiObjList.size());
	}

	g_vars->scene04_bottleIsTaken = false;
	g_vars->scene04_soundPlaying = false;
	g_vars->scene04_kozyawkaOnLadder = false;
	g_vars->scene04_walkingKozyawka = 0;
	g_vars->scene04_bottleWeight = 2;
	g_vars->scene04_dynamicPhaseIndex = 0;

	g_vars->scene04_kozyawkiAni.clear();

	g_nmi->setObjectState(sO_LowerPipe, g_nmi->getObjectEnumState(sO_LowerPipe, sO_IsClosed));

	g_vars->scene04_var07 = false;
	g_vars->scene04_ladderClickable = false;
	g_vars->scene04_coinPut = false;
	g_vars->scene04_handIsDown = false;
	g_vars->scene04_dudeInBottle = false;
	g_vars->scene04_kozHeadRaised = false;
	g_vars->scene04_bottleIsDropped = false;
	g_vars->scene04_bigBallIn = true;
	g_vars->scene04_bigBallCounter = 0;
	g_vars->scene04_bigBallFromLeft = true;

	if (g_nmi->getObjectState(sO_BigMumsy) != g_nmi->getObjectEnumState(sO_BigMumsy, sO_IsGone))
		g_vars->scene04_mamasha->hide();

	g_vars->scene04_speaker = sc->getStaticANIObject1ById(ANI_SPEAKER_4, -1);
	g_vars->scene04_speaker->_callback2 = scene04_speakerCallback;
	g_vars->scene04_speaker->startAnim(MV_SPK4_PLAY, 0, -1);

	g_vars->scene04_speakerVariant = 0;
	g_vars->scene04_speakerPhase = 0;

	g_vars->scene04_musicStage = 0;

	g_nmi->initArcadeKeys("SC_4");
}

bool sceneHandler04_friesAreWalking() {
	if (g_vars->scene04_dudeOnLadder && g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
		int col = g_vars->scene04_ladder->collisionDetection(g_nmi->_aniMan);
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
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor == PIC_SC4_LRTRUBA) {
		if (!g_vars->scene04_objectIsTaken) {
			g_nmi->_cursorId = PIC_CSR_DEFAULT;

			return g_nmi->_cursorId;
		}
	} else if (g_nmi->_objectIdAtCursor == ANI_PLANK || g_nmi->_objectIdAtCursor == PIC_SC4_PLANK) {
		if (g_nmi->_objectIdAtCursor == ANI_PLANK && g_nmi->_cursorId != PIC_CSR_ITN)
			return g_nmi->_cursorId;

		if (g_nmi->_objectIdAtCursor == ANI_PLANK || (g_nmi->_objectIdAtCursor == PIC_SC4_PLANK && g_nmi->_cursorId == PIC_CSR_DEFAULT)) {
			if (sceneHandler04_friesAreWalking()) {
				g_nmi->_cursorId = PIC_CSR_ARCADE1;
				return g_nmi->_cursorId;
			}
			if (g_vars->scene04_soundPlaying) {
				g_nmi->_cursorId = PIC_CSR_DEFAULT;
				return g_nmi->_cursorId;
			}
		}
	}

	if (g_nmi->_cursorId == PIC_CSR_ITN && g_nmi->_objectIdAtCursor == PIC_SC4_DOWNTRUBA)
		g_nmi->_cursorId = PIC_CSR_GOD;

	return g_nmi->_cursorId;
}

void sceneHandler04_checkBigBallClick() {
	StaticANIObject *ball = g_nmi->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (ball)
		for (uint i = 0; i < ball->_movements.size(); i++)
			ball->_movements[i]->_counterMax = 73;

	g_vars->scene04_bigBallIn = true;
}

void sceneHandler04_clickBottle() {
	if (!g_vars->scene04_bottleIsTaken)
		g_vars->scene04_springOffset += 5;
}

void sceneHandler04_clickButton() {
	StaticANIObject *but = g_nmi->_currentScene->getStaticANIObject1ById(ANI_BUTTON, -1);

	if (but) {
		if (!g_vars->scene04_clock->_movement ||
			(g_vars->scene04_clock->_movement->_id == MV_CLK_GO && g_vars->scene04_clock->_movement->_currDynamicPhaseIndex > 3 &&
			 g_vars->scene04_clock->_movement->_currDynamicPhaseIndex < 105)) {
			if (!g_vars->scene04_hand->_movement && !g_vars->scene04_bottleIsTaken) {
				but->startAnim(MV_BTN_CLICK, 0, -1);
				g_vars->scene04_hand->startAnim(MV_HND_POINT, 0, -1);
			}
		}
	}
}

void sceneHandler04_downLadder(int x, int y) {
	g_vars->scene04_ladder->startMove(g_nmi->_aniMan, x + g_vars->scene04_ladder->_ladder_field_20, y + g_vars->scene04_ladder->_ladder_field_24, 0, 0);
}

void sceneHandler04_walkClimbLadder(ExCommand *ex) {
	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	ExCommand *ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex1->_param = g_nmi->_aniMan->_odelay;
	ex1->_excFlags |= 2;

	mq->addExCommandToEnd(ex1);

	ExCommand *ex2 = new ExCommand(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex2->_param = g_nmi->_aniMan->_odelay;
	ex2->_excFlags |= 2;

	mq->addExCommandToEnd(ex2);

	ExCommand *ex3;

	if (ex) {
		ex3 = ex->createClone();
	} else {
		ex3 = new ExCommand(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex3->_excFlags |= 3;
	}

	mq->addExCommandToEnd(ex3);

	mq->setFlags(mq->getFlags() | 1);

	mq->chain(0);

	g_vars->scene04_dudeOnLadder = 1;

	g_vars->scene04_ladder = new MctlLadder;
	g_vars->scene04_ladder->_ladderX = 1089;
	g_vars->scene04_ladder->_ladderY = 406;
	g_vars->scene04_ladder->_ladder_field_14 = 12;
	g_vars->scene04_ladder->_width = 0;
	g_vars->scene04_ladder->_height = -40;
	g_vars->scene04_ladder->_ladder_field_20 = 0;
	g_vars->scene04_ladder->_ladder_field_24 = -60;

	g_vars->scene04_ladder->attachObject(g_nmi->_aniMan);

	if (g_vars->scene04_soundPlaying) {
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpStart = MV_MAN_STARTLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpGo = MV_MAN_GOLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpStop = MV_MAN_STOPLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->staticIds[2] = ST_MAN_GOLADDER2;
	} else {
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpStart = MV_MAN_STARTLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpGo = MV_MAN_GOLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->movVars->varUpStop = MV_MAN_STOPLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->staticIds[2] = ST_MAN_GOLADDER;
	}

	g_nmi->_aniMan->_priority = 12;

	getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->deactivate();
	getGameLoaderInteractionController()->disableFlag24();
}

void sceneHandler04_clickLadder() {
	g_vars->scene04_dudePosX = g_nmi->_aniMan->_ox;
	g_vars->scene04_dudePosY = g_nmi->_aniMan->_oy;

	if (g_vars->scene04_dudeOnLadder) {
		if (!g_nmi->_aniMan->isIdle() || (g_nmi->_aniMan->_flags & 0x100)) {
			g_vars->scene04_ladderClickable = true;
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

			g_vars->scene04_ladderClickable = false;
		}
	} else {
		if (g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
			if (abs(1095 - g_vars->scene04_dudePosX) > 1 || abs(434 - g_vars->scene04_dudePosY) > 1) {
				MessageQueue *mq = getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->startMove(g_nmi->_aniMan, 1095, 434, 1, ST_MAN_UP);
				if (mq) {
					ExCommand *ex = new ExCommand(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);

					ex->_excFlags = 3;
					mq->addExCommandToEnd(ex);

					postExCommand(g_nmi->_aniMan->_id, 2, 1095, 434, 0, -1);
				}
			} else {
				sceneHandler04_walkClimbLadder(0);
			}
		}
	}
}

void sceneHandler04_jumpOnLadder() {
	if (g_nmi->_aniMan->_movement && g_nmi->_aniMan->_movement->_id != MV_MAN_LOOKLADDER)
		return;

	if (g_nmi->_aniMan->_statics->_staticsId != ST_MAN_STANDLADDER && g_nmi->_aniMan->_statics->_staticsId != ST_MAN_LADDERDOWN)
		return;

	g_nmi->_aniMan->changeStatics2(ST_MAN_LADDERDOWN);

	g_nmi->_aniMan->_flags |= 0x100;

	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	aniHandler.attachObject(ANI_MAN);

	mkQueue.ani = g_nmi->_aniMan;
	mkQueue.staticsId2 = ST_MAN_ONPLANK;
	mkQueue.x1 = 938;
	mkQueue.y1 = 442;
	mkQueue.field_1C = 10;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_MAN_JUMPONPLANK;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->_flags |= 1;

		if (!mq->chain(g_nmi->_aniMan))
			delete mq;

		g_nmi->_aniMan->_priority = 10;
	}

	g_vars->scene04_ladderOffset = g_vars->scene04_ladder->collisionDetection(g_nmi->_aniMan);
}

void sceneHandler04_clickPlank() {
	if (sceneHandler04_friesAreWalking())
		sceneHandler04_jumpOnLadder();
	else if (g_vars->scene04_dudeOnLadder)
		g_nmi->playSound(SND_4_033, 0);
	else if (!g_vars->scene04_soundPlaying)
		chainQueue(QU_PNK_CLICK, 0);
}

void sceneHandler04_dropBottle() {
	g_vars->scene04_bottleIsDropped = true;
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
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	aniHandler.attachObject(ANI_MAN);

	mkQueue.ani = g_nmi->_aniMan;
	mkQueue.staticsId2 = ST_MAN_UP;
	mkQueue.x1 = 1095;
	mkQueue.y1 = 434;
	mkQueue.field_1C = 12;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_MAN_PLANKTOLADDER;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->deleteExCommandByIndex(mq->getCount() - 1, 1);

		ExCommand *ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex1->_excFlags = 2;
		ex1->_field_24 = 1;
		ex1->_param = -1;
		mq->addExCommandToEnd(ex1);

		ExCommand *ex2 = new ExCommand(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex2->_excFlags = 2;
		ex2->_field_24 = 1;
		ex2->_param = -1;
		mq->addExCommandToEnd(ex2);

		ExCommand *ex3 = new ExCommand(g_nmi->_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
		ex3->_z = 256;
		ex3->_messageNum = 0;
		ex3->_excFlags |= 3;
		mq->addExCommandToEnd(ex3);

		if (ex) {
			ExCommand *ex4 = ex->createClone();

			mq->addExCommandToEnd(ex4);
		}

		mq->setFlags(mq->getFlags() | 1);

		if (mq->chain(g_nmi->_aniMan)) {
			g_nmi->_aniMan->_priority = 12;
			g_nmi->_aniMan->_flags |= 0x100;
		} else {
			delete mq;
		}
	}

	g_vars->scene04_kozyawkaOnLadder = false;
}

void sceneHandler04_lowerPlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTRIGHT, 0, -1);
}

void sceneHandler04_manFromBottle() {
	for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it)
		if (*it == g_nmi->_aniMan) {
			g_vars->scene04_bottleObjList.erase(it);
			g_vars->scene04_bottleWeight -= 9;
			break;
		}

	if (g_vars->scene04_ladder)
		delete g_vars->scene04_ladder;

	g_vars->scene04_ladder = 0;

	getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->activate();
	getGameLoaderInteractionController()->enableFlag24();
}

void sceneHandler04_manToBottle() {
	g_vars->scene04_bottleObjList.push_back(g_nmi->_aniMan);
	g_vars->scene04_springOffset = 5;
	g_vars->scene04_bottleWeight += 9;
	g_nmi->_aniMan2 = g_nmi->_aniMan;
	g_vars->scene04_dudeInBottle = 1;
}

void sceneHandler04_raisePlank() {
	g_vars->scene04_plank->startAnim(MV_PNK_WEIGHTLEFT, 0, -1);
}

MessageQueue *sceneHandler04_kozFly3(StaticANIObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly3 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x1 = (int)(723.0 - phase * 185.0);
	mkQueue.y1 = 486;
	mkQueue.field_1C = 10;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMP;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		ExCommand *ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		for (int i = 0; i < 5; i++) {
			ex = new ExCommand(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_odelay;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);
	}

	return mq;
}

MessageQueue *sceneHandler04_kozFly5(StaticANIObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly5 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_JUMPOUT;
	mkQueue.x1 = 525;
	mkQueue.y1 = (int)(344.0 - (double)(320 - g_vars->scene04_bottle->_oy) * phase);
	mkQueue.field_1C = 10;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPHIT;

	MessageQueue *mq1 = aniHandler.makeRunQueue(&mkQueue);

	mkQueue.reset();
	mkQueue.ani = ani;
	mkQueue.staticsId1 = ST_KZW_JUMPOUT;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x2 = 525;
	mkQueue.y2 = (int)(344.0 - (double)(320 - g_vars->scene04_bottle->_oy) * phase);
	mkQueue.y1 = 486;
	mkQueue.field_1C = 10;
	mkQueue.field_10 = 1;
	mkQueue.flags = 117;
	mkQueue.movementId = MV_KZW_JUMPOUT;

	MessageQueue *mq2 = aniHandler.makeRunQueue(&mkQueue);

	if (mq1 && mq2) {
		mq1->addExCommandToEnd(mq2->getExCommandByIndex(0)->createClone());

		delete mq2;
		mq2 = 0;

		ExCommand *ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq1->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq1->addExCommandToEnd(ex);

		for (int i = 0; i < 5; i++) {
			ex = new ExCommand(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_odelay;
			mq1->addExCommandToEnd(ex);
		}

		ex = new ExCommand(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq1->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq1->addExCommandToEnd(ex);
	}

	if (mq2)
		delete mq2;

	return mq1;
}

MessageQueue *sceneHandler04_kozFly6(StaticANIObject *ani) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly6 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x1 = 397 - 4 * g_nmi->_rnd.getRandomNumber(1);
	mkQueue.field_1C = ani->_priority;
	mkQueue.y1 = g_vars->scene04_bottle->_oy - 4 * g_nmi->_rnd.getRandomNumber(1) + 109;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPROTATE;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->deleteExCommandByIndex(mq->getCount() - 1, 1);

		ExCommand *ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_RAISEHEAD, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		g_vars->scene04_kozHeadRaised = true;
	}

	return mq;
}

void sceneHandler04_kozMove(Movement *mov, int from, int to, Common::Point *points, double phase) {
	for (int i = from; i < to; i++) {
		mov->setDynamicPhaseIndex(i);

		Common::Point *p;
		if (mov->_framePosOffsets.size()) {
			p = &mov->_framePosOffsets[mov->_currDynamicPhaseIndex];
		} else {
			p = &mov->_somePoint;
			p->x = 0;
			p->y = 0;
		}

		p->y = (int)((double)points[i].y * phase);
	}
}

MessageQueue *sceneHandler04_kozFly7(StaticANIObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly7");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = 560;
	mkQueue.x1 = (int)(250.0 - phase * 100.0);
	mkQueue.y1 = 455;
	mkQueue.field_1C = 10;
	mkQueue.field_10 = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPROTATE;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		sceneHandler04_kozMove(ani->getMovementById(MV_KZW_JUMPROTATE), 1, 9, g_vars->scene04_jumpRotateKozyawki, phase * 0.5 + 1.5);

		ani->_priority = 10;

		ExCommand *ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		for (int i = 0; i < 2; i++) {
			ex = new ExCommand(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_odelay;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_odelay;
		mq->addExCommandToEnd(ex);
	}

	return mq;
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
					g_vars->scene04_lastKozyawka = g_vars->scene04_walkingKozyawka;

					if (!mq->chain(g_vars->scene04_walkingKozyawka) )
						delete mq;
				}
			}
		}
	}

	if (g_vars->scene04_ladderOffset > 3)
		g_nmi->_aniMan->changeStatics1(ST_MAN_LOOKPLANK);

	g_vars->scene04_kozyawkaOnLadder = true;
}

void sceneHandler04_showCoin() {
	StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_SC4_COIN, -1);

	if (ani) {
		ani->show1(MV_BDG_OPEN, MV_MAN_GOU, MV_SC4_COIN_default, 0);

		ani->_priority = 40;
	}
}

void sceneHandler04_stopSound() {
	g_vars->scene04_soundPlaying = false;

	g_nmi->stopSoundStream2();
}

void sceneHandler04_animOutOfBottle(ExCommand *ex) {
	g_nmi->_aniMan->changeStatics2(ST_MAN_SIT);

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC4_MANFROMBOTTLE), 0, 0);

	if (ex) {
		ExCommand *newex = ex->createClone();

		mq->addExCommandToEnd(newex);
	  }

	mq->_flags |= 1;
	mq->chain(0);

	g_vars->scene04_dudeInBottle = false;
	g_nmi->_behaviorManager->setFlagByStaticAniObject(g_nmi->_aniMan, 1);
}

void sceneHandler04_walkKozyawka() {
	debugC(1, kDebugSceneLogic, "scene04: walkKozyawka");

	if (g_vars->scene04_kozyawkiObjList.size()) {
		debugC(1, kDebugSceneLogic, "scene04: walkKozyawka: getting one");

		g_vars->scene04_walkingKozyawka = g_vars->scene04_kozyawkiObjList.front();
		g_vars->scene04_kozyawkiObjList.pop_front();

		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_KOZAW_WALK), 0, 1);
		mq->setParamInt(-1, g_vars->scene04_walkingKozyawka->_odelay);
		mq->chain(0);
	}
}

void sceneHandler04_bottleUpdateObjects(int off) {
	for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it) {
		if ((*it)->_objtype == kObjTypeStaticANIObject) {
			StaticANIObject *st = static_cast<StaticANIObject *>(*it);

			st->setOXY(st->_ox, off + st->_oy);
		} else {
			GameObject *obj = *it;

			obj->setOXY(obj->_ox, off + obj->_oy);
		}
	}
}

void sceneHandler04_springWobble() {
	int oldDynIndex = g_vars->scene04_dynamicPhaseIndex;
	int newdelta = g_vars->scene04_springOffset + g_vars->scene04_dynamicPhaseIndex;

	g_vars->scene04_dynamicPhaseIndex += g_vars->scene04_springOffset;

	if (newdelta < 0) {
		newdelta = 0;
		g_vars->scene04_dynamicPhaseIndex = 0;
		g_vars->scene04_springOffset = 0;
	}

	if (newdelta > 14) {
		newdelta = 14;
		g_vars->scene04_dynamicPhaseIndex = 14;
		g_vars->scene04_springOffset = 0;
	}

	if (g_vars->scene04_bottleWeight > newdelta)
		g_vars->scene04_springOffset++;

	if (g_vars->scene04_bottleWeight < newdelta)
		g_vars->scene04_springOffset--;

	if ((oldDynIndex <= g_vars->scene04_bottleWeight && newdelta > g_vars->scene04_bottleWeight)
		|| (oldDynIndex > g_vars->scene04_bottleWeight && newdelta <= g_vars->scene04_bottleWeight)) {
		g_vars->scene04_springDelay++;

		if (g_vars->scene04_springOffset && g_vars->scene04_springDelay > 1) {
			g_vars->scene04_springDelay = 0;
			g_vars->scene04_springOffset = g_vars->scene04_springOffset - g_vars->scene04_springOffset / abs(g_vars->scene04_springOffset);
		}
	}

	int oldpos = g_vars->scene04_spring->getCurrDimensions().y - oldDynIndex;

	if (g_vars->scene04_dynamicPhaseIndex) {
		if (!g_vars->scene04_spring->_movement)
			g_vars->scene04_spring->startAnim(MV_SPR_LOWER, 0, -1);

		g_vars->scene04_spring->_movement->setDynamicPhaseIndex(g_vars->scene04_dynamicPhaseIndex);
	} else {
		g_vars->scene04_spring->changeStatics2(ST_SPR_UP);
	}

	if (g_vars->scene04_dynamicPhaseIndex != oldDynIndex) {
		sceneHandler04_bottleUpdateObjects(oldpos - (g_vars->scene04_spring->getCurrDimensions().y - g_vars->scene04_dynamicPhaseIndex));
	}
}

void sceneHandler04_leaveScene() {
	g_nmi->_aniMan2 = 0;

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC4_MANTOBOTTLE), 0, 0);
	ExCommand *ex = 0;

	for (uint i = 0; i < mq->getCount(); i++) {
		if (mq->getExCommandByIndex(i)->_messageKind == 27) {
			ex = mq->getExCommandByIndex(i);
			break;
		}
	}

	if (!ex) {
		error("sceneHandler04_leaveScene(): Cannot find exit");
	}

	ex->_y = g_vars->scene04_bottle->_oy - 304;

	mq->chain(0);

	g_vars->scene04_var07 = false;
	g_vars->scene04_dudeOnLadder = 0;

	g_nmi->_behaviorManager->setFlagByStaticAniObject(g_nmi->_aniMan, 0);

	g_nmi->updateMapPiece(PIC_MAP_P03, 1);
}

void sceneHandler04_liftBottle() {
	int newy = g_vars->scene04_bottleY + g_vars->scene04_spring->_oy;

	g_vars->scene04_bottleY += 5;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_oy);

	g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, newy);

	if (g_vars->scene04_bottle->_oy >= 226) {
		sceneHandler04_bottleUpdateObjects(226 - g_vars->scene04_bottle->_oy);

		g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, 437);
		g_vars->scene04_bottleIsDropped = false;
		g_vars->scene04_handIsDown = false;
		g_vars->scene04_objectIsTaken = true;
		g_vars->scene04_bottleWeight = 2;
		g_vars->scene04_springOffset = 10;
		g_vars->scene04_bottleIsTaken = false;

		g_nmi->setObjectState(sO_LowerPipe, g_nmi->getObjectEnumState(sO_LowerPipe, sO_IsClosed));
	}
}

void sceneHandler04_startSounds(const char *snd1, const char *snd2, const char *snd3) {
	g_nmi->playOggSound(snd1, g_nmi->_soundStream2);

	g_nmi->_stream2playing = true;

	g_vars->scene04_musicStage = 1;
}

void updateSound() {
	switch (g_vars->scene04_musicStage) {
	case 0:
	default:
		break;
	case 1:
		if (!g_nmi->_mixer->isSoundHandleActive(g_nmi->_soundStream2)) {
			g_nmi->playOggSound("sc4_loop.ogg", g_nmi->_soundStream3);
			g_vars->scene04_musicStage = 2;
		}
		break;
	case 2:
		if (!g_nmi->_mixer->isSoundHandleActive(g_nmi->_soundStream3)) {
			if (g_nmi->_stream2playing) { // Looop it
				g_nmi->playOggSound("sc4_loop.ogg", g_nmi->_soundStream3);
			} else {
				g_nmi->playOggSound("sc4_stop2.ogg", g_nmi->_soundStream4);
				g_vars->scene04_musicStage = 3;
			}
		}
		break;
	case 3:
		if (!g_nmi->_mixer->isSoundHandleActive(g_nmi->_soundStream4)) {
			g_vars->scene04_musicStage = 0;
		}
		break;
	}
}

void sceneHandler04_goClock() {
	sceneHandler04_walkKozyawka();
	chainQueue(QU_SC4_GOCLOCK, 0);
	g_vars->scene04_soundPlaying = true;
	g_vars->scene04_coinPut = false;

	g_nmi->stopAllSoundStreams();

	sceneHandler04_startSounds("sc4_start.ogg", "sc4_loop.ogg", "sc4_stop2.ogg");

	g_vars->scene04_bigBallCounter = 0;
}

void sceneHandler04_bigBallOut() {
	StaticANIObject *ball =  g_nmi->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (ball && ball->_flags & 4)
		for (uint i = 0; i < ball->_movements.size(); i++)
			ball->_movements[i]->_counterMax = 0;

	g_vars->scene04_bigBallIn = false;
}

void sceneHandler04_leaveLadder(ExCommand *ex) {
	if (!g_nmi->_aniMan->isIdle())
		return;

	if (!(g_nmi->_aniMan->_flags & 0x100)) {
		if (getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->_objtype == kObjTypeMctlCompound) {
			MctlCompound *mc = static_cast<MctlCompound *>(getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId));

			if (mc->_motionControllers[0]->_movGraphReactObj->pointInRegion(g_nmi->_sceneRect.left + ex->_x, g_nmi->_sceneRect.top + ex->_y)) {
				if (g_vars->scene04_ladder->collisionDetection(g_nmi->_aniMan)) {
					MessageQueue *mq = g_vars->scene04_ladder->controllerWalkTo(g_nmi->_aniMan, 0);

					if (mq) {
						mq->addExCommandToEnd(ex->createClone());

						if (mq->chain(g_nmi->_aniMan) )
							ex->_messageKind = 0;
						else
							delete mq;

						if (g_vars->scene04_bigBallIn) {
							sceneHandler04_bigBallOut();
							return;
						}
					}
				} else {
					MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
					ExCommand *ex1;

					if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_LADDERDOWN) {
						ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_LOOKLADDERRV, 0, 0, 0, 1, 0, 0, 0);
						ex1->_param = g_nmi->_aniMan->_odelay;
						ex1->_excFlags |= 2;
						mq->addExCommandToEnd(ex1);
					}

					ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_STARTLADDERD, 0, 0, 0, 1, 0, 0, 0);
					ex1->_param = g_nmi->_aniMan->_odelay;
					ex1->_excFlags |= 2;
					mq->addExCommandToEnd(ex1);

					ex1 = new ExCommand(ANI_MAN, 1, MV_MAN_FROMLADDER, 0, 0, 0, 1, 0, 0, 0);
					ex1->_param = g_nmi->_aniMan->_odelay;
					ex1->_excFlags |= 2;
					mq->addExCommandToEnd(ex1);

					ex1 = ex->createClone();
					mq->addExCommandToEnd(ex1);

					mq->setFlags(mq->getFlags() | 1);

					if (mq->chain(g_nmi->_aniMan)) {
						if (g_vars->scene04_ladder)
							delete g_vars->scene04_ladder;

						g_vars->scene04_ladder = 0;
						g_vars->scene04_dudeOnLadder = 0;

						ex->_messageKind = 0;

						mc->activate();
						getGameLoaderInteractionController()->enableFlag24();
					} else {
						delete mq;
					}

					if (g_vars->scene04_bigBallIn) {
						sceneHandler04_bigBallOut();
						return;
					}
				}
			}
		}
	}
}

void sceneHandler04_handTake() {
	g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);

	if (g_vars->scene04_kozyawkiAni.size()) {
		if (g_vars->scene04_kozyawkiAni.size() == 1) {
			chainQueue(QU_HND_TAKE1, 0);
			g_vars->scene04_objectIsTaken = false;
		} else {
			chainQueue((g_vars->scene04_kozyawkiAni.size() != 2) ? QU_HND_TAKEBOTTLE : QU_HND_TAKE2, 0);
			g_vars->scene04_objectIsTaken = false;
		}
	} else {
		chainQueue(QU_HND_TAKE0, 0);
		g_vars->scene04_objectIsTaken = false;
	}
}

void sceneHandler04_putKozyawkaBack(StaticANIObject *ani) {
	debugC(2, kDebugSceneLogic, "scene04: putKozyawkaBack");

	g_vars->scene04_bottleObjList.push_back(ani);
	g_vars->scene04_kozyawkiAni.push_back(ani);

	g_vars->scene04_bottleWeight += 2;
	g_vars->scene04_walkingKozyawka = 0;
	g_vars->scene04_lastKozyawka = 0;

	if (g_vars->scene04_kozyawkiAni.size() > 1 )
		g_vars->scene04_objectIsTaken = false;

	if (g_vars->scene04_kozyawkiAni.size() <= 2 || g_vars->scene04_hand->_movement) {
		sceneHandler04_walkKozyawka();
	} else {
		sceneHandler04_handTake();
		sceneHandler04_stopSound();
	}
}

void sceneHandler04_bigBallWalkIn() {
	StaticANIObject *ball =  g_nmi->_currentScene->getStaticANIObject1ById(ANI_BIGBALL, -1);

	if (g_vars->scene04_dudeOnLadder
		 && (!ball || !(ball->_flags & 4))
		 && g_vars->scene04_ladder->collisionDetection(g_nmi->_aniMan) > 3) {

		if (!g_nmi->_rnd.getRandomNumber(49)) {
			if (g_vars->scene04_bigBallFromLeft)
				chainQueue(QU_BALL_WALKR, 0);
			else
				chainQueue(QU_BALL_WALKL, 0);

			g_vars->scene04_bigBallFromLeft = !g_vars->scene04_bigBallFromLeft;

			sceneHandler04_checkBigBallClick();

			g_vars->scene04_bigBallCounter = 0;
		}
	}
}

void sceneHandler04_takeBottle() {
	g_vars->scene04_bottleIsTaken = true;
	g_vars->scene04_hand->_priority = 5;

	g_nmi->setObjectState(sO_LowerPipe, g_nmi->getObjectEnumState(sO_LowerPipe, sO_IsOpened));
}

void sceneHandler04_takeKozyawka() {
	if (g_vars->scene04_kozyawkiAni.size() > 0) {
		if (g_vars->scene04_kozyawkiAni.size() == 1)
			g_vars->scene04_objectIsTaken = true;

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
	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (!mq)
		return;

	if (g_vars->scene04_plank->_movement || !g_vars->scene04_plank->_statics || g_vars->scene04_plank->_statics->_staticsId != ST_PNK_WEIGHTLEFT) {
		mq->getExCommandByIndex(0)->_messageNum = MV_KZW_TOHOLERV;
	} else {
		mq->getExCommandByIndex(0)->_messageNum = MV_KZW_WALKPLANK;
	}
}

void sceneHandler04_updateBottle() {
	int yoff;

	if (g_vars->scene04_hand->_movement)
		yoff = g_vars->scene04_hand->_movement->_oy;
	else
		yoff = g_vars->scene04_hand->_oy;

	int newy = g_vars->scene04_hand->getSomeXY().y + yoff + 140;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_oy);

	g_vars->scene04_spring->setOXY(g_vars->scene04_spring->_ox, newy);
}

void sceneHandler04_winArcade() {
	if (g_nmi->getObjectState(sO_LowerPipe) == g_nmi->getObjectEnumState(sO_LowerPipe, sO_IsClosed)
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

		g_vars->scene04_objectIsTaken = false;
		g_vars->scene04_soundPlaying = false;

		getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->activate();

		getGameLoaderInteractionController()->enableFlag24();

		g_nmi->stopSoundStream2();
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
		if (!g_vars->scene04_bottleIsTaken)
			++g_vars->scene04_springOffset;
		break;

	case MSG_STARTHAND:
		g_vars->scene04_handIsDown = true;
		g_vars->scene04_coinPut = false;

		if (g_vars->scene04_dudeInBottle)
			sceneHandler04_animOutOfBottle(0);

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
		debugC(1, kDebugSceneLogic, "scene04: kozawRestart");

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
			g_vars->scene04_dudePosX = g_nmi->_aniMan->_ox;
			g_vars->scene04_dudePosY = g_nmi->_aniMan->_oy;

			int res = 0;

			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan->_ox < g_nmi->_sceneRect.left + 200) {
					g_nmi->_currentScene->_x = g_nmi->_aniMan->_ox - g_nmi->_sceneRect.left - 300;
					g_nmi->_aniMan->_ox = g_vars->scene04_dudePosX;
				}
				if (g_nmi->_aniMan->_ox > g_nmi->_sceneRect.right - 200) {
					g_nmi->_currentScene->_x = g_nmi->_aniMan->_ox - g_nmi->_sceneRect.right + 300;
				}

				res = 1;
			}

			g_nmi->sceneAutoScrolling();

			if (g_nmi->_aniMan2) {
				if (g_vars->scene04_soundPlaying) {
					if (g_nmi->_aniMan->_movement) {
						if (g_nmi->_aniMan->_movement->_id == MV_MAN_TOLADDER) {
							g_nmi->_aniMan2 = 0;

							if (g_nmi->_sceneRect.left > 380)
								g_nmi->_currentScene->_x = 380 - g_nmi->_sceneRect.left;
						}
					}
				}
			} else {
				if (g_nmi->_aniMan->_movement && g_nmi->_aniMan->_movement->_id == MV_MAN_GOD)
					g_nmi->_aniMan2 = g_nmi->_aniMan;
			}

			sceneHandler04_springWobble();

			if (g_vars->scene04_var07 && !g_vars->scene04_handIsDown)
				sceneHandler04_leaveScene();

			if (g_vars->scene04_bottleIsDropped)
				sceneHandler04_liftBottle();

			if (g_vars->scene04_ladderClickable)
				sceneHandler04_clickLadder();

			if (g_vars->scene04_dudeInBottle && g_vars->scene04_hand->_movement)
				sceneHandler04_animOutOfBottle(0);

			if (g_vars->scene04_coinPut && g_vars->scene04_clockCanGo && !g_vars->scene04_handIsDown && !g_vars->scene04_soundPlaying)
				sceneHandler04_goClock();

			if (g_vars->scene04_dudeOnLadder) {
				if (!g_vars->scene04_soundPlaying) {
					g_nmi->startSceneTrack();

					g_nmi->_behaviorManager->updateBehaviors();
					return res;
				}

				g_vars->scene04_bigBallCounter++;

				if (g_vars->scene04_bigBallCounter > 600)
					sceneHandler04_bigBallWalkIn();
			}

			if (g_vars->scene04_soundPlaying) {
				g_nmi->_behaviorManager->updateBehaviors();

				return res;
			}

			g_nmi->startSceneTrack();

			g_nmi->_behaviorManager->updateBehaviors();

			return res;
		}
		break;

	case 29:
		{
			int picid = g_nmi->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if (g_vars->scene04_dudeInBottle) {
				sceneHandler04_animOutOfBottle(ex);

				break;
			}

			if (picid == PIC_SC4_LADDER) {
				if (!g_vars->scene04_kozyawkaOnLadder) {
					g_vars->scene04_sceneClickX = ex->_sceneClickX;
					g_vars->scene04_sceneClickY = ex->_sceneClickY;

					sceneHandler04_clickLadder();

					ex->_messageKind = 0;

					break;
				}

				sceneHandler04_gotoLadder(0);

				break;
			}

			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if ((ani && ani->_id == ANI_PLANK) || picid == PIC_SC4_PLANK) {
				sceneHandler04_clickPlank();

				ex->_messageKind = 0;
			} else if (g_vars->scene04_dudeOnLadder) {
				sceneHandler04_leaveLadder(ex);
			} else if (!ani || !canInteractAny(g_nmi->_aniMan, ani, ex->_param)) {
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picid, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic,ex->_param)) {
					if ((g_nmi->_sceneRect.right - ex->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1)
						|| (ex->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0))
						g_nmi->processArcade(ex);
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
		g_vars->scene04_handIsDown = false;
		g_vars->scene04_objectIsTaken = true;
		break;

	case MSG_SC4_DROPBOTTLE:
		sceneHandler04_dropBottle();
		break;

	case MSG_SC4_COINOUT:
		g_vars->scene04_clock->changeStatics2(ST_CLK_CLOSED);
		g_vars->scene04_coinPut = false;
		sceneHandler04_stopSound();

		if (g_vars->scene04_kozyawkiAni.size() && !g_vars->scene04_bottleIsTaken) {
			g_vars->scene04_handIsDown = true;

			if (g_vars->scene04_dudeInBottle)
				sceneHandler04_animOutOfBottle(0);

			sceneHandler04_handTake();
		}
		break;

	case MSG_SC4_KOZAWFALL:
		{
			ExCommand *exnew;

			if (g_vars->scene04_kozHeadRaised) {
				sceneHandler04_putKozyawkaBack(g_vars->scene04_lastKozyawka);

				g_vars->scene04_kozHeadRaised = 0;

				exnew = new ExCommand(0, 35, SND_4_010, 0, 0, 0, 1, 0, 0, 0);
			} else {
				exnew = new ExCommand(0, 35, SND_4_012, 0, 0, 0, 1, 0, 0, 0);
			}

			exnew->_z = 5;
			exnew->_excFlags |= 2;
			exnew->postMessage();
		}
		break;

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
		g_vars->scene04_coinPut = true;
		break;

	default:
		break;
	}

	updateSound();

	return 0;
}

} // End of namespace NGI
