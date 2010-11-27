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

#include "asylum/resources/actionlist.h"
#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/file.h"

namespace Asylum {

Encounter::Encounter(AsylumEngine *engine) : _vm(engine),
	_index(NULL), _keywordIndex(0), _item(NULL), _objectId1(kObjectNone), _objectId2(kObjectNone), _actorIndex(kActorInvalid),
	_flag1(false), _flag2(false) {

	_messageHandler = new MESSAGE_HANDLER(Encounter, messageHandler, this);

	load();
}

Encounter::~Encounter() {
	delete _messageHandler;

	_item = NULL;

	// Zero-out passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Data
//////////////////////////////////////////////////////////////////////////
void Encounter::load() {
	Common::File file;

	if (!file.open("sntrm.dat"))
		error("[Encounter::load] Could not open encounter data!");

	// Load the variables
	uint16 count = file.readUint16LE();
	for (uint i = 0; i < count; i++)
		_variables.push_back(file.readSint16LE());

	// Read anvil flag
	if (file.readSint16LE())
		error("[Encounter::load] Data file not supported!");

	// Read encounter data
	int16 dataCount = file.readSint16LE();
	for (uint8 i = 0; i < dataCount; i++) {
		EncounterItem item;
		memset(&item, 0, sizeof(EncounterItem));

		item.keywordIndex = file.readSint16LE();
		item.field2       = file.readSint16LE();
		item.scriptResourceId  = (ResourceId)file.readSint32LE();

		for (uint j = 0; j < 50; j++) {
			item.keywords[j] = file.readSint16LE();
		}

		item.value = file.readByte();

		_items.push_back(item);
	}

	file.close();
}

uint32 Encounter::findKeyword(EncounterItem *item, int16 keyword) {
	error("[Encounter::findKeyword] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Run
//////////////////////////////////////////////////////////////////////////
void Encounter::run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex) {
	// Line: 12/15 :: 0x25 (1, 1584, 1584, 0, 0, 0, 0, 0, 0) // First Encounter
	debugC(kDebugLevelEncounter, "Running Encounter %d", encounterIndex);

	if (!_keywordIndex) {
		_item = &_items[0];
		_keywordIndex = _item->keywordIndex;
	}

	if (encounterIndex < 0)
		return;

	// Original engine saves the main event handler (to be restored later)
	_index = encounterIndex;
	_item = &_items[encounterIndex];
	_objectId1 = objectId1;
	_objectId2 = objectId2;
	_actorIndex = actorIndex;

	if (getWorld()->getObjectById(objectId2))
		getWorld()->getObjectById(objectId2)->stopSound();

	getScene()->getActor(actorIndex)->stopSound();

	setVariable(1, 0);
	setVariable(2, _item->value);

	Actor *player = getScene()->getActor();
	if (player->getStatus() == kActorStatusDisabled) {
		_flag2 = true;
	} else {
		_flag2 = false;
		player->updateStatus(kActorStatusDisabled);
	}

	_flag1 = false;

	// Setup encounter event handler
	_vm->switchMessageHandler(_messageHandler);
}

//////////////////////////////////////////////////////////////////////////
// Message handler
//////////////////////////////////////////////////////////////////////////
void Encounter::messageHandler(const AsylumEvent &evt) {
	error("[Encounter::messageHandler] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////
void Encounter::setVariable(uint32 index, int32 val) {
	if (index >= _variables.size())
		error("[Encounter::setVariable] Invalid index (was: %d, max: %d)", index, _variables.size() - 1);

	_variables[index] = val;
}

int32 Encounter::getVariable(uint32 index) {
	if (index >= _variables.size())
		error("[Encounter::getVariable] Invalid index (was: %d, max: %d)", index, _variables.size() - 1);

	return _variables[index];
}
int32 Encounter::getVariableInv(int32 index) {
	if (index >= 0)
		return index;

	return getVariable(-index);
}

//////////////////////////////////////////////////////////////////////////
// Logic
//////////////////////////////////////////////////////////////////////////
void Encounter::resetSpeech(uint32 a1, uint32 a2) {
	error("[Encounter::resetSpeech] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Scripts
//////////////////////////////////////////////////////////////////////////
void Encounter::initScript() {
	_scriptData.reset();
	_flag3 = false;
}

Encounter::ScriptEntry Encounter::getScriptEntry(ResourceId resourceId, uint32 offset) {
	ResourceEntry *entry = getResource()->get(resourceId);

	return (ScriptEntry)entry->getData(offset);
}

void Encounter::runScript() {
	_flag4 = true;
	bool done = false;

	do {
		ScriptEntry entry = getScriptEntry(_scriptData.resourceId, _scriptData.offset);

		debugC(kDebugLevelEncounter, "Encounter %s", entry.toString().c_str());

		switch (entry.opcode) {
		default:
			break;

		case 0:
			_flag4 = false;
			done = true;
			_value1 = 0;
			break;

		case 1:
			_scriptData.vars[entry.param1] = getVariableInv(entry.param2);
			break;

		case 2:
			_scriptData.counter = _scriptData.vars[entry.param1] - getVariableInv(entry.param2);
			break;

		case 3:
			_scriptData.offset = entry.param2 - 1;
			break;

		case 4:
			if (_scriptData.counter >= 0)
				break;

			_scriptData.offset = entry.param2;
			break;

		case 5:
			if (_scriptData.counter > 0)
				break;

			_scriptData.offset = entry.param2;
			break;

		case 6:
			if (_scriptData.counter)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case 7:
			if (!_scriptData.counter)
				break;

			_scriptData.offset = entry.param2;
			break;

		case 8:
			if (_scriptData.counter < 0)
				break;

			_scriptData.offset = entry.param2;
			break;

		case 9:
			if (_scriptData.counter <= 0)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case 10:
			if (entry.param1)
				_item->keywords[findKeyword(_item, entry.param2)] &= ~kEncounterArray2000;
			else
				_item->keywords[findKeyword(_item, entry.param2)] |= kEncounterArray8000;
			break;

		case 11:
			if (entry.param1)
				_item->keywords[findKeyword(_item, entry.param2)] |= kEncounterArray2000;
			else
				_item->keywords[findKeyword(_item, entry.param2)] &= ~kEncounterArray8000;
			break;

		case 12:
			_items[entry.param1].keywords[findKeyword(&_items[entry.param1], entry.param2)] |= kEncounterArray4000;
			_items[entry.param1].keywords[findKeyword(&_items[entry.param1], entry.param2)] |= kEncounterArray8000;
			break;

		case 13:
			if (!_flag3)
				_data_455BD4 = 1;

			done = true;
			break;

		case 14:
			resetSpeech(_item->keywordIndex, getVariableInv(entry.param2));

			done = true;
			break;

		case 15:
			setVariable(entry.param2, _scriptData.vars[entry.param1]);
			break;

		case 16:
			_scriptData.vars[entry.param1] += entry.param2;
			break;

		case 17:
			switch (getVariable(3)) {
			default:
				break;

			case 13:
			case 15:
				if (_actorIndex) {
					getScene()->getActor(_actorIndex)->hide();
				} else {
					getWorld()->getObjectById(_objectId1)->disable();
					getWorld()->getObjectById(_objectId2)->disable();
				}

				if (getVariable(3) == 13)
					break;
				// Fallback to next case

			case 14:
				getScene()->getActor()->hide(); // Hide player
				break;

			case 16: {
				Object *obj = getWorld()->getObjectById((ObjectId)getVariableInv(entry.param2));
				obj->setNextFrame(obj->flags);
				_data_455BF4 = 1;
				done = true;
				}
				break;

			case 17:
				getWorld()->getObjectById((ObjectId)getVariableInv(entry.param2))->setNextFrame(32);
				_data_455BF4 = 1;
				done = true;
				break;

			case 18:
				getWorld()->getObjectById((ObjectId)getVariableInv(entry.param2))->setNextFrame(512);
				_data_455BF4 = 1;
				done = true;
				break;
			}

			if (!done) {
				_objectId3 = (ObjectId)getVariableInv(entry.param2);
				getWorld()->getObjectById(_objectId3)->setNextFrame(8);
				getCursor()->hide();
				done = true;
			}
			break;

		case 18:
			if (entry.param1)
				getScene()->getActor()->process_41BCC0(getVariableInv(entry.param2), _scriptData.vars[1]);
			else
				getScene()->getActor()->process_41BC00(getVariableInv(entry.param2), _scriptData.vars[1]);
			break;

		case 21:
			_scriptData.counter = getScene()->getActor()->process_41BDB0(getVariableInv(entry.param2), _scriptData.vars[1]) ? 0 : 1;
			break;

		case 23:
			if (!getSharedData()->getMatteBarHeight()) {
				getScene()->makeGreyPalette();
				getSharedData()->setMatteBarHeight(1);
				getScene()->actions()->setDelayedVideoIndex(getVariableInv(entry.param2));
				getSharedData()->setMatteVar1(1);
				getSharedData()->setMattePlaySound(true);
				getSharedData()->setMatteInitialized(true);
				getSharedData()->setMatteVar2(0);
				done = true;
			}

			if (!_data_455BE8) {
				_data_455BE8 = true;
				getCursor()->hide();
			}
			break;

		case 24:
			if (entry.param1)
				_vm->setGameFlag((GameFlag)getVariableInv(entry.param2));
			else
				_vm->clearGameFlag((GameFlag)getVariableInv(entry.param2));
			break;

		case 25:
			_scriptData.counter = _vm->isGameFlagSet((GameFlag)getVariableInv(entry.param2)) ?  1 : 0;
			break;
		}

		++_scriptData.offset;

	} while (!done);
}

}
