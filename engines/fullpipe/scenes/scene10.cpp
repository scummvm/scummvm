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
#include "fullpipe/scenes.h"
#include "fullpipe/statics.h"

#include "fullpipe/behavior.h"

namespace Fullpipe {

void scene10_initScene(Scene *sc) {
	g_vars->scene10_gum = sc->getStaticANIObject1ById(sc, ANI_GUM, -1);
	g_vars->scene10_packet = sc->getStaticANIObject1ById(sc, ANI_PACHKA, -1);
	g_vars->scene10_packet2 = sc->getStaticANIObject1ById(sc, ANI_PACHKA2, -1);
	g_vars->scene10_inflater = sc->getStaticANIObject1ById(sc, ANI_NADUVATEL, -1);
	g_vars->scene10_ladder = sc->getPictureObjectById(sc, PIC_SC10_LADDER, 0);

	g_fullpipe->lift_setButton(sO_Level1, ST_LBN_1N);
	g_fullpipe->lift_sub5(sc, QU_SC10_ENTERLIFT, QU_SC10_EXITLIFT);

	if (g_fullpipe->getObjectState(sO_Inflater) == g_fullpipe->getObjectEnumState(sO_Inflater, sO_WithGum)) {
		g_vars->scene10_hasGum = 1;
	} else {
		g_vars->scene10_hasGum = 0;
		g_vars->scene10_gum->hide();
	}
}

} // End of namespace Fullpipe
