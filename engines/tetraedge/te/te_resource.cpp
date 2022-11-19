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

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_resource_manager.h"

namespace Tetraedge {

uint32 TeResource::_idCounter = 1;

TeResource::TeResource() {
	generateAccessName();
}

TeResource::~TeResource() {
	g_engine->getResourceManager()->removeResource(this);
}

void TeResource::generateAccessName() {
	static char hexaChars[] = "0123456789ABCDEF";

	/* Note: the original game this generates a 64-bit ID, but are we
	   really ever going to have 4 billion objects? */
	char name[9];
	name[0] = hexaChars[_idCounter >> 28 & 0xf];
	name[1] = hexaChars[_idCounter >> 24 & 0xf];
	name[2] = hexaChars[_idCounter >> 20 & 0xf];
	name[3] = hexaChars[_idCounter >> 16 & 0xf];
	name[4] = hexaChars[_idCounter >> 12 & 0xf];
	name[5] = hexaChars[_idCounter >> 8  & 0xf];
	name[6] = hexaChars[_idCounter >> 4  & 0xf];
	name[7] = hexaChars[_idCounter		 & 0xf];
	name[8] = '\0';

	_idCounter++;

	_accessName = name;
}

} // end namespace Tetraedge
