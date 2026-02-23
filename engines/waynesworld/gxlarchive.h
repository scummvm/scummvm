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

#ifndef WAYNESWORLD_GXLARCHIVE
#define WAYNESWORLD_GXLARCHIVE

#include "common/archive.h"
#include "gamelogic.h"

namespace Common {
class File;
}

namespace WaynesWorld {

#define GXL_FILENAME_MAX 13

struct GxlHeader {
	int32 offset;
	int32 size;
	char  filename[GXL_FILENAME_MAX];
};

typedef Common::HashMap<Common::Path, Common::ScopedPtr<GxlHeader>, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> GxlHeadersMap;

class GxlArchive : public Common::Archive {
	GxlHeadersMap _headers;
	Common::File *_gxlFile;

	WWSurface *loadSurfaceIntern(const char *filename);

public:
	GxlArchive(const Common::String name);
	~GxlArchive() override;

	// Archive implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	Image::PCXDecoder *loadImage(const char *filename);
	WWSurface *loadRoomSurface(const char *filename);
	WWSurface *loadSurface(const char *filename);
};

} // End of namespace WaynesWorld

#endif
