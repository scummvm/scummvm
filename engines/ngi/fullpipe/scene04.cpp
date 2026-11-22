/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

void sceneHandler04_putKozyawkaBack(AniObject *ani);

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
	Inventory *inv = getGameLoaderInventory();
	inv->addItem(ANI_INV_COIN, 1);
	inv->recalcItemPos();
#endif

	g_vars->scene04_dudeOnLadder = false;
	g_vars->scene04_bottle = sc->findPictureObjectById(PIC_SC4_BOTTLE, 0);
	g_vars->scene04_hand = sc->findAniObjectById(ANI_HAND, -1);
	g_vars->scene04_plank = sc->findAniObjectById(ANI_PLANK, -1);
	g_vars->scene04_clock = sc->findAniObjectById(ANI_CLOCK, -1);
	g_vars->scene04_spring = sc->findAniObjectById(ANI_SPRING, -1);
	g_vars->scene04_mamasha = sc->findAniObjectById(ANI_MAMASHA_4, -1);
	g_vars->scene04_boot = sc->findAniObjectById(ANI_SC4_BOOT, -1);
	g_vars->scene04_ladder = nullptr;

	AniObject *koz = sc->findAniObjectById(ANI_KOZAWKA, -1);

	if (koz) {
		Movement *kozmov = koz->getMovement(MV_KZW_JUMP);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_phases.size() : kozmov->_phases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setCurFrame(i);

				if (kozmov->_framePosOffsets.size()) {
					g_vars->scene04_jumpingKozyawki[i] = kozmov->_framePosOffsets[kozmov->_curFrame];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpingKozyawki[i] = kozmov->_somePoint;
				}
			}
		}

		kozmov = koz->getMovement(MV_KZW_JUMPROTATE);
		if (kozmov) {
			uint kozsize = kozmov->_currMovement ? kozmov->_currMovement->_phases.size() : kozmov->_phases.size();

			for (uint i = 0; i < kozsize; i++) {
				kozmov->setCurFrame(i);

				if (kozmov->_framePosOffsets.size()) {
					g_vars->scene04_jumpRotateKozyawki[i] = kozmov->_framePosOffsets[kozmov->_curFrame];
				} else {
					kozmov->_somePoint.x = 0;
					kozmov->_somePoint.y = 0;
					g_vars->scene04_jumpRotateKozyawki[i] = kozmov->_somePoint;
				}
			}
		}
	}

	Interact *plank = getGameLoaderInteractionController()->findInteraction(ANI_PLANK, 0, 0);
	if (plank)
		plank->_flags |= 8;

	if (g_nmi->getStateId(sO_Jar_4) == g_nmi->getStateValue(sO_Jar_4, sO_UpsideDown)) {
		debugC(2, kDebugSceneLogic, "scene04: Jar is upside down");

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_kozyawkiObjList.clear();

		sc->findPictureObjectById(PIC_SC4_BOTTLE, 0)->_flags &= 0xfffb;
		sc->findPictureObjectById(PIC_SC4_MASK, 0)->_flags &= 0xfffb;
		sc->findAniObjectById(ANI_SPRING, 0)->_flags &= 0xfffb;

		g_vars->scene04_clockCanGo = false;
		g_vars->scene04_objectIsTaken = false;
	} else {
		debugC(2, kDebugSceneLogic, "scene04: Jar is NOT upside down");

		AniObject *spring = sc->findAniObjectById(ANI_SPRING, -1);

		if (spring)
			spring->_callback2 = scene04_springCallback;

		g_vars->scene04_bottleObjList.clear();
		g_vars->scene04_bottleObjList.push_back(sc->findPictureObjectById(PIC_SC4_BOTTLE, 0));
		g_vars->scene04_bottleObjList.push_back(sc->findPictureObjectById(PIC_SC4_MASK, 0));

		g_vars->scene04_kozyawkiObjList.clear();

		if (koz) {
			debugC(2, kDebugSceneLogic, "scene04: We're in koz");

			koz->loadMovements();

			koz->_statics = koz->getStatic(ST_KZW_EMPTY);
			koz->setPos(0, 0);
			koz->hide();

			g_vars->scene04_kozyawkiObjList.push_back(koz);

			for (int i = 0; i < 6; i++) {
				AniObject *koz1 = new AniObject(koz);

				sc->addAni(koz1, 1);
				koz1->_statics = koz->getStatic(ST_KZW_EMPTY);
				koz1->setPos(0, 0);
				koz1->hide();
				g_vars->scene04_kozyawkiObjList.push_back(koz1);
			}
		}
		sc->findPictureObjectById(PIC_SC4_BOTTLE2, 0)->_flags &= 0xfffb;

		g_vars->scene04_clockCanGo = true;
		g_vars->scene04_objectIsTaken = true;

		debugC(2, kDebugSceneLogic, "scene04: kozObjList size: %d", g_vars->scene04_kozyawkiObjList.size());
	}

	g_vars->scene04_bottleIsTaken = false;
	g_vars->scene04_soundPlaying = false;
	g_vars->scene04_kozyawkaOnLadder = false;
	g_vars->scene04_walkingKozyawka = nullptr;
	g_vars->scene04_bottleWeight = 2;
	g_vars->scene04_dynamicPhaseIndex = 0;

	g_vars->scene04_kozyawkiAni.clear();

	g_nmi->setState(sO_LowerPipe, g_nmi->getStateValue(sO_LowerPipe, sO_IsClosed));

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

	if (g_nmi->getStateId(sO_BigMumsy) != g_nmi->getStateValue(sO_BigMumsy, sO_IsGone))
		g_vars->scene04_mamasha->hide();

	g_vars->scene04_speaker = sc->findAniObjectById(ANI_SPEAKER_4, -1);
	g_vars->scene04_speaker->_callback2 = scene04_speakerCallback;
	g_vars->scene04_speaker->startMovement(MV_SPK4_PLAY, 0, -1);

	g_vars->scene04_speakerVariant = 0;
	g_vars->scene04_speakerPhase = 0;

	g_vars->scene04_musicStage = 0;

	g_nmi->initArcadeKeys("SC_4");
}

bool sceneHandler04_friesAreWalking() {
	if (g_vars->scene04_dudeOnLadder && g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
		int col = g_vars->scene04_ladder->getCurrentStep(g_nmi->_aniMan);
		if (col >= 3 && col <= 6 ) {
			Movement *koz;

			if (!g_vars->scene04_walkingKozyawka
				 || (koz = g_vars->scene04_walkingKozyawka->_movement) == nullptr
				 || koz->_id != MV_KZW_WALKPLANK
				 || koz->_curFrame < 10
				 || koz->_curFrame > 41)
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
	AniObject *ball = g_nmi->_currentScene->findAniObjectById(ANI_BIGBALL, -1);

	if (ball)
		for (uint i = 0; i < ball->_movements.size(); i++)
			ball->_movements[i]->_loopDelay = 73;

	g_vars->scene04_bigBallIn = true;
}

void sceneHandler04_clickBottle() {
	if (!g_vars->scene04_bottleIsTaken)
		g_vars->scene04_springOffset += 5;
}

void sceneHandler04_clickButton() {
	AniObject *but = g_nmi->_currentScene->findAniObjectById(ANI_BUTTON, -1);

	if (but) {
		if (!g_vars->scene04_clock->_movement ||
			(g_vars->scene04_clock->_movement->_id == MV_CLK_GO && g_vars->scene04_clock->_movement->_curFrame > 3 &&
			 g_vars->scene04_clock->_movement->_curFrame < 105)) {
			if (!g_vars->scene04_hand->_movement && !g_vars->scene04_bottleIsTaken) {
				but->startMovement(MV_BTN_CLICK, 0, -1);
				g_vars->scene04_hand->startMovement(MV_HND_POINT, 0, -1);
			}
		}
	}
}

void sceneHandler04_downLadder(int x, int y) {
	g_vars->scene04_ladder->startMove(g_nmi->_aniMan, x + g_vars->scene04_ladder->_entryOffsetX, y + g_vars->scene04_ladder->_entryOffsetY, 0, 0);
}

void sceneHandler04_walkClimbLadder(Command *ex) {
	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	Command *ex1 = new Command(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex1->_param = g_nmi->_aniMan->_copyId;
	ex1->_excFlags |= 2;

	mq->addCommandToEnd(ex1);

	Command *ex2 = new Command(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);

	ex2->_param = g_nmi->_aniMan->_copyId;
	ex2->_excFlags |= 2;

	mq->addCommandToEnd(ex2);

	Command *ex3;

	if (ex) {
		ex3 = ex->createClone();
	} else {
		ex3 = new Command(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex3->_excFlags |= 3;
	}

	mq->addCommandToEnd(ex3);

	mq->setFlags(mq->getFlags() | 1);

	mq->run(nullptr);

	g_vars->scene04_dudeOnLadder = 1;

	g_vars->scene04_ladder = new MctlLadder;
	g_vars->scene04_ladder->_x = 1089;
	g_vars->scene04_ladder->_y = 406;
	g_vars->scene04_ladder->_z = 12;
	g_vars->scene04_ladder->_stepX = 0;
	g_vars->scene04_ladder->_stepY = -40;
	g_vars->scene04_ladder->_entryOffsetX = 0;
	g_vars->scene04_ladder->_entryOffsetY = -60;

	g_vars->scene04_ladder->attachObject(g_nmi->_aniMan);

	if (g_vars->scene04_soundPlaying) {
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upStartId = MV_MAN_STARTLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upGoId = MV_MAN_GOLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upStopId = MV_MAN_STOPLADDER2;
		g_vars->scene04_ladder->_ladmovements.front()->staticIds[2] = ST_MAN_GOLADDER2;
	} else {
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upStartId = MV_MAN_STARTLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upGoId = MV_MAN_GOLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->movementIds->upStopId = MV_MAN_STOPLADDER;
		g_vars->scene04_ladder->_ladmovements.front()->staticIds[2] = ST_MAN_GOLADDER;
	}

	g_nmi->_aniMan->_z = 12;

	getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->deactivate();
	getGameLoaderInteractionController()->disable();
}

void sceneHandler04_clickLadder() {
	g_vars->scene04_dudePosX = g_nmi->_aniMan->_x;
	g_vars->scene04_dudePosY = g_nmi->_aniMan->_y;

	if (g_vars->scene04_dudeOnLadder) {
		if (!g_nmi->_aniMan->isIdle() || (g_nmi->_aniMan->_flags & 0x100)) {
			g_vars->scene04_ladderClickable = true;
		} else {
			int h3 = 3 * g_vars->scene04_ladder->_stepY;
			int half = abs(g_vars->scene04_ladder->_stepY) / 2;
			int start = g_vars->scene04_ladder->_y - g_vars->scene04_ladder->_entryOffsetY;
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
					Command *ex = new Command(0, 17, MSG_SC4_CLICKLADDER, 0, 0, 0, 1, 0, 0, 0);

					ex->_excFlags = 3;
					mq->addCommandToEnd(ex);

					postExCommand(g_nmi->_aniMan->_id, 2, 1095, 434, 0, -1);
				}
			} else {
				sceneHandler04_walkClimbLadder(nullptr);
			}
		}
	}
}

void sceneHandler04_jumpOnLadder() {
	if (g_nmi->_aniMan->_movement && g_nmi->_aniMan->_movement->_id != MV_MAN_LOOKLADDER)
		return;

	if (g_nmi->_aniMan->_statics->_staticsId != ST_MAN_STANDLADDER && g_nmi->_aniMan->_statics->_staticsId != ST_MAN_LADDERDOWN)
		return;

	g_nmi->_aniMan->setStatic(ST_MAN_LADDERDOWN);

	g_nmi->_aniMan->_flags |= 0x100;

	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	aniHandler.attachObject(ANI_MAN);

	mkQueue.ani = g_nmi->_aniMan;
	mkQueue.staticsId2 = ST_MAN_ONPLANK;
	mkQueue.x1 = 938;
	mkQueue.y1 = 442;
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_MAN_JUMPONPLANK;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->_flags |= 1;

		if (!mq->run(g_nmi->_aniMan))
			delete mq;

		g_nmi->_aniMan->_z = 10;
	}

	g_vars->scene04_ladderOffset = g_vars->scene04_ladder->getCurrentStep(g_nmi->_aniMan);
}

void sceneHandler04_clickPlank() {
	if (sceneHandler04_friesAreWalking())
		sceneHandler04_jumpOnLadder();
	else if (g_vars->scene04_dudeOnLadder)
		g_nmi->playSound(SND_4_033, 0);
	else if (!g_vars->scene04_soundPlaying)
		runQueue(QU_PNK_CLICK, 0);
}

void sceneHandler04_doDropBottle() {
	g_vars->scene04_bottleIsDropped = true;
	g_vars->scene04_bottleY = 10;
	g_vars->scene04_bottleWeight = 0;

	while (g_vars->scene04_kozyawkiAni.size()) {
		AniObject *koz = g_vars->scene04_kozyawkiAni.front();
		g_vars->scene04_kozyawkiAni.pop_front();

		for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it)
			if (*it == koz) {
				g_vars->scene04_bottleObjList.erase(it);
				break;
			}

		koz->setQueue(nullptr);
		koz->hide();

		g_vars->scene04_kozyawkiObjList.push_back(koz);
	}

	g_vars->scene04_hand->setStatic(ST_HND_EMPTY);

	g_vars->scene04_hand->setPos(429, 21);
	g_vars->scene04_hand->_z = 15;
}

void sceneHandler04_gotoLadder(Command *ex) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	aniHandler.attachObject(ANI_MAN);

	mkQueue.ani = g_nmi->_aniMan;
	mkQueue.staticsId2 = ST_MAN_UP;
	mkQueue.x1 = 1095;
	mkQueue.y1 = 434;
	mkQueue.destZ = 12;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_MAN_PLANKTOLADDER;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->removeCommand(mq->getCount() - 1, 1);

		Command *ex1 = new Command(ANI_MAN, 1, MV_MAN_TOLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex1->_excFlags = 2;
		ex1->_wait = 1;
		ex1->_param = -1;
		mq->addCommandToEnd(ex1);

		Command *ex2 = new Command(ANI_MAN, 1, MV_MAN_STOPLADDER, 0, 0, 0, 1, 0, 0, 0);
		ex2->_excFlags = 2;
		ex2->_wait = 1;
		ex2->_param = -1;
		mq->addCommandToEnd(ex2);

		Command *ex3 = new Command(g_nmi->_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
		ex3->_z = 256;
		ex3->_messageNum = 0;
		ex3->_excFlags |= 3;
		mq->addCommandToEnd(ex3);

		if (ex) {
			Command *ex4 = ex->createClone();

			mq->addCommandToEnd(ex4);
		}

		mq->setFlags(mq->getFlags() | 1);

		if (mq->run(g_nmi->_aniMan)) {
			g_nmi->_aniMan->_z = 12;
			g_nmi->_aniMan->_flags |= 0x100;
		} else {
			delete mq;
		}
	}

	g_vars->scene04_kozyawkaOnLadder = false;
}

void sceneHandler04_lowerPlank() {
	g_vars->scene04_plank->startMovement(MV_PNK_WEIGHTRIGHT, 0, -1);
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

	g_vars->scene04_ladder = nullptr;

	getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->activate();
	getGameLoaderInteractionController()->enable();
}

void sceneHandler04_manToBottle() {
	g_vars->scene04_bottleObjList.push_back(g_nmi->_aniMan);
	g_vars->scene04_springOffset = 5;
	g_vars->scene04_bottleWeight += 9;
	g_nmi->_aniMan2 = g_nmi->_aniMan;
	g_vars->scene04_dudeInBottle = 1;
}

void sceneHandler04_raisePlank() {
	g_vars->scene04_plank->startMovement(MV_PNK_WEIGHTLEFT, 0, -1);
}

MessageQueue *sceneHandler04_kozFly3(AniObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly3 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x1 = (int)(723.0 - phase * 185.0);
	mkQueue.y1 = 486;
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMP;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		Command *ex = new Command(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		for (int i = 0; i < 5; i++) {
			ex = new Command(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_copyId;
			mq->addCommandToEnd(ex);
		}

		ex = new Command(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);
	}

	return mq;
}

MessageQueue *sceneHandler04_kozFly5(AniObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly5 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_JUMPOUT;
	mkQueue.x1 = 525;
	mkQueue.y1 = (int)(344.0 - (double)(320 - g_vars->scene04_bottle->_y) * phase);
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPHIT;

	MessageQueue *mq1 = aniHandler.makeRunQueue(&mkQueue);

	mkQueue.reset();
	mkQueue.ani = ani;
	mkQueue.staticsId1 = ST_KZW_JUMPOUT;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x2 = 525;
	mkQueue.y2 = (int)(344.0 - (double)(320 - g_vars->scene04_bottle->_y) * phase);
	mkQueue.y1 = 486;
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 117;
	mkQueue.movementId = MV_KZW_JUMPOUT;

	MessageQueue *mq2 = aniHandler.makeRunQueue(&mkQueue);

	if (mq1 && mq2) {
		mq1->addCommandToEnd(mq2->getCommandByIndex(0)->createClone());

		delete mq2;
		mq2 = nullptr;

		Command *ex = new Command(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq1->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq1->addCommandToEnd(ex);

		for (int i = 0; i < 5; i++) {
			ex = new Command(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_copyId;
			mq1->addCommandToEnd(ex);
		}

		ex = new Command(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq1->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq1->addCommandToEnd(ex);
	}

	if (mq2)
		delete mq2;

	return mq1;
}

MessageQueue *sceneHandler04_kozFly6(AniObject *ani) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly6 (OK)");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = ST_KZW_SIT;
	mkQueue.x1 = 397 - 4 * g_nmi->_rnd.getRandomNumber(1);
	mkQueue.destZ = ani->_z;
	mkQueue.y1 = g_vars->scene04_bottle->_y - 4 * g_nmi->_rnd.getRandomNumber(1) + 109;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPROTATE;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		mq->removeCommand(mq->getCount() - 1, 1);

		Command *ex = new Command(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 1, MV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 1, MV_KZW_RAISEHEAD, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		g_vars->scene04_kozHeadRaised = true;
	}

	return mq;
}

void sceneHandler04_kozMove(Movement *mov, int from, int to, Common::Point *points, double phase) {
	for (int i = from; i < to; i++) {
		mov->setCurFrame(i);

		Common::Point *p;
		if (mov->_framePosOffsets.size()) {
			p = &mov->_framePosOffsets[mov->_curFrame];
		} else {
			p = &mov->_somePoint;
			p->x = 0;
			p->y = 0;
		}

		p->y = (int)((double)points[i].y * phase);
	}
}

MessageQueue *sceneHandler04_kozFly7(AniObject *ani, double phase) {
	AniHandler aniHandler;
	MakeQueueStruct mkQueue;

	debugC(2, kDebugSceneLogic, "scene04: kozFly7");

	aniHandler.attachObject(ANI_KOZAWKA);

	mkQueue.ani = ani;
	mkQueue.staticsId2 = 560;
	mkQueue.x1 = (int)(250.0 - phase * 100.0);
	mkQueue.y1 = 455;
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = 78;
	mkQueue.movementId = MV_KZW_JUMPROTATE;

	MessageQueue *mq = aniHandler.makeRunQueue(&mkQueue);

	if (mq) {
		sceneHandler04_kozMove(ani->getMovement(MV_KZW_JUMPROTATE), 1, 9, g_vars->scene04_jumpRotateKozyawki, phase * 0.5 + 1.5);

		ani->_z = 10;

		Command *ex = new Command(ANI_KOZAWKA, 1, MV_KZW_STANDUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 1, MV_KZW_TURN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		for (int i = 0; i < 2; i++) {
			ex = new Command(ANI_KOZAWKA, 1, rMV_KZW_GOR, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			ex->_param = ani->_copyId;
			mq->addCommandToEnd(ex);
		}

		ex = new Command(ANI_KOZAWKA, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);

		ex = new Command(ANI_KOZAWKA, 17, MSG_KOZAWRESTART, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = ani->_copyId;
		mq->addCommandToEnd(ex);
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
	g_vars->scene04_plank->setStatic(ST_PNK_WEIGHTRIGHT);

	if (!g_vars->scene04_walkingKozyawka)
		return;

	if (g_vars->scene04_walkingKozyawka->_movement) {
		if (g_vars->scene04_walkingKozyawka->_movement->_id == MV_KZW_WALKPLANK) {
			int dphase = g_vars->scene04_walkingKozyawka->_movement->_curFrame;

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

				g_vars->scene04_walkingKozyawka->setQueue(nullptr);
				g_vars->scene04_walkingKozyawka->_movement = nullptr;
				g_vars->scene04_walkingKozyawka->_statics = g_vars->scene04_walkingKozyawka->getStatic(ST_KZW_RIGHT);

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

					if (!mq->run(g_vars->scene04_walkingKozyawka) )
						delete mq;
				}
			}
		}
	}

	if (g_vars->scene04_ladderOffset > 3)
		g_nmi->_aniMan->moveToStatic(ST_MAN_LOOKPLANK);

	g_vars->scene04_kozyawkaOnLadder = true;
}

void sceneHandler04_doShowCoin() {
	AniObject *ani = g_nmi->_currentScene->findAniObjectById(ANI_SC4_COIN, -1);

	if (ani) {
		ani->show1(MV_BDG_OPEN, MV_MAN_GOU, MV_SC4_COIN_default, 0);

		ani->_z = 40;
	}
}

void sceneHandler04_stopSound() {
	g_vars->scene04_soundPlaying = false;

	g_nmi->stopSoundStream2();
}

void sceneHandler04_animOutOfBottle(Command *ex) {
	g_nmi->_aniMan->setStatic(ST_MAN_SIT);

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC4_MANFROMBOTTLE), 0, 0);

	if (ex) {
		Command *newex = ex->createClone();

		mq->addCommandToEnd(newex);
	  }

	mq->_flags |= 1;
	mq->run(nullptr);

	g_vars->scene04_dudeInBottle = false;
	g_nmi->_behaviorManager->enableAllQueuesForAni(g_nmi->_aniMan, 1);
}

void sceneHandler04_walkKozyawka() {
	debugC(1, kDebugSceneLogic, "scene04: walkKozyawka");

	if (g_vars->scene04_kozyawkiObjList.size()) {
		debugC(1, kDebugSceneLogic, "scene04: walkKozyawka: getting one");

		g_vars->scene04_walkingKozyawka = g_vars->scene04_kozyawkiObjList.front();
		g_vars->scene04_kozyawkiObjList.pop_front();

		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_KOZAW_WALK), 0, 1);
		mq->setObjectCopy(-1, g_vars->scene04_walkingKozyawka->_copyId);
		mq->run(nullptr);
	}
}

void sceneHandler04_bottleUpdateObjects(int off) {
	for (Common::List<GameObject *>::iterator it = g_vars->scene04_bottleObjList.begin(); it != g_vars->scene04_bottleObjList.end(); ++it) {
		if ((*it)->_objtype == kObjTypeStaticANIObject) {
			AniObject *st = static_cast<AniObject *>(*it);

			st->setPos(st->_x, off + st->_y);
		} else {
			GameObject *obj = *it;

			obj->setPos(obj->_x, off + obj->_y);
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

	int oldpos = g_vars->scene04_spring->getFullSize().y - oldDynIndex;

	if (g_vars->scene04_dynamicPhaseIndex) {
		if (!g_vars->scene04_spring->_movement)
			g_vars->scene04_spring->startMovement(MV_SPR_LOWER, 0, -1);

		g_vars->scene04_spring->_movement->setCurFrame(g_vars->scene04_dynamicPhaseIndex);
	} else {
		g_vars->scene04_spring->setStatic(ST_SPR_UP);
	}

	if (g_vars->scene04_dynamicPhaseIndex != oldDynIndex) {
		sceneHandler04_bottleUpdateObjects(oldpos - (g_vars->scene04_spring->getFullSize().y - g_vars->scene04_dynamicPhaseIndex));
	}
}

void sceneHandler04_leaveScene() {
	g_nmi->_aniMan2 = nullptr;

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC4_MANTOBOTTLE), 0, 0);
	Command *ex = nullptr;

	for (uint i = 0; i < mq->getCount(); i++) {
		if (mq->getCommandByIndex(i)->_cmd == 27) {
			ex = mq->getCommandByIndex(i);
			break;
		}
	}

	if (!ex) {
		error("sceneHandler04_leaveScene(): Cannot find exit");
	}

	ex->_y = g_vars->scene04_bottle->_y - 304;

	mq->run(nullptr);

	g_vars->scene04_var07 = false;
	g_vars->scene04_dudeOnLadder = 0;

	g_nmi->_behaviorManager->enableAllQueuesForAni(g_nmi->_aniMan, 0);

	g_nmi->updateMapPiece(PIC_MAP_P03, 1);
}

void sceneHandler04_liftBottle() {
	int newy = g_vars->scene04_bottleY + g_vars->scene04_spring->_y;

	g_vars->scene04_bottleY += 5;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_y);

	g_vars->scene04_spring->setPos(g_vars->scene04_spring->_x, newy);

	if (g_vars->scene04_bottle->_y >= 226) {
		sceneHandler04_bottleUpdateObjects(226 - g_vars->scene04_bottle->_y);

		g_vars->scene04_spring->setPos(g_vars->scene04_spring->_x, 437);
		g_vars->scene04_bottleIsDropped = false;
		g_vars->scene04_handIsDown = false;
		g_vars->scene04_objectIsTaken = true;
		g_vars->scene04_bottleWeight = 2;
		g_vars->scene04_springOffset = 10;
		g_vars->scene04_bottleIsTaken = false;

		g_nmi->setState(sO_LowerPipe, g_nmi->getStateValue(sO_LowerPipe, sO_IsClosed));
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
	runQueue(QU_SC4_GOCLOCK, 0);
	g_vars->scene04_soundPlaying = true;
	g_vars->scene04_coinPut = false;

	g_nmi->stopAllSoundStreams();

	sceneHandler04_startSounds("sc4_start.ogg", "sc4_loop.ogg", "sc4_stop2.ogg");

	g_vars->scene04_bigBallCounter = 0;
}

void sceneHandler04_bigBallOut() {
	AniObject *ball =  g_nmi->_currentScene->findAniObjectById(ANI_BIGBALL, -1);

	if (ball && ball->_flags & 4)
		for (uint i = 0; i < ball->_movements.size(); i++)
			ball->_movements[i]->_loopDelay = 0;

	g_vars->scene04_bigBallIn = false;
}

void sceneHandler04_leaveLadder(Command *ex) {
	if (!g_nmi->_aniMan->isIdle())
		return;

	if (!(g_nmi->_aniMan->_flags & 0x100)) {
		if (getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->_objtype == kObjTypeMctlCompound) {
			MctlCompound *mc = static_cast<MctlCompound *>(getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId));

			if (mc->_motionControllers[0]->_movGraphReactObj->hitTest(g_nmi->_sceneRect.left + ex->_x, g_nmi->_sceneRect.top + ex->_y)) {
				if (g_vars->scene04_ladder->getCurrentStep(g_nmi->_aniMan)) {
					MessageQueue *mq = g_vars->scene04_ladder->makeQueue(g_nmi->_aniMan, 0);

					if (mq) {
						mq->addCommandToEnd(ex->createClone());

						if (mq->run(g_nmi->_aniMan) )
							ex->_cmd = 0;
						else
							delete mq;

						if (g_vars->scene04_bigBallIn) {
							sceneHandler04_bigBallOut();
							return;
						}
					}
				} else {
					MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
					Command *ex1;

					if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN_LADDERDOWN) {
						ex1 = new Command(ANI_MAN, 1, MV_MAN_LOOKLADDERRV, 0, 0, 0, 1, 0, 0, 0);
						ex1->_param = g_nmi->_aniMan->_copyId;
						ex1->_excFlags |= 2;
						mq->addCommandToEnd(ex1);
					}

					ex1 = new Command(ANI_MAN, 1, MV_MAN_STARTLADDERD, 0, 0, 0, 1, 0, 0, 0);
					ex1->_param = g_nmi->_aniMan->_copyId;
					ex1->_excFlags |= 2;
					mq->addCommandToEnd(ex1);

					ex1 = new Command(ANI_MAN, 1, MV_MAN_FROMLADDER, 0, 0, 0, 1, 0, 0, 0);
					ex1->_param = g_nmi->_aniMan->_copyId;
					ex1->_excFlags |= 2;
					mq->addCommandToEnd(ex1);

					ex1 = ex->createClone();
					mq->addCommandToEnd(ex1);

					mq->setFlags(mq->getFlags() | 1);

					if (mq->run(g_nmi->_aniMan)) {
						if (g_vars->scene04_ladder)
							delete g_vars->scene04_ladder;

						g_vars->scene04_ladder = nullptr;
						g_vars->scene04_dudeOnLadder = 0;

						ex->_cmd = 0;

						mc->activate();
						getGameLoaderInteractionController()->enable();
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
	g_vars->scene04_clock->setStatic(ST_CLK_CLOSED);

	if (g_vars->scene04_kozyawkiAni.size()) {
		if (g_vars->scene04_kozyawkiAni.size() == 1) {
			runQueue(QU_HND_TAKE1, 0);
			g_vars->scene04_objectIsTaken = false;
		} else {
			runQueue((g_vars->scene04_kozyawkiAni.size() != 2) ? QU_HND_TAKEBOTTLE : QU_HND_TAKE2, 0);
			g_vars->scene04_objectIsTaken = false;
		}
	} else {
		runQueue(QU_HND_TAKE0, 0);
		g_vars->scene04_objectIsTaken = false;
	}
}

void sceneHandler04_putKozyawkaBack(AniObject *ani) {
	debugC(2, kDebugSceneLogic, "scene04: putKozyawkaBack");

	g_vars->scene04_bottleObjList.push_back(ani);
	g_vars->scene04_kozyawkiAni.push_back(ani);

	g_vars->scene04_bottleWeight += 2;
	g_vars->scene04_walkingKozyawka = nullptr;
	g_vars->scene04_lastKozyawka = nullptr;

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
	AniObject *ball =  g_nmi->_currentScene->findAniObjectById(ANI_BIGBALL, -1);

	if (g_vars->scene04_dudeOnLadder
		 && (!ball || !(ball->_flags & 4))
		 && g_vars->scene04_ladder->getCurrentStep(g_nmi->_aniMan) > 3) {

		if (!g_nmi->_rnd.getRandomNumber(49)) {
			if (g_vars->scene04_bigBallFromLeft)
				runQueue(QU_BALL_WALKR, 0);
			else
				runQueue(QU_BALL_WALKL, 0);

			g_vars->scene04_bigBallFromLeft = !g_vars->scene04_bigBallFromLeft;

			sceneHandler04_checkBigBallClick();

			g_vars->scene04_bigBallCounter = 0;
		}
	}
}

void sceneHandler04_doTakeBottle() {
	g_vars->scene04_bottleIsTaken = true;
	g_vars->scene04_hand->_z = 5;

	g_nmi->setState(sO_LowerPipe, g_nmi->getStateValue(sO_LowerPipe, sO_IsOpened));
}

void sceneHandler04_takeKozyawka() {
	if (g_vars->scene04_kozyawkiAni.size() > 0) {
		if (g_vars->scene04_kozyawkiAni.size() == 1)
			g_vars->scene04_objectIsTaken = true;

		AniObject *koz = g_vars->scene04_kozyawkiAni.front();
		g_vars->scene04_kozyawkiAni.pop_front();

		if (koz) {
			koz->setQueue(nullptr);
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

void sceneHandler04_testPlank(Command *ex) {
	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (!mq)
		return;

	if (g_vars->scene04_plank->_movement || !g_vars->scene04_plank->_statics || g_vars->scene04_plank->_statics->_staticsId != ST_PNK_WEIGHTLEFT) {
		mq->getCommandByIndex(0)->_messageNum = MV_KZW_TOHOLERV;
	} else {
		mq->getCommandByIndex(0)->_messageNum = MV_KZW_WALKPLANK;
	}
}

void sceneHandler04_updateBottle() {
	int yoff;

	if (g_vars->scene04_hand->_movement)
		yoff = g_vars->scene04_hand->_movement->_y;
	else
		yoff = g_vars->scene04_hand->_y;

	int newy = g_vars->scene04_hand->getSomeXY().y + yoff + 140;

	sceneHandler04_bottleUpdateObjects(newy - g_vars->scene04_spring->_y);

	g_vars->scene04_spring->setPos(g_vars->scene04_spring->_x, newy);
}

void sceneHandler04_doWinArcade() {
	if (g_nmi->getStateId(sO_LowerPipe) == g_nmi->getStateValue(sO_LowerPipe, sO_IsClosed)
		&& g_vars->scene04_soundPlaying) {
		g_vars->scene04_clock->setStatic(ST_CLK_CLOSED);
		g_vars->scene04_hand->setStatic(ST_HND_EMPTY);

		runQueue(QU_HND_TAKEBOTTLE, 1);

		if (g_vars->scene04_walkingKozyawka) {
			g_vars->scene04_kozyawkiObjList.push_back(g_vars->scene04_walkingKozyawka);

			g_vars->scene04_walkingKozyawka->setStatic(ST_KZW_EMPTY);
			g_vars->scene04_walkingKozyawka->hide();
			g_vars->scene04_walkingKozyawka = nullptr;
		}

		g_vars->scene04_objectIsTaken = false;
		g_vars->scene04_soundPlaying = false;

		getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->activate();

		getGameLoaderInteractionController()->enable();

		g_nmi->stopSoundStream2();
	}
}

int sceneHandler04(Command *ex) {
	if (ex->_cmd != 17)
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
			sceneHandler04_animOutOfBottle(nullptr);

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
			g_vars->scene04_walkingKozyawka = nullptr;
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
			g_vars->scene04_dudePosX = g_nmi->_aniMan->_x;
			g_vars->scene04_dudePosY = g_nmi->_aniMan->_y;

			int res = 0;

			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan->_x < g_nmi->_sceneRect.left + 200) {
					g_nmi->_currentScene->_x = g_nmi->_aniMan->_x - g_nmi->_sceneRect.left - 300;
					g_nmi->_aniMan->_x = g_vars->scene04_dudePosX;
				}
				if (g_nmi->_aniMan->_x > g_nmi->_sceneRect.right - 200) {
					g_nmi->_currentScene->_x = g_nmi->_aniMan->_x - g_nmi->_sceneRect.right + 300;
				}

				res = 1;
			}

			g_nmi->sceneAutoScrolling();

			if (g_nmi->_aniMan2) {
				if (g_vars->scene04_soundPlaying) {
					if (g_nmi->_aniMan->_movement) {
						if (g_nmi->_aniMan->_movement->_id == MV_MAN_TOLADDER) {
							g_nmi->_aniMan2 = nullptr;

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
				sceneHandler04_animOutOfBottle(nullptr);

			if (g_vars->scene04_coinPut && g_vars->scene04_clockCanGo && !g_vars->scene04_handIsDown && !g_vars->scene04_soundPlaying)
				sceneHandler04_goClock();

			if (g_vars->scene04_dudeOnLadder) {
				if (!g_vars->scene04_soundPlaying) {
					g_nmi->startSceneTrack();

					g_nmi->_behaviorManager->update();
					return res;
				}

				g_vars->scene04_bigBallCounter++;

				if (g_vars->scene04_bigBallCounter > 600)
					sceneHandler04_bigBallWalkIn();
			}

			if (g_vars->scene04_soundPlaying) {
				g_nmi->_behaviorManager->update();

				return res;
			}

			g_nmi->startSceneTrack();

			g_nmi->_behaviorManager->update();

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

					ex->_cmd = 0;

					break;
				}

				sceneHandler04_gotoLadder(nullptr);

				break;
			}

			AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);

			if ((ani && ani->_id == ANI_PLANK) || picid == PIC_SC4_PLANK) {
				sceneHandler04_clickPlank();

				ex->_cmd = 0;
			} else if (g_vars->scene04_dudeOnLadder) {
				sceneHandler04_leaveLadder(ex);
			} else if (!ani || !canInteractAny(g_nmi->_aniMan, ani, ex->_param)) {
				PictureObject *pic = g_nmi->_currentScene->findPictureObjectById(picid, 0);

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
		sceneHandler04_doWinArcade();
		break;

	case MSG_SC4_HANDOVER:
		g_vars->scene04_handIsDown = false;
		g_vars->scene04_objectIsTaken = true;
		break;

	case MSG_SC4_DROPBOTTLE:
		sceneHandler04_doDropBottle();
		break;

	case MSG_SC4_COINOUT:
		g_vars->scene04_clock->setStatic(ST_CLK_CLOSED);
		g_vars->scene04_coinPut = false;
		sceneHandler04_stopSound();

		if (g_vars->scene04_kozyawkiAni.size() && !g_vars->scene04_bottleIsTaken) {
			g_vars->scene04_handIsDown = true;

			if (g_vars->scene04_dudeInBottle)
				sceneHandler04_animOutOfBottle(nullptr);

			sceneHandler04_handTake();
		}
		break;

	case MSG_SC4_KOZAWFALL:
		{
			Command *exnew;

			if (g_vars->scene04_kozHeadRaised) {
				sceneHandler04_putKozyawkaBack(g_vars->scene04_lastKozyawka);

				g_vars->scene04_kozHeadRaised = 0;

				exnew = new Command(0, 35, SND_4_010, 0, 0, 0, 1, 0, 0, 0);
			} else {
				exnew = new Command(0, 35, SND_4_012, 0, 0, 0, 1, 0, 0, 0);
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
		sceneHandler04_doShowCoin();
		break;

	case MSG_TAKEBOTTLE:
		sceneHandler04_doTakeBottle();
		break;

	case MSG_GOTOLADDER:
		sceneHandler04_gotoLadder(nullptr);
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
