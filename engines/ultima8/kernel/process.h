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

#include "ultima8/std/containers.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class IDataSource;
class ODataSource;

class Process {
public:
	friend class Kernel;

	virtual void run() = 0;

	Process(ObjId item_num = 0, uint16 type = 0);
	virtual ~Process() { }

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	// memory pooling stuff
	ENABLE_CUSTOM_MEMORY_ALLOCATION()

	uint32 getProcessFlags() const {
		return flags;
	}
	bool is_active() const {
		return (flags & PROC_ACTIVE);
	}
	bool is_terminated() const {
		return (flags & (PROC_TERMINATED |
		                 PROC_TERM_DEFERRED)) != 0;
	}
	bool is_suspended() const {
		return (flags & PROC_SUSPENDED) != 0;
	}

	//! terminate the process and recursively fail all processes waiting for it
	void fail();

	//! terminate the process. This wakes up all processes waiting for it.
	virtual void terminate();

	//! terminate next frame
	void terminateDeferred() {
		flags |= PROC_TERM_DEFERRED;
	}

	//! suspend until process 'pid' returns. If pid is 0, suspend indefinitely
	void waitFor(ProcId pid);

	//! suspend until process returns. If proc is 0, suspend indefinitely
	void waitFor(Process *proc);

	//! suspend process
	void suspend();

	void wakeUp(uint32 result);

	void setItemNum(ObjId it) {
		item_num = it;
	}
	void setType(uint16 ty) {
		type = ty;
	}

	ProcId getPid() const {
		return pid;
	}
	ObjId getItemNum() const {
		return item_num;
	}
	uint16 getType() const {
		return type;
	}

	//! dump some info about this process to pout
	virtual void dumpInfo();

	//! save this process
	void save(ODataSource *ods);

	//! load Process data
	bool loadData(IDataSource *ids, uint32 version);

protected:
	//! save the Process data
	virtual void saveData(ODataSource *ods);

	void writeProcessHeader(ODataSource *ods);

	//! process id
	ProcId pid;

	uint32 flags;

	//! item we are assigned to
	ObjId item_num;
	uint16 type;

	//! process result
	uint32 result;

	//! Processes waiting for this one to finish.
	//! When this process terminates, awaken them and pass them the result val.
	std::vector<ProcId> waiting;

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

#endif
