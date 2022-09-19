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
		line = Common::String(line.c_str() + 17);

		mon._count = getNextValue(line);
		mon._field10 = getNextValue(line);
		mon._field11 = getNextValue(line);
		mon._field12 = getNextValue(line);

		for (int i = 0; i < 13; ++i)
			mon._unk[i] = getNextValue(line);
	}

	return true;
}

byte Monsters::getNextValue(Common::String &line) {
	// Verify the next comma
	if (!line.hasPrefix(", "))
		return 0;
	line.deleteChar(0);
	line.deleteChar(0);

	// Get the value
	byte result = atoi(line.c_str());
	while (!line.empty() && Common::isDigit(line.firstChar()))
		line.deleteChar(0);

	return result;
}

Graphics::ManagedSurface Monsters::getMonsterImage(int monsterNum) {
	Common::SeekableReadStream *entry = _monPix.load(monsterNum);
	entry->skip(2);

	// Decode the image
	Graphics::ManagedSurface img;
	Gfx::ScreenDecoder decoder;
	if (!decoder.loadStream(*entry, 104, 96))
		error("Failed decoding monster image");

	img.copyFrom(decoder.getSurface());

	return img;
}

} // namespace MM1
} // namespace MM
