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

namespace Ultima {
namespace Ultima8 {

void UCStack::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_size);
	ws->writeUint32LE(getSP());

	ws->write(_bufPtr, stacksize());
}

bool UCStack::load(Common::ReadStream *rs, uint32 version) {
	_size = rs->readUint32LE();
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
	uint32 sp = rs->readUint32LE();
	_bufPtr = _buf + sp;

	rs->read(_bufPtr, _size - sp);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
