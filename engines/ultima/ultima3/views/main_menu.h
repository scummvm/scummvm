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

#ifndef ULTIMA3_VIEWS_MAIN_MENU_H
#define ULTIMA3_VIEWS_MAIN_MENU_H

#include "graphics/views/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

/**
 * The main menu: Return to the View / Organize a Party / Journey Onward.
 * Like the original, it is drawn over whatever the title screen left on the
 * display. Pressing R, O, J or Escape only echoes the choice on the Option
 * line; it must then be confirmed with Enter, or taken back with Backspace
 * or Left. The confirmed options are wired up as the roster/party data
 * model and the world engine land in later stages.
 */
class MainMenu : public View {
private:
	enum State {
		CHOOSING,
		CONFIRMING
	};

	State _state = CHOOSING;
	int _choice = 0;

	void drawBorder(GfxSurface &s);

public:
	MainMenu() : View("MainMenu") {}
	~MainMenu() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
