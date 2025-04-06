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

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(Process)

Process::Process(ObjId it, uint16 ty)
	: _pid(0xFFFF), _flags(0), _itemNum(it), _type(ty), _result(0), _ticksPerRun(2) {
	Kernel::get_instance()->assignPID(this);
	if (GAME_IS_CRUSADER) {
		// Default kernel ticks per run of processes in Crusader
		_ticksPerRun = 1;
	}
}

void Process::fail() {
	_flags |= PROC_FAILED;
	terminate();
}

void Process::terminate() {
	if (_flags & PROC_TERMINATED)
		return;

	Kernel *kernel = Kernel::get_instance();

	// wake up waiting processes
	for (const auto &pid : _waiting) {
		Process *p = kernel->getProcess(pid);
		if (p)
			p->wakeUp(_result);
	}
	_waiting.clear();

	_flags |= PROC_TERMINATED;
}

void Process::wakeUp(uint32 result) {
	_result = result;

	_flags &= ~PROC_SUSPENDED;

	Kernel::get_instance()->setNextProcess(this);

	onWakeUp();
}

void Process::waitFor(ProcId pid) {
	assert(pid != _pid);
	if (pid) {
		Kernel *kernel = Kernel::get_instance();

		// add this process to waiting list of other process
		Process *p = kernel->getProcess(pid);
		assert(p);
		if (p->getProcessFlags() & PROC_TERMINATED) {
			//warning("Proc %d wait for proc %d which is already terminated", _pid, pid);
			return;
		}
		p->_waiting.push_back(_pid);

		// Note: The original games sync itemnum between processes
		// here if either one is zero, but that seems to break things
		// for us so we don't do it.
	}

	_flags |= PROC_SUSPENDED;
}

void Process::waitFor(Process *proc) {
	assert(this != proc);
	ProcId pid = 0;
	if (proc) pid = proc->getPid();

	waitFor(pid);
}

void Process::suspend() {
	_flags |= PROC_SUSPENDED;
}

Common::String Process::dumpInfo() const {
	Common::String info = Common::String::format(
		"Process %d class %s, item %d, type %x, status ",
		getPid(), GetClassType()._className, _itemNum, _type);

	if (_flags & PROC_ACTIVE) info += "A";
	if (_flags & PROC_SUSPENDED) info += "S";
	if (_flags & PROC_TERMINATED) info += "T";
	if (_flags & PROC_TERM_DEFERRED) info += "t";
	if (_flags & PROC_FAILED) info += "F";
	if (_flags & PROC_RUNPAUSED) info += "R";
	if (_flags & PROC_TERM_DISPOSE) info += "D";

	if (!_waiting.empty()) {
		info += ", notify: ";
		for (Std::vector<ProcId>::const_iterator i = _waiting.begin(); i != _waiting.end(); ++i) {
			if (i != _waiting.begin()) info += ", ";
			info += Common::String::format("%d", *i);
		}
	}

	return info;
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

	if (waitcount > 1024*1024) {
		warning("Improbable waitcount %d for proc %d. Corrupt save?", waitcount, _pid);
		return false;
	}

	_waiting.resize(waitcount);
	for (unsigned int i = 0; i < waitcount; ++i)
		_waiting[i] = rs->readUint16LE();

	return true;
}

bool Process::validateWaiters() const {
	for (const auto &pid : _waiting) {
		const Process *p = Kernel::get_instance()->getProcess(pid);
		if (!p) {
			// This can happen if a waiting process gets forcibly terminated.
			warning("Invalid procid %d in waitlist for proc %d. Maybe a bug?", pid, _pid);
		} else if (!p->is_suspended()) {
			// This should never happen.
			warning("Procid %d in waitlist for proc %d but not marked suspended", pid, _pid);
			return false;
		}
	}
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
