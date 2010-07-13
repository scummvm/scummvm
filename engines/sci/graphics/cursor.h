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

#ifndef SCI_GRAPHICS_CURSOR_H
#define SCI_GRAPHICS_CURSOR_H

#include "common/hashmap.h"

namespace Sci {

#define SCI_CURSOR_SCI0_HEIGHTWIDTH 16
#define SCI_CURSOR_SCI0_RESOURCESIZE 68

#define SCI_CURSOR_SCI0_TRANSPARENCYCOLOR 1

class GfxView;
class GfxPalette;

typedef Common::HashMap<int, GfxView *> CursorCache;

class GfxCursor {
public:
	GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen);
	~GfxCursor();

	void init(GfxCoordAdjuster *coordAdjuster, EventManager *event);

	void kernelShow();
	void kernelHide();
	bool isVisible();
	void kernelSetShape(GuiResourceId resourceId);
	void kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot);
	void kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot);
	void setPosition(Common::Point pos);
	Common::Point getPosition();
	void refreshPosition();

	/**
	 * Limits the mouse movement to a given rectangle.
	 *
	 * @param[in] rect	The rectangle
	 */
	void kernelSetMoveZone(Common::Rect zone);

	void kernelSetPos(Common::Point pos);
	void kernelMoveCursor(Common::Point pos);

private:
	void purgeCache();

	ResourceManager *_resMan;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCoordAdjuster *_coordAdjuster;
	EventManager *_event;

	int _upscaledHires;

	Common::Rect _moveZone; // Rectangle in which the pointer can move

	CursorCache _cachedCursors;

	bool _isVisible;
};

} // End of namespace Sci

#endif
