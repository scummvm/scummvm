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
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Process)

DEFINE_CUSTOM_MEMORY_ALLOCATION(Process)

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
}

void Process::waitFor(ProcId pid_) {
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

void Process::save(ODataSource *ods) {
	writeProcessHeader(ods);
	saveData(ods); // virtual
}

void Process::writeProcessHeader(ODataSource *ods) {
	const char *cname = GetClassType()._className; // virtual
	uint16 clen = strlen(cname);

	ods->write2(clen);
	ods->write(cname, clen);
}

void Process::saveData(ODataSource *ods) {
	ods->write2(_pid);
	ods->write4(_flags);
	ods->write2(_itemNum);
	ods->write2(_type);
	ods->write4(_result);
	ods->write4(static_cast<uint32>(_waiting.size()));
	for (unsigned int i = 0; i < _waiting.size(); ++i)
		ods->write2(_waiting[i]);
}

bool Process::loadData(IDataSource *ids, uint32 version) {
	_pid = ids->read2();
	_flags = ids->read4();
	_itemNum = ids->read2();
	_type = ids->read2();
	_result = ids->read4();
	uint32 waitcount = ids->read4();
	_waiting.resize(waitcount);
	for (unsigned int i = 0; i < waitcount; ++i)
		_waiting[i] = ids->read2();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
