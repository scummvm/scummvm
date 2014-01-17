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
#include "fullpipe/input.h"

namespace Fullpipe {

void scene14_initScene(Scene *sc) {
	g_vars->scene14_grandma = sc->getStaticANIObject1ById(ANI_GRANDMA, -1);
	g_vars->scene14_sceneDeltaX = 200;
	g_vars->scene14_sceneDeltaY = 200;
	g_vars->scene14_arcadeIsOn = false;
	g_vars->scene14_dudeIsKicking = false;
	g_vars->scene14_ballIsFlying = false;
	g_vars->scene14_dudeCanKick = false;
	g_vars->scene14_sceneDiffX = 300;
	g_vars->scene14_sceneDiffY = 300;
	g_vars->scene14_pink = 0;
	g_vars->scene14_flyingBall = 0;
	g_vars->scene14_balls.clear();

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_14)) {
		g_vars->scene14_grandmaIsHere = true;

		StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_BALL14, -1);

		ball->_flags &= 0xFFFB;
		g_vars->scene14_balls.push_back(ball);
		
		for (uint i = 0; i < 3; i++) {
			ball = new StaticANIObject(ball); // create a copy

			ball->_flags &= 0xFFFB;
			g_vars->scene14_balls.push_back(ball);

			sc->addStaticANIObject(ball, 1);
		}
	} else {
		g_vars->scene14_grandmaIsHere = false;
		g_vars->scene14_grandma->hide();
	}

	g_fp->lift_setButton(sO_Level4, ST_LBN_4N);
	g_fp->lift_sub5(sc, QU_SC14_ENTERLIFT, QU_SC14_EXITLIFT);

	g_fp->initArcadeKeys("SC_14");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE6);
}

void scene14_setupMusic() {
	if (!g_vars->scene14_grandmaIsHere)
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_14"), "MUSIC2", 0);
}

int scene14_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene14_arcadeIsOn) {
		if (g_vars->scene14_dudeIsKicking) {
			g_fp->_cursorId = PIC_CSR_ARCADE2_D;
		} else {
			if (g_fp->_aniMan != g_fp->_objectAtCursor || g_fp->_aniMan->_movement || g_fp->_cursorId != PIC_CSR_DEFAULT) {
				if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV) {
					g_fp->_cursorId = PIC_CSR_DEFAULT;
				}
			} else {
				g_fp->_cursorId = PIC_CSR_ITN;
			}
		}
	}

	return g_fp->_cursorId;
}

int sceneHandler14_updateScreenCallback() {
	int res;

	res = g_fp->drawArcadeOverlay(g_vars->scene14_arcadeIsOn);
	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

void sceneHandler14_showBallGrandmaHit2() {
	if (g_vars->scene14_flyingBall) {
		g_vars->scene14_flyingBall->show1(g_vars->scene14_grandmaX + 223, g_vars->scene14_grandmaY + 35, MV_BAL14_SPIN, 0);
		g_vars->scene14_flyingBall->_priority = 27;

		g_vars->scene14_pink = g_vars->scene14_flyingBall;

		g_vars->scene14_flyingBall = 0;
	}
}

void sceneHandler14_showBallGrandmaDive() {
	if (g_vars->scene14_flyingBall) {
		g_vars->scene14_flyingBall->show1(g_vars->scene14_grandmaX + 506, g_vars->scene14_grandmaY - 29, -1, 0);

		g_vars->scene14_balls.push_back(g_vars->scene14_flyingBall);
		g_vars->scene14_flyingBall = 0;
	}

	g_fp->_aniMan2 = g_fp->_aniMan;
}

void sceneHandler14_showBallGrandmaHit() {
	if (g_vars->scene14_flyingBall) {
		g_vars->scene14_flyingBall->show1(g_vars->scene14_grandmaX + 190, g_vars->scene14_grandmaY + 56, MV_BAL14_TOGMA, 0);
		g_vars->scene14_flyingBall->_priority = 27;

		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
		ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_TOGMA, 0, 0, 0, 1, 0, 0, 0);

		ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
		ex->_excFlags |= 2;
		ex->_field_24 = 1;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);
		mq->chain(0);

		g_vars->scene14_balls.push_back(g_vars->scene14_flyingBall);
		g_vars->scene14_flyingBall = 0;
	}
}

void sceneHandler14_exitScene() {
	g_vars->scene14_arcadeIsOn = false;

	if (g_fp->_aniMan->_movement)
		g_fp->_aniMan->_movement->gotoLastFrame();

	g_fp->_aniMan->stopAnim_maybe();

	handleObjectInteraction(g_fp->_aniMan, g_fp->_currentScene->getPictureObjectById(PIC_SC14_RTRUBA, 0), 0);

	g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);

	chainQueue(QU_SC14_ENDARCADE, 0);

	getGameLoaderInteractionController()->disableFlag24();
	getCurrSceneSc2MotionController()->clearEnabled();
}

void sceneHandler14_showBallMan() {
	if (g_vars->scene14_flyingBall) {
		g_vars->scene14_flyingBall->show1(g_vars->scene14_dudeX - 166, g_vars->scene14_dudeY + 40, MV_BAL14_TOGMA, 0);
		g_vars->scene14_flyingBall->_priority = 27;

		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
		ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_TOGMA, 0, 0, 0, 1, 0, 0, 0);

		ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
		ex->_excFlags |= 2;
		ex->_field_24 = 1;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);
		mq->chain(0);

		g_vars->scene14_flyingBall->startAnim(MV_BAL14_TOGMA, 0, -1);

		g_vars->scene14_balls.push_back(g_vars->scene14_flyingBall);
		g_vars->scene14_flyingBall = 0;

		if (g_vars->scene14_dudeX >= 1300)
			sceneHandler14_exitScene();
	}
}

void sceneHandler14_manKickBall() {
	int val = (g_vars->scene14_grandmaX + 65 - (g_vars->scene14_dudeX - 85)) / -32;
	int den = val;

	g_vars->scene14_ballX = g_vars->scene14_dudeX - 85;
	g_vars->scene14_ballY = g_vars->scene14_dudeY - 76;
	g_vars->scene14_ballDeltaX = -32;

	if (!val)
		den = 1;

	g_vars->scene14_ballDeltaY = (g_vars->scene14_grandmaY - 102 - val * val / 2 - (g_vars->scene14_dudeY - 76)) / den;

	g_vars->scene14_flyingBall->show1(g_vars->scene14_dudeX - 85, g_vars->scene14_dudeY - 76, -1, 0);
	g_vars->scene14_flyingBall->startAnim(MV_BAL14_SPIN, 0, -1);
	g_vars->scene14_flyingBall->_priority = 5;

	g_vars->scene14_ballIsFlying = true;
}

void sceneHandler14_showBallFly() {
	if (g_vars->scene14_balls.size()) {
		g_vars->scene14_flyingBall = g_vars->scene14_balls.front();
		g_vars->scene14_balls.pop_front();
	}

	int x, y;

	if (g_vars->scene14_grandma->_movement) {
		x = g_vars->scene14_grandma->_movement->_ox;
		g_vars->scene14_ballX = x;
		y = g_vars->scene14_grandma->_movement->_oy;
	} else {
		x = g_vars->scene14_grandmaX;
		y = g_vars->scene14_grandmaY;
	}

	x = x + 38;
	y = y - 77;

	g_vars->scene14_ballDeltaX = 32;

	int dist = (g_vars->scene14_dudeX - 16 - x) / 32;
	int den = dist;

	if (!dist)
		den = 1;

	g_vars->scene14_ballX = x + 32;
	g_vars->scene14_ballDeltaY = (g_vars->scene14_dudeY - 40 - dist * dist / 2 - y) / den;
	g_vars->scene14_ballY = g_vars->scene14_ballDeltaY + y;

	g_vars->scene14_flyingBall->show1(x + 32, g_vars->scene14_ballDeltaY + y, MV_BAL14_SPIN, 0);
	g_vars->scene14_flyingBall->_priority = 5;
	g_vars->scene14_flyingBall->startAnim(MV_BAL14_SPIN, 0, -1);

	g_vars->scene14_ballIsFlying = true;
}

void sceneHandler14_grandmaJump() {
	BehaviorEntryInfo *beh1 = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_JUMPFW);
	BehaviorEntryInfo *beh2 = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_JUMPBK);

	if (beh1) {
		if (beh2) {
			int p = beh1->_percent;
			beh1->_percent = beh2->_percent;
			beh2->_percent = p;
		}
	}
}

void sceneHandler14_endArcade() {
	g_vars->scene14_arcadeIsOn = false;

	setInputDisabled(0);

	getGameLoaderInteractionController()->enableFlag24();
	getCurrSceneSc2MotionController()->setEnabled();

	BehaviorEntryInfo *beh = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_BLINK);
	if (beh)
		beh->_percent = 327;

	beh = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_THROW);
	if (beh)
		beh->_percent = 0;

	g_vars->scene14_sceneDeltaX = 200;
	g_vars->scene14_sceneDeltaY = 200;

	g_fp->_aniMan2 = g_fp->_aniMan;

	g_vars->scene14_sceneDiffX = 300;
	g_vars->scene14_sceneDiffY = 300;
}

void sceneHandler14_winArcade() {
	if (g_vars->scene14_arcadeIsOn) {
		if (g_vars->scene14_dudeIsKicking) {
			g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);

			g_vars->scene14_dudeIsKicking = false;
		}

		if (g_vars->scene14_flyingBall) {
			g_vars->scene14_balls.push_back(g_vars->scene14_flyingBall);

			g_vars->scene14_flyingBall->_flags &= 0xFFFB;
			g_vars->scene14_flyingBall = 0;
		}

		g_vars->scene14_ballIsFlying = false;

		sceneHandler14_endArcade();

		g_vars->scene14_grandmaIsHere = false;

		if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_14)) {
			g_fp->setObjectState(sO_Grandma, g_fp->getObjectEnumState(sO_Grandma, sO_In_15));
			g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);
			g_vars->scene14_grandma->_flags &= 0xFFFB;
		}

		if (g_fp->_currentScene->_messageQueueId) {
			MessageQueue *mq = g_fp->_globalMessageQueueList->getMessageQueueById(g_fp->_currentScene->_messageQueueId);
			if (mq)
				delete mq;

			g_fp->_currentScene->_messageQueueId = 0;
		}
	}
}

void sceneHandler14_showBallLast() {
	if (g_vars->scene14_pink) {
		g_vars->scene14_pink->show1(693, 491, MV_BAL14_SPIN, 0);
		g_vars->scene14_pink->_priority = 27;
	}
}

void sceneHandler14_hideBallLast() {
	if (g_vars->scene14_pink) {
		g_vars->scene14_pink->hide();
		g_vars->scene14_balls.push_back(g_vars->scene14_pink);
		g_vars->scene14_pink = 0;
	}
}

void sceneHandler14_startArcade() {
	g_vars->scene14_arcadeIsOn = true;
	g_vars->scene14_dudeCanKick = true;

	if (g_fp->_aniMan->_movement) {
		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);
		g_fp->_aniMan->setOXY(1237, 451);
		g_fp->_aniMan->_priority = 25;
	}

	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_aniMan2 = 0;
	g_vars->scene14_sceneDeltaX = 50;
	g_vars->scene14_sceneDiffX = 100;
	g_vars->scene14_hitsLeft = 4;
	g_vars->scene14_pink = 0;

	chainQueue(QU_SC14_STARTARCADE, 0);

	g_fp->_updateScreenCallback = sceneHandler14_updateScreenCallback;
}

void sceneHandler14_clearCallback() {
	g_fp->_aniMan->_callback2 = 0;
	g_vars->scene14_dudeIsKicking = false;
}

void sceneHandler14_kickAnimation() {
	if (g_fp->_aniMan->_movement) {
		sceneHandler14_clearCallback();

		if (g_vars->scene14_flyingBall && g_vars->scene14_dudeX - g_vars->scene14_flyingBall->_ox < 180) {
			g_fp->_aniMan->changeStatics2(g_fp->_aniMan->_movement->_staticsObj2->_staticsId);
			g_fp->_aniMan->startAnim(MV_MAN14_KICK, 0, -1);

			g_vars->scene14_ballIsFlying = false;

			g_vars->scene14_flyingBall->stopAnim_maybe();
			g_vars->scene14_flyingBall->hide();
		} else {
			g_fp->_aniMan->changeStatics2(g_fp->_aniMan->_movement->_staticsObj2->_staticsId);
			g_fp->_aniMan->startAnim(MV_MAN14_KICKAIR, 0, -1);
		}
	}
}

void sceneHandler14_declineCallback(int *arg) {
	Common::Point point;

	if (g_vars->scene14_dudeIsKicking) {
		*arg = (int)(sqrt((double)(g_fp->_mouseVirtY - g_vars->scene14_mouseCursorPos.y)
							 * (g_fp->_mouseVirtY - g_vars->scene14_mouseCursorPos.y)
					+ (g_fp->_mouseVirtX - g_vars->scene14_mouseCursorPos.x)
							 * (g_fp->_mouseVirtX - g_vars->scene14_mouseCursorPos.x)) * 0.1);

		if (*arg > 11)
			*arg = 11;
	} else {
		++*arg;
	}
}

void sceneHandler14_dudeDecline() {
	g_vars->scene14_mouseCursorPos.x = g_fp->_mouseVirtX;
	g_vars->scene14_mouseCursorPos.y = g_fp->_mouseVirtY;

	g_fp->_aniMan->_callback2 = sceneHandler14_declineCallback;
	g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);
	g_fp->_aniMan->startAnim(MV_MAN14_DECLINE, 0, -1);

	g_vars->scene14_dudeIsKicking = true;
}

bool sceneHandler14_arcadeProcessClick(ExCommand *cmd) {
	if (!getCurrSceneSc2MotionController()->_isEnabled)
		return 0;

	if (!g_vars->scene14_grandmaIsHere) {
		if (!cmd->_keyCode) {
			if (g_vars->scene14_pink) {
				if (g_vars->scene14_pink->_flags & 4) {
					if (cmd->_sceneClickX < g_vars->scene14_pink->_ox + 40) {
						handleObjectInteraction(g_fp->_aniMan, g_vars->scene14_pink, 0);
						cmd->_messageKind = 0;
						return true;
					}
				}
			}
		}
		return false;
	}

	if (getCurrSceneSc2MotionController()->_objtype != kObjTypeMctlCompound)
		return false;

	if (!getCurrSceneSc2MotionController()->_motionControllers[0]->_movGraphReactObj->pointInRegion(cmd->_sceneClickX, cmd->_sceneClickY))
		return false;

	if (cmd->_sceneClickX > 1237)
		return false;

	MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, 1237, 451, 1, 0);

	if (!mq)
		return false;

	ExCommand *ex = new ExCommand(0, 17, MSG_SC14_STARTARCADE, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);
	mq->setFlags(mq->getFlags() | 1);

	postExCommand(g_fp->_aniMan->_id, 2, 1237, 451, 0, -1);

	cmd->_messageKind = 0;

	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();
	return true;
}

void sceneHandler14_grandmaThrow() {
	g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);

	MessageQueue *mq = new MessageQueue;
	ExCommand *ex = new ExCommand(ANI_GRANDMA, 2, 30, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_THROW, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	mq->chain(0);
}

void sceneHandler14_passToGrandma() {
	g_vars->scene14_flyingBall->stopAnim_maybe();
	g_vars->scene14_flyingBall->_priority = 27;

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
	ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_FALL, 0, 0, 0, 1, 0, 0, 0);

	ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
	ex->_excFlags |= 2;
	ex->_field_24 = 1;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = g_vars->scene14_flyingBall->_okeyCode;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);
	mq->chain(0);

	g_vars->scene14_balls.push_back(g_vars->scene14_flyingBall);
	g_vars->scene14_flyingBall = 0;

	sceneHandler14_grandmaThrow();
}

void sceneHandler14_grandmaJumpThrow() {
	g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);

	MessageQueue *mq = new MessageQueue;
	ExCommand *ex = new ExCommand(ANI_GRANDMA, 2, 30, 0, 0, 0, 1, 0, 0, 0);

	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_JUMPFW, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_THROW, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags |= 2;
	mq->addExCommandToEnd(ex);

	mq->chain(0);

	g_vars->scene14_dude2X += 71;
	g_fp->_currentScene->_x = 71;

	g_fp->_aniMan2 = g_fp->_aniMan;
}

void sceneHandler14_dudeFall() {
	if (!g_fp->_aniMan->_movement || g_fp->_aniMan->_movement->_id != MV_MAN14_FALL) {
		sceneHandler14_clearCallback();

		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);
		g_fp->_aniMan->startAnim(MV_MAN14_FALL, 0, -1);
		g_vars->scene14_flyingBall->stopAnim_maybe();
		g_vars->scene14_flyingBall->hide();

		sceneHandler14_grandmaJumpThrow();
	}
	++g_vars->scene14_hitsLeft;
}

void sceneHandler14_grandmaStepForward() {
	g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);
	g_fp->_aniMan->startAnim(MV_MAN14_STEPFW, 0, -1);

	g_vars->scene14_dude2X -= 71;

	g_fp->_currentScene->_x = -71;
	g_fp->_aniMan2 = g_vars->scene14_grandma;
}

void sceneHandler14_arcadeLogic() {
	g_vars->scene14_flyingBall->stopAnim_maybe();
	g_vars->scene14_flyingBall->hide();

	if (g_vars->scene14_dudeIsKicking)
		sceneHandler14_clearCallback();

	if (g_vars->scene14_hitsLeft <= 1) {
		setInputDisabled(1);

		sceneHandler14_clearCallback();

		g_vars->scene14_dudeCanKick = false;
		g_fp->_aniMan2 = 0;

		chainQueue(QU_SC14_WINARCADE, 1);

		--g_vars->scene14_hitsLeft;
	} else {
		ExCommand *ex;

		g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);

		if (g_vars->scene14_hitsLeft != 3 || g_vars->scene14_pink) {
			MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

			ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_BACKOFF, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_THROW, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);

			mq->chain(0);
		} else {
			MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

			ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_BACKOFF2, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(ANI_GRANDMA, 1, MV_GMA_THROW, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);

			mq->chain(0);
		}

		sceneHandler14_grandmaStepForward();
		--g_vars->scene14_hitsLeft;
	}
}

void sceneHandler14_animateBall() {
	int x = g_vars->scene14_ballDeltaX + g_vars->scene14_ballX;
	int y = g_vars->scene14_ballDeltaY + g_vars->scene14_ballY;

	g_vars->scene14_ballX += g_vars->scene14_ballDeltaX;
	g_vars->scene14_ballY += g_vars->scene14_ballDeltaY;

	g_vars->scene14_ballDeltaY++;

	if (g_vars->scene14_ballDeltaY - 1 + g_vars->scene14_ballY > 517) {
		if (x <= g_vars->scene14_dudeX - 16 ) {
			if ( g_vars->scene14_ballDeltaX >= 0 || x >= g_vars->scene14_grandmaX + 65 || x <= g_vars->scene14_grandmaX - 135 || y <= g_vars->scene14_grandmaY - 102 ) {
				if (g_vars->scene14_flyingBall->_movement)
					g_vars->scene14_flyingBall->_movement->setOXY(x, y);
				else
					g_vars->scene14_flyingBall->setOXY(x, y);
			} else {
				sceneHandler14_arcadeLogic();
				g_vars->scene14_ballIsFlying = false;
			}
		} else {
			sceneHandler14_dudeFall();
			g_vars->scene14_ballIsFlying = false;
		}
	} else {
		sceneHandler14_passToGrandma();
		g_vars->scene14_ballIsFlying = false;
	}
}

int sceneHandler14(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch(cmd->_messageNum) {
	case MSG_SC14_SHOWBALLGMAHIT2:
		sceneHandler14_showBallGrandmaHit2();
		break;

	case MSG_SC14_SHOWBALLGMADIVE:
		sceneHandler14_showBallGrandmaDive();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_animation3();
		break;

	case MSG_SC14_SHOWBALLGMAHIT:
		sceneHandler14_showBallGrandmaHit();
		break;

	case MSG_SC14_SHOWBALLMAN:
		sceneHandler14_showBallMan();
		break;

	case MSG_SC14_MANKICKBALL:
		sceneHandler14_manKickBall();
		break;

	case MSG_SC14_SHOWBALLFLY:
		sceneHandler14_showBallFly();
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC14_GMAJUMP:
		sceneHandler14_grandmaJump();
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_SC14_RESTORESCROLL:
		g_fp->_aniMan2 = g_fp->_aniMan;
		g_fp->_scrollSpeed = 8;
		break;

	case MSG_CMN_WINARCADE:
		sceneHandler14_winArcade();
		break;

	case MSG_SC14_SCROLLLEFT:
		g_fp->_aniMan2 = 0;
		g_fp->_currentScene->_x = -g_fp->_sceneRect.left;
		g_fp->_scrollSpeed = 24;
		break;

	case MSG_SC14_SHOWBALLLAST:
		sceneHandler14_showBallLast();
		break;

	case MSG_SC14_HIDEBALLLAST:
		sceneHandler14_hideBallLast();
		break;

	case MSG_SC14_HIDEPINK:
		if (!g_vars->scene14_pink)
			break;

		g_vars->scene14_pink->hide();
		break;

	case MSG_SC14_GMATOTRUBA:
		g_fp->_currentScene->_x = -g_fp->_sceneRect.left;
		break;

	case MSG_SC14_STARTARCADE:
		sceneHandler14_startArcade();
		break;

	case MSG_SC14_ENDARCADE:
		sceneHandler14_endArcade();

		g_vars->scene14_grandmaIsHere = false;
		break;

	case 64:
		g_fp->lift_sub05(cmd);
		break;

	case 33:
		{
			Movement *mov = g_fp->_aniMan->_movement;

			if (mov) {
				g_vars->scene14_dudeX = mov->_ox;
				g_vars->scene14_dudeY = mov->_oy;

				if (mov->_id == MV_MAN14_KICK)
					g_vars->scene14_dudeX = mov->_ox + 2 * g_fp->_aniMan->_movement->_currDynamicPhaseIndex;
			} else {
				g_vars->scene14_dudeX = g_fp->_aniMan->_ox;
				g_vars->scene14_dudeY = g_fp->_aniMan->_oy;
			}

			mov = g_vars->scene14_grandma->_movement;
			if (mov) {
				g_vars->scene14_grandmaX = mov->_ox;
				g_vars->scene14_grandmaY = mov->_oy;
			} else {
				g_vars->scene14_grandmaX = g_vars->scene14_grandma->_ox;
				g_vars->scene14_grandmaY = g_vars->scene14_grandma->_oy;
			}

			if (g_fp->_aniMan2) {
				int x = g_fp->_aniMan2->_ox;
				g_vars->scene14_dude2X = x;

				if (x < g_fp->_sceneRect.left + g_vars->scene14_sceneDeltaX) {
					g_fp->_currentScene->_x = x - g_vars->scene14_sceneDiffX - g_fp->_sceneRect.left;
					x = g_vars->scene14_dude2X;
				}

				if (x > g_fp->_sceneRect.right - g_vars->scene14_sceneDeltaX)
					g_fp->_currentScene->_x = x + g_vars->scene14_sceneDiffX - g_fp->_sceneRect.right;
			}

			if (g_vars->scene14_ballIsFlying)
				sceneHandler14_animateBall();

			g_fp->_behaviorManager->updateBehaviors();
			g_fp->startSceneTrack();
			break;
		}

	case 30:
		if (g_vars->scene14_dudeIsKicking) {
			sceneHandler14_kickAnimation();
			break;
		}

		if (!g_vars->scene14_arcadeIsOn) {
			break;
		}
		break;

	case 29:
        if (g_vars->scene14_arcadeIsOn) {
			int pixel;

			if (g_vars->scene14_dudeCanKick && g_fp->_aniMan->getPixelAtPos(cmd->_sceneClickX, cmd->_sceneClickY, &pixel) && !g_fp->_aniMan->_movement) {
				sceneHandler14_dudeDecline();
				break;
			}
        } else {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fp->lift_sub1(ani);
				cmd->_messageKind = 0;
				break;
			}

			if (!sceneHandler14_arcadeProcessClick(cmd) && (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode))) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						sceneHandler14_arcadeProcessClick(cmd);
						break;
					}
				}
			}
        }
		break;
    }

	return 0;
}

} // End of namespace Fullpipe
