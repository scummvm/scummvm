/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_inv_window.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/properties.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/engine/script/executing_script.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void sc_invscreen() {
	_G(curscript)->queue_action(ePSAInvScreen, 0, "InventoryScreen");
}

void SetInvDimensions(int ww, int hh) {
	_GP(play).inv_item_wid = ww;
	_GP(play).inv_item_hit = hh;
	_GP(play).inv_numdisp = 0;
	// backwards compatibility
	for (int i = 0; i < _G(numguiinv); i++) {
		_GP(guiinv)[i].ItemWidth = ww;
		_GP(guiinv)[i].ItemHeight = hh;
		_GP(guiinv)[i].OnResized();
	}
}

} // namespace AGS3
