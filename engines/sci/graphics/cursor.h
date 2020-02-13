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

#ifndef SCI_GRAPHICS_CURSOR_H
#define SCI_GRAPHICS_CURSOR_H

#include "common/array.h"
#include "common/hashmap.h"
#include "sci/sci.h"
#include "sci/graphics/helpers.h"
#include "sci/util.h"

namespace Sci {

enum {
	SCI_CURSOR_SCI0_HEIGHTWIDTH = 16,
	SCI_CURSOR_SCI0_RESOURCESIZE = 68,
	SCI_CURSOR_SCI0_TRANSPARENCYCOLOR = 1
};

class GfxView;
class GfxPalette;

typedef Common::HashMap<int, GfxView *> CursorCache;

struct SciCursorSetPositionWorkarounds {
	SciGameId gameId;
	int16 newPositionY;
	int16 newPositionX;
	int16 rectTop;
	int16 rectLeft;
	int16 rectBottom;
	int16 rectRight;
};

class GfxCursor {
public:
	GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen, GfxCoordAdjuster16 *coordAdjuster, EventManager *eventMan);
	~GfxCursor();

	void kernelShow();
	void kernelHide();
	bool isVisible();
	void kernelSetShape(GuiResourceId resourceId);
	void kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot);
	void kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum);
	void setPosition(Common::Point pos);
	Common::Point getPosition();
	void refreshPosition();

	/**
	 * Removes limit for mouse movement
	 */
	void kernelResetMoveZone();

	/**
	 * Limits the mouse movement to a given rectangle.
	 *
	 * @param[in] rect	The rectangle
	 */
	void kernelSetMoveZone(Common::Rect zone);

	/**
	 * Creates a dynamic zoom cursor, that is used to zoom on specific parts of the screen,
	 * using a separate larger picture. This was only used by two SCI1.1 games, Laura Bow 2
	 * (for examining the glyphs), and Freddy Pharkas (for examining the prescription with
	 * the whisky glass).
	 *
	 * In the Mac version of Freddy Pharkas, this was removed completely, and the scene has
	 * been redesigned to work without this functionality. There was no version of LB2 for
	 * the Macintosh platform.
	 */
	void kernelSetZoomZone(byte multiplier, Common::Rect zone, GuiResourceId viewNum, int loopNum, int celNum, GuiResourceId picNum, byte zoomColor);
	void kernelClearZoomZone();

	void kernelSetPos(Common::Point pos);
	void kernelMoveCursor(Common::Point pos);

private:
	void purgeCache();

	ResourceManager *_resMan;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCoordAdjuster16 *_coordAdjuster;
	EventManager *_event;

	int _upscaledHires;

	bool _moveZoneActive;
	Common::Rect _moveZone; // Rectangle in which the pointer can move

	bool _zoomZoneActive;
	Common::Rect _zoomZone;
	GfxView *_zoomCursorView;
	byte _zoomCursorLoop;
	byte _zoomCursorCel;
	GfxView *_zoomPicView;
	byte _zoomColor;
	byte _zoomMultiplier;
	Common::SpanOwner<SciSpan<byte> > _cursorSurface;

	CursorCache _cachedCursors;

	bool _isVisible;

	// KQ6 Windows has different black and white cursors. If this is true (set
	// from the windows_cursors ini setting), then we use these and don't scale
	// them by 2x like the rest of the graphics, like SSCI did. These look very
	// ugly, which is why they aren't enabled by default.
	bool _useOriginalKQ6WinCursors;

	// The CD version of SQ4 contains a complete set of silver mouse cursors.
	// If this is true (set from the silver_cursors ini setting), then we use
	// these instead and replace the game's gold cursors with their silver
	// equivalents.
	bool _useSilverSQ4CDCursors;
};

} // End of namespace Sci

#endif
