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

#ifndef ULTIMA1_VIEWS_STARTUP_H
#define ULTIMA1_VIEWS_STARTUP_H

#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

/**
 * Startup splash screen - the game title and copyright
 */
class Startup : public Shared::Gfx::View {
private:
	void showTitle() {
		replaceView("Title");
	}
public:
	Startup() : View("Startup") {}
	~Startup() override {}

	void draw() override;

	bool msgFocus(const FocusMessage &msg) override;
	void timeout() override;
	bool msgKeypress(const KeypressMessage &msg) override {
		showTitle();
		return true;
	}
	bool msgMouseDown(const MouseDownMessage &msg) override {
		showTitle();
		return true;
	}
	bool msgAction(const ActionMessage &msg) override {
		showTitle();
		return true;
	}
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
