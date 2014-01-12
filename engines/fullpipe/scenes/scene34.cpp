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

void sceneHandler34_setExits() {
	warning("STUB: sceneHandler34_setExits()");
}

void scene34_initScene(Scene *sc) {
	g_vars->scene34_var01 = 200;
	g_vars->scene34_var02 = 200;
	g_vars->scene34_var03 = 300;
	g_vars->scene34_var04 = 300;
	g_vars->scene34_cactus = sc->getStaticANIObject1ById(ANI_CACTUS_34, -1);
	g_vars->scene34_vent = sc->getStaticANIObject1ById(ANI_VENT_34, -1);
	g_vars->scene34_hatch = sc->getStaticANIObject1ById(ANI_LUK_34, -1);
	g_vars->scene34_boot = sc->getStaticANIObject1ById(ANI_BOOT_34, -1);

	if (g_fp->getObjectState(sO_Cactus) == g_fp->getObjectEnumState(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_fp->_currentScene;

		g_fp->_currentScene = sc;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_EMPTY);
		g_vars->scene34_cactus->setOXY(506, 674);
		g_vars->scene34_cactus->_priority = 30;

		g_vars->scene34_cactus->changeStatics2(ST_CTS34_GROWNEMPTY2);
		g_vars->scene34_cactus->_flags |= 4;

		g_fp->_currentScene = oldsc;
	}

	if (g_fp->getObjectState(sO_Grandma) == g_fp->getObjectEnumState(sO_Grandma, sO_Strolling))
		g_fp->setObjectState(sO_Grandma, g_fp->getObjectEnumState(sO_Grandma, sO_OnStool));

	sceneHandler34_setExits();

	g_vars->scene34_var05 = 0;
	g_vars->scene34_var06 = 0;
	g_vars->scene34_var07 = 0;
	g_vars->scene34_var08 = g_fp->_rnd->getRandomNumber(500) + 500;

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_34"));

	g_fp->lift_setButton(sO_Level7, ST_LBN_7N);
	g_fp->lift_sub5(sc, QU_SC34_ENTERLIFT, QU_SC34_EXITLIFT);

	g_fp->initArcadeKeys("SC_34");
}

} // End of namespace Fullpipe
