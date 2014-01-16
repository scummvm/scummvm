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

#include "fullpipe/floaters.h"

namespace Fullpipe {

void scene35_initScene(Scene *sc) {
	g_vars->scene35_var01 = 200;
	g_vars->scene35_var02 = 200;
	g_vars->scene35_var03 = 300;
	g_vars->scene35_var04 = 300;
	g_vars->scene35_hose = sc->getStaticANIObject1ById(ANI_HOSE, -1);
	g_vars->scene35_bellyInflater = sc->getStaticANIObject1ById(ANI_PUZODUV, -1);
	g_vars->scene35_var05 = 0;
	g_vars->scene35_var06 = 0;

	MovGraphLink *lnk = getSc2MctlCompoundBySceneId(sc->_sceneId)->getLinkByName(sO_CloseThing);

	if (g_vars->scene35_bellyInflater->_statics->_staticsId == ST_PDV_LARGE)
		lnk->_flags |= 0x20000000;
	else
		lnk->_flags &= 0xDFFFFFFF;

	int sndId = 0;

	if (g_fp->getObjectState(sO_Valve_35) == g_fp->getObjectEnumState(sO_Valve_35, sO_TurnedOn)) {
		if ((g_vars->scene35_hose->_flags & 4) && g_vars->scene35_hose->_statics->_staticsId == ST_HZE_NORM) {
			sndId = SND_35_012;
		} else if (g_vars->scene35_bellyInflater->_statics->_staticsId == ST_PDV_SMALL) {
			sndId = SND_35_011;
		}
	}

	if (sndId)
		g_fp->playSound(sndId, 1);

	g_fp->lift_setButton(sO_Level6, ST_LBN_6N);
	g_fp->lift_sub5(sc, QU_SC35_ENTERLIFT, QU_SC35_EXITLIFT);

	g_fp->initArcadeKeys("SC_35");

	g_fp->_floaters->init(g_fp->getGameLoaderGameVar()->getSubVarByName("SC_35"));
}

} // End of namespace Fullpipe
