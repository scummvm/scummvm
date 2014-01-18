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

void scene11_dudeSwingCallback(int *arg) {
	int oldarg = *arg;

	*arg = 45 - (int)(g_vars->scene11_swingAngle * -29.66666666666666);

	if (*arg < 1)
		*arg = 1;

	if (*arg > 90)
		*arg = 90;

	g_vars->scene11_swingAngleDiff = (g_vars->scene11_swingAngle - g_vars->scene11_swingOldAngle) * -490.0000000000001;
	g_vars->scene11_swingSpeed = g_vars->scene11_swingAngleDiff * 0.0042 + g_vars->scene11_swingSpeed - g_vars->scene11_swingInertia * (g_vars->scene11_swingAngleDiff * 0.0042 + g_vars->scene11_swingSpeed);
	g_vars->scene11_swingAngle = g_vars->scene11_swingSpeed * 0.0042 + g_vars->scene11_swingAngle;

	if (g_vars->scene11_swingAngle < -1.5) {
		g_vars->scene11_swingAngle = -1.5; //1.0004882812500000;
		g_vars->scene11_swingSpeed = 0.0;
		g_vars->scene11_swingAngleDiff = 0.0;
	}

	if (g_vars->scene11_swingAngle > 1.5) {
		g_vars->scene11_swingAngle = 1.5; //1.9990234375;
		g_vars->scene11_swingSpeed = 0.0;
		g_vars->scene11_swingAngleDiff = 0.0;
	}

	if (g_vars->scene11_swingMaxAngle == *arg && 0.0 != g_vars->scene11_swingSpeed && fabs(g_vars->scene11_swingSpeed) < 2.5) {
		g_vars->scene11_swingSpeed = 0.0;
		g_vars->scene11_swingAngleDiff = 0.0;
		g_vars->scene11_swingAngle = g_vars->scene11_swingOldAngle;
	}

	g_vars->scene11_swingCounter++;

	if (g_vars->scene11_arcadeIsOn) {
		if (g_vars->scene11_hintCounter <= 720) {
			g_vars->scene11_hintCounter++;

			if (g_vars->scene11_hintCounter == 720)
				g_vars->scene11_hint->_flags |= 4;
		}
	}

	if ((oldarg >= 45) != (*arg >= 45) && g_vars->scene11_arcadeIsOn) {
		if (oldarg >= *arg)
			g_fp->playSound(SND_11_031, 0);
		else
			g_fp->playSound(SND_11_020, 0);
	}
}

void scene11_setupMusic() {
	if (g_fp->getObjectState(sO_DudeHasJumped) == g_fp->getObjectEnumState(sO_DudeHasJumped, sO_Yes))
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_11"), "MUSIC2", 1);
}

void scene11_initScene(Scene *sc) {
	g_vars->scene11_swingie = sc->getStaticANIObject1ById(ANI_SWINGER, -1);
	g_vars->scene11_boots = sc->getStaticANIObject1ById(ANI_BOOTS_11, -1);
	g_vars->scene11_mgm.clear();
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_MAN11, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_hint = sc->getPictureObjectById(PIC_SC11_HINT, 0);
	g_vars->scene11_hint->_flags &= 0xFFFB;

	g_vars->scene11_arcadeIsOn = false;
	g_vars->scene11_scrollIsEnabled = false;
	g_vars->scene11_scrollIsMaximized = false;
	g_vars->scene11_hintCounter = 0;
	g_vars->scene11_swingieScreenEdge = 0;
	g_vars->scene11_crySound = 0;
	g_vars->scene11_swingAngle = 0.0;
	g_vars->scene11_swingOldAngle = 0.0;
	g_vars->scene11_swingSpeed = 0.0;
	g_vars->scene11_swingAngleDiff = 0.0;
	g_vars->scene11_swingInertia = 1.28; //1.9849218750000000;
	g_vars->scene11_swingCounter = 0;
	g_vars->scene11_swingCounterPrevTurn = 0;
	g_vars->scene11_swingDirection = 0;
	g_vars->scene11_swingDirectionPrevTurn = 0;

	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	int swingie = g_fp->getObjectState(sO_Swingie);

	if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwinging)
		|| swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwingingWithBoot)) {
		g_vars->scene11_swingIsSwinging = true;
		g_vars->scene11_swingieStands = false;

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(805, 905);

		getSc2MctlCompoundBySceneId(sc->_sceneId)->replaceNodeX(303, 353);
	} else if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)
			   || swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner)) {
		g_vars->scene11_swingIsSwinging = false;
		g_vars->scene11_swingieStands = true;

		g_vars->scene11_swingie->changeStatics2(ST_SWR_STAND3);

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(905, 805);
	} else {
		g_vars->scene11_swingIsSwinging = false;
		g_vars->scene11_swingieStands = false;

		if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting)) {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SIT);
			g_vars->scene11_swingie->setOXY(144, 389);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		} else {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SITBALD);
			g_vars->scene11_swingie->setOXY(144, 415);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		}
	}

	if (!g_vars->scene11_swingIsSwinging) {
		g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC);
		g_vars->scene11_dudeOnSwing->setOXY(691, 371);
		g_vars->scene11_dudeOnSwing->_priority = 20;

		g_vars->scene11_dudeOnSwing->_flags |= 4;
	}

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_11");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE5);
}

void sceneHandler11_restartMan() {
	chainObjQueue(0, QU_SC11_RESTARTMAN, 1);

	getGameLoaderInteractionController()->enableFlag24();
	getCurrSceneSc2MotionController()->setEnabled();

	g_vars->scene11_scrollIsEnabled = false;
}

void sceneHandler11_hitMan() {
	if (g_fp->_aniMan->_ox > 345 && g_fp->_aniMan->_ox < 355) {
		g_fp->_aniMan->changeStatics2(ST_MAN_RIGHT);

		MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC11_MANFALL), 0, 0);

		mq->setFlags(mq->getFlags() | 1);
		if (!mq->chain(g_fp->_aniMan))
			delete mq;

		getCurrSceneSc2MotionController()->replaceNodeX(353, 303);
	}
}

int scene11_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_vars->scene11_arcadeIsOn) {
		if (g_fp->_cursorId != PIC_CSR_DEFAULT_INV && g_fp->_cursorId != PIC_CSR_ITN_INV)
			g_fp->_cursorId = -1;
	} else if (g_vars->scene11_swingie == g_fp->_objectAtCursor && g_fp->_inventory->getSelectedItemId() == ANI_INV_BOOT)
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

int sceneHandler11_updateScreenCallback() {
	int res = g_fp->drawArcadeOverlay(g_vars->scene11_arcadeIsOn);

	if (!res)
		g_fp->_updateScreenCallback = 0;

	return res;
}

void sceneHandler11_manToSwing() {
	g_vars->scene11_arcadeIsOn = true;

	getCurrSceneSc2MotionController()->clearEnabled();
	getGameLoaderInteractionController()->disableFlag24();

	g_fp->_aniMan2->hide();

	g_vars->scene11_swingCounter = 0;
	g_vars->scene11_swingInertia = 1.28; //1.9849218;

	g_vars->scene11_dudeOnSwing->_flags &= 0xFFFB;
	g_vars->scene11_dudeOnSwing = g_fp->_currentScene->getStaticANIObject1ById(ANI_MAN11, -1);
	g_vars->scene11_dudeOnSwing->_statics = g_vars->scene11_dudeOnSwing->getStaticsById(ST_MAN11_EMPTY);
	g_vars->scene11_dudeOnSwing->_movement = 0;
	g_vars->scene11_dudeOnSwing->show1(690, 215, MV_MAN11_SWING_0, 0);
	g_vars->scene11_dudeOnSwing->_priority = 20;
	g_vars->scene11_dudeOnSwing->startAnim(MV_MAN11_SWING_0, 0, -1);
	g_vars->scene11_dudeOnSwing->_movement->setDynamicPhaseIndex(45);

	g_vars->scene11_mgm.addItem(g_fp->_aniMan->_id);

	g_fp->_currentScene->_x = 1400 - g_fp->_sceneRect.right;

	g_vars->scene11_scrollIsEnabled = true;
	g_fp->_updateScreenCallback = sceneHandler11_updateScreenCallback;
}

void sceneHandler11_putABoot() {
	if (g_vars->scene11_boots->_flags & 4) {
		if (g_vars->scene11_boots->_statics->_staticsId == ST_BTS11_ONE)
			chainObjQueue(0, QU_SC11_PUTBOOT2, 1);
	} else {
		chainObjQueue(0, QU_SC11_PUTBOOT1, 1);
	}
}

void sceneHandler11_putBoot() {
	if (abs(353 - g_fp->_aniMan->_ox) > 1 || abs(498 - g_fp->_aniMan->_oy) > 1
		|| g_fp->_aniMan->_movement || g_fp->_aniMan->_statics->_staticsId != ST_MAN_RIGHT) {
		MessageQueue *mq = getCurrSceneSc2MotionController()->method34(g_fp->_aniMan, 353, 498, 1, ST_MAN_RIGHT);

		if (mq) {
			ExCommand *ex = new ExCommand(0, 17, MSG_SC11_PUTBOOT, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 3;

			mq->addExCommandToEnd(ex);

			postExCommand(g_fp->_aniMan->_id, 2, 353, 498, 0, -1);
		}
	} else {
		sceneHandler11_putABoot();
	}
}

void sceneHandler11_showSwing() {
	g_vars->scene11_dudeOnSwing = g_fp->_currentScene->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->_statics = g_vars->scene11_dudeOnSwing->getStaticsById(ST_KCH_0);
	g_vars->scene11_dudeOnSwing->_movement = 0;
	g_vars->scene11_dudeOnSwing->show1(691, 371, MV_KCH_START, 0);
	g_vars->scene11_dudeOnSwing->_priority = 20;
}

void sceneHandler11_jumpFromSwing() {
	g_vars->scene11_arcadeIsOn = false;
	g_vars->scene11_hint->_flags &= 0xFFFB;
	g_vars->scene11_scrollIsEnabled = false;

	getCurrSceneSc2MotionController()->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();

	g_vars->scene11_swingOldAngle = 0.0;
	g_vars->scene11_swingAngleDiff = 0.0;
	g_vars->scene11_swingSpeed = 0.0;
	g_vars->scene11_swingAngle = 0.0;

	g_vars->scene11_dudeOnSwing = g_fp->_currentScene->getStaticANIObject1ById(ANI_MAN11, -1);
	g_vars->scene11_dudeOnSwing->_flags &= 0xFFFB;
	g_vars->scene11_dudeOnSwing = g_fp->_currentScene->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC);
	g_vars->scene11_dudeOnSwing->setOXY(691, 371);
	g_vars->scene11_dudeOnSwing->_priority = 20;
	g_vars->scene11_dudeOnSwing->_flags |= 4;

    MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
    ExCommand *ex = new ExCommand(g_fp->_aniMan->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);
	ex->_field_14 = 256;
	ex->_messageNum = 0;
	ex->_excFlags |= 3;
	mq->addExCommandToEnd(ex);
	mq->setFlags(mq->getFlags() | 1);

	g_fp->_globalMessageQueueList->addMessageQueue(mq);

	g_fp->_aniMan->_flags |= 0x104;
	g_fp->_aniMan->changeStatics2(ST_MAN11_SWING);
	g_fp->_aniMan->setOXY(685, 373);
	g_fp->_aniMan->startAnim(MV_MAN11_JUMPFROMSWING, mq->_id, -1);

	g_fp->_aniMan2 = g_fp->_aniMan;
}

void sceneHandler11_swing0() {
	g_vars->scene11_dudeOnSwing->_statics = g_vars->scene11_dudeOnSwing->getStaticsById(ST_MAN11_EMPTY);
	g_vars->scene11_dudeOnSwing->_movement = 0;
	g_vars->scene11_dudeOnSwing->show1(690, 215, MV_MAN11_SWING_0, 0);
	g_vars->scene11_dudeOnSwing->startAnim(MV_MAN11_SWING_0, 0, -1);
	g_vars->scene11_dudeOnSwing->_movement->setDynamicPhaseIndex(g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex);

	g_vars->scene11_swingDirection = 0;
	g_vars->scene11_swingMaxAngle = 45;
	g_vars->scene11_swingOldAngle = 0.0;
}

void sceneHandler11_swing1() {
	g_vars->scene11_dudeOnSwing->_statics = g_vars->scene11_dudeOnSwing->getStaticsById(ST_MAN11_EMPTY);
	g_vars->scene11_dudeOnSwing->_movement = 0;
	g_vars->scene11_dudeOnSwing->show1(690, 215, MV_MAN11_SWING_1, 0);
	g_vars->scene11_dudeOnSwing->startAnim(MV_MAN11_SWING_1, 0, -1);
	g_vars->scene11_dudeOnSwing->_movement->setDynamicPhaseIndex(g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex);

	g_vars->scene11_swingDirection = 1;
	g_vars->scene11_swingMaxAngle = 42;
	g_vars->scene11_swingOldAngle = -(fabs(g_vars->scene11_swingAngle) * 0.075 + 0.12);
}

void sceneHandler11_swing2() {
	g_vars->scene11_dudeOnSwing->_statics = g_vars->scene11_dudeOnSwing->getStaticsById(ST_MAN11_EMPTY);
	g_vars->scene11_dudeOnSwing->_movement = 0;
	g_vars->scene11_dudeOnSwing->show1(690, 215, MV_MAN11_SWING_2, 0);
	g_vars->scene11_dudeOnSwing->startAnim(MV_MAN11_SWING_2, 0, -1);
	g_vars->scene11_dudeOnSwing->_movement->setDynamicPhaseIndex(g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex);

	g_vars->scene11_swingDirection = 2;
	g_vars->scene11_swingMaxAngle = 48;
	g_vars->scene11_swingOldAngle = fabs(g_vars->scene11_swingAngle) * 0.075 + 0.12;
}

void sceneHandler11_emptySwing() {
	if (g_vars->scene11_swingDirection)
		sceneHandler11_swing0();

	g_vars->scene11_dudeOnSwing->stopAnim_maybe();
	g_vars->scene11_dudeOnSwing->hide();
	g_vars->scene11_dudeOnSwing = g_fp->_currentScene->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->show1(-1, -1, -1, 0);
	g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_EMPTY);
	g_vars->scene11_dudeOnSwing->startAnim(MV_KCH_MOVE2, 0, -1);
	g_vars->scene11_dudeOnSwing->_movement->setDynamicPhaseIndex(g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex);

	g_vars->scene11_swingInertia = 0.03; //1.9881250;
}

void sceneHandler11_jumpHitAndWin() {
	MGMInfo mgminfo;

	sceneHandler11_emptySwing();

	g_fp->_aniMan->show1(690 - (int)(sin(g_vars->scene11_swingAngle) * -267.0), 215 - (int)(cos(g_vars->scene11_swingAngle) * -267.0),
						  MV_MAN11_JUMPHIT, 0);
	g_fp->_aniMan->_priority = 10;

	mgminfo.field_1C = 10;
	mgminfo.ani = g_fp->_aniMan;
	mgminfo.staticsId2 = ST_MAN_1PIX;
	mgminfo.x1 = 1400;
	mgminfo.y1 = 0;
	mgminfo.field_10 = 1;
	mgminfo.flags = 66;
	mgminfo.movementId = MV_MAN11_JUMPHIT;

	MessageQueue *mq = g_vars->scene11_mgm.genMovement(&mgminfo);

	if (mq) {
		g_vars->scene11_crySound = SND_11_024;
		ExCommand *ex = new ExCommand(ANI_MAN, 2, 36, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = -1;
		ex->_excFlags = 2;

		mq->addExCommandToEnd(ex);

		ex = new ExCommand(SC_11, 17, 61, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = TrubaRight;
		ex->_excFlags = 3;

		mq->addExCommandToEnd(ex);

		if (!mq->chain(g_fp->_aniMan))
			delete mq;


		if (g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner))
			g_fp->setObjectState(sO_Swingie, g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting));

		g_fp->setObjectState(sO_DudeHasJumped, g_fp->getObjectEnumState(sO_DudeHasJumped, sO_Yes));
	}
}

void sceneHandler11_jumpOver(double angle) {
	MGMInfo mgminfo;

	sceneHandler11_emptySwing();

	g_fp->_aniMan->show1(690 - (int)(sin(g_vars->scene11_swingAngle) * -267.0), 215 - (int)(cos(g_vars->scene11_swingAngle) * -267.0),
						  MV_MAN11_JUMPOVER, 0);
	g_fp->_aniMan->_priority = 0;

	mgminfo.staticsId2 = ST_MAN_1PIX;
	mgminfo.ani = g_fp->_aniMan;
	mgminfo.x1 = 1163;
	mgminfo.y1 = 837 - (int)(angle * 153.0);
	mgminfo.field_1C = 0;
	mgminfo.field_10 = 1;
	mgminfo.flags = 78;
	mgminfo.movementId = MV_MAN11_JUMPOVER;

	MessageQueue *mq = g_vars->scene11_mgm.genMovement(&mgminfo);

	if (mq) {
		g_vars->scene11_crySound = SND_11_022;

		ExCommand *ex = new ExCommand(0, 17, MSG_SC11_RESTARTMAN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags = 2;

		mq->addExCommandToEnd(ex);

		if (!mq->chain(g_fp->_aniMan))
			delete mq;
	}
}

void sceneHandler11_jumpHit(double angle) {
	MGMInfo mgminfo;

	sceneHandler11_emptySwing();

	if (angle >= 0.0) {
		if (angle > 1.0)
			angle = 1.0;
	} else {
		angle = 0.0;
	}

	g_fp->_aniMan->show1(690 - (int)(sin(g_vars->scene11_swingAngle) * -267.0), 215 - (int)(cos(g_vars->scene11_swingAngle) * -267.0),
						  MV_MAN11_JUMPOVER, 0);
	g_fp->_aniMan->_priority = 0;

	mgminfo.staticsId2 = ST_MAN_1PIX;
	mgminfo.ani = g_fp->_aniMan;
	mgminfo.x1 = 1017 - (int)(angle * -214.0);
	mgminfo.y1 = 700;
	mgminfo.field_1C = 0;
	mgminfo.field_10 = 1;
	mgminfo.flags = 78;
	mgminfo.movementId = MV_MAN11_JUMPHIT;

	MessageQueue *mq = g_vars->scene11_mgm.genMovement(&mgminfo);

	if (mq) {
		g_vars->scene11_crySound = SND_11_022;

		ExCommand *ex = new ExCommand(0, 17, MSG_SC11_RESTARTMAN, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags = 2;

		mq->addExCommandToEnd(ex);

		if (!mq->chain(g_fp->_aniMan))
			delete mq;

	}
}

void sceneHandler11_swingLogic() {
	if (g_vars->scene11_dudeOnSwing->_movement) {
		int ph = g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex;
		if (ph > 53 && ph < 90) {
			if (ph < 70 && g_vars->scene11_swingSpeed >= 22.0) {
				sceneHandler11_jumpOver((double)ph * 0.01428571428571429);  // = 1 / 70
			} else if (ph <= 80 && g_vars->scene11_swingSpeed >= 22.0) {
				sceneHandler11_jumpHitAndWin();
			} else {
				sceneHandler11_jumpHit((double)ph * g_vars->scene11_swingSpeed * 0.0006493506493506494); // = 1/1540
			}

			g_vars->scene11_arcadeIsOn = false;
			g_vars->scene11_hint->_flags &= 0xFFFB;
			return;
		}

		if (ph > 38 && ph < 53 && fabs(g_vars->scene11_swingSpeed) <= 5.0)
			sceneHandler11_jumpFromSwing();
	}
}

void sceneHandler11_setSwingDirection() {
	if (g_vars->scene11_swingDirection == 2)
		g_vars->scene11_swingDirectionPrevTurn = 1;
	else if (g_vars->scene11_swingDirection == 1)
		g_vars->scene11_swingDirectionPrevTurn = 2;
	else
		g_vars->scene11_swingDirectionPrevTurn = (g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex <= 45) + 1;
}

void sceneHandler11_swingieSit() {
	if (g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)) {
		g_vars->scene11_swingIsSwinging = false;
		g_vars->scene11_swingieStands = false;

		g_vars->scene11_swingie->changeStatics2(ST_SWR_SIT);
		g_vars->scene11_swingie->setOXY(144, 389);

		g_fp->setObjectState(sO_Swingie, g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting));

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
	}
}

void sceneHandler11_swingieJumpDown() {
    MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SWR_JUMPDOWN), 0, 0);

	mq->setFlags(mq->getFlags() | 1);

	if (!mq->chain(g_vars->scene11_swingie))
		delete mq;

	g_vars->scene11_swingIsSwinging = false;
	g_vars->scene11_swingieStands = true;
	g_vars->scene11_swingieScreenEdge = g_fp->_sceneRect.left;

	getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
	getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
	getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

	getCurrSceneSc2MotionController()->replaceNodeX(905, 805);
}

void sceneHandler11_winArcade() {
	if (g_vars->scene11_arcadeIsOn) {
		g_vars->scene11_arcadeIsOn = false;

		sceneHandler11_emptySwing();

		g_fp->_gameLoader->preloadScene(SC_11, TrubaRight);
	}
}

int sceneHandler11(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_CMN_WINARCADE:
		sceneHandler11_winArcade();
		break;

	case MSG_SC11_SITSWINGER:
		if (g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)
			    || g_fp->getObjectState(sO_Swingie) == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner)) {
			g_fp->setObjectState(sO_Swingie, g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting));
		}
		break;

	case MSG_SC11_MANCRY:
		g_fp->playSound(g_vars->scene11_crySound, 0);

		g_vars->scene11_crySound = 0;
		break;

	case MSG_SC11_RESTARTMAN:
		sceneHandler11_restartMan();
		break;

	case MSG_SC11_HITMAN:
		sceneHandler11_hitMan();
		break;

	case MSG_SC11_MANTOSWING:
		sceneHandler11_manToSwing();
		break;

	case MSG_SC11_PUTBOOT:
		sceneHandler11_putBoot();
		break;

	case MSG_SC11_SHOWSWING:
		sceneHandler11_showSwing();
		break;

	case 107:
		if (g_vars->scene11_arcadeIsOn)
			sceneHandler11_swingLogic();
		break;

	case 33:
		{
			int res = 0;
			int x, y;

			if (!g_fp->_aniMan2)
				goto LABEL_27;

			x = g_fp->_aniMan2->_ox;
			y = g_fp->_aniMan2->_oy;

			g_vars->scene11_dudeX = x;
			g_vars->scene11_dudeY = y;

			if (g_vars->scene11_scrollIsEnabled) {
				if (x > g_fp->_sceneRect.right - 200)
					g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;
				goto LABEL_26;
			}

			if (g_vars->scene11_scrollIsMaximized) {
				g_fp->_currentScene->_x = g_fp->_sceneWidth - x;

				if (g_vars->scene11_dudeX < 910)
					g_vars->scene11_scrollIsMaximized = false;

			LABEL_26:
				res = 1;
			LABEL_27:
				if (g_vars->scene11_swingieStands) {
					if (g_fp->_sceneRect.left >= 534 && g_vars->scene11_swingieScreenEdge < 534)
						sceneHandler11_swingieSit();

					g_vars->scene11_swingieScreenEdge = g_fp->_sceneRect.left;
				}

				if (!g_vars->scene11_arcadeIsOn)
					goto LABEL_50;

				if (g_vars->scene11_swingCounterPrevTurn <= 0 || g_vars->scene11_swingCounter - g_vars->scene11_swingCounterPrevTurn <= 72) {
				} else {
					sceneHandler11_swing0();
					g_vars->scene11_swingDirectionPrevTurn = 0;
					g_vars->scene11_swingCounterPrevTurn = 0;
				}

				if (!g_vars->scene11_arcadeIsOn)
					goto LABEL_50;

				if (g_vars->scene11_swingDirection == g_vars->scene11_swingDirectionPrevTurn || g_vars->scene11_swingCounterPrevTurn <= 0 || g_vars->scene11_swingCounter - g_vars->scene11_swingCounterPrevTurn <= 2) {
				LABEL_49:
					if (g_vars->scene11_arcadeIsOn) {
						g_fp->_behaviorManager->updateBehaviors();
						g_fp->startSceneTrack();
						return res;
					}
				LABEL_50:
					if (g_vars->scene11_swingIsSwinging
						|| (0.0 == g_vars->scene11_swingSpeed
							&& g_vars->scene11_dudeOnSwing->_movement != 0
							&& g_vars->scene11_dudeOnSwing->_movement->_currDynamicPhaseIndex == 45
							&& (g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC), !g_vars->scene11_arcadeIsOn)
							&& g_vars->scene11_swingIsSwinging)) {
						if (!g_vars->scene11_swingie->_movement) {
							if ((g_vars->scene11_boots->_flags & 4) && g_vars->scene11_boots->_statics->_staticsId == ST_BTS11_2) {
								sceneHandler11_swingieJumpDown();

								g_fp->_behaviorManager->updateBehaviors();
								g_fp->startSceneTrack();
								return res;
							}
							g_vars->scene11_swingie->startAnim(MV_SWR_SWING, 0, -1);
						}
					}
					g_fp->_behaviorManager->updateBehaviors();
					g_fp->startSceneTrack();
					return res;
				}

				if (g_vars->scene11_swingDirectionPrevTurn == 1) {
					if (!g_vars->scene11_swingDirection)
						sceneHandler11_swing1();
					else
						sceneHandler11_swing0();
				} else if (g_vars->scene11_swingDirectionPrevTurn == 2) {
					if (!g_vars->scene11_swingDirection)
						sceneHandler11_swing2();
					else
						sceneHandler11_swing0();
				}

				g_vars->scene11_swingCounterPrevTurn = g_vars->scene11_swingCounter;
				goto LABEL_49;
			}
			if (x >= g_fp->_sceneRect.left + 200) {
				if (x <= g_fp->_sceneRect.right - 200) {
				LABEL_18:
					if (y < g_fp->_sceneRect.top + 200) {
						g_fp->_currentScene->_y = y - g_fp->_sceneRect.top - 300;
						y = g_vars->scene11_dudeY;
						x = g_vars->scene11_dudeX;
					}
					if (y > g_fp->_sceneRect.bottom - 300) {
						g_fp->_currentScene->_y = y - g_fp->_sceneRect.bottom + 300;
						x = g_vars->scene11_dudeX;
					}
					if (x >= 940)
						g_vars->scene11_scrollIsMaximized = true;
					goto LABEL_26;
				}
				g_fp->_currentScene->_x = x - g_fp->_sceneRect.right + 300;
			} else {
				g_fp->_currentScene->_x = x - g_fp->_sceneRect.left - 300;
			}
			y = g_vars->scene11_dudeY;
			x = g_vars->scene11_dudeX;
			goto LABEL_18;
		}

		break;

	case 29:
		if (g_vars->scene11_swingIsSwinging) {
			if (g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y) == g_vars->scene11_swingie
				&& cmd->_keyCode == ANI_INV_BOOT)
				sceneHandler11_putBoot();
		} else {
			if (g_vars->scene11_arcadeIsOn) {
				sceneHandler11_setSwingDirection();

				g_vars->scene11_swingCounterPrevTurn = g_vars->scene11_swingCounter;
			}
		}

		if (!g_vars->scene11_arcadeIsOn) {
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);

						return 0;
					}
				}
			}
			return 0;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
