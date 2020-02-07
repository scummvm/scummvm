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
 */
#include "dragons/dragonvar.h"
#include "dragons/bigfile.h"

namespace Dragons {

DragonVAR::DragonVAR(BigfileArchive *bigfileArchive): _bigfileArchive(bigfileArchive), _data(0) {
	reset();
}

DragonVAR::~DragonVAR() {
	delete _data;
}

uint16 DragonVAR::getVar(uint16 offset) {
	assert(_data);
	assert(offset < 15);
	return READ_LE_UINT16(_data + offset * 2);
}

void DragonVAR::setVar(uint16 offset, uint16 value) {
	assert(_data);
	assert(offset < 15);
	WRITE_LE_INT16(_data + offset * 2, value);
}

void DragonVAR::reset() {
	delete _data;
	uint32 size;
	_data = _bigfileArchive->load("dragon.var", size);
	assert(size == 30);
}

} // End of namespace Dragons
