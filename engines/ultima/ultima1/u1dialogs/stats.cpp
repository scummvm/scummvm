/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/u1dialogs/stats.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Stats, FullScreenDialog)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()


bool Stats::ShowMsg(CShowMsg &msg) {
	addInfoMsg(_game->_res->PRESS_SPACE_TO_CONTINUE, false);
	getKeypress();
	return true;
}

bool Stats::CharacterInputMsg(CCharacterInputMsg &msg) {
	if ((_startingIndex + 26U) < _stats.size()) {
		_startingIndex += 26U;
		setDirty();
		getKeypress();
	} else {
		addInfoMsg("", false, true);
		hide();
	}

	return true;
}

/**
 * Counts the number of a given transport type
 */
template<class T>
void countTransport(Maps::MapOverworld *overworldMap, Common::Array<Stats::StatEntry> &stats, const char *name, byte textColor) {
	// Count the number of transports that are of the given type
	uint total = 0;
	for (uint idx = 0; idx < overworldMap->_widgets.size(); ++idx) {
		if (dynamic_cast<T *>(overworldMap->_widgets[idx].get()))
			++total;
	}

	if (total > 0)
		stats.push_back(Stats::StatEntry(Stats::formatStat(name, total), textColor));
}

void Stats::load() {
	const Shared::Character &c = *_game->_party;
	Maps::MapOverworld *overworld = getMap()->getOverworldMap();

	// Basic attributes
	const uint basicAttributes[7] = { c._hitPoints,c._strength, c._agility, c._stamina, c._charisma,c._wisdom, c._intelligence };
	addStats(_game->_res->STAT_NAMES, basicAttributes, 0, 6);

	// Money line(s)
	if (c._coins % 10)
		_stats.push_back(StatEntry(formatStat(_game->_res->STAT_NAMES[7], c._coins % 10), _game->_textColor));
	if ((c._coins % 100) >= 10)
		_stats.push_back(StatEntry(formatStat(_game->_res->STAT_NAMES[8], (c._coins / 10) % 10), _game->_textColor));
	if (c._coins >= 100)
		_stats.push_back(StatEntry(formatStat(_game->_res->STAT_NAMES[9], c._coins / 100), _game->_textColor));

	// Enemy vessels
	uint enemyVessels = overworld->getEnemyVesselCount();
	if (enemyVessels != 0)
		_stats.push_back(StatEntry(_game->_res->STAT_NAMES[9], enemyVessels));

	// Armor, weapons, & spells
	for (uint idx = 1; idx < c._armour.size(); ++idx) {
		if (!c._armour[idx]->empty())
			_stats.push_back(StatEntry(formatStat(c._armour[idx]->_name.c_str(), c._armour[idx]->_quantity),
				(int)idx == c._equippedArmour ? _game->_highlightColor : _game->_textColor));
	}
	for (uint idx = 1; idx < c._weapons.size(); ++idx) {
		if (!c._weapons[idx]->empty())
			_stats.push_back(StatEntry(formatStat(c._weapons[idx]->_longName.c_str(), c._weapons[idx]->_quantity),
				(int)idx == c._equippedWeapon ? _game->_highlightColor : _game->_textColor));
	}
	for (uint idx = 1; idx < c._spells.size(); ++idx) {
		if (!c._spells[idx]->empty())
			_stats.push_back(StatEntry(formatStat(c._spells[idx]->_name.c_str(), c._spells[idx]->_quantity),
				(int)idx == c._equippedSpell ? _game->_highlightColor : _game->_textColor));
	}

	// Counts of transport types
	countTransport<Widgets::Horse>(overworld, _stats, _game->_res->TRANSPORT_NAMES[1], _game->_textColor);
	countTransport<Widgets::Cart>(overworld, _stats, _game->_res->TRANSPORT_NAMES[2], _game->_textColor);
	countTransport<Widgets::Raft>(overworld, _stats, _game->_res->TRANSPORT_NAMES[3], _game->_textColor);
	countTransport<Widgets::Frigate>(overworld, _stats, _game->_res->TRANSPORT_NAMES[4], _game->_textColor);
	countTransport<Widgets::Aircar>(overworld, _stats, _game->_res->TRANSPORT_NAMES[5], _game->_textColor);
	countTransport<Widgets::Shuttle>(overworld, _stats, _game->_res->TRANSPORT_NAMES[6], _game->_textColor);
	countTransport<Widgets::TimeMachine>(overworld, _stats, _game->_res->TRANSPORT_NAMES[7], _game->_textColor);

	// Add entries for any gems
	addStats(_game->_res->GEM_NAMES, _game->_gems, 0, 3);
}

void Stats::addStats(const char *const *names, const uint *values, int start, int end, int equippedIndex) {
	for (int idx = start; idx <= end; ++idx) {
		if (values[idx]) {
			Common::String line = formatStat(names[idx], values[idx]);
			_stats.push_back(StatEntry(line, (idx == equippedIndex) ? _game->_highlightColor : _game->_textColor));
		}
	}
}

Common::String Stats::formatStat(const char *name, uint value) {
	Common::String line(name);
	Common::String val = Common::String::format("%u", value);
	while ((line.size() + val.size()) < 17)
		line += '.';

	return line + val;
}

void Stats::draw() {
	Dialog::draw();

	drawFrame(_game->_res->INVENTORY);
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;

	// Player name and description
	s.writeString(Common::String::format(_game->_res->PLAYER, c._name.c_str()),
		TextPoint(2, 2), _game->_edgeColor);
	s.writeString(Common::String::format(_game->_res->PLAYER_DESC, c.getLevel(),
		_game->_res->SEX_NAMES[c._sex], _game->_res->RACE_NAMES[c._race], _game->_res->CLASS_NAMES[c._class]),
		TextPoint(2, 3), _game->_edgeColor);

	// Display stats
	for (uint idx = 0; idx < MIN(26U, _stats.size() - _startingIndex); ++idx) {
		s.writeString(_stats[_startingIndex + idx]._line, TextPoint(idx >= 13 ? 21 : 2, (idx % 13) + 5), _stats[_startingIndex + idx]._color);
	}

	// Display a more sign if thare more than 26 remaining entries being displayed
	if ((_startingIndex + 26) < _stats.size()) {
		U1Gfx::DrawingSupport ds(s);
		ds.drawRightArrow(TextPoint(16, 19));
		s.writeString(_game->_res->MORE, TextPoint(17, 19));
		ds.drawLeftArrow(TextPoint(23, 19));
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
