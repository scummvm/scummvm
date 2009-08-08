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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"

namespace Gob {

Init_v6::Init_v6(GobEngine *vm) : Init_v3(vm) {
}

Init_v6::~Init_v6() {
}

void Init_v6::initGame() {
	_vm->_global->_noCd = false;

	if (Common::File::exists("cd1.itk") && Common::File::exists("cd2.itk") &&
	    Common::File::exists("cd3.itk") && Common::File::exists("cd4.itk")) {
		_vm->_global->_noCd = true;
	}

	Init::initGame();
}

} // End of namespace Gob
