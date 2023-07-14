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

#include "common/debug-channels.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include "ags/engine/ac/dynobj/managed_object_pool.h"
#include "ags/shared/gui/gui_defines.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/script/system_imports.h"
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/text_stream_writer.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/ac/dynobj/script_user_object.h"
#include "ags/engine/ac/statobj/ags_static_object.h"
#include "ags/engine/ac/statobj/static_array.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object_addr_and_manager.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/util/string_utils.h" // linux strnicmp definition
#include "ags/detection.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Shared::Memory;

enum ScriptOpArgIsReg {
	kScOpNoArgIsReg     = 0,
	kScOpArg1IsReg      = 0x0001,
	kScOpArg2IsReg      = 0x0002,
	kScOpArg3IsReg      = 0x0004,
	kScOpOneArgIsReg    = kScOpArg1IsReg,
	kScOpTwoArgsAreReg  = kScOpArg1IsReg | kScOpArg2IsReg,
	kScOpTreeArgsAreReg = kScOpArg1IsReg | kScOpArg2IsReg | kScOpArg3IsReg
};

struct ScriptCommandInfo {
	ScriptCommandInfo(int32_t code, const char *cmdname, int arg_count, ScriptOpArgIsReg arg_is_reg) {
		Code        = code;
		CmdName     = cmdname;
		ArgCount    = arg_count;
		ArgIsReg[0] = (arg_is_reg & kScOpArg1IsReg) != 0;
		ArgIsReg[1] = (arg_is_reg & kScOpArg2IsReg) != 0;
		ArgIsReg[2] = (arg_is_reg & kScOpArg3IsReg) != 0;
	}

	int32_t             Code;
	const char          *CmdName;
	int                 ArgCount;
	bool                ArgIsReg[3];
};

struct ScriptCommands {
	const ScriptCommandInfo _items[CC_NUM_SCCMDS] = {
		ScriptCommandInfo(0                    , "NULL"              , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_ADD             , "addi"              , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_SUB             , "subi"              , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_REGTOREG        , "mov"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_WRITELIT        , "memwritelit"       , 2, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_RET             , "ret"               , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_LITTOREG        , "movl"              , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMREAD         , "memread4"          , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMWRITE        , "memwrite4"         , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MULREG          , "mul"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_DIVREG          , "div"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_ADDREG          , "add"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_SUBREG          , "sub"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_BITAND          , "and"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_BITOR           , "or"                , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_ISEQUAL         , "cmpeq"             , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_NOTEQUAL        , "cmpne"             , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_GREATER         , "gt"                , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_LESSTHAN        , "lt"                , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_GTE             , "gte"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_LTE             , "lte"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_AND             , "land"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_OR              , "lor"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_CALL            , "call"              , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMREADB        , "memread1"          , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMREADW        , "memread2"          , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMWRITEB       , "memwrite1"         , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMWRITEW       , "memwrite2"         , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_JZ              , "jzi"               , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_PUSHREG         , "push"              , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_POPREG          , "pop"               , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_JMP             , "jmpi"              , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_MUL             , "muli"              , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_CALLEXT         , "farcall"           , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_PUSHREAL        , "farpush"           , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_SUBREALSTACK    , "farsubsp"          , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_LINENUM         , "sourceline"        , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_CALLAS          , "callscr"           , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_THISBASE        , "thisaddr"          , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_NUMFUNCARGS     , "setfuncargs"       , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_MODREG          , "mod"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_XORREG          , "xor"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_NOTREG          , "not"               , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_SHIFTLEFT       , "shl"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_SHIFTRIGHT      , "shr"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_CALLOBJ         , "callobj"           , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_CHECKBOUNDS     , "checkbounds"       , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMWRITEPTR     , "memwrite.ptr"      , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMREADPTR      , "memread.ptr"       , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_MEMZEROPTR      , "memwrite.ptr.0"    , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_MEMINITPTR      , "meminit.ptr"       , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_LOADSPOFFS      , "load.sp.offs"      , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_CHECKNULL       , "checknull.ptr"     , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_FADD            , "faddi"             , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_FSUB            , "fsubi"             , 2, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_FMULREG         , "fmul"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FDIVREG         , "fdiv"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FADDREG         , "fadd"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FSUBREG         , "fsub"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FGREATER        , "fgt"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FLESSTHAN       , "flt"               , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FGTE            , "fgte"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_FLTE            , "flte"              , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_ZEROMEMORY      , "zeromem"           , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_CREATESTRING    , "newstring"         , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_STRINGSEQUAL    , "streq"             , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_STRINGSNOTEQ    , "strne"             , 2, kScOpTwoArgsAreReg),
		ScriptCommandInfo(SCMD_CHECKNULLREG    , "checknull"         , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_LOOPCHECKOFF    , "loopcheckoff"      , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_MEMZEROPTRND    , "memwrite.ptr.0.nd" , 0, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_JNZ             , "jnzi"              , 1, kScOpNoArgIsReg),
		ScriptCommandInfo(SCMD_DYNAMICBOUNDS   , "dynamicbounds"     , 1, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_NEWARRAY        , "newarray"          , 3, kScOpOneArgIsReg),
		ScriptCommandInfo(SCMD_NEWUSEROBJECT   , "newuserobject"     , 2, kScOpOneArgIsReg),
	};

	const ScriptCommandInfo &operator[](uint idx) {
		return _items[idx];
	}
};
static ScriptCommands *g_commands;

void script_commands_init() {
	g_commands = new ScriptCommands();
}

void script_commands_free() {
	delete g_commands;
}

const char *regnames[] = { "null", "sp", "mar", "ax", "bx", "cx", "op", "dx" };

const char *fixupnames[] = { "null", "fix_gldata", "fix_func", "fix_string", "fix_import", "fix_datadata", "fix_stack" };

String cc_get_callstack(int max_lines) {
	String callstack;
	for (auto sci = _GP(InstThreads).crbegin(); sci != _GP(InstThreads).crend(); ++sci) {
		if (callstack.IsEmpty())
			callstack.Append("in the active script:\n");
		else
			callstack.Append("in the waiting script:\n");
		callstack.Append((*sci)->GetCallStack(max_lines));
	}
	return callstack;
}

// Function call stack is used to temporarily store
// values before passing them to script function
#define MAX_FUNC_PARAMS 20
// An inverted parameter stack
struct FunctionCallStack {
	FunctionCallStack() {
		Head = MAX_FUNC_PARAMS - 1;
		Count = 0;
	}

	inline RuntimeScriptValue *GetHead() {
		return &Entries[Head];
	}
	inline RuntimeScriptValue *GetTail() {
		return &Entries[Head + Count];
	}

	RuntimeScriptValue  Entries[MAX_FUNC_PARAMS + 1];
	int                 Head;
	int                 Count;
};

ccInstance *ccInstance::GetCurrentInstance() {
	return _GP(InstThreads).size() > 0 ? _GP(InstThreads).back() : nullptr;
}

ccInstance *ccInstance::CreateFromScript(PScript scri) {
	return CreateEx(scri, nullptr);
}

ccInstance *ccInstance::CreateEx(PScript scri, ccInstance *joined) {
	// allocate and copy all the memory with data, code and strings across
	ccInstance *cinst = new ccInstance();
	if (!cinst->_Create(scri, joined)) {
		delete cinst;
		return nullptr;
	}

	return cinst;
}

void ccInstance::SetExecTimeout(unsigned sys_poll_ms, unsigned abort_ms, unsigned abort_loops) {
	_G(timeoutCheckMs) = sys_poll_ms;
	_G(timeoutAbortMs) = abort_ms;
	_G(maxWhileLoops) = abort_loops;
}

ccInstance::ccInstance() {
	flags               = 0;
	globaldata          = nullptr;
	globaldatasize      = 0;
	code                = nullptr;
	runningInst         = nullptr;
	codesize            = 0;
	strings             = nullptr;
	stringssize         = 0;
	exports             = nullptr;
	stack               = nullptr;
	num_stackentries    = 0;
	stackdata           = nullptr;
	stackdatasize       = 0;
	stackdata_ptr       = nullptr;
	pc                  = 0;
	line_number         = 0;
	callStackSize       = 0;
	loadedInstanceId    = 0;
	returnValue         = 0;
	numimports = 0;
	resolved_imports = nullptr;
	code_fixups         = nullptr;

	memset(callStackLineNumber, 0, sizeof(callStackLineNumber));
	memset(callStackAddr, 0, sizeof(callStackAddr));
	memset(callStackCodeInst, 0, sizeof(callStackCodeInst));
}

ccInstance::~ccInstance() {
	Free();
}

ccInstance *ccInstance::Fork() {
	return CreateEx(instanceof, this);
}

void ccInstance::Abort() {
	if (pc != 0) {
		flags |= INSTF_ABORTED;
	}
}

void ccInstance::AbortAndDestroy() {
	Abort();
	flags |= INSTF_FREE;
}

#define ASSERT_STACK_SPACE_AVAILABLE(N) \
	if (registers[SREG_SP].RValue + N - &stack[0] >= CC_STACK_SIZE) \
	{ \
		cc_error("stack overflow"); \
		return -1; \
	}

#define ASSERT_STACK_SIZE(N) \
	if (registers[SREG_SP].RValue - N < &stack[0]) \
	{ \
		cc_error("stack underflow"); \
		return -1; \
	}

int ccInstance::CallScriptFunction(const char *funcname, int32_t numargs, const RuntimeScriptValue *params) {
	cc_clear_error();
	_G(currentline) = 0;

	if (numargs > 0 && !params) {
		cc_error("internal error in ccInstance::CallScriptFunction");
		return -1; // TODO: correct error value
	}

	if ((numargs >= 20) || (numargs < 0)) {
		cc_error("too many arguments to function");
		return -3;
	}

	if (pc != 0) {
		cc_error("instance already being executed");
		return -4;
	}

	// NOTE: passing more parameters than expected by the function is fine:
	// the function args are pushed to the stack in REVERSE order, first
	// parameters are always the last, so function code knows how to find them
	// using negative offsets, and does not care about any preceding entries.
	int32_t startat = -1;
	char mangledName[200];
	size_t mangled_len = snprintf(mangledName, sizeof(mangledName), "%s$", funcname);
	int32_t export_args = numargs;

	for (int k = 0; k < instanceof->numexports; k++) {
		char *thisExportName = instanceof->exports[k];
		bool match = false;

		// check for a mangled name match
		if (strncmp(thisExportName, mangledName, mangled_len) == 0) {
			// found, compare the number of parameters
			export_args = atoi(thisExportName + mangled_len);
			if (export_args > numargs) {
				cc_error("Not enough parameters to exported function '%s' (expected %d, supplied %d)",
					funcname, export_args, numargs);
				return -1;
			}
			match = true;
		}
		// check for an exact match (if the script was compiled with an older version)
		if (match || (strcmp(thisExportName, funcname) == 0)) {
			int32_t etype = (instanceof->export_addr[k] >> 24L) & 0x000ff;
			if (etype != EXPORT_FUNCTION) {
				cc_error("symbol is not a function");
				return -1;
			}
			startat = (instanceof->export_addr[k] & 0x00ffffff);
			break;
		}
	}

	if (startat < 0) {
		cc_error("function '%s' not found", funcname);
		return -2;
	}

	// Prepare instance for run
	flags &= ~INSTF_ABORTED;
	// Allow to pass less parameters if script callback has less declared args
	numargs = MIN(numargs, export_args);
	// object pointer needs to start zeroed
	registers[SREG_OP].SetDynamicObject(nullptr, nullptr);
	registers[SREG_SP].SetStackPtr(&stack[0]);
	stackdata_ptr = stackdata;
	// NOTE: Pushing parameters to stack in reverse order
	ASSERT_STACK_SPACE_AVAILABLE(numargs + 1 /* return address */)
		for (int i = numargs - 1; i >= 0; --i) {
			PushValueToStack(params[i]);
		}
	PushValueToStack(RuntimeScriptValue().SetInt32(0)); // return address on stack

	_GP(InstThreads).push_back(this); // push instance thread
	runningInst = this;
	int reterr = Run(startat);
	// Cleanup before returning, even if error
	ASSERT_STACK_SIZE(numargs);
	PopValuesFromStack(numargs);
	pc = 0;
	_G(currentline) = 0;
	_GP(InstThreads).pop_back(); // pop instance thread
	if (reterr != 0)
		return reterr;

	// NOTE that if proper multithreading is added this will need
	// to be reconsidered, since the GC could be run in the middle
	// of a RET from a function or something where there is an
	// object with ref count 0 that is in use
	_GP(pool).RunGarbageCollectionIfAppropriate();

	if (_G(new_line_hook))
		_G(new_line_hook)(nullptr, 0);

	if (flags & INSTF_ABORTED) {
		flags &= ~INSTF_ABORTED;

		if (flags & INSTF_FREE)
			Free();
		return 100;
	}

	if (registers[SREG_SP].RValue != &stack[0]) {
		cc_error("stack pointer was not zero at completion of script");
		return -5;
	}
	return cc_has_error();
}

// Macros to maintain the call stack
#define PUSH_CALL_STACK \
	if (callStackSize >= MAX_CALL_STACK) { \
		cc_error("CallScriptFunction stack overflow (recursive call error?)"); \
		return -1; \
	} \
	callStackLineNumber[callStackSize] = line_number;  \
	callStackCodeInst[callStackSize] = runningInst;  \
	callStackAddr[callStackSize] = pc;  \
	callStackSize++

#define POP_CALL_STACK \
	if (callStackSize < 1) { \
		cc_error("CallScriptFunction stack underflow -- internal error"); \
		return -1; \
	} \
	callStackSize--;\
	line_number = callStackLineNumber[callStackSize];\
	_G(currentline) = line_number

#define MAXNEST 50  // number of recursive function calls allowed
int ccInstance::Run(int32_t curpc) {
	pc = curpc;
	returnValue = -1;

	if ((curpc < 0) || (curpc >= runningInst->codesize)) {
		cc_error("specified code offset is not valid");
		return -1;
	}

	int32_t thisbase[MAXNEST], funcstart[MAXNEST];
	int was_just_callas = -1;
	int curnest = 0;
	int num_args_to_func = -1;
	int next_call_needs_object = 0;
	thisbase[0] = 0;
	funcstart[0] = pc;
	ccInstance *codeInst = runningInst;
	bool write_debug_dump = ccGetOption(SCOPT_DEBUGRUN) ||
		(gDebugLevel > 0 && DebugMan.isDebugChannelEnabled(::AGS::kDebugScript));
	ScriptOperation codeOp;
	FunctionCallStack func_callstack;
	int loopIterationCheckDisabled = 0;
	unsigned loopIterations = 0u;      // any loop iterations (needed for timeout test)
	unsigned loopCheckIterations = 0u; // loop iterations accumulated only if check is enabled

	const auto timeout = std::chrono::milliseconds(_G(timeoutCheckMs));
	// NOTE: removed timeout_abort check for now: was working *logically* wrong;
	//const auto timeout_abort = std::chrono::milliseconds(_G(timeoutAbortMs));
	_lastAliveTs = AGS_Clock::now();

	while ((flags & INSTF_ABORTED) == 0) {
		if (_G(abort_engine))
			return -1;

		/*
		if (!codeInst->ReadOperation(codeOp, pc))
		{
		    return -1;
		}
		*/
		/* ReadOperation */
		//=====================================================================
		codeOp.Instruction.Code         = codeInst->code[pc];
		codeOp.Instruction.InstanceId   = (codeOp.Instruction.Code >> INSTANCE_ID_SHIFT) & INSTANCE_ID_MASK;
		codeOp.Instruction.Code        &= INSTANCE_ID_REMOVEMASK; // now this is pure instruction code

		if (codeOp.Instruction.Code < 0 || codeOp.Instruction.Code >= CC_NUM_SCCMDS) {
			cc_error("invalid instruction %d found in code stream", codeOp.Instruction.Code);
			return -1;
		}

		codeOp.ArgCount = (*g_commands)[codeOp.Instruction.Code].ArgCount;
		if (pc + codeOp.ArgCount >= codeInst->codesize) {
			cc_error("unexpected end of code data (%d; %d)", pc + codeOp.ArgCount, codeInst->codesize);
			return -1;
		}

		int pc_at = pc + 1;
		for (int i = 0; i < codeOp.ArgCount; ++i, ++pc_at) {
			char fixup = codeInst->code_fixups[pc_at];
			if (fixup > 0) {
				// could be relative pointer or import address
				/*
				if (!FixupArgument(code[pc], fixup, codeOp.Args[i]))
				{
				    return -1;
				}
				*/
				/* FixupArgument */
				//=====================================================================
				switch (fixup) {
				case FIXUP_GLOBALDATA: {
					ScriptVariable *gl_var = (ScriptVariable *)codeInst->code[pc_at];
					codeOp.Args[i].SetGlobalVar(&gl_var->RValue);
				}
				break;
				case FIXUP_FUNCTION:
					// originally commented -- CHECKME: could this be used in very old versions of AGS?
					//      code[fixup] += (long)&code[0];
					// This is a program counter value, presumably will be used as SCMD_CALL argument
					codeOp.Args[i].SetInt32((int32_t)codeInst->code[pc_at]);
					break;
				case FIXUP_STRING:
					codeOp.Args[i].SetStringLiteral(&codeInst->strings[0] + codeInst->code[pc_at]);
					break;
				case FIXUP_IMPORT: {
					const ScriptImport *import = _GP(simp).getByIndex(static_cast<uint32_t>(codeInst->code[pc_at]));
					if (import) {
						codeOp.Args[i] = import->Value;
					} else {
						cc_error("cannot resolve import, key = %ld", codeInst->code[pc_at]);
						return -1;
					}
				}
				break;
				case FIXUP_STACK:
					codeOp.Args[i] = GetStackPtrOffsetFw((int32_t)codeInst->code[pc_at]);
					break;
				default:
					cc_error("internal fixup type error: %d", fixup);
					return -1;
				}
				/* End FixupArgument */
				//=====================================================================
			} else {
				// should be a numeric literal (int32 or float)
				codeOp.Args[i].SetInt32((int32_t)codeInst->code[pc_at]);
			}
		}
		/* End ReadOperation */
		//=====================================================================

		// save the arguments for quick access
		RuntimeScriptValue &arg1 = codeOp.Args[0];
		RuntimeScriptValue &arg2 = codeOp.Args[1];
		RuntimeScriptValue &arg3 = codeOp.Args[2];
		RuntimeScriptValue &reg1 =
		    registers[arg1.IValue >= 0 && arg1.IValue < CC_NUM_REGISTERS ? arg1.IValue : 0];
		RuntimeScriptValue &reg2 =
		    registers[arg2.IValue >= 0 && arg2.IValue < CC_NUM_REGISTERS ? arg2.IValue : 0];

		const char *direct_ptr1;
		const char *direct_ptr2;

		if (write_debug_dump) {
			DumpInstruction(codeOp);
		}

		switch (codeOp.Instruction.Code) {
		case SCMD_LINENUM:
			line_number = arg1.IValue;
			_G(currentline) = arg1.IValue;
			if (_G(new_line_hook))
				_G(new_line_hook)(this, _G(currentline));
			break;
		case SCMD_ADD:
			// If the register is SREG_SP, we are allocating new variable on the stack
			if (arg1.IValue == SREG_SP) {
				// Only allocate new data if current stack entry is invalid;
				// in some cases this may be advancing over value that was written by MEMWRITE*
				ASSERT_STACK_SPACE_AVAILABLE(1);
				if (reg1.RValue->IsValid()) {
					// TODO: perhaps should add a flag here to ensure this happens only after MEMWRITE-ing to stack
					registers[SREG_SP].RValue++;
				} else {
					PushDataToStack(arg2.IValue);
					if (cc_has_error()) {
						return -1;
					}
				}
			} else {
				reg1.IValue += arg2.IValue;
			}
			break;
		case SCMD_SUB:
			if (reg1.Type == kScValStackPtr) {
				// If this is SREG_SP, this is stack pop, which frees local variables;
				// Other than SREG_SP this may be AGS 2.x method to offset stack in SREG_MAR;
				// quote JJS:
				// // AGS 2.x games also perform relative stack access by copying SREG_SP to SREG_MAR
				// // and then subtracting from that.
				if (arg1.IValue == SREG_SP) {
					PopDataFromStack(arg2.IValue);
				} else {
					// This is practically LOADSPOFFS
					reg1 = GetStackPtrOffsetRw(arg2.IValue);
				}
				if (cc_has_error()) {
					return -1;
				}
			} else {
				reg1.IValue -= arg2.IValue;
			}
			break;
		case SCMD_REGTOREG:
			reg2 = reg1;
			break;
		case SCMD_WRITELIT:
			// Take the data address from reg[MAR] and copy there arg1 bytes from arg2 address
			//
			// NOTE: since it reads directly from arg2 (which originally was
			// long, or rather int32 due x32 build), written value may normally
			// be only up to 4 bytes large;
			// I guess that's an obsolete way to do WRITE, WRITEW and WRITEB
			switch (arg1.IValue) {
			case sizeof(char):
				registers[SREG_MAR].WriteByte(arg2.IValue);
				break;
			case sizeof(int16_t):
				registers[SREG_MAR].WriteInt16(arg2.IValue);
				break;
			case sizeof(int32_t):
				// We do not know if this is math integer or some pointer, etc
				registers[SREG_MAR].WriteValue(arg2);
				break;
			default:
				warning("unexpected data size for WRITELIT op: %d", arg1.IValue);
				break;
			}
			break;
		case SCMD_RET: {
			if (loopIterationCheckDisabled > 0)
				loopIterationCheckDisabled--;

			ASSERT_STACK_SIZE(1);
			RuntimeScriptValue rval = PopValueFromStack();
			curnest--;
			pc = rval.IValue;
			if (pc == 0) {
				returnValue = registers[SREG_AX].IValue;
				return 0;
			}
			POP_CALL_STACK;
			continue; // continue so that the PC doesn't get overwritten
		}
		case SCMD_LITTOREG:
			reg1 = arg2;
			break;
		case SCMD_MEMREAD:
			// Take the data address from reg[MAR] and copy int32_t to reg[arg1]
			reg1 = registers[SREG_MAR].ReadValue();
			break;
		case SCMD_MEMWRITE:
			// Take the data address from reg[MAR] and copy there int32_t from reg[arg1]
			registers[SREG_MAR].WriteValue(reg1);
			break;
		case SCMD_LOADSPOFFS:
			registers[SREG_MAR] = GetStackPtrOffsetRw(arg1.IValue);
			if (cc_has_error()) {
				return -1;
			}
			break;

		// 64 bit: Force 32 bit math
		case SCMD_MULREG:
			reg1.SetInt32(reg1.IValue * reg2.IValue);
			break;
		case SCMD_DIVREG:
			if (reg2.IValue == 0) {
				cc_error("!Integer divide by zero");
				return -1;
			}
			reg1.SetInt32(reg1.IValue / reg2.IValue);
			break;
		case SCMD_ADDREG:
			// This may be pointer arithmetics, in which case IValue stores offset from base pointer
			reg1.IValue += reg2.IValue;
			break;
		case SCMD_SUBREG:
			// This may be pointer arithmetics, in which case IValue stores offset from base pointer
			reg1.IValue -= reg2.IValue;
			break;
		case SCMD_BITAND:
			reg1.SetInt32(reg1.IValue & reg2.IValue);
			break;
		case SCMD_BITOR:
			reg1.SetInt32(reg1.IValue | reg2.IValue);
			break;
		case SCMD_ISEQUAL:
			reg1.SetInt32AsBool(reg1 == reg2);
			break;
		case SCMD_NOTEQUAL:
			reg1.SetInt32AsBool(reg1 != reg2);
			break;
		case SCMD_GREATER:
			reg1.SetInt32AsBool(reg1.IValue > reg2.IValue);
			break;
		case SCMD_LESSTHAN:
			reg1.SetInt32AsBool(reg1.IValue < reg2.IValue);
			break;
		case SCMD_GTE:
			reg1.SetInt32AsBool(reg1.IValue >= reg2.IValue);
			break;
		case SCMD_LTE:
			reg1.SetInt32AsBool(reg1.IValue <= reg2.IValue);
			break;
		case SCMD_AND:
			reg1.SetInt32AsBool(reg1.IValue && reg2.IValue);
			break;
		case SCMD_OR:
			reg1.SetInt32AsBool(reg1.IValue || reg2.IValue);
			break;
		case SCMD_XORREG:
			reg1.SetInt32(reg1.IValue ^ reg2.IValue);
			break;
		case SCMD_MODREG:
			if (reg2.IValue == 0) {
				cc_error("!Integer divide by zero");
				return -1;
			}
			reg1.SetInt32(reg1.IValue % reg2.IValue);
			break;
		case SCMD_NOTREG:
			reg1 = !(reg1);
			break;
		case SCMD_CALL:
			// Call another function within same script, just save PC
			// and continue from there
			if (curnest >= MAXNEST - 1) {
				cc_error("!call stack overflow, recursive call problem?");
				return -1;
			}

			PUSH_CALL_STACK;

			ASSERT_STACK_SPACE_AVAILABLE(1);
			PushValueToStack(RuntimeScriptValue().SetInt32(pc + codeOp.ArgCount + 1));

			if (thisbase[curnest] == 0)
				pc = reg1.IValue;
			else {
				pc = funcstart[curnest];
				pc += (reg1.IValue - thisbase[curnest]);
			}

			next_call_needs_object = 0;

			if (loopIterationCheckDisabled)
				loopIterationCheckDisabled++;

			curnest++;
			thisbase[curnest] = 0;
			funcstart[curnest] = pc;
			continue; // continue so that the PC doesn't get overwritten
		case SCMD_MEMREADB:
			// Take the data address from reg[MAR] and copy byte to reg[arg1]
			reg1.SetUInt8(registers[SREG_MAR].ReadByte());
			break;
		case SCMD_MEMREADW:
			// Take the data address from reg[MAR] and copy int16_t to reg[arg1]
			reg1.SetInt16(registers[SREG_MAR].ReadInt16());
			break;
		case SCMD_MEMWRITEB:
			// Take the data address from reg[MAR] and copy there byte from reg[arg1]
			registers[SREG_MAR].WriteByte(reg1.IValue);
			break;
		case SCMD_MEMWRITEW:
			// Take the data address from reg[MAR] and copy there int16_t from reg[arg1]
			registers[SREG_MAR].WriteInt16(reg1.IValue);
			break;
		case SCMD_JZ:
			if (registers[SREG_AX].IsNull())
				pc += arg1.IValue;
			break;
		case SCMD_JNZ:
			if (!registers[SREG_AX].IsNull())
				pc += arg1.IValue;
			break;
		case SCMD_PUSHREG:
			// Push reg[arg1] value to the stack
			ASSERT_STACK_SPACE_AVAILABLE(1);
			PushValueToStack(reg1);
			break;
		case SCMD_POPREG:
			ASSERT_STACK_SIZE(1);
			reg1 = PopValueFromStack();
			break;
		case SCMD_JMP:
			pc += arg1.IValue;

			// Make sure it's not stuck in a While loop
			if (arg1.IValue < 0) {
				++loopIterations;
				if (flags & INSTF_RUNNING) {
					// was notified still running, don't do anything
					flags &= ~INSTF_RUNNING;
					loopIterations = 0u;
					loopCheckIterations = 0u;
				} else if ((loopIterationCheckDisabled == 0) && (_G(maxWhileLoops) > 0) && (++loopIterations > _G(maxWhileLoops))) {
					cc_error("!Script appears to be hung (a while loop ran %d times). The problem may be in a calling function; check the call stack.", (int)loopCheckIterations);
					return -1;
				} else if ((loopIterations & 0x3FF) == 0 && // test each 1024 loops (arbitrary)
						   (std::chrono::duration_cast<std::chrono::milliseconds>(AGS_Clock::now() - _lastAliveTs) > timeout)) {
					// minimal timeout occurred
					// NOTE: removed timeout_abort check for now: was working *logically* wrong;
					sys_evt_process_pending();
					_lastAliveTs = AGS_Clock::now();
				}
			}
			break;
		case SCMD_MUL:
			reg1.IValue *= arg2.IValue;
			break;
		case SCMD_CHECKBOUNDS:
			if ((reg1.IValue < 0) ||
			        (reg1.IValue >= arg2.IValue)) {
				cc_error("!Array index out of bounds (index: %d, bounds: 0..%d)", reg1.IValue, arg2.IValue - 1);
				return -1;
			}
			break;
		case SCMD_DYNAMICBOUNDS: {
			// TODO: test reg[MAR] type here;
			// That might be dynamic object, but also a non-managed dynamic array, "allocated"
			// on global or local memspace (buffer)
			int32_t upperBoundInBytes = *((int32_t *)(registers[SREG_MAR].GetPtrWithOffset() - 4));
			if ((reg1.IValue < 0) ||
			        (reg1.IValue >= upperBoundInBytes)) {
				int32_t upperBound = *((int32_t *)(registers[SREG_MAR].GetPtrWithOffset() - 8)) & (~ARRAY_MANAGED_TYPE_FLAG);
				if (upperBound <= 0) {
					cc_error("!Array has an invalid size (%d) and cannot be accessed", upperBound);
				} else {
					int elementSize = (upperBoundInBytes / upperBound);
					cc_error("!Array index out of bounds (index: %d, bounds: 0..%d)", reg1.IValue / elementSize, upperBound - 1);
				}
				return -1;
			}
			break;
		}

		// 64 bit: Handles are always 32 bit values. They are not C pointer.

		case SCMD_MEMREADPTR: {
			cc_clear_error();

			int32_t handle = registers[SREG_MAR].ReadInt32();
			void *object;
			ICCDynamicObject *manager;
			ScriptValueType obj_type = ccGetObjectAddressAndManagerFromHandle(handle, object, manager);
			if (obj_type == kScValPluginObject) {
				reg1.SetPluginObject(object, manager);
			} else {
				reg1.SetDynamicObject(object, manager);
			}

			// if error occurred, cc_error will have been set
			if (cc_has_error())
				return -1;
			break;
		}
		case SCMD_MEMWRITEPTR: {

			int32_t handle = registers[SREG_MAR].ReadInt32();
			const char *address = nullptr;

			if (reg1.Type == kScValStaticArray && reg1.StcArr->GetDynamicManager()) {
				address = (const char *)reg1.StcArr->GetElementPtr(reg1.Ptr, reg1.IValue);
			} else if (reg1.Type == kScValDynamicObject ||
			           reg1.Type == kScValPluginObject) {
				address = reg1.Ptr;
			} else if (reg1.Type == kScValPluginArg) {
				// TODO: plugin API is currently strictly 32-bit, so this may break on 64-bit systems
				address = Int32ToPtr<char>(reg1.IValue);
			}
			// There's one possible case when the reg1 is 0, which means writing nullptr
			else if (!reg1.IsNull()) {
				cc_error("internal error: MEMWRITEPTR argument is not dynamic object");
				return -1;
			}

			int32_t newHandle = ccGetObjectHandleFromAddress(address);
			if (newHandle == -1)
				return -1;

			if (handle != newHandle) {
				ccReleaseObjectReference(handle);
				ccAddObjectReference(newHandle);
				registers[SREG_MAR].WriteInt32(newHandle);
			}
			break;
		}
		case SCMD_MEMINITPTR: {
			const char *address = nullptr;

			if (reg1.Type == kScValStaticArray && reg1.StcArr->GetDynamicManager()) {
				address = (const char *)reg1.StcArr->GetElementPtr(reg1.Ptr, reg1.IValue);
			} else if (reg1.Type == kScValDynamicObject ||
			           reg1.Type == kScValPluginObject) {
				address = reg1.Ptr;
			} else if (reg1.Type == kScValPluginArg) {
				// TODO: plugin API is currently strictly 32-bit, so this may break on 64-bit systems
				address = Int32ToPtr<char>(reg1.IValue);
			}
			// There's one possible case when the reg1 is 0, which means writing nullptr
			else if (!reg1.IsNull()) {
				cc_error("internal error: SCMD_MEMINITPTR argument is not dynamic object");
				return -1;
			}
			// like memwriteptr, but doesn't attempt to free the old one
			int32_t newHandle = ccGetObjectHandleFromAddress(address);
			if (newHandle == -1)
				return -1;

			ccAddObjectReference(newHandle);
			registers[SREG_MAR].WriteInt32(newHandle);
			break;
		}
		case SCMD_MEMZEROPTR: {
			int32_t handle = registers[SREG_MAR].ReadInt32();
			ccReleaseObjectReference(handle);
			registers[SREG_MAR].WriteInt32(0);
			break;
		}
		case SCMD_MEMZEROPTRND: {
			int32_t handle = registers[SREG_MAR].ReadInt32();

			// don't do the Dispose check for the object being returned -- this is
			// for returning a String (or other pointer) from a custom function.
			// Note: we might be freeing a dynamic array which contains the DisableDispose
			// object, that will be handled inside the recursive call to SubRef.
			// CHECKME!! what type of data may reg1 point to?
			_GP(pool).disableDisposeForObject = (const char *)registers[SREG_AX].Ptr;
			ccReleaseObjectReference(handle);
			_GP(pool).disableDisposeForObject = nullptr;
			registers[SREG_MAR].WriteInt32(0);
			break;
		}
		case SCMD_CHECKNULL:
			if (registers[SREG_MAR].IsNull()) {
				cc_error("!Null pointer referenced");
				return -1;
			}
			break;
		case SCMD_CHECKNULLREG:
			if (reg1.IsNull()) {
				cc_error("!Null string referenced");
				return -1;
			}
			break;
		case SCMD_NUMFUNCARGS:
			num_args_to_func = arg1.IValue;
			break;
		case SCMD_CALLAS: {
			PUSH_CALL_STACK;

			// Call to a function in another script

			// If there are nested CALLAS calls, the stack might
			// contain 2 calls worth of parameters, so only
			// push args for this call
			if (num_args_to_func < 0) {
				num_args_to_func = func_callstack.Count;
			}
			ASSERT_STACK_SPACE_AVAILABLE(num_args_to_func + 1 /* return address */);
			for (const RuntimeScriptValue *prval = func_callstack.GetHead() + num_args_to_func;
			        prval > func_callstack.GetHead(); --prval) {
				PushValueToStack(*prval);
			}

			// 0, so that the cc_run_code returns
			RuntimeScriptValue oldstack = registers[SREG_SP];
			PushValueToStack(RuntimeScriptValue().SetInt32(0));
			if (cc_has_error()) {
				return -1;
			}

			int oldpc = pc;
			ccInstance *wasRunning = runningInst;

			// extract the instance ID
			int32_t instId = codeOp.Instruction.InstanceId;
			// determine the offset into the code of the instance we want
			runningInst = _G(loadedInstances)[instId];
			intptr_t callAddr = reg1.Ptr - (char *)&runningInst->code[0];
			if (callAddr % sizeof(intptr_t) != 0) {
				cc_error("call address not aligned");
				return -1;
			}
			callAddr /= sizeof(intptr_t); // size of ccScript::code elements

			if (Run((int32_t)callAddr))
				return -1;

			runningInst = wasRunning;

			if ((flags & INSTF_ABORTED) == 0) {
				if (oldstack != registers[SREG_SP]) {
					cc_error("stack corrupt after function call");
					return -1;
				}
			}

			next_call_needs_object = 0;

			pc = oldpc;
			was_just_callas = func_callstack.Count;
			num_args_to_func = -1;
			POP_CALL_STACK;
			break;
		}
		case SCMD_CALLEXT: {
			// Call to a real 'C' code function
			was_just_callas = -1;
			if (num_args_to_func < 0) {
				num_args_to_func = func_callstack.Count;
			}

			// Convert pointer arguments to simple types
			for (RuntimeScriptValue *prval = func_callstack.GetHead() + num_args_to_func;
			        prval > func_callstack.GetHead(); --prval) {
				prval->DirectPtr();
			}

			RuntimeScriptValue return_value;

			if (reg1.Type == kScValPluginFunction) {
				_GP(GlobalReturnValue).Invalidate();
				NumberPtr fnResult;
				if (next_call_needs_object) {
					RuntimeScriptValue obj_rval = registers[SREG_OP];
					obj_rval.DirectPtrObj();
					fnResult = call_function(reg1.pluginMethod(),
						&obj_rval, num_args_to_func, func_callstack.GetHead() + 1);
				} else {
					fnResult = call_function(reg1.pluginMethod(),
						nullptr, num_args_to_func, func_callstack.GetHead() + 1);
				}

				if (_GP(GlobalReturnValue).IsValid()) {
					return_value = _GP(GlobalReturnValue);
				} else {
					// TODO: Though some plugin methods return pointers, the SetPluginArgument
					// call only supports a 32-bit value. This is fine in most cases, since
					// methods mostly set the ptr on GlobalReturnValue, so it doesn't reach here.
					// But just in case, throw a wobbly if it reaches here with a 64-bit pointer
					if (fnResult._ptr > reinterpret_cast<void *>(static_cast<uintptr>(0xffffffffu)))
						error("Uhandled 64-bit pointer result from plugin method call");

					return_value.SetPluginArgument(fnResult);
				}
			} else if (next_call_needs_object) {
				// member function call
				if (reg1.Type == kScValObjectFunction) {
					RuntimeScriptValue obj_rval = registers[SREG_OP];
					obj_rval.DirectPtrObj();
					return_value = reg1.ObjPfn(obj_rval.Ptr, func_callstack.GetHead() + 1, num_args_to_func);
				} else {
					cc_error("invalid pointer type for object function call: %d", reg1.Type);
				}
			} else if (reg1.Type == kScValStaticFunction) {
				return_value = reg1.SPfn(func_callstack.GetHead() + 1, num_args_to_func);
			} else if (reg1.Type == kScValObjectFunction) {
				cc_error("unexpected object function pointer on SCMD_CALLEXT");
			} else {
				cc_error("invalid pointer type for function call: %d", reg1.Type);
			}

			if (cc_has_error() || _G(abort_engine)) {
				return -1;
			}

			registers[SREG_AX] = return_value;
			next_call_needs_object = 0;
			num_args_to_func = -1;
			break;
		}
		case SCMD_PUSHREAL:
			PushToFuncCallStack(func_callstack, reg1);
			break;
		case SCMD_SUBREALSTACK:
			PopFromFuncCallStack(func_callstack, arg1.IValue);
			if (was_just_callas >= 0) {
				ASSERT_STACK_SIZE(arg1.IValue);
				PopValuesFromStack(arg1.IValue);
				was_just_callas = -1;
			}
			break;
		case SCMD_CALLOBJ:
			// set the OP register
			if (reg1.IsNull()) {
				cc_error("!Null pointer referenced");
				return -1;
			}
			switch (reg1.Type) {
			// This might be a static object, passed to the user-defined extender function
			case kScValStaticObject:
			case kScValDynamicObject:
			case kScValPluginObject:
			case kScValPluginArg:
			// This might be an object of USER-DEFINED type, calling its MEMBER-FUNCTION.
			// Note, that this is the only case known when such object is written into reg[SREG_OP];
			// in any other case that would count as error.
			case kScValGlobalVar:
			case kScValStackPtr:
				registers[SREG_OP] = reg1;
				break;
			case kScValStaticArray:
				if (reg1.StcArr->GetDynamicManager()) {
					registers[SREG_OP].SetDynamicObject(
					    (char *)reg1.StcArr->GetElementPtr(reg1.Ptr, reg1.IValue),
					    reg1.StcArr->GetDynamicManager());
					break;
				}
			// fall through
			default:
				cc_error("internal error: SCMD_CALLOBJ argument is not an object of built-in or user-defined type");
				return -1;
			}
			next_call_needs_object = 1;
			break;
		case SCMD_SHIFTLEFT:
			reg1.SetInt32(reg1.IValue << reg2.IValue);
			break;
		case SCMD_SHIFTRIGHT:
			reg1.SetInt32(reg1.IValue >> reg2.IValue);
			break;
		case SCMD_THISBASE:
			thisbase[curnest] = arg1.IValue;
			break;
		case SCMD_NEWARRAY: {
			int numElements = reg1.IValue;
			if (numElements < 1) {
				cc_error("invalid size for dynamic array; requested: %d, range: 1..%d", numElements, INT32_MAX);
				return -1;
			}
			DynObjectRef ref = _GP(globalDynamicArray).Create(numElements, arg2.IValue, arg3.GetAsBool());
			reg1.SetDynamicObject(ref.second, &_GP(globalDynamicArray));
			break;
		}
		case SCMD_NEWUSEROBJECT: {
			const int32_t size = arg2.IValue;
			if (size < 0) {
				cc_error("Invalid size for user object; requested: %d (or %d), range: 0..%d", (uint32_t)size, size, INT_MAX);
				return -1;
			}
			ScriptUserObject *suo = ScriptUserObject::CreateManaged(size);
			reg1.SetDynamicObject(suo, suo);
			break;
		}
		case SCMD_FADD:
			reg1.SetFloat(reg1.FValue + arg2.IValue); // arg2 was used as int here originally
			break;
		case SCMD_FSUB:
			reg1.SetFloat(reg1.FValue - arg2.IValue); // arg2 was used as int here originally
			break;
		case SCMD_FMULREG:
			reg1.SetFloat(reg1.FValue * reg2.FValue);
			break;
		case SCMD_FDIVREG:
			if (reg2.FValue == 0.0) {
				cc_error("!Floating point divide by zero");
				return -1;
			}
			reg1.SetFloat(reg1.FValue / reg2.FValue);
			break;
		case SCMD_FADDREG:
			reg1.SetFloat(reg1.FValue + reg2.FValue);
			break;
		case SCMD_FSUBREG:
			reg1.SetFloat(reg1.FValue - reg2.FValue);
			break;
		case SCMD_FGREATER:
			reg1.SetFloatAsBool(reg1.FValue > reg2.FValue);
			break;
		case SCMD_FLESSTHAN:
			reg1.SetFloatAsBool(reg1.FValue < reg2.FValue);
			break;
		case SCMD_FGTE:
			reg1.SetFloatAsBool(reg1.FValue >= reg2.FValue);
			break;
		case SCMD_FLTE:
			reg1.SetFloatAsBool(reg1.FValue <= reg2.FValue);
			break;
		case SCMD_ZEROMEMORY:
			// Check if we are zeroing at stack tail
			if (registers[SREG_MAR] == registers[SREG_SP]) {
				// creating a local variable -- check the stack to ensure no mem overrun
				int currentStackSize = registers[SREG_SP].RValue - &stack[0];
				int currentDataSize = stackdata_ptr - stackdata;
				if (currentStackSize + 1 >= CC_STACK_SIZE ||
				        currentDataSize + arg1.IValue >= (int32_t)CC_STACK_DATA_SIZE) {
					cc_error("stack overflow, attempted grow to %d bytes", currentDataSize + arg1.IValue);
					return -1;
				}
				// NOTE: according to compiler's logic, this is always followed
				// by SCMD_ADD, and that is where the data is "allocated", here we
				// just clean the place.
				// CHECKME -- since we zero memory in PushDataToStack anyway, this is not needed at all?
				memset(stackdata_ptr, 0, arg1.IValue);
			} else {
				cc_error("internal error: stack tail address expected on SCMD_ZEROMEMORY instruction, reg[MAR] type is %d",
				         registers[SREG_MAR].Type);
				return -1;
			}
			break;
		case SCMD_CREATESTRING:
			if (_G(stringClassImpl) == nullptr) {
				cc_error("No string class implementation set, but opcode was used");
				return -1;
			}
			direct_ptr1 = (const char *)reg1.GetDirectPtr();
			reg1.SetDynamicObject(
			    _G(stringClassImpl)->CreateString(direct_ptr1).second,
			    &_GP(myScriptStringImpl));
			break;
		case SCMD_STRINGSEQUAL:
			if ((reg1.IsNull()) || (reg2.IsNull())) {
				cc_error("!Null pointer referenced");
				return -1;
			}
			direct_ptr1 = (const char *)reg1.GetDirectPtr();
			direct_ptr2 = (const char *)reg2.GetDirectPtr();
			reg1.SetInt32AsBool(strcmp(direct_ptr1, direct_ptr2) == 0);

			break;
		case SCMD_STRINGSNOTEQ:
			if ((reg1.IsNull()) || (reg2.IsNull())) {
				cc_error("!Null pointer referenced");
				return -1;
			}
			direct_ptr1 = (const char *)reg1.GetDirectPtr();
			direct_ptr2 = (const char *)reg2.GetDirectPtr();
			reg1.SetInt32AsBool(strcmp(direct_ptr1, direct_ptr2) != 0);
			break;
		case SCMD_LOOPCHECKOFF:
			if (loopIterationCheckDisabled == 0)
				loopIterationCheckDisabled++;
			break;
		default:
			cc_error("instruction %d is not implemented", codeOp.Instruction.Code);
			return -1;
		}

		pc += codeOp.ArgCount + 1;
	}
	return 0;
}

String ccInstance::GetCallStack(int maxLines) const {
	String buffer = String::FromFormat("in \"%s\", line %d\n", runningInst->instanceof->GetSectionName(pc), line_number);

	int linesDone = 0;
	for (int j = callStackSize - 1; (j >= 0) && (linesDone < maxLines); j--, linesDone++) {
		String lineBuffer = String::FromFormat("from \"%s\", line %d\n",
		                                       callStackCodeInst[j]->instanceof->GetSectionName(callStackAddr[j]), callStackLineNumber[j]);
		buffer.Append(lineBuffer);
		if (linesDone == maxLines - 1)
			buffer.Append("(and more...)\n");
	}
	return buffer;
}

void ccInstance::GetScriptPosition(ScriptPosition &script_pos) const {
	script_pos.Section = runningInst->instanceof->GetSectionName(pc);
	script_pos.Line    = line_number;
}

// get a pointer to a variable or function exported by the script
RuntimeScriptValue ccInstance::GetSymbolAddress(const char *symname) const {
	int k;
	char altName[200];
	snprintf(altName, sizeof(altName), "%s$", symname);
	RuntimeScriptValue rval_null;
	size_t len_altName = strlen(altName);
	for (k = 0; k < instanceof->numexports; k++) {
		if (strcmp(instanceof->exports[k], symname) == 0)
			return exports[k];
		// mangled function name
		if (strncmp(instanceof->exports[k], altName, len_altName) == 0)
			return exports[k];
	}
	return rval_null;
}

void ccInstance::DumpInstruction(const ScriptOperation &op) const {
	// line_num local var should be shared between all the instances
	static int line_num = 0;

	if (op.Instruction.Code == SCMD_LINENUM) {
		line_num = op.Args[0].IValue;
		return;
	}

	debugN("Line %3d, IP:%8d (SP:%p) ", line_num, pc, (void *)(registers[SREG_SP].RValue));

	const ScriptCommandInfo &cmd_info = (*g_commands)[op.Instruction.Code];
	debugN("%s", cmd_info.CmdName);

	for (int i = 0; i < cmd_info.ArgCount; ++i) {
		if (i > 0) {
			debugN(",");
		}
		if (cmd_info.ArgIsReg[i]) {
			debugN(" %s", regnames[op.Args[i].IValue]);
		} else {
			RuntimeScriptValue arg = op.Args[i];
			if (arg.Type == kScValStackPtr || arg.Type == kScValGlobalVar) {
				arg = *arg.RValue;
			}
			switch (arg.Type) {
			case kScValInteger:
			case kScValPluginArg:
				debugN(" %d", arg.IValue);
				break;
			case kScValFloat:
				debugN(" %f", arg.FValue);
				break;
			case kScValStringLiteral:
				debugN(" \"%s\"", arg.Ptr);
				break;
			case kScValStackPtr:
			case kScValGlobalVar:
				debugN(" %p", (void *)(arg.RValue));
				break;
			case kScValData:
			case kScValCodePtr:
				debugN(" %p", (void *)arg.GetPtrWithOffset());
				break;
			case kScValStaticArray:
			case kScValStaticObject:
			case kScValDynamicObject:
			case kScValStaticFunction:
			case kScValObjectFunction:
			case kScValPluginFunction:
			case kScValPluginObject: {
				String name = _GP(simp).findName(arg);
				if (!name.IsEmpty()) {
					debugN(" &%s", name.GetCStr());
				} else {
					debugN(" %p", (void *)arg.GetPtrWithOffset());
				}
			}
			break;
			case kScValUndefined:
				debugN("undefined");
				break;
			}
		}
	}

	debugN("\n");
}

bool ccInstance::IsBeingRun() const {
	return pc != 0;
}

void ccInstance::NotifyAlive() {
	flags |= INSTF_RUNNING;
	_lastAliveTs = AGS_Clock::now();
}

bool ccInstance::_Create(PScript scri, ccInstance *joined) {
	_G(currentline) = -1;
	if ((scri == nullptr) && (joined != nullptr))
		scri = joined->instanceof;

	if (scri == nullptr) {
		cc_error("null pointer passed");
		return false;
	}

	if (joined != nullptr) {
		// share memory space with an existing instance (ie. this is a thread/fork)
		globalvars = joined->globalvars;
		globaldatasize = joined->globaldatasize;
		globaldata = joined->globaldata;
		code = joined->code;
		codesize = joined->codesize;
	} else {
		// create own memory space
		// NOTE: globalvars are created in CreateGlobalVars()
		globalvars.reset(new ScVarMap());
		globaldatasize = scri->globaldatasize;
		globaldata = nullptr;
		if (globaldatasize > 0) {
			globaldata = (char *)malloc(globaldatasize);
			memcpy(globaldata, scri->globaldata, globaldatasize);
		}

		codesize = scri->codesize;
		code = nullptr;
		if (codesize > 0) {
			code = (intptr_t *)malloc(codesize * sizeof(intptr_t));
			// 64 bit: Read code into 8 byte array, necessary for being able to perform
			// relocations on the references.
			for (int i = 0; i < codesize; ++i)
				code[i] = scri->code[i];
		}
	}

	// just use the pointer to the strings since they don't change
	strings = scri->strings;
	stringssize = scri->stringssize;
	// create a stack
	stackdatasize = CC_STACK_DATA_SIZE;
	// This is quite a random choice; there's no way to deduce number of stack
	// entries needed without knowing amount of local variables (at least)
	num_stackentries = CC_STACK_SIZE;
	stack       = new RuntimeScriptValue[num_stackentries];
	stackdata   = new char[stackdatasize];
	if (stack == nullptr || stackdata == nullptr) {
		cc_error("not enough memory to allocate stack");
		return false;
	}

	// find a LoadedInstance slot for it
	for (int i = 0; i < MAX_LOADED_INSTANCES; i++) {
		if (_G(loadedInstances)[i] == nullptr) {
			_G(loadedInstances)[i] = this;
			loadedInstanceId = i;
			break;
		}
		if (i == MAX_LOADED_INSTANCES - 1) {
			cc_error("too many active instances");
			return false;
		}
	}

	if (joined) {
		resolved_imports = joined->resolved_imports;
		code_fixups = joined->code_fixups;
	} else {
		if (!CreateGlobalVars(scri.get())) {
			return false;
		}
		if (!CreateRuntimeCodeFixups(scri.get())) {
			return false;
		}
	}

	exports = new RuntimeScriptValue[scri->numexports];

	// find the real address of the exports
	for (int i = 0; i < scri->numexports; i++) {
		int32_t etype = (scri->export_addr[i] >> 24L) & 0x000ff;
		int32_t eaddr = (scri->export_addr[i] & 0x00ffffff);
		if (etype == EXPORT_FUNCTION) {
			// NOTE: unfortunately, there seems to be no way to know if
			// that's an extender function that expects object pointer
			exports[i].SetCodePtr((char *)((intptr_t)eaddr * sizeof(intptr_t) + (char *)(&code[0])));
		} else if (etype == EXPORT_DATA) {
			ScriptVariable *gl_var = FindGlobalVar(eaddr);
			if (gl_var) {
				exports[i].SetGlobalVar(&gl_var->RValue);
			} else {
				cc_error("cannot resolve global variable, key = %d", eaddr);
				return false;
			}
		} else {
			cc_error("internal export fixup error");
			return false;
		}
	}
	instanceof = scri;
	pc = 0;
	flags = 0;
	if (joined != nullptr)
		flags = INSTF_SHAREDATA;
	scri->instances++;

	if ((scri->instances == 1) && (ccGetOption(SCOPT_AUTOIMPORT) != 0)) {
		// import all the exported stuff from this script
		for (int i = 0; i < scri->numexports; i++) {
			if (!ccAddExternalScriptSymbol(scri->exports[i], exports[i], this)) {
				cc_error("Export table overflow at '%s'", scri->exports[i]);
				return false;
			}
		}
	}
	return true;
}

void ccInstance::Free() {
	if (instanceof != nullptr) {
		instanceof->instances--;
		if (instanceof->instances == 0) {
			_GP(simp).RemoveScriptExports(this);
		}
	}

	// remove from the Active Instances list
	if (_G(loadedInstances)[loadedInstanceId] == this)
		_G(loadedInstances)[loadedInstanceId] = nullptr;

	if ((flags & INSTF_SHAREDATA) == 0) {
		nullfree(globaldata);
		nullfree(code);
	}
	globalvars.reset();
	globaldata = nullptr;
	code = nullptr;
	strings = nullptr;

	delete[] stack;
	delete[] stackdata;
	delete[] exports;
	stack = nullptr;
	stackdata = nullptr;
	exports = nullptr;

	if ((flags & INSTF_SHAREDATA) == 0) {
		delete[] resolved_imports;
		delete[] code_fixups;
	}
	resolved_imports = nullptr;
	code_fixups = nullptr;
}

bool ccInstance::ResolveScriptImports(const ccScript *scri) {
	// Script keeps the information of what imports are used as an array of names.
	// When an import is referenced in the code, it's addressed by its index in this
	// array. Different scripts have differing arrays of imports; indexes
	// into 'imports[]' are NOT unique and relative to the respective script only.
	// To allow real-time import use, the sequence of imports in 'imports[]'
	// and 'resolved_imports[]' should not be modified.

	numimports = scri->numimports;
	if (numimports == 0) {
		// [PGB] AFAICS there's nothing wrong with not having any imports, and
		// it doesn't lead to trouble. However, if it turns out that we do need
		// to return 'false' here, we should also report why with a 'Debug::Printf()' call.
		resolved_imports = nullptr;
		return true;
	}

	resolved_imports = new uint32_t[numimports];
	size_t errors = 0, last_err_idx = 0;
	for (int import_idx = 0; import_idx < scri->numimports; ++import_idx) {
		if (scri->imports[import_idx] == nullptr) {
			resolved_imports[import_idx] = UINT32_MAX;
			continue;
		}

		resolved_imports[import_idx] = _GP(simp).get_index_of(scri->imports[import_idx]);
		if (resolved_imports[import_idx] == UINT32_MAX) {
			Debug::Printf(kDbgMsg_Error, "unresolved import '%s' in '%s'", scri->imports[import_idx], scri->numSections > 0 ? scri->sectionNames[0] : "<unknown>");
			errors++;
			last_err_idx = import_idx;
		}
	}

	if (errors > 0)
		cc_error("in %s: %d unresolved imports (last: %s)",
			scri->numSections > 0 ? scri->sectionNames[0] : "<unknown>",
			errors,
			scri->imports[last_err_idx]);

	return errors == 0;
}

// TODO: it is possible to deduce global var's size at start with
// certain accuracy after all global vars are registered. Each
// global var's size would be limited by closest next var's ScAddress
// and globaldatasize.
bool ccInstance::CreateGlobalVars(const ccScript *scri) {
	ScriptVariable glvar;

	// Step One: deduce global variables from fixups
	for (int i = 0; i < scri->numfixups; ++i) {
		switch (scri->fixuptypes[i]) {
		case FIXUP_GLOBALDATA:
			// GLOBALDATA fixup takes relative address of global data element from code array;
			// this is the address of actual data
			glvar.ScAddress = (int32_t)code[scri->fixups[i]];
			glvar.RValue.SetData(globaldata + glvar.ScAddress, 0);
			break;
		case FIXUP_DATADATA: {
			// DATADATA fixup takes relative address of global data element from fixups array;
			// this is the address of element, which stores address of actual data
			glvar.ScAddress = scri->fixups[i];
			int32_t data_addr = BBOp::Int32FromLE(*(int32_t *)&globaldata[glvar.ScAddress]);
			if (glvar.ScAddress - data_addr != 200 /* size of old AGS string */) {
				// CHECKME: probably replace with mere warning in the log?
				cc_error("unexpected old-style string's alignment");
				return false;
			}
			// TODO: register this explicitly as a string instead (can do this later)
			glvar.RValue.SetStaticObject(globaldata + data_addr, &_GP(GlobalStaticManager));
		}
		break;
		default:
			// other fixups are of no use here
			continue;
		}

		AddGlobalVar(glvar);
	}

	// Step Two: deduce global variables from exports
	for (int i = 0; i < scri->numexports; ++i) {
		int32_t etype = (scri->export_addr[i] >> 24L) & 0x000ff;
		int32_t eaddr = (scri->export_addr[i] & 0x00ffffff);
		if (etype == EXPORT_DATA) {
			// NOTE: old-style strings could not be exported in AGS,
			// no need to worry about these here
			glvar.ScAddress = eaddr;
			glvar.RValue.SetData(globaldata + glvar.ScAddress, 0);
			AddGlobalVar(glvar);
		}
	}

	return true;
}

bool ccInstance::AddGlobalVar(const ScriptVariable &glvar) {
	// NOTE:
	// We suppress the error here, because unfortunately at least one existing
	// game ("Metal Dead", built with AGS 3.21.1115) fails to pass this check.
	// It has been found that this may be caused by a global variable of zero
	// size (an instance of empty struct) placed in the end of the script.
	// TODO: invent some workaround?
	// TODO: enable the error back in AGS 4, as this is not a normal behavior.
	if (glvar.ScAddress < 0 || glvar.ScAddress >= globaldatasize) {
		/* return false; */
		Debug::Printf(kDbgMsg_Warn, "WARNING: global variable refers to data beyond allocated buffer (%d, %d)", glvar.ScAddress, globaldatasize);
	}
	globalvars->insert(std::make_pair(glvar.ScAddress, glvar));
	return true;
}

ScriptVariable *ccInstance::FindGlobalVar(int32_t var_addr) {
	// NOTE: see comment for AddGlobalVar()
	if (var_addr < 0 || var_addr >= globaldatasize) {
		/*
		return NULL;
		*/
		Debug::Printf(kDbgMsg_Warn, "WARNING: looking up for global variable beyond allocated buffer (%d, %d)", var_addr, globaldatasize);
	}
	ScVarMap::iterator it = globalvars->find(var_addr);
	return it != globalvars->end() ? &it->_value : nullptr;
}

static int DetermineScriptLine(const int32_t *code, size_t codesz, size_t at_pc) {
	int line = -1;
	for (size_t pc = 0; (pc <= at_pc) && (pc < codesz); ++pc) {
		int op = code[pc] & INSTANCE_ID_REMOVEMASK;
		if (op < 0 || op >= CC_NUM_SCCMDS) return -1;
		if (pc + (*g_commands)[op].ArgCount >= codesz) return -1;
		if (op == SCMD_LINENUM)
			line = code[pc + 1];
		pc += (*g_commands)[op].ArgCount;
	}
	return line;
}

static void cc_error_fixups(const ccScript *scri, size_t pc, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	String displbuf = String::FromFormatV(fmt, ap);
	va_end(ap);
	const char *scname = scri->numSections > 0 ? scri->sectionNames[0] : "?";
	if (pc == SIZE_MAX) {
		cc_error("in script %s: %s", scname, displbuf.GetCStr());
	} else {
		int line = DetermineScriptLine(scri->code, scri->codesize, pc);
		cc_error("in script %s around line %d: %s", scname, line, displbuf.GetCStr());
	}
}

bool ccInstance::CreateRuntimeCodeFixups(const ccScript *scri) {
	code_fixups = new char[scri->codesize]();
	for (int i = 0; i < scri->numfixups; ++i) {
		if (scri->fixuptypes[i] == FIXUP_DATADATA) {
			continue;
		}

		int32_t fixup = scri->fixups[i];
		code_fixups[fixup] = scri->fixuptypes[i];

		switch (scri->fixuptypes[i]) {
		case FIXUP_GLOBALDATA: {
			ScriptVariable *gl_var = FindGlobalVar((int32_t)code[fixup]);
			if (!gl_var) {
				cc_error_fixups(scri, fixup, "cannot resolve global variable (bytecode pos %d, key %d)", fixup, (int32_t)code[fixup]);
				return false;
			}
			code[fixup] = (intptr_t)gl_var;
		}
		break;
		case FIXUP_FUNCTION:
		case FIXUP_STRING:
		case FIXUP_STACK:
		case FIXUP_IMPORT:
			break; // do nothing yet
		default:
			cc_error_fixups(scri, UINT32_MAX, "unknown fixup type: %d (fixup num %d)", scri->fixuptypes[i], i);
			return false;
		}
	}

	return true;
}

bool ccInstance::ResolveImportFixups(const ccScript *scri) {
	for (int fixup_idx = 0; fixup_idx < scri->numfixups; ++fixup_idx) {
		if (scri->fixuptypes[fixup_idx] != FIXUP_IMPORT)
			continue;

		uint32_t const fixup = scri->fixups[fixup_idx];
		uint32_t const import_index = resolved_imports[code[fixup]];
		ScriptImport const *import = _GP(simp).getByIndex(import_index);
		if (!import) {
			cc_error_fixups(scri, fixup, "cannot resolve import (bytecode pos %d, key %d)", fixup, import_index);
			return false;
		}
		code[fixup] = import_index;
		// If the call is to another script function next CALLEXT
		// must be replaced with CALLAS
		if (import->InstancePtr != nullptr && (code[fixup + 1] & INSTANCE_ID_REMOVEMASK) == SCMD_CALLEXT)
			code[fixup + 1] = SCMD_CALLAS | (import->InstancePtr->loadedInstanceId << INSTANCE_ID_SHIFT);
	}
	return true;
}

/*
bool ccInstance::ReadOperation(ScriptOperation &op, int32_t at_pc)
{
    op.Instruction.Code         = code[at_pc];
    op.Instruction.InstanceId   = (op.Instruction.Code >> INSTANCE_ID_SHIFT) & INSTANCE_ID_MASK;
    op.Instruction.Code        &= INSTANCE_ID_REMOVEMASK; // now this is pure instruction code

    int want_args = (*g_commands)[op.Instruction.Code].ArgCount;
    if (at_pc + want_args >= codesize)
    {
        cc_error("unexpected end of code data at %d", at_pc + want_args);
        return false;
    }
    op.ArgCount = want_args;

    at_pc++;
    for (int i = 0; i < op.ArgCount; ++i, ++at_pc)
    {
        char fixup = code_fixups[at_pc];
        if (fixup > 0)
        {
            // could be relative pointer or import address
            if (!FixupArgument(code[at_pc], fixup, op.Args[i]))
            {
                return false;
            }
        }
        else
        {
            // should be a numeric literal (int32 or float)
            op.Args[i].SetInt32( (int32_t)code[at_pc] );
        }
    }

    return true;
}
*/
/*
bool ccInstance::FixupArgument(intptr_t code_value, char fixup_type, RuntimeScriptValue &argument)
{
    switch (fixup_type)
    {
    case FIXUP_GLOBALDATA:
        {
            ScriptVariable *gl_var = (ScriptVariable*)code_value;
            argument.SetGlobalVar(&gl_var->RValue);
        }
        break;
    case FIXUP_FUNCTION:
        // originally commented -- CHECKME: could this be used in very old versions of AGS?
        //      code[fixup] += (long)&code[0];
        // This is a program counter value, presumably will be used as SCMD_CALL argument
        argument.SetInt32((int32_t)code_value);
        break;
    case FIXUP_STRING:
        argument.SetStringLiteral(&strings[0] + code_value);
        break;
    case FIXUP_IMPORT:
        {
            const ScriptImport *import = _GP(simp).getByIndex((int32_t)code_value);
            if (import)
            {
                argument = import->Value;
            }
            else
            {
                cc_error("cannot resolve import, key = %ld", code_value);
                return false;
            }
        }
        break;
    case FIXUP_STACK:
        argument = GetStackPtrOffsetFw((int32_t)code_value);
        break;
    default:
        cc_error("internal fixup type error: %d", fixup_type);
        return false;
    }
    return true;
}
*/
//-----------------------------------------------------------------------------

void ccInstance::PushValueToStack(const RuntimeScriptValue &rval) {
	// Write value to the stack tail and advance stack ptr
	registers[SREG_SP].WriteValue(rval);
	registers[SREG_SP].RValue++;
}

void ccInstance::PushDataToStack(int32_t num_bytes) {
	if (registers[SREG_SP].RValue->IsValid()) {
		cc_error("internal error: valid data beyond stack ptr");
		return;
	}
	// Zero memory, assign pointer to data block to the stack tail, advance both stack ptr and stack data ptr
	memset(stackdata_ptr, 0, num_bytes);
	registers[SREG_SP].RValue->SetData(stackdata_ptr, num_bytes);
	stackdata_ptr += num_bytes;
	registers[SREG_SP].RValue++;
}

RuntimeScriptValue ccInstance::PopValueFromStack() {
	// rewind stack ptr to the last valid value, decrement stack data ptr if needed and invalidate the stack tail
	registers[SREG_SP].RValue--;
	RuntimeScriptValue rval = *registers[SREG_SP].RValue;
	if (rval.Type == kScValData) {
		stackdata_ptr -= rval.Size;
	}
	registers[SREG_SP].RValue->Invalidate();
	return rval;
}

void ccInstance::PopValuesFromStack(int32_t num_entries = 1) {
	for (int i = 0; i < num_entries; ++i) {
		// rewind stack ptr to the last valid value, decrement stack data ptr if needed and invalidate the stack tail
		registers[SREG_SP].RValue--;
		if (registers[SREG_SP].RValue->Type == kScValData) {
			stackdata_ptr -= registers[SREG_SP].RValue->Size;
		}
		registers[SREG_SP].RValue->Invalidate();
	}
}

void ccInstance::PopDataFromStack(int32_t num_bytes) {
	int32_t total_pop = 0;
	while (total_pop < num_bytes && registers[SREG_SP].RValue > &stack[0]) {
		// rewind stack ptr to the last valid value, decrement stack data ptr if needed and invalidate the stack tail
		registers[SREG_SP].RValue--;
		// remember popped bytes count
		total_pop += registers[SREG_SP].RValue->Size;
		if (registers[SREG_SP].RValue->Type == kScValData) {
			stackdata_ptr -= registers[SREG_SP].RValue->Size;
		}
		registers[SREG_SP].RValue->Invalidate();
	}
	if (total_pop < num_bytes) {
		cc_error("stack underflow");
	} else if (total_pop > num_bytes) {
		cc_error("stack pointer points inside local variable after pop, stack corrupted?");
	}
}

RuntimeScriptValue ccInstance::GetStackPtrOffsetFw(int32_t fw_offset) {
	int32_t total_off = 0;
	RuntimeScriptValue *stack_entry = &stack[0];
	while (total_off < fw_offset && stack_entry - &stack[0] < CC_STACK_SIZE) {
		if (stack_entry->Size > 0) {
			total_off += stack_entry->Size;
		}
		stack_entry++;
	}
	if (total_off < fw_offset) {
		cc_error("accessing address beyond stack's tail");
		return RuntimeScriptValue();
	}
	RuntimeScriptValue stack_ptr;
	stack_ptr.SetStackPtr(stack_entry);
	if (total_off > fw_offset) {
		// Forward offset should always set ptr at the beginning of stack entry
		cc_error("stack offset forward: trying to access stack data inside stack entry, stack corrupted?");
	}
	return stack_ptr;
}

RuntimeScriptValue ccInstance::GetStackPtrOffsetRw(int32_t rw_offset) {
	int32_t total_off = 0;
	RuntimeScriptValue *stack_entry = registers[SREG_SP].RValue;
	while (total_off < rw_offset && stack_entry >= &stack[0]) {
		stack_entry--;
		total_off += stack_entry->Size;
	}
	if (total_off < rw_offset) {
		cc_error("accessing address before stack's head");
		return RuntimeScriptValue();
	}
	RuntimeScriptValue stack_ptr;
	stack_ptr.SetStackPtr(stack_entry);
	if (total_off > rw_offset) {
		// Could be accessing array element, so state error only if stack entry does not refer to data array
		if (stack_entry->Type == kScValData) {
			stack_ptr.IValue += total_off - rw_offset;
		} else {
			cc_error("stack offset backward: trying to access stack data inside stack entry, stack corrupted?");
		}
	}
	return stack_ptr;
}

void ccInstance::PushToFuncCallStack(FunctionCallStack &func_callstack, const RuntimeScriptValue &rval) {
	if (func_callstack.Count >= MAX_FUNC_PARAMS) {
		cc_error("function callstack overflow");
		return;
	}

	func_callstack.Entries[func_callstack.Head] = rval;
	func_callstack.Head--;
	func_callstack.Count++;
}

void ccInstance::PopFromFuncCallStack(FunctionCallStack &func_callstack, int32_t num_entries) {
	if (func_callstack.Count == 0) {
		cc_error("function callstack underflow");
		return;
	}

	func_callstack.Head += num_entries;
	func_callstack.Count -= num_entries;
}

} // namespace AGS3
