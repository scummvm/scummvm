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
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/kernel.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Process)

Process::Process(ObjId it, uint16 ty)
	: _pid(0xFFFF), _flags(0), _itemNum(it), _type(ty), _result(0) {
	Kernel::get_instance()->assignPID(this);
}

void Process::fail() {
	assert(!(_flags & PROC_TERMINATED));

	_flags |= PROC_FAILED;
	terminate();
}

void Process::terminate() {
	assert(!(_flags & PROC_TERMINATED));

	Kernel *kernel = Kernel::get_instance();

	// wake up waiting processes
	for (Std::vector<ProcId>::iterator i = _waiting.begin();
	        i != _waiting.end(); ++i) {
		Process *p = kernel->getProcess(*i);
		if (p)
			p->wakeUp(_result);
	}
	_waiting.clear();

	_flags |= PROC_TERMINATED;
}

void Process::wakeUp(uint32 result_) {
	_result = result_;

	_flags &= ~PROC_SUSPENDED;

	Kernel::get_instance()->setNextProcess(this);

	onWakeUp();
}

void Process::waitFor(ProcId pid_) {
	assert(pid_ != _pid);
	if (pid_) {
		Kernel *kernel = Kernel::get_instance();

		// add this process to waiting list of process pid_
		Process *p = kernel->getProcess(pid_);
		assert(p);
		p->_waiting.push_back(_pid);
	}

	_flags |= PROC_SUSPENDED;
}

void Process::waitFor(Process *proc) {
	assert(this != proc);
	ProcId pid_ = 0;
	if (proc) pid_ = proc->getPid();

	waitFor(pid_);
}

void Process::suspend() {
	_flags |= PROC_SUSPENDED;
}

void Process::dumpInfo() const {
	Common::String info = Common::String::format(
		"Process %d class %s, item %d, type %x, status ",
		getPid(), GetClassType()._className, _itemNum, _type);

	if (_flags & PROC_ACTIVE) info += "A";
	if (_flags & PROC_SUSPENDED) info += "S";
	if (_flags & PROC_TERMINATED) info += "T";
	if (_flags & PROC_TERM_DEFERRED) info += "t";
	if (_flags & PROC_FAILED) info += "F";
	if (_flags & PROC_RUNPAUSED) info += "R";
	if (!_waiting.empty()) {
		info += ", notify: ";
		for (Std::vector<ProcId>::const_iterator i = _waiting.begin(); i != _waiting.end(); ++i) {
			if (i != _waiting.begin()) info += ", ";
			info += *i;
		}
	}

	g_debugger->debugPrintf("%s\n", info.c_str());
}

void Process::saveData(Common::WriteStream *ws) {
	ws->writeUint16LE(_pid);
	ws->writeUint32LE(_flags);
	ws->writeUint16LE(_itemNum);
	ws->writeUint16LE(_type);
	ws->writeUint32LE(_result);
	ws->writeUint32LE(static_cast<uint32>(_waiting.size()));
	for (unsigned int i = 0; i < _waiting.size(); ++i)
		ws->writeUint16LE(_waiting[i]);
}

bool Process::loadData(Common::ReadStream *rs, uint32 version) {
	_pid = rs->readUint16LE();
	_flags = rs->readUint32LE();
	_itemNum = rs->readUint16LE();
	_type = rs->readUint16LE();
	_result = rs->readUint32LE();
	uint32 waitcount = rs->readUint32LE();
	_waiting.resize(waitcount);
	for (unsigned int i = 0; i < waitcount; ++i)
		_waiting[i] = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
