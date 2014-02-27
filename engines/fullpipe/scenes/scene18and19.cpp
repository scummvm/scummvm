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


namespace Fullpipe {

void scene18_preload() {
	warning("WARNING: scene18_preload()");
}

void scene19_preload(Scene *sc, int key) {
	warning("WARNING: scene19_preload()");
}

void scene18_sub2(StaticANIObject *ani, Scene *sc) {
	warning("WARNING: scene18_sub2()");
}

void scene18_initScene2(Scene *sc) {
	g_vars->scene18_var16 = 200;
	g_vars->scene18_var17 = 200;
	g_vars->scene18_var18 = 300;
	g_vars->scene18_var19 = 300;
	g_vars->scene18_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLIGIG_18, -1);
	g_vars->scene18_var20 = 1032;
	g_vars->scene18_var04 = -318;

	StaticANIObject *armchair = sc->getStaticANIObject1ById(ANI_KRESLO, -1);

	armchair->loadMovementsPixelData();

	g_vars->scene18_var03 = (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging));

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene18_var08 = 1;
		g_fp->playSound(SND_18_006, 1);
	} else {
		g_vars->scene18_var08 = 0;
		g_fp->playSound(SND_18_010, 1);
	}

	scene18_sub2(armchair, sc);

	g_vars->scene18_var21 = 0;
	g_vars->scene18_var12 = 0;
	g_vars->scene18_var22 = 1;
	g_vars->scene18_var23 = -1;
	g_vars->scene18_var24 = 0;
	g_vars->scene18_var25 = 0;
	g_vars->scene18_var26 = 1;
	g_vars->scene18_var27 = -1;
	g_vars->scene18_var13 = -1;
	g_vars->scene18_var14 = -1;
	g_vars->scene18_var28 = 0;
	g_vars->scene18_var15 = 0;
	g_vars->scene18_boy = sc->getStaticANIObject1ById(ANI_BOY18, -1);
	g_vars->scene18_girl = sc->getStaticANIObject1ById(ANI_GIRL18, -1);
	g_vars->scene18_domino = sc->getStaticANIObject1ById(ANI_DOMINO_18, -1);
	g_vars->scene18_var29 = 290;
	g_vars->scene18_var30 = -363;
	g_vars->scene18_var05 = 283;
	g_vars->scene18_var06 = -350;

	g_fp->initArcadeKeys("SC_18");
}

void scene19_initScene2() {
	g_fp->_aniMan2 = 0;
	g_vars->scene19_var01 = 200;
	g_vars->scene19_var02 = 200;
	g_vars->scene19_var03 = 300;
	g_vars->scene19_var04 = 300;
}

} // End of namespace Fullpipe
