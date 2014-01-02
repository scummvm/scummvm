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

void scene30_enablePass(Scene *sc) {
	warning("STUB: scene30_enablePass()");
}

void scene30_initScene(Scene *sc, int flag) {
	g_vars->scene30_var01 = 200;
	g_vars->scene30_var02 = 200;
	g_vars->scene30_var03 = 300;
	g_vars->scene30_var04 = 300;

	Scene *oldsc = g_fp->_currentScene;

	g_vars->scene30_leg = sc->getStaticANIObject1ById(ANI_LEG, -1);
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Leg) == g_fp->getObjectEnumState(sO_Leg, sO_ShowingHeel))
		g_vars->scene30_leg->changeStatics2(ST_LEG_UP);
	else if (g_fp->getObjectState(sO_Leg) == g_fp->getObjectEnumState(sO_Leg, sO_WithoutJugs))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN);
    else if (g_fp->getObjectState(sO_Leg) == g_fp->getObjectEnumState(sO_Leg, sO_WithBig))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN1);
	else if (g_fp->getObjectState(sO_Leg) == g_fp->getObjectEnumState(sO_Leg, sO_WithSmall))
		g_vars->scene30_leg->changeStatics2(ST_LEG_DOWN2);
	else if (g_fp->getObjectState(sO_Leg) == g_fp->getObjectEnumState(sO_Leg, sO_WithAll))
		g_vars->scene30_leg->changeStatics2(ST_LEG_EMPTY);

	g_fp->_currentScene = oldsc;

	scene30_enablePass(sc);

	if (flag == LiftUp || flag == LiftDown)
		g_vars->scene30_var05 = 0;
	else
		g_vars->scene30_var05 = 1;

	g_fp->lift_setButton(sO_Level8, ST_LBN_8N);

	g_fp->lift_sub5(sc, QU_SC30_ENTERLIFT, QU_SC30_EXITLIFT);
}

} // End of namespace Fullpipe
