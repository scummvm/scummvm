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

#ifndef ULTIMA1_VIEWS_GAME_H
#define ULTIMA1_VIEWS_GAME_H

#include "ultima/ultima1/views/dialog.h"
#include "ultima/ultima1/views/commands.h"
#include "ultima/ultima1/views/stats.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

/**
 * The main in-game screen. Draws the outer frame, and contains the shared
 * Commands area and Stats display. Only the actual map/dungeon area changes.
 */
class Game : public Dialog {
private:
	Commands _commands;
	Stats _stats;
	UIElement *_map = nullptr;

public:
	Game();
	~Game() override;

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
