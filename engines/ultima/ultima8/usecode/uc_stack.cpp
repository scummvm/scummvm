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
	ods->write4(_size);
	ods->write4(getSP());

	ods->write(_bufPtr, stacksize());
}

bool UCStack::load(IDataSource *ids, uint32 version) {
	_size = ids->read4();
#ifdef USE_DYNAMIC_UCSTACK
	if (_buf) delete[] _buf;
	_buf = new uint8[_size];
#else
	if (_size > sizeof(_bufArray)) {
		perr << "Error: UCStack _size mismatch (buf_array too small)" << Std::endl;
		return false;
	}
	_buf = _bufArray;
#endif
	uint32 sp = ids->read4();
	_bufPtr = _buf + sp;

	ids->read(_bufPtr, _size - sp);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
