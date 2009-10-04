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

#ifndef SCI_GUI_HELPERS_H
#define SCI_GUI_HELPERS_H

#include "common/rect.h"
#include "sci/engine/vm_types.h"

namespace Sci {

typedef int GUIResourceId; // is a resource-number and -1 means no parameter given
typedef reg_t GUIMemoryHandle;
typedef int16 GUIViewLoopNo;
typedef int16 GUIViewCellNo;

typedef uint16 GUIHandle;

struct GUIPort {
	uint16 id;
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	GUIResourceId fontId;
	int16 textFace, penClr, backClr;
	int16 penMode;

	GUIPort(uint16 theId) : id(theId), top(0), left(0),
		curTop(0), curLeft(0),
		fontHeight(0), fontId(0), textFace(0),
		penClr(0), backClr(0xFF), penMode(0) {
	}
};

struct GUIWindow : public GUIPort {
	Common::Rect dims; // client area of window
	Common::Rect restoreRect; // total area of window including borders
	uint16 wndStyle;
	uint16 uSaveFlag;
	reg_t hSaved1;
	reg_t hSaved2;
	Common::String title;
	bool bDrawn;

	GUIWindow(uint16 theId) : GUIPort(theId),
		wndStyle(0), uSaveFlag(0),
		hSaved1(NULL_REG), hSaved2(NULL_REG),
		bDrawn(false) {
	}
};

struct GUICast {
	uint16 view;
	uint16 loop;
	uint16 cel;
	uint16 z;
	uint16 pal;
	GUIHandle hSaved;
	Common::Rect rect;
};

struct GUIColor {
	byte used;
	byte r, g, b;
};

struct GUIPalette {
	byte mapping[256];
	uint32 timestamp;
	GUIColor colors[256];
	byte intensity[256];
};

struct GUIPalSchedule {
	byte from;
	uint32 schedule;
};

#define STUB(str) debug("STUB: '%s' at file %s line %d",str,__FILE__,__LINE__)


#define GETFTR_WINDOWMGR(x) (x & 0x00000001)
#define GETFTR_CANBEHERE(x) (x & 0x00000002)
#define GETFTR_SOUND(x)     (x & 0x0000000C)
#define GETFTR_SHOWPIC(x)   (x & 0x00000010)

enum {
	GFX_STOPUPDATE = 0x01,
	GFX_VIEWUPDATED = 0x02,
	GFX_NOUPDATE = 0x04,
	GFX_HIDDEN = 0x08,
	GFX_FIXEDPRIO=0x10,
	GFX_ALWAYSUPDATE = 0x20,
	GFX_FORCEUPDATE = 0x40,
	GFX_REMOVEVIEW = 0x80
};

} // End of namespace Sci

#endif
