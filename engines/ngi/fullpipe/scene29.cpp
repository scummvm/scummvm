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

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "ngi/interaction.h"
#include "ngi/behavior.h"


#define DEBUG 0

namespace NGI {

struct WalkingBearder {
	AniObject *ani;
	int wbflag;
	int wbcounter;
};

void scene29_initScene(Scene *sc) {
	g_vars->scene29_porter = sc->findAniObjectById(ANI_PORTER, -1);
	g_vars->scene29_shooter1 = sc->findAniObjectById(ANI_SHOOTER1, -1);
	g_vars->scene29_shooter2 = sc->findAniObjectById(ANI_SHOOTER2, -1);
	g_vars->scene29_ass = sc->findAniObjectById(ANI_ASS, -1);

	g_vars->scene29_greenBalls.clear();
	g_vars->scene29_flyingGreenBalls.clear();

	AniObject *ani = sc->findAniObjectById(ANI_SHELL_GREEN, -1);
	g_vars->scene29_greenBalls.push_back(ani);

	for (int i = 0; i < 2; i++) {
		AniObject *newani = new AniObject(ani);

		sc->addAni(newani, 1);

		g_vars->scene29_greenBalls.push_back(newani);
	}

	g_vars->scene29_redBalls.clear();
	g_vars->scene29_flyingRedBalls.clear();

	ani = sc->findAniObjectById(ANI_SHELL_RED, -1);

	g_vars->scene29_redBalls.push_back(ani);

	for (int i = 0; i < 2; i++) {
		AniObject *newani = new AniObject(ani);

		sc->addAni(newani, 1);

		g_vars->scene29_redBalls.push_back(newani);
	}

	g_vars->scene29_bearders.clear();

	ani = new AniObject(g_nmi->findScene(SC_COMMON)->findAniObjectById(ANI_BEARDED_CMN, -1));

	ani->_statics = ani->getStatic(ST_BRDCMN_EMPTY);

	sc->addAni(ani, 1);

	WalkingBearder *wb = new WalkingBearder;

	wb->ani = ani;
	wb->wbflag = 0;
	wb->wbcounter = 0;

	g_vars->scene29_bearders.push_back(wb);

	g_vars->scene29_manIsRiding = false;
	g_vars->scene29_arcadeIsOn = false;
	g_vars->scene29_reachedFarRight = false;
	g_vars->scene29_rideBackEnabled = false;
	g_vars->scene29_shootCountdown = 0;
	g_vars->scene29_shootDistance = 75;
	g_vars->scene29_manIsHit = false;
	g_vars->scene29_scrollSpeed = 0;
	g_vars->scene29_scrollingDisabled = false;
	g_vars->scene29_hitBall = 0;

	g_nmi->setArcadeOverlay(PIC_CSR_ARCADE8);
}

void sceneHandler29_winArcade() {
	if (g_vars->scene29_shooter2->_flags & 4) {
		g_vars->scene29_shootCountdown = 0;

		g_vars->scene29_shooter1->setStatic(ST_STR1_STAND);
		g_vars->scene29_shooter2->setStatic(ST_STR2_STAND);

		g_vars->scene29_shooter2->_flags &= 0xFFFB;

		AniObject *ani;

		while (g_vars->scene29_flyingGreenBalls.size()) {
			ani = g_vars->scene29_flyingGreenBalls.front();
			g_vars->scene29_flyingGreenBalls.remove_at(0);

			g_vars->scene29_greenBalls.push_back(ani);

			ani->hide();
		}

		while (g_vars->scene29_flyingRedBalls.size()) {
			ani = g_vars->scene29_flyingRedBalls.front();
			g_vars->scene29_flyingRedBalls.remove_at(0);

			g_vars->scene29_redBalls.push_back(ani);

			ani->hide();
		}

		g_vars->scene29_ass->setQueue(nullptr);
		g_vars->scene29_ass->_flags &= 0xFFFB;

		runQueue(QU_SC29_ESCAPE, 1);
	}

	g_nmi->setState(sO_LeftPipe_29, g_nmi->getStateValue(sO_LeftPipe_29, sO_IsOpened));
}

void sceneHandler29_shootGreen() {
	if (g_vars->scene29_greenBalls.size()) {
		int x = g_vars->scene29_shooter1->_x - 113;
		int y = g_vars->scene29_shooter1->_y - 48;
		AniObject *ani = g_vars->scene29_greenBalls.front();
		g_vars->scene29_greenBalls.remove_at(0);

		ani->show1(x, y, MV_SHG_NORM, 0);
		ani->_z = 5;

		g_vars->scene29_flyingGreenBalls.push_back(ani);
	}
}

void sceneHandler29_shootRed() {
	if (g_vars->scene29_redBalls.size()) {
		int x = g_vars->scene29_shooter1->_x - 101;
		int y = g_vars->scene29_shooter1->_y - 14;
		AniObject *ani = g_vars->scene29_redBalls.front();
		g_vars->scene29_redBalls.remove_at(0);

		ani->show1(x, y, MV_SHR_NORM, 0);
		ani->_z = 5;

		g_vars->scene29_flyingRedBalls.push_back(ani);
	}
}

void sceneHandler29_manJump() {
	if (!g_nmi->_aniMan->_movement || g_nmi->_aniMan->_movement->_id == MV_MAN29_RUN || g_nmi->_aniMan->_movement->_id == MV_MAN29_STANDUP) {
		g_vars->scene29_rideBackEnabled = false;
		g_vars->scene29_manIsHit = false;
		g_vars->scene29_reachedFarRight = true;

		g_nmi->_aniMan->setStatic(ST_MAN29_RUNR);
		g_nmi->_aniMan->startMovement(MV_MAN29_JUMP, 0, -1);
	}

	g_vars->scene29_manX = g_nmi->_aniMan->_x;
	g_vars->scene29_manY = g_nmi->_aniMan->_y;
}

void sceneHandler29_manBend() {
	if (!g_nmi->_aniMan->_movement || g_nmi->_aniMan->_movement->_id == MV_MAN29_RUN || g_nmi->_aniMan->_movement->_id == MV_MAN29_STANDUP) {
		g_vars->scene29_rideBackEnabled = false;
		g_vars->scene29_manIsHit = false;
		g_vars->scene29_reachedFarRight = true;

		g_nmi->_aniMan->setStatic(ST_MAN29_RUNR);
		g_nmi->_aniMan->startMovement(MV_MAN29_BEND, 0, -1);
	}

	g_vars->scene29_manX = g_nmi->_aniMan->_x;
	g_vars->scene29_manY = g_nmi->_aniMan->_y;
}

bool sceneHandler29_checkRedBallHit(AniObject *ani, int maxx) {
#if DEBUG
	return false;
#endif

	if (!g_vars->scene29_arcadeIsOn || g_vars->scene29_manIsHit)
		return false;

	if ((ani->_x >= g_vars->scene29_manX + 42 || ani->_x <= g_vars->scene29_manX + 8)
		&& (ani->_x < g_vars->scene29_manX + 8 || maxx > g_vars->scene29_manX + 27))
		return false;

	if (!g_nmi->_aniMan->_movement)
		return true;

	int phase = g_nmi->_aniMan->_movement->_curFrame;

	if (g_nmi->_aniMan->_movement->_id != MV_MAN29_BEND && g_nmi->_aniMan->_movement->_id != MV_MAN29_RUN
		&& (g_nmi->_aniMan->_movement->_id != MV_MAN29_JUMP || (phase >= 3 && phase <= 6)))
		return false;
	else
		return true;
}

bool sceneHandler29_checkGreenBallHit(AniObject *ani, int maxx) {
#if DEBUG
	return false;
#endif

	if (!g_vars->scene29_arcadeIsOn || g_vars->scene29_manIsHit)
		return false;

	if (ani->_x >= g_vars->scene29_manX + 40) {
		if (maxx > g_vars->scene29_manX + 27)
			return false;
	} else {
		if (ani->_x <= g_vars->scene29_manX + 10) {
			if (ani->_x < g_vars->scene29_manX + 40)
				return false;

			if (maxx > g_vars->scene29_manX + 27)
				return false;
		}
	}

	if (!g_nmi->_aniMan->_movement)
		return true;

	if (g_nmi->_aniMan->_movement->_id == MV_MAN29_JUMP)
		return true;

	if (g_nmi->_aniMan->_movement->_id == MV_MAN29_RUN)
		return true;

	if (g_nmi->_aniMan->_movement->_id == MV_MAN29_BEND) {
		if (g_nmi->_aniMan->_movement->_curFrame < 1 || g_nmi->_aniMan->_movement->_curFrame > 5)
			return true;
	}

	return false;
}

void sceneHandler29_manHit() {
	MakeQueueStruct mkQueue;

	g_vars->scene29_manIsHit = true;

	g_nmi->_aniMan->setStatic(ST_MAN29_RUNR);
	g_nmi->_aniMan->setPos(g_vars->scene29_manX, g_vars->scene29_manY);

	mkQueue.ani = g_nmi->_aniMan;
	mkQueue.staticsId2 = ST_MAN29_SITR;
	mkQueue.y1 = 463;
	mkQueue.x1 = g_vars->scene29_manX <= 638 ? 351 : 0;
	mkQueue.destZ = 10;
	mkQueue.cycleCount = 1;
	mkQueue.flags = (g_vars->scene29_manX <= 638 ? 2 : 0) | 0x44;
	mkQueue.movementId = MV_MAN29_HIT;

	MessageQueue *mq = g_vars->scene29_aniHandler.makeRunQueue(&mkQueue);
	Command *ex;

	if (mq) {
		if (g_vars->scene29_manX <= 638) {
			ex = new Command(ANI_MAN, 1, MV_MAN29_STANDUP_NORM, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags = 2;
			ex->_param = g_nmi->_aniMan->_copyId;
			mq->addCommandToEnd(ex);

			ex = new Command(0, 17, MSG_SC29_STOPRIDE, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags = 2;
			mq->addCommandToEnd(ex);

			g_vars->scene29_manIsRiding = false;
			g_vars->scene29_arcadeIsOn = false;
			g_vars->scene29_reachedFarRight = false;
			g_vars->scene29_rideBackEnabled = false;
		} else {
			ex = new Command(ANI_MAN, 1, MV_MAN29_STANDUP, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags = 2;
			ex->_param = g_nmi->_aniMan->_copyId;
			mq->addCommandToEnd(ex);
		}

		mq->setFlags(mq->getFlags() | 1);

		if (!mq->run(g_nmi->_aniMan))
			delete mq;
	}
}

void sceneHandler29_assHitRed() {
	if (g_vars->scene29_ass->_statics->_staticsId == ST_ASS_NORM) {
		g_vars->scene29_ass->setStatic(ST_ASS_NORM);
		g_vars->scene29_ass->startMovement(MV_ASS_HITRED, 0, -1);
	}
}

void sceneHandler29_assHitGreen() {
	if (g_vars->scene29_ass->_statics->_staticsId == ST_ASS_NORM) {
		g_vars->scene29_ass->setStatic(ST_ASS_NORM);
		g_vars->scene29_ass->startMovement(MV_ASS_HITGREEN, 0, -1);
	}
}

void sceneHandler29_ballHitCheck() {
	int x, y;

	for (int i = (int)g_vars->scene29_flyingGreenBalls.size() - 1; i >= 0; i--) {
		AniObject *ani = g_vars->scene29_flyingGreenBalls[i];

		x = ani->_x - 30;
		y = ani->_y;

		if (x >= 186) {
			if (sceneHandler29_checkGreenBallHit(ani, x)) {
				g_vars->scene29_greenBalls.push_back(ani);

				g_vars->scene29_flyingGreenBalls.remove_at(i);

				sceneHandler29_manHit();

				g_nmi->playSound(SND_29_014, 0);

				ani->startMovement(MV_SHG_HITMAN, 0, -1);

				g_vars->scene29_hitBall = ani->_id;
			} else {
				ani->setPos(x, y);
			}
		} else {
			g_vars->scene29_greenBalls.push_back(ani);

			ani->hide();

			g_vars->scene29_flyingGreenBalls.remove_at(i);

			sceneHandler29_assHitGreen();
		}
	}

	for (int i = (int)g_vars->scene29_flyingRedBalls.size() - 1; i >= 0; i--) {
		AniObject *ani = g_vars->scene29_flyingRedBalls[i];

		x = ani->_x - 30;
		y = ani->_y;

		if (x >= 147) {
			if (sceneHandler29_checkRedBallHit(ani, x)) {
				g_vars->scene29_redBalls.push_back(ani);

				g_vars->scene29_flyingRedBalls.remove_at(i);

				sceneHandler29_manHit();

				g_nmi->playSound(SND_29_027, 0);

				ani->startMovement(MV_SHR_HITMAN, 0, -1);

				g_vars->scene29_hitBall = ani->_id;
			} else {
				ani->setPos(x, y);
			}
		} else {
			g_vars->scene29_redBalls.push_back(ani);

			ani->hide();

			g_vars->scene29_flyingRedBalls.remove_at(i);

			sceneHandler29_assHitRed();
		}
	}
}

void sceneHandler29_manFromL() {
	debugC(2, kDebugSceneLogic, "scene29: manFromL");

	if (g_vars->scene29_manX < 497 && !g_vars->scene29_scrollingDisabled) {
		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enable();

		g_nmi->_aniMan->setStatic(ST_MAN_RIGHT | 0x4000);
		runQueue(QU_SC29_MANFROM_L, 1);

		g_vars->scene29_scrollingDisabled = true;

		g_nmi->_scrollSpeed = g_vars->scene29_scrollSpeed;
	}
}

void sceneHandler29_manFromR() {
	debugC(2, kDebugSceneLogic, "scene29: manFromR");

	getCurrSceneSc2MotionController()->activate();
	getGameLoaderInteractionController()->enable();

	runQueue(QU_SC29_MANFROM_R, 1);

	g_vars->scene29_arcadeIsOn = false;
	g_vars->scene29_rideBackEnabled = false;
}

int sceneHandler29_updateScreenCallback() {
	int res;

	res = g_nmi->drawArcadeOverlay(g_vars->scene29_arcadeIsOn);

	if (!res)
		g_nmi->_updateScreenCallback = nullptr;

	return res;
}

void sceneHandler29_manToL() {
	debugC(2, kDebugSceneLogic, "scene29: manToL");

	getCurrSceneSc2MotionController()->deactivate();
	getGameLoaderInteractionController()->disable();

	runQueue(QU_SC29_MANTO_L, 1);

	g_vars->scene29_arcadeIsOn = true;

	g_vars->scene29_aniHandler.attachObject(g_nmi->_aniMan->_id);

	g_nmi->_updateScreenCallback = sceneHandler29_updateScreenCallback;

	g_nmi->_msgY = -1;
	g_nmi->_msgX = -1;
}

void sceneHandler29_manToR() {
	debugC(2, kDebugSceneLogic, "scene29: manToR");

	getCurrSceneSc2MotionController()->deactivate();
	getGameLoaderInteractionController()->disable();

	runQueue(QU_SC29_MANTO_R, 1);

	g_vars->scene29_manIsRiding = true;
	g_nmi->_msgY = -1;
	g_nmi->_msgX = -1;

	g_vars->scene29_scrollingDisabled = false;

	g_vars->scene29_scrollSpeed = g_nmi->_scrollSpeed;
	g_nmi->_scrollSpeed = 4;
}

void sceneHandler29_clickPorter(Command *cmd) {
	if (!g_nmi->_aniMan->isIdle() || g_nmi->_aniMan->_flags & 0x100) {
		cmd->_cmd = 0;

		return;
	}

	if (g_vars->scene29_manX <= g_vars->scene29_porter->_x) {
		if (ABS(351 - g_vars->scene29_manX) > 1 || ABS(443 - g_vars->scene29_manY) > 1
			|| g_nmi->_aniMan->_movement || g_nmi->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
			if (g_nmi->_msgX != 351 || g_nmi->_msgY != 443) {
				MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 351, 443, 1, ST_MAN_RIGHT);

				if (mq) {
					mq->addCommandToEnd(cmd->createClone());

					postExCommand(g_nmi->_aniMan->_id, 2, 351, 443, 0, -1);
				}
			}
		} else {
			sceneHandler29_manToL();
		}
	} else {
		g_vars->scene29_manX = g_nmi->_aniMan->_x;
		g_vars->scene29_manY = g_nmi->_aniMan->_y;

		if (ABS(1582 - g_vars->scene29_manX) > 1 || ABS(445 - g_nmi->_aniMan->_y) > 1
			|| g_nmi->_aniMan->_movement || g_nmi->_aniMan->_statics->_staticsId != (0x4000 | ST_MAN_RIGHT)) {
			if (g_nmi->_msgX != 1582 || g_nmi->_msgY != 445) {
				MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, 1582, 445, 1, (0x4000 | ST_MAN_RIGHT));

				if (mq) {
					mq->addCommandToEnd(cmd->createClone());

					postExCommand(g_nmi->_aniMan->_id, 2, 1582, 445, 0, -1);
				}
			}
		} else {
			sceneHandler29_manToR();
		}
	}
}

void sceneHandler29_shootersProcess() {
	if (g_nmi->_aniMan->_statics->_staticsId == ST_MAN29_RUNR) {
		if (g_vars->scene29_manX > 1436) {
			sceneHandler29_manFromR();
		} else {
			g_vars->scene29_shootDistance = (1310 - g_vars->scene29_manX) * 5213 / 100000 + 25;

			if (!g_vars->scene29_manIsHit)
				g_nmi->_aniMan->startMovement(MV_MAN29_RUN, 0, -1);
		}
	}

	g_vars->scene29_manX = g_nmi->_aniMan->_x;
	g_vars->scene29_manY = g_nmi->_aniMan->_y;
}

void sceneHandler29_shootersEscape() {
	if (g_vars->scene29_arcadeIsOn) {
		g_vars->scene29_manX += 2;

		g_nmi->_aniMan->setPos(g_vars->scene29_manX, g_vars->scene29_manY);

		if (g_vars->scene29_manX > 1310 && !g_vars->scene29_shooter1->_movement && !g_vars->scene29_shooter2->_movement
				&& g_vars->scene29_shooter1->_statics->_staticsId == ST_STR1_RIGHT) {
			debugC(2, kDebugSceneLogic, "scene29: shootersEscape");

			g_vars->scene29_shootCountdown = 0;

			g_vars->scene29_shooter1->setStatic(ST_STR1_STAND);
			g_vars->scene29_shooter2->setStatic(ST_STR2_STAND);

			runQueue(QU_SC29_ESCAPE, 1);

			g_vars->scene29_ass->setQueue(nullptr);
			g_vars->scene29_ass->hide();

			g_nmi->setState(sO_LeftPipe_29, g_nmi->getStateValue(sO_LeftPipe_29, sO_IsOpened));
		}
	} else if (g_vars->scene29_manIsRiding) {
		g_vars->scene29_manX -= 4;

		g_nmi->_aniMan->setPos(g_vars->scene29_manX, g_vars->scene29_manY);
	}
}

void sceneHandler29_manRideBack() {
	g_vars->scene29_manX -= 2;

	g_nmi->_aniMan->setPos(g_vars->scene29_manX, g_vars->scene29_manY);
}

void sceneHandler29_shoot() {
	if (g_vars->scene29_arcadeIsOn && g_vars->scene29_manX < 1310) {
		if (g_nmi->_rnd.getRandomNumber(32767) <= 16383|| g_vars->scene29_shooter1->_movement || g_vars->scene29_shooter1->_statics->_staticsId != ST_STR1_RIGHT) {
			if (!g_vars->scene29_shooter2->_movement && g_vars->scene29_shooter2->_statics->_staticsId == ST_STR2_RIGHT) {
				if (g_vars->scene29_shooter2->_flags & 4) {
					g_vars->scene29_shooter2->startMovement(MV_STR2_SHOOT, 0, -1);

					g_vars->scene29_shootCountdown = 0;
				}
			}
		} else {
			g_vars->scene29_shooter1->startMovement(MV_STR1_SHOOT, 0, -1);

			g_vars->scene29_shootCountdown = 0;
		}
	}
}

void sceneHandler29_animBearded() {
	MessageQueue *mq;

	for (uint i = 0; i < g_vars->scene29_bearders.size(); i++) {
		AniObject *ani = g_vars->scene29_bearders[i]->ani;

		if (g_vars->scene29_bearders[i]->wbflag) {
			int x = ani->_x;
			int y = ani->_y;

			if (!ani->_movement && ani->_statics->_staticsId == (ST_BRDCMN_RIGHT | 0x4000)) {
				x -= 4;

				if (x - g_vars->scene29_manX < 100 || !g_vars->scene29_arcadeIsOn) {
					mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC29_BRDOUT1), 0, 1);

					mq->setObjectCopy(-1, ani->_copyId);
					mq->run(nullptr);

					g_vars->scene29_bearders[i]->wbflag = 0;
					g_vars->scene29_bearders[i]->wbcounter = 0;
				}
			}

			if (!ani->_movement && ani->_statics->_staticsId == ST_BRDCMN_GOR)
				ani->startMovement(MV_BRDCMN_GOR, 0, -1);

			if (ani->_movement) {
				if (ani->_movement->_id == MV_BRDCMN_GOR) {
					x -= 4;

					if (g_vars->scene29_manX - x < 60 || x - g_vars->scene29_manX < -260 || !g_vars->scene29_arcadeIsOn) {
						ani->setStatic(ST_BRDCMN_RIGHT);

						mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC29_BRDOUT2), 0, 1);

						mq->setObjectCopy(-1, ani->_copyId);
						mq->run(nullptr);

						g_vars->scene29_bearders[i]->wbflag = 0;
						g_vars->scene29_bearders[i]->wbcounter = 0;
					}
				}
			}

			ani->setPos(x, y);
			continue;
		}

		if (g_vars->scene29_arcadeIsOn && g_vars->scene29_bearders[i]->wbcounter > 30) {
			int newx;

			if (g_nmi->_rnd.getRandomNumber(1))
				goto dostuff;

			if (g_vars->scene29_manX <= 700) {
				g_vars->scene29_bearders[i]->wbcounter++;
				continue;
			}

			if (g_vars->scene29_manX >= 1100) {
			dostuff:
				if (g_vars->scene29_manX <= 700 || g_vars->scene29_manX >= 1350) {
					g_vars->scene29_bearders[i]->wbcounter++;
					continue;
				}

				mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC29_BRD2), 0, 1);

				newx = g_vars->scene29_manX - 200;
			} else {
				mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(QU_SC29_BRD1), 0, 1);

				newx = g_vars->scene29_manX + 350;
			}

			mq->getCommandByIndex(0)->_x = newx;
			mq->setObjectCopy(-1, ani->_copyId);
			mq->run(nullptr);

			g_vars->scene29_bearders[i]->wbflag = 1;
			g_vars->scene29_bearders[i]->wbcounter = 0;
		}

		g_vars->scene29_bearders[i]->wbcounter++;
	}
}



int sceneHandler29(Command *cmd) {
	if (cmd->_cmd != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler29_winArcade();
		break;

	case MSG_SC29_LAUGH:
		if (g_vars->scene29_hitBall == ANI_SHELL_GREEN) {
			g_nmi->playSound(SND_29_028, 0);
		} else {
			g_nmi->playSound(SND_29_029, 0);
		}
		break;

	case MSG_SC29_SHOWLASTRED:
		if (g_vars->scene29_redBalls.size()) { // original checks size of the scene29_greenBalls which looks like a copy/paste error
			debugC(2, kDebugSceneLogic, "scene29: showLastRed");

			g_vars->scene29_redBalls.back()->show1(-1, -1, -1, 0);
			g_vars->scene29_redBalls.back()->startMovement(MV_SHR_HITASS, 0, -1);
		}

		break;

	case MSG_SC29_SHOOTGREEN:
		sceneHandler29_shootGreen();
		break;

	case MSG_SC29_SHOOTRED:
		sceneHandler29_shootRed();
		break;

	case MSG_SC29_SHOWLASTGREEN:
		if (g_vars->scene29_greenBalls.size()) {
			debugC(2, kDebugSceneLogic, "scene29: showLastGreen");

			g_vars->scene29_greenBalls.back()->show1(-1, -1, -1, 0);
			g_vars->scene29_greenBalls.back()->startMovement(MV_SHG_HITASS, 0, -1);
		}
		break;

	case MSG_SC29_STOPRIDE:
		g_vars->scene29_manIsRiding = false;
		g_vars->scene29_arcadeIsOn = false;
		g_vars->scene29_reachedFarRight = false;
		g_vars->scene29_rideBackEnabled = false;

		debugC(2, kDebugSceneLogic, "scene29: stopRide");

		getCurrSceneSc2MotionController()->activate();
		getGameLoaderInteractionController()->enable();
		break;

	case MSG_SC29_DISABLERIDEBACK:
		g_vars->scene29_rideBackEnabled = false;
		break;

	case MSG_SC29_ENABLERIDEBACK:
		g_vars->scene29_rideBackEnabled = true;
		g_vars->scene29_reachedFarRight = false;
		break;

	case MSG_SC29_DISABLEPORTER:
		g_vars->scene29_reachedFarRight = false;
		break;

	case MSG_SC29_ENABLEPORTER:
		g_vars->scene29_reachedFarRight = true;
		g_vars->scene29_rideBackEnabled = false;
		g_vars->scene29_manIsHit = false;
		break;

	case 29:
		if (!g_vars->scene29_manIsRiding || g_vars->scene29_arcadeIsOn) {
			if (!g_vars->scene29_arcadeIsOn) {
				AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

				if (ani && ani == g_vars->scene29_porter) {
					sceneHandler29_clickPorter(cmd);

					cmd->_cmd = 0;
					break;
				}
			} else {
				sceneHandler29_manJump();
				cmd->_cmd = 0;
			}
		}
		break;

	case 107:
		if (g_vars->scene29_arcadeIsOn) {
			sceneHandler29_manBend();
		}
		break;

	case 33:
		if (g_vars->scene29_arcadeIsOn) {
			if (g_vars->scene29_manX > g_nmi->_sceneRect.right - 500)
				g_nmi->_currentScene->_x = g_nmi->_sceneRect.right - g_vars->scene29_manX - 350;

			if (g_vars->scene29_manX < g_nmi->_sceneRect.left + 100)
				g_nmi->_currentScene->_x = g_vars->scene29_manX - g_nmi->_sceneRect.left - 100;

		} else if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_x;

			if (x < g_nmi->_sceneRect.left + 300)
				g_nmi->_currentScene->_x = x - 400 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 300)
				g_nmi->_currentScene->_x = x + 400 - g_nmi->_sceneRect.right;
		}

		g_vars->scene29_manX = g_nmi->_aniMan->_x;
		g_vars->scene29_manY = g_nmi->_aniMan->_y;

		sceneHandler29_ballHitCheck();

		if (!g_vars->scene29_porter->_movement)
			g_vars->scene29_porter->startMovement(MV_PTR_MOVEFAST, 0, -1);

		if (g_vars->scene29_manIsRiding)
			sceneHandler29_manFromL();
		else if (g_vars->scene29_arcadeIsOn && !g_nmi->_aniMan->_movement)
			sceneHandler29_shootersProcess();

		if (g_vars->scene29_reachedFarRight)
			sceneHandler29_shootersEscape();
		else if (g_vars->scene29_rideBackEnabled)
			sceneHandler29_manRideBack();

		g_vars->scene29_shootCountdown++;

		if (g_vars->scene29_shootCountdown > g_vars->scene29_shootDistance)
			sceneHandler29_shoot();

		sceneHandler29_animBearded();

		g_nmi->_behaviorManager->update();
		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

int scene29_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_vars->scene29_arcadeIsOn) {
		if (g_nmi->_cursorId != PIC_CSR_DEFAULT_INV && g_nmi->_cursorId != PIC_CSR_ITN_INV)
			g_nmi->_cursorId = -1;
	} else if (g_vars->scene29_manIsRiding) {
		if (g_nmi->_cursorId != PIC_CSR_DEFAULT_INV && g_nmi->_cursorId != PIC_CSR_ITN_INV)
			g_nmi->_cursorId = PIC_CSR_DEFAULT;
	} else if (g_nmi->_objectIdAtCursor == ANI_PORTER) {
		if (g_nmi->_cursorId == PIC_CSR_DEFAULT)
			g_nmi->_cursorId = PIC_CSR_ITN;
	} else {
		if (g_nmi->_objectIdAtCursor == PIC_SC29_LTRUBA && g_nmi->_cursorId == PIC_CSR_ITN)
			g_nmi->_cursorId = PIC_CSR_GOL;
	}
	return g_nmi->_cursorId;
}

} // End of namespace NGI
