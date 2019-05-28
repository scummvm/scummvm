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

#include "illusions/illusions.h"
#include "illusions/specialcode.h"
#include "illusions/bbdou/bbdou_specialcode.h"

namespace Illusions {

// SpecialCodeLoader

void SpecialCodeLoader::load(Resource *resource) {
	debug("SpecialCodeLoader::load() Loading special code %08X...", resource->_resId);
	_vm->loadSpecialCode(resource->_resId);
}

void SpecialCodeLoader::unload(Resource *resource) {
	debug("SpecialCodeLoader::unload() Unloading special code %08X...", resource->_resId);
	_vm->unloadSpecialCode(resource->_resId);
}

void SpecialCodeLoader::buildFilename(Resource *resource) {
	resource->_filename = Common::String::format("%08X.dll", resource->_resId);
}

bool SpecialCodeLoader::isFlag(int flag) {
	return false;
}

} // End of namespace Illusions
