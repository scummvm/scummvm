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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/support/files_manager.h"
#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Titanic {

TTsentenceEntries *TTnpcScript::_defaultEntries;

static const char *const ITEMS[] = {
	"chicken", "napkin", "parrot", "moth", "fuse", "eye", "nose", "ear", "mouth",
	"auditorycenter", "visioncenter", "olfactorycenter", "speechcenter", "stick",
	"longstick", "bomb", "lemon", "puree", "television", "hammer", nullptr
};

struct ItemRec {
	const char *const _name;
	uint _id;
};
static const ItemRec ARRAY1[] = {
	{ ITEMS[0], 290138 },
	{ ITEMS[1], 290139 },
	{ ITEMS[2], 290141 },
	{ ITEMS[3], 290142 },
	{ ITEMS[4], 290153 },
	{ ITEMS[5], 290158 },
	{ ITEMS[6], 290159 },
	{ ITEMS[7], 290160 },
	{ ITEMS[8], 290161 },
	{ ITEMS[9], 290162 },
	{ ITEMS[10], 290163 },
	{ ITEMS[11], 290164 },
	{ ITEMS[12], 290165 },
	{ ITEMS[13], 290166 },
	{ ITEMS[14], 290166 },
	{ ITEMS[15], 290178 },
	{ ITEMS[16], 290174 },
	{ ITEMS[17], 290175 },
	{ ITEMS[18], 290176 },
	{ ITEMS[19], 290179 },
	{ nullptr, 0 }
};
static const uint ARRAY2[] = {
	290167, 290178, 290183, 290144, 290148, 290151, 290154, 290156, 290158, 290159, 290160, 290161,
	290162, 290163, 290164, 290165, 290177, 290181, 0
};
static const uint RANDOM1[] = {
	290184, 290185, 290187, 290188, 290190, 290191, 290193, 290195, 290196, 290197, 290198, 290199,
	290202, 290205, 0
};
static const uint RANDOM2[] = {
	290186, 290187, 290188, 290190, 290191, 290193, 290194, 290195, 290196, 290197, 290198, 290199,
	290200, 290201, 290202, 290204, 290205, 0
};
static const uint RANDOM3[] = {
	290188, 290190, 290192, 290194, 290197, 290200, 290201, 290202, 290204, 290205, 0
};
static const uint RANDOM4[] = {
	290206, 290207, 290209, 290210, 290211, 290212, 290216, 290217, 290218, 290219, 290222, 290223, 0
};
static const uint RANDOM5[] = {
	290208, 290209, 290210, 290211, 290212, 290214, 290215, 290216, 290217, 290218, 290219, 290221,
	290222, 290223, 0
};
static const uint RANDOM6[] = {
	290210, 290211, 290213, 290214, 290215, 290220, 290221, 290222, 290223, 0
};
static const uint RANDOM7[] = {
	290225, 290226, 290228, 290229, 290230, 290232, 290231, 290235, 290236, 290237, 290238, 290241, 0
};
static const uint RANDOM8[] = {
	290227, 290228, 290229, 290230, 290231, 290232, 290233, 290234, 290235, 290236, 290237, 290238,
	290240, 290241, 0
};
static const uint RANDOM9[] = {
	290228, 290229, 290230, 290232, 290233, 290234, 290239, 290240, 290241, 0
};

/*------------------------------------------------------------------------*/

TTnpcData::TTnpcData() {
	Common::fill(&_array[0], &_array[136], 0);
}

void TTnpcData::resetFlags() {
	Common::fill(&_array[20], &_array[136], 0);
}

void TTnpcData::copyData() {
	if (_array[20]) {
		Common::copy_backward(&_array[16], &_array[128], &_array[136]);
		Common::fill(&_array[16], &_array[24], 0);
	}
}

/*------------------------------------------------------------------------*/

TTnpcScriptBase::TTnpcScriptBase(int charId_, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTscriptBase(0, charClass, v2, charName, v3, v4, v5, v6, v7),
		_charId(charId_), _field54(0), _val2(val2) {
}

/*------------------------------------------------------------------------*/

void TTnpcScript::init() {
	_defaultEntries = new TTsentenceEntries();
	_defaultEntries->load("Sentences/Default");
}

void TTnpcScript::deinit() {
	delete _defaultEntries;
	_defaultEntries = nullptr;
}

TTnpcScript::TTnpcScript(int charId_, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScriptBase(charId_, charClass, v2, charName, v3, val2, v4, v5, v6, v7),
		_entryCount(0), _field68(0), _field6C(0), _rangeResetCtr(0),
		_currentDialNum(0), _dialDelta(0), _field7C(0), _itemStringP(nullptr), _field2CC(false) {
	CTrueTalkManager::_v2 = 0;
	Common::fill(&_dialValues[0], &_dialValues[DIALS_ARRAY_COUNT], 0);

	if (!CTrueTalkManager::_v10) {
		Common::fill(&CTrueTalkManager::_v11[0], &CTrueTalkManager::_v11[41], 0);
		CTrueTalkManager::_v10 = true;
	}

	resetFlags();
}

void TTnpcScript::loadResponses(const char *name, int valuesPerResponse) {
	_valuesPerResponse = valuesPerResponse;
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTnpcScriptResponse sr;
		sr._tag = r->readUint32LE();
		for (int idx = 0; idx < valuesPerResponse; ++idx)
			sr._values[idx] = r->readUint32LE();

		_responses.push_back(sr);
	}

	delete r;
}

void TTnpcScript::loadRanges(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		Common::Array<uint> values;
		uint id = r->readUint32LE();
		bool isRandom = r->readByte();
		bool isSequential = r->readByte();

		uint v;
		do {
			v = r->readUint32LE();
			values.push_back(v);
		} while (v);

		addRange(id, values, isRandom, isSequential);
	}

	delete r;
}

void TTnpcScript::resetFlags() {
	_data.resetFlags();
	_field2CC = false;
}

void TTnpcScript::setupDials(int dial1, int dial2, int dial3) {
	_dialValues[0] = dial1;
	_dialValues[1] = dial2;
	_dialValues[2] = dial3;
	_currentDialNum = getRandomNumber(3) - 1;
	_dialDelta = getRandomNumber(5) + 6;

	if (_dialValues[0] > 70)
		_dialDelta = -_dialDelta;
}

void TTnpcScript::addResponse(int id) {
	if (id > 200000)
		id = getDialogueId(id);

	handleWord(id);
	TTscriptBase::addResponse(id);
}

int TTnpcScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	for (uint idx = 0; idx < _responses.size(); ++idx) {
		const TTnpcScriptResponse &response = _responses[idx];

		if (response._tag == tag) {
			if (_valuesPerResponse == 1) {
				selectResponse(response._values[0]);
			} else {
				int valIndex = getRandomNumber(response.size()) - 1;
				uint diagId = getDialogueId(response._values[valIndex]);
				addResponse(diagId);
			}

			applyResponse();
			return 2;
		}
	}

	return 1;
}

int TTnpcScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	return processEntries(&_entries, _entryCount, roomScript, sentence);
}

int TTnpcScript::proc8() const {
	return 0;
}

int TTnpcScript::proc11() const {
	return 2;
}

int TTnpcScript::proc12() const {
	return 1;
}

void TTnpcScript::selectResponse(int id) {
	if (id >= 200000 && id <= 290264)
		id = getDialogueId(id);

	addResponse(id);
}

bool TTnpcScript::handleWord(uint id) const {
	if (_words.empty())
		return false;

	for (uint idx = 0; idx < _words.size(); ++idx) {
		const TTwordEntry &we = _words[idx];
		if (we._id == id) {
			TTstring str(we._text);
			g_vm->_scriptHandler->handleWord(&str);
			return true;
		}
	}

	g_vm->_scriptHandler->handleWord(nullptr);
	return true;
}

int TTnpcScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder) {
	if (_quotes.empty())
		return 1;

	for (uint idx = 3; idx < _quotes.size(); ++idx) {
		const TThandleQuoteEntry *qe = &_quotes[idx];

		if (qe->_tag1 == tag1 && 
				(qe->_tag2 == tag2 || qe->_tag2 < MKTAG('A', 'A', 'A', 'A'))) {
			uint threshold = qe->_tag2;
			if (threshold > 0 && threshold < 100) {
				if (!tag2)
					threshold >>= 1;
				if (getRandomNumber(100) < threshold)
					return 1;
			}

			uint dialogueId = qe->_index;
			if (dialogueId >= _quotes._rangeStart && dialogueId <= _quotes._rangeEnd) {
				dialogueId -= _quotes._rangeStart;
				if (dialogueId >= _quotes.size())
					error("Invalid dialogue index in bot script");
				TThandleQuoteEntry &quote = _quotes[dialogueId];

				int rangeLimit = quote._index;
				if (isQuoteDialled()) {
					// Barbot and Doorbot response is affected by dial region
					int dialRegion = getDialRegion(0);
					if (dialRegion != 1) {
						rangeLimit = MAX((int)quote._tag1 - 20, 20);
					}
				}

				dialogueId = ((remainder + _quotes._incr) % 100) >= (uint)rangeLimit
					? quote._tag2 : quote._tag1;
			}

			addResponse(getDialogueId(dialogueId));
			applyResponse();
			return 2;
		}
	}

	return 1;
}

uint TTnpcScript::getRangeValue(uint id) {
	TTscriptRange *range = findRange(id);
	if (!range)
		return id;

	switch (range->_mode) {
	case SF_RANDOM: {
		uint count = range->_values.size();

		int index = (int)getRandomNumber(count) - 1;
		if (count > 1 && range->_values[index] == range->_priorIndex) {
			for (int retry = 0; retry < 8 && index != (int)range->_priorIndex; ++retry)
				index = (int)getRandomNumber(count) - 1;
		}

		assert(index >= 0);
		range->_priorIndex = index;
		return range->_values[index];
	}

	case SF_SEQUENTIAL: {
		// Get the next value from the array sequentially
		int val = range->_values[range->_priorIndex];
		if (!val) {
			// Reached end of array, so reset back to start
			range->_priorIndex = 1;
			val = range->_values[1];
		}

		++range->_priorIndex;
		return val;
	}

	default:
		if (range->_values[range->_priorIndex])
			return range->_values[range->_priorIndex++];

		range->_priorIndex = 1;
		++_rangeResetCtr;
		return range->_values[0];
	}
}

void TTnpcScript::resetRange(int id) {
	TTscriptRange *range = findRange(id);
	if (range && range->_mode != SF_RANDOM)
		range->_priorIndex = 0;
}

int TTnpcScript::updateState(uint oldId, uint newId, int index) {
	return newId;
}

int TTnpcScript::preResponse(uint id) {
	return 0;
}

const TTscriptMapping *TTnpcScript::getMapping(int index) {
	if (index >= 0 && index < (int)_mappings.size())
		return &_mappings[index];
	return nullptr;
}

int TTnpcScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	if (g_language != Common::DE_DEU || !roomScript)
		return 0;

	switch (val1) {
	case 516:
		return getValue(1) == 1 ? 0 : 1;
	case 517:
		return getValue(1) == 2 ? 0 : 1;
	case 518:
		return getValue(1) == 3 ? 0 : 1;
	case 519:
		return getValue(1) != 1 ? 0 : 1;
	case 520:
		return getValue(1) != 2 ? 0 : 1;
	case 521:
		return getValue(1) != 3 ? 0 : 1;

	case 522:
		return roomScript->_scriptId == 101 ? 0 : 1;
	case 523:
		return roomScript->_scriptId == 106 ? 0 : 1;
	case 524:
		return roomScript->_scriptId == 107 ? 0 : 1;
	case 525:
		return roomScript->_scriptId == 108 ? 0 : 1;
	case 526:
		return roomScript->_scriptId == 109 ? 0 : 1;
	case 527:
		return roomScript->_scriptId == 110 ? 0 : 1;
	case 528:
		return roomScript->_scriptId == 111 ? 0 : 1;
	case 529:
		return roomScript->_scriptId == 112 ? 0 : 1;
	case 530:
		return roomScript->_scriptId == 113 ? 0 : 1;
	case 531:
		return roomScript->_scriptId == 114 ? 0 : 1;
	case 532:
		return roomScript->_scriptId == 115 ? 0 : 1;
	case 533:
		return roomScript->_scriptId == 116 ? 0 : 1;
	case 534:
		return roomScript->_scriptId == 117 ? 0 : 1;
	case 535:
		return roomScript->_scriptId == 118 ? 0 : 1;
	case 536:
		return roomScript->_scriptId == 120 ? 0 : 1;
	case 537:
		return roomScript->_scriptId == 121 ? 0 : 1;
	case 538:
		return roomScript->_scriptId == 122 ? 0 : 1;
	case 539:
		return roomScript->_scriptId == 123 ? 0 : 1;
	case 540:
		return roomScript->_scriptId == 124 ? 0 : 1;
	case 541:
		return roomScript->_scriptId == 125 ? 0 : 1;
	case 542:
		return roomScript->_scriptId == 126 ? 0 : 1;
	case 543:
		return roomScript->_scriptId == 127 ? 0 : 1;
	case 544:
		return roomScript->_scriptId == 128 ? 0 : 1;
	case 545:
		return roomScript->_scriptId == 129 ? 0 : 1;
	case 546:
		return roomScript->_scriptId == 130 ? 0 : 1;
	case 547:
		return roomScript->_scriptId == 131 ? 0 : 1;
	case 548:
		return roomScript->_scriptId == 132 ? 0 : 1;

	case 549:
		return roomScript->_scriptId != 101 ? 0 : 1;
	case 550:
		return roomScript->_scriptId != 106 ? 0 : 1;
	case 551:
		return roomScript->_scriptId != 107 ? 0 : 1;
	case 552:
		return roomScript->_scriptId != 108 ? 0 : 1;
	case 553:
		return roomScript->_scriptId != 109 ? 0 : 1;
	case 554:
		return roomScript->_scriptId != 110 ? 0 : 1;
	case 555:
		return roomScript->_scriptId != 111 ? 0 : 1;
	case 556:
		return roomScript->_scriptId != 112 ? 0 : 1;
	case 557:
		return roomScript->_scriptId != 113 ? 0 : 1;
	case 558:
		return roomScript->_scriptId != 114 ? 0 : 1;
	case 559:
		return roomScript->_scriptId != 115 ? 0 : 1;
	case 560:
		return roomScript->_scriptId != 116 ? 0 : 1;
	case 561:
		return roomScript->_scriptId != 117 ? 0 : 1;
	case 562:
		return roomScript->_scriptId != 118 ? 0 : 1;
	case 563:
		return roomScript->_scriptId != 120 ? 0 : 1;
	case 564:
		return roomScript->_scriptId != 121 ? 0 : 1;
	case 565:
		return roomScript->_scriptId != 122 ? 0 : 1;
	case 566:
		return roomScript->_scriptId != 123 ? 0 : 1;
	case 567:
		return roomScript->_scriptId != 124 ? 0 : 1;
	case 568:
		return roomScript->_scriptId != 125 ? 0 : 1;
	case 569:
		return roomScript->_scriptId != 126 ? 0 : 1;
	case 570:
		return roomScript->_scriptId != 127 ? 0 : 1;
	case 571:
		return roomScript->_scriptId != 128 ? 0 : 1;
	case 572:
		return roomScript->_scriptId != 129 ? 0 : 1;
	case 573:
		return roomScript->_scriptId != 130 ? 0 : 1;
	case 574:
		return roomScript->_scriptId != 131 ? 0 : 1;
	case 575:
		return roomScript->_scriptId != 132 ? 0 : 1;
	default:
		return 0;
	}
}

void TTnpcScript::save(SimpleFile *file) {
	file->writeNumber(charId());
	saveBody(file);

	file->writeNumber(4);
	file->writeNumber(_rangeResetCtr);
	file->writeNumber(_currentDialNum);
	file->writeNumber(_dialDelta);
	file->writeNumber(_field7C);

	// Write out the dial values
	file->writeNumber(10);
	for (int idx = 0; idx < 10; ++idx)
		file->writeNumber(_dialValues[idx]);
}

void TTnpcScript::load(SimpleFile *file) {
	loadBody(file);

	int count = file->readNumber();
	assert(count == 4);
	_rangeResetCtr = file->readNumber();
	_currentDialNum = file->readNumber();
	_dialDelta = file->readNumber();
	_field7C = file->readNumber();

	for (int idx = count; idx > 4; --idx)
		file->readNumber();

	// Read in the dial values
	count = file->readNumber();
	for (int idx = 0; idx < count; ++idx) {
		int v = file->readNumber();
		if (idx < 10)
			_dialValues[idx] = v;
	}
}

void TTnpcScript::saveBody(SimpleFile *file) {
	int count = getRangesCount();
	file->writeNumber(count);

	if (count > 0) {
		for (uint idx = 0; idx < _ranges.size(); ++idx) {
			const TTscriptRange &item = _ranges[idx];
			if (item._mode != SF_RANDOM && item._priorIndex) {
				file->writeNumber(item._id);
				file->writeNumber(item._priorIndex);
			}
		}
	}
}

void TTnpcScript::loadBody(SimpleFile *file) {
	int count = file->readNumber();
	preLoad();

	for (int index = 0; index < count; index += 2) {
		int id = file->readNumber();
		int val = file->readNumber();

		for (uint idx = 0; idx < _ranges.size(); ++idx) {
			TTscriptRange &item = _ranges[idx];
			if (item._id == (uint)id) {
				item._priorIndex = val;
				break;
			}
		}
	}
}

int TTnpcScript::getRangesCount() const {
	int count = 0;
	for (uint idx = 0; idx < _ranges.size(); ++idx) {
		const TTscriptRange &item = _ranges[idx];
		if (item._mode != SF_RANDOM && item._priorIndex)
			++count;
	}

	return count * 2;
}

void TTnpcScript::setDialRegion(int dialNum, int region) {
	if (dialNum < DIALS_ARRAY_COUNT)
		_dialValues[dialNum] = region * 100;

	if (g_vm->_trueTalkManager) {
		CPetControl *petControl = getPetControl(g_vm->_trueTalkManager->getGameManager());
		if (petControl)
			petControl->playSound(1);
	}
}

void TTnpcScript::setDial(int dialNum, int value) {
	if (dialNum < DIALS_ARRAY_COUNT) {
		int oldRegion = getDialRegion(dialNum);

		int newRegion = 1;
		if (value < 50)
			newRegion = 0;
		else if (value > 150)
			newRegion = 2;

		if (oldRegion == newRegion)
			setDialRegion(dialNum, newRegion);

		_dialValues[dialNum] = value;
	}

	if (g_vm->_trueTalkManager) {
		CPetControl *petControl = getPetControl(g_vm->_trueTalkManager->getGameManager());
		if (petControl)
			petControl->convResetDials();
	}
}

int TTnpcScript::getDialRegion(int dialNum) const {
	if (dialNum < DIALS_ARRAY_COUNT) {
		int value = _dialValues[dialNum];
		if (value < 50)
			return 0;
		else if (value > 150)
			return 2;
		else
			return 1;
	} else {
		return 0;
	}
}

int TTnpcScript::getDialLevel(uint dialNum, bool randomizeFlag) {
	int result = _dialValues[dialNum];
	if (randomizeFlag) {
		bool lowFlag = result <= 50;
		result = CLIP(result + (int)getRandomNumber(18) - 9, 0, 100);

		if (lowFlag) {
			result = MIN(result, 46);
		} else {
			result = MAX(result, 54);
		}
	}

	return result;
}

bool TTnpcScript::randomResponse(uint index) {
	return false;
}

uint TTnpcScript::translateId(uint id) const {
	for (uint idx = 0; idx < _tagMappings.size(); ++idx) {
		if (_tagMappings[idx]._src == id)
			return _tagMappings[idx]._dest;
	}

	return 0;
}

void TTnpcScript::preLoad() {
	for (uint idx = 0; idx < _ranges.size(); ++idx)
		_ranges[idx]._priorIndex = 0;
}

int TTnpcScript::getRoom54(int roomId) {
	TTroomScript *room = g_vm->_trueTalkManager->getRoomScript(roomId);
	return room ? room->_field54 : 0;
}

int TTnpcScript::getValue(int testNum) const {
	switch (testNum) {
	case 0:
		return CTrueTalkManager::_v2;

	case 1:
		if (g_vm->_trueTalkManager)
			CTrueTalkManager::_v3 = g_vm->_trueTalkManager->getPassengerClass();
		return CTrueTalkManager::_v3;

	case 2:
		return CTrueTalkManager::_v4;

	case 3:
		return CTrueTalkManager::_v5 != 0;

	case 4:
		if (g_vm->_trueTalkManager) {
			switch (g_vm->_trueTalkManager->getCurrentSeason()) {
			case SEASON_AUTUMN:
				CTrueTalkManager::_v6 = 3;
				break;
			case SEASON_WINTER:
				CTrueTalkManager::_v6 = 0;
				break;
			case SEASON_SPRING:
				CTrueTalkManager::_v6 = 1;
				break;
			default:
				CTrueTalkManager::_v6 = 2;
				break;
			}
		}
		return CTrueTalkManager::_v6;

	case 5:
		return CTrueTalkManager::_v7;

	case 6:
		return CTrueTalkManager::_v8 != 0;

	case 7:
		return !!getRoom54(123);

	default:
		return CTrueTalkManager::_v11[testNum];
	}
}

uint TTnpcScript::getRandomNumber(int max) const {
	return 1 + g_vm->getRandomNumber(max - 1);
}

uint TTnpcScript::getDialogueId(uint tagId) {
	if (tagId < 200000)
		return tagId;

	// Perform any script specific translation
	uint origId = tagId;
	if (tagId >= 290000 && tagId <= 290263)
		tagId = translateId(tagId);
	if (!tagId)
		return 0;

	if (!_field2CC) {
		_field2CC = true;
		int val = translateByArray(tagId);
		if (val > 0) {
			if (randomResponse(val))
				return 4;
		}
	}

	tagId = getRangeValue(tagId);
	if (tagId != origId)
		tagId = getRangeValue(tagId);

	uint dialBits = getDialsBitset();
	uint newId = updateState(origId, tagId, dialBits);
	if (!newId)
		return 0;

	int idx = 0;
	const TTscriptMapping *tableP;
	for (;;) {
		tableP = getMapping(idx++);
		if (!tableP)
			return 0;

		if (tableP->_id == newId)
			break;
	}
	uint newVal = tableP->_values[dialBits];

	// First slot dialogue Ids
	idx = 0;
	int *arrP = _data.getSlot(0);
	while (idx < 4 && arrP[idx])
		++idx;

	if (idx == 4)
		return newVal;
	arrP[idx] = origId;

	// Second slot dialogue Ids
	idx = 0;
	arrP = _data.getSlot(1);
	while (idx < 4 && arrP[idx])
		++idx;

	if (idx == 4)
		return newVal;
	arrP[idx] = newVal;

	return newVal;
}

int TTnpcScript::translateByArray(int id) {
	for (uint idx = 1, arrIndex = 35; idx < 15; ++idx, arrIndex += 8) {
		if (_data[idx - 1] == id && _data[idx] == 0)
			return idx;
	}

	return -1;
}

CPetControl *TTnpcScript::getPetControl(CGameManager *gameManager) {
	if (gameManager && gameManager->_project)
		return gameManager->_project->getPetControl();
	return nullptr;
}

int TTnpcScript::processEntries(const TTsentenceEntries *entries, uint entryCount, const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!entries)
		return SS_1;
	if (!entryCount)
		// No count specified, so use entire list
		entryCount = entries->size();
	int categoryNum = sentence->_category;

	for (uint loopCtr = 0; loopCtr < 2; ++loopCtr) {
		for (uint entryCtr = 0; entryCtr < entryCount; ++entryCtr) {
			const TTsentenceEntry &entry = (*entries)[entryCtr];
			if (entry._category != categoryNum && (loopCtr == 0 || entry._category))
				continue;

			bool flag;
			if (entry._fieldC || entry._string10.empty()) {
				flag = sentence->fn1(entry._string8, entry._fieldC,
					entry._string14, entry._string18, entry._string1C,
					entry._field20, entry._field28, 0, nullptr);
			} else {
				flag = sentence->fn3(entry._string8, entry._string10,
					entry._string14, entry._string18, entry._string1C,
					entry._string24, entry._field28, 0, nullptr);
			}

			if (flag) {
				if (entry._field2C) {
					bool flag2 = true;
					if (entry._field2C & 0x1000000)
						flag2 = sentence->isConcept34(1);

					if (entry._field2C & 0x2000000)
						flag2 = sentence->isConcept34(0) || sentence->isConcept34(4);

					if (!flag2) {
						flag = false;
					} else {
						int result = doSentenceEntry(entry._field2C & 0xFFFFFF, &entry._field0,
							roomScript, sentence);
						if (result == 2)
							return 2;
						flag = !result;
					}
				}

				if (flag) {
					int dialogueId = getDialogueId(entry._field0);
					int id;
					if (!dialogueId)
						return 1;
					else if (dialogueId == 4)
						return 2;
					addResponse(dialogueId);

					id = preResponse(dialogueId);
					if (id)
						addResponse(getDialogueId(id));
					applyResponse();

					if (entry._field30)
						postResponse(entry._field30, &entry, roomScript, sentence);

					return 2;
				}
			}
		}
	}

	return 1;
}

bool TTnpcScript::defaultProcess(const TTroomScript *roomScript, const TTsentence *sentence) {
	uint remainder;
	TTtreeResult treeResult[32];
	const TTstring &line = sentence->_normalizedLine;

	uint tagId = g_vm->_trueTalkManager->_quotes.find(line.c_str());
	int val = g_vm->_trueTalkManager->_quotesTree.search(line.c_str(), TREE_1, &treeResult[0], tagId, &remainder);

	if (val > 0) {
		if (!handleQuote(roomScript, sentence, val, tagId, remainder))
			return true;
	}

	if (tagId) {
		if (chooseResponse(roomScript, sentence, tagId) == 2)
			return true;
	}

	return false;
}

void TTnpcScript::addRange(uint id, const Common::Array<uint> &values, bool isRandom, bool isSequential) {
	_ranges.push_back(TTscriptRange(id, values, isRandom, isSequential));
}

TTscriptRange *TTnpcScript::findRange(uint id) {
	for (uint idx = 0; idx < _ranges.size(); ++idx) {
		if (_ranges[idx]._id == id)
			return &_ranges[idx];
	}

	return nullptr;
}

void TTnpcScript::checkItems(const TTroomScript *roomScript, const TTsentence *sentence) {
	_data.copyData();
	_field2CC = 0;
	++CTrueTalkManager::_v2;

	if (sentence) {
		if (!_itemStringP || getRandomNumber(100) > 80) {
			for (const char *const *strP = &ITEMS[0]; *strP; ++strP) {
				if (sentence->localWord(*strP)) {
					_itemStringP = *strP;
					break;
				}
			}
		}

		if (sentence->localWord("bomb"))
			_field7C = 1;
	}
}

bool TTnpcScript::addRandomResponse(bool flag) {
	if (getValue(1) > 3)
		return false;

	const uint *data;
	if (flag) {
		if (getValue(1) == 2)
			data = RANDOM8;
		else if (getValue(1) == 1)
			data = RANDOM7;
		else
			data = RANDOM9;
	} else if (getRandomBit()) {
		if (getValue(1) == 2)
			data = RANDOM2;
		else if (getValue(1) == 1)
			data = RANDOM1;
		else
			data = RANDOM3;
	} else {
		if (getValue(1) == 2)
			data = RANDOM5;
		else if (getValue(1) == 1)
			data = RANDOM4;
		else
			data = RANDOM6;
	}

	// Pick a random entry
	uint count = 0;
	while (data[count])
		++count;
	uint id = data[getRandomNumber(count - 1)];

	if (id == 290188 && getRoom54(101))
		id = 290189;
	else if (id == 290202 && getRoom54(123))
		id = 290203;

	if (!id)
		return false;
	id = getDialogueId(id);
	if (id == 4)
		return true;
	if (!id)
		return false;

	if (flag)
		addResponse(getDialogueId(290224));

	addResponse(id);
	applyResponse();
	return true;
}

void TTnpcScript::updateCurrentDial(bool changeDial) {
	int dialLevel = CLIP(getDialLevel(_currentDialNum) + _dialDelta, 0, 100);
	setDial(_currentDialNum, dialLevel);

	bool edgeFlag = false;
	if (_dialDelta < 0) {
		if (dialLevel < 10 || getRandomNumber(100) > 93)
			edgeFlag = true;
	} else {
		if (dialLevel > 90 || getRandomNumber(100) > 93)
			edgeFlag = true;
	}

	if (edgeFlag) {
		if (changeDial)
			_currentDialNum = getRandomNumber(3);

		_dialDelta = getRandomNumber(12) + 3;
		dialLevel = getDialLevel(_currentDialNum, false);
		if (dialLevel > 50)
			_dialDelta = -_dialDelta;
	}
}

bool TTnpcScript::fn10(bool flag) {
	if (_itemStringP) {
		for (const ItemRec *ir = ARRAY1; ir->_id; ++ir) {
			if (!strcmp(ir->_name, _itemStringP)) {
				_itemStringP = nullptr;
				uint id = getDialogueId(ir->_id);
				if (id == 4) {
					return true;
				} else if (id != 0) {
					addResponse(id);
					applyResponse();
					return true;
				}
				break;
			}
		}

		_itemStringP = nullptr;
	}

	if (flag && getRandomNumber(100) > 60) {
		int val = getRandomNumber(18) - 1;

		if (val == 0 && !getRoom54(101) && !getRoom54(132))
			val = -1;
		else if ((val == 1 && !_field7C) || val == 2)
			val = -1;

		if (val >= 0) {
			val = getDialogueId(ARRAY2[val]);
			if (val == 4) {
				return true;
			} else {
				addResponse(val);
				applyResponse();
				return true;
			}
		}
	}

	return false;
}

bool TTnpcScript::getStateValue() const {
	if (!CTrueTalkManager::_currentNPC)
		return false;

	CGameObject *bomb;
	if (CTrueTalkManager::_currentNPC->find("Bomb", &bomb, FIND_GLOBAL) && bomb) {
		CTrueTalkGetStateValueMsg stateMsg(10, -1000);
		stateMsg.execute(bomb);
		if (stateMsg._stateVal)
			return true;
	}

	return false;
}

bool TTnpcScript::sentence2C(const TTsentence *sentence) {
	return sentence->_category >= 2 && sentence->_category <= 7;
}

void TTnpcScript::getAssignedRoom(int *roomNum, int *floorNum, int *elevatorNum) const {
	if (roomNum)
		*roomNum = 5;
	if (floorNum)
		*floorNum = 40;
	if (elevatorNum)
		*elevatorNum = 3;

	CGameManager *gameManager = g_vm->_trueTalkManager->getGameManager();
	CPetControl *petControl = getPetControl(gameManager);
	if (petControl) {
		if (roomNum)
			*roomNum = petControl->getAssignedRoomNum();
		if (floorNum)
			*floorNum = petControl->getAssignedFloorNum();
		if (elevatorNum)
			*elevatorNum = petControl->getAssignedElevatorNum();
	}

	if (floorNum)
		*floorNum = CLIP(*floorNum, 1, 42);
	if (roomNum)
		*roomNum = CLIP(*roomNum, 1, 18);
	if (elevatorNum)
		*elevatorNum = CLIP(*elevatorNum, 1, 4);
}

void TTnpcScript::setResponseFromArray(int index, int id) {
	if (index >= 0 && index <= 15) {
		deleteResponses();
		if (id)
			addResponse(getDialogueId(id));

		// Add any loaded responses
		int *vals = _data.getSlot(index + 1);
		for (int idx = 0; idx < 4; ++idx) {
			if (vals[idx])
				addResponse(vals[idx]);
		}
		applyResponse();

		// Clear out the values used
		if (index)
			Common::fill(vals, vals + 4, 0);
	}
}

} // End of namespace Titanic
