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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/shared/std/string.h"
#include "ultima/ultima8/ultima8.h"
#include "common/system.h"
#include "common/memstream.h"
#include "common/savefile.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

FileSystem *FileSystem::_fileSystem = nullptr;

FileSystem::FileSystem(bool noforced)
	: _noForcedVPaths(noforced), _allowDataOverride(true) {
	debugN(MM_INFO, "Creating FileSystem...\n");

	_fileSystem = this;
	AddVirtualPath("@home", "");
}

FileSystem::~FileSystem() {
	debugN(MM_INFO, "Destroying FileSystem...\n");

	_fileSystem = nullptr;
}


// Open a streaming file as readable. Streamed (0 on failure)
IDataSource *FileSystem::ReadFile(const string &vfn, bool is_text) {
	IDataSource *data = checkBuiltinData(vfn, is_text);

	// allow data-override?
	if (!_allowDataOverride && data)
		return data;

	if (data)
		delete data;

	Common::SeekableReadStream *readStream;
	if (!rawOpen(readStream, vfn))
		return nullptr;

	return new IFileDataSource(readStream);
}

// Open a streaming file as writeable. Streamed (0 on failure)
Common::WriteStream *FileSystem::WriteFile(const string &vfn, bool is_text) {
	string filename = vfn;
	Common::WriteStream *writeStream;

	if (!rawOpen(writeStream, filename))
		return nullptr;

	return writeStream;
}

bool FileSystem::rawOpen(Common::SeekableReadStream *&in, const string &fname) {
	string name = fname;
	Common::File *f;

	// Handle reading files from the ultima.dat data
	if (name.hasPrefix("@data/")) {
		// It's a file specifically from the ultima.dat file
		f = new Common::File();
		if (f->open(Common::String::format("data/%s", name.substr(6).c_str()))) {
			in = f;
			return true;
		}

		f->close();
		delete f;
	}

	// Handle opening savegames
	if (name.hasPrefix("@save/")) {
		int slotNumber = Std::atoi(name.c_str() + 6);
		Std::string saveFilename = Ultima8Engine::get_instance()->getSaveStateName(slotNumber);

		in = g_system->getSavefileManager()->openForLoading(saveFilename);
		return in != nullptr;
	}

	if (!rewrite_virtual_path(name))
		return false;

	switch_slashes(name);

	int uppercasecount = 0;
	f = new Common::File();
	do {
		if (f->open(name)) {
			in = f;
			return true;
		}
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found
	delete f;
	return false;
}


bool FileSystem::rawOpen(Common::WriteStream *&out,  const string &fname) {
	string name = fname;
	switch_slashes(name);

	if (name.hasPrefix("@save/")) {
		int slotNumber = Std::atoi(name.c_str() + 6);
		Std::string saveFilename = Ultima8Engine::get_instance()->getSaveStateName(slotNumber);

		out = g_system->getSavefileManager()->openForSaving(saveFilename, false);
		return out != nullptr;
	} else {
		return false;
	}

#if 0
	if (!rewrite_virtual_path(name)) {
		warning("Illegal file access");
		return false;
	}

	int uppercasecount = 0;
	do {
		if (out.open(name))
			return true;

	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found
	return false;
#endif
}

void FileSystem::switch_slashes(string &name) {
	for (string::iterator X = name.begin(); X != name.end(); ++X) {
		if (*X == '\\')
			*X =  '/';
	}
}

/*
 *  Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

bool FileSystem::base_to_uppercase(string &str, int count) {
	if (count <= 0) return true;

	int todo = count;
	// Go backwards.
	string::reverse_iterator X;
	for (X = str.rbegin(); X != str.rend(); ++X) {
		// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

		*X = static_cast<char>(Std::toUpper(*X));
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}

bool FileSystem::AddVirtualPath(const string &vpath, const string &realpath, const bool create) {
	string vp = vpath, rp = realpath;

	// remove trailing slash
	if (vp.size() && vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	if (rp.size() && rp.rfind('/') == rp.size() - 1)
		rp.erase(rp.rfind('/'));

	if (rp.find("..") != string::npos) {
		warning("Error mounting virtual path \"%s\": \"..\" not allowed", vp.c_str());
		return false;
	}

	// Finding Reserved Virtual Path Names
	// memory path is reserved
	if (vp == "@memory" || vp.substr(0, 8) == "@memory/") {
		warning("Error mounting virtual path \"%s\": \"@memory\" is a reserved virtual path name",
			vp.c_str());
		return false;
	}

	string fullpath = rp;
	rewrite_virtual_path(fullpath);
	// When mounting a memory file, it wont exist, so don't attempt to create the dir
#ifdef DEBUG
	debugN(MM_INFO, "virtual path \"%s\": %s\n", vp.c_str(), fullpath.c_str());
#endif
	if (!(fullpath.substr(0, 8) == "@memory/") && rp.length()) {
		if (!IsDir(fullpath)) {
			if (!create) {
#ifdef DEBUG
				warning("Problem mounting virtual path \"%s\": directory not found: %s",
					vp.c_str(), fullpath.c_str());
#endif
				return false;
			} else {
				MkDir(fullpath);
			}
		}
	}

	_virtualPaths[vp] = rp;
	return true;
}

bool FileSystem::RemoveVirtualPath(const string &vpath) {
	string vp = vpath;

	// remove trailing slash
	if (vp.size() && vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	Std::map<Common::String, string>::iterator i = _virtualPaths.find(vp);

	if (i == _virtualPaths.end()) {
		return false;
	} else {
		_virtualPaths.erase(vp);
		return true;
	}
}

IDataSource *FileSystem::checkBuiltinData(const Std::string &vfn, bool is_text) {
	// Is it a Memory file?
	Std::map<Common::String, MemoryFile *>::const_iterator mf = _memoryFiles.find(vfn);

	if (mf != _memoryFiles.end())
		return new IBufferDataSource(mf->_value->_data,
		                             mf->_value->_len, is_text);

	return nullptr;
}

bool FileSystem::rewrite_virtual_path(string &vfn) const {
	bool ret = false;
	string::size_type pos = vfn.size();

	while ((pos = vfn.rfind('/', pos)) != Std::string::npos) {
//		perr << vfn << ", '" << vfn.substr(0, pos) << "', " << pos << Std::endl;
		Std::map<Common::String, string>::const_iterator p = _virtualPaths.find(
		            vfn.substr(0, pos));

		if (p != _virtualPaths.end()) {
			ret = true;
			// rewrite first part of path
			vfn = p->_value + vfn.substr(pos + 1);
			pos = string::npos;
		} else {
			if (pos == 0)
				break;
			--pos;
		}
	}

	// We will allow all paths to work
	if (_noForcedVPaths) ret = true;

	return ret;
}


bool FileSystem::IsDir(const string &path) {
	Common::FSNode node(path);
	return node.isDirectory();
}

/*
 *  Create a directory
 */

bool FileSystem::MkDir(const string &path) {
	Common::FSNode newDir(path);
	return newDir.createDirectory();
}

} // End of namespace Ultima8
} // End of namespace Ultima
