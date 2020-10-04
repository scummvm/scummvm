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

namespace NGI {

struct Hanger {
	StaticANIObject *ani;
	int field_4;
	int field_8;
	int phase;
};

void scene09_setupGrit(Scene *sc) {
	if (g_vars->scene09_grit->_statics->_staticsId == ST_GRT9_GRIT) {
		if (!getGameLoaderInventory()->getCountItemsWithId(ANI_INV_COIN)) {
			if (g_nmi->getObjectState(sO_CoinSlot_1) == g_nmi->getObjectEnumState(sO_CoinSlot_1, sO_Empty)
				&& (g_vars->swallowedEgg1->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg2->_value.intValue == ANI_INV_EGGBOOT || g_vars->swallowedEgg3->_value.intValue == ANI_INV_EGGBOOT)) {
				Scene *oldsc = g_nmi->_currentScene;
				g_nmi->_currentScene = sc;
				g_vars->scene09_grit->changeStatics2(ST_GRT9_NORM);
				g_nmi->_currentScene = oldsc;
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

	g_vars->scene09_flyingBalls.clear();

	for (uint i = 0; i < g_vars->scene09_hangers.size(); i++)
		delete g_vars->scene09_hangers[i];

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
		sc->addStaticANIObject(ani, 1);

		hng = new Hanger;

		hng->ani = ani;
		hng->phase = 0;
		hng->field_4 = 0;
		hng->field_8 = 0;

		g_vars->scene09_hangers.push_back(hng);
	}

	g_vars->scene09_sceneBalls.clear();

	StaticANIObject *newball1 = new StaticANIObject(sc->getStaticANIObject1ById(ANI_BALL9, -1));
	newball1->setAlpha(0xc8);

	for (int i = 0; i < 4; i++) {
		StaticANIObject *newball = new StaticANIObject(newball1);

		newball->setAlpha(0xc8);
		g_vars->scene09_sceneBalls.push_back(newball);

		sc->addStaticANIObject(newball, 1);
	}

	delete newball1;

	g_nmi->setObjectState(sO_RightStairs_9, g_nmi->getObjectEnumState(sO_RightStairs_9, sO_IsClosed));

	GameVar *eggvar = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = eggvar->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = eggvar->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = eggvar->getSubVarByName(sO_Egg3);

	scene09_setupGrit(sc);

	g_nmi->initArcadeKeys("SC_9");

	g_nmi->lift_setButton(sO_Level1, ST_LBN_1N);

	g_nmi->setArcadeOverlay(PIC_CSR_ARCADE4);
}

int sceneHandler09_updateScreenCallback() {
	int res = g_nmi->drawArcadeOverlay(g_nmi->_objectIdAtCursor == ANI_VISUNCHIK || g_vars->scene09_interactingHanger >= 0);

	if (!res)
		g_nmi->_updateScreenCallback = 0;

	return res;
}

int scene09_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_vars->scene09_interactingHanger < 0) {
		if (g_nmi->_objectIdAtCursor == ANI_VISUNCHIK) {
			if (g_nmi->_cursorId == PIC_CSR_ITN)
				g_nmi->_updateScreenCallback = sceneHandler09_updateScreenCallback;
		} else {
			if (g_nmi->_objectIdAtCursor == PIC_SC9_LADDER_R && g_nmi->_cursorId == PIC_CSR_ITN)
				g_nmi->_cursorId = (g_vars->scene09_dudeY < 350) ? PIC_CSR_GOD : PIC_CSR_GOU;
		}
	} else {
		g_nmi->_cursorId = PIC_CSR_ITN;
	}

	return g_nmi->_cursorId;
}

void sceneHandler09_winArcade() {
	if (g_vars->scene09_gulper->_flags & 4) {
		g_vars->scene09_gulper->changeStatics2(ST_GLT_SIT);
		g_vars->scene09_gulper->startAnim(MV_GLT_FLYAWAY, 0, -1);

		g_nmi->setObjectState(sO_Jug, g_nmi->getObjectEnumState(sO_Jug, sO_Unblocked));
		g_nmi->setObjectState(sO_RightStairs_9, g_nmi->getObjectEnumState(sO_RightStairs_9, sO_IsOpened));

		g_vars->scene09_gulperIsPresent = false;
	}
}

void sceneHandler09_startAuntie() {
	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_TTA9_GOL), 0, 1);

	mq->getExCommandByIndex(0)->_x = g_nmi->_sceneRect.right + 30;
	mq->chain(0);
}

void sceneHandler09_spitterClick() {
	debugC(2, kDebugSceneLogic, "scene09: spitterClick");

	if (g_vars->scene09_spitter->_flags & 4) {
		PicAniInfo info;

		g_vars->scene09_spitter->getPicAniInfo(info);
		g_vars->scene09_spitter->_messageQueueId = 0;
		g_vars->scene09_spitter->changeStatics2(ST_PLV_SIT);

		int x = g_vars->scene09_spitter->_ox - 10;
		int y = g_vars->scene09_spitter->_oy + 145;

		g_vars->scene09_spitter->setPicAniInfo(info);

		if (ABS(x - g_nmi->_aniMan->_ox) > 1 || ABS(y - g_nmi->_aniMan->_oy) > 1) {
			MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, x, y, 1, ST_MAN_UP);

			if (mq) {
				ExCommand *ex = new ExCommand(0, 17, MSG_SC9_PLVCLICK, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags = 2;
				mq->addExCommandToEnd(ex);

				postExCommand(g_nmi->_aniMan->_id, 2, x, y, 0, -1);
			}
		} else {
			if (!g_nmi->_aniMan->_movement) {
				g_vars->scene09_spitter->changeStatics2(ST_PLV_SIT);
				g_vars->scene09_spitter->hide();

				g_nmi->_aniMan->startAnim(MV_MAN9_SHOOT, 0, -1);

				g_nmi->stopAllSoundInstances(SND_9_006);
			}

			g_nmi->_aniMan2 = 0;

			if (g_nmi->_sceneRect.left < 800)
				g_nmi->_currentScene->_x = 800 - g_nmi->_sceneRect.left;
		}
	}
}

void sceneHandler09_eatBall() {
	debugC(2, kDebugSceneLogic, "scene09: eatBall");

	if (g_vars->scene09_flyingBall) {
		g_vars->scene09_flyingBall->hide();

		g_vars->scene09_flyingBalls.pop_back();
		//g_vars->scene09_sceneBalls.pop_back();

		g_vars->scene09_flyingBall = 0;
		g_vars->scene09_numSwallenBalls++;

		if (g_vars->scene09_numSwallenBalls >= 3) {
			MessageQueue *mq = g_vars->scene09_gulper->getMessageQueue();

			if (mq) {
				ExCommand *ex = new ExCommand(ANI_GLOTATEL, 1, MV_GLT_FLYAWAY, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 2;

				mq->addExCommandToEnd(ex);
			}

			g_nmi->setObjectState(sO_Jug, g_nmi->getObjectEnumState(sO_Jug, sO_Unblocked));
			g_nmi->setObjectState(sO_RightStairs_9, g_nmi->getObjectEnumState(sO_RightStairs_9, sO_IsOpened));

			g_vars->scene09_gulperIsPresent = false;
		}
	}
}

void sceneHandler09_showBall() {
	debugC(2, kDebugSceneLogic, "scene09: showBall");

	if (g_vars->scene09_sceneBalls.size()) {
		StaticANIObject *ani = g_vars->scene09_sceneBalls.front();
		g_vars->scene09_sceneBalls.push_back(ani);
		g_vars->scene09_sceneBalls.remove_at(0);

		g_vars->scene09_flyingBalls.insert_at(0, ani);

		ani->show1(g_nmi->_aniMan->_ox + 94, g_nmi->_aniMan->_oy - 162, MV_BALL9_EXPLODE, 0);
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

void sceneHandler09_collideBall(uint num) {
	debugC(2, kDebugSceneLogic, "scene09: collideBall");

	if (g_vars->scene09_gulperIsPresent) {
		g_vars->scene09_flyingBall = g_vars->scene09_flyingBalls[num];

		if (g_vars->scene09_gulper) {
			g_vars->scene09_gulper->changeStatics2(ST_GLT_SIT);

			MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC9_EATBALL), 0, 0);

			mq->setFlags(mq->getFlags() | 1);

			if (!mq->chain(g_vars->scene09_gulper))
				delete mq;
		}
	}
}

void sceneHandler09_ballExplode(uint num) {
	debugC(2, kDebugSceneLogic, "scene09: ballExplode(%d) of %d", num, g_vars->scene09_flyingBalls.size());

	StaticANIObject *ball = g_vars->scene09_flyingBalls[num];

	g_vars->scene09_flyingBalls.remove_at(num);

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC9_BALLEXPLODE), 0, 1);

	mq->setParamInt(-1, ball->_odelay);

	if (!mq->chain(ball))
		delete mq;
}

void sceneHandler09_checkHangerCollide() {
	for (uint b = 0; b < g_vars->scene09_flyingBalls.size(); b++) {
		StaticANIObject *ball = g_vars->scene09_flyingBalls[b];

		int newx = ball->_ox + 5;

		ball->setOXY(newx, ball->_oy);

		if (newx <= 1398 || g_vars->scene09_flyingBall) {
			if (g_vars->scene09_gulperIsPresent)
				goto LABEL_11;
		} else if (g_vars->scene09_gulperIsPresent) {
			sceneHandler09_collideBall(b);
			continue;
		}

		if (newx > 1600) {
			sceneHandler09_ballExplode(b);
			continue;
		}

	LABEL_11:
		bool hit;

		for (int i = 0; i < g_vars->scene09_numMovingHangers; i++) {
			for (int j = 0; j < 4; j++) {
				int x1 = newx + g_vars->scene09_hangerOffsets[j].x;
				int y1 = ball->_oy + g_vars->scene09_hangerOffsets[j].y;

				// Check 2 pixels to compensate cord width
				hit = g_vars->scene09_hangers[i]->ani->isPixelHitAtPos(x1, y1)
							&& g_vars->scene09_hangers[i]->ani->isPixelHitAtPos(x1 + 10, y1);

				if (hit) {
					sceneHandler09_ballExplode(b);
					break;
				}
			}

			if (hit)
				break;
		}
	}
}

void sceneHandler09_hangerStartCycle() {
	StaticANIObject *ani = g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani;

	if (ani->_movement) {
		ani->startAnim(MV_VSN_CYCLE2, 0, -1);
		g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->field_8 = 0;
		g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase = g_vars->scene09_intHangerPhase + (g_nmi->_mouseScreenPos.y - g_vars->scene09_clickY) / 2;

		if (g_vars->scene09_intHangerMaxPhase != -1000 && g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase != g_vars->scene09_intHangerMaxPhase) {
			ExCommand *ex = new ExCommand(0, 35, SND_9_019, 0, 0, 0, 1, 0, 0, 0);

			ex->_z = 1;
			ex->_excFlags |= 2;
			ex->postMessage();

			g_vars->scene09_intHangerMaxPhase = -1000;
		}
	} else {
		g_vars->scene09_interactingHanger = -1;
	}
}

void scene09_visCallback(int *phase) {
	// do nothing
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
		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enableFlag24();

		g_vars->scene09_dudeIsOnLadder = false;
		break;

	case MSG_SC9_TOLADDER:
		getCurrSceneSc2MotionController()->deactivate();
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

	case 367:
		if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS) {
			g_nmi->_needRestart = true;
			return 0;
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_nmi->_aniMan2) {
				int x = g_nmi->_aniMan2->_ox;

				g_vars->scene09_dudeY = g_nmi->_aniMan2->_oy;

				if (x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = x - g_nmi->_sceneRect.left - 300;

				if (x > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = x - g_nmi->_sceneRect.right + 300;

				res = 1;

				g_nmi->sceneAutoScrolling();
			} else {
				if (g_nmi->_aniMan->_movement && g_nmi->_aniMan->_movement->_id != MV_MAN9_SHOOT)
					g_nmi->_aniMan2 = g_nmi->_aniMan;
			}

			sceneHandler09_cycleHangers();
			sceneHandler09_limitHangerPhase();
			sceneHandler09_checkHangerCollide();

			if (g_vars->scene09_interactingHanger >= 0)
				sceneHandler09_hangerStartCycle();

			g_nmi->_behaviorManager->updateBehaviors();

			g_nmi->startSceneTrack();

			return res;
		}
		break;

	case 30:
		if (g_vars->scene09_interactingHanger >= 0)  {
			if (ABS(g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->phase) < 15) {
				g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani->_callback2 = 0; // Really NULL
				g_vars->scene09_hangers[g_vars->scene09_interactingHanger]->ani->changeStatics2(ST_VSN_NORMAL);
			}
		}

		g_vars->scene09_interactingHanger = -1;

		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

			if (ani) {
				if (ani->_id == ANI_PLEVATEL) {
					sceneHandler09_spitterClick();
					break;
				}

				if (ani->_id == ANI_VISUNCHIK) {
					debugC(2, kDebugSceneLogic, "scene09: VISUNCHIK");

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
							g_vars->scene09_hangers[hng]->ani->_callback2 = scene09_visCallback;
						}

						ExCommand *ex = new ExCommand(0, 35, SND_9_018, 0, 0, 0, 1, 0, 0, 0);

						ex->_z = 1;
						ex->_excFlags |= 2;
						ex->postMessage();
					}

					break;
				}
			}

			if (g_vars->scene09_dudeIsOnLadder && g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY) == PIC_SC9_LADDER_R
				&& !cmd->_param && !g_nmi->_aniMan->_movement) {
				handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->getPictureObjectById(PIC_SC9_LADDER_R, 0), 0);
			}

			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1) || (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0))
						g_nmi->processArcade(cmd);
				}
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
