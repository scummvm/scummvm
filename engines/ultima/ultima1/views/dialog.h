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

#ifndef ULTIMA1_VIEWS_DIALOG_H
#define ULTIMA1_VIEWS_DIALOG_H

#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

/**
 * Base class for views with the thick beveled-corner frame drawn around the edge of the screen
 */
class Dialog : public Shared::Gfx::View {
protected:
	/**
	 * Draws the border frame around the edge of the screen
	 */
	void drawFrame();

	/**
	 * Draws the dividers splitting the interior of the frame into a map
	 * viewport (above) and a command/message log and stats panel (below).
	 * Shared by the overworld, dungeon, and outer space views
	 */
	void drawGameDividers();

	/**
	 * Draws a small triangular right-arrow glyph, in text-cell coordinates
	 */
	void drawRightArrow(const Common::Point &pt);

	/**
	 * Draws a small triangular left-arrow glyph, in text-cell coordinates
	 */
	void drawLeftArrow(const Common::Point &pt);

public:
	Dialog(const Common::String &name) : View(name) {}
	~Dialog() override {}
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
