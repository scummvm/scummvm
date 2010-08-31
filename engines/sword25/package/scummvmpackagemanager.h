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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SWORD25_SCUMMVM_PACKAGE_MANAGER_H
#define SWORD25_SCUMMVM_PACKAGE_MANAGER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/archive.h"
#include "common/array.h"
#include "common/fs.h"
#include "common/str.h"
#include "sword25/kernel/common.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class Kernel;

class ScummVMPackageManager : public PackageManager {
private:
	class ArchiveEntry {
	public:
		Common::Archive *archive;
		Common::String _mountPath;

		ArchiveEntry(Common::Archive *archive_, const Common::String &mountPath_):
			archive(archive_), _mountPath(mountPath_) {
		}
		~ArchiveEntry() {
			delete archive;
		}
	};

	Common::String _currentDirectory;
	Common::FSNode _rootFolder;
	Common::List<ArchiveEntry *> _archiveList;

	Common::ArchiveMemberPtr GetArchiveMember(const Common::String &fileName);

public:
	ScummVMPackageManager(Kernel *kernelPtr);
	virtual ~ScummVMPackageManager();

	virtual bool LoadPackage(const Common::String &fileName, const Common::String &mountPosition);
	virtual bool LoadDirectoryAsPackage(const Common::String &directoryName, const Common::String &mountPosition);
	virtual byte *GetFile(const Common::String &fileName, unsigned int *fileSizePtr = 0);
	virtual Common::SeekableReadStream *GetStream(const Common::String &fileName);
	virtual Common::String GetCurrentDirectory();
	virtual bool ChangeDirectory(const Common::String &directory);
	virtual Common::String GetAbsolutePath(const Common::String &fileName);
	virtual int doSearch(Common::ArchiveMemberList &list, const Common::String &filter, const Common::String &path, unsigned int typeFilter = FT_DIRECTORY | FT_FILE);
	virtual unsigned int GetFileSize(const Common::String &fileName);
	virtual unsigned int GetFileType(const Common::String &fileName);
	virtual bool FileExists(const Common::String &fileName);
};

} // End of namespace Sword25

#endif
