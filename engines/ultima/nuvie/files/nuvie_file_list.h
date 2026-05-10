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

#ifndef NUVIE_FILES_NUVIE_FILE_LIST_H
#define NUVIE_FILES_NUVIE_FILE_LIST_H

#include "common/str.h"
#include "common/fs.h"

namespace Ultima {
namespace Nuvie {


#define NUVIE_SORT_TIME_DESC 0x1
#define NUVIE_SORT_TIME_ASC  0x2
#define NUVIE_SORT_NAME_DESC 0x3
#define NUVIE_SORT_NAME_ASC  0x5

class Configuration;

class NuvieFileDesc {
public:
	Common::String filename;
	uint32 m_time;

	bool operator<(const NuvieFileDesc &rhs) const {
		return (rhs.m_time < this->m_time);
	};
	bool operator()(const NuvieFileDesc &lhs, const NuvieFileDesc &rhs) {
		return (lhs.m_time > rhs.m_time);
	};
};

class NuvieFileList {
protected:
	Common::List<NuvieFileDesc> file_list;

	uint8 sort_mode;
protected:
	bool add_filename(const Common::String &fileName);
public:

	NuvieFileList();
	~NuvieFileList();

	bool open(const Common::Path &directory, const char *restrict, uint8 sort_mode);

	Common::String *next();
	const Common::String *get_latest() const;
	uint32 get_num_files() const;

	const Common::List<NuvieFileDesc> &get_filelist() const;

	void close();
};


} // End of namespace Nuvie
} // End of namespace Ultima

#endif
