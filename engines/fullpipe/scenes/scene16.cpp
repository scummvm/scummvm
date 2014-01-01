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

void scene16_initScene(Scene *sc) {
	g_vars->scene16_var01 = 200;
	g_vars->scene16_var02 = 200;
	g_vars->scene16_var03 = 300;
	g_vars->scene16_var04 = 300;
	g_vars->scene16_var05.clear();
	g_vars->scene16_var06 = 0;
	g_vars->scene16_var07 = 0;
	g_vars->scene16_var08 = 200;
	g_vars->scene16_wire = sc->getStaticANIObject1ById(ANI_WIRE16, -1);
	g_vars->scene16_mug = sc->getStaticANIObject1ById(ANI_MUG, -1);
	g_vars->scene16_jettie = sc->getStaticANIObject1ById(ANI_JETTIE, -1);
	g_vars->scene16_boot = sc->getStaticANIObject1ById(ANI_BOOT_16, -1);
	g_vars->scene16_var09 = 0;
	g_vars->scene16_sound = SND_16_034;

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene16_var10 = 1;

		StaticANIObject *boy[2];
		boy[0] = sc->getStaticANIObject1ById(ANI_BOY, -1);
		boy[0]->loadMovementsPixelData();

		boy[1] = new StaticANIObject(boy[0]);
		sc->addStaticANIObject(boy[1], 1);
		
		int idx = 0;
		
		for (int i = 0; i < 3; i++) {
			g_vars->scene16_var05.push_back(boy[idx]);

			idx++;

			if (idx >= 2)
				idx = 0;
		}
		
		g_vars->scene16_var05.push_back(sc->getStaticANIObject1ById(ANI_GIRL, -1));

		for (int i = 0; i < 4; i++) {
			g_vars->scene16_var05.push_back(boy[idx]);

			idx++;

			if (idx >= 2)
				idx = 0;
		}
	} else {
		g_fp->setObjectState(sO_Girl, g_fp->getObjectEnumState(sO_Girl, sO_IsSwinging));

		g_vars->scene16_var10 = 0;

		StaticANIObject *ani = new StaticANIObject(g_fp->accessScene(SC_COMMON)->getStaticANIObject1ById(ANI_BEARDED_CMN, -1));
		ani->_movement = 0;
		ani->_statics = (Statics *)ani->_staticsList[0];
		sc->addStaticANIObject(ani, 1);
	}

	if (g_fp->getObjectState(sO_Girl) == g_fp->getObjectEnumState(sO_Girl, sO_IsLaughing)) {
		StaticANIObject *girl = sc->getStaticANIObject1ById(ANI_GIRL, -1);

		girl->show1(554, 432, MV_GRL_LAUGH_POPA, 0);
		girl->_priority = 20;
	}

	if (g_fp->getObjectState(sO_Cup) == g_fp->getObjectEnumState(sO_Cup, sO_In_16)) {
		g_vars->scene16_mug->_statics = g_vars->scene16_mug->getStaticsById(ST_MUG_EMPTY);
		g_vars->scene16_mug->_movement = 0;
		g_vars->scene16_mug->setOXY(409, 459);
		g_vars->scene16_mug->_priority = 5;
		g_vars->scene16_mug->_flags |= 4;
	}
}

} // End of namespace Fullpipe
