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

#ifndef PRIVATE_SAVEGAME_H
#define PRIVATE_SAVEGAME_H

#include "common/language.h"
#include "common/platform.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Private {

// Savegame format history:
//
// Version - new/changed feature
// =============================
//       4 - _pausedBackgroundSoundName (December 2025)
//       3 - Radio detailed state (December 2025)
//       2 - Phone clip detailed state (December 2025)
//       1 - Metadata header and more game state (November 2025)
//
// Earlier versions did not have a header and not supported.

const uint16 kCurrentSavegameVersion = 4;
const uint16 kMinimumSavegameVersion = 3;

struct SavegameMetadata {
	uint16 version;
	Common::Language language;
	Common::Platform platform;
};

/**
 * Write the header to a savegame.
 */
void writeSavegameMetadata(Common::WriteStream *stream, const SavegameMetadata &meta);

/**
 * Read the header from a savegame.
 */
bool readSavegameMetadata(Common::SeekableReadStream *stream, SavegameMetadata &meta);

} // End of namespace Private

#endif
