/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_SAVELOAD_H
#define M4_SAVELOAD_H

#include "m4/graphics.h"
#include "common/ptr.h"

#define MAX_SAVEGAME_NAME 80

namespace M4 {

typedef Common::List<Common::String> SaveGameList;
typedef SaveGameList::iterator SaveGameIterator;

class SaveLoad {
private:
	MadsM4Engine *_vm;
	bool _emulateOriginal;

	const char *generateSaveName(int slotNumber);
public:
	SaveLoad(MadsM4Engine *vm);

	bool hasSaves();
	SaveGameList *getSaves();
	M4Surface *getThumbnail(int slotNumber);
	bool load(int slotNumber);
	bool save(int slotNumber, Common::String saveName);
};

}

#endif
