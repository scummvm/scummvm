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

#ifndef ULTIMA3_VIEWS_LOGO_SCREEN_H
#define ULTIMA3_VIEWS_LOGO_SCREEN_H

#include "ultima/ultima3/views/window_view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int DEMO_COLS = 19;
constexpr int DEMO_ROWS = 6;
constexpr int MOVES_COUNT = 0x200;

/**
 * The attract-mode screen shown below the title art: a small animated view
 * of the world, whose tiles are changed over time by a recorded script of
 * edits. Any key opens the main menu.
 */
class LogoScreen : public WindowView {
private:
	byte _demo[DEMO_COLS * DEMO_ROWS] = {};
	// The first half holds the tile each edit changes, and the second its new
	// value. An edit with a tile of 0xFF is a pause of that many rounds
	byte _moves[MOVES_COUNT * 2] = {};
	int _movePos = MOVES_COUNT - 1;
	int _roundsLeft = 0;

	void fetchMove();

public:
	LogoScreen() : WindowView("LogoScreen") {}
	~LogoScreen() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	void timeout() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
