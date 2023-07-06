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

#ifndef GRIM_LAB_H
#define GRIM_LAB_H

#include "common/archive.h"

namespace Common {
	class File;
}

namespace Grim {

class Lab;

class LabEntry : public Common::ArchiveMember {
	Lab *_parent;
	Common::String _name;
	uint32 _offset, _len;
public:
	LabEntry(const Common::String &name, uint32 offset, uint32 len, Lab *parent);
	Common::String getName() const override { return _name; }
	Common::String getFileName() const override { return _name; }
	Common::Path getPathInArchive() const override { return _name; }
	Common::SeekableReadStream *createReadStream() const override;
	friend class Lab;
};

class Lab : public Common::Archive {
public:
	bool open(const Common::String &filename, bool keepStream = false);
	Lab();
	~Lab();
	// Common::Archive implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	void parseGrimFileTable(Common::File *_f);
	void parseMonkey4FileTable(Common::File *_f);

	Common::String _labFileName;
	typedef Common::SharedPtr<LabEntry> LabEntryPtr;
	typedef Common::HashMap<Common::String, LabEntryPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LabMap;
	LabMap _entries;
	Common::SeekableReadStream *_stream;
};

} // end of namespace Grim

#endif
