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

void scene20_setExits(Scene *sc) {
	warning("STUB: scene20_setExits()");
}

void scene20_initScene(Scene *sc) {
	g_vars->scene20_var01 = 200;
	g_vars->scene20_var02 = 200;
	g_vars->scene20_var03 = 300;
	g_vars->scene20_var04 = 300;

	Scene *oldsc = g_fp->_currentScene;

	g_vars->scene20_grandma = sc->getStaticANIObject1ById(ANI_GRANDMA_20, -1);

	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_OnTheFloor))
		g_fp->setObjectState(sO_Grandma, g_fp->getObjectEnumState(sO_Grandma, sO_NearPipe));

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_OnStool)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_STOOL);
	} else if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_OnTheFloor)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_FLOOR);
	} else if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_NearPipe)
			   || g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_NearPipeWithStool)) {
		g_vars->scene20_grandma->changeStatics2(ST_GMA20_STAND);
	} else {
		g_vars->scene20_grandma->hide();
	}

	scene20_setExits(sc);

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_20"));

	for (int i = 0; i < 3; i++) {
		g_fp->_floaters->genFlies(sc, g_fp->_rnd->getRandomNumber(101) + 70, g_fp->_rnd->getRandomNumber(51) + 175, 100, 0);
		g_fp->_floaters->_array2[g_fp->_floaters->_array2.size() - 1]->val13 = g_fp->_rnd->getRandomNumber(9);
	}

	g_fp->_currentScene = oldsc;

	g_vars->scene20_var05 = g_fp->_rnd->getRandomNumber(200) + 400;
}

} // End of namespace Fullpipe
