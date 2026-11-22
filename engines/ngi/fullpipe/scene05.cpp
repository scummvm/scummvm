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

#include "ngi/gameloader.h"
#include "ngi/objects.h"
#include "ngi/objectnames.h"
#include "ngi/constants.h"
#include "ngi/statics.h"
#include "ngi/scene.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/messages.h"
#include "ngi/floaters.h"
#include "ngi/behavior.h"

namespace NGI {

void scene05_initScene(Scene *sc) {
	debugC(1, kDebugSceneLogic, "scene05_initScene()");

#if 0
	Inventory *inv = getGameLoaderInventory();
	inv->addItem(ANI_INV_BOX, 1);
	inv->recalcItemPos();
#endif

	g_vars->scene05_handle = sc->findAniObjectById(ANI_HANDLE, -1);
	g_vars->scene05_wacko = sc->findAniObjectById(ANI_OTMOROZ, -1);
	g_vars->scene05_bigHatch = sc->findAniObjectById(ANI_BIGLUK, -1);


	g_vars->scene05_wackoTicker = 0;
	g_vars->scene05_handleFlipper = 1;
	g_vars->scene05_floatersTicker = 1000;

	Scene *oldscene = g_nmi->_currentScene;

	g_nmi->_currentScene = sc;

	debugC(2, kDebugSceneLogic, "scene05: Weird Wacko state: %d", g_nmi->getStateId(sO_WeirdWacko));

	if (g_nmi->getStateId(sO_WeirdWacko) == g_nmi->getStateValue(sO_WeirdWacko, sO_InGlasses)) {
		debugC(2, kDebugSceneLogic, "scene05: In glasses");
		g_vars->scene05_wacko->setStatic(ST_OTM_GLS_LEFT);
		g_vars->scene05_bigHatch->setStatic(ST_BLK_CLOSED);

		g_vars->scene05_handle->setStatic(ST_HDL_UP);
		g_vars->scene05_handle->_flags |= 4;
	} else if (g_nmi->getStateId(sO_WeirdWacko) == g_nmi->getStateValue(sO_WeirdWacko, sO_WithDrawer)) {
		debugC(2, kDebugSceneLogic, "scene05: With Drawer");
		g_vars->scene05_wacko->setStatic(ST_OTM_BOX_LEFT);
		g_vars->scene05_bigHatch->setStatic(ST_BLK_CLOSED);
		g_vars->scene05_handle->setStatic(ST_HDL_UP);
		g_vars->scene05_handle->_flags |= 4;
	} else {
		if (g_nmi->getStateId(sO_WeirdWacko) != g_nmi->getStateValue(sO_WeirdWacko, sO_WithPlunger)) {
			debugC(2, kDebugSceneLogic, "scene05: Without plunger");

			g_vars->scene05_handle->setStatic(ST_HDL_BROKEN);
			g_vars->scene05_bigHatch->setStatic(ST_BLK_CLOSED);
		} else {
			debugC(2, kDebugSceneLogic, "scene05: With plunger");
		}

		g_vars->scene05_wacko->setStatic(ST_OTM_VNT_LEFT);
	}

	g_nmi->_currentScene = oldscene;
}

void sceneHandler05_makeManFlight() {
	int qid;

	if (!g_vars->scene05_bigHatch->_statics || g_vars->scene05_bigHatch->_statics->_staticsId != ST_BLK_OPEN)
		qid = QU_SC5_MANBUMP;
	else
		qid = QU_SC5_MANFLY;

	MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->findMessageQueue(qid), 0, 0);

	mq->setFlags(mq->getFlags() | 1);

	mq->run(nullptr);
}

void sceneHandler05_doOtmFeedback() {
	int staticsId1;
	int staticsId2;

	if (g_nmi->getStateId(sO_WeirdWacko) == g_nmi->getStateValue(sO_WeirdWacko, sO_InGlasses)) {
		staticsId1 = ST_OTM_GLS_LEFT;
		staticsId2 = (g_vars->scene05_handle->_statics->_staticsId == ST_HDL_DOWN) ? MV_OTM_HANDLEUP : MV_OTM_HANDLEDOWN;
	} else if (g_nmi->getStateId(sO_WeirdWacko) != g_nmi->getStateValue(sO_WeirdWacko, sO_WithDrawer)) {
		return;
	} else {
		staticsId1 = ST_OTM_BOX_LEFT;
		staticsId2 = (g_vars->scene05_handle->_statics->_staticsId == ST_HDL_DOWN) ? MV_OTM_BOXHANDLEUP : MV_OTM_BOXHANDLEDOWN;
	}

	if (g_vars->scene05_wacko->_movement)
		g_vars->scene05_wacko->setStatic(g_vars->scene05_wacko->_movement->_nextStatic->_staticsId);

	if (staticsId1 == g_vars->scene05_wacko->_statics->_staticsId) {
		g_vars->scene05_wacko->startMovement(staticsId2, 0, -1);
	} else {
		MessageQueue *mq = g_vars->scene05_wacko->moveToStatic(staticsId1);

		if (mq) {
			mq->setFlags(mq->getFlags() | 1);

			Command *ex = new Command(0, 17, MSG_SC5_MAKEOTMFEEDBACK, 0, 0, 0, 1, 0, 0, 0);

			ex->_excFlags |= 2;

			mq->addCommandToEnd(ex);
			mq->_isFinished = 0;
		}
	}
}

void sceneHandler05_resetTicks() {
	if (g_nmi->_aniMan->_movement && (g_nmi->_aniMan->_movement->_id == MV_MANHDL_HANDLEUP
											|| g_nmi->_aniMan->_movement->_id == MV_MANHDL_HANDLEDOWN))
		g_vars->scene05_wackoTicker = g_nmi->_updateTicks;
	else
		g_vars->scene05_wackoTicker = 0;
}

void sceneHandler05_genFlies() {
	if (g_vars->scene05_floatersTicker <= 1000)
		return;

	if (g_nmi->_rnd.getRandomNumber(1)) {
		int numFlies = g_nmi->_rnd.getRandomNumber(3) + 1;

		for (int i = 0; i < numFlies; i++) {
			int x = g_nmi->_rnd.getRandomNumber(55) + 538;
			int y = g_nmi->_rnd.getRandomNumber(60) + i * 30 + 520;

			g_nmi->_floaters->addFly(g_nmi->_currentScene, x, y, 5, 1);
			g_nmi->_floaters->_flies.back().targetX = 585;
			g_nmi->_floaters->_flies.back().targetY = -70;
			g_nmi->_floaters->_flies.back().maxSpeed = 8.0;
		}
	}

	g_vars->scene05_floatersTicker = 0;
}

void sceneHandler05_showHandle() {
	g_nmi->_currentScene->findAniObjectById(ANI_HANDLE, -1)->show1(-1, -1, -1, 0);
}

void sceneHandler05_openLuke() {
	AniObject *hatch = g_nmi->_currentScene->findAniObjectById(ANI_BIGLUK, -1);

	hatch->setStatic(ST_BLK_CLOSED);
	hatch->startMovement(MV_BLK_OPEN, 0, -1);

	sceneHandler05_resetTicks();
	sceneHandler05_genFlies();
}

void sceneHandler05_hideHandle() {
	g_nmi->_currentScene->findAniObjectById(ANI_HANDLE, -1)->hide();
}

void sceneHandler05_closeLuke() {
	AniObject *hatch = g_nmi->_currentScene->findAniObjectById(ANI_BIGLUK, -1);

	hatch->setStatic(ST_BLK_OPEN);
	hatch->startMovement(MV_BLK_CLOSE, 0, -1);

	sceneHandler05_resetTicks();
}

void sceneHandler05_testHatch(Command *inex) {
	Command *ex;

	if (g_nmi->_currentScene->findAniObjectById(ANI_BIGLUK, -1)->_statics->_staticsId == ST_BLK_CLOSED) {
		ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = TrubaLeft;
		ex->_excFlags |= 2;
		ex->postMessage();

		return;
	}

	AniObject *wacko = g_nmi->_currentScene->findAniObjectById(ANI_OTMOROZ, -1);

	if (wacko->_movement)
		wacko->setStatic(wacko->_movement->_nextStatic->_staticsId);

	if (g_nmi->getStateId(sO_WeirdWacko) == g_nmi->getStateValue(sO_WeirdWacko, sO_InGlasses)) {
		MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(inex->_parId);

		if (mq)
			mq->removeCommand(mq->getCount() - 1, 1);

		if (wacko->_statics->_staticsId != ST_OTM_GLS_LEFT) {
			mq = wacko->moveToStatic(ST_OTM_GLS_LEFT);

			if (!mq) {
				wacko->setStatic(ST_OTM_GLS_LEFT);
				mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
			}

			mq->setFlags(mq->getFlags() | 1);

			ex = new Command(ANI_OTMOROZ, 1, MV_OTM_HANDLEUP, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2u;
			mq->addCommandToEnd(ex);

			ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = TrubaLeft;
			ex->_excFlags |= 2;
			mq->addCommandToEnd(ex);

			mq->_isFinished = 0;
			return;
		}

		mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
		mq->setFlags(mq->getFlags() | 1);

		ex = new Command(ANI_OTMOROZ, 1, MV_OTM_HANDLEUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);

		ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = TrubaLeft;
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);

		if (!mq->run(wacko))
			delete mq;
	} else if (g_nmi->getStateId(sO_WeirdWacko) == g_nmi->getStateValue(sO_WeirdWacko, sO_WithDrawer)) {
		MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(inex->_parId);

		if (mq)
			mq->removeCommand(mq->getCount() - 1, 1);

		if (wacko->_statics->_staticsId != ST_OTM_BOX_LEFT) {
			mq = wacko->moveToStatic(ST_OTM_BOX_LEFT);
			if (!mq) {
				wacko->setStatic(ST_OTM_BOX_LEFT);
				mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
			}

			mq->setFlags(mq->getFlags() | 1);

			ex = new Command(ANI_OTMOROZ, 1, MV_OTM_BOXHANDLEUP, 0, 0, 0, 1, 0, 0, 0);
			ex->_excFlags |= 2;
			mq->addCommandToEnd(ex);

			ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = TrubaLeft;
			ex->_excFlags |= 2;
			mq->addCommandToEnd(ex);

			mq->_isFinished = 0;

			if (!mq->run(wacko))
				delete mq;

			return;
		}

		mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
		mq->setFlags(mq->getFlags() | 1);

		ex = new Command(ANI_OTMOROZ, 1, MV_OTM_BOXHANDLEUP, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);

		ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = TrubaLeft;
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);

		if (!mq->run(wacko))
			delete mq;

		return;
	} else {
		ex = new Command(SC_5, 17, 61, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = TrubaLeft;
		ex->_excFlags |= 2;
		ex->postMessage();

		return;
	}
}


int sceneHandler05(Command *ex) {
	if (ex->_cmd != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_SC5_BGRSOUNDOFF:
		g_nmi->stopAllSoundInstances(SND_5_026);
		break;

	case MSG_SC5_BGRSOUNDON:
		g_nmi->playSound(SND_5_026, 1);
		break;

	case MSG_SC5_MAKEMANFLIGHT:
		sceneHandler05_makeManFlight();
		break;

	case MSG_SC5_MAKEOTMFEEDBACK:
		if (!g_nmi->_aniMan->_movement || (g_nmi->_aniMan->_movement->_id != MV_MANHDL_HANDLEUP
												&& g_nmi->_aniMan->_movement->_id != MV_MANHDL_HANDLEDOWN)) {
			sceneHandler05_doOtmFeedback();
			g_vars->scene05_wackoTicker = 0;
		}
		break;

	case MSG_SC5_SHOWHANDLE:
		sceneHandler05_showHandle();
		break;

	case MSG_SC5_HANDLEDOWN:
		g_vars->scene05_handle->setStatic(ST_HDL_DOWN);
		sceneHandler05_openLuke();
		break;

	case MSG_SC5_HIDEHANDLE:
		sceneHandler05_hideHandle();
		break;

	case MSG_SC5_HANDLEUP:
		g_vars->scene05_handle->setStatic(ST_HDL_UP);
		sceneHandler05_closeLuke();
		break;

	case MSG_SC5_TESTLUK:
		sceneHandler05_testHatch(ex);
		break;

	case 33:
		{
			int res = 0;
			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan2->_x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.left - 300;

				if (g_nmi->_aniMan2->_x > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.right + 300;

				res = 1;
			}

			if (g_vars->scene05_wackoTicker) {
				if ((g_nmi->_updateTicks - g_vars->scene05_wackoTicker) > 62) {
					if (!g_nmi->_aniMan->_movement || (g_nmi->_aniMan->_movement->_id != MV_MANHDL_HANDLEUP
															&& g_nmi->_aniMan->_movement->_id != MV_MANHDL_HANDLEDOWN)) {
						if (g_vars->scene05_handleFlipper % 2)
							sceneHandler05_doOtmFeedback();

						g_vars->scene05_wackoTicker = 0;

						++g_vars->scene05_handleFlipper;
					}
				}
			}

			++g_vars->scene05_floatersTicker;

			g_nmi->_floaters->onUpdate();

			g_nmi->_behaviorManager->update();

			g_nmi->startSceneTrack();

			return res;
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
