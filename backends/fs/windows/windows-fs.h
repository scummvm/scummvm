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

#ifndef WINDOWS_FILESYSTEM_H
#define WINDOWS_FILESYSTEM_H

#include <windows.h>

#include "backends/fs/abstract-fs.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

/**
 * Implementation of the ScummVM file system API based on Windows API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WindowsFilesystemNode final : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isPseudoRoot;
	bool _isValid;

public:
	/**
	 * Creates a WindowsFilesystemNode with the root node as path.
	 *
	 * In regular windows systems, a virtual root path is used "".
	 */
	WindowsFilesystemNode();

	/**
	 * Creates a WindowsFilesystemNode for a given path.
	 *
	 * Examples:
	 *			path=c:\foo\bar.txt, currentDir=false -> c:\foo\bar.txt
	 *			path=c:\foo\bar.txt, currentDir=true -> current directory
	 *			path=NULL, currentDir=true -> current directory
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param currentDir if true, the path parameter will be ignored and the resulting node will point to the current directory.
	 */
	WindowsFilesystemNode(const Common::String &path, const bool currentDir);

	bool exists() const override;
	Common::U32String getDisplayName() const override { return _displayName; }
	Common::String getName() const override { return _displayName; }
	Common::String getPath() const override { return _path; }
	bool isDirectory() const override { return _isDirectory; }
	bool isReadable() const override;
	bool isWritable() const override;

	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	bool createDirectory() override;

private:
	/**
	 * Adds a single WindowsFilesystemNode to a given list.
	 * This method is used by getChildren() to populate the directory entries list.
	 *
	 * @param list         List to put the file entry node in.
	 * @param mode         Mode to use while adding the file entry to the list.
	 * @param base         Common::String with the directory being listed.
	 * @param hidden       true if hidden files should be added, false otherwise
	 * @param find_data    Describes a file that the FindFirstFile, FindFirstFileEx, or FindNextFile functions find.
	 */
	static void addFile(AbstractFSList &list, ListMode mode, const char *base, bool hidden, WIN32_FIND_DATA* find_data);

	/**
	 * Converts a string of TCHARs returned from a Windows API function to
	 * a character string. If UNICODE is defined then the incoming string
	 * is wide characters and is converted to UTF8, otherwise the incoming
	 * string is returned with no conversion.
	 *
	 * @param str String to convert if UNICODE is defined
	 * @return str in UTF8 format if UNICODE is defined, otherwise just str
	 */
	static const char *tcharToChar(const TCHAR *str);

	/**
	 * Converts a character string to a string of TCHARs for passing
	 * to a Windows API function. If UNICODE is defined then the incoming
	 * string is converted from UTF8 to wide characters, otherwise the incoming
	 * string is returned with no conversion.
	 *
	 * @param str String to convert if UNICODE is defined
	 * @return str in wide character format if UNICODE is defined, otherwise just str
	 */
	static const TCHAR* charToTchar(const char *str);

	/**
	 * Tests and sets the _isValid and _isDirectory flags, using the GetFileAttributes() function.
	 */
	virtual void setFlags();
};

#endif
