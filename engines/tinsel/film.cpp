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

#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

MULTI_INIT *FREEL::GetMultiInit() {
	return (MULTI_INIT *)_vm->_handle->LockMem(FROM_32(mobj));
}

const MULTI_INIT *FREEL::GetMultiInit() const {
	return (const MULTI_INIT *)_vm->_handle->LockMem(FROM_32(mobj));
}

} // End of namespace Tinsel
