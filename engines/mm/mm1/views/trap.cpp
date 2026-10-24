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

#include "mm/mm1/views/trap.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

static const char *const TRAP_TEXT[11] = {
	"Darts!! a swarm of poisonous darts\n"
		"        fill the air!",
	"Spikes!! Deadly spikes spring forth!",
	"Arrows!! A sudden onslaught of poisonous         arrows permeate the party!",
	"Blades!! Razor sharp blades slice\n"
		"         through the party!",
	"Boiling oil!! Streams of boiling oil\n"
		"              cover the party!",
	"Gas!! A faint hiss can be heard as\n"
		"      noxious fumes fill the air!",
	"Fireball!! A fiery explosion engulfs\n"
		"           the area!",
	"Lightning bolt!! Electric currents\n"
		"                 singe the party!",
	"Acid!! A fine mist of volatile acid\n"
		"       sprays the party!",
	"Ice storm!! Particles of splintered ice\n"
		"            hail through the air!",
	"Death ray!! A blinding light sears\n"
		"            through the party!"
};

Trap::Trap() : TextView("Trap") {
	_bounds = getLineBounds(20, 24);
}

bool Trap::msgGame(const GameMessage &msg) {
	if (msg._name == "TRIGGER") {
		open();
		trigger();
		return true;

	} else if (msg._name == "TRAP") {
		open();
		trap();
		return true;
	}

	return false;
}

void Trap::draw() {
	clearSurface();

	if (_mode == MODE_TRIGGER) {
		writeString(9, 1, "Oops a trap!");
	} else {
		writeString(0, 1, TRAP_TEXT[_trapType]);
	}
}

bool Trap::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (_mode == MODE_TRIGGER) {
		trap();
	} else {
		close();
	}

	return true;
}

bool Trap::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (_mode == MODE_TRIGGER) {
		trap();
	} else {
		close();
	}

	return true;
}

void Trap::trap() {
	TrapData::trap();

	_mode = MODE_TRAP;
	draw();
}

void Trap::trigger() {
	_mode = MODE_TRIGGER;
	g_globals->_treasure._container =
		g_maps->_currentMap->dataByte(Maps::MAP_49);
	g_globals->_currCharacter = &g_globals->_party[0];

	Sound::sound(SOUND_2);
	delaySeconds(2);
}

void Trap::timeout() {
	trap();
}

} // namespace Views
} // namespace MM1
} // namespace MM
