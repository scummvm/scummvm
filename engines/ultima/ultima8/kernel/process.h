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

#ifndef ULTIMA8_KERNEL_USECODE_PROCESS_H
#define ULTIMA8_KERNEL_USECODE_PROCESS_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"
#include "ultima/ultima8/misc/pent_include.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;

class Process {
	friend class Kernel;
	friend class Debugger;
public:
	virtual void run() = 0;

	Process(ObjId _itemNum = 0, uint16 type = 0);
	virtual ~Process() { }

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE_BASE()

	uint32 getProcessFlags() const {
		return _flags;
	}
	bool is_active() const {
		return (_flags & PROC_ACTIVE);
	}
	bool is_terminated() const {
		return (_flags & (PROC_TERMINATED |
		                 PROC_TERM_DEFERRED)) != 0;
	}
	bool is_suspended() const {
		return (_flags & PROC_SUSPENDED) != 0;
	}

	//! terminate the process and recursively fail all processes waiting for it
	void fail();

	//! terminate the process. This wakes up all processes waiting for it.
	virtual void terminate();

	//! terminate next frame
	void terminateDeferred() {
		_flags |= PROC_TERM_DEFERRED;
	}

	//! run even when paused
	void setRunPaused() {
		_flags |= PROC_RUNPAUSED;
	};

	//! suspend until process 'pid' returns. If pid is 0, suspend indefinitely
	void waitFor(ProcId pid);

	//! suspend until process returns. If proc is 0, suspend indefinitely
	void waitFor(Process *proc);

	//! suspend process
	void suspend();

	//! Wake up when the process we were waiting for has finished
	void wakeUp(uint32 result);

	//! A hook to add aditional behavior on wakeup, before anything else happens
	virtual void onWakeUp() {};

	void setItemNum(ObjId it) {
		_itemNum = it;
	}
	void setType(uint16 ty) {
		_type = ty;
	}
	void setTicksPerRun(uint32 val) {
		_ticksPerRun = val;
	}

	ProcId getPid() const {
		return _pid;
	}
	ObjId getItemNum() const {
		return _itemNum;
	}
	uint16 getType() const {
		return _type;
	}
	uint32 getTicksPerRun() const {
		return _ticksPerRun;
	}

	//! dump some info about this process to pout
	virtual void dumpInfo() const;

	//! load Process data
	bool loadData(Common::ReadStream *rs, uint32 version);

	//! save Process data
	virtual void saveData(Common::WriteStream *ws);

protected:
	//! process id
	ProcId _pid;

	uint32 _flags;

	//! how many kernel ticks between when this process should be run.
	//! not saved because it's fixed by process type and game.
	uint32 _ticksPerRun;

	//! item we are assigned to
	ObjId _itemNum;
	uint16 _type;

	//! process result
	uint32 _result;

	//! Processes waiting for this one to finish.
	//! When this process terminates, awaken them and pass them the result val.
	Std::vector<ProcId> _waiting;

public:

	enum processflags {
		PROC_ACTIVE      = 0x0001,   //!< is the process in the run-list?
		PROC_SUSPENDED   = 0x0002,   //!< suspended? (because it's waiting)
		PROC_TERMINATED  = 0x0004,
		PROC_TERM_DEFERRED = 0x0008, //!< automatically call terminate next frame
		PROC_FAILED      = 0x0010,
		PROC_RUNPAUSED   = 0x0020    //!< run even if game is paused
	};

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
