/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA3_VIEWS_WINDOW_VIEW_H
#define ULTIMA3_VIEWS_WINDOW_VIEW_H

#include "ultima/shared/engine/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

/**
 * Base for the party management screens, which are all drawn inside the
 * bordered window at the bottom of the display, over whatever the title
 * screen left above it
 */
class WindowView : public Shared::View {
protected:
	/**
	 * Blanks the inside of the window
	 */
	static void clearWindow(GfxSurface &s);

	/**
	 * Draws the magenta window border
	 */
	static void drawBorder(GfxSurface &s);

	/**
	 * Draws the "Press <Space>" label on the bottom border
	 */
	static void drawSpacePrompt(GfxSurface &s);

	/**
	 * Returns true if the keypress is the space bar
	 */
	static bool isSpaceKey(const KeypressMessage &msg);

public:
	WindowView(const Common::String &name) : View(name) {}
	~WindowView() override {}
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
