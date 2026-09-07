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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_ENTERING_CRAFT_H
#define ULTIMA1_VIEWS_INTERACTIONS_ENTERING_CRAFT_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Shown after boarding the time machine with all four gems - a four page
 * instructional dialog ("Entering the craft, thou dost remark upon four
 * holes marked...") leading up to the trip through time. Unlike the other
 * interactions, this one draws its own thin bordered box that doesn't
 * cover the whole map area, matching the original's fillDialogArea/
 * drawDialogBorder. Space (or any keypress) advances through the four
 * pages; after the last page, closes and switches to the Mondain map
 */
class EnteringCraft : public Interaction {
private:
	int _page = 1;

	/**
	 * Draws the lines of text for the current page
	 */
	void drawPageText();

	/**
	 * Draws the four gems under the "R  G  B  W" heading, shown only on
	 * the first page
	 */
	void drawGemHoles();

	/**
	 * Draws a small solid diamond gem icon within an 8x8 text cell
	 */
	void drawGem(int cellX, int cellY, byte color);

	/**
	 * Shows the current page, and prompts to continue
	 */
	void showPage();

	/**
	 * Advances to the next page, or finishes once the last page is done
	 */
	void nextPage();

public:
	EnteringCraft() : Interaction("EnteringCraft") {
	}
	~EnteringCraft() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
