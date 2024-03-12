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
private:
	Graphics::ManagedSurface _circles[2];

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

public:
	MainMenu() : ScrollView("MainMenu") {}
	virtual ~MainMenu() {}

	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
