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

#ifndef ULTIMA6_FILES_NUVIE_FILE_LIST_H
#define ULTIMA6_FILES_NUVIE_FILE_LIST_H

#include <sys/types.h>
#include "ultima/shared/std/string.h"
#include <list>
#include <set>

namespace Ultima {
namespace Ultima6 {

using std::list;
using std::string;


#define NUVIE_SORT_TIME_DESC 0x1
#define NUVIE_SORT_TIME_ASC  0x2
#define NUVIE_SORT_NAME_DESC 0x3
#define NUVIE_SORT_NAME_ASC  0x5

class Configuration;

class NuvieFileDesc {
public:

	std::string filename;
	time_t m_time;

	bool operator<(const NuvieFileDesc &rhs) const {
		return (rhs.m_time < this->m_time);
	};
	bool operator()(const NuvieFileDesc &lhs, const NuvieFileDesc &rhs) {
		return (lhs.m_time > rhs.m_time);
	};
};

class NuvieFileList {
protected:

	std::list<NuvieFileDesc> file_list;
	std::list<NuvieFileDesc>::iterator list_ptr;

	std::string search_prefix;
	uint8 sort_mode;

public:

	NuvieFileList();
	~NuvieFileList();

	bool open(const char *directory, const char *restrict, uint8 sort_mode);


	std::string *next();
	std::string *get_latest();
	uint32 get_num_files();

	std::set<std::string> get_filenames();

	void close();

protected:

	bool add_filename(const char *directory, const char *filename);

};


} // End of namespace Ultima6
} // End of namespace Ultima

#endif
