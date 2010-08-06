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

class BS_Kernel;

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

struct PathElement {
public:
	PathElement() { }
	PathElement(Common::String::const_iterator Begin, Common::String::const_iterator End) : m_Begin(Begin), m_End(End) {}

	Common::String::const_iterator GetBegin() const {
		return m_Begin;
	}
	Common::String::const_iterator GetEnd() const {
		return m_End;
	}

private:
	Common::String::const_iterator m_Begin;
	Common::String::const_iterator m_End;
};

typedef Common::Array<PathElement> PathElementArray;

class BS_ScummVMPackageManager : public BS_PackageManager {
private:
	class ArchiveEntry {
	public:
		Common::Archive *Archive;
		PathElementArray MountPath;

		ArchiveEntry(Common::Archive *Archive_, const PathElementArray &MountPath_):
			Archive(Archive_), MountPath(MountPath_) {
		}
		~ArchiveEntry() {
			delete Archive;
		}
	};

	Common::String _currentDirectory;
	Common::FSNode _rootFolder;
	Common::List<ArchiveEntry> _archiveList;

	Common::FSNode GetFSNode(const Common::String &FileName);
public:
	BS_ScummVMPackageManager(BS_Kernel *KernelPtr);
	virtual ~BS_ScummVMPackageManager();

	virtual bool LoadPackage(const Common::String &FileName, const Common::String &MountPosition);
	virtual bool LoadDirectoryAsPackage(const Common::String &DirectoryName, const Common::String &MountPosition);
	virtual void *GetFile(const Common::String &FileName, unsigned int *FileSizePtr = 0);
	virtual Common::String GetCurrentDirectory();
	virtual bool ChangeDirectory(const Common::String &Directory);
	virtual Common::String GetAbsolutePath(const Common::String &FileName);
	virtual FileSearch *CreateSearch(const Common::String &Filter, const Common::String &Path, unsigned int TypeFilter = FT_DIRECTORY | FT_FILE);
	virtual unsigned int GetFileSize(const Common::String &FileName);
	virtual unsigned int GetFileType(const Common::String &FileName);
	virtual bool FileExists(const Common::String &FileName);
};

} // End of namespace Sword25

#endif
