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

#ifndef ULTIMA8_FILESYS_FILESYSTEM_H
#define ULTIMA8_FILESYS_FILESYSTEM_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima8 {

class FileSystem {
public:
	//! \param noforcedvpaths if false, all file operations must use vpaths
	FileSystem(bool noforcedvpaths = false);
	~FileSystem();

	//! Initialize builtin data files.
	void initBuiltinData(bool allowoverride);

	static FileSystem *get_instance() {
		return _fileSystem;
	}

	//! Open a file as readable. Streamed.
	//! \param vfn the (virtual) filename
	//! \param is_text open in text mode?
	//! \return nullptr on failure
	IDataSource *ReadFile(const Std::string &vfn, bool is_text = false);

	//! Open a file as writable. Streamed.
	//! \param vfn the (virtual) filename
	//! \param is_text open in text mode?
	//! \return a new writestream, or nullptr on failure
	Common::WriteStream *WriteFile(const Std::string &vfn, bool is_text = false);

	//! Mount a virtual path
	//! \param vpath the name of the vpath (should start with '@')
	//! \param realpath the name of the path to mount (note that this can
	//!                 be a virtual path itself)
	//! \param create create realpath directory if it doesn't exist?
	//! \return true if succesful
	bool AddVirtualPath(const Std::string &vpath, const Std::string &realpath,
	                    bool create = false);

	//! Unmount a virtual path
	bool RemoveVirtualPath(const Std::string &vpath);

	//! Create a directory
	//! \param path the directory to create. (Can be virtual)
	//! \return true if successful; otherwise, false.
	bool MkDir(const Std::string &path); // can handle both paths and vpaths

private:
	static void switch_slashes(Std::string &name);
	static bool base_to_uppercase(Std::string &str, int count);

	static bool IsDir(const Std::string &path);

	static FileSystem *_fileSystem;

	/**
	 *	Open a file for reading,
	 *	Output: false if couldn't open.
	 */
	bool rawOpen(Common::SeekableReadStream *&in, const Std::string &fname);

	/**
	 *	Open a file for writing,
	 *	Output: false if couldn't open.
	 */
	bool rawOpen(Common::WriteStream *&out, const Std::string &fname);

	// This will disable the usage of forced virtual paths.
	// It's useful for 'tools'
	bool    _noForcedVPaths;

	// This enables/disables overriding builtin data files with external ones
	bool    _allowDataOverride;

	// rewrite virtual path in-place (i.e., fvn is replaced)
	// returns false if no rewriting was done
	bool rewrite_virtual_path(Std::string &vfn) const;

	Std::map<Common::String, Std::string> _virtualPaths;

	//! Check if the given file is a builtin data file.
	//! If so, return an IDataSource for it. If not, return nullptr.
	IDataSource *checkBuiltinData(const Std::string &vfn, bool is_text = false);

	struct MemoryFile {
		MemoryFile(const uint8 *data, const uint32 len)
			: _data(data), _len(len) { }
		const uint8 *_data;
		const uint32 _len;
	};
	Std::map<Common::String, MemoryFile *> _memoryFiles; // Files mounted in memory
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
