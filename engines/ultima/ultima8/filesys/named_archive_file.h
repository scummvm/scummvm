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
 */

#ifndef ULTIMA8_FILESYS_NAMEDARCHIVEFILE_H
#define ULTIMA8_FILESYS_NAMEDARCHIVEFILE_H

#include "ultima/ultima8/filesys/archive_file.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class NamedArchiveFile : public ArchiveFile {
public:
	NamedArchiveFile() : _indexCount(0) { }
	~NamedArchiveFile() override { }

	bool exists(uint32 index) override {
		Std::string name;
		return (indexToName(index, name));
	}
	bool exists(const Std::string &name) override = 0;

	uint8 *getObject(uint32 index, uint32 *size = 0) override {
		Std::string name;
		if (!indexToName(index, name))
			return nullptr;
		return getObject(name, size);
	}
	uint8 *getObject(const Std::string &name, uint32 *size = 0) override = 0;

	uint32 getSize(uint32 index) const override {
		Std::string name;
		if (!indexToName(index, name))
			return 0;
		return getSize(name);
	}
	uint32 getSize(const Std::string &name) const override = 0;

	uint32 getCount() const override = 0;

	uint32 getIndexCount() const override {
		return _indexCount;
	}

	bool isIndexed() const override {
		return false;
	}
	bool isNamed() const override {
		return true;
	}

protected:
	bool indexToName(uint32 index, Std::string &name) const {
		Common::HashMap<uint32, Std::string>::const_iterator iter;
		iter = _indexedNames.find(index);
		if (iter == _indexedNames.end()) return false;
		name = iter->_value;
		return true;
	}

	void storeIndexedName(const Std::string &name) {
		uint32 index;
		bool hasIndex = extractIndexFromName(name, index);
		if (hasIndex) {
			_indexedNames[index] = name;
			if (index >= _indexCount) _indexCount = index + 1;
		}
	}

	Common::HashMap<uint32, Std::string> _indexedNames;
	uint32 _indexCount;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
