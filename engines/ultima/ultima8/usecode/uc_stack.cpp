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

#include "ultima/ultima8/usecode/uc_stack.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

void UCStack::save(ODataSource *ods) {
	ods->write4(size);
	ods->write4(getSP());

	ods->write(buf_ptr, stacksize());
}

bool UCStack::load(IDataSource *ids, uint32 version) {
	size = ids->read4();
#ifdef USE_DYNAMIC_UCSTACK
	if (buf) delete[] buf;
	buf = new uint8[size];
#else
	if (size > sizeof(buf_array)) {
		perr << "Error: UCStack size mismatch (buf_array too small)" << std::endl;
		return false;
	}
	buf = buf_array;
#endif
	uint32 sp = ids->read4();
	buf_ptr = buf + sp;

	ids->read(buf_ptr, size - sp);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
