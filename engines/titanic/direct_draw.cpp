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

#include "titanic/direct_draw.h"

namespace Titanic {

DirectDraw::DirectDraw(TitanicEngine *vm) : Manager(vm) {
	_field8 = 0;
	_fieldC = 0;
	_width = 0;
	_height = 0;
	_bpp = 0;
	_field1C = 0;
	_field24 = 0;
}

/*------------------------------------------------------------------------*/

DirectDrawManager::DirectDrawManager(TitanicEngine *vm) : 
		Manager(vm), _directDraw(vm) {
	_mainSurface = nullptr;
	_backSurfaces[0] = _backSurfaces[1] = nullptr;
}

} // End of namespace Titanic
