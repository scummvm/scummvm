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

void MonsterLogic::checkForAttacks(PLAYER &p, DUNGEONMAP &d) {
	int i, Attacked;
	double Dist;

	// Go through all monsters
	for (i = 0; i < d.MonstCount; i++) {
		MONSTER &m = d.Monster[i];				// Pointer to MONSTER &/
		Dist = pow(m.Loc.x - p.Dungeon.x, 2);	// Calculate Distance
		Dist = Dist + pow(m.Loc.y - p.Dungeon.y, 2);
		Dist = sqrt(Dist);

		// If alive
		if (m.Alive != 0) {
			Attacked = 0;

			// If within range
			if (Dist < 1.3)
				Attacked = attack(m, p);

			// If didn't attack, then move
			if (Attacked == 0) {
				// Mimics only if near enough
				if (m.Type != MN_MIMIC || Dist >= 3.0)
					move(m, p, d);

				// Recovers if didn't attack
				if (m.Strength < p.Level * p.Skill)
					m.Strength = m.Strength + p.Level;
			}
		}
	}
}

void MonsterLogic::showLines(const Common::String &msg) {
	g_events->send("DungeonStatus", GameMessage("LINES", msg));
}

int MonsterLogic::attack(MONSTER &m, PLAYER &p) {
	int n;

	if (m.Type == MN_GREMLIN ||		// Special case for Gremlin/Thief
		m.Type == MN_THIEF)
		if (RND() > 0.5)			// Half the time
			return steal(m, p);

	Common::String msg = Common::String::format("You are being attacked by a %s !!!.\n",
		MONSTER_INFO[m.Type].Name);

	n = urand() % 20;					// Calculate hit chance
	if (p.Object[OB_SHIELD] > 0) n--;
	n = n - p.Attr[AT_STAMINA];
	n = n + m.Type + p.Level;
	if (n < 0) {
		// Missed !
		msg += "Missed !\n";
	} else {
		// Hit !
		n = urand() % m.Type;		// Calculate damage done.
		n = n + p.Level;
		p.Attr[AT_HP] -= n;			// Adjust hit points
		msg += "Hit !!!\n";
	}

	showLines(msg);
	return 1;
}

void MonsterLogic::move(MONSTER &m, PLAYER &p, DUNGEONMAP &d) {
	int x, y, xi, yi;

	// Calculate direction
	xi = yi = 0;
	if (p.Dungeon.x != m.Loc.x)
		xi = (p.Dungeon.x > m.Loc.x) ? 1 : -1;
	if (p.Dungeon.y != m.Loc.y)
		yi = (p.Dungeon.y > m.Loc.y) ? 1 : -1;

	// Running away
	if (m.Strength < p.Level * p.Skill) {
		xi = -xi; yi = -yi;
	}

	// Get position
	x = m.Loc.x; y = m.Loc.y;

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
	if (!canMoveTo(d, x, y))	// Fail if can't move there
		return;
	if (x == p.Dungeon.x &&			// Can't move onto us
		y == p.Dungeon.y) return;
	m.Loc.x = x; m.Loc.y = y;			// Move to new position
}

bool MonsterLogic::canMoveTo(DUNGEONMAP &d, int x, int y) {
	COORD c;
	int t = d.Map[x][y];				// See what's there
	if (!ISWALKTHRU(t)) return 0;		// Can't walk through walls
	c.x = x; c.y = y;					// Set up coord structure

	// True if no monster here
	return d.findMonster(c) < 0;
}

int MonsterLogic::steal(MONSTER &m, PLAYER &p) {
	int n;
	const char *s1, *s2;

	if (m.Type == MN_GREMLIN) {
		// HALVES the food.... aargh
		p.Object[OB_FOOD] = floor(p.Object[OB_FOOD]) / 2.0;
		showLines("A Gremlin stole some food.\n");

	} else if (m.Type == MN_THIEF) {
		// Figure out what stolen
		do {
			n = urand() % p.Objects;
		} while (p.Object[n] == 0);

		p.Object[n]--;					// Stole one
		s2 = OBJECT_INFO[n].Name; s1 = "a";

		if (strchr("aeiou", tolower(*s2))) s1 = "an";
		if (n == 0) s1 = "some";

		showLines(Common::String::format("A Thief stole %s %s.\n", s1, s2));
	}

	return 1;
}

} // namespace Ultima0
} // namespace Ultima
