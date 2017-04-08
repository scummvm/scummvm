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

#ifndef GRAPHICS_MACGUI_MACTEXTWINDOW_H
#define GRAPHICS_MACGUI_MACTEXTWINDOW_H

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"

namespace Graphics {

struct SelectedText {
	int startX = 0, startY = 0;
	int endX = 0, endY = 0;

	bool needsRender() {
		return startX != endX || startY != endY;
	}
};

class MacTextWindow : public MacWindow {
public:
	MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor,
		int bgcolor, int maxWidth, TextAlign textAlignment);
	~MacTextWindow();

	const MacFont *getTextWindowFont();

	void drawText(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	void appendText(Common::String str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular);
	void appendText(Common::String str, const MacFont *macFont);
	void clearText();

	void setSelection(int selStartX, int selStartY, int selEndX, int selEndY);

private:
	MacText *_mactext;
	const MacFont *_font;

	SelectedText _selectedText;
};

} // End of namespace Graphics

#endif
