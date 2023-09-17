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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_CIF_H
#define NANCY_CIF_H

#include "common/archive.h"
#include "common/rect.h"
#include "common/hashmap.h"

namespace Common {
class SeekableReadStream;
class Serializer;
}

namespace Nancy {

struct CifInfo {
	enum ResType : byte {
		kResTypeAny,
		// Type 1 seems to be obsolete
		kResTypeImage = 2,
		kResTypeScript = 3,
		kResTypeEmpty = 4
	};

	enum ResCompression {
		kResCompressionNone = 1,
		kResCompression = 2
	};

	Common::Path name;
	ResType type = kResTypeEmpty; // ResType
	ResCompression comp = kResCompressionNone; // ResCompression
	uint16 width = 0, pitch = 0, height = 0;
	byte depth = 0; // Bit depth
	uint32 compressedSize = 0, size = 0;
	Common::Rect src, dest; // Used when drawing conversation cels

	uint32 dataOffset;
};

// Wrapper for a single file. Exclusively used for scene IFFs, though it can contain anything.
class CifFile {
private:
friend class ResourceManager;
	CifFile() : _stream(nullptr) {}
public:
	CifFile(Common::SeekableReadStream *stream, const Common::Path &name);
	~CifFile();

	Common::SeekableReadStream *createReadStream() const;

private:
	bool sync(Common::Serializer &ser);
	Common::SeekableReadStream *createReadStreamRaw() const;

	Common::SeekableReadStream *_stream;
	CifInfo _info;
};

// Container type comprising of multiple CIF files. Contrary to its name it contains no tree structure.
class CifTree : public Common::Archive {
protected:
friend class ResourceManager;
	CifTree() : _stream(nullptr) {}
	CifTree(Common::SeekableReadStream *stream, const Common::Path &name);
	virtual ~CifTree();

public:
	// Used for extracting additional image data for conversation cels (nancy2 and up)
	const CifInfo &getCifInfo(const Common::Path &name) const;

	// Archive interface
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	Common::Path getName() const { return _name; }

	static CifTree *makeCifTreeArchive(const Common::String &name, const Common::String &ext);

private:
	bool sync(Common::Serializer &ser);
	Common::SeekableReadStream *createReadStreamRaw(const Common::Path &path) const;

	Common::Path _name;
	Common::SeekableReadStream *_stream;
	Common::HashMap<Common::Path, CifInfo, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> _fileMap;
	Common::Array<CifInfo> _writeFileMap;
};

// Ciftree that only provides a file if a certain ConfMan flag is true. Used for handling game patches
class PatchTree : public CifTree {
public:
	PatchTree(Common::SeekableReadStream *stream, const Common::Path &name) : CifTree(stream, name) {}
	virtual ~PatchTree() {}

	bool hasFile(const Common::Path &path) const override;

	Common::Array<Common::Pair<Common::Array<Common::Pair<Common::String, Common::String>>, Common::Array<Common::Path>>> _associations;
};

} // End of namespace Nancy

#endif // NANCY_CIF_H
