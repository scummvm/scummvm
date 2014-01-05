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

void scene26_initScene(Scene *sc) {
	g_vars->scene26_chhi = sc->getStaticANIObject1ById(ANI_CHHI, -1);
	g_vars->scene26_drop = sc->getStaticANIObject1ById(ANI_DROP_26, -1);
	g_vars->scene26_sockPic = sc->getPictureObjectById(PIC_SC26_SOCK, 0);
	g_vars->scene26_sock = sc->getStaticANIObject1ById(ANI_SOCK_26, -1);
	g_vars->scene26_var01 = 200;
	g_vars->scene26_var02 = 200;
	g_vars->scene26_var03 = 300;
	g_vars->scene26_var04 = 300;

	if (g_fp->getObjectState(sO_Hatch_26) == g_fp->getObjectEnumState(sO_Hatch_26, sO_WithSock)) {
		g_fp->setObjectState(sO_Hatch_26, g_fp->getObjectEnumState(sO_Hatch_26, sO_Closed));
		g_fp->setObjectState(sO_Sock_26, g_fp->getObjectEnumState(sO_Sock_26, sO_HangsOnPipe));
	}

	Interaction *inter = getGameLoaderInteractionController()->getInteractionByObjectIds(ANI_LUK26, ANI_MAN, ANI_INV_SOCK);

	if (getGameLoaderInventory()->getCountItemsWithId(ANI_INV_VENT) == 0)
		inter->_flags &= 0xFFFDFFFF;
	else
		inter->_flags |= 0x20000;

	if (g_fp->getObjectState(sO_Sock_26) == g_fp->getObjectEnumState(sO_Sock_26, sO_HangsOnPipe))
		g_vars->scene26_sockPic->_flags |= 4;
	else
		g_vars->scene26_sockPic->_flags &= 0xFFFB;

	if (g_fp->getObjectState(sO_Valve1_26) == g_fp->getObjectEnumState(sO_Valve1_26, sO_Opened))
		g_fp->playSound(SND_26_018, 1);
}

} // End of namespace Fullpipe
