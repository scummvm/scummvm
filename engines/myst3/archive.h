/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MYST3_ARCHIVE_H
#define MYST3_ARCHIVE_H

#include "common/array.h"
#include "common/file.h"

#include "math/vector3d.h"

namespace Myst3 {

class ArchiveVisitor;
class ResourceDescription;

typedef Common::Array<ResourceDescription> ResourceDescriptionArray;

class Archive {
public:
	enum ResourceType {
		kCubeFace           =  0,
		kWaterEffectMask    =  1,
		kLavaEffectMask     =  2,
		kMagneticEffectMask =  3,
		kShieldEffectMask   =  4,
		kSpotItem           =  5,
		kFrame              =  6,
		kRawData            =  7,
		kMovie              =  8,
		kStillMovie         = 10,
		kText               = 11,
		kTextMetadata       = 12,
		kNumMetadata        = 13,
		kLocalizedSpotItem  = 69,
		kLocalizedFrame     = 70,
		kMultitrackMovie    = 72,
		kDialogMovie        = 74
	};

	struct DirectorySubEntry {
		uint32 offset;
		uint32 size;
		byte face;
		ResourceType type;
		Common::Array<uint32> metadata;

		DirectorySubEntry() : offset(0), size(0), face(0), type(kCubeFace) {}
	};

	struct DirectoryEntry {
		Common::String roomName;
		uint32 index;
		Common::Array<DirectorySubEntry> subentries;

		DirectoryEntry() : index(0) {}
	};

	ResourceDescription getDescription(const Common::String &room, uint32 index, uint16 face,
	                                   ResourceType type);
	ResourceDescriptionArray listFilesMatching(const Common::String &room, uint32 index, uint16 face,
	                                           ResourceType type);

	Common::SeekableReadStream *dumpToMemory(uint32 offset, uint32 size);
	uint32 copyTo(uint32 offset, uint32 size, Common::WriteStream &out);
	void visit(ArchiveVisitor &visitor);

	bool open(const char *fileName, const char *room);
	void close();

	const Common::String &getRoomName() const { return _roomName; }
	uint32 getDirectorySize() const { return _directorySize; }

private:
	Common::String _roomName;
	Common::File _file;
	uint32 _directorySize;
	Common::Array<DirectoryEntry> _directory;

	void decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream);
	void readDirectory();
	DirectorySubEntry readSubEntry(Common::ReadStream &stream);
	DirectoryEntry readEntry(Common::ReadStream &stream);
	const DirectoryEntry *getEntry(const Common::String &room, uint32 index) const;
};

class ResourceDescription {
public:
	struct SpotItemData {
		uint32 u;
		uint32 v;
	};

	struct VideoData {
		Math::Vector3d v1;
		Math::Vector3d v2;
		int32 u;
		int32 v;
		int32 width;
		int32 height;
	};

	ResourceDescription();
	ResourceDescription(Archive *archive, const Archive::DirectorySubEntry &subentry);

	bool isValid() const { return _archive && _subentry; }

	Common::SeekableReadStream *getData() const;
	uint16 getFace() const { return _subentry->face; }
	Archive::ResourceType getType() const { return _subentry->type; }
	SpotItemData getSpotItemData() const;
	VideoData getVideoData() const;
	uint32 getMiscData(uint index) const;
	Common::String getTextData(uint index) const;

private:
	Archive *_archive;
	const Archive::DirectorySubEntry *_subentry;
};

class ArchiveVisitor {
public:
	virtual ~ArchiveVisitor();

	virtual void visitArchive(Archive &archive) {}
	virtual void visitDirectoryEntry(Archive::DirectoryEntry &directoryEntry) {}
	virtual void visitDirectorySubEntry(Archive::DirectorySubEntry &directorySubEntry) {}
};

} // End of namespace Myst3

#endif
