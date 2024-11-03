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

#ifndef AGS_ENGINE_SCRIPT_CC_INSTANCE_H
#define AGS_ENGINE_SCRIPT_CC_INSTANCE_H

#include "common/std/memory.h"
#include "common/std/map.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/script/cc_internal.h"
#include "ags/shared/script/cc_script.h"  // ccScript
#include "ags/engine/script/non_blocking_script_function.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using namespace AGS;

#define INSTF_SHAREDATA     1
#define INSTF_ABORTED       2
#define INSTF_FREE          4
#define INSTF_RUNNING       8   // set by main code to confirm script isn't stuck

// Size of stack in RuntimeScriptValues (aka distinct variables)
#define CC_STACK_SIZE		256
// Size of stack in bytes (raw data storage)
#define CC_STACK_DATA_SIZE	(1024 * sizeof(int32_t))
#define MAX_CALL_STACK		128
#define MAX_FUNCTION_PARAMS	20

// 256 because we use 8 bits to hold instance number
#define MAX_LOADED_INSTANCES 256

#define INSTANCE_ID_SHIFT 24LL
#define INSTANCE_ID_MASK  0x00000000000000ffLL
#define INSTANCE_ID_REMOVEMASK 0x0000000000ffffffLL

// Script executor debugging flag:
// enables mistake checks, but slows things down!
#ifndef DEBUG_CC_EXEC
#define DEBUG_CC_EXEC (0)
#endif

struct ScriptInstruction {
	ScriptInstruction() = default;
	ScriptInstruction(int code, int instid) : Code(code), InstanceId(instid) {}

	int32_t Code = 0;
	int32_t InstanceId = 0;
};

struct ScriptOperation {
	ScriptInstruction   Instruction;
	RuntimeScriptValue  Args[MAX_SCMD_ARGS];
	int                 ArgCount = 0;

	// Helper functions for clarity of intent:
	// returns argN, 1-based
	inline const RuntimeScriptValue &Arg1() const { return Args[0]; }
	inline const RuntimeScriptValue &Arg2() const { return Args[1]; }
	inline const RuntimeScriptValue &Arg3() const { return Args[2]; }
	// returns argN as a integer literal
	inline int Arg1i() const { return Args[0].IValue; }
	inline int Arg2i() const { return Args[1].IValue; }
	inline int Arg3i() const { return Args[2].IValue; }
};

struct ScriptVariable {
	ScriptVariable() {
		ScAddress = -1; // address = 0 is valid one, -1 means undefined
	}

	int32_t             ScAddress;  // original 32-bit relative data address, written in compiled script;
	// if we are to use Map or HashMap, this could be used as Key
	RuntimeScriptValue  RValue;
};

struct FunctionCallStack;

struct ScriptPosition {
	ScriptPosition()
		: Line(0) {
	}

	ScriptPosition(const Shared::String &section, int32_t line)
		: Section(section)
		, Line(line) {
	}

	Shared::String  Section;
	int32_t         Line;
};


// Running instance of the script
struct ccInstance {
public:
	typedef std::unordered_map<int32_t, ScriptVariable> ScVarMap;
	typedef std::shared_ptr<ScVarMap>                   PScVarMap;
public:
	int32_t flags;
	PScVarMap globalvars;
	char *globaldata;
	int32_t globaldatasize;
	// Executed byte-code. Unlike ccScript's code array which is int32_t, the one
	// in ccInstance must be intptr_t to accommodate real pointers placed after
	// performing fixups.
	intptr_t *code;
	ccInstance *runningInst;  // might point to another instance if in far call
	int32_t codesize;
	char *strings;
	int32_t stringssize;
	RuntimeScriptValue *exports;
	RuntimeScriptValue *stack;
	int  num_stackentries;
	// An array for keeping stack data; stack entries reference unknown data from here
	// TODO: probably change to dynamic array later
	char *stackdata;    // for storing stack data of unknown type
	char *stackdata_ptr;// works similar to original stack pointer, points to the next unused byte in stack data array
	int32_t stackdatasize; // conventional size of stack data in bytes
	//
	RuntimeScriptValue registers[CC_NUM_REGISTERS];
	int32_t pc;                     // program counter
	int32_t line_number;            // source code line number
	PScript instanceof;
	int  loadedInstanceId;
	int  returnValue;

	int  callStackSize;
	int32_t callStackLineNumber[MAX_CALL_STACK];
	int32_t callStackAddr[MAX_CALL_STACK];
	ccInstance *callStackCodeInst[MAX_CALL_STACK];

	// array of real import indexes used in script
	uint32_t *resolved_imports;
	int  numimports;

	char *code_fixups;

	// returns the currently executing instance, or NULL if none
	static ccInstance *GetCurrentInstance(void);
	// clears recorded stack of current instances
	// FIXME: reimplement this in a safer way, this must be done automatically
	// when destroying all script instances, e.g. on game quit.
	static void FreeInstanceStack();
	// create a runnable instance of the supplied script
	static ccInstance *CreateFromScript(PScript script);
	static ccInstance *CreateEx(PScript scri, const ccInstance *joined);
	static void SetExecTimeout(unsigned sys_poll_ms, unsigned abort_ms, unsigned abort_loops);

	ccInstance();
	~ccInstance();
	// Create a runnable instance of the same script, sharing global memory
	ccInstance *Fork();
	// Specifies that when the current function returns to the script, it
	// will stop and return from CallInstance
	void    Abort();
	// Aborts instance, then frees the memory later when it is done with
	void    AbortAndDestroy();

	// Call an exported function in the script
	int     CallScriptFunction(const char *funcname, int32_t num_params, const RuntimeScriptValue *params);

	// Get the script's execution position and callstack as human-readable text
	Shared::String GetCallStack(int max_lines = INT_MAX) const;
	// Get the script's execution position
	void    GetScriptPosition(ScriptPosition &script_pos) const;
	// Get the address of an exported symbol (function or variable) in the script
	RuntimeScriptValue GetSymbolAddress(const char *symname) const;
	void    DumpInstruction(const ScriptOperation &op) const;
	// Tells whether this instance is in the process of executing the byte-code
	bool    IsBeingRun() const;
	// Notifies that the game was being updated (script not hanging)
	void	NotifyAlive();

	// For each import, find the instance that corresponds to it and save it
	// in resolved_imports[]. Return whether the function is successful
	bool    ResolveScriptImports(const ccScript *scri);

	// Using resolved_imports[], resolve the IMPORT fixups
	// Also change CALLEXT op-codes to CALLAS when they pertain to a script instance
	bool    ResolveImportFixups(const ccScript *scri);

private:
	bool    _Create(PScript scri, const ccInstance *joined);
	// free the memory associated with the instance
	void    Free();

	bool    CreateGlobalVars(const ccScript *scri);
	bool    AddGlobalVar(const ScriptVariable &glvar);
	ScriptVariable *FindGlobalVar(int32_t var_addr);
	bool    CreateRuntimeCodeFixups(const ccScript *scri);

	// Begin executing script starting from the given bytecode index
	int     Run(int32_t curpc);

	// Stack processing
	// Push writes new value and increments stack ptr;
	// stack ptr now points to the __next empty__ entry
	void    PushValueToStack(const RuntimeScriptValue &rval);
	void    PushDataToStack(int32_t num_bytes);
	// Pop decrements stack ptr, returns last stored value and invalidates! stack tail;
	// stack ptr now points to the __next empty__ entry
	RuntimeScriptValue PopValueFromStack();
	// helper function to pop & dump several values
	void    PopValuesFromStack(int32_t num_entries);
	void    PopDataFromStack(int32_t num_bytes);
	// Return stack ptr at given offset from stack tail;
	// Offset is in data bytes; program stack ptr is __not__ changed
	RuntimeScriptValue GetStackPtrOffsetRw(int32_t rw_offset);

	// Function call stack processing
	void    PushToFuncCallStack(FunctionCallStack &func_callstack, const RuntimeScriptValue &rval);
	void    PopFromFuncCallStack(FunctionCallStack &func_callstack, int32_t num_entries);

	// Last time the script was noted of being "alive"
	AGS_Clock::time_point _lastAliveTs;
};

extern void script_commands_init();
extern void script_commands_free();

} // namespace AGS3

#endif
