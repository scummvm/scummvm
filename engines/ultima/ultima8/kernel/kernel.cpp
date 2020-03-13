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

Kernel *Kernel::_kernel = nullptr;

Kernel::Kernel() : _loading(false) {
	debugN(MM_INFO, "Creating Kernel...\n");

	_kernel = this;
	_pIDs = new idMan(1, 32766, 128);
	current_process = _processes.end();
	_frameNum = 0;
	_paused = 0;
	_runningProcess = nullptr;
	_frameByFrame = false;
}

Kernel::~Kernel() {
	reset();
	debugN(MM_INFO, "Destroying Kernel...\n");

	_kernel = nullptr;

	delete _pIDs;
}

void Kernel::reset() {
	debugN(MM_INFO, "Resetting Kernel...\n");

	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		delete(*it);
	}
	_processes.clear();
	current_process = _processes.begin();

	_pIDs->clearAll();

	_paused = 0;
	_runningProcess = nullptr;

	// if we're in frame-by-frame mode, reset to a _paused state
	if (_frameByFrame) _paused = 1;
}

ProcId Kernel::assignPID(Process *proc) {
	// to prevent new processes from getting a PID while loading
	if (_loading) return 0xFFFF;

	// Get a pID
	proc->_pid = _pIDs->getNewID();

	return proc->_pid;
}

ProcId Kernel::addProcess(Process *proc) {
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->_pid != 0 && proc->_pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
	     << ", pid = " << proc->_pid << Std::endl;
#endif

//	processes.push_back(proc);
//	proc->active = true;
	setNextProcess(proc);
	return proc->_pid;
}

ProcId Kernel::addProcessExec(Process *proc) {
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->_pid != 0 && proc->_pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
	     << ", pid = " << proc->_pid << Std::endl;
#endif

	_processes.push_back(proc);
	proc->_flags |= Process::PROC_ACTIVE;

	Process *oldrunning = _runningProcess;
	_runningProcess = proc;
	proc->run();
	_runningProcess = oldrunning;

	return proc->_pid;
}

void Kernel::removeProcess(Process *proc) {
	//! the way to remove processes has to be thought over sometime
	//! we probably want to flag them as terminated before actually
	//! removing/deleting it or something
	//! also have to look out for deleting processes while iterating
	//! over the list. (Hence the special 'erase' in runProcs below, which
	//! is very Std::list-specific, incidentally)

	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		if (*it == proc) {
			proc->_flags &= ~Process::PROC_ACTIVE;

			perr << "[Kernel] Removing process " << proc << Std::endl;

			_processes.erase(it);

			// Clear pid
			_pIDs->clearID(proc->_pid);

			return;
		}
	}
}


void Kernel::runProcesses() {
	if (!_paused)
		_frameNum++;

	if (_processes.size() == 0) {
		return;
		/*
		perr << "Process queue is empty?! Aborting.\n";

		//! do this in a cleaner way
		exit(0);
		*/
	}
	current_process = _processes.begin();
	while (current_process != _processes.end()) {
		Process *p = *current_process;

		if (!_paused && ((p->_flags & (Process::PROC_TERMINATED |
		                             Process::PROC_TERM_DEFERRED))
		                == Process::PROC_TERM_DEFERRED)) {
			p->terminate();
		}
		if (!(p->is_terminated() || p->is_suspended()) &&
		        (!_paused || (p->_flags & Process::PROC_RUNPAUSED))) {
			_runningProcess = p;
			p->run();

			if (!_runningProcess)
				return; // If this happens then the list was reset so leave NOW!

			_runningProcess = nullptr;
		}
		if (!_paused && (p->_flags & Process::PROC_TERMINATED)) {
			// process is killed, so remove it from the list
			current_process = _processes.erase(current_process);

			// Clear pid
			_pIDs->clearID(p->_pid);

			//! is this the right place to delete processes?
			delete p;
		} else
			++current_process;
	}

	if (!_paused && _frameByFrame) pause();
}

void Kernel::setNextProcess(Process *proc) {
	if (current_process != _processes.end() && *current_process == proc) return;

	if (proc->_flags & Process::PROC_ACTIVE) {
		for (ProcessIterator it = _processes.begin();
		        it != _processes.end(); ++it) {
			if (*it == proc) {
				_processes.erase(it);
				break;
			}
		}
	} else {
		proc->_flags |= Process::PROC_ACTIVE;
	}

	if (current_process == _processes.end()) {
		_processes.push_front(proc);
	} else {
		ProcessIterator t = current_process;
		++t;

		_processes.insert(t, proc);
	}
}

Process *Kernel::getProcess(ProcId pid) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;
		if (p->_pid == pid)
			return p;
	}
	return nullptr;
}

void Kernel::kernelStats() {
	g_debugger->debugPrintf("Kernel memory stats:\n");
	g_debugger->debugPrintf("Processes  : %u/32765\n", _processes.size());
}

void Kernel::processTypes() {
	g_debugger->debugPrintf("Current process types:\n");
	Std::map<Common::String, unsigned int> processtypes;
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;
		processtypes[p->GetClassType()._className]++;
	}
	Std::map<Common::String, unsigned int>::iterator iter;
	for (iter = processtypes.begin(); iter != processtypes.end(); ++iter) {
		g_debugger->debugPrintf("%s: %u\n", (*iter)._key.c_str(), (*iter)._value);
	}
}

uint32 Kernel::getNumProcesses(ObjId objid, uint16 processtype) {
	uint32 count = 0;

	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		// Don't count us, we are not really here
		if (p->is_terminated()) continue;

		if ((objid == 0 || objid == p->_itemNum) &&
		        (processtype == 6 || processtype == p->_type))
			count++;
	}

	return count;
}

Process *Kernel::findProcess(ObjId objid, uint16 processtype) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		// Don't count us, we are not really here
		if (p->is_terminated()) continue;

		if ((objid == 0 || objid == p->_itemNum) &&
		        (processtype == 6 || processtype == p->_type)) {
			return p;
		}
	}

	return nullptr;
}


void Kernel::killProcesses(ObjId objid, uint16 processtype, bool fail) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		if (p->_itemNum != 0 && (objid == 0 || objid == p->_itemNum) &&
		        (processtype == 6 || processtype == p->_type) &&
		        !(p->_flags & Process::PROC_TERMINATED) &&
		        !(p->_flags & Process::PROC_TERM_DEFERRED)) {
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::killProcessesNotOfType(ObjId objid, uint16 processtype, bool fail) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		if (p->_itemNum != 0 && (objid == 0 || objid == p->_itemNum) &&
		        (p->_type != processtype) &&
		        !(p->_flags & Process::PROC_TERMINATED) &&
		        !(p->_flags & Process::PROC_TERM_DEFERRED)) {
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::save(ODataSource *ods) {
	ods->write4(_frameNum);
	_pIDs->save(ods);
	ods->write4(_processes.size());
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		(*it)->save(ods);
	}
}

bool Kernel::load(IDataSource *ids, uint32 version) {
	_frameNum = ids->read4();

	if (!_pIDs->load(ids, version)) return false;

	const uint32 pcount = ids->read4();

	for (unsigned int i = 0; i < pcount; ++i) {
		Process *p = loadProcess(ids, version);
		if (!p) return false;
		_processes.push_back(p);
	}

	return true;
}

Process *Kernel::loadProcess(IDataSource *ids, uint32 version) {
	const uint16 classlen = ids->read2();
	char *buf = new char[classlen + 1];
	ids->read(buf, classlen);
	buf[classlen] = 0;

	Std::string classname = buf;
	delete[] buf;

	Std::map<Common::String, ProcessLoadFunc>::iterator iter;
	iter = _processLoaders.find(classname);

	if (iter == _processLoaders.end()) {
		perr << "Unknown Process class: " << classname << Std::endl;
		return nullptr;
	}


	_loading = true;

	Process *p = (*(iter->_value))(ids, version);

	_loading = false;

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
