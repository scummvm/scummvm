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

#ifndef SCI_GUI_CURSOR_H
#define SCI_GUI_CURSOR_H

#include "sci/gui/gui.h"

namespace Sci {

#define SCI_CURSOR_SCI0_HEIGHTWIDTH 16
#define SCI_CURSOR_SCI0_RESOURCESIZE 68

#define SCI_CURSOR_SCI0_TRANSPARENCYCOLOR 1

class SciGuiView;
class SciGuiPalette;
class SciGuiCursor {
public:
	SciGuiCursor(ResourceManager *resMan, SciGuiPalette *palette);
	~SciGuiCursor();

	void show();
	void hide();
	void setShape(GuiResourceId resourceId);
	void setPosition(Common::Point pos);
	Common::Point getPosition();
	void refreshPosition();

	/**
	 * Limits the mouse movement to a given rectangle.
	 *
	 * @param[in] rect	The rectangle
	 */
	void setMoveZone(Common::Rect zone) { _moveZone = zone; }

private:
	ResourceManager *_resMan;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;

	byte *_rawBitmap;
	Common::Rect _moveZone; // Rectangle in which the pointer can move
};

} // End of namespace Sci

#endif
