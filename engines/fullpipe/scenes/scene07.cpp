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
#include "fullpipe/scenes.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"

namespace Fullpipe {

void scene07_initScene(Scene *sc) {
	g_vars->scene07_lukeAnim = 0;
	g_vars->scene07_lukePercent = 0;
	g_vars->scene07_plusMinus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_fullpipe->getObjectState(sO_Guard_1) == g_fullpipe->getObjectEnumState(sO_Guard_1, sO_Off))
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene07_plusMinus->_statics = g_vars->scene07_plusMinus->getStaticsById(ST_PMS_PLUS);

	if (g_fullpipe->getObjectState(sO_HareTheNooksiter) == g_fullpipe->getObjectEnumState(sO_HareTheNooksiter, sO_WithoutHandle)) {
		Scene *oldsc = g_fullpipe->_currentScene;

		g_fullpipe->_currentScene = sc;

		sc->getStaticANIObject1ById(ANI_CORNERSITTER, -1)->changeStatics2(ST_CST_HANDLELESS);

		g_fullpipe->_currentScene = oldsc;
	}
}

} // End of namespace Fullpipe
