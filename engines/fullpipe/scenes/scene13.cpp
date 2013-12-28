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

void scene13_initScene(Scene *sc) {
	g_vars->scene13_whirlgig = sc->getStaticANIObject1ById(ANI_WHIRLGIG_13, -1);
	g_vars->scene13_guard = sc->getStaticANIObject1ById(ANI_STOROZH, -1);
	g_vars->scene13_handleR = sc->getStaticANIObject1ById(ANI_HANDLE_R, -1);
	g_vars->scene13_handleL = sc->getStaticANIObject1ById(ANI_HANDLE_L, -1);
	g_vars->scene13_bridge = sc->getStaticANIObject1ById(ANI_BRIDGE, -1);
	g_vars->scene13_var01 = 0;
	g_vars->scene13_var02 = 1;

	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_Bridge);
	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_Bridge) == g_fp->getObjectEnumState(sO_Bridge, sO_Convoluted)) {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_CLOSED);

		lnk->_flags |= 0x20000000u;

		g_fp->playSound(SND_13_018, 1);

		g_vars->scene13_whirlgig->_callback2 = 0;
	} else {
		g_vars->scene13_bridge->changeStatics2(ST_BDG_OPEN2);

		lnk->_flags &= 0xDFFFFFFF;

		g_vars->scene13_whirlgig->stopAnim_maybe();
		g_vars->scene13_whirlgig->_callback2 = 0;
		g_vars->scene13_whirlgig->startAnim(MV_WHR13_SPIN, 0, -1);

		if (g_vars->scene13_whirlgig->_movement)
			g_vars->scene13_whirlgig->_movement->setDynamicPhaseIndex(30);

		g_fp->playSound(SND_13_037, 1);
	}

	g_vars->scene13_bridge->_flags &= 0xFFFD;

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_13");
}

} // End of namespace Fullpipe
