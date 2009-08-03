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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ENCOUNTERS_H_
#define ASYLUM_ENCOUNTERS_H_

#include "common/array.h"

namespace Asylum {

typedef struct EncounterItem {
	uint32 keywordIndex;
	uint32 field2;
	uint32 scriptResId;
	uint32 array[50];
	uint16 value;
} EncounterItem;

typedef struct EncounterStruct {
	uint32 x1;
	uint32 y1;
	uint32 x2;
	uint32 y2;
	uint32 frameNum;
	uint32 transTableNum;
	uint32 status;
	uint32 grResId;
} EncounterStruct;

class Encounter {
public:
	Encounter();
	virtual ~Encounter();

private:
	uint16 *_variables;
	uint16 _anvilStyleFlag;
	Common::Array<EncounterItem> _items;

}; // end of class Encounter

} // end of namespace Asylum

#endif
