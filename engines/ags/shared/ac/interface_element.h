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

#ifndef AGS_SHARED_AC_INTERFACE_ELEMENT_H
#define AGS_SHARED_AC_INTERFACE_ELEMENT_H

#if defined (OBSOLETE)

#include "ags/shared/ac/interface_button.h" // InterfaceButton

namespace AGS3 {

// this struct should go in a Game struct, not the room structure.
struct InterfaceElement {
	int             x, y, x2, y2;
	int             bgcol, fgcol, bordercol;
	int             vtextxp, vtextyp, vtextalign;  // X & Y relative to topleft of interface
	char            vtext[40];
	int             numbuttons;
	InterfaceButton button[MAXBUTTON];
	int             flags;
	int             reserved_for_future;
	int             popupyp;   // pops up when _G(mousey) < this
	int8            popup;     // does it pop up? (like sierra icon bar)
	int8            on;
	InterfaceElement();
};

} // namespace AGS3

#endif // OBSOLETE

#endif
