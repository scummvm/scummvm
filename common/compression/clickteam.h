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

#ifndef COMMON_CLICKTEAM_H
#define COMMON_CLICKTEAM_H

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {
class ClickteamInstaller : public MemcachingCaseInsensitiveArchive {
public:
	enum class ClickteamTagId : uint16 {
		BANNER_IMAGE = 0x1235,
		FILE_LIST = 0x123a,
		STRINGS = 0x123e,
		UNINSTALLER = 0x123f,
		FILE_PATCHING_LIST = 0x1242,
	};

	class ClickteamTag : Common::NonCopyable {
	protected:
		ClickteamTag(uint16 tagId, byte *contents, uint32 size) : _tagId(tagId), _contents(contents), _size(size) {
		}
		friend class ClickteamInstaller;
	public:
		uint16 _tagId;
		byte *_contents;
		uint32 _size;

		~ClickteamTag() {
			delete[] _contents;
		}
	};

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList&) const override;
	const ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SharedArchiveContents readContentsForPath(const Common::String& translated) const override;

	ClickteamTag* getTag(ClickteamTagId tagId) const;

	static ClickteamInstaller* openPatch(Common::SeekableReadStream *stream, bool verifyOriginal = true, bool verifyAllowSkip = true,
					     Common::Archive *reference = &SearchMan, DisposeAfterUse::Flag dispose = DisposeAfterUse::NO);
	static ClickteamInstaller* open(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose = DisposeAfterUse::NO);

private:
	struct ClickteamPatchDescriptor {
		uint32 _originalCRC;
		uint32 _originalSize;
		uint32 _patchDataOffset;
		uint32 _patchSize;
	};
	class ClickteamFileDescriptor {
	private:
		Common::String _fileName;

		// Offset of the file contents relative to the beginning of block3
		uint32 _fileDataOffset;
		// Offset of file descriptor
		uint32 _fileDescriptorOffset;
		uint32 _compressedSize;
		uint32 _uncompressedSize;
		uint32 _expectedCRC;
		bool _supported;
		bool _isPatchFile;
		bool _crcIsXorred;
		bool _isReferenceMissing;
		//uint16 _field1c;
		Common::Array<ClickteamPatchDescriptor> _patchEntries;

		ClickteamFileDescriptor(const ClickteamTag& contentsTag, uint32 off);
		friend class ClickteamInstaller;
	public:
		// It's public for hashmap
		ClickteamFileDescriptor() : _fileDataOffset(0), _fileDescriptorOffset(0), _compressedSize(0), _uncompressedSize(0) {}
	};

	ClickteamInstaller(Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> files,
			   Common::HashMap<uint16, Common::SharedPtr<ClickteamTag>> tags,
			   uint32 crcXor, uint32 block3Offset, uint32 block3Size, Common::SeekableReadStream *stream,
			   Common::Archive *reference,
			   DisposeAfterUse::Flag dispose)
		: _files(files), _tags(tags), _crcXor(crcXor), _block3Offset(block3Offset), /*_block3Size(block3Size), */_stream(stream, dispose),
		  _reference(reference) {
	}

	static int findPatchIdx(const ClickteamFileDescriptor &desc, Common::SeekableReadStream *refStream, const Common::String &fileName,
				uint32 crcXor, bool doWarn);
	Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _files;
	Common::HashMap<uint16, Common::SharedPtr<ClickteamTag>> _tags;
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	uint32 _crcXor, _block3Offset/*, _block3Size*/;
	Common::Archive *_reference;
};
}
#endif
