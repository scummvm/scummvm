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

#include "fullpipe/objects.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/scenes.h"
#include "fullpipe/messages.h"
#include "fullpipe/floaters.h"
#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene05_initScene(Scene *sc) {
	g_vars->scene05_handle = sc->getStaticANIObject1ById(ANI_HANDLE, -1);
	g_vars->scene05_wacko = sc->getStaticANIObject1ById(ANI_OTMOROZ, -1);
	g_vars->scene05_bigHatch = sc->getStaticANIObject1ById(ANI_BIGLUK, -1);


	g_vars->scene05_var01 = 0;
	g_vars->scene05_var02 = 1;
	g_vars->scene05_var03 = 1000;

	Scene *oldscene = g_fullpipe->_currentScene;

	g_fullpipe->_currentScene = sc;

	if (g_fullpipe->getObjectState(sO_WeirdWacko) == g_fullpipe->getObjectEnumState(sO_WeirdWacko, sO_InGlasses)) {
		g_vars->scene05_wacko->changeStatics2(ST_OTM_GLS_LEFT);
		g_vars->scene05_bigHatch->changeStatics2(ST_BLK_CLOSED);

		g_vars->scene05_handle->changeStatics2(ST_HDL_UP);
		g_vars->scene05_handle->_flags |= 4;
	} else if (g_fullpipe->getObjectState(sO_WeirdWacko) == g_fullpipe->getObjectEnumState(sO_WeirdWacko, sO_WithDrawer)) {
		g_vars->scene05_wacko->changeStatics2(ST_OTM_BOX_LEFT);
		g_vars->scene05_bigHatch->changeStatics2(ST_BLK_CLOSED);
		g_vars->scene05_handle->changeStatics2(ST_HDL_UP);
		g_vars->scene05_handle->_flags |= 4;
	} else {
		g_vars->scene05_wacko->changeStatics2(ST_OTM_VNT_LEFT);

		if (g_fullpipe->getObjectState(sO_WeirdWacko) != g_fullpipe->getObjectEnumState(sO_WeirdWacko, sO_WithPlunger)) {
			g_vars->scene05_handle->changeStatics2(ST_HDL_BROKEN);
			g_vars->scene05_bigHatch->changeStatics2(ST_BLK_CLOSED);
		}
	}

	g_fullpipe->_currentScene = oldscene;
}

void sceneHandler05_makeManFlight() {
	int qid;

	if (!g_vars->scene05_bigHatch->_statics || g_vars->scene05_bigHatch->_statics->_staticsId != ST_BLK_OPEN)
		qid = QU_SC5_MANBUMP;
	else
		qid = QU_SC5_MANFLY;

	MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(qid), 0, 0);

	mq->setFlags(mq->getFlags() | 1);

	mq->chain(0);
}

void sceneHandler05_makeWackoFeedback() {
	int staticsId1;
	int staticsId2;

	if (g_fullpipe->getObjectState(sO_WeirdWacko) == g_fullpipe->getObjectEnumState(sO_WeirdWacko, sO_InGlasses)) {
		staticsId1 = ST_OTM_GLS_LEFT;
		staticsId2 = (g_vars->scene05_handle->_statics->_staticsId == ST_HDL_DOWN) ? MV_OTM_HANDLEUP : MV_OTM_HANDLEDOWN;
	} else if (g_fullpipe->getObjectState(sO_WeirdWacko) != g_fullpipe->getObjectEnumState(sO_WeirdWacko, sO_WithDrawer)) {
		return;
	} else {
		staticsId1 = ST_OTM_BOX_LEFT;
		staticsId2 = (g_vars->scene05_handle->_statics->_staticsId == ST_HDL_DOWN) ? MV_OTM_BOXHANDLEUP : MV_OTM_BOXHANDLEDOWN;
	}

	if (g_vars->scene05_wacko->_movement)
		g_vars->scene05_wacko->changeStatics2(g_vars->scene05_wacko->_movement->_staticsObj2->_staticsId);

	if (staticsId1 == g_vars->scene05_wacko->_statics->_staticsId) {
		g_vars->scene05_wacko->startAnim(staticsId2, 0, -1);
	} else {
		MessageQueue *mq = g_vars->scene05_wacko->changeStatics1(staticsId1);

		if (mq) {
			mq->setFlags(mq->getFlags() | 1);

			ExCommand *ex = new ExCommand(0, 17, MSG_SC5_MAKEOTMFEEDBACK, 0, 0, 0, 1, 0, 0, 0);

			ex->_excFlags |= 2;

			mq->addExCommandToEnd(ex);
			mq->_isFinished = 0;
		}
	}
}

void sceneHandler05_showHandle() {
	g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_HANDLE, -1)->show1(-1, -1, -1, 0);
}

void sceneHandler05_handleDown() {
	warning("STUB: sceneHandler05_handleDown()");
}

void sceneHandler05_hideHandle() {
	g_fullpipe->_currentScene->getStaticANIObject1ById(ANI_HANDLE, -1)->hide();
}

void sceneHandler05_handleUp() {
	warning("STUB: sceneHandler05_handleUp()");
}

void sceneHandler05_testHatch(ExCommand *ex) {
	warning("STUB: sceneHandler05_testHatch()");
}


int sceneHandler05(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	switch (ex->_messageNum) {
	case MSG_SC5_BGRSOUNDOFF:
        g_fullpipe->stopAllSoundInstances(SND_5_026);
		break;

	case MSG_SC5_BGRSOUNDON:
        g_fullpipe->playSound(SND_5_026, 1);
		break;

	case MSG_SC5_MAKEMANFLIGHT:
        sceneHandler05_makeManFlight();
		break;

	case MSG_SC5_MAKEOTMFEEDBACK:
        if (!g_fullpipe->_aniMan->_movement || (g_fullpipe->_aniMan->_movement->_id != MV_MANHDL_HANDLEUP 
												&& g_fullpipe->_aniMan->_movement->_id != MV_MANHDL_HANDLEDOWN)) {
			sceneHandler05_makeWackoFeedback();
			g_vars->scene05_var01 = 0;
        }
		break;

	case MSG_SC5_SHOWHANDLE:
		sceneHandler05_showHandle();
		break;

	case MSG_SC5_HANDLEDOWN:
		g_vars->scene05_handle->changeStatics2(ST_HDL_DOWN);
		sceneHandler05_handleDown();
		break;

	case MSG_SC5_HIDEHANDLE:
		sceneHandler05_hideHandle();
		break;

	case MSG_SC5_HANDLEUP:
		g_vars->scene05_handle->changeStatics2(ST_HDL_UP);
		sceneHandler05_handleUp();
		break;

	case MSG_SC5_TESTLUK:
		sceneHandler05_testHatch(ex);
		break;

	case 33: 
		{
			int res = 0;
			if (g_fullpipe->_aniMan2) {
				if (g_fullpipe->_aniMan2->_ox < g_fullpipe->_sceneRect.left + 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.left - 300;

				if (g_fullpipe->_aniMan2->_ox > g_fullpipe->_sceneRect.right - 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.right + 300;

				res = 1;
			}

			if (g_vars->scene05_var01) {
				if ((g_fullpipe->_updateTicks - g_vars->scene05_var01) > 62 ) {
					if (!g_fullpipe->_aniMan->_movement || (g_fullpipe->_aniMan->_movement->_id != MV_MANHDL_HANDLEUP 
															&& g_fullpipe->_aniMan->_movement->_id != MV_MANHDL_HANDLEDOWN)) {
						if (g_vars->scene05_var02 % 2)
							sceneHandler05_makeWackoFeedback();

						g_vars->scene05_var01 = 0;

						++g_vars->scene05_var02;
					}
				}
			}

			++g_vars->scene05_var03;
			g_fullpipe->_floaters->update();

			g_fullpipe->_behaviorManager->updateBehaviors();

			g_fullpipe->startSceneTrack();

			return res;
		}
	}

	return 0;
}

} // End of namespace Fullpipe
