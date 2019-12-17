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

#include "nuvieDefs.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <list>
#include <dirent.h>

#include "Console.h"

#include "U6misc.h"

#include "NuvieFileList.h"

namespace Ultima {
namespace Ultima6 {

NuvieFileList::NuvieFileList() {
}

NuvieFileList::~NuvieFileList() {
}

bool NuvieFileList::open(const char *directory, const char *search, uint8 s_mode) {
	DIR *dir;
	struct dirent *entry;

	search_prefix.assign(search);
	sort_mode = s_mode;

	dir = opendir(directory);
	if (dir == NULL) {
		ConsoleAddWarning(std::string("Failed to open ") + directory);
		return false;
	}

	for (; (entry = readdir(dir));) {
		add_filename(directory, entry->d_name);
	}

	closedir(dir);

#if (_MSC_VER <= 1200)
	file_list.sort();
#else
	file_list.sort(NuvieFileDesc()); //sort list by time last modified in decending order.
#endif

	list_ptr = file_list.begin();

	return true;
}

bool NuvieFileList::add_filename(const char *directory, const char *filename) {
	struct stat sb;
	const char *sp =  search_prefix.c_str();
	NuvieFileDesc filedesc;
	std::string fullpath;

	if (filename == NULL || strlen(filename) == 0)
		return false;

	if (!strcmp(".", filename) || !strcmp("..", filename))
		return false;

	if (strlen(sp)) {
		if (strncmp(sp, filename, strlen(sp)) != 0)
			return false;
	}

	build_path(directory, filename, fullpath);

	if (stat(fullpath.c_str(), &sb) != 0) {
		DEBUG(0, LEVEL_ERROR, "Couldn't stat() file %s\n", fullpath.c_str());
		return false;
	}

	filedesc.m_time = sb.st_mtime;
	filedesc.filename.assign(filename);

	file_list.push_front(filedesc);

	return true;
}

std::string *NuvieFileList::next() {
	if (list_ptr != file_list.end()) {

		std::string *filename = &((*list_ptr).filename);
		list_ptr++;

		return filename;
	}

	return NULL;
}

std::string *NuvieFileList::get_latest() {
	std::list<NuvieFileDesc>::iterator iter;

	iter = file_list.begin();

	if (iter != file_list.end()) {
		std::string *filename = &((*iter).filename);

		return filename;
	}

	return NULL;
}

uint32 NuvieFileList::get_num_files() {
	return (uint32)file_list.size();
}

void NuvieFileList::close() {
	return;
}

std::set<std::string> NuvieFileList::get_filenames() {
	std::set<std::string> filenames;
	std::list<NuvieFileDesc>::iterator iter = file_list.begin();

	while (iter != file_list.end()) {
		filenames.insert((*iter).filename);
		iter++;
	}

	return filenames;
}

} // End of namespace Ultima6
} // End of namespace Ultima
