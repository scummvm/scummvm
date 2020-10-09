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

#include "ngi/floaters.h"

namespace NGI {

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

	if (g_nmi->getObjectState(sO_Valve_35) == g_nmi->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
		if ((g_vars->scene35_hose->_flags & 4) && g_vars->scene35_hose->_statics->_staticsId == ST_HZE_NORM) {
			sndId = SND_35_012;
		} else if (g_vars->scene35_bellyInflater->_statics->_staticsId == ST_PDV_SMALL) {
			sndId = SND_35_011;
		}
	}

	if (sndId)
		g_nmi->playSound(sndId, 1);

	g_nmi->lift_setButton(sO_Level6, ST_LBN_6N);
	g_nmi->lift_init(sc, QU_SC35_ENTERLIFT, QU_SC35_EXITLIFT);

	g_nmi->initArcadeKeys("SC_35");

	g_nmi->_floaters->init(g_nmi->getGameLoaderGameVar()->getSubVarByName("SC_35"));
}

void sceneHandler35_stopFlow() {
	g_nmi->setObjectState(sO_Valve_35, g_nmi->getObjectEnumState(sO_Valve_35, sO_TurnedOff));
	g_nmi->stopAllSoundInstances(SND_35_011);
	g_nmi->playSound(SND_35_026, 0);
}

void sceneHandler35_shrink() {
	getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing, 0);
}

void sceneHandler35_startFlow() {
	if (g_nmi->getObjectState(sO_Valve_35) == g_nmi->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
		if ((g_vars->scene35_hose->_flags & 4) && g_vars->scene35_hose->_statics->_staticsId == ST_HZE_NORM) {
			g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene35_bellyInflater, ST_PDV_SMALL, QU_PDV_SML_BLINK, 0);
			g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene35_bellyInflater, ST_PDV_SMALL, QU_PDV_SML_TRY, 0);

			g_vars->scene35_bellyInflater->changeStatics2(ST_PDV_SMALL);
			g_vars->scene35_bellyInflater->_flags &= ~0x100;

			MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(QU_SC35_EATHOZE), 0, 0);

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

			g_nmi->playSound(SND_35_012, 1);
		} else {
			if (!g_vars->scene35_flowCounter)
				g_vars->scene35_flowCounter = 98;

			g_nmi->playSound(SND_35_011, 1);
		}
	}
}

void sceneHandler35_genFlies() {
	StaticANIObject *fly = g_nmi->_currentScene->getStaticANIObject1ById(ANI_FLY, -1);

	int xoff = 0;
	if ((!fly || !(fly->_flags & 4)) && !(g_nmi->_rnd.getRandomNumber(32767) % 30)) {
		int x, y;

		if (g_nmi->_rnd.getRandomNumber(1)) {
			x = 600;
			y = 0;
		} else {
			x = 0;
			y = 600;
		}

		int numFlies = g_nmi->_rnd.getRandomNumber(3) + 1;

		while (numFlies--) {
			g_nmi->_floaters->genFlies(g_nmi->_currentScene, g_nmi->_rnd.getRandomNumber(55) + 1057,  g_nmi->_rnd.getRandomNumber(60) + x + xoff, 4, 1);

			xoff += 40;

			g_nmi->_floaters->_array2[g_nmi->_floaters->_array2.size() - 1].val2 = 1084;
			g_nmi->_floaters->_array2[g_nmi->_floaters->_array2.size() - 1].val3 = y;
			g_nmi->_floaters->_array2[g_nmi->_floaters->_array2.size() - 1].val11 = 8.0;
		}

		g_vars->scene35_fliesCounter = 0;
	}
}

int sceneHandler35(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SC35_STOPFLOW:
		sceneHandler35_stopFlow();
		break;

	case MSG_SC35_CHECKPIPESOUND:
		if (g_nmi->getObjectState(sO_Valve_35) == g_nmi->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
			g_nmi->stopAllSoundInstances(SND_35_011);
			g_nmi->playSound(SND_35_012, 1);

			g_vars->scene35_flowCounter = 0;
		}
		break;

	case MSG_SC35_SHRINK:
		sceneHandler35_shrink();
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case MSG_SC35_STARTFLOW:
	case MSG_SC35_PLUGHOSE:
		sceneHandler35_startFlow();
		break;

	case 64:
		g_nmi->lift_hoverButton(cmd);
		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

			if (ani)
				if (ani->_id == ANI_LIFTBUTTON) {
					g_nmi->lift_animateButton(ani);
					cmd->_messageKind = 0;
					break;
				}

			if (!ani || !canInteractAny(g_nmi->_aniMan, ani, cmd->_param)) {
				int picId = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_nmi->_aniMan, pic, cmd->_param)) {
					if ((g_nmi->_sceneRect.right - cmd->_sceneClickX < 47 && g_nmi->_sceneRect.right < g_nmi->_sceneWidth - 1) || (cmd->_sceneClickX - g_nmi->_sceneRect.left < 47 && g_nmi->_sceneRect.left > 0)) {
						g_nmi->processArcade(cmd);
						break;
					}
				}
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;

			g_nmi->sceneAutoScrolling();
		}

		if (g_vars->scene35_flowCounter > 0) {
			--g_vars->scene35_flowCounter;

			if (!g_vars->scene35_flowCounter)
				sceneHandler35_stopFlow();
		}

		g_vars->scene35_fliesCounter++;

		if (g_vars->scene35_fliesCounter >= 160)
			sceneHandler35_genFlies();

		g_nmi->_floaters->update();

		g_nmi->_behaviorManager->updateBehaviors();
		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
