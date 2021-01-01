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
#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/usecode/uc_machine.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Object)

Object::~Object() {
	if (_objId != 0xFFFF)
		ObjectManager::get_instance()->clearObjId(_objId);
}

ObjId Object::assignObjId() {
	if (_objId == 0xFFFF)
		_objId = ObjectManager::get_instance()->assignObjId(this);
	return _objId;
}

void Object::clearObjId() {
	// On clearObjId we kill all processes that belonged to us
	Kernel::get_instance()->killProcesses(_objId, 6, true);

	if (_objId != 0xFFFF)
		ObjectManager::get_instance()->clearObjId(_objId);
	_objId = 0xFFFF;
}

void Object::dumpInfo() const {
	g_debugger->debugPrintf("Object %d (class %s)\n", getObjId(), GetClassType()._className);
}

ProcId Object::callUsecode(uint16 classid, uint16 offset,
                           const uint8 *args, int argsize) {
	uint32 objptr = UCMachine::objectToPtr(getObjId());
	UCProcess *p = new UCProcess(classid, offset, objptr, 2, args, argsize);
	return Kernel::get_instance()->addProcess(p);
}

void Object::saveData(Common::WriteStream *ws) {
	// note: Object is unversioned. If we ever want to version it,
	// increase the global savegame version

	ws->writeUint16LE(_objId);
}

bool Object::loadData(Common::ReadStream *rs, uint32 version) {
	// If we are loading into an object that already got an ID defined, then
	// there is a problem - default constructors should not allocate object
	// IDs, otherwise we can end up with the wrong IDs during load, because
	// we blindly reload the old object IDs and then assign the kernel pointer
	// table using those.
	assert(_objId == 0xFFFF);

	_objId = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
