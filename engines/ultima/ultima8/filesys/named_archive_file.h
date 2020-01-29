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

#ifndef ULTIMA8_FILESYS_NAMEDARCHIVEFILE_H
#define ULTIMA8_FILESYS_NAMEDARCHIVEFILE_H

#include "ultima/ultima8/filesys/archive_file.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class NamedArchiveFile : public ArchiveFile {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	NamedArchiveFile() : indexCount(0) { }
	virtual ~NamedArchiveFile() { }

	virtual bool exists(uint32 index) {
		Std::string name;
		return (indexToName(index, name));
	}
	virtual bool exists(const Std::string &name) = 0;

	virtual uint8 *getObject(uint32 index, uint32 *size = 0) {
		Std::string name;
		if (!indexToName(index, name)) return 0;
		return getObject(name, size);
	}
	virtual uint8 *getObject(const Std::string &name, uint32 *size = 0) = 0;

	virtual uint32 getSize(uint32 index) {
		Std::string name;
		if (!indexToName(index, name)) return 0;
		return getSize(name);
	}
	virtual uint32 getSize(const Std::string &name) = 0;

	virtual uint32 getCount() = 0;

	virtual uint32 getIndexCount() {
		return indexCount;
	}

	virtual bool isIndexed() const {
		return false;
	}
	virtual bool isNamed() const {
		return true;
	}

protected:
	bool indexToName(uint32 index, Std::string &name) {
		Std::map<uint32, Std::string>::iterator iter;
		iter = indexedNames.find(index);
		if (iter == indexedNames.end()) return false;
		name = iter->_value;
		return true;
	}

	void storeIndexedName(const Std::string &name) {
		uint32 index;
		bool hasIndex = extractIndexFromName(name, index);
		if (hasIndex) {
			indexedNames[index] = name;
			if (index >= indexCount) indexCount = index + 1;
		}
	}

	Std::map<uint32, Std::string> indexedNames;
	uint32 indexCount;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
