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

namespace Fullpipe {

void scene08_initScene(Scene *sc) {
	g_vars->scene08_var06 = 0;
	g_vars->scene08_var01 = 0;
	g_vars->scene08_var02 = 0;
	g_vars->scene08_var03 = 0;
	g_vars->scene08_batuta = sc->getStaticANIObject1ById(ANI_BATUTA, -1);
	g_vars->scene08_vmyats = sc->getStaticANIObject1ById(ANI_VMYATS, -1);
	g_vars->scene08_clock = sc->getStaticANIObject1ById(ANI_CLOCK_8, -1);
	g_vars->scene08_var04 = -37;
	g_vars->scene08_var05 = -1;

	Scene *oldsc = g_fullpipe->_currentScene;
	g_fullpipe->_currentScene = sc;

	int batuta = g_fullpipe->getObjectState(sO_TummyTrampie);
	MovGraphLink *lock = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_CloseThing);

	if (lock)
		lock->_flags &= 0xDFFFFFFF;

	if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsEating)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SPOON);
	} else if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsDrinking)) {
		g_vars->scene08_batuta->changeStatics2(g_vars->scene08_batuta, ST_BTT_NOSPOON);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(3);
	} else if (batuta== g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsScratchingBelly)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_CHESHET);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(8);
	} else if (batuta == g_fullpipe->getObjectEnumState(sO_TummyTrampie, sO_IsSleeping)) {
		g_vars->scene08_batuta->changeStatics2(ST_BTT_SLEEPS);

		g_vars->scene08_clock->startAnim(MV_CLK8_GO, 0, -1);
		g_vars->scene08_clock->_movement->setDynamicPhaseIndex(2);

		if (lock)
			lock->_flags |= 0x20000000;

		g_vars->scene08_var05 = 71;
	}

	g_vars->scene08_clock->_callback2 = 0;

	if (g_fullpipe->getObjectState(sO_StairsUp_8) == g_fullpipe->getObjectEnumState(sO_StairsUp_8, sO_Broken)) {
		g_vars->scene08_var07 = 0;

		sc->getPictureObjectById(PIC_SC8_LADDER, 0)->_flags &= 0xFFFB;

		g_vars->scene08_var04 = -39;
	} else {
		g_vars->scene08_var07 = 1;
	}

	sc->getPictureObjectById(PIC_SC8_ARCADENOW, 0)->_flags &= 0xFFFB;

	g_fullpipe->_currentScene = oldsc;

	g_fullpipe->_floaters->init(getGameLoaderGameVar()->getSubVarByName("SC_8"));
	g_fullpipe->_floaters->genFlies(sc, 100, 100, 0, 0);

	setArcadeOverlay(PIC_CSR_ARCADE3);
}

} // End of namespace Fullpipe
