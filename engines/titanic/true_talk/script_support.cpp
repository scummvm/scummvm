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

#include "titanic/true_talk/script_support.h"
#include "titanic/support/files_manager.h"
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

TTscriptRange::TTscriptRange(uint id, const Common::Array<uint> &values,
		bool isRandom, bool isSequential) :
		_id(id), _nextP(nullptr), _priorIndex(0) {
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
	_category = s->readUint32LE();
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

void TTwordEntries::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTwordEntry we;
		we._id = r->readUint32LE();
		we._text = readStringFromStream(r);

		push_back(we);
	}

	delete r;
}

/*------------------------------------------------------------------------*/

void TThandleQuoteEntries::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	_rangeStart = r->readUint32LE();
	_rangeEnd = r->readUint32LE();
	_incr = r->readUint32LE();

	while (r->pos() < r->size()) {
		TThandleQuoteEntry qe;
		qe._tag1 = r->readUint32LE();
		qe._tag2= r->readUint32LE();
		qe._index = r->readUint32LE();

		push_back(qe);
	}

	delete r;
}

/*------------------------------------------------------------------------*/

void TTmapEntryArray::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTmapEntry us;
		us._src = r->readUint32LE();
		us._dest = r->readUint32LE();

		push_back(us);
	}

	delete r;
}

int TTmapEntryArray::find(uint id) const {
	for (uint idx = 0; idx < size(); ++idx) {
		const TTmapEntry &me = (*this)[idx];
		if (me._src == id)
			return me._dest;
	}

	return 0;
}

/*------------------------------------------------------------------------*/

void TTupdateStateArray::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTupdateState us;
		us._newId = r->readUint32LE();
		us._newValue = r->readUint32LE();
		us._dialBits = r->readUint32LE();

		push_back(us);
	}

	delete r;
}

/*------------------------------------------------------------------------*/

void TTcommonPhraseArray::load(const char *name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);

	while (r->pos() < r->size()) {
		TTcommonPhrase cp;
		cp._str = readStringFromStream(r);
		cp._dialogueId = r->readUint32LE();
		cp._roomNum = r->readUint32LE();
		cp._val1 = r->readUint32LE();

		push_back(cp);
	}

	delete r;
}

} // End of namespace Titanic
