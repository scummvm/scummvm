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
	: pid(0xFFFF), flags(0), item_num(it), type(ty), result(0) {
	Kernel::get_instance()->assignPID(this);
}

void Process::fail() {
	assert(!(flags & PROC_TERMINATED));

	flags |= PROC_FAILED;
	terminate();
}

void Process::terminate() {
	assert(!(flags & PROC_TERMINATED));

	Kernel *kernel = Kernel::get_instance();

	// wake up waiting processes
	for (std::vector<ProcId>::iterator i = waiting.begin();
	        i != waiting.end(); ++i) {
		Process *p = kernel->getProcess(*i);
		if (p)
			p->wakeUp(result);
	}
	waiting.clear();

	flags |= PROC_TERMINATED;
}

void Process::wakeUp(uint32 result_) {
	result = result_;

	flags &= ~PROC_SUSPENDED;

	Kernel::get_instance()->setNextProcess(this);
}

void Process::waitFor(ProcId pid_) {
	if (pid_) {
		Kernel *kernel = Kernel::get_instance();

		// add this process to waiting list of process pid_
		Process *p = kernel->getProcess(pid_);
		assert(p);
		p->waiting.push_back(pid);
	}

	flags |= PROC_SUSPENDED;
}

void Process::waitFor(Process *proc) {
	ProcId pid_ = 0;
	if (proc) pid_ = proc->getPid();

	waitFor(pid_);
}

void Process::suspend() {
	flags |= PROC_SUSPENDED;
}

void Process::dumpInfo() {
	Common::String info = Common::String::format(
		"Process %d class %s, item %d, type %x, status ",
		getPid(), GetClassType().class_name, item_num, type);

	pout << info.c_str();
	if (flags & PROC_ACTIVE) pout << "A";
	if (flags & PROC_SUSPENDED) pout << "S";
	if (flags & PROC_TERMINATED) pout << "T";
	if (flags & PROC_TERM_DEFERRED) pout << "t";
	if (flags & PROC_FAILED) pout << "F";
	if (flags & PROC_RUNPAUSED) pout << "R";
	if (!waiting.empty()) {
		pout << ", notify: ";
		for (std::vector<ProcId>::iterator i = waiting.begin();
		        i != waiting.end(); ++i) {
			if (i != waiting.begin()) pout << ", ";
			pout << *i;
		}
	}
	pout << std::endl;
}

void Process::save(ODataSource *ods) {
	writeProcessHeader(ods);
	saveData(ods); // virtual
}

void Process::writeProcessHeader(ODataSource *ods) {
	const char *cname = GetClassType().class_name; // virtual
	uint16 clen = strlen(cname);

	ods->write2(clen);
	ods->write(cname, clen);
}

void Process::saveData(ODataSource *ods) {
	ods->write2(pid);
	ods->write4(flags);
	ods->write2(item_num);
	ods->write2(type);
	ods->write4(result);
	ods->write4(static_cast<uint32>(waiting.size()));
	for (unsigned int i = 0; i < waiting.size(); ++i)
		ods->write2(waiting[i]);
}

bool Process::loadData(IDataSource *ids, uint32 version) {
	pid = ids->read2();
	flags = ids->read4();
	item_num = ids->read2();
	type = ids->read2();
	result = ids->read4();
	uint32 waitcount = ids->read4();
	waiting.resize(waitcount);
	for (unsigned int i = 0; i < waitcount; ++i)
		waiting[i] = ids->read2();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
