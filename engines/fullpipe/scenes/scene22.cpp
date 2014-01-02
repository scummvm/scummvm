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

void scene22_initScene(Scene *sc) {
	g_vars->scene22_var01 = 200;
	g_vars->scene22_var02 = 200;
	g_vars->scene22_var03 = 300;
	g_vars->scene22_var04 = 300;
	g_vars->scene22_bag = sc->getStaticANIObject1ById(ANI_MESHOK, -1);

	Scene *oldsc = g_fp->_currentScene;
	g_fp->_currentScene = sc;

	g_vars->scene22_giraffeMiddle = sc->getStaticANIObject1ById(ANI_GIRAFFE_MIDDLE, -1);
	g_vars->scene22_var07 = 0;
	g_vars->scene22_var08 = 0;
	g_vars->scene22_var09 = 0;
	g_vars->scene22_var10 = 1;

	if (g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_NotFallen))
		g_vars->scene22_var11 = 0;
	else if (g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_FallenOnce))
		g_vars->scene22_var11 = 1;
	else if ( g_fp->getObjectState(sO_Bag_22) == g_fp->getObjectEnumState(sO_Bag_22, sO_FallenTwice))
		g_vars->scene22_var11 = 2;
	else {
		g_vars->scene22_var11 = 3;
        g_vars->scene22_var10 = 0;
	}


	if ( g_fp->getObjectState(sO_LowerPipe_21) == g_fp->getObjectEnumState(sO_LowerPipe_21, sO_IsOpened))
		g_vars->scene22_giraffeMiddle->changeStatics2(ST_GRFM_AFTER);
	else
		g_vars->scene22_giraffeMiddle->changeStatics2(ST_GRFM_NORM);

	g_fp->_currentScene = oldsc;

	g_fp->initArcadeKeys("SC_22");
}

int scene22_updateCursor() {
	g_fp->updateCursorCommon();

	if (g_fp->_objectIdAtCursor != ANI_HANDLE_L)
		return g_fp->_cursorId;

	int sel = g_fp->_inventory->getSelectedItemId();

	if (!sel) {
		g_fp->_cursorId = PIC_CSR_ITN;
		return g_fp->_cursorId;
	}

	if (g_vars->scene22_var07 || (sel != ANI_INV_STOOL && sel != ANI_INV_BOX))
		; //empty
	else
		g_fp->_cursorId = PIC_CSR_ITN_INV;

	return g_fp->_cursorId;
}

void scene22_setBagState() {
	if (g_vars->scene22_var10) {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 1);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 0);
	} else {
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_CRANEOUT, 0);
		g_fp->_behaviorManager->setBehaviorEnabled(g_vars->scene22_bag, ST_MSH_SIT, QU_MSH_MOVE, 1);
	}
}

} // End of namespace Fullpipe
