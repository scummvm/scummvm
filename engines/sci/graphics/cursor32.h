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

#ifndef SCI_GRAPHICS_CURSOR32_H
#define SCI_GRAPHICS_CURSOR32_H

#include "common/array.h"           // for Array
#include "common/rect.h"            // for Point, Rect
#include "common/scummsys.h"        // for int16, byte, uint8
#include "common/serializer.h"      // for Serializable, Serializer (ptr only)
#include "sci/graphics/celobj32.h"  // for CelInfo32
#include "sci/graphics/helpers.h"   // for GuiResourceId

namespace Sci {

class GfxCursor32 : public Common::Serializable {
public:
	GfxCursor32();
	~GfxCursor32() override;

	/**
	 * Initialises the cursor system with the given buffer to use as the output
	 * buffer for rendering the cursor.
	 */
	void init(const Buffer &outputBuffer);

	/**
	 * Called when the hardware mouse moves.
	 */
	bool deviceMoved(Common::Point &position);

	/**
	 * Called by GfxFrameout once for each show rectangle that is going to be
	 * drawn to hardware.
	 */
	virtual void gonnaPaint(Common::Rect paintRect);

	/**
	 * Called by GfxFrameout when the rendering to hardware begins.
	 */
	virtual void paintStarting();

	/**
	 * Called by GfxFrameout when the output buffer has finished rendering to
	 * hardware.
	 */
	virtual void donePainting();

	/**
	 * Hides the cursor. Each call to `hide` will increment a hide counter,
	 * which must be returned to 0 before the cursor will be shown again.
	 */
	virtual void hide();

	/**
	 * Shows the cursor, if the hide counter is returned to 0.
	 */
	virtual void unhide();

	/**
	 * Shows the cursor regardless of the state of the hide counter.
	 */
	virtual void show();

	/**
	 * Sets the view used to render the cursor.
	 */
	virtual void setView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo);

	/**
	 * Explicitly sets the position of the cursor, in game script coordinates.
	 */
	void setPosition(const Common::Point &position);

	/**
	 * Sets the region that the mouse is allowed to move within.
	 */
	void setRestrictedArea(const Common::Rect &rect);

	/**
	 * Removes restrictions on mouse movement.
	 */
	void clearRestrictedArea();

	void saveLoadWithSerializer(Common::Serializer &ser) override;

protected:
	/**
	 * Information about the current cursor. Used to restore cursor when loading
	 * a savegame.
	 */
	CelInfo32 _cursorInfo;

	/**
	 * The number of times the cursor has been hidden.
	 */
	int _hideCount;

private:
	struct DrawRegion {
		Common::Rect rect;
		byte *data;
		uint8 skipColor;

		DrawRegion() : data(nullptr) {}
	};

	/**
	 * The content of the frame buffer which was behind the cursor prior to its
	 * being drawn.
	 */
	DrawRegion _cursorBack;

	/**
	 * Scratch buffer.
	 */
	DrawRegion _scratch1;

	/**
	 * Scratch buffer 2.
	 */
	DrawRegion _scratch2;

	/**
	 * A draw region representing the entire output buffer.
	 */
	DrawRegion _screenRegion;

	/**
	 * The region behind the cursor immediately before it is painted to the
	 * output buffer.
	 */
	DrawRegion _savedScreenRegion;

	/**
	 * The cursor bitmap.
	 */
	DrawRegion _cursor;

	/**
	 * The width and height of the cursor, in screen coordinates.
	 */
	int16 _width, _height;

	/**
	 * The output buffer where the cursor is rendered.
	 */
	Buffer _screen;

	/**
	 * The rendered position of the cursor, in screen coordinates.
	 */
	Common::Point _position;

	/**
	 * The position of the cursor hot spot, relative to the cursor origin, in
	 * screen pixels.
	 */
	Common::Point _hotSpot;

	/**
	 * The area within which the cursor is allowed to move, in screen pixels.
	 */
	Common::Rect _restrictedArea;

	/**
	 * Indicates whether or not the cursor needs to be repainted on the output
	 * buffer due to a change of graphics in the area underneath the cursor.
	 */
	bool _needsPaint;

	/**
	 * Reads data from the output buffer to the given draw region.
	 */
	void copyFromScreen(DrawRegion &target);

	/**
	 * Copies pixel data from the given source to the given target. If SKIP is
	 * true, pixels that match the `skipColor` property of the source will be
	 * skipped.
	 *
	 * @note In SSCI, the function that did not handle skip color was called
	 * `copy` and the one that did was called `paint`.
	 */
	template <bool SKIP>
	void copy(DrawRegion &target, const DrawRegion &source);

	/**
	 * Draws the cursor to the position it was drawn to prior to moving
	 * offscreen or being hidden by a call to `hide`.
	 */
	void revealCursor();

	/**
	 * Draws the given source to the output buffer.
	 */
	void drawToScreen(const DrawRegion &source);

	/**
	 * Renders the cursor at its new location.
	 */
	virtual void move();

public:
	virtual void setMacCursorRemapList(int cursorCount, reg_t *cursors) {}
};

} // End of namespace Sci
#endif
