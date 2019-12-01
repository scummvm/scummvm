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

#include "ultima8/misc/pent_include.h"

#include "ultima8/usecode/uc_process.h"
#include "ultima8/usecode/uc_machine.h"
#include "Usecode.h"
#include "ultima8/games/game_data.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(UCProcess, Process);

UCProcess::UCProcess() : Process() { // !! fixme
	usecode = GameData::get_instance()->getMainUsecode();
}

UCProcess::UCProcess(uint16 classid_, uint16 offset_, uint32 this_ptr,
                     int thissize, const uint8 *args, int argsize)
	: Process() {
	classid = 0xFFFF;
	ip = 0xFFFF;
	bp = 0x0000;
	usecode = GameData::get_instance()->getMainUsecode();
	temp32 = 0;

	load(classid_, offset_, this_ptr, thissize, args, argsize);
}

UCProcess::~UCProcess() {

}

void UCProcess::load(uint16 classid_, uint16 offset_, uint32 this_ptr,
                     int thissize, const uint8 *args, int argsize) {
	if (usecode->get_class_size(classid_) == 0)
		perr << "Class is empty..." << std::endl;

	classid = 0xFFFF;
	ip = 0xFFFF;
	bp = 0x0000;
	uint16 thissp = 0;

	// first, push the derefenced this pointer
	if (this_ptr != 0 && thissize > 0) {
		stack.addSP(-thissize);
		UCMachine::get_instance()->
		dereferencePointer(this_ptr, stack.access(), thissize);
		thissp = stack.getSP();
	}

	// next, push the arguments
	stack.push(args, argsize);

	// then, push the new this pointer
	if (thissp != 0)
		stack.push4(UCMachine::stackToPtr(pid, thissp));

	// finally, call the specified function
	call(classid_, offset_);
}

void UCProcess::run() {
	if (flags & PROC_SUSPENDED)
		return;

	// pass to UCMachine for execution
	UCMachine::get_instance()->execProcess(this);
}

void UCProcess::call(uint16 classid_, uint16 offset_) {
	stack.push2(classid); // BP+04 prev class
	stack.push2(ip);      // BP+02 prev IP
	stack.push2(bp);      // BP+00 prev BP

	classid = classid_;
	ip = offset_;
	bp = static_cast<uint16>(stack.getSP()); // TRUNCATES!
}

bool UCProcess::ret() {
	stack.setSP(bp);

	bp = stack.pop2();
	ip = stack.pop2();
	classid = stack.pop2();

	if (ip == 0xFFFF && classid == 0xFFFF)
		return true;
	else
		return false;
}

void UCProcess::freeOnTerminate(uint16 index, int type) {
	assert(type >= 1 && type <= 3);

	std::pair<uint16, int> p;
	p.first = index;
	p.second = type;

	freeonterminate.push_back(p);
}

void UCProcess::terminate() {
	std::list<std::pair<uint16, int> >::iterator i;

	for (i = freeonterminate.begin(); i != freeonterminate.end(); ++i) {
		uint16 index = (*i).first;
		int type = (*i).second;

		switch (type) {
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

	freeonterminate.clear();

	Process::terminate();
}

void UCProcess::dumpInfo() {
	Process::dumpInfo();

	if (classid == 0xFFFF) {
		pout.printf("IP undefined\n");
	} else {
		const char *classname = GameData::get_instance()->getMainUsecode()->
		                        get_class_name(classid);
		pout.printf("classname: %s, IP: %04X:%04X\n", classname, classid, ip);
	}
}

void UCProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(bp);
	ods->write2(classid);
	ods->write2(ip);
	ods->write4(temp32);
	ods->write4(static_cast<uint32>(freeonterminate.size()));
	std::list<std::pair<uint16, int> >::iterator iter;
	for (iter = freeonterminate.begin(); iter != freeonterminate.end(); ++iter) {
		ods->write2(iter->first);
		ods->write4(static_cast<uint32>(iter->_value));
	}
	stack.save(ods);
}

bool UCProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	bp = ids->read2();
	classid = ids->read2();
	ip = ids->read2();
	temp32 = ids->read4();
	uint32 freecount = ids->read4();
	for (unsigned int i = 0; i < freecount; ++i) {
		std::pair<uint16, int> p;
		p.first = ids->read2();
		p.second = static_cast<int>(ids->read4());
		freeonterminate.push_back(p);
	}
	stack.load(ids, version);

	return true;
}

} // End of namespace Ultima8
