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

#include "m4/burger/gui/gui_gizmo.h"
#include "m4/burger/vars.h"
#include "m4/core/errors.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"

namespace M4 {
namespace Burger {
namespace GUI {

void gizmo_initialize(RGB8 *pal) {
	if (!_GIZMO(initialized)) {
		_GIZMO(initialized) = true;

		// Remember whether interface was visible when gizmo was initialized
		if (INTERFACE_VISIBLE) {
			_GIZMO(interface_visible) = true;
			interface_hide();
		} else {
			_GIZMO(interface_visible) = false;
		}

		_GIZMO(palette) = pal;
		krn_fade_to_grey(pal, 5, 1);

	}
}

void gizmo_shutdown(bool fade) {
	if (_GIZMO(initialized)) {
		_GIZMO(val1) = 0;

		if (_GIZMO(lowMemory2)) {
			if (!adv_restoreBackground())
				error_show(FL, 0, "unable to restore background");
		}

		if (_GIZMO(lowMemory1)) {
			if (!adv_restoreCodes())
				error_show(FL, 0, "unable to restore screen codes");
		}

		krn_fade_from_grey(_GIZMO(palette), 5, 1, fade ? 1 : 2);

		if (_GIZMO(interface_visible))
			interface_show();

		mouse_unlock_sprite();
		game_pause(false);
		_GIZMO(initialized) = false;
	}
}

} // namespace GUI
} // namespace Burger
} // namespace M4
