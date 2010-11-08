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

#include "asylum/resources/encounters.h"

#include "asylum/asylum.h"

#include "common/file.h"

namespace Asylum {

Encounter::Encounter(AsylumEngine *engine) : _vm(engine) {
	memset(_flags, 0, sizeof(_flags));

	Common::File file;

	// TODO error checks
	file.open("sntrm.dat");

	int16 _count = file.readSint16LE();

	_variables = (int16*)malloc(_count);
	file.read(_variables, _count);

	file.seek(2 + _count * 2, SEEK_SET);

	// TODO assert if true
	_anvilStyleFlag = file.readSint16LE();

	int16 _dataCount = file.readSint16LE();

	for (uint8 i = 0; i < _dataCount; i++) {
		EncounterItem item;
		memset(&item, 0, sizeof(EncounterItem));

		item.keywordIndex = file.readSint32LE();
		item.field2       = file.readSint32LE();
		item.scriptResourceId  = (ResourceId)file.readSint32LE();
		for (uint8 j = 0; j < 50; j++) {
			item.array[j] = file.readSint32LE();
		}
		item.value = file.readSint16LE();

		_items.push_back(item);
	}

	file.close();
}

//////////////////////////////////////////////////////////////////////////
// Flags

int32 Encounter::getFlag(EncounterFlag flag) {
	if (flag > ARRAYSIZE(_flags))
		error("[Encounter::getFlag] Invalid flag index!");

	return _flags[flag];
}

void Encounter::setFlag(EncounterFlag flag, int32 val) {
	if (flag > ARRAYSIZE(_flags))
		error("[Encounter::getFlag] Invalid flag index!");

	_flags[flag] = val;
}

void Encounter::run(int32 encounterIdx, int32 objectId1, int32 objectId2, int32 characterIdx) {
	// Line: 12/15 :: 0x25 (1, 1584, 1584, 0, 0, 0, 0, 0, 0) // First Encounter

	//debugC(kDebugLevelEncounter, "Running Encounter %d", encounterIdx);

	_currentEncounter = &_items[encounterIdx];
	setVariable(1, 0);
	setVariable(2, _currentEncounter->value);

	//Object *b1 = _scene->worldstats()->getObjectById(objectId1);
	/*
	 int32 __cdecl runEncounter(int32 newMessageHandler, int32 encounterIndex, int32 objectId1, int32 objectId2, int32 characterIndex)
	{
	  int32 result; // eax@7
	  EncounterItem *v6; // eax@2
	  int32 v7; // ST04_4@4
	  int32 v8; // eax@4

	  if ( !encounterKeywordIndex )
	  {
	    v6 = getEncounterItem(0);
	    encounterItem = v6;
	    encounterKeywordIndex = *(_DWORD *)&v6->keywordIndex;
	  }
	  if ( encounterIndex < 0 )
	  {
	    result = 0;
	  }
	  else
	  {
	    encounter_newMessageHandler = newMessageHandler;
	    encounterIndex = encounterIndex;
	    encounterItem = getEncounterItem(encounterIndex);
	    encounter_objectId01 = objectId1;
	    v7 = characterIndex;
	    encounter_objectId02 = objectId2;
	    characterIndex2 = characterIndex;
	    v8 = getObjectIndexById(objectId2);
	    object_sound_sub_414C30(v8, v7);
	    setEncounterVariable(1, 0);
	    setEncounterVariable(2, encounterItem->value);
	    if ( scene.characters[playerCharacterIndex].field_40 == 5 )
	    {
	      encounter_flag02 = 1;
	    }
	    else
	    {
	      encounter_flag02 = 0;
	      character_sub_4072A0(playerCharacterIndex, 5);
	    }
	    flag04 = 0;
	    switchMessageHandler((int32 (__cdecl *)(_DWORD, _DWORD, _DWORD))handleMessageEncounter);
	    result = 1;
	  }
	  return result;
	}
	 */
}

Encounter::~Encounter() {
	free(_variables);
}

}
