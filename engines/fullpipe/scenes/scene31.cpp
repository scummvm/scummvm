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

void scene31_initScene(Scene *sc) {
	g_vars->scene31_var01 = 200;
	g_vars->scene31_var02 = 200;
	g_vars->scene31_var03 = 300;
	g_vars->scene31_var04 = 300;
	g_vars->scene31_var05 = 0;
	g_vars->scene31_cactus = sc->getStaticANIObject1ById(ANI_CACTUS_31, -1);

	if (g_fp->getObjectState(sO_Cactus) == g_fp->getObjectEnumState(sO_Cactus, sO_HasGrown)) {
		Scene *oldsc = g_fp->_currentScene;
		g_fp->_currentScene = sc;

		g_vars->scene31_cactus->changeStatics2(ST_CTS31_GROWN2);
		g_vars->scene31_cactus->_priority = 22;

		g_fp->_currentScene = oldsc;
	} else {
		g_vars->scene31_cactus->hide();
	}

	g_vars->scene31_plusMinus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_fp->getObjectState(sO_Guard_3) == g_fp->getObjectEnumState(sO_Guard_3, sO_Off))
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene31_plusMinus->_statics = g_vars->scene31_plusMinus->getStaticsById(ST_PMS_PLUS);
}

} // End of namespace Fullpipe
