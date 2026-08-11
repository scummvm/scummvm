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

#ifndef MM1_VIEWS_ENH_MAIN_MENU_H
#define MM1_VIEWS_ENH_MAIN_MENU_H

#include "mm/mm1/views_enh/scroll_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class MainMenu : public ScrollView {
	enum Choice {
		CHOICE_NONE,
		CHOICE_CREATE,
		CHOICE_VIEW,
		CHOICE_TOWN
	};
private:
	Graphics::ManagedSurface _circles[2];
	Choice _hoverChoice = CHOICE_NONE;

	/**
	 * Draws the two circles from the original game screens
	 */
	void drawCircles();

	/**
	 * Copies the circle
	 */
	void copyCircle(const Graphics::Surface *src, const Common::Point &destPos);

	/**
	 * Copies a line segment within the circle
	 */
	void drawCircleLine(const Graphics::Surface *src,
		Graphics::ManagedSurface *dest, int x, int y);

	/**
	 * Gets the choice at the given mouse position
	 */
	Choice getChoiceAt(const Common::Point &pos);

public:
	MainMenu() : ScrollView("MainMenu") {}
	virtual ~MainMenu() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
