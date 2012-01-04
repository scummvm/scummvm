/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_LAB_H
#define GRIM_LAB_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/types.h"

namespace Grim {

class Lab;

class LabEntry : public Common::ArchiveMember {
	Lab *_parent;
	Common::String _name;
	uint32 _offset, _len;
public:
	LabEntry();
	LabEntry(Common::String name, uint32 offset, uint32 len, Lab *parent);
	Common::String getName() const { return _name; }
	Common::SeekableReadStream *createReadStream() const;
	friend class Lab;
};

class Lab : public Common::Archive {
public:
	Lab() : _f(NULL), _memLab(NULL) { }
	~Lab() { close(); }

	bool open(const Common::String &filename);
	bool open(const byte *memLab, const uint32 size);
	void close();

	// Common::Archive implementation
	virtual bool hasFile(const Common::String &name); //TODO: Remove at next scummvm sync
	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list);
	virtual Common::ArchiveMemberPtr getMember(const Common::String &name);
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

private:
	bool loadLab();
	void parseGrimFileTable();
	void parseMonkey4FileTable();

	Common::SeekableReadStream *_f;
	const byte *_memLab;
	Common::String _labFileName;
	typedef Common::SharedPtr<LabEntry> LabEntryPtr;
	typedef Common::HashMap<Common::String, LabEntryPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LabMap;
	LabMap _entries;
};

} // end of namespace Grim

#endif
