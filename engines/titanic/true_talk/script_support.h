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

#ifndef TITANIC_SCRIPT_SUPPORT_H
#define TITANIC_SCRIPT_SUPPORT_H

#include "titanic/support/simple_file.h"

namespace Titanic {

#define DIALS_ARRAY_COUNT 10

enum ScriptArrayFlag { SF_NONE = 0, SF_RANDOM = 1, SF_SEQUENTIAL = 2 };

struct RoomDialogueId {
	uint _roomNum;
	uint _dialogueId;
};

struct TTnpcScriptResponse {
	uint _tag;
	uint _values[4];

	TTnpcScriptResponse() : _tag(0) {
		_values[0] = _values[1] = _values[2] = _values[3] = 0;
	}

	/**
	 * Returns the size of the values list plus 1
	 */
	int size() const;
};

struct TTscriptRange {
	uint _id;
	Common::Array<uint> _values;
	TTscriptRange *_nextP;
	uint _priorIndex;
	ScriptArrayFlag _mode;

	TTscriptRange() : _id(0), _nextP(nullptr),
		_priorIndex(0), _mode(SF_NONE) {}
	TTscriptRange(uint id, const Common::Array<uint> &values, bool isRandom,
		bool isSequential);
};

struct TTsentenceEntry {
	int _field0;
	int _category;
	CString _string8;
	int _fieldC;
	CString _string10;
	CString _string14;
	CString _string18;
	CString _string1C;
	int _field20;
	CString _string24;
	int _field28;
	int _field2C;
	int _field30;

	TTsentenceEntry() : _field0(0), _category(0), _fieldC(0),
		_field20(0), _field28(0), _field2C(0), _field30(0) {}

	/**
	* Load an entry from the passed stream, and returns true
	* if an entry was successfully loaded
	*/
	bool load(Common::SeekableReadStream *s);
};

class TTsentenceEntries : public Common::Array<TTsentenceEntry> {
public:
	/**
	* Load a list of entries from the specified resource
	*/
	void load(const CString &resName);
};

struct TTscriptMapping {
	uint _id;
	uint _values[8];

	TTscriptMapping();
};

class TTscriptMappings : public Common::Array<TTscriptMapping> {
public:
	int _valuesPerMapping;

	void load(const char *name, int valuesPerMapping);
};

struct TTtagMapping {
	uint _src, _dest;
	TTtagMapping() : _src(0), _dest(0) {}
	TTtagMapping(uint src, uint dest) : _src(src), _dest(dest) {}
};

class TTtagMappings : public Common::Array<TTtagMapping> {
public:
	void load(const char *name);
};

struct TTwordEntry {
	uint _id;
	CString _text;

	TTwordEntry() : _id(0) {}
};

class TTwordEntries : public Common::Array<TTwordEntry> {
public:
	void load(const char *name);
};

struct TThandleQuoteEntry {
	uint _tag1;
	uint _tag2;
	uint _index;

	TThandleQuoteEntry() : _tag1(0), _tag2(0), _index(0) {}
};

class TThandleQuoteEntries : public Common::Array<TThandleQuoteEntry> {
public:
	uint _rangeStart, _rangeEnd;
	uint _incr;
public:
	TThandleQuoteEntries() : _rangeStart(0), _rangeEnd(0), _incr(0) {}
	void load(const char *name);
};

struct TTmapEntry {
	uint _src;
	uint _dest;

	TTmapEntry() : _src(0), _dest(0) {}
};

class TTmapEntryArray : public Common::Array<TTmapEntry> {
public:
	void load(const char *name);

	/**
	 * Finds a record by Id, and returns it's associated value
	 */
	int find(uint id) const;
};

struct TTupdateState {
	uint _newId;
	uint _newValue;
	uint _dialBits;

	TTupdateState() : _newId(0), _newValue(0), _dialBits(0) {}
};

class TTupdateStateArray : public Common::Array<TTupdateState> {
public:
	void load(const char *name);
};

struct TTcommonPhrase {
	CString _str;
	uint _dialogueId;
	uint _roomNum;
	uint _val1;
};

class TTcommonPhraseArray : public Common::Array<TTcommonPhrase> {
public:
	void load(const char *name);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_NPC_SCRIPT_H */
