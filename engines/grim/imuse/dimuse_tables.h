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

#ifndef GRIM_IMUSE_TABLES_H
#define GRIM_IMUSE_TABLES_H

#include "common/scummsys.h"

namespace Grim {

struct IMuseDigiStateTable {
	char name[32];
	int transitionId;
	int soundId;
	char internalName[24];
	int attribPos;
	int hookId;
	int fadeTime;
	int volume;
	int pan;
};

extern const IMuseDigiStateTable _grimStateMusicTable[181];
extern const IMuseDigiStateTable _grimSeqMusicTable[41];

// Sound id of a state, given its position in the table of the old iMUSE
int grimLegacyStateSoundId(int tableIndex);

} // End of namespace Grim

#endif
