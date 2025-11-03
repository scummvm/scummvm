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

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "common/archive.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
struct PixelFormat;
} // namespace Graphics

namespace AGDS {

class ResourceManager {
private:
	class GrpFile;
	class ArchiveMember : public Common::ArchiveMember {
		GrpFile *_parent;
		Common::String _name;
		uint32 _offset;
		uint32 _size;

	public:
		ArchiveMember(GrpFile *parent, const Common::String &name, uint32 offset, uint32 size) : _parent(parent), _name(name), _offset(offset), _size(size) {
		}
		Common::SeekableReadStream *createReadStream() const override;
		Common::String getName() const override {
			return getFileName();
		}
		Common::Path getPathInArchive() const override {
			return Common::Path{_name};
		}
		Common::String getFileName() const override {
			return _name;
		}
		Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override {
			return nullptr;
		}
	};
	using ArchiveMemberPtr = Common::SharedPtr<ArchiveMember>;

	class GrpFile : public Common::Archive {
		Common::File _file;

		using MembersType = Common::HashMap<Common::String, ArchiveMemberPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>;
		MembersType _members;

	public:
		bool load(const Common::Path &path);

		Common::SeekableReadStream &getArchiveStream() {
			return _file;
		}

		bool hasFile(const Common::Path &name) const override;
		int listMembers(Common::ArchiveMemberList &list) const override;
		const Common::ArchiveMemberPtr getMember(const Common::Path &name) const override;
		Common::SeekableReadStream *createReadStreamForMember(const Common::Path &name) const override;
	};

public:
	ResourceManager();
	~ResourceManager();

	static void decrypt(uint8 *data, unsigned size);
	static bool IsBMP(Common::SeekableReadStream &stream);

	bool addPath(const Common::Path &grpFilename);

	Common::SeekableReadStream *getResource(const Common::String &name) const;
	Graphics::Surface *loadPicture(const Common::String &name, const Graphics::PixelFormat &format);

	static Common::String loadText(Common::SeekableReadStream &stream);
	Common::String loadText(const Common::String &name) const;
};

Common::String readString(Common::ReadStream &stream, uint size = 32);
void writeString(Common::WriteStream &stream, const Common::String &string, uint size = 32);

} // End of namespace AGDS

#endif /* AGDS_RESOURCE_MANAGER_H */
