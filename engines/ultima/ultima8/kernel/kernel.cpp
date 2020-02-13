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
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

typedef Std::list<Process *>::iterator ProcessIterator;

Kernel *Kernel::kernel = 0;

Kernel::Kernel() : loading(false) {
	con->Print(MM_INFO, "Creating Kernel...\n");

	kernel = this;
	pIDs = new idMan(1, 32766, 128);
	current_process = processes.end();
	framenum = 0;
	paused = 0;
	runningprocess = 0;
	framebyframe = false;
}

Kernel::~Kernel() {
	reset();
	con->Print(MM_INFO, "Destroying Kernel...\n");

	kernel = 0;

	delete pIDs;
}

void Kernel::reset() {
	con->Print(MM_INFO, "Resetting Kernel...\n");

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		delete(*it);
	}
	processes.clear();
	current_process = processes.begin();

	pIDs->clearAll();

	paused = 0;
	runningprocess = 0;

	// if we're in frame-by-frame mode, reset to a paused state
	if (framebyframe) paused = 1;
}

ProcId Kernel::assignPID(Process *proc) {
	// to prevent new processes from getting a PID while loading
	if (loading) return 0xFFFF;

	// Get a pID
	proc->pid = pIDs->getNewID();

	return proc->pid;
}

ProcId Kernel::addProcess(Process *proc) {
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->pid != 0 && proc->pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
	     << ", pid = " << proc->pid << Std::endl;
#endif

//	processes.push_back(proc);
//	proc->active = true;
	setNextProcess(proc);
	return proc->pid;
}

ProcId Kernel::addProcessExec(Process *proc) {
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->pid != 0 && proc->pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
	     << ", pid = " << proc->pid << Std::endl;
#endif

	processes.push_back(proc);
	proc->flags |= Process::PROC_ACTIVE;

	Process *oldrunning = runningprocess;
	runningprocess = proc;
	proc->run();
	runningprocess = oldrunning;

	return proc->pid;
}

void Kernel::removeProcess(Process *proc) {
	//! the way to remove processes has to be thought over sometime
	//! we probably want to flag them as terminated before actually
	//! removing/deleting it or something
	//! also have to look out for deleting processes while iterating
	//! over the list. (Hence the special 'erase' in runProcs below, which
	//! is very Std::list-specific, incidentally)

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc) {
			proc->flags &= ~Process::PROC_ACTIVE;

			perr << "[Kernel] Removing process " << proc << Std::endl;

			processes.erase(it);

			// Clear pid
			pIDs->clearID(proc->pid);

			return;
		}
	}
}


void Kernel::runProcesses() {
	if (!paused)
		framenum++;

	if (processes.size() == 0) {
		return;
		/*
		perr << "Process queue is empty?! Aborting.\n";

		//! do this in a cleaner way
		exit(0);
		*/
	}
	current_process = processes.begin();
	while (current_process != processes.end()) {
		Process *p = *current_process;

		if (!paused && ((p->flags & (Process::PROC_TERMINATED |
		                             Process::PROC_TERM_DEFERRED))
		                == Process::PROC_TERM_DEFERRED)) {
			p->terminate();
		}
		if (!(p->is_terminated() || p->is_suspended()) &&
		        (!paused || (p->flags & Process::PROC_RUNPAUSED))) {
			runningprocess = p;
			p->run();

			if (!runningprocess)
				return; // If this happens then the list was reset so leave NOW!

			runningprocess = 0;
		}
		if (!paused && (p->flags & Process::PROC_TERMINATED)) {
			// process is killed, so remove it from the list
			current_process = processes.erase(current_process);

			// Clear pid
			pIDs->clearID(p->pid);

			//! is this the right place to delete processes?
			delete p;
		} else
			++current_process;
	}

	if (!paused && framebyframe) pause();
}

void Kernel::setNextProcess(Process *proc) {
	if (current_process != processes.end() && *current_process == proc) return;

	if (proc->flags & Process::PROC_ACTIVE) {
		for (ProcessIterator it = processes.begin();
		        it != processes.end(); ++it) {
			if (*it == proc) {
				processes.erase(it);
				break;
			}
		}
	} else {
		proc->flags |= Process::PROC_ACTIVE;
	}

	if (current_process == processes.end()) {
		processes.push_front(proc);
	} else {
		ProcessIterator t = current_process;
		++t;

		processes.insert(t, proc);
	}
}

Process *Kernel::getProcess(ProcId pid) {
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;
		if (p->pid == pid)
			return p;
	}
	return 0;
}

void Kernel::kernelStats() {
	pout << "Kernel memory stats:" << Std::endl;
	pout << "Processes  : " << processes.size() << "/32765" << Std::endl;
}

void Kernel::processTypes() {
	pout << "Current process types:" << Std::endl;
	Std::map<Common::String, unsigned int> processtypes;
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;
		processtypes[p->GetClassType().class_name]++;
	}
	Std::map<Common::String, unsigned int>::iterator iter;
	for (iter = processtypes.begin(); iter != processtypes.end(); ++iter) {
		pout << (*iter)._key << ": " << (*iter)._value << Std::endl;
	}
}

void Kernel::ConCmd_processTypes(const Console::ArgvType & /*argv*/) {
	Kernel::get_instance()->processTypes();
}

void Kernel::ConCmd_listProcesses(const Console::ArgvType &argv) {
	if (argv.size() > 2) {
		pout << "usage: listProcesses [<itemnum>]" << Std::endl;
		return;
	}

	Kernel *kern = Kernel::get_instance();
	ObjId item = 0;
	if (argv.size() == 2) {
		item = static_cast<ObjId>(strtol(argv[1].c_str(), 0, 0));
		pout << "Processes for item " << item << ":" << Std::endl;
	} else {
		pout << "Processes:" << Std::endl;
	}
	for (ProcessIterator it = kern->processes.begin();
	        it != kern->processes.end(); ++it) {
		Process *p = *it;
		if (argv.size() == 1 || p->item_num == item)
			p->dumpInfo();
	}

}

void Kernel::ConCmd_processInfo(const Console::ArgvType &argv) {
	if (argv.size() != 2) {
		pout << "usage: processInfo <objectnum>" << Std::endl;
		return;
	}

	Kernel *kern = Kernel::get_instance();

	ProcId procid = static_cast<ProcId>(strtol(argv[1].c_str(), 0, 0));

	Process *p = kern->getProcess(procid);
	if (p == 0) {
		pout << "No such process: " << procid << Std::endl;
	} else {
		p->dumpInfo();
	}
}

void Kernel::ConCmd_toggleFrameByFrame(const Console::ArgvType &argv) {
	Kernel *kern = Kernel::get_instance();
	bool fbf = !kern->isFrameByFrame();
	kern->setFrameByFrame(fbf);
	pout << "FrameByFrame = " << fbf << Std::endl;
	if (fbf)
		kern->pause();
	else
		kern->unpause();
}

void Kernel::ConCmd_advanceFrame(const Console::ArgvType &argv) {
	Kernel *kern = Kernel::get_instance();
	if (kern->isFrameByFrame()) {
		kern->unpause();
		pout << "FrameByFrame: Next Frame" << Std::endl;
	}
}

uint32 Kernel::getNumProcesses(ObjId objid, uint16 processtype) {
	uint32 count = 0;

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;

		// Don't count us, we are not really here
		if (p->is_terminated()) continue;

		if ((objid == 0 || objid == p->item_num) &&
		        (processtype == 6 || processtype == p->type))
			count++;
	}

	return count;
}

Process *Kernel::findProcess(ObjId objid, uint16 processtype) {
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;

		// Don't count us, we are not really here
		if (p->is_terminated()) continue;

		if ((objid == 0 || objid == p->item_num) &&
		        (processtype == 6 || processtype == p->type)) {
			return p;
		}
	}

	return 0;
}


void Kernel::killProcesses(ObjId objid, uint16 processtype, bool fail) {
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;

		if (p->item_num != 0 && (objid == 0 || objid == p->item_num) &&
		        (processtype == 6 || processtype == p->type) &&
		        !(p->flags & Process::PROC_TERMINATED) &&
		        !(p->flags & Process::PROC_TERM_DEFERRED)) {
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::killProcessesNotOfType(ObjId objid, uint16 processtype, bool fail) {
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process *p = *it;

		if (p->item_num != 0 && (objid == 0 || objid == p->item_num) &&
		        (p->type != processtype) &&
		        !(p->flags & Process::PROC_TERMINATED) &&
		        !(p->flags & Process::PROC_TERM_DEFERRED)) {
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::save(ODataSource *ods) {
	ods->write4(framenum);
	pIDs->save(ods);
	ods->write4(processes.size());
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		(*it)->save(ods);
	}
}

bool Kernel::load(IDataSource *ids, uint32 version) {
	framenum = ids->read4();

	if (!pIDs->load(ids, version)) return false;

	uint32 pcount = ids->read4();

	for (unsigned int i = 0; i < pcount; ++i) {
		Process *p = loadProcess(ids, version);
		if (!p) return false;
		processes.push_back(p);
	}

	return true;
}

Process *Kernel::loadProcess(IDataSource *ids, uint32 version) {
	uint16 classlen = ids->read2();
	char *buf = new char[classlen + 1];
	ids->read(buf, classlen);
	buf[classlen] = 0;

	Std::string classname = buf;
	delete[] buf;

	Std::map<Common::String, ProcessLoadFunc>::iterator iter;
	iter = processloaders.find(classname);

	if (iter == processloaders.end()) {
		perr << "Unknown Process class: " << classname << Std::endl;
		return 0;
	}


	loading = true;

	Process *p = (*(iter->_value))(ids, version);

	loading = false;

	return p;
}

uint32 Kernel::I_getNumProcesses(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(item);
	ARG_UINT16(type);

	return Kernel::get_instance()->getNumProcesses(item, type);
}

uint32 Kernel::I_resetRef(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(item);
	ARG_UINT16(type);

	Kernel::get_instance()->killProcesses(item, type, true);
	return 0;
}

uint getRandom() {
	return Ultima8Engine::get_instance()->getRandomNumber(0x7fffffff);
}

} // End of namespace Ultima8
} // End of namespace Ultima
