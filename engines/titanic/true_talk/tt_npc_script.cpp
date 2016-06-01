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
#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/true_talk_manager.h"
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

TTnpcScriptBase::TTnpcScriptBase(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTscriptBase(0, charClass, v2, charName, v3, v4, v5, v6, v7),
		_charId(charId), _field54(0), _val2(val2) {
}

/*------------------------------------------------------------------------*/

TTnpcScript::TTnpcScript(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScriptBase(charId, charClass, v2, charName, v3, val2, v4, v5, v6, v7),
		_subPtr(nullptr), _field60(0), _field64(0), _field68(0),
		_field6C(0), _field70(0), _field74(0), _field78(0),
		_field7C(0), _field80(0), _field2CC(false) {
	CTrueTalkManager::_v2 = 0;
	Common::fill(&_array[0], &_array[146], 0);

	if (!CTrueTalkManager::_v10) {
		Common::fill(&CTrueTalkManager::_v11[0], &CTrueTalkManager::_v11[41], 0);
		CTrueTalkManager::_v10 = true;
	}

	resetFlags();
}

void TTnpcScript::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTnpcScriptResponse sr;
		sr._tag = r->readUint32LE();
		for (int idx = 0; idx < 4; ++idx)
			sr._values[idx] = r->readUint32LE();
		
		_responses.push_back(sr);
	}

	delete r;
}

void TTnpcScript::resetFlags() {
	Common::fill(&_array[20], &_array[140], 0);
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
			int valIndex = getRandomNumber(response.size()) - 1;
			uint diagId = getDialogueId(response._values[valIndex]);
			addResponse(diagId);
			applyResponse();
			return 2;
		}
	}

	return 1;
}

void TTnpcScript::proc7(int v1, int v2) {
	warning("TODO");
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

bool TTnpcScript::proc13() const {
	warning("TODO");
	return true;
}

void TTnpcScript::proc14(int v) {
	warning("TODO");
}

int TTnpcScript::proc15() const {
	return 0;
}

bool TTnpcScript::proc16() const {
	return true;
}

bool TTnpcScript::proc17() const {
	return true;
}

bool TTnpcScript::proc18() const {
	return true;
}

uint TTnpcScript::proc19(uint v) {
	warning("TODO");
	return 0;
}

void TTnpcScript::proc20(int v) {
	warning("TODO");
}

int TTnpcScript::proc21(int v1, int v2, int v3) {
	// TODO
	return v1;
}

int TTnpcScript::proc22() const {
	return 0;
}

int TTnpcScript::proc23() const {
	return 0;
}

int TTnpcScript::proc25() const {
	return 0;
}

void TTnpcScript::proc26() {
}

void TTnpcScript::save(SimpleFile *file) {
	file->writeNumber(charId());
	saveBody(file);

	file->writeNumber(4);
	file->writeNumber(_field70);
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
	_field70 = file->readNumber();
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

	if (v > 0 && _subPtr) {
		warning("TODO");
	}
}

void TTnpcScript::loadBody(SimpleFile *file) {
	int count = file->readNumber();
	preLoad();

	for (int index = 0; index < count; index += 2) {
		int v = file->readNumber();

		if (_subPtr) {
			error("TODO - %d", v);
		}
	}
}

int TTnpcScript::proc31() {
	warning("TODO");
	return 0;
}

void TTnpcScript::proc32() {
	warning("TODO");
}

void TTnpcScript::proc33(int v1, int v2) {
	warning("TODO");
}

int TTnpcScript::proc34() {
	warning("TODO");
	return 0;
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
	if (_subPtr) {
		error("TODO");
	}
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
	tagId = proc19(tagId);
	if (tagId != oldTagId)
		tagId = proc19(tagId);

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

} // End of namespace Titanic
