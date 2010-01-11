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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GRAPHICS_HELPERS_H
#define SCI_GRAPHICS_HELPERS_H

#include "common/endian.h"	// for READ_LE_UINT16
#include "common/rect.h"
#include "sci/engine/vm_types.h"

namespace Sci {

// Cache limits
#define MAX_CACHED_CURSORS 10
#define MAX_CACHED_VIEWS 50

#define SCI_SHAKE_DIRECTION_VERTICAL 1
#define SCI_SHAKE_DIRECTION_HORIZONTAL 2

typedef int GuiResourceId; // is a resource-number and -1 means no parameter given

typedef int16 TextAlignment;

struct Port {
	uint16 id;
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	GuiResourceId fontId;
	bool greyedOutput;
	int16 penClr, backClr;
	int16 penMode;

	Port(uint16 theId) : id(theId), top(0), left(0),
		curTop(0), curLeft(0),
		fontHeight(0), fontId(0), greyedOutput(false),
		penClr(0), backClr(0xFF), penMode(0) {
	}
};

struct Window : public Port {
	Common::Rect dims; // client area of window
	Common::Rect restoreRect; // total area of window including borders
	uint16 wndStyle;
	uint16 saveScreenMask;
	reg_t hSaved1;
	reg_t hSaved2;
	Common::String title;
	bool bDrawn;

	Window(uint16 theId) : Port(theId),
		wndStyle(0), saveScreenMask(0),
		hSaved1(NULL_REG), hSaved2(NULL_REG),
		bDrawn(false) {
	}
};

struct AnimateEntry {
	reg_t object;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 paletteNo;
	int16 x, y, z;
	int16 priority;
	uint16 signal;
	Common::Rect celRect;
	bool showBitsFlag;
	reg_t castHandle;
};
typedef Common::List<AnimateEntry *> AnimateList;

struct Color {
	byte used;
	byte r, g, b;
};

struct Palette {
	byte mapping[256];
	uint32 timestamp;
	Color colors[256];
	byte intensity[256];
};

struct PalSchedule {
	byte from;
	uint32 schedule;
};

/** Button and frame control flags. */
enum controlStateFlags {
	kControlStateEnabled      = 0x0001,  ///< 0001 - enabled buttons (used by the interpreter)
	kControlStateDisabled     = 0x0004,  ///< 0010 - grayed out buttons (used by the interpreter)
	kControlStateFramed       = 0x0008,  ///< 1000 - widgets surrounded by a frame (used by the interpreter)
	kControlStateDitherFramed = 0x1000   ///< 0001 0000 0000 0000 - widgets surrounded by a dithered frame (used in kgraphics)
};

enum ViewType {
	kViewUnknown,
	kViewEga,
	kViewVga,
	kViewVga11,
	kViewAmiga
};

} // End of namespace Sci

#endif
