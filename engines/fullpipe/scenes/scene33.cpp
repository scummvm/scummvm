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

void scene33_initScene(Scene *sc) {
	g_vars->scene33_var01 = 200;
	g_vars->scene33_var02 = 200;
	g_vars->scene33_var03 = 300;
	g_vars->scene33_var04 = 300;
	g_vars->scene33_mug = Scene_getStaticANIObject1ById(sc, ANI_MUG_33, -1);

	memcpy(g_vars->scene33_ventsState, g_vars->scene33_var06, 36u);

	g_vars->scene33_jettie = Scene_getStaticANIObject1ById(sc, ANI_JETTIE_FLOW, -1);
	g_vars->scene33_var07 = 0;
	g_vars->scene33_var08 = -1;
	g_vars->scene33_var09 = 0;

	if (getObjectState("Кубик") == getObjectEnumState("Кубик", "В_33")) {
		MessageQueue *mq = new MessageQueue(sc->getMessageQueueById(QU_KBK33_START), 0, 0);

		mq->sendNextCommand();
	}


	for (i = 0; i < 9; i++)
		g_vars->scene33_ventsX[i] = sc->getStaticANIObject1ById(sc, ANI_VENT_33, i)->_ox;

	initArcadeKeys("SC_33");
}

} // End of namespace Fullpipe
