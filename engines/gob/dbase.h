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

#ifndef GOB_DBASE_H
#define GOB_DBASE_H

#include "common/system.h"
#include "common/util.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/array.h"

namespace Gob {

/**
 * A class for reading Multiple Index (.mdx) files for dBase.
 * Currently, we only read the key definitions, not the actual index data.
 *
 */
class dbaseMultipeIndex {

public:
	dbaseMultipeIndex();
	~dbaseMultipeIndex() {}

	bool load(Common::SeekableReadStream &stream);
	void clear();

	class FieldReference {
	public:
		FieldReference() : _fieldName(""), _maxLength(0) {}
		FieldReference(const Common::String &fieldName, size_t maxLength) : _fieldName(fieldName), _maxLength(maxLength) {}

		const Common::String &getFieldName() const { return _fieldName; }
		size_t getMaxLength() const { return _maxLength; }

	private:
		Common::String _fieldName;
		size_t _maxLength;
	};

	static Common::Array<FieldReference> parseKeyDefinition(const Common::String &keyDefinition);

	const Common::Array<FieldReference>* getTagKeyDefinition(Common::String tagName) const;

private:
	byte _version;
	TimeDate _creationDate;
	Common::String _dataFilename;
	uint16 _nbrOfTagsInUse;
	TimeDate _lastUpdate;

	static const uint16 INDEX_PAGE_SIZE = 512;
	Common::HashMap<Common::String, Common::Array<FieldReference>> _tagKeyDefinitions;
};

/**
 * A class for reading dBase files.
 *
 * Only dBase III files supported for now, and only field type
 * string is actually useful.
 */
class dBase {
public:
	enum Type {
		kTypeString = 0x43, // 'C'
		kTypeDate   = 0x44, // 'D'
		kTypeBool   = 0x4C, // 'L'
		kTypeMemo   = 0x4D, // 'M'
		kTypeNumber = 0x4E  // 'N'
	};

	/** A field description. */
	struct Field {
		Common::String name; ///< Name of the field.

		Type  type;     ///< Type of the field.
		uint8 size;     ///< Size of raw field data in bytes.
		uint8 decimals; ///< Number of decimals the field holds.
	};

	/** A record. */
	struct Record {
		bool deleted; ///< Has this record been deleted?
		Common::Array<const byte *> fields; ///< Raw field data.
	};

	struct FieldPattern {
		size_t fieldIndex;
		size_t maxLength;
		Common::String pattern;
	};

	dBase();
	~dBase();

	bool load(Common::SeekableReadStream &stream);
	bool loadMemo(Common::SeekableReadStream &stream);
	bool loadMultipleIndex(Common::SeekableReadStream &stream);
	void clear();

	byte getVersion() const;
	bool hasMemo() const;

	/** Return the date the database was last updated. */
	TimeDate getLastUpdate() const;

	const Common::Array<Field>  &getFields()  const;
	const Common::Array<Record> &getRecords() const;

	/** Extract a string out of raw field data. */
	Common::String getString(const Record &record, int field) const;

	void setQuery(const Common::String &query);
	void setCurrentIndex(const Common::String &tagName);
	void findFirstMatchingRecord();
	void findNextMatchingRecord();
	bool hasMatchingRecord();
	Common::String getFieldOfMatchingRecord(Common::String fieldName);

private:
	byte _version;
	byte _versionMajor;
	bool _hasMemo;

	TimeDate _lastUpdate;

	Common::Array<Field>  _fields;
	Common::Array<Record> _records;

	int _currentRecordIndex;
	Common::Array<FieldPattern> _currentFieldFilter;

	byte *_recordData;

	Common::Array<Common::String> _memoData;

	Common::String _currentIndexTag;
	bool _hasMultipleIndex;
	dbaseMultipeIndex _multipleIndex;

	static const uint16 MEMO_BLOCK_SIZE = 512;

	static inline uint32 stringLength(const byte *data, uint32 max);
	static inline Common::String readString(Common::SeekableReadStream &stream, int n);
};

} // End of namespace Gob

#endif // GOB_DBASE_H
