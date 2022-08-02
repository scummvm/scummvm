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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/resources/encounters.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/staticres.h"

namespace Asylum {

#define KEYWORD_MASK 0xFFF

#define OPCODE_NAME(index) (index > 25 ? "INVALID" : opcodeNames[index])
const char *opcodeNames[] = {
	"Return",
	"SetScriptVariable",
	"SetCounterFromVars",
	"SetOffset",
	"SetOffsetIfCounterNegative",
	"SetOffsetIfCounterNegativeOrNull",
	"SetOffsetIfCounterIsNull",
	"SetOffsetIfCounterIsNotNull",
	"SetOffsetIfCounterPositiveOrNull",
	"SetOffsetIfCounterPositive",
	"SetCurrentItemOptions",
	"ClearCurrentItemOptions",
	"SetItemOptions",
	"CloseDialog",
	"ResetSpeech",
	"SetVariable",
	"IncrementScriptVariable",
	"ProcessVariable3",
	"AddRemoveInventoryItem",
	"UNUSED (19)",
	"UNUSED (20)",
	"SetCounterIfInventoryOmits",
	"UNUSED (22)",
	"PrepareMovie",
	"SetClearGameFlag",
	"SetCounterFromGameFlag"
};

static const struct {
	int textResourceId;
	int millis;
} tickDecrements[] = {
	{0x0A8C, 1000},
	{0x0CFC, 5000}
};

Common::String Encounter::ScriptEntry::toString() {
	return Common::String::format("0x%02X: %s (%d, %d)", opcode, OPCODE_NAME(opcode), param1, param2);
}

Encounter::Encounter(AsylumEngine *engine) : _vm(engine),
	_index(0), _speechResourceId(0), _soundResourceId(kResourceNone), _item(NULL), _objectId1(kObjectNone), _objectId2(kObjectNone), _objectId3(kObjectNone),
	_actorIndex(kActorInvalid), _shouldEnablePlayer(false), _wasPlayerDisabled(false), _isClosing(false), _isScriptRunning(false) {

	memset(&_keywordIndexes, 0, sizeof(_keywordIndexes));
	_rectIndex = -1;

	_value1 = 0;
	_tick = 0;
	_data_455B14 = 0;
	_data_455B3C = 0;
	_data_455B70 = 0;
	_data_455BCC = false;
	_isDialogOpen = false;
	_shouldCloseDialog = false;
	_data_455BD8 = false;
	_data_455BDC = false;
	_data_455BE0 = false;
	_shouldCloseBackground = false;
	_data_455BE8 = false;
	_data_455BF0 = 0;
	_data_455BF4 = 0;
	_keywordStartIndex = 0;

	if (_vm->checkGameVersion("Demo")) {
		_keywordsOffset = 204;
	} else {
		switch (_vm->getLanguage()) {
		default:
		case Common::EN_ANY:
			_keywordsOffset = 3681;
			break;

		case Common::DE_DEU:
			_keywordsOffset = 1760;
			break;

		case Common::FR_FRA:
			_keywordsOffset = 1741;
			break;
		}
	}

	load();
}

//////////////////////////////////////////////////////////////////////////
// Data
//////////////////////////////////////////////////////////////////////////
void Encounter::load() {
	Common::File file;

	if (!file.open("sntrm.dat"))
		if (!file.open("asylum.dat"))
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

		item.speechResourceId = file.readSint32LE();
		item.scriptResourceId = (ResourceId)file.readSint32LE();

		for (uint j = 0; j < ARRAYSIZE(item.keywords); j++)
			item.keywords[j] = file.readSint16LE();

		item.variable2 = file.readByte();

		_items.push_back(item);
	}

	file.close();
}

void Encounter::initData() {
	memset(&_keywordIndexes, -1, sizeof(_keywordIndexes));

	uint32 currentIndex = 0;

	for (uint i = 0; i < 50; i++) {
		if (_item->keywords[i] & KEYWORD_MASK) {
			if (!isKeywordDisabled(_item->keywords[i])) {
				_keywordIndexes[currentIndex] = i;
				currentIndex++;
			}
		}
	}

	for (uint i = 0; i < 50; i++) {
		if (_item->keywords[i] & KEYWORD_MASK) {
			if (isKeywordDisabled(_item->keywords[i])) {
				_keywordIndexes[currentIndex] = i;
				currentIndex++;
			}
		}
	}
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

	if (_vm->checkGameVersion("Demo")) {
		if (_index == 1)
			_portrait2.resourceId = getWorld()->graphicResourceIds[35];
		else
			_portrait2.resourceId = getWorld()->graphicResourceIds[34];
	}

	if (_portrait2.resourceId == kResourceInvalid)
		error("[Encounter::initPortraits] No portrait 2 for this encounter!");

	_portrait2.frameIndex = 0;
	_portrait2.frameCount = GraphicResource::getFrameCount(_vm, _portrait2.resourceId);
	_portrait2.rect = GraphicResource::getFrameRect(_vm, _portrait2.resourceId, 0);
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

//////////////////////////////////////////////////////////////////////////
// Run
//////////////////////////////////////////////////////////////////////////
void Encounter::run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex) {
	debugC(kDebugLevelEncounter, "[Encounter] Running Encounter %d: object1 = %d object2 = %d actor = %d",
			encounterIndex, objectId1, objectId2, actorIndex);

	if (!_speechResourceId) {
		_item = &_items[0];
		_speechResourceId = _item->speechResourceId;
	}

	if (encounterIndex < 0)
		return;

	// Original engine saves the event handler passed as first parameter (to be restored later)
	// Since it always passes the current scene instance, we restore it at the end and skip
	// saving it here
	_index = encounterIndex;
	_item = &_items[_index];
	_objectId1 = objectId1;
	_objectId2 = objectId2;
	_actorIndex = actorIndex;

	if (getWorld()->getObjectById(objectId2))
		getWorld()->getObjectById(objectId2)->stopSound();

	getScene()->getActor(actorIndex)->stopSound();

	setVariable(1, 0);
	setVariable(2, _item->variable2);

	// Disable the player and store the previous state as a flag
	Actor *player = getScene()->getActor();
	if (player->getStatus() == kActorStatusDisabled) {
		_wasPlayerDisabled = true;
	} else {
		_wasPlayerDisabled = false;
		player->changeStatus(kActorStatusDisabled);
	}

	_shouldEnablePlayer = false;

	// Setup encounter event handler
	_vm->switchEventHandler(this);
}

void Encounter::exitEncounter() {
	setVariable(1, 32767);
	initScript(_item->scriptResourceId);
	_isClosing = true;
	runScript();

	setupEntities(true);

	++_item->variable2;
	// Original saves the modified item back here (we are using a reference to it all along)

	// Update flags
	getSharedData()->setFlag(kFlagIsEncounterRunning, false);
	getSharedData()->setFlag(kFlag3, true);

	// Check if we need to re-enable the player
	if (_wasPlayerDisabled)
		_wasPlayerDisabled = false;
	else
		_shouldEnablePlayer = true;

	if (getSharedData()->getFlag(kFlagEncounterDisablePlayerOnExit))
		getScene()->getActor()->changeStatus(kActorStatusDisabled);

	getSharedData()->setFlag(kFlagEncounterDisablePlayerOnExit, false);

	if (getSound()->getMusicVolume() != Config.musicVolume)
		getSound()->setMusicVolume(Config.musicVolume);

	// Restore scene event handler
	_vm->switchEventHandler(getScene());
}

//////////////////////////////////////////////////////////////////////////
// Message handler
//////////////////////////////////////////////////////////////////////////
bool Encounter::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();

	case EVENT_ASYLUM_UPDATE:
		return update();

	case Common::EVENT_KEYDOWN:
		return key(evt);

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		return mouse(evt);

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		if ((AsylumAction)evt.customType == kAsylumActionShowMenu) {
			if (!isSpeaking()
			 && _isDialogOpen
			 && !getSpeech()->getTextData()
			 && !getSpeech()->getTextDataPos())
				_shouldCloseDialog = true;
		}
		return true;
	}

	return false;
}

bool Encounter::init() {
	// Lower music volume while we are in the encounter
	if (getSound()->getMusicVolume() != Config.musicVolume - 500)
		getSound()->setMusicVolume(Config.musicVolume - 500);

	if (!getSharedData()->getMatteBarHeight()) {
		getSharedData()->setFlag(kFlagIsEncounterRunning, true);
		_shouldCloseDialog = false;
		_data_455BD8 = false;
		_data_455BDC = false;
		_data_455BE0 = false;
		_shouldCloseBackground = false;
		_data_455BCC = false;
		_data_455B3C = 1;
		_rectIndex = -1;
		_value1 = 0;
		_data_455BF4 = 0;
		_keywordStartIndex = 0;
		_data_455B14 = -1;

		getSpeech()->resetTextData();

		initData();

		getText()->loadFont(getWorld()->font1);

		initBackground();
		initPortraits();
		initDrawStructs();
	}

	_isDialogOpen = false;
	getCursor()->set(getWorld()->cursorResources[kCursorResourceTalkNPC], -1, kCursorAnimationMirror);

	if (!getSharedData()->getMatteBarHeight())
		initScript(_item->scriptResourceId);

	return true;
}

bool Encounter::update() {
	// Check that the music volume is lowered
	if (getSound()->getMusicVolume() != Config.musicVolume - 500)
		getSound()->setMusicVolume(Config.musicVolume - 500);

	uint32 tick = _vm->getTick();
	ResourceId id = kResourceNone;

	if (_objectId3) {
		_isDialogOpen = false;

		Object *object = getWorld()->getObjectById(_objectId3);
		id = object->getResourceId();

		if (object->getFrameIndex() == object->getFrameCount() - 1) {
			switch (getVariable(3)) {
			default:
				break;

			case 13:
				if (_actorIndex)
					getScene()->getActor(_actorIndex)->show();
				else
					setupEntities(true);
				break;

			case 14:
				getScene()->getActor()->show();
				break;

			case 15:
				if (_actorIndex)
					getScene()->getActor(_actorIndex)->show();
				else
					setupEntities(true);

				getScene()->getActor()->show();
				break;
			}

			object->disable();
			_objectId3 = kObjectNone;
			_data_455BF4 = 0;
			getCursor()->show();
		}
	}

	if (_data_455BE8) {
		if (getSharedData()->getMatteBarHeight()) {
			_isDialogOpen = false;
		} else {
			getCursor()->show();
			_data_455BE8 = false;
			_data_455BF4 = 0;
		}
	}

	if (_isDialogOpen) {
		if (_data_455BF4 == 1) {
			_data_455BF4 = 2;
			runScript();
		}

		bool doScript = false;
		if ((_soundResourceId/*getSpeech()->getSoundResourceId()*/ && !getSound()->isPlaying(_soundResourceId/*getSpeech()->getSoundResourceId()*/) && !_data_455BE0) // TODO replace with speech
		 /*|| (getSpeech()->getTick() && tick >= getSpeech()->getTick())*/)
			doScript = true;

		if (doScript
		 && !getSharedData()->getMatteBarHeight()
		 && _isScriptRunning
		 && !setupSpeechTest(id))
			runScript();
	}

	// Redraw screen
	if (!getSharedData()->getFlag(kFlagRedraw)) {
		if (updateScreen())
			return true;

		getSharedData()->setFlag(kFlagRedraw, true);
	}

	if (tick >= getSharedData()->getNextScreenUpdate() && getSharedData()->getFlag(kFlagRedraw)) {
		if (getSharedData()->getMatteBarHeight() <= 0) {
			getScreen()->copyBackBufferToScreen();
		} else {
			drawScreen();
		}

		getSharedData()->setFlag(kFlagRedraw, false);
		getSharedData()->setNextScreenUpdate(tick + 55);
	}

	return true;
}

bool Encounter::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_LBUTTONDOWN:
		if (!_data_455BD8) {
			_data_455BD8 = true;
			_rectIndex = findRect();

			if (_rectIndex != -1)
				updateDrawingStatus2(_rectIndex);
		}
		break;

	case Common::EVENT_LBUTTONUP:
		if (_rectIndex == -1) {
			if (!isSpeaking())
				choose(getKeywordIndex());

			_data_455BD8 = false;
		} else {
			_rectIndex = -1;
			updateDrawingStatus1(_rectIndex);
			_data_455BD8 = false;
		}
		break;


	case Common::EVENT_RBUTTONDOWN:
		if (!isSpeaking()
		 && _isDialogOpen
		 && !getSpeech()->getTextData()
		 && !getSpeech()->getTextDataPos())
			_shouldCloseDialog = true;
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////
void Encounter::setVariable(uint32 index, int16 val) {
	if (index >= _variables.size())
		error("[Encounter::setVariable] Invalid index (was: %d, max: %d)", index, _variables.size() - 1);

	_variables[index] = val;
}

int16 Encounter::getVariable(uint32 index) {
	if (index >= _variables.size())
		error("[Encounter::getVariable] Invalid index (was: %d, max: %d)", index, _variables.size() - 1);

	return _variables[index];
}

int16 Encounter::getVariableInv(int16 index) {
	if (index >= 0)
		return index;

	return getVariable((uint16)-index);
}

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////
uint32 Encounter::findKeyword(EncounterItem *item, int16 keyword) const {
	for (uint i = 0; i < ARRAYSIZE(item->keywords); i++) {
		if ((item->keywords[i] & KEYWORD_MASK) == keyword)
			return i;
	}

	error("[Encounter::findKeyword] Could not find a valid keyword!");
}

int32 Encounter::getKeywordIndex() {
	Common::Point mousePos = getCursor()->position();

	int16 counter = 0;
	for (uint i = _keywordStartIndex; i < ARRAYSIZE(_keywordIndexes); i++) {
		int32 index = _keywordIndexes[i];

		if (counter / 3 >= 8)
			break;

		if ((_item->keywords[index] & KEYWORD_MASK) > 0 && isKeywordVisible(_item->keywords[index])) {
			int32 x = _drawingStructs[0].point1.y + 145 * (counter % 3) + _point.x + _portrait1.rect.width() + 15;
			int32 y = 16 * (counter / 3) + _point.y + 5;

			if (mousePos.x >= x && mousePos.x <= (x + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, _keywordsOffset + (_item->keywords[index] & KEYWORD_MASK))))
			 && mousePos.y >= y && mousePos.y <= (y + 16))
				return index;

			++counter;
		}
	}

	return -1;
}

void Encounter::choose(int32 index) {
	if (_isScriptRunning || index == -1)
		return;

	if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index])) {

		_value1 = (_item->keywords[index] & KEYWORD_MASK);
		setVariable(1, _value1);

		const char *goodBye;
		switch (_vm->getLanguage()) {
		default:
		case Common::EN_ANY:
			goodBye = "Goodbye";
			break;

		case Common::DE_DEU:
			goodBye = "Auf Wiedersehen";
			break;

		case Common::FR_FRA:
			goodBye = "Au Revoir";
			break;

		case Common::RU_RUS:
			goodBye = "\xc4\xee\x20\xf1\xe2\xe8\xe4\xe0\xed\xe8\xff"; // До свидания
			break;
		}

		if (strcmp(goodBye, getText()->get(MAKE_RESOURCE(kResourcePackText, _keywordsOffset + _value1))))
			if (_index != 79)
				BYTE1(_item->keywords[index]) |= kKeywordOptionsDisabled;

		initScript(_item->scriptResourceId);
		runScript();
	}
}

bool Encounter::checkKeywords() const {
	if (_data_455B14 == -1 || _data_455B14 + 1 >= 50)
		return false;

	for (int32 i = _data_455B14 + 1; i < ARRAYSIZE(_keywordIndexes); i++) {
		int32 index = _keywordIndexes[i];

		if (index < 0)
			continue;

		if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index]))
			return true;
	}

	return false;
}

bool Encounter::checkKeywords2() const {
	for (uint32 i = 0; i < _keywordStartIndex; i++) {
		int32 index = _keywordIndexes[i];

		if (index < 0)
			continue;

		if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index]))
			return true;
	}

	return false;
}

void Encounter::updateFromRect(int32 rectIndex)  {
	if (rectIndex) {
		if (rectIndex == 1 && (_data_455B14 + 1) < 50) {
			bool cont = false;

			for (int32 i = _data_455B14 + 1; i < ARRAYSIZE(_keywordIndexes); i++) {
				int32 index = _keywordIndexes[i];

				if (index < 0)
					continue;

				if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index])) {
					cont = true;
					break;
				}
			}

			if (!cont)
				return;

			uint32 counter = 0;
			for (uint32 i = _keywordStartIndex + 1; i < ARRAYSIZE(_keywordIndexes); i++) {
				int32 index = _keywordIndexes[i];

				if (counter == 3)
					break;

				if (index < 0)
					continue;

				if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index])) {
					_keywordStartIndex = i;
					++counter;
				}
			}
		}
	} else {
		uint32 counter = 0;
		for (int32 i = (int32)_keywordStartIndex - 1; i > -1; i--) {
			int32 index = _keywordIndexes[i];

			if (counter == 3)
				break;

			if (index < 0)
				continue;

			if ((_item->keywords[index] & KEYWORD_MASK) && isKeywordVisible(_item->keywords[index])) {
				_keywordStartIndex = (uint32)i;
				++counter;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Speech
//////////////////////////////////////////////////////////////////////////
void Encounter::resetSpeech(int16 keywordIndex, int16 a2) {
	getSpeech()->resetTextData();
	setupPortraits();

	_data_455BCC = false;
	_data_455B3C = 1;

	if (keywordIndex || _vm->checkGameVersion("Demo")) {
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

void Encounter::setupSpeechData(char val, EncounterGraphic *encounterGraphic) const {
	switch (val) {
	default:
		break;

	case 'N':
		encounterGraphic->frameIndex = 0;
		break;

	case 'H':
		encounterGraphic->frameIndex = 1;
		break;

	case 'E':
		encounterGraphic->frameIndex = 2;
		break;

	case 'S':
		encounterGraphic->frameIndex = 3;
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
		getSpeech()->setTextDataPos(text);
		if (*text == '/')
			getSpeech()->setTextDataPos(text + 2);

		setupSpeech(getSpeech()->getTextResourceId(), getWorld()->font3);
	}

	_data_455BCC = false;
	_data_455B3C = 1;
}

void Encounter::setupSpeech(ResourceId textResourceId, ResourceId fontResourceId) {
	getText()->loadFont(fontResourceId);

	char *text = getText()->get(textResourceId);

	if (*text == '{') {
		_data_455BDC = true;
		setupEntities(true);
		setupSpeechData(text[1], &_portrait1);
	} else {
		_data_455BDC = false;
		setupEntities(false);
		if (*text == '/') {
			char *c = text + 1;
			while (*c != ' ') {
				setupSpeechData(*(c + 1), &_portrait2);
				c++;
			}
		}
	}

	_data_455BE0 = true;
	_soundResourceId = MAKE_RESOURCE(kResourcePackSharedSound, textResourceId - _speechResourceId + _vm->checkGameVersion("Demo") + 7 * _vm->isAltDemo());
	//_soundResourceId = MAKE_RESOURCE(kResourcePackSpeech, textResourceId - _speechResourceId); // todo replace with speech (this is being override by the actor speechs)
	//getSpeech()->setSoundResourceId(MAKE_RESOURCE(kResourcePackSpeech, textResourceId - _speechResourceId)); 
}

bool Encounter::setupSpeechTest(ResourceId id) {
	getSpeech()->setTick(0);
	_soundResourceId = kResourceNone; // TODO replace with speech (this is being override by the actor speechs)
	//getSpeech()->setSoundResourceId(kResourceNone);

	setupEntities(false);

	if (id == kResourceNone)
		id = getSpeech()->getTextResourceId();

	char *text = getText()->get(id);
	if (text[strlen(text) - 1] == 1) {
		setupEntities(true);
		getSpeech()->setTextResourceId(kResourceNone);
		getSpeech()->setTextData(NULL);
		getSpeech()->setTextDataPos(NULL);

		_data_455BCC = false;
		_data_455B3C = 1;

		setupPortraits();

		return false;
	}

	getSpeech()->setTextResourceId(getSpeech()->getTextResourceId() + 1);
	setupSpeechText();

	return true;
}

bool Encounter::isSpeaking() {
	if (!_isScriptRunning)
		return false;

	if (_soundResourceId/*getSpeech()->getSoundResourceId()*/ != kResourceNone && getSound()->isPlaying(_soundResourceId/*getSpeech()->getSoundResourceId()*/)) { // TODO replace with speech
		getSound()->stopAll(_soundResourceId);
		return true;
	}

	if (getSpeech()->getTick()) {
		getSpeech()->setTick(_vm->getTick());
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
bool Encounter::drawBackground() {
	if (Config.performance > 1) {
		if (_vm->isGameFlagSet(kGameFlag528)) {
			Common::Point origin;

			getScreen()->addGraphicToQueueCrossfade(_background.resourceId,
			                                        _background.frameIndex,
			                                        _point,
			                                        getWorld()->getObjectById(kObjectCave)->getResourceId(),
			                                        origin,
			                                        (uint32)_background.transTableNum);
		} else {
			Common::Point origin(getWorld()->xLeft, getWorld()->yTop);

			getScreen()->addGraphicToQueueCrossfade(_background.resourceId,
			                                        _background.frameIndex,
			                                        _point,
			                                        getWorld()->backgroundImage,
			                                        origin,
			                                        (uint32)_background.transTableNum);
		}
	} else {
		getScreen()->draw(_background.resourceId, _background.frameIndex, _point);
	}

	if (_shouldCloseBackground) {
		if (_background.frameIndex == 0)
			exitEncounter();
		else
			--_background.frameIndex;

		return false;
	}

	if (_background.frameIndex < _background.frameCount - 1) {
		++_background.frameIndex;

		return false;
	}

	return true;
}

bool Encounter::drawPortraits() {
	bool ret = true;

	if (_shouldCloseDialog) {
		_portrait1.transTableMax = 0;
		_portrait2.transTableMax = 0;
	}

	// Portrait 1
	if (_portrait1.transTableNum == 3)
		getScreen()->draw(_portrait1.resourceId,
		                  _portrait1.frameIndex,
		                  _point + Common::Point(5, 5));
	else
		getScreen()->drawTransparent(_portrait1.resourceId,
		                             _portrait1.frameIndex,
		                             _point + Common::Point(5, 5),
		                             kDrawFlagNone,
		                             (uint32)_portrait1.transTableNum);

	if (_portrait1.transTableNum != _portrait1.transTableMax) {
		if (_portrait1.transTableNum <= _portrait1.transTableMax)
			++_portrait1.transTableNum;
		else
			--_portrait1.transTableNum;

		ret = false;
	}

	if (_portrait1.speech0) {
		if (_portrait1.speech0 == 1)
			++_portrait1.frameIndex;
		else
			--_portrait1.frameIndex;

		_portrait1.frameIndex %= _portrait1.frameCount;
	}

	// Portrait 2
	Common::Rect frameRect = GraphicResource::getFrameRect(_vm, _portrait2.resourceId, _portrait2.frameIndex);

	if (_portrait2.transTableNum == 3)
		getScreen()->draw(_portrait2.resourceId,
		                  _portrait2.frameIndex,
		                  Common::Point(_point.x + _background.rect.width() - (frameRect.width() + 6), _point.y + 5));
	else
		getScreen()->drawTransparent(_portrait2.resourceId,
		                             _portrait2.frameIndex,
		                             Common::Point(_point.x + _background.rect.width() - (frameRect.width() + 6), _point.y + 5),
		                             kDrawFlagNone,
		                             (uint32)_portrait2.transTableNum);

	if (_portrait2.transTableNum != _portrait2.transTableMax) {
		if (_portrait2.transTableNum <= _portrait2.transTableMax)
			++_portrait2.transTableNum;
		else
			--_portrait2.transTableNum;

		ret = false;
	}

	if (_portrait2.speech0) {
		if (_portrait2.speech0 == 1)
			++_portrait2.frameIndex;
		else
			--_portrait2.frameIndex;

		_portrait2.frameIndex %= _portrait2.frameCount;
	}

	if (_shouldCloseDialog)
		if (_portrait1.transTableNum == _portrait1.transTableMax
		 && _portrait2.transTableNum == _portrait2.transTableMax)
			_shouldCloseBackground = true;

	return ret;
}

void Encounter::drawStructs() {
	// Drawing structure 1
	if (_drawingStructs[0].transTableNum < -1 || _drawingStructs[0].transTableNum > 3)
		error("[Encounter::drawStructs] Something got <redacted> wrong!");

	if (checkKeywords2() || _drawingStructs[0].transTableNum > -1) {
		int32 val = _drawingStructs[0].transTableNum;

		if (_shouldCloseDialog
		 && _drawingStructs[0].status != 2
		 && _drawingStructs[0].transTableNum > -1) {
			val = _drawingStructs[0].transTableNum - 1;
			_drawingStructs[0].status = 2;
			--_drawingStructs[0].transTableNum;
		}

		switch (val) {
		default:
			break;

		case -1:
			getScreen()->drawTransparent(_drawingStructs[0].resourceId,
			                             _drawingStructs[0].frameIndex,
			                             _drawingStructs[0].point2,
			                             kDrawFlagNone,
			                             0);

			_drawingStructs[0].status = 0;
			break;

		case 0:
		case 1:
		case 2:
			getScreen()->drawTransparent(_drawingStructs[0].resourceId,
			                             _drawingStructs[0].frameIndex,
			                             _drawingStructs[0].point2,
			                             kDrawFlagNone,
			                             (uint32)val);

			if (_drawingStructs[0].status == 1)
				++_drawingStructs[0].transTableNum;
			else if (_drawingStructs[0].status == 2)
				--_drawingStructs[0].transTableNum;

			break;

		case 3:
			getScreen()->draw(_drawingStructs[0].resourceId,
			                  _drawingStructs[0].frameIndex,
			                  _drawingStructs[0].point2);

			_drawingStructs[0].status = 0;
			break;
		}
	}

	// Drawing structure 2
	if (_drawingStructs[1].transTableNum < -1 || _drawingStructs[1].transTableNum > 3)
		error("[Encounter::drawStructs] Something got <redacted> wrong!");

	if (checkKeywords() || _drawingStructs[1].transTableNum > -1) {
		int32 val = _drawingStructs[1].transTableNum;

		if (_shouldCloseDialog
		 && _drawingStructs[1].status != 2
		 && _drawingStructs[1].transTableNum > -1) {
			val = _drawingStructs[1].transTableNum - 1;
			_drawingStructs[1].status = 2;
			--_drawingStructs[1].transTableNum;
		}

		switch (val) {
		default:
			break;

		case -1:
			getScreen()->drawTransparent(_drawingStructs[1].resourceId,
			                             _drawingStructs[1].frameIndex,
			                             _drawingStructs[1].point2,
			                             kDrawFlagNone,
			                             0);

			_drawingStructs[1].status = 0;
			break;

		case 0:
		case 1:
		case 2:
			getScreen()->drawTransparent(_drawingStructs[1].resourceId,
			                             _drawingStructs[1].frameIndex,
			                             _drawingStructs[1].point2,
			                             kDrawFlagNone,
			                             (uint32)val);

			if (_drawingStructs[1].status == 1)
				++_drawingStructs[1].transTableNum;
			else if (_drawingStructs[1].status == 2)
				--_drawingStructs[1].transTableNum;

			break;

		case 3:
			getScreen()->draw(_drawingStructs[1].resourceId,
			                  _drawingStructs[1].frameIndex,
			                  _drawingStructs[1].point2);

			_drawingStructs[1].status = 0;
			break;
		}
	}
}

void Encounter::drawDialogOptions() {
	getText()->loadFont(getWorld()->font1);

	int16 counter = 0;

	for (uint32 i = _keywordStartIndex; i < ARRAYSIZE(_keywordIndexes); i++) {
		if (counter / 3 >= 8)
			return;

		int32 keywordIndex = _keywordIndexes[i];
		if (keywordIndex < 0)
			continue;

		int16 keyword = _item->keywords[keywordIndex];
		if ((keyword & KEYWORD_MASK) > 0 && isKeywordVisible(keyword)) {

			if (isKeywordDisabled(keyword))
				getText()->loadFont(getWorld()->font2);
			else
				getText()->loadFont(getWorld()->font1);

			Common::Point coords(_drawingStructs[0].point1.y + 145 * (counter % 3) + _point.x + (counter % 3) + _portrait1.rect.width() + 15,
			                     _point.y + (int16)(16 * (counter / 3)));

			if (getKeywordIndex() == keywordIndex)
				getScreen()->fillRect(coords.x - 1, coords.y + 5, getText()->getWidth(MAKE_RESOURCE(kResourcePackText, _keywordsOffset + (keyword & KEYWORD_MASK))) + 2, 18, 0);

			getText()->setPosition(coords);
			getText()->draw(MAKE_RESOURCE(kResourcePackText, _keywordsOffset + (keyword & KEYWORD_MASK)));

			++counter;
			_data_455B14 = i;
		}
	}
}

void Encounter::drawSubtitle(char *text, ResourceId font, int16 y) {
	if (!text)
		return;

	int16 width = _background.rect.width() - (_portrait1.rect.width() + _portrait2.rect.width() + 20);
	int16 x = _point.x + _portrait1.rect.width() + 10;

	getText()->loadFont(font);

	if (_data_455BCC) {
		if (_data_455B3C != 1 && _tick < _vm->getTick()) {
			_tick = _vm->getTick() + 1000 * (getResource()->get(_soundResourceId/*getSpeech()->getSoundResourceId()*/)->size / 11025) / (uint16)_data_455B3C; // TODO replace with speech

			if ((_data_455BF0 + 8) < _data_455B70)
				_data_455BF0 += 8;
		}
	} else {
		_data_455BCC = true;
		_data_455B70 = getText()->draw(kTextCalculate, Common::Point(x, y), 16, width, text);
		_data_455B3C = _data_455B70 / 8 + 1;
		_data_455BF0 = 0;
		_tick = _vm->getTick() + 1000 * (getResource()->get(_soundResourceId/*getSpeech()->getSoundResourceId()*/)->size / 11025) / (uint16)_data_455B3C; // TODO replace with speech

		// WORKAROUND: Handle multipage dialogs
		for (int i = 0; i < ARRAYSIZE(tickDecrements); i++)
			if (getSpeech()->getTextResourceId() == tickDecrements[i].textResourceId) {
				_tick -= tickDecrements[i].millis;
				break;
			}
	}

	getText()->draw(_data_455BF0, 7, kTextCenter, Common::Point(x, y), 16, width, text);
}

void Encounter::drawScreen() {
	getScene()->getActor()->setLastScreenUpdate(_vm->screenUpdateCount);

	if (!getSharedData()->getMatteInitialized())
		getSharedData()->setMatteBarHeight(85);

	if (getSharedData()->getMatteBarHeight() >= 84) {
		if (getSharedData()->getMatteBarHeight() == 85) {
			if (getSharedData()->getMatteInitialized()) {
				getScreen()->drawWideScreenBars(82);

				getScreen()->updatePalette();
				getScreen()->setupPalette(NULL, 0, 0);
				getScreen()->paletteFade(0, 25, 10);
			} else {
				getSharedData()->setMatteInitialized(true);
				getScreen()->clear();
			}

			if (getSharedData()->getMatteVar1()) {
				if (!getSharedData()->getMatteVar2())
					getSound()->playMusic(kResourceNone, 0);

				// Play movie
				getScreen()->clear();
				getVideo()->play(getSharedData()->movieIndex, getSharedData()->getFlag(kFlagIsEncounterRunning) ? (EventHandler*)this : getScene());
				getScreen()->clearGraphicsInQueue();
				getScreen()->clear();
				getCursor()->hide();

				if (getSharedData()->getMattePlaySound()) {
					getScreen()->paletteFade(0, 2, 1);
					getScene()->updateScreen();
					getScreen()->drawWideScreenBars(82);

					getScreen()->updatePalette(0);
					getScreen()->setupPalette(NULL, 0, 0);

					if (getSharedData()->getMattePlaySound() /* Scene::updateScreen() does script processing, so the value might have changed */
					 && !getSharedData()->getMatteVar2()
					 && getWorld()->musicCurrentResourceIndex != kMusicStopped)
						getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicCurrentResourceIndex));
				}

				getSharedData()->setMatteBarHeight(getSharedData()->getMattePlaySound() ? 346 : 170);
			} else {
				getSharedData()->setMatteBarHeight(170);
			}
		} else if (getSharedData()->getMatteBarHeight() >= 170) {
			if (getSharedData()->getFlag(kFlagIsEncounterRunning)) {
				getSharedData()->setMatteBarHeight(0);
				getCursor()->show();
			}
		} else {
			getScreen()->drawWideScreenBars(172 - getSharedData()->getMatteBarHeight());
			getSharedData()->setMatteBarHeight(getSharedData()->getMatteBarHeight() + 4);

			ResourceId paletteId = getWorld()->actions[getScene()->getActor()->getActionIndex3()]->paletteResourceId;
			getScreen()->setPaletteGamma(paletteId ? paletteId : getWorld()->currentPaletteId);

			updatePalette1();
			getScreen()->setupPalette(NULL, 0, 0);
		}
	} else {
		getScreen()->drawWideScreenBars(getSharedData()->getMatteBarHeight());
		getSharedData()->setMatteBarHeight(getSharedData()->getMatteBarHeight() + 4);

		getScreen()->setPaletteGamma(getWorld()->currentPaletteId);

		updatePalette2();
		getScreen()->setupPalette(NULL, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Encounter::setupEntities(bool type4) {
	// Actor
	if (_actorIndex) {
		Actor *actor = getScene()->getActor(_actorIndex);

		if (actor->canChangeStatus(20))
			actor->changeStatus(type4 ? kActorStatusDisabled : kActorStatus8);

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

int32 Encounter::findRect() {
	Common::Point mousePos = getCursor()->position();

	for (uint32 i = 0; i < ARRAYSIZE(_drawingStructs); i++) {
		EncounterDrawingStruct *drawStruct = &_drawingStructs[i];

		if (mousePos.x >= drawStruct->point2.x
		 && mousePos.x < (drawStruct->point2.x + drawStruct->point1.y)
		 && mousePos.y >= drawStruct->point2.y
		 && mousePos.y < (drawStruct->point2.y + drawStruct->point1.x))
			return i;
	}

	return -1;
}

void Encounter::updateDrawingStatus() {
	if (checkKeywords2()) {
		if (_rectIndex) {
			if (_drawingStructs[0].transTableNum == -1) {
				_drawingStructs[0].status = 1;
				_drawingStructs[0].transTableNum = 0;
			}
		}
	} else {
		if (_drawingStructs[0].transTableNum == 3) {
			_drawingStructs[0].status = 2;
			_drawingStructs[0].transTableNum = 2;
		}
	}

	if (checkKeywords()) {
		if (_rectIndex != 1) {
			if (_drawingStructs[1].transTableNum == -1) {
				_drawingStructs[1].status = 1;
				_drawingStructs[1].transTableNum = 0;
			}
		}
	} else {
		if (_drawingStructs[1].transTableNum == 3) {
			_drawingStructs[1].status = 2;
			_drawingStructs[1].transTableNum = 2;
		}
	}
}

void Encounter::updateDrawingStatus1(int32 rectIndex) {
	switch (rectIndex) {
	default:
		error("[Encounter::updateDrawingStatus1] Invalid rect index (%d)", rectIndex);
		break;

	case 0:
		if (checkKeywords2()) {
		    _drawingStructs[rectIndex].status = 1;
		    ++_drawingStructs[rectIndex].transTableNum;
	    }
		break;

	case 1:
		if (checkKeywords()) {
		    _drawingStructs[rectIndex].status = 1;
		    ++_drawingStructs[rectIndex].transTableNum;
	    }
		break;
	}
}

void Encounter::updateDrawingStatus2(int32 rectIndex) {
	switch (rectIndex) {
	default:
		error("[Encounter::updateDrawingStatus1] Invalid rect index (%d)", rectIndex);
		break;

	case 0:
		if (checkKeywords2()) {
		    _drawingStructs[rectIndex].status = 2;
		    --_drawingStructs[rectIndex].transTableNum;
			updateFromRect(rectIndex);
	    }
		break;

	case 1:
		if (checkKeywords()) {
		    _drawingStructs[rectIndex].status = 2;
		    --_drawingStructs[rectIndex].transTableNum;
			updateFromRect(rectIndex);
	    }
		break;
	}
}

bool Encounter::updateScreen() {
	if (getScene()->updateScreen())
		return true;

	getText()->loadFont(getWorld()->font1);

	if (!drawBackground()) {
		_isDialogOpen = false;
		return false;
	}

	if (!drawPortraits()) {
		_isDialogOpen = false;

		if (_shouldCloseDialog)
			drawStructs();

		return false;
	}

	if (_isDialogOpen) {

		if (!getSpeech()->getTextDataPos() && !getSpeech()->getTextData()) {

			drawDialogOptions();
			updateDrawingStatus();
			drawStructs();

			if (_rectIndex != -1 && findRect() == _rectIndex)
				updateFromRect(_rectIndex);

			return false;
		}

		if (Config.showEncounterSubtitles) {
			drawSubtitle(getSpeech()->getTextDataPos(), getWorld()->font3, _point.y);
			drawSubtitle(getSpeech()->getTextData(), getWorld()->font1, _point.y);
		}

		if (_data_455BE0) {
			_data_455BE0 = false;

			// Check resources
			if (!_data_455BDC && !_objectId1 && !_objectId2 && !_actorIndex)
				error("[Encounter::updateScreen] Invalid encounter resources!");

			getSound()->playSound(_soundResourceId/*getSpeech()->getSoundResourceId()*/, false, Config.voiceVolume); // TODO replace with speech
		}

		return false;
	}

	if (_objectId3 || _data_455BE8)
		return false;

	if (!_data_455BF4)
		_data_455BF4 = 1;

	_isDialogOpen = true;

	return false;
}

void Encounter::updatePalette1() {
	debugC(kDebugLevelEncounter, "[Encounter::updatePalette1] Not implemented!");
}

void Encounter::updatePalette2() {
	debugC(kDebugLevelEncounter, "[Encounter::updatePalette2] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Scripts
//////////////////////////////////////////////////////////////////////////
void Encounter::initScript(ResourceId resourceId) {
	_scriptData.reset(resourceId);
	_isClosing = false;
}

Encounter::ScriptEntry Encounter::getScriptEntry(ResourceId resourceId, uint32 offset) {
	ResourceEntry *entry = getResource()->get(resourceId);
	return ScriptEntry(entry->data + offset * 4);
}

void Encounter::runScript() {
	_isScriptRunning = true;
	bool done = false;

	do {
		ScriptEntry entry = getScriptEntry(_scriptData.resourceId, _scriptData.offset);

		debugC(kDebugLevelEncounter, "[Encounter] %s", entry.toString().c_str());

		switch (entry.opcode) {
		default:
			break;

		case kOpcodeEncounterReturn:
			_isScriptRunning = false;
			done = true;
			_value1 = 0;
			break;

		case kOpcodeSetScriptVariable:
			_scriptData.vars[entry.param1] = getVariableInv(entry.param2);
			break;

		case kOpcodeSetCounterFromVars:
			_scriptData.counter = _scriptData.vars[entry.param1] - getVariableInv(entry.param2);
			break;

		case kOpcodeSetOffset:
			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterNegative:
			if (_scriptData.counter >= 0)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterNegativeOrNull:
			if (_scriptData.counter > 0)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterIsNull:
			if (_scriptData.counter)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterIsNotNull:
			if (!_scriptData.counter)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterPositiveOrNull:
			if (_scriptData.counter < 0)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetOffsetIfCounterPositive:
			if (_scriptData.counter <= 0)
				break;

			_scriptData.offset = entry.param2 - 1;
			break;

		case kOpcodeSetCurrentItemOptions:
			if (entry.param1)
				_item->keywords[findKeyword(_item, entry.param2)] &= -((kKeywordOptionsDisabled << 8) + 1);
			else
				_item->keywords[findKeyword(_item, entry.param2)] |= (kKeywordOptionsVisible << 8);
			break;

		case kOpcodeClearCurrentItemOptions:
			if (entry.param1)
				_item->keywords[findKeyword(_item, entry.param2)] |= (kKeywordOptionsDisabled << 8);
			else
				_item->keywords[findKeyword(_item, entry.param2)] &= -((kKeywordOptionsVisible << 8) + 1);
			break;

		case kOpcodeSetItemOptions:
			_items[entry.param1].keywords[findKeyword(&_items[entry.param1], entry.param2)] |= (kKeywordOptionsUnknown << 8);
			_items[entry.param1].keywords[findKeyword(&_items[entry.param1], entry.param2)] |= (kKeywordOptionsVisible << 8);

			// Original saves the item back here
			break;

		case kOpcodeCloseDialog:
			if (!_isClosing)
				_shouldCloseDialog = true;

			done = true;
			break;

		case kOpcodeResetSpeech:
			resetSpeech(_item->speechResourceId, getVariableInv(entry.param2));

			done = true;
			break;

		case kOpcodeSetVariable:
			setVariable(entry.param2, (int16)_scriptData.vars[entry.param1]);
			break;

		case kOpcodeIncrementScriptVariable:
			_scriptData.vars[entry.param1] += entry.param2;
			break;

		case kOpcodeProcessVariable3:
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
				// fallthrough

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

		case kOpcodeAddRemoveInventoryItem:
			if (entry.param1)
				getScene()->getActor()->inventory.remove(getVariableInv(entry.param2), _scriptData.vars[1]);
			else
				getScene()->getActor()->inventory.add(getVariableInv(entry.param2), _scriptData.vars[1]);
			break;

		case kOpcodeSetCounterIfInventoryOmits:
			_scriptData.counter = getScene()->getActor()->inventory.contains(getVariableInv(entry.param2), _scriptData.vars[1]) ? 0 : 1;
			break;

		case kOpcodePrepareMovie:
			if (!getSharedData()->getMatteBarHeight()) {
				getScreen()->loadGrayPalette();
				getSharedData()->setMatteBarHeight(1);
				getSharedData()->movieIndex = (uint16)getVariableInv(entry.param2);
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

		case kOpcodeSetClearGameFlag:
			if (entry.param1)
				_vm->setGameFlag((GameFlag)getVariableInv(entry.param2));
			else
				_vm->clearGameFlag((GameFlag)getVariableInv(entry.param2));
			break;

		case kOpcodeSetCounterFromGameFlag:
			_scriptData.counter = _vm->isGameFlagSet((GameFlag)getVariableInv(entry.param2)) ? 1 : 0;
			break;
		}

		++_scriptData.offset;

	} while (!done);
}

}
