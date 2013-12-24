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
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene12_initScene() {
	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName("SC_12");
	g_fp->_floaters->init(var);

	g_vars->scene12_fly = g_fp->getObjectState("Муха_12");

	if (g_vars->scene12_fly)
		g_vars->scene12_flyCountdown = 600 * g_fp->_rnd->getRandomNumber(32767) / 0x7fff + 600;

	g_fp->setObjectState("Муха_12", g_fp->_rnd->getRandomNumber(1));
}

} // End of namespace Fullpipe
