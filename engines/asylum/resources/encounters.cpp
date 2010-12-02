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
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/staticres.h"

#include "common/file.h"

namespace Asylum {

#define KEYWORD_MASK 0xFFF

Encounter::Encounter(AsylumEngine *engine) : _vm(engine),
	_index(0), _keywordIndex(0), _item(NULL), _objectId1(kObjectNone), _objectId2(kObjectNone), _actorIndex(kActorInvalid),
	_flag1(false), _flag2(false) {

	// TODO init rest of members

	load();
}

Encounter::~Encounter() {
	// Pointing to existing data
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

void Encounter::initData() {
	memset(&_keywordIndexes, -1, sizeof(_keywordIndexes));

	uint32 currentIndex = 0;

	for (uint i = 0; i < 50; i++)
		if (_item->keywords[i] & KEYWORD_MASK)
			if (!(BYTE1(_item->keywords[i]) & 0x20))
				_keywordIndexes[currentIndex++] = i;

	for (uint i = 0; i < 50; i++)
		if (_item->keywords[i] & KEYWORD_MASK)
			if (!(BYTE1(_item->keywords[i]) & 0x20))
				_keywordIndexes[currentIndex++] = i;
}

void Encounter::initBackground() {
	_background.resourceId = getWorld()->encounterFrameBg;
	_background.frameIndex = 0;
	_background.frameCount = GraphicResource::getFrameCount(_vm, _background.resourceId);
	_background.rect = GraphicResource::getFrameRect(_vm, _background.resourceId, _background.frameCount - 1);

	Common::Point point;
	Actor *player = getScene()->getActor();
	player->adjustCoordinates(&point);
	_point = Common::Point(15, (point.y < 240) ? 464 - _background.rect.height() : 15);

	_background.transTableNum = 1;
}

void Encounter::initPortraits() {
	// Portrait 1
	if (_index == 18)
		_portrait1.resourceId = getWorld()->graphicResourceIds[51];
	else
		_portrait1.resourceId = getWorld()->graphicResourceIds[encounterPortrait1Index[getWorld()->chapter == kChapter9 ? getWorld()->actorType + 9 : getWorld()->chapter]];

	if (_portrait1.resourceId == kResourceInvalid && getWorld()->chapter == kChapter1)
		_portrait1.resourceId = getWorld()->graphicResourceIds[36];

	if (_portrait1.resourceId == kResourceInvalid)
		error("[Encounter::initPortraits] No portrait 1 for this encounter!");

	_portrait1.frameIndex = 0;
	_portrait1.frameCount = GraphicResource::getFrameCount(_vm, _portrait1.resourceId);
	_portrait1.rect = GraphicResource::getFrameRect(_vm, _portrait1.resourceId, 0);
	_portrait1.transTableNum = 0;
	_portrait1.transTableMax = 3;
	_portrait1.speech0 = 0;

	// Portrait 2
	if (_index != 59)
		_portrait2.resourceId = getWorld()->graphicResourceIds[encounterPortrait2Index[_index]];
	else if (_vm->isGameFlagSet(kGameFlag353))
		_portrait2.resourceId = getWorld()->graphicResourceIds[16];
	else if (_vm->isGameFlagSet(kGameFlag354))
		_portrait2.resourceId = getWorld()->graphicResourceIds[23];
	else if (_vm->isGameFlagSet(kGameFlag355))
		_portrait2.resourceId = getWorld()->graphicResourceIds[24];

	if (_portrait2.resourceId == kResourceInvalid && getWorld()->chapter == kChapter1)
		_portrait2.resourceId = getWorld()->graphicResourceIds[36];

	if (_portrait2.resourceId == kResourceInvalid)
		error("[Encounter::initPortraits] No portrait 2 for this encounter!");

	_portrait2.frameIndex = 0;
	_portrait2.frameCount = GraphicResource::getFrameCount(_vm, _portrait1.resourceId);
	_portrait2.rect = GraphicResource::getFrameRect(_vm, _portrait1.resourceId, 0);
	_portrait2.transTableNum = 0;
	_portrait2.transTableMax = 0;
	_portrait2.speech0 = 0;
}

void Encounter::initDrawStructs() {
	for (uint i = 0; i < ARRAYSIZE(_drawingStructs); i++) {
		_drawingStructs[i].resourceId = (i == 0) ? getWorld()->smallCurUp : getWorld()->smallCurDown;
		_drawingStructs[i].frameIndex = 0;
		_drawingStructs[i].status = 0;
		_drawingStructs[i].transTableNum = -1;

		Common::Rect frameRect = GraphicResource::getFrameRect(_vm, _drawingStructs[i].resourceId, 0);
		_drawingStructs[i].point1 = Common::Point(frameRect.height(), frameRect.width());
		_drawingStructs[i].point2 = Common::Point(_point.x + _background.rect.width() + 10,
		                                          _point.y + (i == 0 ? 5 : _point.x + _background.rect.height() - 6));
	}
}

uint32 Encounter::findKeyword(EncounterItem *item, int16 keyword) {
	for (uint i = 0; i < ARRAYSIZE(item->keywords); i++) {
		if ((item->keywords[i] & KEYWORD_MASK) == keyword)
			return i;
	}

	error("[Encounter::findKeyword] Could not find a valid keyword!");
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
	_vm->switchEventHandler(this);
}

//////////////////////////////////////////////////////////////////////////
// Message handler
//////////////////////////////////////////////////////////////////////////
bool Encounter::handleEvent(const AsylumEvent &evt) {
	switch ((uint32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();
		break;

	case EVENT_ASYLUM_UPDATE:
		return update();
		break;

	case Common::EVENT_KEYDOWN:
		return key(evt);
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		return mouse(evt);
		break;
	}

	return false;
}

bool Encounter::init() {
	if (getSound()->getMusicVolume() != Config.musicVolume - 500)
		getSound()->setMusicVolume(Config.musicVolume - 500);

	if (!getSharedData()->getMatteBarHeight()) {
		_flag6 = true;
		_data_455BD4 = 0;
		_data_455BD8 = 0;
		_data_455BDC = 0;
		_data_455BE0 = 0;
		_data_455BE4 = 0;
		_data_455BCC = 0;
		_data_455B3C = 1;
		_rectIndex = -1;
		_value1 = 0;
		_data_455BF4 = 0;
		_data_455BF8 = 0;
		_data_455B14 = -1;

		getSpeech()->resetTextData();

		initData();

		getText()->loadFont(getWorld()->font1);

		initBackground();
		initPortraits();
		initDrawStructs();
	}

	_data_455BD0 = 0;
	getCursor()->set(getWorld()->curTalkNPC, -1, 2);

	if (!getSharedData()->getMatteBarHeight())
		initScript(_item->scriptResourceId);

	return true;
}

bool Encounter::update() {
	if (getSound()->getMusicVolume() != Config.musicVolume - 500)
		getSound()->setMusicVolume(Config.musicVolume - 500);

	error("[Encounter::update] Not implemented!");
}

bool Encounter::key(const AsylumEvent &evt) {
	error("[Encounter::key] Not implemented!");
}

bool Encounter::mouse(const AsylumEvent &evt) {
	error("[Encounter::mouse] Not implemented!");
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
// Speech
//////////////////////////////////////////////////////////////////////////
void Encounter::resetSpeech(uint32 keywordIndex, uint32 a2) {
	getSpeech()->resetTextData();
	setupPortraits();

	_data_455BCC = 0;
	_data_455B3C = 0;

	if (keywordIndex) {
		getSpeech()->setTextResourceId(keywordIndex + a2);
		setupSpeechText();
	}
}

void Encounter::setupPortraits() {
	_portrait1.transTableMax = 3;
	_portrait2.transTableMax = 0;
	setupSpeechData('N', &_portrait1);
	setupSpeechData('N', &_portrait2);
}

void Encounter::setupSpeechData(char val, EncounterGraphic *encounterGraphic) {
	switch (val) {
	default:
		break;

	case 'N':
		encounterGraphic->speech3 = 0;
		break;

	case 'H':
		encounterGraphic->speech3 = 1;
		break;

	case 'E':
		encounterGraphic->speech3 = 2;
		break;

	case 'S':
		encounterGraphic->speech3 = 3;
		break;
	}
}

void Encounter::setupSpeechText() {
	setupSpeechData('N', &_portrait1);
	setupSpeechData('N', &_portrait2);

	char *text = getText()->get(getSpeech()->getTextResourceId());

	if (*text == '{') {
		_portrait1.transTableMax = 3;
		_portrait2.transTableMax = 0;

		getSpeech()->setTextData(text + 3);
		getSpeech()->setTextDataPos(NULL);

		setupSpeech(getSpeech()->getTextResourceId(), getWorld()->font1);
	} else {
		_portrait1.transTableMax = 0;
		_portrait2.transTableMax = 3;

		getSpeech()->setTextData(NULL);
		if (*text == '/')
			getSpeech()->setTextDataPos(text + 2);

		setupSpeech(getSpeech()->getTextResourceId(), getWorld()->font3);
	}

	_data_455BCC = 0;
	_data_455B3C = 1;
}

void Encounter::setupSpeech(ResourceId textResourceId, ResourceId fontResourceId) {
	getText()->loadFont(fontResourceId);

	char *text = getText()->get(textResourceId);

	if (*text == '{') {
		_data_455BDC = 1;
		setupEntities(true);
		setupSpeechData(text[1], &_portrait1);
	} else {
		_data_455BDC = 0;
		setupEntities(false);
		if (*text == '/') {
			char *c = text + 1;
			while (*c != ' ') {
				setupSpeechData(*(c + 1), &_portrait2);
				c++;
			}
		}
	}

	_data_455BE0 = 1;
	getSpeech()->setSoundResourceId(MAKE_RESOURCE(kResourcePackSharedSound, textResourceId - _keywordIndex));
}

bool Encounter::isSpeaking() {
	if (!_flag4)
		return false;

	if (getSpeech()->getSoundResourceId() != kResourceNone && getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
		getSound()->stopAll(getSpeech()->getSoundResourceId());
		return true;
	}

	if (getSpeech()->getTick()) {
		getSpeech()->setTick(_vm->getTick());
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Actor & Object
//////////////////////////////////////////////////////////////////////////
void Encounter::setupEntities(bool type4) {
	// Actor
	if (_actorIndex) {
		Actor *actor = getScene()->getActor(_actorIndex);

		if (actor->isDefaultDirection(20))
			actor->updateStatus(type4 ? kActorStatusDisabled : kActorStatus8);

		return;
	}

	// Objects
	if (_objectId1 == _objectId2)
		return;

	if (_index == 37) {
		if (getVariable(3)) {
			if (getVariable(3) == 1 && _vm->isGameFlagNotSet(kGameFlag470)) {
				getWorld()->getObjectById(type4 ? kObjectNPC026Talking : kObjectNPC026TalkStatusQuo)->disable();
				getWorld()->getObjectById(type4 ? kObjectNPC026TalkStatusQuo : kObjectNPC026Talking)->setNextFrame(getWorld()->getObjectById(type4 ? kObjectNPC026TalkStatusQuo : kObjectNPC026Talking)->flags);
			}
		} else {
			getWorld()->getObjectById(type4 ? _objectId1 : _objectId2)->disable();
			getWorld()->getObjectById(type4 ? _objectId2 : _objectId1)->setNextFrame(getWorld()->getObjectById(type4 ? _objectId2 : _objectId1)->flags);
		}
	} else {
		if (type4) {
			getWorld()->getObjectById(_objectId1)->disable();
			getWorld()->getObjectById(_objectId2)->setNextFrame(getWorld()->getObjectById(_objectId2)->flags);
		} else {
			if (_index == 5) {
				if (!_vm->isGameFlagSet(kGameFlag262)) {
					getWorld()->getObjectById(kObjectMariaPointsLeft)->disable();
					getWorld()->getObjectById(_objectId2)->disable();

					_objectId1 = kObjectMariaPointsRight;
					getWorld()->getObjectById(_objectId1)->setNextFrame(getWorld()->getObjectById(_objectId1)->flags);
				} else {
					getWorld()->getObjectById(_objectId1)->disable();
					getWorld()->getObjectById(_objectId2)->disable();

					_objectId1 = kObjectMariaPointsLeft;
					getWorld()->getObjectById(_objectId1)->setNextFrame(getWorld()->getObjectById(_objectId1)->flags);
				}
			} else {
				getWorld()->getObjectById(_objectId2)->disable();
				getWorld()->getObjectById(_objectId1)->setNextFrame(getWorld()->getObjectById(_objectId1)->flags);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Scripts
//////////////////////////////////////////////////////////////////////////
void Encounter::initScript(ResourceId resourceId) {
	_scriptData.reset(resourceId);
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
				getScreen()->makeGreyPalette();
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
