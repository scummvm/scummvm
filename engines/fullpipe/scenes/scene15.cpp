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

void scene15_initScene(Scene *sc) {
	g_vars->scene15_var01 = 200;
	g_vars->scene15_var02 = 200;
	g_vars->scene15_var03 = 300;
	g_vars->scene15_var04 = 300;
	g_vars->scene15_var05 = 0;

	StaticANIObject *grandma = sc->getStaticANIObject1ById(ANI_GRANDMA_ASS, -1);

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	int grandmaState = g_fp->getObjectState(sO_Grandma);

	if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_1)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(86, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_2)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(71, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_In_15_3)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(49, 399);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else if (grandmaState == g_fp->getObjectEnumState(sO_Grandma, sO_WithoutBoot)) {
		grandma->changeStatics2(ST_GMS_BOOT);
		grandma->setOXY(97, 399);
		grandma->changeStatics2(ST_GMS_BOOTLESS2);
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsClosed));
	} else {
		grandma->hide();
		g_fp->setObjectState(sO_LeftPipe_15, g_fp->getObjectEnumState(sO_LeftPipe_15, sO_IsOpened));
	}
	
	g_vars->scene15_plusminus = sc->getStaticANIObject1ById(ANI_PLUSMINUS, -1);

	if (g_fp->getObjectState(sO_Gurad_2) == g_fp->getObjectEnumState(sO_Gurad_2, sO_Off))
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_MINUS);
	else
		g_vars->scene15_plusminus->_statics = g_vars->scene15_plusminus->getStaticsById(ST_PMS_PLUS);

 	g_vars->scene15_ladder = sc->getPictureObjectById(PIC_SC15_LADDER, 0);
	g_vars->scene15_boot = sc->getStaticANIObject1ById(ANI_BOOT_15, -1);

	if (g_fp->getObjectState(sO_Boot_15) != g_fp->getObjectEnumState(sO_Boot_15, sO_Available))
		g_vars->scene15_boot->_flags &= 0xFFFB;

	g_fp->_currentScene = oldsc;

	g_fp->lift_setButton(sO_Level5, ST_LBN_5N);
	g_fp->lift_sub5(sc, QU_SC15_ENTERLIFT, QU_SC15_EXITLIFT);
}

} // End of namespace Fullpipe
