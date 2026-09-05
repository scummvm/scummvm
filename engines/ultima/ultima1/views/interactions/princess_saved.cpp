
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

#include "ultima/ultima1/views/interactions/princess_saved.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

// Indexed by Map::_castleNum (0-7)
static const char *const PRINCESS_NAMES[8] = {
	"Julia", "Cassandra", "Marsha", "Donna", "Kristen", "Beth", "Lori", "Dianne"
};

PrincessSaved::PrincessSaved() : Interaction("PrincessSaved") {
}

bool PrincessSaved::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(0, 0, 0, 0));
	auto &sg = _G(savegame);

	// Whether the player has already done enough to be ready to win, checked
	// before this rescue's reward is applied, matching the original
	bool readyForTimeTravel = sg._enemyVessels >= 20 && sg._experience >= 7000;

	writeString(Common::String::format("Thou hast saved the princess %s.  She giveth thee\n",
		PRINCESS_NAMES[_G(map)._castleNum]));
	writeString("500 hit points, pence and experience points.\n");

	sg._coins += 500;
	sg._hits += 500;
	sg._experience += 500;
	g_engine->findView("Stats")->draw();

	if (readyForTimeTravel) {
		writeString("\n");
		writeString("She informs thee that thou art now ready for time travel,\n");
		writeString("and that there is a time machine far to the northwest!\n");

		if (sg._transports[Data::TRANSPORT_TIME_MACHINE] < 9) {
			++sg._transports[Data::TRANSPORT_TIME_MACHINE];
			spawnTimeMachine();
		}
	}

	writeString("\n");
	showCursor();
	return true;
}

bool PrincessSaved::msgAction(const ActionMessage &msg) {
	finish();
	return true;
}

bool PrincessSaved::msgKeypress(const KeypressMessage &msg) {
	finish();
	return true;
}

void PrincessSaved::spawnTimeMachine() {
	static const int BASE_X[4] = { 0, 83, 0, 83 };
	static const int BASE_Y[4] = { 0, 0, 77, 77 };

	int continent = _G(map).getContinentAt(_G(savegame)._overworldPos);
	int baseX = BASE_X[continent];
	int baseY = BASE_Y[continent];

	for (int i = 0; i < 500; ++i) {
		int x = baseX + g_engine->getRandomNumber(12, 21);
		int y = baseY + g_engine->getRandomNumber(11, 15);

		if (_G(map).getTileAt(x, y) == Data::TILE_WOODS) {
			if (_G(overworldEntityCount) < Data::OVERWORLD_ENTITY_COUNT - 1) {
				++_G(overworldEntityCount);
				auto &e = _G(savegame)._overworldEntities[_G(overworldEntityCount)];
				e._type = Data::TILE_TIME_MACHINE;
				e._data = Data::TILE_WOODS;
				e._x = x;
				e._y = y;
				e._hits = 0;
			}
			break;
		}
	}
}

void PrincessSaved::finish() {
	// Clear out the Castle logic handler and load the overworld map
	delete _G(logic);
	_G(logic) = nullptr;
	_G(map).load(Data::MAP_OVERWORLD);
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
