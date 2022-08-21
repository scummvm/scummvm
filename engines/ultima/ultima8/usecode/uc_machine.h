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

#ifndef ULTIMA8_USECODE_UCMACHINE_H
#define ULTIMA8_USECODE_UCMACHINE_H

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima {
namespace Ultima8 {

class Debugger;
class Process;
class UCProcess;
class ConvertUsecode;
class GlobalStorage;
class UCList;
class idMan;

class UCMachine {
	friend class Debugger;
public:
	UCMachine(Intrinsic *iset, unsigned int icount);
	~UCMachine();

	static UCMachine *get_instance() {
		return _ucMachine;
	}

	void reset();

	void execProcess(UCProcess *proc);

	const Std::string &getString(uint16 str) const;
	UCList *getList(uint16 l);

	void freeString(uint16 s);
	void freeStringList(uint16 l);
	void freeList(uint16 l);

	uint16 duplicateString(uint16 str);

	void usecodeStats() const;

	static uint32 listToPtr(uint16 l);
	static uint32 stringToPtr(uint16 s);
	static uint32 stackToPtr(uint16 pid, uint16 offset);
	static uint32 globalToPtr(uint16 offset);
	static uint32 objectToPtr(uint16 objID);

	static uint16 ptrToObject(uint32 ptr);

	bool assignPointer(uint32 ptr, const uint8 *data, uint32 size);
	bool dereferencePointer(uint32 ptr, uint8 *data, uint32 size);

	void saveGlobals(Common::WriteStream *ws) const;
	void saveStrings(Common::WriteStream *ws) const;
	void saveLists(Common::WriteStream *ws) const;

	bool loadGlobals(Common::ReadStream *rs, uint32 version);
	bool loadStrings(Common::ReadStream *rs, uint32 version);
	bool loadLists(Common::ReadStream *rs, uint32 version);

	INTRINSIC(I_true);
	INTRINSIC(I_false);
	INTRINSIC(I_dummyProcess);
	INTRINSIC(I_getName);
	INTRINSIC(I_urandom);
	INTRINSIC(I_rndRange);
	INTRINSIC(I_numToStr);

protected:
	void loadIntrinsics(Intrinsic *i, unsigned int icount);

private:
	ConvertUsecode *_convUse;
	Intrinsic *_intrinsics;
	unsigned int _intrinsicCount;

	GlobalStorage *_globals;

	Common::HashMap<uint16, UCList *> _listHeap;
	Common::HashMap<uint16, Std::string> _stringHeap;

	// Add a string to the list (copies the string)
	uint16 assignString(const char *str);
	uint16 assignList(UCList *l);

	idMan *_listIDs;
	idMan *_stringIDs;

	static UCMachine *_ucMachine;

#ifdef DEBUG
	// tracing
	bool _tracingEnabled;
	bool _traceAll;
	bool _traceEvents;
	Std::set<ObjId> _traceObjIDs;
	Std::set<ProcId> _tracePIDs;
	Std::set<uint16> _traceClasses;

	inline bool trace_show(ProcId pid, ObjId objid, uint16 ucclass) {
		if (!_tracingEnabled) return false;
		if (_traceAll) return true;
		if (_traceObjIDs.find(objid) != _traceObjIDs.end()) return true;
		if (_tracePIDs.find(pid) != _tracePIDs.end()) return true;
		if (_traceClasses.find(ucclass) != _traceClasses.end()) return true;
		return false;
	}

public:
	bool trace_event() const {
		return (_tracingEnabled && (_traceAll || _traceEvents));
	}
#endif
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
