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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_invwindow.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/properties.h"
#include "ags/shared/gui/guiinv.h"
#include "ags/engine/script/executingscript.h"

namespace AGS3 {

extern ExecutingScript *curscript;
extern GameState play;

void sc_invscreen() {
	curscript->queue_action(ePSAInvScreen, 0, "InventoryScreen");
}

void SetInvDimensions(int ww, int hh) {
	play.inv_item_wid = ww;
	play.inv_item_hit = hh;
	play.inv_numdisp = 0;
	// backwards compatibility
	for (int i = 0; i < numguiinv; i++) {
		guiinv[i].ItemWidth = ww;
		guiinv[i].ItemHeight = hh;
		guiinv[i].OnResized();
	}
	guis_need_update = 1;
}

} // namespace AGS3
