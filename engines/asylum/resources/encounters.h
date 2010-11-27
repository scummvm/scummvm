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

class Encounter {
public:
	Encounter(AsylumEngine *engine);
	virtual ~Encounter();

	void run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex);

private:
	AsylumEngine *_vm;

	//////////////////////////////////////////////////////////////////////////
	// Data
	enum EncounterArray {
		kEncounterArray2000 = 0x2000,
		kEncounterArray4000 = 0x4000,
		kEncounterArray8000 = 0x8000,
	};

	typedef struct EncounterItem {
		int16 keywordIndex;
		int16 field2;
		ResourceId scriptResourceId;
		int16 keywords[50];
		byte value;
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

	Common::Array<int16> _variables;
	int16 _anvilStyleFlag;
	Common::Array<EncounterItem> _items;

	// Running encounter data
	int32 _index;
	int32 _keywordIndex;
	EncounterItem *_item;
	ObjectId _objectId1;
	ObjectId _objectId2;
	ObjectId _objectId3;
	ActorIndex _actorIndex;

	uint32 _value1;

	// Internal data
	bool _data_455BE8;
	uint32 _data_455BF4;
	bool _data_455BD4;

	// Internal flags
	bool _flag1;
	bool _flag2;
	bool _flag3;
	bool _flag4;

	//////////////////////////////////////////////////////////////////////////
	// Data
	void load();
	uint32 findKeyword(EncounterItem *item, int16 keyword);

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	Common::Functor1Mem<const AsylumEvent &, void, Encounter> *_messageHandler;
	void messageHandler(const AsylumEvent &evt);
	void init();
	void update();
	void key(const AsylumEvent &evt);
	void mouse(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Variables
	void setVariable(uint32 index, int32 val);\
	int32 getVariable(uint32 index);
	int32 getVariableInv(int32 index);

	//////////////////////////////////////////////////////////////////////////
	// Logic
	void resetSpeech(uint32 a1, uint32 a2);

	//////////////////////////////////////////////////////////////////////////
	// Scripts
	struct ScriptEntry {
		union {
			byte opcode;
			byte param1;
			uint16 param2;
			uint32 data;
		};

		ScriptEntry(uint32 val) {
			data = val;
		}

		Common::String toString() {
			return Common::String::format("[0x%02X] %d %d", opcode, param1, param2);
		}
	};

	struct ScriptData {
		int32 vars[10];
		uint32 offset;
		int32 counter;
		uint32 resourceId;

		ScriptData() {
			reset();
		}

		void reset() {
			memset(&vars, 0, sizeof(vars));
			offset = 0;
			counter = 0;
			resourceId = kResourceNone;
		}
	};

	ScriptData _scriptData;

	void initScript();
	ScriptEntry getScriptEntry(ResourceId resourceId, uint32 offset);
	void runScript();

	friend class Console;
}; // end of class Encounter

} // end of namespace Asylum

#endif
