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
	g_vars->scene14_var01 = 200;
	g_vars->scene14_var02 = 200;
	g_vars->scene14_var03 = 0;
	g_vars->scene14_var04 = 0;
	g_vars->scene14_var05 = 0;
	g_vars->scene14_var06 = 0;
	g_vars->scene14_var07 = 300;
	g_vars->scene14_var08 = 300;
	g_vars->scene14_pink = 0;
	g_vars->scene14_var10 = 0;
	g_vars->scene14_var11.clear();
	g_vars->scene14_var12.clear();

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_In_14)) {
		g_vars->scene14_var13 = 1;

		StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_BALL14, -1);

		ball->_flags &= 0xFFFB;
		g_vars->scene14_var11.push_back(ball);
		
		for (uint i = 0; i < 3; i++) {
			ball = new StaticANIObject(ball); // create a copy

			ball->_flags &= 0xFFFB;
			g_vars->scene14_var11.push_back(ball);

			sc->addStaticANIObject(ball, 1);
		}
	} else {
		g_vars->scene14_var13 = 0;
		g_vars->scene14_grandma->hide();
	}

	g_fp->lift_setButton(sO_Level4, ST_LBN_4N);
	g_fp->lift_sub5(sc, QU_SC14_ENTERLIFT, QU_SC14_EXITLIFT);

	g_fp->initArcadeKeys("SC_14");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE6);
}

void scene14_setupMusic() {
	if (!g_vars->scene14_var13)
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_14"), "MUSIC2", 0);
}

int scene14_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene14_var03) {
		if (g_vars->scene14_var04) {
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

	res = g_fp->drawArcadeOverlay(g_vars->scene14_var03);
	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

void sceneHandler14_showBallGrandmaHit2() {
	if (g_vars->scene14_var10) {
		g_vars->scene14_var10->show1(g_vars->scene14_var16 + 223, g_vars->scene14_var17 + 35, MV_BAL14_SPIN, 0);
		g_vars->scene14_var10->_priority = 27;

		g_vars->scene14_pink = g_vars->scene14_var10;

		g_vars->scene14_var10 = 0;
	}
}

void sceneHandler14_showBallGrandmaDive() {
	if (g_vars->scene14_var10) {
		g_vars->scene14_var10->show1(g_vars->scene14_var16 + 506, g_vars->scene14_var17 - 29, -1, 0);

		g_vars->scene14_var11.push_back(g_vars->scene14_var10);
		g_vars->scene14_var10 = 0;
	}

	g_fp->_aniMan2 = g_fp->_aniMan;
}

void sceneHandler14_showBallGrandmaHit() {
	if (g_vars->scene14_var10) {
		g_vars->scene14_var10->show1(g_vars->scene14_var16 + 190, g_vars->scene14_var17 + 56, MV_BAL14_TOGMA, 0);
		g_vars->scene14_var10->_priority = 27;

		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
		ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_TOGMA, 0, 0, 0, 1, 0, 0, 0);

		ex->_keyCode = g_vars->scene14_var10->_okeyCode;
		ex->_excFlags |= 2;
		ex->_field_24 = 1;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = g_vars->scene14_var10->_okeyCode;
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);
		mq->chain(0);

		g_vars->scene14_var11.push_back(g_vars->scene14_var10);
		g_vars->scene14_var10 = 0;
	}
}

void sceneHandler14_exitScene() {
	g_vars->scene14_var03 = 0;

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
	if (g_vars->scene14_var10) {
		g_vars->scene14_var10->show1(g_vars->scene14_var14 - 166, g_vars->scene14_var15 + 40, MV_BAL14_TOGMA, 0);
		g_vars->scene14_var10->_priority = 27;

		MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
		ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_TOGMA, 0, 0, 0, 1, 0, 0, 0);

		ex->_keyCode = g_vars->scene14_var10->_okeyCode;
		ex->_excFlags |= 2;
		ex->_field_24 = 1;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = g_vars->scene14_var10->_okeyCode;
		ex->_excFlags |= 3;
		mq->addExCommandToEnd(ex);
		mq->chain(0);

		g_vars->scene14_var10->startAnim(MV_BAL14_TOGMA, 0, -1);

		g_vars->scene14_var11.push_back(g_vars->scene14_var10);
		g_vars->scene14_var10 = 0;

		if (g_vars->scene14_var14 >= 1300)
			sceneHandler14_exitScene();
	}
}

void sceneHandler14_manKickBall() {
	int val = (g_vars->scene14_var16 + 65 - (g_vars->scene14_var14 - 85)) / -32;
	int den = val;

	g_vars->scene14_var22 = g_vars->scene14_var14 - 85;
	g_vars->scene14_var23 = g_vars->scene14_var15 - 76;
	g_vars->scene14_var20 = -32;

	if (!val)
		den = 1;

	g_vars->scene14_var21 = (g_vars->scene14_var17 - 102 - val * val / 2 - (g_vars->scene14_var15 - 76)) / den;

	g_vars->scene14_var10->show1(g_vars->scene14_var14 - 85, g_vars->scene14_var15 - 76, -1, 0);
	g_vars->scene14_var10->startAnim(MV_BAL14_SPIN, 0, -1);
	g_vars->scene14_var10->_priority = 5;

	g_vars->scene14_var05 = 1;
}

void sceneHandler14_showBallFly() {
	if (g_vars->scene14_var11.size()) {
		g_vars->scene14_var10 = g_vars->scene14_var11.front();
		g_vars->scene14_var11.pop_front();
	}

	int x, y;

	if (g_vars->scene14_grandma->_movement) {
		x = g_vars->scene14_grandma->_movement->_ox;
		g_vars->scene14_var22 = x;
		y = g_vars->scene14_grandma->_movement->_oy;
	} else {
		x = g_vars->scene14_var16;
		y = g_vars->scene14_var17;
	}

	x = x + 38;
	y = y - 77;

	g_vars->scene14_var20 = 32;

	int dist = (g_vars->scene14_var14 - 16 - x) / 32;
	int den = dist;

	if (!dist)
		den = 1;

	g_vars->scene14_var22 = x + 32;
	g_vars->scene14_var21 = (g_vars->scene14_var15 - 40 - dist * dist / 2 - y) / den;
	g_vars->scene14_var23 = g_vars->scene14_var21 + y;

	g_vars->scene14_var10->show1(x + 32, g_vars->scene14_var21 + y, MV_BAL14_SPIN, 0);
	g_vars->scene14_var10->_priority = 5;
	g_vars->scene14_var10->startAnim(MV_BAL14_SPIN, 0, -1);

	g_vars->scene14_var05 = 1;
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
	g_vars->scene14_var03 = 0;

	setInputDisabled(0);

	getGameLoaderInteractionController()->enableFlag24();
	getCurrSceneSc2MotionController()->setEnabled();

	BehaviorEntryInfo *beh = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_BLINK);
	if (beh)
		beh->_percent = 327;

	beh = g_fp->_behaviorManager->getBehaviorEntryInfoByMessageQueueDataId(g_vars->scene14_grandma, ST_GMA_SIT, QU_GMA_THROW);
	if (beh)
		beh->_percent = 0;

	g_vars->scene14_var01 = 200;
	g_vars->scene14_var02 = 200;

	g_fp->_aniMan2 = g_fp->_aniMan;

	g_vars->scene14_var07 = 300;
	g_vars->scene14_var08 = 300;
}

void sceneHandler14_winArcade() {
	if (g_vars->scene14_var03) {
		if (g_vars->scene14_var04) {
			g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);

			g_vars->scene14_var04 = 0;
		}

		if (g_vars->scene14_var10) {
			g_vars->scene14_var11.push_back(g_vars->scene14_var10);

			g_vars->scene14_var10->_flags &= 0xFFFB;
			g_vars->scene14_var10 = 0;
		}

		g_vars->scene14_var05 = 0;

		sceneHandler14_endArcade();

		g_vars->scene14_var13 = 0;

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
		g_vars->scene14_var11.push_back(g_vars->scene14_pink);
		g_vars->scene14_pink = 0;
	}
}

void sceneHandler14_startArcade() {
	g_vars->scene14_var03 = 1;
	g_vars->scene14_var06 = 1;

	if (g_fp->_aniMan->_movement) {
		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT | 0x4000);
		g_fp->_aniMan->setOXY(1237, 451);
		g_fp->_aniMan->_priority = 25;
	}

	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_aniMan2 = 0;
	g_vars->scene14_var01 = 50;
	g_vars->scene14_var07 = 100;
	g_vars->scene14_var24 = 4;
	g_vars->scene14_pink = 0;

	chainQueue(QU_SC14_STARTARCADE, 0);

	g_fp->_updateScreenCallback = sceneHandler14_updateScreenCallback;
}

void sceneHandler14_sub06() {
	g_fp->_aniMan->_callback2 = 0;
	g_vars->scene14_var04 = 0;
}

void sceneHandler14_kickAnimation() {
	if (g_fp->_aniMan->_movement) {
		sceneHandler14_sub06();

		if (g_vars->scene14_var10 && g_vars->scene14_var14 - g_vars->scene14_var10->_ox < 180) {
			g_fp->_aniMan->changeStatics2(g_fp->_aniMan->_movement->_staticsObj2->_staticsId);
			g_fp->_aniMan->startAnim(MV_MAN14_KICK, 0, -1);

			g_vars->scene14_var05 = 0;

			g_vars->scene14_var10->stopAnim_maybe();
			g_vars->scene14_var10->hide();
		} else {
			g_fp->_aniMan->changeStatics2(g_fp->_aniMan->_movement->_staticsObj2->_staticsId);
			g_fp->_aniMan->startAnim(MV_MAN14_KICKAIR, 0, -1);
		}
	}
}

void sceneHandler14_declineCallback(int *arg) {
	Common::Point point;

	if (g_vars->scene14_var04) {
		*arg = (int)(sqrt((g_fp->_mouseVirtY - g_vars->scene14_mouseCursorPos.y)
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

	g_vars->scene14_var04 = 1;
}

bool sceneHandler14_sub04(ExCommand *cmd) {
	if (!getCurrSceneSc2MotionController()->_isEnabled)
		return 0;

	if (!g_vars->scene14_var13) {
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
	g_vars->scene14_var10->stopAnim_maybe();
	g_vars->scene14_var10->_priority = 27;

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
	ExCommand *ex = new ExCommand(ANI_BALL14, 1, MV_BAL14_FALL, 0, 0, 0, 1, 0, 0, 0);

	ex->_keyCode = g_vars->scene14_var10->_okeyCode;
	ex->_excFlags |= 2;
	ex->_field_24 = 1;
	mq->addExCommandToEnd(ex);

	ex = new ExCommand(ANI_BALL14, 6, 0, 0, 0, 0, 1, 0, 0, 0);
	ex->_keyCode = g_vars->scene14_var10->_okeyCode;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);
	mq->chain(0);

	g_vars->scene14_var11.push_back(g_vars->scene14_var10);
	g_vars->scene14_var10 = 0;

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

	g_vars->scene14_var18 += 71;
	g_fp->_currentScene->_x = 71;

	g_fp->_aniMan2 = g_fp->_aniMan;
}

void sceneHandler14_dudeFall() {
	if (!g_fp->_aniMan->_movement || g_fp->_aniMan->_movement->_id != MV_MAN14_FALL) {
		sceneHandler14_sub06();

		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);
		g_fp->_aniMan->startAnim(MV_MAN14_FALL, 0, -1);
		g_vars->scene14_var10->stopAnim_maybe();
		g_vars->scene14_var10->hide();

		sceneHandler14_grandmaJumpThrow();
	}
	++g_vars->scene14_var24;
}

void sceneHandler14_grandmaStepForward() {
	g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT|0x4000);
	g_fp->_aniMan->startAnim(MV_MAN14_STEPFW, 0, -1);

	g_vars->scene14_var18 -= 71;

	g_fp->_currentScene->_x = -71;
	g_fp->_aniMan2 = g_vars->scene14_grandma;
}

void sceneHandler14_arcadeLogic() {
	g_vars->scene14_var10->stopAnim_maybe();
	g_vars->scene14_var10->hide();

	if (g_vars->scene14_var04)
		sceneHandler14_sub06();

	if (g_vars->scene14_var24 <= 1) {
		setInputDisabled(1);

		sceneHandler14_sub06();

		g_vars->scene14_var06 = 0;
		g_fp->_aniMan2 = 0;

		chainQueue(QU_SC14_WINARCADE, 1);

		--g_vars->scene14_var24;
	} else {
		ExCommand *ex;

		g_vars->scene14_grandma->changeStatics2(ST_GMA_SIT);

		if (g_vars->scene14_var24 != 3 || g_vars->scene14_pink) {
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
		--g_vars->scene14_var24;
	}
}

void sceneHandler14_sub01() {
	int x = g_vars->scene14_var20 + g_vars->scene14_var22;
	int y = g_vars->scene14_var21 + g_vars->scene14_var23;

	g_vars->scene14_var22 += g_vars->scene14_var20;
	g_vars->scene14_var23 += g_vars->scene14_var21;

	g_vars->scene14_var21++;

	if (g_vars->scene14_var21 - 1 + g_vars->scene14_var23 > 517) {
		if (x <= g_vars->scene14_var14 - 16 ) {
			if ( g_vars->scene14_var20 >= 0 || x >= g_vars->scene14_var16 + 65 || x <= g_vars->scene14_var16 - 135 || y <= g_vars->scene14_var17 - 102 ) {
				if (g_vars->scene14_var10->_movement)
					g_vars->scene14_var10->_movement->setOXY(x, y);
				else
					g_vars->scene14_var10->setOXY(x, y);
			} else {
				sceneHandler14_arcadeLogic();
				g_vars->scene14_var05 = 0;
			}
		} else {
			sceneHandler14_dudeFall();
			g_vars->scene14_var05 = 0;
		}
	} else {
		sceneHandler14_passToGrandma();
		g_vars->scene14_var05 = 0;
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

		g_vars->scene14_var13 = 0;
		break;

	case 64:
		g_fp->lift_sub05(cmd);
		break;

	case 33:
		{
			Movement *mov = g_fp->_aniMan->_movement;

			if (mov) {
				g_vars->scene14_var14 = mov->_ox;
				g_vars->scene14_var15 = mov->_oy;

				if (mov->_id == MV_MAN14_KICK)
					g_vars->scene14_var14 = mov->_ox + 2 * g_fp->_aniMan->_movement->_currDynamicPhaseIndex;
			} else {
				g_vars->scene14_var14 = g_fp->_aniMan->_ox;
				g_vars->scene14_var15 = g_fp->_aniMan->_oy;
			}

			mov = g_vars->scene14_grandma->_movement;
			if (mov) {
				g_vars->scene14_var16 = mov->_ox;
				g_vars->scene14_var17 = mov->_oy;
			} else {
				g_vars->scene14_var16 = g_vars->scene14_grandma->_ox;
				g_vars->scene14_var17 = g_vars->scene14_grandma->_oy;
			}

			if (g_fp->_aniMan2) {
				int x = g_fp->_aniMan2->_ox;
				g_vars->scene14_var18 = x;
				g_vars->scene14_var19 = g_fp->_aniMan2->_oy;

				if (x < g_fp->_sceneRect.left + g_vars->scene14_var01) {
					g_fp->_currentScene->_x = x - g_vars->scene14_var07 - g_fp->_sceneRect.left;
					x = g_vars->scene14_var18;
				}

				if (x > g_fp->_sceneRect.right - g_vars->scene14_var01)
					g_fp->_currentScene->_x = x + g_vars->scene14_var07 - g_fp->_sceneRect.right;
			}

			if (g_vars->scene14_var05)
				sceneHandler14_sub01();

			g_fp->_behaviorManager->updateBehaviors();
			g_fp->startSceneTrack();
			break;
		}

	case 30:
		if (g_vars->scene14_var04) {
			sceneHandler14_kickAnimation();
			break;
		}

		if (!g_vars->scene14_var03) {
			break;
		}
		break;

	case 29:
        if (g_vars->scene14_var03) {
			int pixel;

			if (g_vars->scene14_var06 && g_fp->_aniMan->getPixelAtPos(cmd->_sceneClickX, cmd->_sceneClickY, &pixel) && !g_fp->_aniMan->_movement) {
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

			if (!sceneHandler14_sub04(cmd) && (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode))) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						sceneHandler14_sub04(cmd);
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
