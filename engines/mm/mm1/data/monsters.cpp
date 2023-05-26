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

#include "common/file.h"
#include "mm/mm1/data/monsters.h"
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

static const uint16 PALETTE[76] = {
	0xf470, 0xf420, 0xfe20, 0xf630, 0xf420, 0xf620, 0xf460, 0xf6e0,
	0xf510, 0xfe40, 0xf420, 0xf410, 0xfd50, 0xfc90, 0xf430, 0xfc30,
	0xf770, 0xfc30, 0xf420, 0xf430, 0xf420, 0xf490, 0xf110, 0xf4e0,
	0xf430, 0xfd60, 0xf430, 0xfc20, 0xf2a0, 0xf470, 0xf4e0, 0xf250,
	0xf430, 0xf320, 0xfee0, 0xf420, 0xf220, 0xf420, 0xfdd0, 0xf420,
	0xf620, 0xfc20, 0xfc10, 0xf520, 0xf420, 0xf220, 0xf420, 0xfa50,
	0xfe20, 0xf620, 0xf470, 0xf420, 0xfe10, 0xf4e0, 0xfe40, 0xf140,
	0xf290, 0xf410, 0xf520, 0xf410, 0xfc10, 0xf120, 0xf420, 0xfe10,
	0xf520, 0xf4a0, 0xfe60, 0xfe60, 0xf620, 0xf620, 0xfce0, 0xf420,
	0xfc20, 0xfc20, 0xfd90, 0xf420
};

Monsters::Monsters() : _monPix(MONPIX_DTA) {
}

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

Graphics::ManagedSurface Monsters::getMonsterImage(int imgNum) {
	Common::SeekableReadStream *entry = _monPix.load(imgNum);
	entry->skip(2);

	// Decode the image
	Graphics::ManagedSurface img;
	Gfx::ScreenDecoder decoder;

	uint pal = PALETTE[imgNum];
	decoder._indexes[0] = pal & 0xf;
	decoder._indexes[1] = (pal >> 4) & 0xf;
	decoder._indexes[2] = (pal >> 8) & 0xf;
	decoder._indexes[3] = (pal >> 12) & 0xf;

	if (!decoder.loadStream(*entry, 104, 96))
		error("Failed decoding monster image");

	img.copyFrom(decoder.getSurface());

	return img;
}

} // namespace MM1
} // namespace MM
