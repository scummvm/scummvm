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

#ifndef SCI_GUI_TRANSITIONS_H
#define SCI_GUI_TRANSITIONS_H

#include "sci/gui/gui_helpers.h"

namespace Sci {

enum {
	SCI_TRANSITIONS_EGA_BLOCKS			= 8,
	SCI_TRANSITIONS_EGA_PIXELATION		= 18,
	SCI_TRANSITIONS_EGA_FADEPALETTE		= 30,
	SCI_TRANSITIONS_EGA_SCROLLRIGHT		= 40,
	SCI_TRANSITIONS_EGA_SCROLLLEFT		= 41,
	SCI_TRANSITIONS_EGA_SCROLLUP		= 42,
	SCI_TRANSITIONS_EGA_SCROLLDOWN		= 43
};

enum {
	SCI_TRANSITIONS_VGA_BLOCKS			= 8,
	SCI_TRANSITIONS_VGA_PIXELATION		= 9,
	SCI_TRANSITIONS_VGA_FADEPALETTE		= 10,
	SCI_TRANSITIONS_VGA_SCROLLRIGHT		= 11,
	SCI_TRANSITIONS_VGA_SCROLLLEFT		= 12,
	SCI_TRANSITIONS_VGA_SCROLLUP		= 13,
	SCI_TRANSITIONS_VGA_SCROLLDOWN		= 14
};

enum {
	SCI_TRANSITIONS_SCROLL_RIGHT		= 1,
	SCI_TRANSITIONS_SCROLL_LEFT			= 2,
	SCI_TRANSITIONS_SCROLL_UP			= 3
};

class SciGuiScreen;
class SciGuiTransitions {
public:
	SciGuiTransitions(SciGui *gui, SciGuiScreen *screen, SciGuiPalette *palette, bool isVGA);
	~SciGuiTransitions();

	void setup(int16 number);
	void doit(Common::Rect picRect);

private:
	void init(void);
	void setNewPalette();
	void setNewScreen();
	void fadeOut();
	void fadeIn();
	void pixelation();
	void blocks();
	void scroll(int16 direction);

	SciGui *_gui;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;

	bool _isVGA;
	int16 _number;
	Common::Rect _picRect;
	byte *_oldScreen; // buffer for saving current active screen data to, has dimenions of _screen->_displayScreen
};

} // End of namespace Sci

#endif
