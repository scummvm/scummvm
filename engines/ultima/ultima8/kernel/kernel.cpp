/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

Kernel *Kernel::_kernel = nullptr;

const uint32 Kernel::TICKS_PER_FRAME = 2;
const uint32 Kernel::TICKS_PER_SECOND = 60;
const uint32 Kernel::FRAMES_PER_SECOND = Kernel::TICKS_PER_SECOND / Kernel::TICKS_PER_FRAME;

// A special proc type which means "all"
const uint16 Kernel::PROC_TYPE_ALL = 6;

// The same as above, but for Crusader.
// Used in Usecode functions to translate.
static const uint16 CRU_PROC_TYPE_ALL = 0xc;

Kernel::Kernel() : _loading(false), _tickNum(0), _paused(0),
		_runningProcess(nullptr), _frameByFrame(false) {
	debugN(MM_INFO, "Creating Kernel...\n");

	_kernel = this;
	_pIDs = new idMan(1, 32766, 128);
	_currentProcess = _processes.end();
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
	_currentProcess = _processes.begin();

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
	<< ", pid = " << proc->_pid << " type " << proc->GetClassType()._className << Std::endl;
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

void Kernel::runProcesses() {
	if (!_paused)
		_tickNum++;

	if (_processes.size() == 0) {
		warning("Process queue is empty?! Aborting.");
		return;
	}

	int num_run = 0;

	_currentProcess = _processes.begin();
	while (_currentProcess != _processes.end()) {
		Process *p = *_currentProcess;

		if (!_paused && ((p->_flags & (Process::PROC_TERMINATED |
		                             Process::PROC_TERM_DEFERRED))
		                == Process::PROC_TERM_DEFERRED)) {
			p->terminate();
		}
		if (!(p->is_terminated() || p->is_suspended()) &&
		        (!_paused || (p->_flags & Process::PROC_RUNPAUSED)) &&
				(_paused || _tickNum % p->getTicksPerRun() == 0)) {
			_runningProcess = p;
			p->run();

			num_run++;

			//
			// WORKAROUND:
			// In Crusader: No Remorse, the HOVER near the end of Mission 3
			// (Floor 1) gets stuck in a tight loop after moving to the
			// destination (path egg frame 0).
			//
			// Something is probably not right about the switch trigger, but until
			// we can work out what it is avoid the game totally hanging at this
			// point.
			//
			// If this threshold is set too low, it can cause issues with U8 map
			// transitions (eg, bug #12913).  If it's too high, Crusader locks up
			// for a really long time at this point.  Set it high enough that
			// a process going through all map items should still terminate.
			//
			if (((num_run > 8192 && GAME_IS_CRUSADER) || num_run > 65534)
					&& !p->is_terminated()) {
				warning("Seem to be stuck in process loop - killing current process");
				p->fail();
			}

			if (!_runningProcess)
				return; // If this happens then the list was reset so leave NOW!

			_runningProcess = nullptr;
		}
		if (!_paused && (p->_flags & Process::PROC_TERMINATED)) {
			// process is killed, so remove it from the list
			_currentProcess = _processes.erase(_currentProcess);

			// Clear pid
			_pIDs->clearID(p->_pid);

			//! is this the right place to delete processes?
			delete p;
		} else if (!_paused && (p->_flags & Process::PROC_TERM_DEFERRED) && GAME_IS_CRUSADER) {
			//
			// In Crusader, move term deferred processes to the end to clean up after
			// others have run.  This gets the right speed on ELEVAT (which should
			// execute one movement per tick)
			//
			// In U8, frame-count comparison for Devon turning at the start shows this
			// *shouldn't* be used, and the process should be cleaned up next tick.
			//
			_processes.push_back(p);
			_currentProcess = _processes.erase(_currentProcess);
		} else {
			++_currentProcess;
		}
	}

	if (!_paused && _frameByFrame) pause();
}

void Kernel::setNextProcess(Process *proc) {
	if (_currentProcess != _processes.end() && *_currentProcess == proc) return;

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

	if (_currentProcess == _processes.end()) {
		// Not currently running processes, add to the start of the next run.
		_processes.push_front(proc);
	} else {
		ProcessIterator t = _currentProcess;
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
	Common::HashMap<Common::String, unsigned int> processtypes;
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;
		processtypes[p->GetClassType()._className]++;
	}
	Common::HashMap<Common::String, unsigned int>::const_iterator iter;
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
		        (processtype == PROC_TYPE_ALL || processtype == p->_type))
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
		        (processtype == PROC_TYPE_ALL || processtype == p->_type)) {
			return p;
		}
	}

	return nullptr;
}


void Kernel::killProcesses(ObjId objid, uint16 processtype, bool fail) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		if (p->_itemNum != 0 && (objid == 0 || objid == p->_itemNum) &&
		        (processtype == PROC_TYPE_ALL || processtype == p->_type) &&
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

		// * If objid is 0, terminate procs for all objects.
		// * Never terminate procs with objid 0
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

void Kernel::killAllProcessesNotOfTypeExcludeCurrent(uint16 processtype, bool fail) {
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		Process *p = *it;

		// Don't kill the running process
		if (p == _runningProcess)
			continue;

		if ((p->_type != processtype) &&
				!(p->_flags & Process::PROC_TERMINATED) &&
				!(p->_flags & Process::PROC_TERM_DEFERRED)) {
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}


void Kernel::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_tickNum);
	_pIDs->save(ws);
	ws->writeUint32LE(_processes.size());
	for (ProcessIterator it = _processes.begin(); it != _processes.end(); ++it) {
		const Std::string & classname = (*it)->GetClassType()._className; // virtual
		assert(classname.size());

		Common::HashMap<Common::String, ProcessLoadFunc>::iterator iter;
		iter = _processLoaders.find(classname);

		if (iter == _processLoaders.end()) {
			error("Process class cannot save without registered loader: %s", classname.c_str());
		}

		ws->writeUint16LE(classname.size());
		ws->write(classname.c_str(), classname.size());
		(*it)->saveData(ws);
	}
}

bool Kernel::load(Common::ReadStream *rs, uint32 version) {
	_tickNum = rs->readUint32LE();

	if (!_pIDs->load(rs, version)) return false;

	const uint32 pcount = rs->readUint32LE();

	for (unsigned int i = 0; i < pcount; ++i) {
		Process *p = loadProcess(rs, version);
		if (!p) return false;
		_processes.push_back(p);
	}

	// Integrity check for processes
	Std::set<ProcId> procs;
	for (Std::list<Process *>::const_iterator iter = _processes.begin(); iter != _processes.end(); iter++) {
		const Process *p = *iter;
		if (!_pIDs->isIDUsed(p->getPid())) {
			warning("Process id %d exists but not marked used.  Corrupt save?", p->getPid());
			return false;
		}
		if (procs.find(p->getPid()) != procs.end()) {
			warning("Duplicate process id %d in processes.  Corrupt save?", p->getPid());
			return false;
		}
		procs.insert(p->getPid());
		if (!p->validateWaiters()) {
			return false;
		}
		if (p->getTicksPerRun() > 100) {
			warning("Improbable value for ticks per run %d in process id %d .  Corrupt save?", p->getTicksPerRun(), p->getPid());
			return false;
		}
		if (p->getType() > 0x1000) {
			warning("Improbable value for proctype %x in process id %d .  Corrupt save?", p->getType(), p->getPid());
			return false;
		}
	}

	return true;
}

Process *Kernel::loadProcess(Common::ReadStream *rs, uint32 version) {
	const uint16 classlen = rs->readUint16LE();
	assert(classlen > 0);
	char *buf = new char[classlen + 1];
	rs->read(buf, classlen);
	buf[classlen] = 0;

	Std::string classname = buf;
	delete[] buf;

	Common::HashMap<Common::String, ProcessLoadFunc>::iterator iter;
	iter = _processLoaders.find(classname);

	if (iter == _processLoaders.end()) {
		perr << "Unknown Process class: " << classname << Std::endl;
		return nullptr;
	}

	_loading = true;

	Process *p = (*(iter->_value))(rs, version);

	_loading = false;

	return p;
}

uint32 Kernel::I_getNumProcesses(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(item);
	ARG_UINT16(type);

	if (GAME_IS_CRUSADER && type == CRU_PROC_TYPE_ALL)
		type = PROC_TYPE_ALL;

	return Kernel::get_instance()->getNumProcesses(item, type);
}

uint32 Kernel::I_resetRef(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(item);
	ARG_UINT16(type);

	if (GAME_IS_CRUSADER && type == CRU_PROC_TYPE_ALL)
		type = PROC_TYPE_ALL;

	Kernel::get_instance()->killProcesses(item, type, true);
	return 0;
}

const uint U8_RAND_MAX = 0x7fffffff;

uint getRandom() {
	return Ultima8Engine::get_instance()->getRandomNumber(U8_RAND_MAX);
}

} // End of namespace Ultima8
} // End of namespace Ultima
