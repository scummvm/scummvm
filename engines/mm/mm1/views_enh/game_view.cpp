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

#include "mm/mm1/views_enh/game_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define TICKS_PER_FRAME 4

struct LocationEntry {
	const char *const _prefix;
	uint _count;
	uint _frameCount;
};

static const LocationEntry LOCATIONS[5] = {
	{ "trng", 2, 16 },
	{ "gild", 4, 32 },	// Xeen guild anim used for market
	{ "tmpl", 4, 26 },
	{ "blck", 2, 13 },
	{ "tvrn", 2, 16 }
};

bool GameView::msgGame(const GameMessage &msg) {
	if (msg._name == "LOCATION") {
		showLocation(msg._value);

	} else if (msg._name == "LOCATION_DRAW") {
		UIElement *view = g_events->findView("Game");
		view->draw();

	} else {
		return Views::GameView::msgGame(msg);
	}

	return true;
}

void GameView::showLocation(int locationId) {
	if (locationId == -1) {
		_backgrounds.clear();
		_frameCount = 0;
		_locationId = -1;
	} else {
		assert(LOCATIONS[locationId]._prefix);
		_locationId = locationId;
		_frameIndex = _timerCtr = 0;
		_frameCount = LOCATIONS[locationId]._frameCount;

		_backgrounds.resize(LOCATIONS[locationId]._count);
		for (uint i = 0; i < _backgrounds.size(); ++i) {
			Common::String name = Common::String::format("%s%d.twn",
				LOCATIONS[locationId]._prefix, i + 1);
			_backgrounds[i].load(name);
		}
	}
}

void GameView::draw() {
	if (_locationId == -1) {
		Views::GameView::draw();

	} else {
		Graphics::ManagedSurface s = getSurface();
		_backgrounds[_frameIndex / 8].draw(&s, _frameIndex % 8,
			Common::Point(0, 0));
	}
}

bool GameView::tick() {
	if (_locationId != -1) {
		if (++_timerCtr >= TICKS_PER_FRAME) {
			_timerCtr = 0;
			_frameIndex = (_frameIndex + 1) % _frameCount;
		}

		redraw();
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
