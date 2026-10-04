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

#ifndef ULTIMA3_VIEWS_GAME_H
#define ULTIMA3_VIEWS_GAME_H

#include "graphics/views/view.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

using namespace Graphics::Views;

constexpr int FRAME_COLS = 40;

/**
 * Base for the in-game screens. It draws the frame around the 11x11 tile
 * viewport on the left, the party's status panels and the scrolling message
 * window on the right, and runs the animation that goes on while waiting
 * for a command. Derived views supply the contents of the viewport.
 */
class Game : public View {
private:
	int _idleFrames = 0;
	int _windCounter = 1;

	void drawFrame(GfxSurface &s);
	void drawPartyStatus(GfxSurface &s);
	void drawWind(GfxSurface &s);
	void invertRect(GfxSurface &s, const Common::Rect &r);

	/**
	 * Every so often picks a new wind direction
	 */
	void updateWind();

protected:
	bool _gameOver = false;

	/**
	 * Draws the 11x11 tile viewport
	 */
	virtual void drawViewport(GfxSurface &s) = 0;

	/**
	 * Called when no command has been given for several seconds
	 */
	virtual void idleTimeout() = 0;

	/**
	 * Resets the state for a new game
	 */
	void startGame();

	/**
	 * Prints the command prompt and restarts the wait for a command
	 */
	void startPrompt();

	/**
	 * Ends the game if every party member is dead, showing why
	 * @returns		True if the game is over
	 */
	bool checkPartyWipedOut();

public:
	Game(const Common::String &name) : View(name) {}
	~Game() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	void timeout() override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
