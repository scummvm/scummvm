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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/files/u6_lib_n.h"

namespace Ultima {
namespace Nuvie {

U6Lib_n::U6Lib_n() : num_offsets(0), items(NULL), data(NULL),
	del_data(false), filesize(0), game_type(NUVIE_GAME_U6), lib_size(0) {
}


U6Lib_n::~U6Lib_n(void) {
	close();
}

// load u6lib from `filename'
bool U6Lib_n::open(Std::string &filename, uint8 size, uint8 type) {
	NuvieIOFileRead *file;

	file = new NuvieIOFileRead();

	if (file->open(filename) == false) {
		delete file;
		return false;
	}

	del_data = true;

	return open((NuvieIO *)file, size, type);
}


// load u6lib from opened stream
bool U6Lib_n::open(NuvieIO *new_data, uint8 size, uint8 type) {
	game_type = type;
	data = new_data;

	lib_size = size;
	this->parse_lib();

	return true;
}

void U6Lib_n::close() {
	if (items) {
		for (uint32 i = 0; i < num_offsets; i++)
			delete items[i].name;
		free(items);
	}
	items = NULL;

	if (data != NULL)
		data->close();

	if (del_data)
		delete data;

	data = NULL;
	del_data = false;

	num_offsets = 0;

	return;
}

/* Open a ^new^ file for writing, with lib_size and type.
 */
bool U6Lib_n::create(Std::string &filename, uint8 size, uint8 type) {
	NuvieIOFileWrite *file = new NuvieIOFileWrite();
	if (!file->open(filename)) {
		DEBUG(0, LEVEL_ERROR, "U6Lib: Error creating %s\n", filename.c_str());
		delete file;
		return (false);
	}
	game_type = type;
	lib_size = size;
	data = (NuvieIO *)file;
	return (true);
}


uint32 U6Lib_n::get_num_items(void) {
	return num_offsets;
}


/* Returns the location of `item_number' in the library file.
 */
uint32 U6Lib_n::get_item_offset(uint32 item_number) {
	if (item_number >= num_offsets)
		return (0);
	return (items[item_number].offset);
}

uint32 U6Lib_n::get_item_size(uint32 item_number) {
	if (item_number >= num_offsets)
		return (0);

	return (items[item_number].uncomp_size);
}


// read and return item data
unsigned char *U6Lib_n::get_item(uint32 item_number, unsigned char *ret_buf) {
	U6LibItem *item;
	unsigned char *buf, *lzw_buf;

	if (item_number >= num_offsets)
		return NULL;

	item = &items[item_number];

	if (item->size == 0 || item->offset == 0)
		return NULL;

	if (ret_buf == NULL)
		buf = (unsigned char *)malloc(item->uncomp_size);
	else
		buf = ret_buf;

	data->seek(item->offset);

	if (is_compressed(item_number)) {
		U6Lzw lzw;
		lzw_buf = (unsigned char *)malloc(item->size);
		data->readToBuf(lzw_buf, item->size);
		lzw.decompress_buffer(lzw_buf, item->size, buf, item->uncomp_size);
	} else {
		data->readToBuf(buf, item->size);
	}
	return buf;
}

bool U6Lib_n::is_compressed(uint32 item_number) {
	uint32 i;

	switch (items[item_number].flag) {
	case 0x1 :
	case 0x20 :
		return true;
	case 0xff :
		for (i = item_number; i < num_offsets; i++) {
			if (items[i].flag != 0xff)
				break;
		}
		if (i < num_offsets)
			return is_compressed(i);
		break;
	}

	return false;
}

void U6Lib_n::parse_lib() {
	uint32 i;
	bool skip4 = false;

	if (lib_size != 2 && lib_size != 4)
		return;

	data->seekStart();

	if (game_type != NUVIE_GAME_U6) { //U6 doesn't have a 4 byte filesize header.
		skip4 = true;
		filesize = data->read4();
	} else
		filesize = data->get_size();

	num_offsets = calculate_num_offsets(skip4);

	items = (U6LibItem *)malloc(sizeof(U6LibItem) * (num_offsets + 1));
	memset(items, 0, sizeof(U6LibItem) * (num_offsets + 1));

	data->seekStart();
	if (skip4)
		data->seek(0x4);
	for (i = 0; i < num_offsets && !data->is_end(); i++) {
		if (lib_size == 2)
			items[i].offset = data->read2();
		else {
			items[i].offset = data->read4();
			// U6 converse files dont have flag?
			items[i].flag = (items[i].offset & 0xff000000) >> 24; //extract flag byte
			items[i].offset &= 0xffffff;
		}
	}

	items[num_offsets].offset = filesize; //this is used to calculate the size of the last item in the lib.

	calculate_item_sizes();

	return;
}


// for reading, calculate item sizes based on offsets
void U6Lib_n::calculate_item_sizes() {
	uint32 i, next_offset = 0;

	for (i = 0; i < num_offsets; i++) {
		items[i].size = 0;
		// get next non-zero offset, including the filesize at items[num_offsets]
		for (uint32 o = (i + 1); o <= num_offsets; o++)
			if (items[o].offset) {
				next_offset = items[o].offset;
				break;
			}

		if (items[i].offset && (next_offset > items[i].offset))
			items[i].size = next_offset - items[i].offset;

		items[i].uncomp_size = calculate_item_uncomp_size(&items[i]);
	}

	return;
}

// for reading, calculate uncompressed item size based on item flag
uint32 U6Lib_n::calculate_item_uncomp_size(U6LibItem *item) {
	uint32 uncomp_size = 0;

	switch (item->flag) {
	case 0x01 : //compressed
	case 0x20 : //MD fonts.lzc, MDD_MUS.LZC use this tag among others
		data->seek(item->offset);
		uncomp_size = data->read4();
		break;

	//FIX check this. uncompressed 4 byte item size header
	case 0xc1 :
		uncomp_size = item->size; // - 4;
		break;

	// uncompressed
	case 0x0  :
	case 0x2  :
	case 0xe0 :
	default   :
		uncomp_size = item->size;
		break;
	}

	return uncomp_size;
}

// we need to handle NULL offsets at the start of the offset table in the converse.a file
uint32 U6Lib_n::calculate_num_offsets(bool skip4) { //skip4 bytes of header.
	uint32 i;
	uint32 offset = 0;

	if (skip4)
		data->seek(0x4);


// We assume the first data in the file is directly behind the offset table,
// so we continue scanning until we hit a data block.
	uint32 max_count = 0xffffffff;
	for (i = 0; !data->is_end(); i++) {
		if (i == max_count)
			return i;

		if (lib_size == 2)
			offset = data->read2();
		else {
			offset = data->read4();
			offset &= 0xffffff; // clear flag byte.
		}
		if (offset != 0) {
			if (skip4)
				offset -= 4;

			if (offset / lib_size < max_count)
				max_count = offset / lib_size;
		}
	}

	return 0;
}


/* For writing multiple files to a lib, read in source filenames and offsets
 * from an opened index file. Offsets may be ignored when writing.
 */
void U6Lib_n::load_index(Common::ReadStream *index_f) {
	char input[256] = "", // input line
	                  offset_str[9] = "", // listed offset
	                                  name[256] = ""; // source file name
	int in_len = 0, oc = 0; // length of input line, character in copy string
	int c = 0, entry_count = 0; // character in input line, number of entries

	if (!index_f)
		return;
	while (strgets(input, 256, index_f)) {
		in_len = strlen(input);
		// skip spaces, read offset, break on #
		for (c = 0; c < in_len && Common::isSpace(input[c]) && input[c] != '#'; c++);
		for (oc = 0; c < in_len && !Common::isSpace(input[c]) && input[c] != '#'; c++)
			offset_str[oc++] = input[c];
		offset_str[oc] = '\0';
		// skip spaces, read name, break on # or \n or \r
		for (; c < in_len && Common::isSpace(input[c]) && input[c] != '#'; c++);
		for (oc = 0; c < in_len && input[c] != '\n' && input[c] != '\r' && input[c] != '#'; c++)
			name[oc++] = input[c];
		name[oc] = '\0';
		if (strlen(offset_str)) { // if line is not empty (!= zero entry)
			uint32 offset32 = strtol(offset_str, NULL, 16);
			add_item(offset32, name);
			++entry_count;
		}
		offset_str[0] = '\0';
		oc = 0;
	}
}


/* Append an offset and a name to the library. The other fields are initialized.
 */
void U6Lib_n::add_item(uint32 offset32, const char *name) {
	if (!num_offsets)
		items = (U6LibItem *)malloc(sizeof(U6LibItem));
	else
		items = (U6LibItem *)nuvie_realloc(items, sizeof(U6LibItem) * (num_offsets + 1));
	U6LibItem *item = &items[num_offsets];
	item->offset = offset32;
	item->name = new string(name);
	item->size = 0;
	item->uncomp_size = 0;
	item->flag = 0; // uncompressed
	item->data = NULL;
	++num_offsets;
}


/* Returns the name of (filename associated with) `item_number'.
 */
const char *U6Lib_n::get_item_name(uint32 item_number) {
	if (item_number >= num_offsets)
		return (NULL);
	return (items[item_number].name ? items[item_number].name->c_str() : NULL);
}


/* Set data for an item, in preparation of writing or to cache the library.
 * Size & uncompressed size is set to source length.
 */
void U6Lib_n::set_item_data(uint32 item_number, unsigned char *src, uint32 src_len) {
	unsigned char *dcopy = 0;
	if (item_number >= num_offsets)
		return;
// FIXME: need a way to set an item as compressed or uncompressed so we know
// which size to set
	items[item_number].size = src_len;
	items[item_number].uncomp_size = src_len;
	if (src_len) {
		dcopy = (unsigned char *)malloc(src_len);
		memcpy(dcopy, src, src_len);
		items[item_number].data = dcopy;
	} else
		items[item_number].data = 0;
}


/* For writing, (re)calculate item offsets from item sizes.
 */
void U6Lib_n::calc_item_offsets() {
	if (num_offsets == 0)
		return;
	if (items[0].size) // first offset is past library index
		items[0].offset = (num_offsets * lib_size);
	else
		items[0].offset = 0; // 0 = no data, no affect on other items
//    DEBUG(0,LEVEL_DEBUGGING,"calc_item_offsets: sizes[0] == %d\n", sizes[0]);
//    DEBUG(0,LEVEL_DEBUGGING,"calc_item_offsets: offsets[0] == %d\n", offsets[0]);
	for (uint32 i = 1; i < num_offsets; i++) {
		if (items[i].size) {
			// find previous item with non-zero offset
			uint32 prev_i = 0;
			for (uint32 i_sub = 1; i_sub <= i; i_sub++) {
				prev_i = i - i_sub;
				if (items[prev_i].offset != 0)
					break;
			}
			items[i].offset = (items[prev_i].offset + items[prev_i].size);
			if (items[i].offset == 0) // last item had no data; skip index here
				items[i].offset = (num_offsets * lib_size);
		} else
			items[i].offset = 0; // 0 = no data, no affect on other items
//        DEBUG(0,LEVEL_DEBUGGING,"calc_item_offsets: sizes[%d] == %d\n", i, sizes[i]);
//        DEBUG(0,LEVEL_DEBUGGING,"calc_item_offsets: offsets[%d] == %d\n", i, offsets[i]);
	}
}

void U6Lib_n::write_header() {
	data->seekStart();
	if (game_type == NUVIE_GAME_U6)
		return;

	uint32 totalSize = 4 + num_offsets * lib_size;

	for (uint i = 0; i < num_offsets; i++) {
		totalSize += items[i].size;
	}

	data->write4(totalSize);
}

/* Write the library index. (the 2 or 4 byte offsets before the data)
 */
void U6Lib_n::write_index() {
	data->seekStart();
	if (game_type != NUVIE_GAME_U6) {
		data->seek(4);
	}

	for (uint32 o = 0; o < num_offsets; o++) {
		uint32 offset = items[o].offset;
		if (game_type != NUVIE_GAME_U6 && offset != 0) {
			offset += 4;
		}
		if (lib_size == 2)
			data->write2((uint16)offset);
		else if (lib_size == 4)
			data->write4(offset);
	}
}


/* Write all item data to the library file at their respective offsets.
 */
void U6Lib_n::write_items() {
	for (uint32 i = 0; i < num_offsets; i++)
		write_item(i);
}


/* Write item data to the library file at the indicated offset, unless the
 * offset is 0 (then the data is considered empty).
 */
void U6Lib_n::write_item(uint32 item_number) {
	if (item_number >= num_offsets
	        || items[item_number].offset == 0 || items[item_number].size == 0)
		return;
	if (game_type == NUVIE_GAME_U6)
		data->seek(items[item_number].offset);
	else
		data->seek(items[item_number].offset + 4);
	((NuvieIOFileWrite *)data)->writeBuf(items[item_number].data, items[item_number].size);
}

} // End of namespace Nuvie
} // End of namespace Ultima
