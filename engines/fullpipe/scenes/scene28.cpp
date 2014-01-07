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
#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene28_initScene(Scene *sc) {
	g_vars->scene28_var01 = 200;
	g_vars->scene28_var02 = 200;
	g_vars->scene28_var03 = 300;
	g_vars->scene28_var04 = 300;
	g_vars->scene28_var05 = 1;
	g_vars->scene28_var06 = 1;
	g_vars->scene28_var07 = 0;
	g_vars->scene28_var08 = 0;
	g_vars->scene28_var09 = 0;
	g_vars->scene28_var10 = 0;
	g_vars->scene28_var11 = 0;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"));
	
	g_fp->initArcadeKeys("SC_28");
}

int scene28_updateCursor() {
	g_fp->updateCursorCommon();
	if (g_fp->_objectIdAtCursor == ANI_LIFT || g_fp->_objectIdAtCursor == ANI_LIFT_28)
		if (g_fp->_cursorId == PIC_CSR_DEFAULT)
			g_fp->_cursorId = PIC_CSR_ITN;

	return g_fp->_cursorId;
}

void sceneHandler28_lift1ShowAfter() {
	warning("STUB: sceneHandler28_lift1ShowAfter()");
}

void sceneHandler28_makeFaces(ExCommand *cmd) {
	warning("STUB: sceneHandler28_makeFaces()");
}

void sceneHandler28_trySecondaryPers() {
	warning("STUB: sceneHandler28_trySecondaryPers()");
}

void sceneHandler28_turnOn2() {
	warning("STUB: sceneHandler28_turnOn2()");
}

void sceneHandler28_startWork1() {
	warning("STUB: sceneHandler28_startWork1()");
}

void sceneHandler28_clickLift(int keyCode) {
	warning("STUB: sceneHandler28_clickLift()");
}

void sceneHandler28_lift0Start() {
	chainQueue(QU_SC28_LIFT0_START, 1);
}

void sceneHandler28_lift1Start() {
	warning("STUB: sceneHandler28_lift1Start()");
}

void sceneHandler28_lift2Start() {
	chainQueue(QU_SC28_LIFT2_START, 1);
}

void sceneHandler28_lift3Start() {
	chainQueue(QU_SC28_LIFT3_START, 1);
}

void sceneHandler28_lift4Start() {
	warning("STUB: sceneHandler28_lift4Start()");
}

void sceneHandler28_lift5Start() {
	chainQueue(QU_SC28_LIFT5_START, 1);
}

void sceneHandler28_lift6Start() {
	warning("STUB: sceneHandler28_lift6Start()");
}


int sceneHandler28(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC28_LIFT6MUSIC:
		g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"), "MUSIC_TIOTIA", 1);
		break;

	case MSG_SC28_LIFT6INSIDE:
		g_vars->scene28_var11 = 1;
		break;

	case MSG_SC28_LIFT1_SHOWAFTER:
		sceneHandler28_lift1ShowAfter();
		break;

	case MSG_SC28_MAKEFACES:
		sceneHandler28_makeFaces(cmd);
		break;

	case MSG_SC28_TRYVTORPERS:
		sceneHandler28_trySecondaryPers();
		break;

	case MSG_SC28_TURNOFF_0:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK0, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_TURNON_0:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK0, 0);
		break;

	case MSG_SC28_TURNON_1:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK1, 0);
		break;

	case MSG_SC28_TURNOFF_1:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK1, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_TURNON_2:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK2, 0);
		sceneHandler28_turnOn2();
		break;

	case MSG_SC28_TURNOFF_2:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK2, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_TURNON_3:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK3, 0);
		break;

	case MSG_SC28_TURNOFF_3:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK3, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_TURNON_4:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK4, 0);
		break;

	case MSG_SC28_TURNOFF_4:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK4, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_TURNON_6:
		g_vars->scene28_var07 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK6, 0);
		break;

	case MSG_SC28_TURNOFF_6:
		g_vars->scene28_var08 = g_fp->_currentScene->getPictureObjectById(PIC_SC28_DARK6, 0);
		g_vars->scene28_var08->_flags |= 4;
		break;

	case MSG_SC28_STARTWORK1:
		sceneHandler28_startWork1();
		break;

	case MSG_SC28_CLICKLIFT:
		sceneHandler28_clickLift(cmd->_keyCode);
		break;

	case MSG_SC28_ENDLIFT1:
	case MSG_SC28_ENDLIFT6:
	case MSG_SC28_ENDCABIN:
		g_fp->_behaviorManager->setFlagByStaticAniObject(g_fp->_aniMan, 1);
		g_fp->_aniMan->_flags &= 0xFEFF;
		break;

	case 29:
		{
			if (g_vars->scene28_var11) {
				chainObjQueue(g_fp->_aniMan, QU_SC28_LIFT6_END, 1);

				g_fp->playTrack(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_28"), "MUSIC", 1);

				g_vars->scene28_var11 = 0;
			}

			StaticANIObject *ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani)
				if (ani->_id == ANI_LIFT || ani->_id == ANI_LIFT_28 ) {
					sceneHandler28_clickLift(ani->_okeyCode);

					cmd->_messageKind = 0;
					break;
				}

			if (!ani || !canInteractAny(g_fp->_aniMan, ani, cmd->_keyCode)) {
				int picId = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				PictureObject *pic = g_fp->_currentScene->getPictureObjectById(picId, 0);

				if (!pic || !canInteractAny(g_fp->_aniMan, pic, cmd->_keyCode)) {
					if ((g_fp->_sceneRect.right - cmd->_sceneClickX < 47 && g_fp->_sceneRect.right < g_fp->_sceneWidth - 1)
						|| (cmd->_sceneClickX - g_fp->_sceneRect.left < 47 && g_fp->_sceneRect.left > 0))
						g_fp->processArcade(cmd);
				}
			}
			break;
		}

	case 33:
		if (g_fp->_aniMan2) {
			int x = g_fp->_aniMan2->_ox;

			if (x < g_fp->_sceneRect.left + g_vars->scene28_var01)
				g_fp->_currentScene->_x = x - g_vars->scene28_var03 - g_fp->_sceneRect.left;

			if (x > g_fp->_sceneRect.right - g_vars->scene28_var01)
				g_fp->_currentScene->_x = x + g_vars->scene28_var03 - g_fp->_sceneRect.right;
		}

		if (g_vars->scene28_var07) {
			if (g_vars->scene28_var07->_picture->getAlpha() > 10) {
				g_vars->scene28_var07->_picture->setAlpha(g_vars->scene28_var07->_picture->getAlpha() - 10);
			} else {
				g_vars->scene28_var07->_flags &= 0xFFFB;

				g_vars->scene28_var07 = 0;
			}
		}

		if (g_vars->scene28_var08) {
			if (g_vars->scene28_var08->_picture->getAlpha() < 0xF9u ) {
				g_vars->scene28_var08->_picture->setAlpha(g_vars->scene28_var08->_picture->getAlpha() + 6);
			} else {
				g_vars->scene28_var08->_picture->setAlpha(0xff);

				g_vars->scene28_var08 = 0;
			}
		}

		g_fp->_floaters->update();

		for (uint i = 0; i < g_fp->_floaters->_array2.size(); i++)
			if (g_fp->_floaters->_array2[i]->val13 == 1)
				g_fp->_floaters->_array2[i]->ani->_priority = 15;

		g_fp->_behaviorManager->updateBehaviors();

		g_fp->startSceneTrack();

		break;
	}

	return 0;
}

} // End of namespace Fullpipe
