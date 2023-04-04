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

#include "mm/mm1/data/monsters.h"
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "common/file.h"

namespace MM {
namespace MM1 {

bool Monsters::load() {
	Common::File f;
	if (!f.open("monsters.txt"))
		return false;

	for (int lineNum = 0; lineNum < MONSTERS_COUNT; ++lineNum) {
		Monster &mon = _monsters[lineNum];
		Common::String line = f.readLine();
		assert(line.size() > 20 && line[0] == '"' && line[16] == '"');

		mon._name = Common::String(line.c_str() + 1, line.c_str() + 15);
		while (mon._name.hasSuffix(" "))
			mon._name.deleteLastChar();

		line = Common::String(line.c_str() + 17);
		mon._count = getNextValue(line);
		mon._fleeThreshold = getNextValue(line);
		mon._defaultHP = getNextValue(line);
		mon._defaultAC = getNextValue(line);
		mon._maxDamage = getNextValue(line);
		mon._numberOfAttacks = getNextValue(line);
		mon._speed = getNextValue(line);
		mon._experience = getNextValue(line);
		mon._loot = getNextValue(line);
		mon._resistUndead = getNextValue(line);
		mon._resistances = getNextValue(line);
		mon._bonusOnTouch = getNextValue(line);
		mon._specialAbility = getNextValue(line);
		mon._specialThreshold = getNextValue(line);
		mon._counterFlags = getNextValue(line);
		mon._imgNum = getNextValue(line);
	}

	return true;
}

Graphics::ManagedSurface Monsters::getMonsterImage(int monsterNum) {
	Common::SeekableReadStream *entry = _monPix.load(monsterNum);
	entry->skip(2);

	// Decode the image
	Graphics::ManagedSurface img;
	Gfx::ScreenDecoder decoder;
	// TODO: Figure out if the indexes are map-dependant
	decoder._indexes[0] = 0;
	decoder._indexes[1] = 2;
	decoder._indexes[2] = 4;
	decoder._indexes[3] = 15;

	if (!decoder.loadStream(*entry, 104, 96))
		error("Failed decoding monster image");

	img.copyFrom(decoder.getSurface());

	return img;
}

} // namespace MM1
} // namespace MM
