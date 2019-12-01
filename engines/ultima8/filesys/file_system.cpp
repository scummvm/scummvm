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

#include "ultima8/misc/pent_include.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/std/string.h"
#include "ultima8/ultima8.h"

namespace Ultima8 {

using std::string;

FileSystem *FileSystem::filesystem = 0;

FileSystem::FileSystem(bool noforced)
	: noforcedvpaths(noforced), allowdataoverride(true) {
	con.Print(MM_INFO, "Creating FileSystem...\n");

	assert(filesystem == 0);
	filesystem = this;

#ifdef UNDER_CE
	TCHAR module_filename[256];
	TCHAR *c = module_filename;
	TCHAR *last = NULL;
	GetModuleFileName(NULL, module_filename, 256);

	while (*c) {
		if (*c == '/' || *c == '\\')
			last = c;

		c++;
	}

	if (last) {
		*last = 0;
	} else {
		module_filename[0] = '\\';
		module_filename[1] = 0;
	}

	size_t len = _tcslen(module_filename) + 1;
	char *str = (char *) _alloca(len);
	WideCharToMultiByte(CP_ACP, 0, module_filename, -1, str, len, NULL, NULL);

	AddVirtualPath(".", str);

#endif

}

FileSystem::~FileSystem() {
	con.Print(MM_INFO, "Destroying FileSystem...\n");

	filesystem = 0;
}


// Open a streaming file as readable. Streamed (0 on failure)
IDataSource *FileSystem::ReadFile(const string &vfn, bool is_text) {
	string filename = vfn;

	IDataSource *data = checkBuiltinData(vfn, is_text);

	// allow data-override?
	if (!allowdataoverride && data) return data;

	Common::File *f = new Common::File();
	if (!f->open(filename)) {
		delete f;
		return data;
	}

	return new IFileDataSource(f);
}

// Open a streaming file as writeable. Streamed (0 on failure)
ODataSource *FileSystem::WriteFile(const string &vfn, bool is_text) {
	string filename = vfn;
	Common::DumpFile *f = new Common::DumpFile();

	if (!f->open(filename)) {
		delete f;
		return nullptr;
	}

	return new OFileDataSource(f);
}

void FileSystem::switch_slashes(string &name) {
	for (string::iterator X = name.begin(); X != name.end(); ++X) {
		if (*X == '/')
			*X =  '\\';
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
	string::iterator X;
	for (X = str.end(); X >= str.begin(); --X) {
		// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = static_cast<char>(std::toupper(*X));
#endif
	}
	if (X >= str.begin())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}

bool FileSystem::AddVirtualPath(const string &vpath, const string &realpath, const bool create) {
	string vp = vpath, rp = realpath;

	// remove trailing slash
	if (vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	if (rp.rfind('/') == rp.size() - 1)
		rp.erase(rp.rfind('/'));

	if (rp.find("..") != string::npos) {
		con.Printf_err(MM_MINOR_ERR,
		               "Error mounting virtual path \"%s\": \"..\" not allowed.\n",
		               vp.c_str());
		return false;
	}

	// Finding Reserved Virtual Path Names
	// memory path is reserved
	if (vp == "@memory" || vp.substr(0, 8) == "@memory/") {
		con.Printf_err(MM_MINOR_ERR,
		               "Error mounting virtual path \"%s\": %s\"@memory\" is a reserved virtual path name.\n",
		               vp.c_str());
		return false;
	}

	string fullpath = rp;
	rewrite_virtual_path(fullpath);
	// When mounting a memory file, it wont exist, so don't attempt to create the dir
#ifdef DEBUG
	con.Printf(MM_INFO, "virtual path \"%s\": %s\n", vp.c_str(), fullpath.c_str());
#endif
	if (!(fullpath.substr(0, 8) == "@memory/")) {
		if (!IsDir(fullpath)) {
			if (!create) {
#ifdef DEBUG
				con.Printf_err(MM_MINOR_WARN,
				               "Problem mounting virtual path \"%s\": directory not found: %s\n",
				               vp.c_str(), fullpath.c_str());
#endif
				return false;
			} else {
				MkDir(fullpath);
			}
		}
	}

	virtualpaths[vp] = rp;
	return true;
}

bool FileSystem::RemoveVirtualPath(const string &vpath) {
	string vp = vpath;

	// remove trailing slash
	if (vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	std::map<Common::String, string>::iterator i = virtualpaths.find(vp);

	if (i == virtualpaths.end()) {
		return false;
	} else {
		virtualpaths.erase(vp);
		return true;
	}
}

IDataSource *FileSystem::checkBuiltinData(const std::string &vfn, bool is_text) {
	// Is it a Memory file?
	std::map<Common::String, MemoryFile *>::iterator mf = memoryfiles.find(vfn);

	if (mf != memoryfiles.end())
		return new IBufferDataSource(mf->_value->data,
		                             mf->_value->len, is_text);

	return 0;
}

bool FileSystem::rewrite_virtual_path(string &vfn) {
	bool ret = false;
	string::size_type pos = vfn.size();

	while ((pos = vfn.rfind('/', pos)) != std::string::npos) {
//		perr << vfn << ", " << vfn.substr(0, pos) << ", " << pos << std::endl;
		std::map<Common::String, string>::iterator p = virtualpaths.find(
		            vfn.substr(0, pos));

		if (p != virtualpaths.end()) {
			ret = true;
			// rewrite first part of path
			vfn = p->_value + vfn.substr(pos);
			pos = string::npos;
		} else {
			if (pos == 0)
				break;
			--pos;
		}
	}

	// We will allow all paths to work
	if (noforcedvpaths) ret = true;

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

/*
 *  Get the current users pentagram home path
 */

std::string FileSystem::getHomePath() {
	Common::FSNode gameDir = g_vm->getGameDirectory();
	return gameDir.getPath();
}

} // End of namespace Ultima8
