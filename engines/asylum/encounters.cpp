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

#include "common/file.h"

#include "asylum/encounters.h"

namespace Asylum {

Encounter::Encounter(Scene *scene) {
	Common::File file;

	// TODO error checks
	file.open("sntrm.dat");

	uint16 _count = file.readUint16LE();

	_variables = (uint16*)malloc(_count);
	file.read(_variables, _count);

	file.seek(2 + _count * 2, SEEK_SET);

	// TODO assert if true
	_anvilStyleFlag = file.readUint16LE();

	uint16 _dataCount = file.readUint16LE();

	for (uint8 i = 0; i < _dataCount; i++) {
		EncounterItem item;
		memset(&item, 0, sizeof(EncounterItem));

		item.keywordIndex = file.readUint32LE();
		item.field2       = file.readUint32LE();
		item.scriptResId  = file.readUint32LE();
		for (uint8 j = 0; j < 50; j++) {
			item.array[j] = file.readUint32LE();
		}
		item.value = file.readUint16LE();

		_items.push_back(item);
	}

	file.close();

	_scene = scene;
}

void Encounter::run(int encounterIdx, int barrierId1, int barrierId2, int characterIdx) {
	// Line: 12/15 :: 0x25 (1, 1584, 1584, 0, 0, 0, 0, 0, 0) // First Encounter

	//debugC(kDebugLevelEncounter, "Running Encounter %d", encounterIdx);

	_currentEncounter = &_items[encounterIdx];
	setVariable(1, 0);
	setVariable(2, _currentEncounter->value);

	Barrier *b1 = _scene->worldstats()->getBarrierById(barrierId1);
	/*
	 int __cdecl runEncounter(int newMessageHandler, int encounterIndex, int objectId1, int objectId2, int characterIndex)
	{
	  int result; // eax@7
	  EncounterItem *v6; // eax@2
	  int v7; // ST04_4@4
	  int v8; // eax@4

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
	    switchMessageHandler((int (__cdecl *)(_DWORD, _DWORD, _DWORD))handleMessageEncounter);
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
