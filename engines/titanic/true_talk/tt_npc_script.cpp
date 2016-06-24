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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

static const char *const ITEMS[] = {
	"chicken", "napkin", "parrot", "moth", "fuse", "eye", "nose", "ear", "mouth",
	"auditorycenter", "visioncenter", "olfactorycenter", "speechcenter", "stick",
	"longstick", "bomb", "lemon", "puree", "television", "hammer", nullptr
};

/*------------------------------------------------------------------------*/

int TTnpcScriptResponse::size() const {
	for (int idx = 0; idx < 4; ++idx) {
		if (_values[idx] == 0)
			return idx;
	}

	return 4;
}

/*------------------------------------------------------------------------*/

TTscriptRange::TTscriptRange(uint id, const Common::Array<uint> &values, 
		bool isRandom, bool isSequential) :
		_id(id), _nextP(nullptr) {
	_mode = SF_NONE;
	if (isRandom)
		_mode = SF_RANDOM;
	if (isSequential)
		_mode = SF_SEQUENTIAL;

	for (uint idx = 0; idx < values.size(); ++idx)
		_values.push_back(values[idx]);
}

/*------------------------------------------------------------------------*/


bool TTsentenceEntry::load(Common::SeekableReadStream *s) {
	if (s->pos() >= s->size())
		return false;

	_field0 = s->readUint32LE();
	_field4 = s->readUint32LE();
	_string8 = readStringFromStream(s);
	_fieldC = s->readUint32LE();
	_string10 = readStringFromStream(s);
	_string14 = readStringFromStream(s);
	_string18 = readStringFromStream(s);
	_string1C = readStringFromStream(s);
	_field20 = s->readUint32LE();
	_string24 = readStringFromStream(s);
	_field28 = s->readUint32LE();
	_field2C = s->readUint32LE();
	_field30 = s->readUint32LE();

	return true;
}

/*------------------------------------------------------------------------*/

void TTsentenceEntries::load(const CString &resName) {
	TTsentenceEntry entry;
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(resName);

	while (entry.load(r))
		push_back(entry);

	delete r;
}

/*------------------------------------------------------------------------*/

TTscriptMapping::TTscriptMapping() : _id(0) {
	Common::fill(&_values[0], &_values[8], 0);
}

/*------------------------------------------------------------------------*/

void TTscriptMappings::load(const char *name, int valuesPerMapping) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);
	_valuesPerMapping = valuesPerMapping;

	while (r->pos() < r->size()) {
		resize(size() + 1);
		TTscriptMapping &m = (*this)[size() - 1];

		m._id = r->readUint32LE();
		for (int idx = 0; idx < valuesPerMapping; ++idx)
			m._values[idx] = r->readUint32LE();
	}

	delete r;
}

/*------------------------------------------------------------------------*/

void TTtagMappings::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		uint src = r->readUint32LE();
		uint dest = r->readUint32LE();

		push_back(TTtagMapping(src, dest));
	}

	delete r;
}

/*------------------------------------------------------------------------*/

TTnpcScriptBase::TTnpcScriptBase(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTscriptBase(0, charClass, v2, charName, v3, v4, v5, v6, v7),
		_charId(charId), _field54(0), _val2(val2) {
}

/*------------------------------------------------------------------------*/

TTnpcScript::TTnpcScript(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScriptBase(charId, charClass, v2, charName, v3, val2, v4, v5, v6, v7),
		_entryCount(0), _field68(0), _field6C(0), _rangeResetCtr(0),
		_field74(0), _field78(0), _field7C(0), _itemStringP(nullptr), _field2CC(false) {
	CTrueTalkManager::_v2 = 0;
	Common::fill(&_dialValues[0], &_dialValues[DIALS_ARRAY_COUNT], 0);
	Common::fill(&_array[0], &_array[136], 0);

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
	Common::fill(&_array[20], &_array[136], 0);
	_field2CC = false;
}

void TTnpcScript::setupDials(int dial1, int dial2, int dial3) {
	_dialValues[0] = dial1;
	_dialValues[1] = dial2;
	_dialValues[2] = dial3;
	_field74 = getRandomNumber(3) - 1;
	_field78 = getRandomNumber(5) + 6;

	if (_dialValues[0] > 70)
		_field78 = -_field78;
}

void TTnpcScript::addResponse(int id) {
	if (id > 200000)
		id = getDialogueId(id);

	proc15(id);
	TTscriptBase::addResponse(id);
}

int TTnpcScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
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

int TTnpcScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	return processEntries(&_entries, _entryCount, roomScript, sentence);
}

int TTnpcScript::proc8() const {
	return 0;
}

int TTnpcScript::proc9() const {
	return 2;
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

int TTnpcScript::proc15(int id) const {
	return 0;
}

bool TTnpcScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const {
	return true;
}

uint TTnpcScript::getRangeValue(uint id) {
	TTscriptRange *range = findRange(id);
	if (!range)
		return 0;

	switch (range->_mode) {
	case SF_RANDOM: {
		uint count = range->_values.size();

		uint index = getRandomNumber(count) - 1;
		if (count > 1 && range->_values[index] == range->_priorIndex) {
			for (int retry = 0; retry < 8 && index != range->_priorIndex; ++retry)
				index = getRandomNumber(count) - 1;
		}

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

int TTnpcScript::proc21(int v1, int v2, int v3) {
	return v2;
}

int TTnpcScript::proc22(int id) const {
	return 0;
}

int TTnpcScript::proc23() const {
	return 0;
}

const TTscriptMapping *TTnpcScript::getMapping(int index) {
	if (index >= 0 && index < (int)_mappings.size())
		return &_mappings[index];
	return nullptr;
}

int TTnpcScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	return 0;
}

void TTnpcScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

void TTnpcScript::save(SimpleFile *file) {
	file->writeNumber(charId());
	saveBody(file);

	file->writeNumber(4);
	file->writeNumber(_rangeResetCtr);
	file->writeNumber(_field74);
	file->writeNumber(_field78);
	file->writeNumber(_field7C);
	
	file->writeNumber(10);
	for (int idx = 0; idx < 10; ++idx)
		file->writeNumber(_array[idx]);
}

void TTnpcScript::load(SimpleFile *file) {
	loadBody(file);

	int count = file->readNumber();
	_rangeResetCtr = file->readNumber();
	_field74 = file->readNumber();
	_field78 = file->readNumber();
	_field7C = file->readNumber();

	for (int idx = count; idx > 4; --idx)
		file->readNumber();

	count = file->readNumber();
	for (int idx = 0; idx < count; ++idx) {
		int v = file->readNumber();
		if (idx < 10)
			_array[idx] = v;
	}
}

void TTnpcScript::saveBody(SimpleFile *file) {
	int v = proc31();
	file->writeNumber(v);

	if (v > 0) {
		for (uint idx = 0; idx < _ranges.size(); ++idx) {
			const TTscriptRange &item = _ranges[idx];
			if (item._mode == SF_RANDOM && item._priorIndex) {
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
		int v = file->readNumber();

		for (uint idx = 0; idx < _ranges.size(); ++idx) {
			TTscriptRange &item = _ranges[idx];
			if (!item._id) {
				item._id = v;
				break;
			}
		}
	}
}

int TTnpcScript::proc31() const {
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

int TTnpcScript::proc36(int id) const {
	return 0;
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

int TTnpcScript::getValue(int testNum) {
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
			switch (g_vm->_trueTalkManager->getState14()) {
			case 1:
				CTrueTalkManager::_v6 = 3;
				break;
			case 2:
				CTrueTalkManager::_v6 = 0;
				break;
			case 3:
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
			if (proc36(val))
				return 4;
		}
	}

	uint oldTagId = tagId;
	tagId = getRangeValue(tagId);
	if (tagId != oldTagId)
		tagId = getRangeValue(tagId);

	oldTagId = proc23();
	uint newId = proc21(origId, tagId, oldTagId);
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
	uint newVal = tableP->_values[oldTagId];

	idx = 0;
	int *arrP = &_array[26];
	while (idx < 4 && arrP[idx])
		++idx;
	if (idx == 4)
		return newVal;

	_array[26] = origId;
	idx = 0;
	arrP = &_array[30];
	while (idx < 4 && arrP[idx])
		++idx;
	if (idx == 4)
		return newVal;

	arrP[idx] = newVal;
	return newVal;
}

int TTnpcScript::translateByArray(int id) {
	for (uint idx = 1, arrIndex = 35; idx < 15; ++idx, arrIndex += 8) {
		if (_array[idx - 1] == id && _array[idx] == 0)
			return idx;
	}

	return -1;
}

CPetControl *TTnpcScript::getPetControl(CGameManager *gameManager) {
	if (gameManager && gameManager->_project)
		return gameManager->_project->getPetControl();
	return nullptr;
}

int TTnpcScript::processEntries(const TTsentenceEntries *entries, uint entryCount, TTroomScript *roomScript, TTsentence *sentence) {
	if (!entries)
		return SS_1;
	if (!entryCount)
		// No count specified, so use entire list
		entryCount = entries->size();
	int entryId = _field2C;

	for (uint loopCtr = 0; loopCtr < 2; ++loopCtr) {
		for (uint entryCtr = 0; entryCtr < entryCount; ++entryCtr) {
			const TTsentenceEntry &entry = (*entries)[entryCtr];
			if (entry._field4 != entryId && (loopCtr == 0 || entry._field4))
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
						int result = proc25(entry._field2C & 0xFFFFFF, entry._field0,
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

					id = proc22(dialogueId);
					if (id)
						addResponse(getDialogueId(id));
					applyResponse();

					if (entry._field30)
						proc26(entry._field30, &entry, roomScript, sentence);

					return 2;
				}
			}
		}
	}

	return 1;
}

bool TTnpcScript::defaultProcess(TTroomScript *roomScript, TTsentence *sentence) {
	uint remainder;
	TTtreeResult results[32];
	const TTstring &line = sentence->_normalizedLine;

	uint tagId = g_vm->_trueTalkManager->_quotes.find(line.c_str());
	int val = g_vm->_trueTalkManager->_quotesTree.search(line.c_str(), TREE_1, results, tagId, &remainder);

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

void TTnpcScript::checkItems(TTroomScript *roomScript, TTsentence *sentence) {
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

} // End of namespace Titanic
