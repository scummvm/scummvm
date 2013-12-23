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

void scene11_dudeSwingCallback(int *arg) {
	warning("STUB: scene11_dudeSwingCallback()");
}

void scene11_initScene(Scene *sc) {
	g_vars->scene11_swingie = sc->getStaticANIObject1ById(ANI_SWINGER, -1);
	g_vars->scene11_boots = sc->getStaticANIObject1ById(ANI_BOOTS_11, -1);
	g_vars->scene11_var01.clear();
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_MAN11, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_dudeOnSwing = sc->getStaticANIObject1ById(ANI_KACHELI, -1);
	g_vars->scene11_dudeOnSwing->_callback2 = scene11_dudeSwingCallback;
	g_vars->scene11_hint = sc->getPictureObjectById(PIC_SC11_HINT, 0);
	g_vars->scene11_hint->_flags &= 0xFFFB;

	g_vars->scene11_var02 = 0;
	g_vars->scene11_var03 = 0;
	g_vars->scene11_var04 = 0;
	g_vars->scene11_var05 = 0;
	g_vars->scene11_var06 = 0;
	g_vars->scene11_var07 = 0;
	g_vars->scene11_var08 = 1.0;
	g_vars->scene11_var09 = 1.0;
	g_vars->scene11_var10 = 1.0;
	g_vars->scene11_var11 = 1.0;
	g_vars->scene11_var12 = 1.9849218750000000;
	g_vars->scene11_var13 = 0;
	g_vars->scene11_var14 = 0;
	g_vars->scene11_var15 = 0;
	g_vars->scene11_var16 = 0;
	g_vars->scene11_var17 = 0;
	g_vars->scene11_var18 = 0;

	Scene *oldsc = g_fp->_currentScene;

	g_fp->_currentScene = sc;

	int swingie = g_fp->getObjectState(sO_Swingie);

	if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwinging)
		|| swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSwingingWithBoot)) {
		g_vars->scene11_var19 = 1;
		g_vars->scene11_var20 = 0;

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(805, 905);

		getSc2MctlCompoundBySceneId(sc->_sceneId)->replaceNodeX(303, 353);
	} else if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInBoots)
			   || swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsStandingInCorner)) {
		g_vars->scene11_var19 = 0;
		g_vars->scene11_var20 = 1;

		g_vars->scene11_swingie->changeStatics2(ST_SWR_STAND3);

		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 1);
		getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 0);

		((MctlCompound *)getCurrSceneSc2MotionController())->replaceNodeX(905, 805);
	} else {
		g_vars->scene11_var19 = 0;
		g_vars->scene11_var20 = 0;

		if (swingie == g_fp->getObjectEnumState(sO_Swingie, sO_IsSitting)) {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SIT);
			g_vars->scene11_swingie->setOXY(144, 389);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		} else {
			g_vars->scene11_swingie->_movement = 0;
			g_vars->scene11_swingie->_statics = g_vars->scene11_swingie->getStaticsById(ST_SWR_SITBALD);
			g_vars->scene11_swingie->setOXY(144, 415);

			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing1, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing2, 0);
			getCurrSceneSc2MotionController()->enableLinks(sO_CloseThing3, 1);
		}
	}

	if (!g_vars->scene11_var19) {
		g_vars->scene11_dudeOnSwing->changeStatics2(ST_KCH_STATIC);
		g_vars->scene11_dudeOnSwing->setOXY(691, 371);
		g_vars->scene11_dudeOnSwing->_priority = 20;

		g_vars->scene11_dudeOnSwing->_flags |= 4;
	}

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_11");
	g_fp->setArcadeOverlay(PIC_CSR_ARCADE5);
}

} // End of namespace Fullpipe
