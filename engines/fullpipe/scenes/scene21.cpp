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

void scene21_initScene(Scene *sc) {
	g_vars->scene21_var01 = 200;
	g_vars->scene21_var02 = 200;
	g_vars->scene21_var03 = 300;
	g_vars->scene21_var04 = 300;

	Scene *oldsc = g_fp->_currentScene;

	g_vars->scene21_giraffeBottom = sc->getStaticANIObject1ById(ANI_GIRAFFE_BOTTOM, -1);
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
		g_vars->scene21_giraffeBottom->changeStatics2(ST_GRFB_HANG);
		g_vars->scene21_var05 = 1;
		g_vars->scene21_var06 = 0.0;
		g_vars->scene21_giraffeBottomX = g_vars->scene21_giraffeBottom->_ox;
		g_vars->scene21_giraffeBottomY = g_vars->scene21_giraffeBottom->_oy;
		g_vars->scene21_var07 = 0;
	} else {
		g_vars->scene21_var05 = 0;
	}
	g_fp->_currentScene = oldsc;
	g_fp->initArcadeKeys("SC_21");
}

} // End of namespace Fullpipe
