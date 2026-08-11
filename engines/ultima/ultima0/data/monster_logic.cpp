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

#include "ultima/ultima0/data/monster_logic.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

void MonsterLogic::checkForAttacks(PlayerInfo &p, DungeonMapInfo &d) {
	int attacked;
	double dist;

	// Go through all monsters
	for (MonsterEntry &m : d._monsters) {
		dist = pow(m._loc.x - p._dungeonPos.x, 2);	// Calculate Distance
		dist = dist + pow(m._loc.y - p._dungeonPos.y, 2);
		dist = sqrt(dist);

		// If alive
		if (m._alive) {
			attacked = 0;

			// If within range
			if (dist < 1.3)
				attacked = attack(m, p);

			// If didn't attack, then move
			if (attacked == 0) {
				// Mimics only if near enough
				if (m._type != MN_MIMIC || dist >= 3.0)
					move(m, p, d);

				// Recovers if didn't attack
				if (m._strength < p._level * p._skill)
					m._strength = m._strength + p._level;
			}
		}
	}
}

void MonsterLogic::showLines(const Common::String &msg) {
	g_events->send("DungeonStatus", GameMessage("LINES", msg));
}

int MonsterLogic::attack(MonsterEntry &m, PlayerInfo &p) {
	int n;

	Common::String msg = "You are being attacked\nby a ";
	msg += (char)C_BLUE;
	msg += MONSTER_INFO[m._type]._name;
	msg += (char)C_TEXT_DEFAULT;
	msg += '\n';

	// Special case for Gremlin/Thief stealing
	if (m._type == MN_GREMLIN || m._type == MN_THIEF)
		if (RND() > 0.5)
			// Half the time
			return steal(msg, m, p);

	n = urand() % 20;					// Calculate hit chance
	if (p._object[OB_SHIELD] > 0) n--;
	n = n - p._attr[AT_STAMINA];
	n = n + m._type + p._level;

	if (n < 0) {
		// Missed
		msg += (char)C_TOMATO;
		msg += "Missed!";
	} else {
		// Hit
		n = urand() % m._type;			// Calculate damage done.
		n = n + p._level;
		p._attr[AT_HP] -= n;			// Adjust hit points

		msg += (char)C_TOMATO;
		msg += "Hit! ";
		msg += (char)C_TEXT_DEFAULT;
		msg += " Damage = ";
		msg += (char)C_BLUE;
		msg += Common::String::format("%d", n);
	}

	showLines(msg);
	return 1;
}

void MonsterLogic::move(MonsterEntry &m, PlayerInfo &p, DungeonMapInfo &d) {
	int x, y, xi, yi;

	// Calculate direction
	xi = yi = 0;
	if (p._dungeonPos.x != m._loc.x)
		xi = (p._dungeonPos.x > m._loc.x) ? 1 : -1;
	if (p._dungeonPos.y != m._loc.y)
		yi = (p._dungeonPos.y > m._loc.y) ? 1 : -1;

	// Running away
	if (m._strength < p._level * p._skill) {
		xi = -xi; yi = -yi;
	}

	// Get position
	x = m._loc.x; y = m._loc.y;

	// Check move okay
	if (ABS(xi) > ABS(yi)) {
		if (canMoveTo(d, x + xi, yi)) yi = 0;
		else if (canMoveTo(d, x, y + yi)) xi = 0;
	} else {
		if (canMoveTo(d, x, y + yi)) xi = 0;
		else if (canMoveTo(d, x + xi, yi)) yi = 0;
	}

	if (xi == 0 && yi == 0)
		return;		// No move

	x = x + xi; y = y + yi;				// Work out new position
	if (!canMoveTo(d, x, y))			// Fail if can't move there
		return;
	if (x == p._dungeonPos.x &&			// Can't move onto us
		y == p._dungeonPos.y) return;

	// Move to new position
	m._loc.x = x; m._loc.y = y;

	// If the tile was for a hidden door, flag it as a normal visible door
	if (d._map[x][y] == DT_HIDDENDOOR)
		d._map[x][y] = DT_DOOR;
}

bool MonsterLogic::canMoveTo(DungeonMapInfo &d, int x, int y) {
	Common::Point c;
	int t = d._map[x][y];				// See what's there
	if (!ISWALKTHRU(t))
		return 0;						// Can't walk through walls
	c.x = x; c.y = y;					// Set up coord structure

	// True if no monster here
	return d.findMonster(c) < 0;
}

int MonsterLogic::steal(const Common::String &attackStr, MonsterEntry &m, PlayerInfo &p) {
	int n;
	const char *s1, *s2;

	Common::String msg = attackStr;
	msg += (char)C_GREEN;
	msg += "A ";
	msg += MONSTER_INFO[m._type]._name;
	msg += " stole ";

	if (m._type == MN_GREMLIN) {
		// HALVES the food.... aargh
		p._object[OB_FOOD] = floor(p._object[OB_FOOD]) / 2.0;
		msg += "some ";
		msg += (char)C_BLUE;
		msg += "Food";
		showLines(msg);

	} else if (m._type == MN_THIEF) {
		// Figure out what stolen
		do {
			n = urand() % MAX_OBJ;
		} while (p._object[n] == 0);

		p._object[n]--;					// Stole one
		s2 = OBJECT_INFO[n]._name;
		s1 = "a";

		if (strchr("aeiou", tolower(*s2))) s1 = "an";
		if (n == 0) s1 = "some";

		msg += s1;
		msg += ' ';
		msg += (char)C_BLUE;
		msg += s2;
		showLines(msg);
	}

	return 1;
}

} // namespace Ultima0
} // namespace Ultima
