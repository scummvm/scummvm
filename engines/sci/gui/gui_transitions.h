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

struct GuiTransitionTranslateEntry {
	int16 oldId;
	int16 realId;
	bool blackoutFlag;
};

enum {
	SCI_TRANSITIONS_VERTICALROLLFROMCENTER		= 0,
	SCI_TRANSITIONS_HORIZONTALROLLFROMCENTER	= 1,
	SCI_TRANSITIONS_BLOCKS						= 8,
	SCI_TRANSITIONS_PIXELATION					= 9,
	SCI_TRANSITIONS_FADEPALETTE					= 10,
	SCI_TRANSITIONS_SCROLLRIGHT					= 11,
	SCI_TRANSITIONS_SCROLLLEFT					= 12,
	SCI_TRANSITIONS_SCROLLUP					= 13,
	SCI_TRANSITIONS_SCROLLDOWN					= 14,
	SCI_TRANSITIONS_NONE						= 100,
	// here are transitions that are used by the old tableset, but are not included anymore in the new tableset
	SCI_TRANSITIONS_VERTICALROLLTOCENTER		= 300,
	SCI_TRANSITIONS_HORIZONTALROLLTOCENTER		= 301
};

class SciGuiScreen;
class SciGuiTransitions {
public:
	SciGuiTransitions(SciGui *gui, SciGuiScreen *screen, SciGuiPalette *palette, bool isVGA);
	~SciGuiTransitions();

	void setup(int16 number, bool blackoutFlag);
	void doit(Common::Rect picRect);

private:
	void init(void);
	void setNewPalette();
	void setNewScreen();
	void fadeOut();
	void fadeIn();
	void pixelation();
	void blocks();
	void scroll();
	void verticalRollFromCenter();
	void verticalRollToCenter();
	void horizontalRollFromCenter();
	void horizontalRollToCenter();

	SciGui *_gui;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;

	bool _isVGA;
	const GuiTransitionTranslateEntry *_translationTable;
	int16 _number;
	bool _blackoutFlag;
	Common::Rect _picRect;
	byte *_oldScreen; // buffer for saving current active screen data to, has dimenions of _screen->_displayScreen
};

} // End of namespace Sci

#endif
