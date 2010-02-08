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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_UNARJ_H
#define COMMON_UNARJ_H

#include "common/file.h"
#include "common/hash-str.h"
#include "common/archive.h"

namespace Common {

struct ArjHeader;

typedef HashMap<String, int, IgnoreCase_Hash, IgnoreCase_EqualTo> ArjFilesMap;

class ArjArchive : public Common::Archive {

	Common::Array<ArjHeader *> _headers;
	ArjFilesMap _fileMap;

	Common::String _arjFilename;

public:
	ArjArchive(const String &name);
	virtual ~ArjArchive();

	// Common::Archive implementation
	virtual bool hasFile(const String &name);
	virtual int listMembers(ArchiveMemberList &list);
	virtual ArchiveMemberPtr getMember(const String &name);
	virtual SeekableReadStream *createReadStreamForMember(const String &name) const;
};

// TODO: Get rid of this class, by implementing an ArjArchive subclass of Common::Archive.
// Then ArjFile can be substituted by a SearchSet full of ArjArchives plus SearchMan.
class ArjFile : public SearchSet {
public:
	ArjFile();
	~ArjFile();

	void enableFallback(bool val) { _fallBack = val; }

	void registerArchive(const String &filename);

	SeekableReadStream *open(const Common::String &filename);

private:
	bool _fallBack;

};

} // End of namespace Common

#endif
