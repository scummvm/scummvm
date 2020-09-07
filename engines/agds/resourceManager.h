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

#ifndef AGDS_RESOURCE_MANAGER_H
#define AGDS_RESOURCE_MANAGER_H

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/str.h"
#include "common/ptr.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Graphics { struct Surface; struct PixelFormat; }

namespace AGDS {

class ResourceManager {
private:

	class GrpFile;
	class ArchiveMember : public Common::ArchiveMember
	{
		GrpFile *		_parent;
		Common::String	_name;
		uint32			_offset;
		uint32			_size;

	public:
		ArchiveMember(GrpFile * parent, const Common::String &name, uint32 offset, uint32 size):
			_parent(parent), _name(name), _offset(offset), _size(size) {
		}
		virtual Common::SeekableReadStream *createReadStream() const;
		virtual Common::String getName() const {
			return _name;
		}
	};
	typedef Common::SharedPtr<ArchiveMember> ArchiveMemberPtr;

	class GrpFile : public Common::Archive
	{
		Common::File	_file;

		typedef Common::HashMap<Common::String, ArchiveMemberPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> MembersType;
		MembersType _members;

	public:
		bool load(const Common::String &fname);

		Common::SeekableReadStream &getArchiveStream() {
			return _file;
		}

		virtual bool hasFile(const Common::String &name) const
		{ return _members.find(name) != _members.end(); }

		virtual int listMembers(Common::ArchiveMemberList &list) const;
		virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
		virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
	};

public:
	ResourceManager();
	~ResourceManager();

	static void decrypt(uint8 * data, unsigned size);

	bool addPath(const Common::String &grpFilename);

	Common::SeekableReadStream * getResource(const Common::String &name) const;
	Graphics::Surface * loadPicture(const Common::String & name, const Graphics::PixelFormat &format);

	static Common::String loadText(Common::SeekableReadStream *stream);
	Common::String loadText(const Common::String & name) const;
};

Common::String readString(Common::SeekableReadStream * stream, uint size = 32);

} // End of namespace AGDS

#endif /* AGDS_RESOURCE_MANAGER_H */
