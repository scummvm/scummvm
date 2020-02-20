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
#include "ultima/ultima8/kernel/memory_manager.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Object)

DEFINE_CUSTOM_MEMORY_ALLOCATION(Object)

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


void Object::save(ODataSource *ods) {
	writeObjectHeader(ods);
	saveData(ods); // virtual
}

void Object::writeObjectHeader(ODataSource *ods) const {
	const char *cname = GetClassType()._className; // note: virtual
	uint16 clen = strlen(cname);

	ods->write2(clen);
	ods->write(cname, clen);
}

void Object::saveData(ODataSource *ods) {
	// note: Object is unversioned. If we ever want to version it,
	// increase the global savegame version

	ods->write2(_objId);
}

bool Object::loadData(IDataSource *ids, uint32 version) {
	_objId = ids->read2();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
