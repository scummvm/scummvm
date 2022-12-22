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

#ifndef MADS_MPS_INSTALLER_H
#define MADS_MPS_INSTALLER_H

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace MADS {

class MpsInstaller : public Common::MemcachingCaseInsensitiveArchive {
public:
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList&) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SharedArchiveContents readContentsForPath(const Common::String& translatedPath) const override;

	static MpsInstaller* open(const Common::Path& baseName);

private:
	// Similar to FileDescriptionBin but in native-endian and native strings.
	class FileDescriptor {
	public:
		// Public for hashmap
		FileDescriptor() : _compressedSize(0),
				   _uncompressedSize(),
				   _compressionAlgo(0),
				   _offsetInVolume(0),
				   _volumeNumber(0) {}
	protected:
		FileDescriptor(const Common::String& name,
			       uint16 compression,
			       uint16 volumeNumber,
			       uint32 offsetInVolume,
			       uint32 compressedSize,
			       uint32 uncompressedSize) :
			_fileName(name),
			_compressionAlgo(compression),
			_volumeNumber(volumeNumber),
			_offsetInVolume(offsetInVolume),
			_compressedSize(compressedSize),
			_uncompressedSize(uncompressedSize) {}
		
		Common::String _fileName;
		uint _compressionAlgo;
		uint _volumeNumber;
		uint32 _offsetInVolume;
		uint32 _compressedSize;
		uint32 _uncompressedSize;

		friend class MpsInstaller;
	};

    	typedef Common::HashMap<Common::String, FileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	MpsInstaller(const FileMap& files,
		     const Common::Path& baseName) : _files(files), _baseName(baseName) {}

	FileMap _files;
	Common::Path _baseName;
};
}

#endif
