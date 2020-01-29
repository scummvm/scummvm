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

#include "ultima/ultima8/filesys/dir_file.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/file_system.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(DirFile, NamedArchiveFile)

DirFile::DirFile(const Std::string &path_) {
	path = path_;
	if (path.size() == 0) {
		valid = false;
		return;
	}

	if (path[path.size() - 1] != '/') path += "/";

	valid = readMetadata();
}


DirFile::~DirFile() {

}

bool DirFile::readMetadata() {
	FileSystem *filesys = FileSystem::get_instance();
	FileSystem::FileList files;

	/*int ret =*/ filesys->ListFiles(path + "*", files);

	// TODO: check if directory actually exists

	count = files.size();

	FileSystem::FileList::iterator iter;

	for (iter = files.begin(); iter != files.end(); ++iter) {
		Std::string name = *iter;
		Std::string::size_type pos = name.rfind("/");
		if (pos != Std::string::npos) {
			name.erase(0, pos + 1);
			//pout << "DirFile: " << name << Std::endl;
			storeIndexedName(name);
		}
	}

	return true;
}

bool DirFile::exists(const Std::string &name) {
	FileSystem *filesys = FileSystem::get_instance();
	IDataSource *ids = filesys->ReadFile(path + name);
	if (!ids) return false;

	delete ids;
	return true;
}

uint32 DirFile::getSize(const Std::string &name) {
	FileSystem *filesys = FileSystem::get_instance();
	IDataSource *ids = filesys->ReadFile(path + name);
	if (!ids) return 0;

	uint32 size = ids->getSize();
	delete ids;
	return size;
}

uint8 *DirFile::getObject(const Std::string &name, uint32 *sizep) {
	FileSystem *filesys = FileSystem::get_instance();
	IDataSource *ids = filesys->ReadFile(path + name);
	if (!ids) return 0;

	uint32 size = ids->getSize();
	if (size == 0) return 0;

	uint8 *buf = new uint8[size];
	ids->read(buf, size);
	delete ids;

	if (sizep) *sizep = size;

	return buf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
