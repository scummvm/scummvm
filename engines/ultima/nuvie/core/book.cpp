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

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/book.h"

namespace Ultima {
namespace Nuvie {

Book::Book(Configuration *cfg) {
	config = cfg;
	books = new U6Lib_n;
}

Book::~Book() {
	delete books;
}

bool Book::init() {
	Std::string filename;

	config_get_path(config, "book.dat", filename);

	if (books->open(filename, 2) == false)
		return false;

	return true;
}

char *Book::get_book_data(uint16 num) {
	if (num >= books->get_num_items())
		return NULL;

	return reinterpret_cast<char *>(books->get_item(num));
}

} // End of namespace Nuvie
} // End of namespace Ultima
