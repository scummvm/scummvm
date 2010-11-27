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

#include "asylum/asylum.h"
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

	void run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex);

	void setVariable(uint32 index, int32 val);

private:
	AsylumEngine *_vm;

	Common::Functor1Mem<const AsylumEvent &, void, Encounter> *_messageHandler;

	// Data
	Common::Array<int16> _variables;
	int16 _anvilStyleFlag;
	Common::Array<EncounterItem> _items;

	// Running encounter data
	int32 _index;
	int32 _keywordIndex;
	EncounterItem *_item;
	ObjectId _objectId1;
	ObjectId _objectId2;
	ActorIndex _actorIndex;

	// Internal flags
	bool _flag1;
	bool _flag2;

	void load();

	void messageHandler(const AsylumEvent &evt);

	friend class Console;
}; // end of class Encounter

} // end of namespace Asylum

#endif
