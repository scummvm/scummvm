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

#ifndef DUNE_ARCHIVE_H
#define DUNE_ARCHIVE_H

#define USE_CXX11

#include "common/file.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/str.h"

namespace Dune {

typedef Common::SharedPtr<Common::SeekableReadStream> Resource;

class Archive {
private:
	struct Member {
		byte    name[16];
		int32_t size;
		int32_t offset;
		byte    flag;
	};

	Common::File          _archive;
	int                   _count;
	Common::Array<Member> _members;

public:
	Archive();

	bool openArchive(const Common::String &archivePath);

	Resource openMember(const Common::String &name);
};

} // End of namespace Dune

#endif
