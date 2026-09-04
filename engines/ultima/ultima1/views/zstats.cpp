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

#include "ultima/ultima1/views/zstats.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int COLOR_TEXT = 11;      // light cyan - normal values
constexpr int COLOR_HIGHLIGHT = 12; // light red - currently equipped/readied item

// Two columns of 13 rows each per page
constexpr int LINES_PER_PAGE = 26;

bool ZStats::msgFocus(const FocusMessage &msg) {
	g_engine->send("Commands", GameMessage("TEXT", "Press Space to continue: "));
	g_engine->send("Commands", GameMessage("SHOW_CURSOR"));

	buildLines();
	_pageStart = 0;
	return Dialog::msgFocus(msg);
}

bool ZStats::msgUnfocus(const UnfocusMessage &msg) {
	// Switch back to an action prompt in the commands area
	g_engine->send("Commands", GameMessage("PROMPT"));
	return Dialog::msgUnfocus(msg);
}

void ZStats::draw() {
	const auto &sg = _G(savegame);
	auto s = getSurface();
	Dialog::draw();

	s.writeString(Point(13, 0), "\x10 Inventory \x11");
	s.writeString(Point(2, 2), "Player: %s", sg._name);
	s.writeString(Point(2, 3), "A Level %d %s %s %s", sg.getCurrentLevel(), Data::SEX_NAMES[sg._sex],
		Data::RACE_NAMES[sg._race], Data::CLASS_NAMES[sg._class]);

	uint pageEnd = MIN(_pageStart + (uint)LINES_PER_PAGE, (uint)_lines.size());
	for (uint idx = _pageStart; idx < pageEnd; ++idx) {
		const StatLine &line = _lines[idx];

		s.setColor(line._color);
		s.writeString(linePos(idx - _pageStart), line._text);
	}

	// If there's more to show on a following page, prompt for it
	if (pageEnd < _lines.size()) {
		s.setColor(COLOR_TEXT);
		s.writeString(Point(16, 19), "\x10 More \x11");
	}
}

Common::Point ZStats::linePos(int lineNum) const {
	int col = (lineNum > 12) ? 1 : 0;
	return Common::Point(col * 19 + 2, (lineNum % 13) + 5);
}

void ZStats::addLine(const char *name, int value, bool highlighted) {
	if (!value)
		return;

	StatLine line;
	line._text = getLine(name, value);
	line._color = highlighted ? COLOR_HIGHLIGHT : COLOR_TEXT;
	_lines.push_back(line);
}

void ZStats::buildLines() {
	const auto &sg = _G(savegame);
	_lines.clear();

	addLine("Hit Points", sg._hits);
	addLine("Strength", sg._strength);
	addLine("Agility", sg._agility);
	addLine("Stamina", sg._stamina);
	addLine("Charisma", sg._charisma);
	addLine("Wisdom", sg._wisdom);
	addLine("Intelligence", sg._intelligence);

	// Coin breakdown - only shown for the denominations actually present
	if (sg._coins % 10 != 0)
		addLine("Copper pence.", sg._coins % 10);
	if (sg._coins % 100 > 9)
		addLine("Silver pieces", (sg._coins % 100) / 10);
	if (sg._coins > 99)
		addLine("Gold crowns..", sg._coins / 100);

	addLine("Enemy vessels", sg._enemyVessels);

	// Owned armor/weapons/spells/transports - index 0 of each is the
	// innate/default state (Skin/Hands/Prayer/Foot) and is never listed.
	// The currently equipped/readied one of each is drawn highlighted
	for (int idx = 1; idx < Data::ARMOR_COUNT; ++idx)
		addLine(Data::ARMOR_NAMES[idx], sg._armor[idx], idx == sg._equippedArmor);
	for (int idx = 1; idx < Data::WEAPON_COUNT; ++idx)
		addLine(Data::WEAPON_NAMES[idx], sg._weapons[idx], idx == sg._equippedWeapon);
	for (int idx = 1; idx < Data::SPELL_COUNT; ++idx)
		addLine(Data::SPELL_NAMES[idx], sg._spells[idx], idx == sg._equippedSpell);
	for (int idx = 1; idx < Data::TRANSPORT_COUNT; ++idx)
		addLine(Data::TRANSPORT_NAMES[idx], sg._transports[idx], idx == sg._transportType);

	const int16 gemValues[4] = { sg._redGems, sg._greenGems, sg._blueGem, sg._whiteGem };
	for (int idx = 0; idx < 4; ++idx)
		addLine(Data::GEM_NAMES[idx], gemValues[idx]);
}

Common::String ZStats::getLine(const char *name, int value) {
	Common::String numStr = Common::String::format("%d", value);
	Common::String result(name);

	while (result.size() < (17 - numStr.size()))
		result += '.';

	return result + numStr;
}

bool ZStats::msgAction(const ActionMessage &msg) {
	keyPressed();
	return true;
}

bool ZStats::msgKeypress(const KeypressMessage &msg) {
	keyPressed();
	return true;
}

void ZStats::keyPressed() {
	_pageStart += LINES_PER_PAGE;

	if (_pageStart < _lines.size())
		redraw();
	else
		close();
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
