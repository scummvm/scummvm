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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

int TTnpcScriptResponse::size() const {
	for (int idx = 0; idx < 4; ++idx) {
		if (_values[idx] == 0)
			return idx;
	}

	return 4;
}

/*------------------------------------------------------------------------*/

TTscriptRange::TTscriptRange(uint id, const uint *arrayP, bool isRandom,
		bool isSequential) :
		_id(id), _arrayP(arrayP), _nextP(nullptr) {
	_mode = SF_NONE;
	if (isRandom)
		_mode = SF_RANDOM;
	if (isSequential)
		_mode = SF_SEQUENTIAL;
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
		_entriesP(nullptr), _entryCount(0), _field68(0), _field6C(0), _rangeResetCtr(0),
		_field74(0), _field78(0), _field7C(0), _field80(0), _field2CC(false) {
	CTrueTalkManager::_v2 = 0;
	Common::fill(&_dialValues[0], &_dialValues[DIALS_ARRAY_COUNT], 0);
	Common::fill(&_array[0], &_array[136], 0);

	if (!CTrueTalkManager::_v10) {
		Common::fill(&CTrueTalkManager::_v11[0], &CTrueTalkManager::_v11[41], 0);
		CTrueTalkManager::_v10 = true;
	}

	resetFlags();
}

void TTnpcScript::load(const char *name, int valuesPerResponse) {
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

void TTnpcScript::resetFlags() {
	Common::fill(&_array[20], &_array[136], 0);
	_field2CC = false;
}

void TTnpcScript::randomizeFlags() {
	warning("TODO");
}

void TTnpcScript::proc4(int v) {
	warning("TODO");
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

void TTnpcScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	processSentence(_entriesP, _entryCount, roomScript, sentence);
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

bool TTnpcScript::loadQuotes() {
	// Original did a load of a global quotes here the first time
	// this method is called. ScummVM implementation has refactored
	// the loading to the CTrueTalkManager constructor

	if (!proc18()) {
		return false;
	} else {
		return proc17();
	}
}

void TTnpcScript::selectResponse(int id) {
	if (id >= 200000 && id <= 290264)
		id = getDialogueId(id);

	addResponse(id);
}

int TTnpcScript::proc15() const {
	return 0;
}

bool TTnpcScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const {
	return true;
}

bool TTnpcScript::proc17() const {
	return true;
}

bool TTnpcScript::proc18() const {
	return true;
}

uint TTnpcScript::getRangeValue(uint id) {
	TTscriptRange *range = findRange(id);
	if (!range)
		return 0;

	switch (range->_mode) {
	case SF_RANDOM: {
		uint count = 0;
		for (const uint *p = range->_arrayP; *p; ++p)
			++count;

		uint index = getRandomNumber(count) - 1;
		if (count > 1 && range->_arrayP[index] == range->_priorIndex) {
			for (int retry = 0; retry < 8 && index != range->_priorIndex; ++retry)
				index = getRandomNumber(count) - 1;
		}

		range->_priorIndex = index;
		return range->_arrayP[index];
	}

	case SF_SEQUENTIAL: {
		// Get the next value from the array sequentially
		int val = range->_arrayP[range->_priorIndex];
		if (!val) {
			// Reached end of array, so reset back to start
			range->_priorIndex = 1;
			val = range->_arrayP[1];
		}

		++range->_priorIndex;
		return val;
	}

	default:
		if (range->_arrayP[range->_priorIndex])
			return range->_arrayP[range->_priorIndex++];
		
		range->_priorIndex = 1;
		++_rangeResetCtr;
		return range->_arrayP[0];
	}
}

void TTnpcScript::proc20(int v) {
	warning("TODO");
}

int TTnpcScript::proc21(int v1, int v2, int v3) {
	// TODO
	return v1;
}

int TTnpcScript::proc22(int id) const {
	return 0;
}

int TTnpcScript::proc23() const {
	return 0;
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
			petControl->resetDials();
	}
}

int TTnpcScript::getDialRegion(int dialNum) {
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

int TTnpcScript::getDialLevel(uint dialNum, bool flag) {
	warning("TODO");
	return 0;
}

int TTnpcScript::proc36(int id) const {
	return 0;
}

uint TTnpcScript::translateId(uint id) const {
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
	int v21 = proc21(origId, tagId, oldTagId);
	if (!v21)
		return 0;

	int idx = 0;
	const int *tableP;
	for (;;) {
		tableP = getTablePtr(idx++);
		if (!tableP)
			return 0;

		if (*tableP == v21)
			break;
	}
	uint newVal = tableP[oldTagId + 1];

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

int TTnpcScript::processSentence(const TTsentenceEntries *entries, uint entryCount, TTroomScript *roomScript, TTsentence *sentence) {
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

	// TODO
	return false;
}

void TTnpcScript::addRange(uint id, const uint *arrayP, bool isRandom, bool isSequential) {
	_ranges.push_back(TTscriptRange(id, arrayP, isRandom, isSequential));
}

TTscriptRange *TTnpcScript::findRange(uint id) {
	for (uint idx = 0; idx < _ranges.size(); ++idx) {
		if (_ranges[idx]._id == id)
			return &_ranges[idx];
	}

	return nullptr;
}

} // End of namespace Titanic
