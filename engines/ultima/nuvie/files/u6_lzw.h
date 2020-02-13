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

#ifndef NUVIE_FILES_U6_LZW_H
#define NUVIE_FILES_U6_LZW_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

class NuvieIOFileRead;

// LZW Stack

#define STACK_SIZE 10000

class U6LzwStack {
protected:
	unsigned char stack[STACK_SIZE];
	int contains;

public:
	U6LzwStack();

	void reset(void);
	bool is_empty(void);
	bool is_full(void);
	void push(unsigned char element);
	unsigned char pop(void);
	unsigned char gettop(void);
};

// LZW dictionary

#define DICT_SIZE 10000

typedef struct {
	unsigned char root;
	int codeword;
	int contains;
} dict_entry;

class U6LzwDict {
	dict_entry dict[DICT_SIZE];
	int contains;

public:

	U6LzwDict();

	void reset(void);
	void add(unsigned char root, int codeword);
	unsigned char get_root(int codeword);
	int get_codeword(int codeword);
};

class U6Lzw {
	U6LzwStack *stack;
	U6LzwDict *dict;
	const char *errstr; // error string
public:

	U6Lzw(void);
	~U6Lzw(void);

	unsigned char *decompress_buffer(unsigned char *source, uint32 source_length, uint32 &destination_length);
	bool decompress_buffer(unsigned char *source, uint32 source_length, unsigned char *destination, uint32 destination_length);
	unsigned char *decompress_file(Std::string filename, uint32 &destination_length);
	unsigned char *compress_buffer(unsigned char *src, uint32 src_len,
	                               uint32 &dest_len);
	const char *strerror() {
		return (const char *)errstr;    // get error string
	}
protected:

	bool is_valid_lzw_file(NuvieIOFileRead *input_file);
	bool is_valid_lzw_buffer(unsigned char *buf, uint32 length);

	long get_uncompressed_file_size(NuvieIOFileRead *input_file);
	long get_uncompressed_buffer_size(unsigned char *buf, uint32 length);

	int get_next_codeword(long *bits_read, unsigned char *source,
	                      int codeword_size);
	void output_root(unsigned char root, unsigned char *destination,
	                 long *position);
	void get_string(int codeword);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
