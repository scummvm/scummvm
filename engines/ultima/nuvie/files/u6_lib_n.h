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

#ifndef NUVIE_FILES_U6LIB_N_H
#define NUVIE_FILES_U6LIB_N_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "common/stream.h"

namespace Ultima {
namespace Nuvie {

using Std::string;
//using Std::vector;

class NuvieIO;

struct U6LibItem {
	uint32 offset;
	uint8 flag;
	uint32 uncomp_size;
	uint32 size;
	string *name;
	unsigned char *data; // for writing or cache
};

class U6Lib_n {
	uint32 filesize; // total size of file
	uint8 game_type; // there are three types of lib files.
	uint8 lib_size; // measured in bytes either 2 or 4
	uint32 num_offsets; // number of items, size of lists
	U6LibItem *items;
	NuvieIO *data;
	bool del_data;

public:
	U6Lib_n();
	~U6Lib_n();

	bool open(Std::string &filename, uint8 size, uint8 type = NUVIE_GAME_U6);
	bool open(NuvieIO *new_data, uint8 size, uint8 type = NUVIE_GAME_U6);
	void close();
	bool create(Std::string &filename, uint8 size, uint8 type = NUVIE_GAME_U6);
	uint8 get_game_type() {
		return game_type;
	}

	unsigned char *get_item(uint32 item_number, unsigned char *buf = NULL); // read
	void set_item_data(uint32 item_number, unsigned char *src, uint32 src_len);

	uint32 get_num_items();
	uint32 get_item_size(uint32 item_number);
	uint32 get_item_offset(uint32 item_number);
	const char *get_item_name(uint32 item_number);
	bool is_compressed(uint32 item_number);

	void add_item(uint32 offset32, const char *name = NULL);
	void write_item(uint32 item_number);
	void write_items();

	void load_index(Common::ReadStream *index_f);
	void write_index();
	void write_header();

	void calc_item_offsets();

protected:
	void parse_lib();
	void calculate_item_sizes();
	uint32 calculate_item_uncomp_size(U6LibItem *item);
	uint32 calculate_num_offsets(bool skip4);
};

#if 0
class U6ConverseLib: U6Lib_n {
private:
	uint32 zero_offset_count;
	string *conversefile;
};
#endif
} // End of namespace Nuvie
} // End of namespace Ultima


#endif
