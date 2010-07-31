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

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_PackageManager
 * -----------------
 * This is the package manager interface, that contains all the methods that a package manager
 * must implement.
 * In the package manager, note the following:
 * 1. It creates a completely new (virtual) directory tree in the packages and directories
 *    can be mounted.
 * 2. To seperate elements of a directory path '/' must be used rather than '\'
 * 3. LoadDirectoryAsPackage should only be used for testing. The final release will be
 *    have all files in packages.
 *
 * Autor: Malte Thiesen, $author$
 */

#ifndef SWORD25_PACKAGE_MANAGER_H
#define SWORD25_PACKAGE_MANAGER_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/service.h"

namespace Sword25 {

// Class definitions

/**
 * The Package Manager interface
 *
 * 1. It creates a completely new (virtual) directory tree in the packages and directories
 *    can be mounted.
 * 2. To seperate elements of a directory path '/' must be used rather than '\'
 * 3. LoadDirectoryAsPackage should only be used for testing. The final release will be
 *    have all files in packages.
 */
class BS_PackageManager : public BS_Service {
public:
	BS_PackageManager(BS_Kernel* pKernel);
	virtual ~BS_PackageManager() {};

	enum FILE_TYPES {
		FT_DIRECTORY	= (1<<0),
		FT_FILE			= (1<<1)
	};

	/**
	 * File search class
	 * 
	 * These objects are created with BS_PackageManager::CreateSearch
	 */
	class FileSearch {
	public:
		virtual ~FileSearch(){};

		/**
		 * Returns the filename of the current file
		 * @return			Returns the filename of the current file
		 */
		virtual Common::String GetCurFileName() = 0;
		/**
		 * Returns the type of the current file
		 * @return			Returns the type of the current file
		 * This is either BS_PackageManager::FT_FILE or BS_PackageManager::FT_DIRECTORY.
		 */
		virtual unsigned int GetCurFileType() = 0;
		/**
		 * Returns the size of the current file
		 * @return			Returns the size of the current file
		 * For directories, this value is always 0
		 */
		virtual unsigned int GetCurFileSize() = 0;
		// Finds the next file
		// Returns false if no more files are found.
		/**
		 * Finds the next file.
		 * @return			Returns false if no other file fulfills the search criteria
		*/
		virtual bool NextFile() = 0;
	};

	/**
	 * Mounts the contents of a package in the directory specified in the virtual directory tree.
	 * @param FileName		The filename of the package to mount
	 * @param MountPosition	The directory name under which the package should be mounted
	 * @return				Returns true if the mount was successful, otherwise false.
	 */
	virtual bool LoadPackage(const Common::String &FileName, const Common::String &MountPosition) = 0;
	/**
	 * Mounts the contents of a directory in the specified directory in the virtual directory tree.
	 * @param				The name of the directory to mount
	 * @param MountPosition	The directory name under which the package should be mounted
	 * @return				Returns true if the mount was successful, otherwise false.
	 */
	virtual bool LoadDirectoryAsPackage(const Common::String &DirectoryName, const Common::String &MountPosition) = 0;
	/**
	 * Downloads a file from the virtual directory tree
	 * @param FileName		The filename of the file to load
	 * @param pFileSize		Pointer to the variable that will contain the size of the loaded file. The deafult is NULL.
	 * @return				Specifies a pointer to the loaded data of the file
	 * @remark				The client must not forget to release the data of the file using BE_DELETE_A.
	 */
	virtual void *GetFile(const Common::String &FileName, unsigned int *pFileSize = NULL) = 0;
	/**
	 * Returns the path to the current directory.
	 * @return				Returns a string containing the path to the current directory.
	 * If the path could not be determined, an empty string is returned.
	 * @remark				For cutting path elements '\' is used rather than '/' elements.
	 */
	virtual		Common::String GetCurrentDirectory() = 0;
	/**
	 * Changes the current directory.
	 * @param Directory		The path to the new directory. The path can be relative.
	 * @return				Returns true if the operation was successful, otherwise false.
	 * @remark				For cutting path elements '\' is used rather than '/' elements.
	 */
	virtual bool ChangeDirectory(const Common::String& Directory) = 0;
	/**
	 * Returns the absolute path to a file in the virtual directory tree.
	 * @param FileName		The filename of the file whose absolute path is to be determined.
	 * These parameters may include both relative and absolute paths.
	 * @return				Returns an absolute path to the given file.
	 * @remark				For cutting path elements '\' is used rather than '/' elements.
	 */
	virtual Common::String GetAbsolutePath(const Common::String& FileName) = 0;
	/**
	 * Creates a BS_PackageManager::FileSearch object to search for files
	 * @param Filter		Specifies the search string. Wildcards of '*' and '?' are allowed
	 * @param Path			Specifies the directory that should be searched.
	 * @param TypeFilter	A combination of flags BS_PackageManager::FT_DIRECTORY and BS_PackageManager::FT_FILE.
	 * These flags indicate whether to search for files, directories, or both. 
	 * The default is BS_PackageManager::FT_DIRECTORY | BS_PackageManager::FT_FILE
	 * @return				Specifies a pointer to a BS_PackageManager::FileSearch object, or NULL if no file was found.
	 * @remark				Do not forget to delete the object after use.
	*/
	virtual FileSearch *CreateSearch(const Common::String &Filter, const Common::String &Path, unsigned int TypeFilter = FT_DIRECTORY | FT_FILE) = 0;

	/**
	 * Returns a file's size
	 * @param FileName		The filename
	 * @return				The file size. If an error occurs, then 0xffffffff is returned.
	 * @remarks				For files in packages, then uncompressed size is returned.
	 **/
	virtual unsigned int GetFileSize(const Common::String &FileName) = 0;

	/**
	 * Returns the type of a file.
	 * @param FileName		The filename
	 * @return				Returns the file type, either (BS_PackageManager::FT_DIRECTORY 
	 * or BS_PackageManager::FT_FILE).
	 * If the file was not found, then 0 is returned.
	 */
	virtual unsigned int GetFileType(const Common::String &FileName) = 0;

	/**
	 * Determines whether a file exists
	 * @param FileName		The filename
	 * @return				Returns true if the file exists, otherwise false.
	 */
	virtual bool FileExists(const Common::String & FileName) = 0;

private:
	bool _RegisterScriptBindings();
};

} // ENd of namespace Sword25

#endif
