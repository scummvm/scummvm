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

#ifndef GOT_VIEWS_GAME_STATUS_H
#define GOT_VIEWS_GAME_STATUS_H

#include "got/views/view.h"

namespace Got {
namespace Views {

class GameStatus : public View {
private:
	int _scoreCountdown = 0;
	Common::String _endMessage;

	void displayHealth(GfxSurface &s);
	void displayMagic(GfxSurface &s);
	void displayJewels(GfxSurface &s);
	void displayScore(GfxSurface &s);
	void displayKeys(GfxSurface &s);
	void displayItem(GfxSurface &s);

public:
	GameStatus() : View("GameStatus") {}
	virtual ~GameStatus() {}

	void draw() override;
	bool msgGame(const GameMessage &msg) override;
	bool tick() override;
};

} // namespace Views
} // namespace Got

#endif
