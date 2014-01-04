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

void __thiscall scene23_initScene(Scene *sc) {
	g_vars->scene23_var01 = 200;
	g_vars->scene23_var02 = 200;
	g_vars->scene23_var03 = 300;
	g_vars->scene23_var04 = 300;
	g_vars->scene23_calend0 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 0);
	g_vars->scene23_calend1 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 1);
	g_vars->scene23_calend2 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 2);
	g_vars->scene23_calend3 = sc->getStaticANIObject1ById(ANI_CALENDWHEEL, 3);
	g_vars->scene23_var05 = 0;
	g_vars->scene23_var06 = 0;
	g_vars->scene23_var07 = 0;
	g_vars->scene23_giraffeTop = sc->getStaticANIObject1ById(ANI_GIRAFFE_TOP, -1);
	g_vars->scene23_giraffee = sc->getStaticANIObject1ById(ANI_GIRAFFEE, -1);

	g_fp->_floaters->init(getGameLoaderGameVar()->getSubVarByName("SC_23"));

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	if (g_fp->getObjectState(sO_UpperHatch_23) == g_fp->getObjectEnumState(sO_UpperHatch_23, sO_Opened)) {
		sc->getPictureObjectById(PIC_SC23_BOXOPEN, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BOXCLOSED, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN1, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN2, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN3, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN4, 0)->_flags |= 4;

		if (g_vars->scene23_giraffee->_statics->_staticsId == ST_GRFG_EMPTY || !(g_vars->scene23_giraffee->_flags & 4)) {
			g_vars->scene23_giraffee->changeStatics2(ST_GRFG_BALD);
			g_vars->scene23_giraffee->_flags |= 4;
		}
		g_vars->scene23_calend0->show1(-1, -1, -1, 0);
		g_vars->scene23_calend1->show1(-1, -1, -1, 0);
		g_vars->scene23_calend2->show1(-1, -1, -1, 0);
		g_vars->scene23_calend3->show1(-1, -1, -1, 0);

		sc->getStaticANIObject1ById(ANI_LUK23_U, -1)->changeStatics2(ST_LUK23U_OPEN);
	} else {
		sc->getPictureObjectById(PIC_SC23_BOXOPEN, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BOXCLOSED, 0)->_flags |= 4;
		sc->getPictureObjectById(PIC_SC23_BTN1, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN2, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN3, 0)->_flags &= 0xFFFB;
		sc->getPictureObjectById(PIC_SC23_BTN4, 0)->_flags &= 0xFFFB;

		g_vars->scene23_giraffee->hide();
		g_vars->scene23_calend0->hide();
		g_vars->scene23_calend1->hide();
		g_vars->scene23_calend2->hide();
		g_vars->scene23_calend3->hide();

		sc->getStaticANIObject1ById(ANI_LUK23_U, -1)->changeStatics2(ST_LUK23U_CLOSED);

		g_fp->_floaters->genFlies(sc, 600, 90, 0, 0);
	}

	if (g_fp->getObjectState(sO_LowerHatch_23) == g_fp->getObjectEnumState(sO_LowerHatch_23, sO_Opened)) {
		g_vars->scene23_giraffeTop->show1(-1, -1, -1, 0);
		g_vars->scene23_giraffeTop->changeStatics2(ST_GRFU_UP);

		if (g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened)) {
			g_vars->scene23_giraffeTop->changeStatics2(ST_GRFU_KISS);
			g_vars->scene23_giraffee->hide();
		} else {
			if (g_fp->getObjectState(sO_UpperHatch_23) == g_fp->getObjectEnumState(sO_UpperHatch_23, sO_Opened)
				&& (g_vars->scene23_giraffee->_flags & 4))
				g_vars->scene23_giraffeTop->setOXY(614, 362);
			else
				g_vars->scene23_giraffeTop->setOXY(618, 350);

			if (sceneHandler23_testCalendar())
				g_vars->scene23_calend1->_statics = g_vars->scene23_calend1->getStaticsById(ST_CND_5);
		}

		sc->getStaticANIObject1ById(sc, ANI_LUK23_D, -1)->changeStatics2(ST_LUK23_OPEN);

		if (g_fp->getObjectState(sO_Lever_23) == g_fp->getObjectEnumState(sO_Lever_23, sO_Taken))
			sc->getStaticANIObject1ById(ANI_INV_LEVERHANDLE, -1)->hide();

		sc->getStaticANIObject1ById(ANI_HANDLE23, -1)->hide();
	} else {
		g_vars->scene23_giraffeTop->hide();

		sc->getStaticANIObject1ById(ANI_LUK23_D, -1)->changeStatics2(ST_LUK23_WHANDLE2);

		sc->getStaticANIObject1ById(ANI_INV_LEVERHANDLE, -1)->hide();
	}
	
	StaticANIObject_hide();

	g_fp->_currentScene = oldsc;
}

} // End of namespace Fullpipe
