/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/stream.h"

namespace Common {


void MemoryReadStream::seek(uint32 offs, int whence) {
	// Pre-Condition
	assert(_pos <= _bufSize);
	switch (whence) {
	case SEEK_END:
		// SEEK_END works just like SEEK_SET, only 'reversed',
		// i.e. from the end.
		offs = _bufSize - offs;
		// Fall through
	case SEEK_SET:
		_ptr = _ptrOrig + offs;
		_pos = offs;
		break;

	case SEEK_CUR:
		_ptr += offs;
		_pos += offs;
		break;
	}
	// Post-Condition
	assert(_pos <= _bufSize);
}


}	// End of namespace Common
