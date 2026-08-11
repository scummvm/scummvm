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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "gob/dbase.h"

#include "common/tokenizer.h"

namespace Gob {

dbaseMultipeIndex::dbaseMultipeIndex() {
	clear();
}

void dbaseMultipeIndex::clear() {
	memset(&_creationDate, 0, sizeof(_creationDate));
	memset(&_lastUpdate, 0, sizeof(_lastUpdate));

	_version = 0;
}

// Supported key definition syntax:
// key_definition ::= field_definition { "+" field_definition }
// field_definition ::= field_name | STR(field_name, length, 0)
Common::Array<dbaseMultipeIndex::FieldReference> dbaseMultipeIndex::parseKeyDefinition(const Common::String &keyDefinition) {
	Common::Array<FieldReference> fieldReferences;
	Common::StringTokenizer tokenizer(keyDefinition, "+");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token.hasPrefix("STR(")) {
			// STR(field_name, length, 0) expression
			size_t firstCommaPos = token.find(',');
			size_t secondCommaPos = token.find(',', firstCommaPos + 1);
			Common::String fieldName = token.substr(4, firstCommaPos - 4);
			size_t length = 0;
			if (secondCommaPos != token.npos)
				length = atoi(token.substr(firstCommaPos + 1, secondCommaPos - firstCommaPos - 1).c_str());

			fieldReferences.push_back({fieldName, length});
		} else {
			// Field name only
			fieldReferences.push_back({token, 0});
		}
	}

	return fieldReferences;
}

const Common::Array<dbaseMultipeIndex::FieldReference>* dbaseMultipeIndex::getTagKeyDefinition(Common::String tagName) const {
	if (!_tagKeyDefinitions.contains(tagName))
		return nullptr;
	else
		return &_tagKeyDefinitions[tagName];
}

bool dbaseMultipeIndex::load(Common::SeekableReadStream &stream) {
	_version = stream.readByte();
	_creationDate.tm_year = stream.readByte();
	_creationDate.tm_mon  = stream.readByte() - 1;
	_creationDate.tm_mday = stream.readByte();

	uint32 pos = stream.pos();
	_dataFilename = stream.readString('\0', 16);
	stream.seek(pos + 16);
	stream.skip(2); // Block size
	stream.skip(2); // Page size
	stream.skip(1); // Production flag
	stream.skip(1); // Max number of tags
	stream.skip(1); // Tag length
	stream.skip(1); // Reserved
	_nbrOfTagsInUse = stream.readUint16LE(); // Number of tags in use
	stream.skip(2); // Reserved
	stream.skip(4); // Number of pages in tag file
	stream.skip(4); // Pointer to first free page
	stream.skip(4); // Number of available blocks

	_lastUpdate.tm_year = stream.readByte();
	_lastUpdate.tm_mon  = stream.readByte() - 1;
	_lastUpdate.tm_mday = stream.readByte();

	stream.seek(544); // Go past the header (size 32 + 512)

	for (int i = 0; i < _nbrOfTagsInUse; ++i) {
		uint32 tagHeaderPage = stream.readUint32LE();
		Common::String tagName = stream.readString('\0', 11);
		stream.skip(1); // key format
		stream.skip(1); // forward tag thread inf
		stream.skip(1); // forward tag thread sup
		stream.skip(1); // backward tag thread
		stream.skip(1); // reserved
		stream.skip(1); // key type
		stream.skip(11); // reserved

		int64 tagEntryEnd = stream.pos();

		stream.seek(tagHeaderPage * INDEX_PAGE_SIZE);
		stream.skip(4); // tag root page
		stream.skip(4); // file size in pages
		stream.skip(1); // key format
		stream.skip(1); // key type
		stream.skip(2); // reserved
		stream.skip(2); // index key length
		stream.skip(2); // max nbr of keys per page
		stream.skip(2); // secondary key type
		stream.skip(2); // index key item length
		stream.skip(3); // reserved
		stream.skip(1); // unique flag

		Common::String keyExpression = stream.readString('\0', 488);
		_tagKeyDefinitions[tagName] = parseKeyDefinition(keyExpression);

		// For now, we do not need to read the B-tree structure itself.
		// The key definition is enough for our use cases.

		stream.seek(tagEntryEnd);
	}

	return true;
}

dBase::dBase() : _recordData(nullptr) {
	clear();
}

dBase::~dBase() {
	clear();
}

bool dBase::load(Common::SeekableReadStream &stream) {
	clear();

	uint32 startPos = stream.pos();

	_version = stream.readByte();
	if (_version == 0x03 || _version == 0x83) {
		_versionMajor = 3;
	} else if (_version == 0x04 || _version == 0x7B || _version == 0x8B) {
		_versionMajor = 4;
	} else {
		warning("dBase::load() called on unsupported dBase version %d", _version);
		return false;
	}

	_hasMemo = (_version & 0x80) != 0;

	_lastUpdate.tm_year = stream.readByte();
	_lastUpdate.tm_mon  = stream.readByte() - 1;
	_lastUpdate.tm_mday = stream.readByte();
	_lastUpdate.tm_hour = 0;
	_lastUpdate.tm_min  = 0;
	_lastUpdate.tm_sec  = 0;

	uint32 recordCount = stream.readUint32LE();
	uint32 headerSize  = stream.readUint16LE();
	uint32 recordSize  = stream.readUint16LE();

	stream.skip(20); // Reserved

	// Read all field descriptions, 0x0D is the end marker
	uint32 fieldsLength = 0;
	while (!stream.eos() && !stream.err() && (stream.readByte() != 0x0D)) {
		Field field;

		stream.seek(-1, SEEK_CUR);

		field.name = readString(stream, 11);
		field.type = (Type) stream.readByte();

		stream.skip(4); // Field data address

		field.size     = stream.readByte();
		field.decimals = stream.readByte();

		fieldsLength += field.size;

		stream.skip(14); // Reserved and/or useless for us

		_fields.push_back(field);
	}

	if (stream.eos() || stream.err())
		return false;

	if ((stream.pos() - startPos) != headerSize)
		// Corrupted file / unknown format
		return false;

	if (recordSize != (fieldsLength + 1))
		// Corrupted file / unknown format
		return false;

	_recordData = new byte[recordSize * recordCount];
	if (stream.read(_recordData, recordSize * recordCount) != (recordSize * recordCount))
		return false;

	if (stream.readByte() != 0x1A)
		// Missing end marker
		return false;

	uint32 fieldCount = _fields.size();

	// Create the records array
	_records.resize(recordCount);
	for (uint32 i = 0; i < recordCount; i++) {
		Record &record = _records[i];
		const byte *data = _recordData + i * recordSize;

		char status = *data++;
		if ((status != ' ') && (status != '*'))
			// Corrupted file / unknown format
			return false;

		record.deleted = status == '*';

		record.fields.resize(fieldCount);
		for (uint32 j = 0; j < fieldCount; j++) {
			record.fields[j] = data;
			data += _fields[j].size;
		}
	}

	return true;
}

bool dBase::loadMemo(Common::SeekableReadStream &stream) {
	uint32 nextBlock = stream.readUint32LE();
	if (nextBlock < 2)
		return true; // No data blocks

	uint32 nbrOfDataBlocks = nextBlock - 2;

	_memoData.clear();
	_memoData.resize(nbrOfDataBlocks * MEMO_BLOCK_SIZE);

	for (uint32 i = 1; i < nextBlock; i++) {
		stream.seek(i * MEMO_BLOCK_SIZE, SEEK_SET);
		uint32 type = stream.readUint32LE();
		if (type != 0x8FFFF) {
			warning("dBase::loadMemo() found unexpected memo record type %08X", type);
		}

		int32 memoSize = stream.readSint32LE();
		if (memoSize < 8) // Header size (8) is included in memoSize
			continue; // Empty memo

		_memoData[i - 1] = readString(stream, memoSize - 8);
	}

	return true;
}


bool dBase::loadMultipleIndex(Common::SeekableReadStream &stream) {
	if (_multipleIndex.load(stream)) {
		_hasMultipleIndex = true;
		return true;
	} else
		return false;
}

void dBase::clear() {
	memset(&_lastUpdate, 0, sizeof(_lastUpdate));

	_version = 0;
	_hasMemo = false;

	_fields.clear();
	_records.clear();

	delete[] _recordData;
	_recordData = nullptr;
}

byte dBase::getVersion() const {
	return _version;
}

bool dBase::hasMemo() const {
	return _hasMemo;
}

TimeDate dBase::getLastUpdate() const {
	return _lastUpdate;
}

const Common::Array<dBase::Field> &dBase::getFields() const {
	return _fields;
}

const Common::Array<dBase::Record> &dBase::getRecords() const {
	return _records;
}

Common::String dBase::getString(const Record &record, int field) const {
	Type type = _fields[field].type;

	switch (type) {
	case kTypeString: {
		uint32 fieldLength = stringLength(record.fields[field], _fields[field].size);
		return Common::String((const char *) record.fields[field], fieldLength);
	}

	case kTypeNumber: {
		Common::String str = Common::String((const char *) record.fields[field], _fields[field].size);
		str.trim();
		return str;
	}

	case kTypeMemo: {
		Common::String blockNbrStr = Common::String((const char *) record.fields[field],  _fields[field].size);
		int blockNbr = atoi(blockNbrStr.c_str());
		if ((blockNbr < 1) || ((size_t) blockNbr > _memoData.size())) {
			warning("dBase::getString() called on invalid memo block %d", blockNbr);
			return "";
		}

		return _memoData[blockNbr - 1];
	}

	default:
		// Unsupported type
		warning("dBase::getString() called on unsupported field type %d", type);
	}

	return "";
}

void dBase::setQuery(const Common::String &query) {
	_currentFieldFilter.clear();

	if (!_hasMultipleIndex) {
		warning("dBase::setQuery() called on a database without multiple index");
		return;
	}

	const Common::Array<dbaseMultipeIndex::FieldReference>* keyDefinition = _multipleIndex.getTagKeyDefinition(_currentIndexTag);
	if (!keyDefinition) {
		warning("dBase::setQuery(): key definition not found for tag '%s'", _currentIndexTag.c_str());
		return;
	}

	// Parse the query. Field separator is ';', catch-all is '?'
	Common::StringTokenizer tokenizer(query, ";");
	size_t fieldIndex = 0;
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token != "?") {
			if (fieldIndex >= keyDefinition->size()) {
				warning("dBase::setQuery(): too many fields in query");
				return;
			}

			const dbaseMultipeIndex::FieldReference &fieldReference = (*keyDefinition)[fieldIndex];
			const Common::String &fieldName = fieldReference.getFieldName();
			for (size_t i = 0; i < _fields.size(); ++i) {
				if (_fields[i].name == fieldName) {
					_currentFieldFilter.push_back({i, fieldReference.getMaxLength(), token});
					break;
				}

				if (i == _fields.size() - 1) {
					warning("dBase::setQuery(): field '%s' not found", fieldName.c_str());
					return;
				}
			}
		}

		++fieldIndex;
	}
}

void dBase::setCurrentIndex(const Common::String &tagName) {
	_currentIndexTag = tagName;
	_currentRecordIndex = -1;
	_currentFieldFilter.clear();
}

void dBase::findNextMatchingRecord() {
	++_currentRecordIndex;

	if (_currentFieldFilter.empty()) {
		_currentRecordIndex = _records.size();
		return;
	}

	for (; _currentRecordIndex < (int)_records.size(); ++_currentRecordIndex) {
		const Record &record = _records[_currentRecordIndex];

		bool match = true;
		for (const FieldPattern &pattern : _currentFieldFilter) {
			if (pattern.fieldIndex >= _fields.size()) {
				match = false;
				break;
			}

			Common::String fieldValue = getString(record, pattern.fieldIndex);
			if (pattern.maxLength > 0)
				fieldValue = fieldValue.substr(0, pattern.maxLength);

			if (fieldValue != pattern.pattern) {
				match = false;
				break;
			}
		}

		if (match)
			return;
	}
}

void dBase::findFirstMatchingRecord() {
	_currentRecordIndex = -1;
	findNextMatchingRecord();
}

bool dBase::hasMatchingRecord() {
	return _currentRecordIndex >= 0 && _currentRecordIndex < (int) _records.size();
}

Common::String dBase::getFieldOfMatchingRecord(Common::String fieldName) {
	if (!hasMatchingRecord())
		return "";

	const Record &record = _records[_currentRecordIndex];
	size_t fieldIndex = 0;
	for (const Field &field : _fields) {
		if (field.name == fieldName) {
			return getString(record, fieldIndex);
		}

		++fieldIndex;
	}

	return "";
}

// String fields are padded with spaces. This finds the real length.
inline uint32 dBase::stringLength(const byte *data, uint32 max) {
	while (max-- > 0)
		if ((data[max] != 0x20) && (data[max] != 0x00))
			return max + 1;

	return 0;
}

// Read a constant-length string out of a stream.
inline Common::String dBase::readString(Common::SeekableReadStream &stream, int n) {
	Common::String str;

	char c;
	while (n-- > 0) {
		if ((c = stream.readByte()) == '\0')
			break;

		str += c;
	}

	if (n > 0)
		stream.skip(n);

	return str;
}

} // End of namespace Gob
