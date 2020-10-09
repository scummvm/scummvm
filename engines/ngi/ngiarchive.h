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

#ifndef NGI_NGIARCHIVE_H
#define NGI_NGIARCHIVE_H

#include "common/ptr.h"
#include "common/str.h"

namespace NGI {

class Archive;

#define NGI_FILENAME_MAX 13

struct NgiHeader {
	int32 pos;
	int32 extVal;
	int32 flags;
	int32 size;
	char  filename[NGI_FILENAME_MAX];
};

typedef Common::HashMap<Common::String, Common::ScopedPtr<NgiHeader>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> NgiHeadersMap;

class NGIArchive : public Common::Archive {
	NgiHeadersMap _headers;
	Common::String _ngiFilename;

public:
	NGIArchive(const Common::String &name);
	~NGIArchive() override;

	// Archive implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

/**
 * This factory method creates an Archive instance corresponding to the content
 * of the NGI compressed file with the given name.
 *
 * May return 0 in case of a failure.
 */
NGIArchive *makeNGIArchive(const Common::String &name);

} // End of namespace NGI

#endif
