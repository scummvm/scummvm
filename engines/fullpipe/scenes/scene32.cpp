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

void scene32_initScene(Scene *sc) {
	g_vars->scene32_var01 = 200;
	g_vars->scene32_var02 = 200;
	g_vars->scene32_var03 = 300;
	g_vars->scene32_var04 = 300;
	g_vars->scene32_var05 = 0;
	g_vars->scene32_var06 = 0;
	g_vars->scene32_var07 = 0;
	g_vars->scene32_var08 = -1;
	g_vars->scene32_var09 = 0;
	g_vars->scene32_var10 = 0;
	g_vars->scene32_flag = sc->getStaticANIObject1ById(ANI_FLAG, -1);
	g_vars->scene32_cactus = sc->getStaticANIObject1ById(ANI_CACTUS, -1);
	g_vars->scene32_massOrange = sc->getStaticANIObject1ById(ANI_TESTO_ORANGE, -1);
	g_vars->scene32_massBlue = sc->getStaticANIObject1ById(ANI_TESTO_BLUE, -1);
	g_vars->scene32_massGreen = sc->getStaticANIObject1ById(ANI_TESTO_GREEN, -1);
	g_vars->scene32_button = sc->getStaticANIObject1ById(ANI_BUTTON_32, -1);

	g_vars->scene32_massOrange->startAnim(MV_TSTO_FLOW, 0, -1);
	g_vars->scene32_massOrange->_movement->setDynamicPhaseIndex(15);

	g_vars->scene32_massGreen->startAnim(MV_TSTG_FLOW, 0, -1);
	g_vars->scene32_massGreen->_movement->setDynamicPhaseIndex(26);

	Scene *oldsc = g_fp->_currentScene;
	StaticANIObject *ani;

	if (getObjectState(sO_ClockHandle) == getObjectEnumState(sO_ClockHandle, sO_In_32_Lies)) {
		ani = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
		if (ani) {
			g_fp->_currentScene = sc;

			ani->changeStatics2(ST_HDL_LAID);
		}
	} else {
		if (getObjectState(sO_ClockHandle) == getObjectEnumState(sO_ClockHandle, sO_In_32_Sticks)) {
			ani = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);

			g_fp->_currentScene = sc;

			if (ani)
				ani->changeStatics2(ST_HDL_PLUGGED);

			g_vars->scene32_button->changeStatics2(ST_BTN32_ON);
		}
	}

	g_fp->_currentScene = oldsc;

	if (getObjectState(sO_Cube) == getObjectEnumState(sO_Cube, sO_In_32)) {
		MessageQueue *mq = new Message(sc->getMessageQueueById(QU_KBK32_START), 0, 0);

		mq->sendNextCommand();
	}

	g_fp->lift_setButton(sO_Level9, ST_LBN_9N);
	g_fp->lift_sub5(sc, QU_SC32_ENTERLIFT, QU_SC32_EXITLIFT);

	g_fp->initArcadeKeys("SC_32");
}

} // End of namespace Fullpipe
