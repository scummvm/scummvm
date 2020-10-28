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
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/games/game_data.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(UCProcess)

UCProcess::UCProcess() : Process(), _classId(0xFFFF), _ip(0xFFFF),
		_bp(0x0000), _temp32(0) { // !! fixme
	_usecode = GameData::get_instance()->getMainUsecode();
}

UCProcess::UCProcess(uint16 classid, uint16 offset, uint32 this_ptr,
                     int thissize, const uint8 *args, int argsize)
	: Process(), _classId(0xFFFF), _ip(0xFFFF), _bp(0x0000), _temp32(0) {
	_usecode = GameData::get_instance()->getMainUsecode();

	load(classid, offset, this_ptr, thissize, args, argsize);
}

UCProcess::~UCProcess() {
}

void UCProcess::load(uint16 classid, uint16 offset, uint32 this_ptr,
                     int thissize, const uint8 *args, int argsize) {
	if (_usecode->get_class_size(classid) == 0)
		perr << "Class is empty..." << Std::endl;

	_classId = 0xFFFF;
	_ip = 0xFFFF;
	_bp = 0x0000;
	uint16 thissp = 0;

	// first, push the derefenced this pointer
	if (this_ptr != 0 && thissize > 0) {
		_stack.addSP(-thissize);
		UCMachine::get_instance()->
		dereferencePointer(this_ptr, _stack.access(), thissize);
		thissp = _stack.getSP();
	}

	// next, push the arguments
	_stack.push(args, argsize);

	// then, push the new this pointer
	if (thissp != 0)
		_stack.push4(UCMachine::stackToPtr(_pid, thissp));

	// finally, call the specified function
	call(classid, offset);
}

void UCProcess::run() {
	if (_flags & PROC_SUSPENDED)
		return;

	// pass to UCMachine for execution
	UCMachine::get_instance()->execProcess(this);
}

void UCProcess::call(uint16 classid, uint16 offset) {
	_stack.push2(_classId); // BP+04 prev class
	_stack.push2(_ip);      // BP+02 prev IP
	_stack.push2(_bp);      // BP+00 prev BP

	_classId = classid;
	_ip = offset;
	_bp = static_cast<uint16>(_stack.getSP()); // TRUNCATES!
}

bool UCProcess::ret() {
	_stack.setSP(_bp);

	_bp = _stack.pop2();
	_ip = _stack.pop2();
	_classId = _stack.pop2();

	if (_ip == 0xFFFF && _classId == 0xFFFF)
		return true;
	else
		return false;
}

void UCProcess::freeOnTerminate(uint16 index, int type) {
	assert(type >= 1 && type <= 3);

	Std::pair<uint16, int> p;
	p.first = index;
	p.second = type;

	_freeOnTerminate.push_back(p);
}

void UCProcess::terminate() {
	Std::list<Std::pair<uint16, int> >::iterator i;

	for (i = _freeOnTerminate.begin(); i != _freeOnTerminate.end(); ++i) {
		uint16 index = (*i).first;
		int typeNum = (*i).second;

		switch (typeNum) {
		case 1: // string
			UCMachine::get_instance()->freeString(index);
			break;
		case 2: // stringlist
			UCMachine::get_instance()->freeStringList(index);
			break;
		case 3: // list
			UCMachine::get_instance()->freeList(index);
			break;
		}
	}

	_freeOnTerminate.clear();

	Process::terminate();
}

void UCProcess::dumpInfo() const {
	Process::dumpInfo();

	if (_classId == 0xFFFF) {
		pout.Print("IP undefined\n");
	} else {
		const char *classname = GameData::get_instance()->getMainUsecode()->
		                        get_class_name(_classId);
		pout.Print("classname: %s, IP: %04X:%04X\n", classname, _classId, _ip);
	}
}

void UCProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_bp);
	ws->writeUint16LE(_classId);
	ws->writeUint16LE(_ip);
	ws->writeUint32LE(_temp32);
	ws->writeUint32LE(static_cast<uint32>(_freeOnTerminate.size()));
	Std::list<Std::pair<uint16, int> >::iterator iter;
	for (iter = _freeOnTerminate.begin(); iter != _freeOnTerminate.end(); ++iter) {
		ws->writeUint16LE(iter->first);
		ws->writeUint32LE(static_cast<uint32>(iter->second));
	}
	_stack.save(ws);
}

bool UCProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_bp = rs->readUint16LE();
	_classId = rs->readUint16LE();
	_ip = rs->readUint16LE();
	_temp32 = rs->readUint32LE();
	uint32 freecount = rs->readUint32LE();
	for (unsigned int i = 0; i < freecount; ++i) {
		Std::pair<uint16, int> p;
		p.first = rs->readUint16LE();
		p.second = static_cast<int>(rs->readUint32LE());
		_freeOnTerminate.push_back(p);
	}
	_stack.load(rs, version);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
