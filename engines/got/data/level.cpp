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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "got/data/defines.h"
#include "got/vars.h"

namespace Got {

void Level::sync(Common::Serializer &s) {
	for (int i = 0; i < 12; ++i)
		s.syncBytes(_iconGrid[i], 20);

	s.syncAsByte(_backgroundColor);
	s.syncAsByte(_music);

	s.syncBytes(_actorType, 16);
	s.syncBytes(_actorLoc, 16);
	s.syncBytes(_actorValue, 16);
	s.syncBytes(_palColors, 3);
	s.syncBytes(_actorInvis, 16);
	s.syncBytes(_extra, 13);
	s.syncBytes(_staticObject, 30);

	for (int i = 0; i < 30; ++i)
		s.syncAsSint16LE(_staticX[i]);
	for (int i = 0; i < 30; ++i)
		s.syncAsSint16LE(_staticY[i]);

	s.syncBytes(_newLevel, 10);
	s.syncBytes(_newLevelLocation, 10);

	s.syncAsByte(_area);
	s.syncBytes(_actorDir, 16);
	s.syncBytes(_filler, 3);
}

void Level::load(int level) {
	Common::MemoryReadStream src(_G(sdData)[level], 512);
	Common::Serializer s(&src, nullptr);
	sync(s);
}

void Level::save(int level) {
	Common::MemoryWriteStream dest(_G(sdData)[level], 512);
	Common::Serializer s(nullptr, &dest);
	sync(s);
}

} // namespace Got
