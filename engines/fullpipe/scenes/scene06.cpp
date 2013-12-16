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
#include "fullpipe/gfx.h"
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"

namespace Fullpipe {

void sceneHandler06_setExits(Scene *sc) {
	warning("STUB: sceneHandler06_setExits()");
}

void scene06_initScene(Scene *sc) {
	g_vars->scene06_mumsy = sc->getStaticANIObject1ById(ANI_MAMASHA, -1);
	g_vars->scene06_var06 = 0;
	g_vars->scene06_invHandle = sc->getStaticANIObject1ById(ANI_INV_HANDLE, -1);
	g_vars->scene06_liftButton = sc->getStaticANIObject1ById(ANI_BUTTON_6, -1);
	g_vars->scene06_ballDrop = sc->getStaticANIObject1ById(ANI_BALLDROP, -1);
	g_vars->scene06_var07 = 0;
	g_vars->scene06_var08 = 0;
	g_vars->scene06_var09 = 0;
	g_vars->scene06_var10 = 0;
	g_vars->scene06_var11 = 0;
	g_vars->scene06_balls.clear();
	g_vars->scene06_var12 = 0;
	g_vars->scene06_var13 = 0;
	g_vars->scene06_var14 = 0;
	g_vars->scene06_var15 = 1;

	StaticANIObject *ball = sc->getStaticANIObject1ById(ANI_NEWBALL, -1);

	ball->hide();
	ball->_statics = ball->getStaticsById(ST_NBL_NORM);
	g_vars->scene06_balls.push_back(ball);

	for (int i = 0; i < 3; i++) {
		StaticANIObject *ball2 = new StaticANIObject(ball);

		ball2->hide();
		ball2->_statics = ball2->getStaticsById(ST_NBL_NORM);

		sc->addStaticANIObject(ball2, 1);

		g_vars->scene06_balls.push_back(ball2);
	}

	if (g_fullpipe->getObjectState(sO_BigMumsy) == g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsPlaying))
		g_fullpipe->setObjectState(sO_BigMumsy, g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsSleeping));

	if (g_fullpipe->getObjectState(sO_BigMumsy) != g_fullpipe->getObjectEnumState(sO_BigMumsy, sO_IsSleeping))
		g_vars->scene06_mumsy->hide();

	g_fullpipe->lift_setButton(sO_Level3, ST_LBN_3N);
	g_fullpipe->lift_sub5(sc, QU_SC6_ENTERLIFT, QU_SC6_EXITLIFT);
	g_fullpipe->initArcadeKeys("SC_6");

	sceneHandler06_setExits(sc);

	g_fullpipe->setArcadeOverlay(PIC_CSR_ARCADE2);
}

} // End of namespace Fullpipe
