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

#include "common/endian.h"	// for READ_LE_UINT16
#include "common/rect.h"
#include "sci/engine/vm_types.h"

namespace Sci {

typedef int GuiResourceId; // is a resource-number and -1 means no parameter given
typedef reg_t GuiMemoryHandle;
typedef int16 GuiViewLoopNo;
typedef int16 GuiViewCelNo;

struct GuiPort {
	uint16 id;
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	GuiResourceId fontId;
	int16 textFace, penClr, backClr;
	int16 penMode;

	GuiPort(uint16 theId) : id(theId), top(0), left(0),
		curTop(0), curLeft(0),
		fontHeight(0), fontId(0), textFace(0),
		penClr(0), backClr(0xFF), penMode(0) {
	}
};

struct GuiWindow : public GuiPort {
	Common::Rect dims; // client area of window
	Common::Rect restoreRect; // total area of window including borders
	uint16 wndStyle;
	uint16 uSaveFlag;
	reg_t hSaved1;
	reg_t hSaved2;
	Common::String title;
	bool bDrawn;

	GuiWindow(uint16 theId) : GuiPort(theId),
		wndStyle(0), uSaveFlag(0),
		hSaved1(NULL_REG), hSaved2(NULL_REG),
		bDrawn(false) {
	}
};

struct GuiCast {
	uint16 view;
	uint16 loop;
	uint16 cel;
	uint16 z;
	uint16 pal;
	Common::Rect rect;
};

struct GuiColor {
	byte used;
	byte r, g, b;
};

struct GuiPalette {
	byte mapping[256];
	uint32 timestamp;
	GuiColor colors[256];
	byte intensity[256];
};

struct GuiPalSchedule {
	byte from;
	uint32 schedule;
};

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

#define SCI_PAL_FORMAT_CONSTANT 1
#define SCI_PAL_FORMAT_VARIABLE 0

static inline void CreatePaletteFromData(byte *data, GuiPalette *paletteOut) {
	int palFormat = 0;
	int palOffset = 0;
	int palColorStart = 0;
	int palColorCount = 0;
	int colorNo = 0;

	memset(paletteOut, 0, sizeof(GuiPalette));
	// Setup default mapping
	for (colorNo = 0; colorNo < 256; colorNo++) {
		paletteOut->mapping[colorNo] = colorNo;
	}
	if (data[0] == 0 && data[1] == 1) {
		// SCI0/SCI1 palette
		palFormat = SCI_PAL_FORMAT_VARIABLE; // CONSTANT;
		palOffset = 260;
		palColorStart = 0; palColorCount = 256;
		//memcpy(&paletteOut->mapping, data, 256);
	} else {
		// SCI1.1 palette
		palFormat = data[32];
		palOffset = 37;
		palColorStart = READ_LE_UINT16(data + 25); palColorCount = READ_LE_UINT16(data + 29);
	}
	switch (palFormat) {
		case SCI_PAL_FORMAT_CONSTANT:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = 1;
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
		case SCI_PAL_FORMAT_VARIABLE:
			for (colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
				paletteOut->colors[colorNo].used = data[palOffset++];
				paletteOut->colors[colorNo].r = data[palOffset++];
				paletteOut->colors[colorNo].g = data[palOffset++];
				paletteOut->colors[colorNo].b = data[palOffset++];
			}
			break;
	}
}

} // End of namespace Sci

#endif
