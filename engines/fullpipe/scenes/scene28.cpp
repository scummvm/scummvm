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

void scene28_initScene() {
	g_vars->scene28_var01 = 200;
	g_vars->scene28_var02 = 200;
	g_vars->scene28_var03 = 300;
	g_vars->scene28_var04 = 300;
	g_vars->scene28_var05 = 1;
	g_vars->scene28_var06 = 1;
	g_vars->scene28_var07 = 0;
	g_vars->scene28_var08 = 0;
	g_vars->scene28_var09 = 0;
	g_vars->scene28_var10 = 0;
	g_vars->scene28_var11 = 0;

	g_fp->_floaters->init(getGameLoaderGameVar()->getSubVarByName("SC_28"));
	
	g_fp->initArcadeKeys("SC_28");
}

} // End of namespace Fullpipe
