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

#include "got/game/status.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

void add_jewels(int num) {
	_G(thor_info).jewels = CLIP(_G(thor_info).jewels + num, 0, 999);
}

void add_score(int num) {
	_G(thor_info).score = CLIP(_G(thor_info).score + num, 0l, 999999l);
}

void add_magic(int num) {
	_G(thor_info).magic = CLIP(_G(thor_info).magic + num, 0, 150);
}

void add_health(int num) {
	_G(thor)->health = CLIP(_G(thor)->health + num, 0, 150);

	if (_G(thor)->health < 1)
		g_events->send(GameMessage("THOR_DIES"));
}

void add_keys(int num) {
	_G(thor_info).keys = CLIP(_G(thor_info).keys + num, 0, 99);
}

void fill_health() {
	add_health(150);
}

void fill_magic() {
	add_magic(150);
}

void fill_score(int num, const char *endMessage) {
	GameMessage msg("FILL_SCORE");
	if (endMessage)
		msg._stringValue = endMessage;
	msg._value = num;
	g_events->send("GameStatus", msg);
}

} // namespace Got
