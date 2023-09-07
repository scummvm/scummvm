
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

#ifndef M4_BURGER_GUI_GIZMO_H
#define M4_BURGER_GUI_GIZMO_H

#include "m4/m4_types.h"

namespace M4 {
namespace Burger {
namespace GUI {

#define _GIZMO(X) g_vars->_gizmo._##X

struct Gizmo_Globals {
	bool _initialized = false;
	bool _interface_visible = false;
	RGB8 *_palette = nullptr;
	bool _lowMemory1 = false;
	bool _lowMemory2 = false;
	int _val1 = 0;
};

extern void gizmo_initialize(RGB8 *pal);
extern void gizmo_shutdown(bool fade);

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
