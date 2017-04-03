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

class MacTextWindow : public MacWindow {
public:
	MacTextWindow(MacWindowManager *wm, const Font *font, int fgcolor,
		int bgcolor, int maxWidth, TextAlign textAlignment);
	~MacTextWindow();

	const Font *getTextWindowFont();

	void drawText(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff);
	void appendText(Common::String str, int fontId = kMacFontChicago, int fontSize = 12, int fontSlant = kMacFontRegular);

private:
	MacText *_mactext;
	const Font *_font;
};

} // End of namespace Graphics

#endif
