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

#ifndef GRAPHICS_MACGUI_MACWINDOWBORDER_H
#define GRAPHICS_MACGUI_MACWINDOWBORDER_H

#include "common/str.h"
#include "common/list.h"

#include "graphics/nine_patch.h"
#include "graphics/managed_surface.h"
#include "graphics/transparent_surface.h"

namespace Graphics {

struct BorderOffsets {
	int left;
	int right;
	int top;
	int bottom;
	int titleTop;
	int titleBottom;
	bool dark;
};

/**
 * A representation of a custom border, which allows for arbitrary border offsets
 * and nine-patch resizable displays for both active and inactive states.
 * However, the border offsets are the same for both active and inactive states.
 */
class MacWindowBorder {
public:
	MacWindowBorder();
	~MacWindowBorder();

	/**
	 * Accessor to check whether or not a border is loaded.
	 * @param active State that we want to check. If true it checks for active border, if false it checks for inactive.
	 * @return True if the checked state has a border loaded, false otherwise.
	 */
	bool hasBorder(bool active);

	/**
	 * Add the given surface as the display of the border in the active state.
	 * Will fail if there is already an active border.
	 * @param The surface that will be displayed.
	 */
	void addActiveBorder(TransparentSurface *source);

	/**
	 * Add the given surface as the display of the border in the inactive state.
	 * Will fail if there is already an inactive border.
	 * @param The surface that will be displayed.
	 */
	void addInactiveBorder(TransparentSurface *source);

	/**
	 * Accessor function for the custom offsets.
	 * @return True if custom offsets have been indicated (setOffsets has been called previously).
	 */
	bool hasOffsets();

	/**
	 * Mutator method to indicate the custom border offsets.
	 * These should be set to the desired thickness of each side of the border.
	 * e.g. For a border that is 10 pixels wide and 5 pixels tall, the call should be:
	 * setOffsets(10, 10, 5, 5)
	 * Note that this function does not check whether those borders form
	 * a valid rect when combined with the window dimensions.
	 * @param left Thickness (in pixels) of the left side of the border.
	 * @param right Thickness (in pixels) of the right side of the border.
	 * @param top Thickness (in pixels) of the top side of the border.
	 * @param bottom Thickness (in pixels) of the bottom side of the border.
	 */
	void setOffsets(int left, int right, int top, int bottom);
	void setOffsets(Common::Rect &rect);
	void setOffsets(const BorderOffsets &offsets);

	/**
	 * Accessor method to retrieve a given border.
	 * Note that it does not check for validity, and thus if setOffsets
	 * was not called before it might return garbage.
	 * @param offset The identifier of the offset wanted.
	 * @return The desired offset in pixels.
	 */
	BorderOffsets &getOffset();

	/**
	 * Blit the desired border (active or inactive) into a destination surface.
	 * It automatically resizes the border to fit the given surface.
	 * @param destination The surface we want to blit into.
	 * @param active True if we want to blit the active border, false otherwise.
	 * @param wm The window manager.
	 */
	void blitBorderInto(ManagedSurface &destination, bool active, MacWindowManager *wm);

private:

	NinePatchBitmap *_activeBorder;
	NinePatchBitmap *_inactiveBorder;

	bool _activeInitialized;
	bool _inactiveInitialized;

	BorderOffsets _borderOffsets;

};

} // End of namespace Graphics
#endif
