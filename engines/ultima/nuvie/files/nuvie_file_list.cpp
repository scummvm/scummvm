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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/shared/std/string.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_file_list.h"
#include "common/fs.h"

namespace Ultima {
namespace Nuvie {

NuvieFileList::NuvieFileList() : sort_mode(NUVIE_SORT_NAME_ASC) {
}

NuvieFileList::~NuvieFileList() {
}

bool NuvieFileList::open(const char *directory, const char *search, uint8 s_mode) {
	Common::FSNode dir(directory);
	Common::FSList list;

	search_prefix.assign(search);
	sort_mode = s_mode;

	if (!dir.isDirectory()) {
		ConsoleAddWarning(Std::string("Failed to open ") + directory);
		return false;
	}

	if (!dir.getChildren(list, Common::FSNode::kListFilesOnly)) {
		ConsoleAddWarning(Std::string("Failed to get children of ") + directory);
		return false;
	};
	for (Common::FSList::iterator it = list.begin(); it != list.end(); ++it)
		add_filename(*it);

	//sort list by time last modified in decending order.
	Common::sort(file_list.begin(), file_list.end(), NuvieFileDesc());
	list_ptr = file_list.begin();

	return true;
}

bool NuvieFileList::add_filename(const Common::FSNode &file) {
	NuvieFileDesc filedesc;
	filedesc.m_time = 0;
	filedesc.filename.assign(file.getName());

	file_list.push_front(filedesc);

	return true;
}

Std::string *NuvieFileList::next() {
	if (list_ptr != file_list.end()) {

		Std::string *filename = &((*list_ptr).filename);
		list_ptr++;

		return filename;
	}

	return NULL;
}

Std::string *NuvieFileList::get_latest() {
	Std::list<NuvieFileDesc>::iterator iter;

	iter = file_list.begin();

	if (iter != file_list.end()) {
		Std::string *filename = &((*iter).filename);

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

Std::set<Std::string> NuvieFileList::get_filenames() {
	Std::set<Std::string> filenames;
	Std::list<NuvieFileDesc>::iterator iter = file_list.begin();

	while (iter != file_list.end()) {
		filenames.insert((*iter).filename);
		iter++;
	}

	return filenames;
}

} // End of namespace Nuvie
} // End of namespace Ultima
