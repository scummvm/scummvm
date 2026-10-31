
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

// Same generic dialog box geometry the Lose and Win views use
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

// Indexed by Map::_castleNum (0-7)
static const char *const PRINCESS_NAMES[8] = {
	"Julia", "Cassandra", "Marsha", "Donna", "Kristen", "Beth", "Lori", "Dianne"
};

PrincessSaved::PrincessSaved() : Interaction("PrincessSaved") {
}

bool PrincessSaved::msgFocus(const FocusMessage &msg) {
	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	auto &sg = _G(savegame);

	// Whether the player has already done enough to be ready to win
	_readyForTimeTravel = sg._enemyVessels >= 20 && sg._experience >= 7000;

	sg._coins += 500;
	sg._hits += 500;
	sg._experience += 500;
	g_engine->findView("Stats")->draw();

	if (_readyForTimeTravel && sg._transports[Data::TRANSPORT_TIME_MACHINE] < 9) {
		++sg._transports[Data::TRANSPORT_TIME_MACHINE];
		spawnTimeMachine();
	}

	g_engine->send("Commands", GameMessage("SPACE2"));
	return true;
}

void PrincessSaved::draw() {
	// Without the time travel message the text sits a little lower
	int y = _readyForTimeTravel ? 0 : 2;

	auto s = getSurface();
	s.fillRect(Common::Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Common::Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), _G(colors)._border);
	s.setColor(_G(colors)._text);

	s.writeString(Common::Point(5, y + 5), " Thou hast saved the princess");
	s.writeString(Common::Point(5, y + 6), Common::String::format("%s.  She giveth thee",
		PRINCESS_NAMES[_G(map)._castleNum]));
	s.writeString(Common::Point(8, y + 7), "500 hit points, pence and");
	s.writeString(Common::Point(12, y + 8), "experience points.");

	if (_readyForTimeTravel) {
		s.writeString(Common::Point(5, 10), "  She informs thee that thou");
		s.writeString(Common::Point(5, 11), "art now ready for time travel,");
		s.writeString(Common::Point(5, 12), "    and that there is a time");
		s.writeString(Common::Point(5, 13), " machine far to the northwest!");
	}
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

		if (_G(map).getOverworldTileAt(x, y) == Data::TILE_WOODS) {
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
	// Switch back to the overworld map
	_G(map).load(Data::MAP_OVERWORLD);
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
