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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objectnames.h"
#include "fullpipe/constants.h"

#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/interaction.h"
#include "fullpipe/behavior.h"

#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene35_initScene(Scene *sc) {
	g_vars->scene35_hose = sc->getStaticANIObject1ById(ANI_HOSE, -1);
	g_vars->scene35_bellyInflater = sc->getStaticANIObject1ById(ANI_PUZODUV, -1);
	g_vars->scene35_flowCounter = 0;
	g_vars->scene35_fliesCounter = 0;

	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_CloseThing);

	if (g_vars->scene35_bellyInflater->_statics->_staticsId == ST_PDV_LARGE)
		lnk->_flags |= 0x20000000;
	else
		lnk->_flags &= 0xDFFFFFFF;

	int sndId = 0;

	if (g_fp->getObjectState(sO_Valve_35) == g_fp->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
		if ((g_vars->scene35_hose->_flags & 4) && g_vars->scene35_hose->_statics->_staticsId == ST_HZE_NORM) {
			sndId = SND_35_012;
		} else if (g_vars->scene35_bellyInflater->_statics->_staticsId == ST_PDV_SMALL) {
			sndId = SND_35_011;
		}
	}

	if (sndId)
		g_fp->playSound(sndId, 1);

	g_fp->lift_setButton(sO_Level6, ST_LBN_6N);
	g_fp->lift_init(sc, QU_SC35_ENTERLIFT, QU_SC35_EXITLIFT);

	g_fp->initArcadeKeys("SC_35");

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_35"));
}

void sceneHandler35_stopFlow() {
	g_fp->setObjectState(sO_Valve_35, g_fp->getObjectEnumState(sO_Valve_35, sO_TurnedOff));
	g_fp->stopAllSoundInstances(SND_35_011);
	g_fp->playSound(SND_35_026, 0);
}

void sceneHandler35_shrink() {
	getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing, 0);
}

void sceneHandler35_startFlow() {
	if (g_fp->getObjectState(sO_Valve_35) == g_fp->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
		if ((g_vars->scene35_hose->_flags & 4) && g_vars->scene35_hose->_statics->_staticsId == ST_HZE_NORM) {
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene35_bellyInflater, ST_PDV_SMALL, QU_PDV_SML_BLINK, 0);
			g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene35_bellyInflater, ST_PDV_SMALL, QU_PDV_SML_TRY, 0);

			g_vars->scene35_bellyInflater->changeStatics2(ST_PDV_SMALL);
			g_vars->scene35_bellyInflater->_flags &= ~0x100;

			MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(QU_SC35_EATHOZE), 0, 0);

			mq->setFlags(mq->getFlags() | 1);

			ExCommand *cmd = new ExCommand(g_vars->scene35_bellyInflater->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

			cmd->_excFlags |= 3;
			cmd->_z = 256;
			cmd->_messageNum = 0;

			mq->addExCommandToEnd(cmd);

			if (!mq->chain(g_vars->scene35_bellyInflater))
				delete mq;

			g_vars->scene35_bellyInflater->_flags |= 0x100;

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing, 1);

			g_fp->playSound(SND_35_012, 1);
		} else {
			if (!g_vars->scene35_flowCounter)
				g_vars->scene35_flowCounter = 98;

			g_fp->playSound(SND_35_011, 1);
		}
	}
}

void sceneHandler35_genFlies() {
	StaticANIObject *fly = g_fp->_currentScene->getStaticANIObject1ById(ANI_FLY, -1);

	int xoff = 0;
	if ((!fly || !(fly->_flags & 4)) && !(g_fp->_rnd.getRandomNumber(32767) % 30)) {
		int x, y;

		if (g_fp->_rnd.getRandomNumber(1)) {
			x = 600;
			y = 0;
		} else {
			x = 0;
			y = 600;
		}

		int numFlies = g_fp->_rnd.getRandomNumber(3) + 1;

		while (numFlies--) {
			g_fp->_floaters->genFlies(g_fp->_currentScene, g_fp->_rnd.getRandomNumber(55) + 1057,  g_fp->_rnd.getRandomNumber(60) + x + xoff, 4, 1);

			xoff += 40;

			g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1].val2 = 1084;
			g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1].val3 = y;
			g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1].val11 = 8.0;
		}

		g_vars->scene35_fliesCounter = 0;
	}
}

int sceneHandler35(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_fp->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_fp->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fp->lift_startExitQueue();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fp->lift_clickButton();
		break;

	case MSG_SC35_STOPFLOW:
		sceneHandler35_stopFlow();
		break;

	case MSG_SC35_CHECKPIPESOUND:
		if (g_fp->getObjectState(sO_Valve_35) == g_fp->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
			g_fp->stopAllSoundInstances(SND_35_011);
			g_fp->playSound(SND_35_012, 1);

			g_vars->scene35_flowCounter = 0;
			break;
		}
		break;

	case MSG_SC35_SHRINK:
		sceneHandler35_shrink();
		break;

	case MSG_LIFT_GO:
		g_fp->lift_goAnimation();
		break;

	case MSG_SC35_STARTFLOW:
	case MSG_SC35_PLUGHOSE:
		sceneHandler35_startFlow();
		break;

	case 64:
		g_fp->lift_hoverButton(cmd);
		break;

	case 29:
		{
			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(g_fp->_sceneRect.left + cmd->_x, g_fp->_sceneRect.top + cmd->_y);

			if (ani)
				if (ani->_id == ANI_LIFTBUTTON) {
					g_fp->lift_animateButton(ani);
					cmd->_messageKind = 0;
					break;
				}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_param)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_param)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1) || (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0)) {
						g_fp->processArcade(cmd);
						break;
					}
				}
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + 200)
				g_fp->_currentScene->_x = x - 300 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - 200)
				g_fp->_currentScene->_x = x + 300 - g_fp->_sceneRect.right;

			g_fp->sceneAutoScrolling();
		}

		if (g_vars->scene35_flowCounter > 0) {
			--g_vars->scene35_flowCounter;

			if (!g_vars->scene35_flowCounter)
				sceneHandler35_stopFlow();
		}

		g_vars->scene35_fliesCounter++;

		if (g_vars->scene35_fliesCounter >= 160)
			sceneHandler35_genFlies();

		g_fp->_floaters->update();

		g_fp->_behaviorManager->updateBehaviors();
		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
