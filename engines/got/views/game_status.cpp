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

#include "got/views/game_status.h"
#include "got/game/status.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define STAT_COLOR 206
#define SCORE_INTERVAL 5

void GameStatus::draw() {
	GfxSurface s = getSurface();

	// Draw the status background
	const Graphics::ManagedSurface &status = _G(status[0]);
	s.simpleBlitFrom(status);

	// Draw the elements
	displayHealth(s);
	displayMagic(s);
	displayJewels(s);
	displayScore(s);
	displayKeys(s);
	displayItem(s);
}

void GameStatus::displayHealth(GfxSurface &s) {
	const int x = 59 + _G(thor)->_health;

	s.fillRect(Common::Rect(59, 8, x, 12), 32);
	s.fillRect(Common::Rect(x, 8, 209, 12), STAT_COLOR);
}

void GameStatus::displayMagic(GfxSurface &s) {
	const int x = 59 + _G(thor_info)._magic;

	s.fillRect(Common::Rect(59, 20, x, 24), 96);
	s.fillRect(Common::Rect(x, 20, 209, 24), STAT_COLOR);
}

void GameStatus::displayJewels(GfxSurface &s) {
	const Common::String str = Common::String::format("%d", _G(thor_info)._jewels);
	int x;
	if (str.size() == 1)
		x = 70;
	else if (str.size() == 2)
		x = 66;
	else
		x = 62;

	s.fillRect(Common::Rect(59, 32, 85, 42), STAT_COLOR);
	s.print(Common::Point(x, 32), str, 14);
}

void GameStatus::displayScore(GfxSurface &s) {
	const Common::String str = Common::String::format("%ld", _G(thor_info)._score);
	const int x = 276 - (str.size() * 8);

	s.fillRect(Common::Rect(223, 32, 279, 42), STAT_COLOR);
	s.print(Common::Point(x, 32), str, 14);
}

void GameStatus::displayKeys(GfxSurface &s) {
	const Common::String str = Common::String::format("%d", _G(thor_info)._keys);

	int x;
	if (str.size() == 1)
		x = 150;
	else if (str.size() == 2)
		x = 146;
	else
		x = 142;

	s.fillRect(Common::Rect(139, 32, 164, 42), STAT_COLOR);
	s.print(Common::Point(x, 32), str, 14);
}

void GameStatus::displayItem(GfxSurface &s) {
	s.fillRect(Common::Rect(280, 8, 296, 24), STAT_COLOR);

	ThorInfo thorInfo = _G(thor_info);
	if (thorInfo._selectedItem) {
		if (thorInfo._selectedItem == 7)
			s.simpleBlitFrom(_G(objects[thorInfo._object + 10]), Common::Point(280, 8));
		else
			s.simpleBlitFrom(_G(objects[thorInfo._selectedItem + 25]), Common::Point(280, 8));
	}
}

bool GameStatus::msgGame(const GameMessage &msg) {
	if (msg._name == "FILL_SCORE") {
		_G(gameMode) = MODE_ADD_SCORE;
		_scoreCountdown = msg._value * SCORE_INTERVAL;
		_endMessage = msg._stringValue;
		return true;
	}

	return false;
}

bool GameStatus::tick() {
	if (_scoreCountdown > 0) {
		if ((_scoreCountdown % SCORE_INTERVAL) == 0) {
			_G(sound).playSound(WOOP, 1);
			addScore(1000);
		}

		if (--_scoreCountdown == 0) {
			_G(gameMode) = MODE_NORMAL;
			if (!_endMessage.empty())
				g_events->send(GameMessage(_endMessage));
		}
	}

	return false;
}

} // namespace Views
} // namespace Got
