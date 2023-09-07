
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
#include "m4/gui/gui_vmng_core.h"
#include "m4/graphics/gr_buff.h"

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
	ScreenContext *_gui = nullptr;
	MemHandle _seriesHandle = nullptr;
	int32 _celsOffset = 0;
	int32 _palOffset = 0;
	char *_assetName = nullptr;
	int _spriteCount = 0;
	M4sprite **_sprites = nullptr;
};

struct Gizmo {
	GrBuff *_grBuff = nullptr;
	int _field4 = 0;
	int _field8 = 0;
	int _fieldC = 0;
	EventHandler _eventHandler = nullptr;
};

struct GizmoItem {
	// TODO
};

struct GizmoButton {
	// TODO
};

extern void gizmo_anim(RGB8 *pal);
extern void gizmo_initialize(RGB8 *pal);
extern void gizmo_shutdown(void *, void *);

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
