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

#ifndef MADS_HAG_H
#define MADS_HAG_H

#include "common/archive.h"

namespace MADS {

enum ResourceType {
	RESTYPE_ROOM, RESTYPE_SC, RESTYPE_TEXT, RESTYPE_QUO, RESTYPE_I,
	RESTYPE_OB, RESTYPE_FONT, RESTYPE_SOUND, RESTYPE_SPEECH, RESTYPE_HAS_EXT, RESTYPE_NO_EXT
};

/**
 * HAG Archives implementation
 */
class HagArchive : public Common::Archive {
private:
	/**
	 * Details of a single entry in a HAG file index
	 */
	struct HagEntry {
		Common::Path _resourceName;
		uint32 _offset;
		uint32 _size;

		HagEntry() : _offset(0), _size(0) {
		}
		HagEntry(const Common::Path &resourceName, uint32 offset, uint32 size)
			: _resourceName(resourceName), _offset(offset), _size(size) {
		}
	};

	class HagIndex {
	public:
		Common::List<HagEntry> _entries;
		Common::Path _filename;
	};

	Common::Array<HagIndex> _index;

	/**
	 * Load the index of all the game's HAG files
	 */
	void loadIndex(int gameID, bool isDemo);

	/**
	 * Given a resource name, opens up the correct HAG file and returns whether
	 * an entry with the given name exists.
	 */
	bool getHeaderEntry(const Common::Path &resourceName, HagIndex &hagIndex, HagEntry &hagEntry) const;

	/**
	 * Returns the HAG resource filename that will contain a given resource
	 */
	Common::Path getResourceFilename(const Common::Path &resourceName) const;

	/**
	 * Return a resource type given a resource name
	 */
	ResourceType getResourceType(const Common::String &resourceName) const;
public:
	explicit HagArchive(int gameID, bool isDemo);
	~HagArchive() override;

	// Archive implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
};

} // namespace MADS

#endif
