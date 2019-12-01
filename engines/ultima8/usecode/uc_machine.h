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

#ifndef ULTIMA8_USECODE_UCMACHINE_H
#define ULTIMA8_USECODE_UCMACHINE_H

#include "ultima8/std/containers.h"
#include "ultima8/usecode/intrinsics.h"

namespace Ultima8 {

class Process;
class UCProcess;
class ConvertUsecode;
class IDataSource;
class ODataSource;
class BitSet;
class UCList;
class idMan;

class UCMachine {
public:
	UCMachine(Intrinsic *iset, unsigned int icount);
	~UCMachine();

	static UCMachine *get_instance() {
		return ucmachine;
	}

	void reset();

	void execProcess(UCProcess *proc);

	std::string &getString(uint16 str);
	UCList *getList(uint16 l);

	void freeString(uint16 s);
	void freeStringList(uint16 l);
	void freeList(uint16 l);

	uint16 duplicateString(uint16 str);

	void usecodeStats();

	static uint32 listToPtr(uint16 l);
	static uint32 stringToPtr(uint16 s);
	static uint32 stackToPtr(uint16 pid, uint16 offset);
	static uint32 globalToPtr(uint16 offset);
	static uint32 objectToPtr(uint16 objID);

	static uint16 ptrToObject(uint32 ptr);

	bool assignPointer(uint32 ptr, const uint8 *data, uint32 size);
	bool dereferencePointer(uint32 ptr, uint8 *data, uint32 size);

	void saveGlobals(ODataSource *ods);
	void saveStrings(ODataSource *ods);
	void saveLists(ODataSource *ods);

	bool loadGlobals(IDataSource *ids, uint32 version);
	bool loadStrings(IDataSource *ids, uint32 version);
	bool loadLists(IDataSource *ids, uint32 version);

	INTRINSIC(I_true);
	INTRINSIC(I_dummyProcess);
	INTRINSIC(I_getName);
	INTRINSIC(I_urandom);
	INTRINSIC(I_rndRange);
	INTRINSIC(I_numToStr);
	INTRINSIC(I_getCurrentTimerTick);

protected:
	void loadIntrinsics(Intrinsic *i, unsigned int icount);

private:

	ConvertUsecode *convuse;
	Intrinsic *intrinsics;
	unsigned int intrinsiccount;

	BitSet *globals;

	std::map<uint16, UCList *> listHeap;
	std::map<uint16, std::string> stringHeap;

	uint16 assignString(const char *str);
	uint16 assignList(UCList *l);

	idMan *listIDs;
	idMan *stringIDs;

	static UCMachine *ucmachine;

	static void     ConCmd_getGlobal(const Console::ArgvType &argv);
	static void     ConCmd_setGlobal(const Console::ArgvType &argv);


#ifdef DEBUG
	// tracing
	bool tracing_enabled;
	bool trace_all;
	bool trace_events;
	std::set<ObjId> trace_ObjIDs;
	std::set<ProcId> trace_PIDs;
	std::set<uint16> trace_classes;

	inline bool trace_show(ProcId pid, ObjId objid, uint16 ucclass) {
		if (!tracing_enabled) return false;
		if (trace_all) return true;
		if (trace_ObjIDs.find(objid) != trace_ObjIDs.end()) return true;
		if (trace_PIDs.find(pid) != trace_PIDs.end()) return true;
		if (trace_classes.find(ucclass) != trace_classes.end()) return true;
		return false;
	}

public:
	bool trace_event() {
		return (tracing_enabled && (trace_all || trace_events));
	}

private:
	static void     ConCmd_tracePID(const Console::ArgvType &argv);
	static void     ConCmd_traceObjID(const Console::ArgvType &argv);
	static void     ConCmd_traceClass(const Console::ArgvType &argv);
	static void     ConCmd_traceAll(const Console::ArgvType &argv);
	static void     ConCmd_traceEvents(const Console::ArgvType &argv);
	static void     ConCmd_stopTrace(const Console::ArgvType &argv);
#endif
};

} // End of namespace Ultima8

#endif
