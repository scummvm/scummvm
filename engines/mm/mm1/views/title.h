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

#ifndef MM1_VIEWS_TITLE_H
#define MM1_VIEWS_TITLE_H

#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace Views {

#define SCREENS_COUNT 10

class Title : public UIElement {
private:
	Graphics::ManagedSurface _screens[SCREENS_COUNT];
	int _screenNum = -1;
	int _fadeIndex = 0;

	/**
	 * Starts the slideshow of game scenes
	 */
	void startSlideshow();
public:
	Title();
	virtual ~Title() {}

	/**
	 * Called when the screen is displayed
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Called when the screen is hidden
	 */
	bool msgUnfocus(const UnfocusMessage &msg) override;

	/**
	 * Draw the screen
	 */
	void draw() override;

	/**
	 * Delay timeout
	 */
	void timeout() override;

	/**
	 * Handles keypresses
	 */
	bool msgKeypress(const KeypressMessage &msg) override;

	/**
	 * Handle actions
	 */
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
