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

#ifndef ASYLUM_ENCOUNTERS_H
#define ASYLUM_ENCOUNTERS_H

#include "asylum/shared.h"

#include "common/array.h"

namespace Asylum {

class AsylumEngine;

typedef struct EncounterItem {
	int32 keywordIndex;
	int32 field2;
	ResourceId scriptResourceId;
	int32 array[50];
	int16 value;
} EncounterItem;

typedef struct EncounterStruct {
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;
	int32 frameNum;
	int32 transTableNum;
	int32 status;
	ResourceId graphicResourceId;
} EncounterStruct;

class Encounter {
public:
	Encounter(AsylumEngine *engine);
	virtual ~Encounter();

	void setVariable(int32 idx, int32 value) {
		_variables[idx] = value;
	}
	void run(int32 encounterIdx, int32 objectId1, int32 objectId2, int32 characterIdx);

private:
	AsylumEngine *_vm;

	int16 *_variables;
	int16 _anvilStyleFlag;

	EncounterItem *_currentEncounter;
	Common::Array<EncounterItem> _items;

	friend class Console;
}; // end of class Encounter

} // end of namespace Asylum

#endif
