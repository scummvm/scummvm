// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_PHYSFS_PACKAGE_MANAGER_H
#define BS_PHYSFS_PACKAGE_MANAGER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "packagemanager.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class BS_Kernel;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_PhysfsPackageManager : public BS_PackageManager
{
public:
	BS_PhysfsPackageManager(BS_Kernel * KernelPtr);
	virtual ~BS_PhysfsPackageManager();

	virtual bool LoadPackage(const std::string & FileName, const std::string& MountPosition);
	virtual bool LoadDirectoryAsPackage(const std::string & DirectoryName, const std::string& MountPosition);
	virtual void* GetFile(const std::string& FileName, unsigned int * FileSizePtr = 0);
	virtual	std::string GetCurrentDirectory();
	virtual bool ChangeDirectory(const std::string & Directory);
	virtual std::string GetAbsolutePath(const std::string & FileName);
	virtual FileSearch* CreateSearch(const std::string & Filter, const std::string& Path, unsigned int TypeFilter = FT_DIRECTORY | FT_FILE);
	virtual unsigned int GetFileSize(const std::string & FileName);
	virtual unsigned int GetFileType(const std::string & FileName);
	virtual bool FileExists(const std::string & FileName);

private:
	std::string m_CurrentDirectory;
};

#endif
