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

#ifndef SCI_GRAPHICS_MACCURSOR32_H
#define SCI_GRAPHICS_MACCURSOR32_H

#include "common/array.h"           // for Array
#include "common/rect.h"            // for Point, Rect
#include "common/scummsys.h"        // for int16, byte, uint8
#include "sci/graphics/cursor32.h"  // for GfxCursor32
#include "sci/graphics/helpers.h"   // for GuiResourceId

namespace Sci {

/**
 * GfxMacCursor32 handles SCI32 games with native Mac cursor resources.
 *
 * GfxCursor32 was modeled after the PC SSCI structure in which cursors are
 * views that use the game's palette. It draws the cursor directly to the
 * game's frame. Mac cursors have their own individual palettes and are drawn 
 * independent of the rest of the game. GfxMacCursor32 uses CursorManager to
 * handle this, since it supports cursor palettes, and disables the GfxCursor32
 * functionality which CursorManager handles.
 */
class GfxMacCursor32 : public GfxCursor32 {
public:
	GfxMacCursor32() : GfxCursor32() {}

	/**
	 * Sets the remap list used to map views to native Mac cursor resources.
	 */
	void setMacCursorRemapList(int cursorCount, reg_t *cursors) override;

	/**
	 * Sets the view used to render the cursor.
	 */
	void setView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) override;

	/**
	 * Hides the cursor. Each call to `hide` will increment a hide counter,
	 * which must be returned to 0 before the cursor will be shown again.
	 */
	void hide() override;

	/**
	 * Shows the cursor, if the hide counter is returned to 0.
	 */
	void unhide() override;

	/**
	 * Shows the cursor regardless of the state of the hide counter.
	 */
	void show() override;

	/**
	 * Called by GfxFrameout, ignored since CursorManager handles Mac cursors.
	 */
	void gonnaPaint(Common::Rect paintRect) override {}

	/**
	 * Called by GfxFrameout, ignored since CursorManager handles Mac cursors.
	 */
	void paintStarting() override {}

	/**
	 * Called by GfxFrameout, ignored since CursorManager handles Mac cursors.
	 */
	void donePainting() override {}

private:
	/**
	 * Renders the cursor at its new location, but ignored since CursorManager
	 * handles Mac cursors.
	*/
	void move() override {}

	/**
	 * Remap list of views to native Mac cursor resources.
	 */
	Common::Array<uint16> _macCursorRemap;
};

} // End of namespace Sci

#endif
