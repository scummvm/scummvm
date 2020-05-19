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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/engine/features.h"
#include "sci/engine/guest_additions.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/selector.h"	// for SELECTOR
#include "sci/engine/gc.h"
#include "sci/engine/workarounds.h"
#include "sci/engine/scriptdebug.h"
#include "sci/engine/vm_hooks.h"

namespace Sci {

const reg_t NULL_REG = {0, 0};
const reg_t SIGNAL_REG = {0, SIGNAL_OFFSET};
const reg_t TRUE_REG = {0, 1};
// Enable the define below to have the VM abort on cases where a conditional
// statement is followed by an unconditional jump (which will most likely lead
// to an infinite loop). Aids in detecting script bugs such as #3040722.
//#define ABORT_ON_INFINITE_LOOP

// validation functionality

static reg_t &validate_property(EngineState *s, Object *obj, int index) {
	// A static dummy reg_t, which we return if obj or index turn out to be
	// invalid. Note that we cannot just return NULL_REG, because client code
	// may modify the value of the returned reg_t.
	static reg_t dummyReg = NULL_REG;

	// If this occurs, it means there's probably something wrong with the garbage
	// collector, so don't hide it with fake return values
	if (!obj)
		error("validate_property: Sending to disposed object");

	if (getSciVersion() == SCI_VERSION_3)
		index = obj->locateVarSelector(s->_segMan, index);
	else
		index >>= 1;

	if (index < 0 || (uint)index >= obj->getVarCount()) {
		// This is same way sierra does it and there are some games, that contain such scripts like
		//  iceman script 998 (fred::canBeHere, executed right at the start)
		debugC(kDebugLevelVM, "[VM] Invalid property #%d (out of [0..%d]) requested from object %04x:%04x (%s)",
			index, obj->getVarCount(), PRINT_REG(obj->getPos()), s->_segMan->getObjectName(obj->getPos()));
		return dummyReg;
	}

	return obj->getVariableRef(index);
}

static StackPtr validate_stack_addr(EngineState *s, StackPtr sp) {
	if (sp >= s->stack_base && sp < s->stack_top)
		return sp;
	else
	error("[VM] Stack index %d out of valid range [%d..%d]",
		(int)(sp - s->stack_base), 0, (int)(s->stack_top - s->stack_base - 1));
}

static bool validate_variable(reg_t *r, reg_t *stack_base, int type, int max, int index) {
	const char *names[4] = {"global", "local", "temp", "param"};

	if (index < 0 || index >= max) {
		Common::String txt = Common::String::format(
							"[VM] Attempt to use invalid %s variable %04x ",
							names[type], index);
		if (max == 0)
			txt += "(variable type invalid)";
		else
			txt += Common::String::format("(out of range [%d..%d])", 0, max - 1);

		if (type == VAR_PARAM || type == VAR_TEMP) {
			int total_offset = r - stack_base;
			if (total_offset < 0 || total_offset >= VM_STACK_SIZE) {
				// Fatal, as the game is trying to do an OOB access
				error("%s. [VM] Access would be outside even of the stack (%d); access denied", txt.c_str(), total_offset);
				return false;
			} else {
				debugC(kDebugLevelVM, "%s", txt.c_str());
				debugC(kDebugLevelVM, "[VM] Access within stack boundaries; access granted.");
				return true;
			}
		}
		return false;
	}

	return true;
}

static reg_t read_var(EngineState *s, int type, int index) {
	if (validate_variable(s->variables[type], s->stack_base, type, s->variablesMax[type], index)) {
		if (s->variables[type][index].getSegment() == kUninitializedSegment) {
			switch (type) {
			case VAR_TEMP: {
				// Uninitialized read on a temp
				//  We need to find correct replacements for each situation manually
				SciCallOrigin originReply;
				SciWorkaroundSolution solution = trackOriginAndFindWorkaround(index, uninitializedReadWorkarounds, &originReply);
				if (solution.type == WORKAROUND_NONE) {
#ifdef RELEASE_BUILD
					// If we are running an official ScummVM release -> fake 0 in unknown cases
					warning("Uninitialized read for temp %d from %s", index, originReply.toString().c_str());

					s->variables[type][index] = NULL_REG;
					break;
#else
					error("Uninitialized read for temp %d from %s", index, originReply.toString().c_str());
#endif
				}
				assert(solution.type == WORKAROUND_FAKE);
				s->variables[type][index] = make_reg(0, solution.value);
				break;
			}
			case VAR_PARAM: {
				// Out-of-bounds read for a parameter that goes onto stack and hits an uninitialized temp
				//  We need to find correct replacements for each situation manually
				SciCallOrigin originReply;
				SciWorkaroundSolution solution = trackOriginAndFindWorkaround(index, uninitializedReadForParamWorkarounds, &originReply);
				if (solution.type == WORKAROUND_NONE) {
					warning("Uninitialized read for parameter %d from %s", index, originReply.toString().c_str());
					return NULL_REG;
				} else {
					return make_reg(0, solution.value);
				}
			}
			default:
				break;
			}
		}
		return s->variables[type][index];
	} else
		return s->r_acc;
}

static void write_var(EngineState *s, int type, int index, reg_t value) {
	if (validate_variable(s->variables[type], s->stack_base, type, s->variablesMax[type], index)) {

		// WORKAROUND: This code is needed to work around a probable script bug, or a
		// limitation of the original SCI engine, which can be observed in LSL5.
		//
		// In some games, ego walks via the "Grooper" object, in particular its "stopGroop"
		// child. In LSL5, during the game, ego is swapped from Larry to Patti. When this
		// happens in the original interpreter, the new actor is loaded in the same memory
		// location as the old one, therefore the client variable in the stopGroop object
		// points to the new actor. This is probably why the reference of the stopGroop
		// object is never updated (which is why I mentioned that this is either a script
		// bug or some kind of limitation).
		//
		// In our implementation, each new object is loaded in a different memory location,
		// and we can't overwrite the old one. This means that in our implementation,
		// whenever ego is changed, we need to update the "client" variable of the
		// stopGroop object, which points to ego, to the new ego object. If this is not
		// done, ego's movement will not be updated properly, so the result is
		// unpredictable (for example in LSL5, Patti spins around instead of walking).
		if (index == kGlobalVarEgo && type == VAR_GLOBAL && getSciVersion() > SCI_VERSION_0_EARLY) {
			reg_t stopGroopPos = s->_segMan->findObjectByName("stopGroop");
			if (!stopGroopPos.isNull()) {	// does the game have a stopGroop object?
				// Find the "client" member variable of the stopGroop object, and update it
				ObjVarRef varp;
				if (lookupSelector(s->_segMan, stopGroopPos, SELECTOR(client), &varp, NULL) == kSelectorVariable) {
					reg_t *clientVar = varp.getPointer(s->_segMan);
					*clientVar = value;
				}
			}
		}

		// If we are writing an uninitialized value into a temp, we remove the uninitialized segment
		//  this happens at least in sq1/room 44 (slot-machine), because a send is missing parameters, then
		//  those parameters are taken from uninitialized stack and afterwards they are copied back into temps
		//  if we don't remove the segment, we would get false-positive uninitialized reads later
		if (type == VAR_TEMP && value.getSegment() == kUninitializedSegment)
			value.setSegment(0);

		s->variables[type][index] = value;

		g_sci->_guestAdditions->writeVarHook(type, index, value);
	}
}

// Operating on the stack
// 16 bit:
#define PUSH(v) PUSH32(make_reg(0, v))
// 32 bit:
#define PUSH32(a) (*(validate_stack_addr(s, (s->xs->sp)++)) = (a))
#define POP32() (*(validate_stack_addr(s, --(s->xs->sp))))

ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct, StackPtr sp, reg_t calling_obj, uint16 argc, StackPtr argp) {
	int seg = s->_segMan->getScriptSegment(script);
	Script *scr = s->_segMan->getScriptIfLoaded(seg);

	if (!scr || scr->isMarkedAsDeleted()) { // Script not present yet?
		seg = s->_segMan->instantiateScript(script);
		scr = s->_segMan->getScript(seg);
	}

	// Check if a breakpoint is set on this method
	if (g_sci->checkExportBreakpoint(script, pubfunct)) {
		logExportCall(script, pubfunct, s, argc, argp);
	}

	uint32 exportAddr = scr->validateExportFunc(pubfunct, false);
	if (!exportAddr)
		return NULL;

	assert(argp[0].toUint16() == argc); // The first argument is argc
	ExecStack xstack(calling_obj, calling_obj, sp, argc, argp,
						seg, make_reg32(seg, exportAddr), -1, -1, -1, pubfunct, -1,
						s->_executionStack.size() - 1, EXEC_STACK_TYPE_CALL);
	s->_executionStack.push_back(xstack);
	return &(s->_executionStack.back());
}

static void _exec_varselectors(EngineState *s) {
	// Executes all varselector read/write ops on the TOS
	while (!s->_executionStack.empty() && s->_executionStack.back().type == EXEC_STACK_TYPE_VARSELECTOR) {
		ExecStack &xs = s->_executionStack.back();
		reg_t *var = xs.getVarPointer(s->_segMan);
		if (!var) {
			error("Invalid varselector exec stack entry");
		} else {
			// varselector access?
			if (xs.argc) { // write?
				*var = xs.variables_argp[1];

#ifdef ENABLE_SCI32
				updateInfoFlagViewVisible(s->_segMan->getObject(xs.addr.varp.obj), xs.addr.varp.varindex);
#endif

			} else // No, read
				s->r_acc = *var;
		}
		s->_executionStack.pop_back();
	}
}


ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj, StackPtr sp, int framesize, StackPtr argp) {
	// send_obj and work_obj are equal for anything but 'super'
	// Returns a pointer to the TOS exec_stack element
	assert(s);

	reg_t funcp;
	Selector selector;
	int argc;
	int origin = s->_executionStack.size() - 1; // Origin: Used for debugging
	int activeBreakpointTypes = g_sci->_debugState._activeBreakpointTypes;
	ObjVarRef varp;

	Common::List<ExecStack>::iterator prevElementIterator = s->_executionStack.end();

	while (framesize > 0) {
		selector = argp->requireUint16();
		argp++;
		argc = argp->requireUint16();

		if (argc > 0x800)	// More arguments than the stack could possibly accomodate for
			error("send_selector(): More than 0x800 arguments to function call");

#ifdef ENABLE_SCI32
		g_sci->_guestAdditions->sendSelectorHook(send_obj, selector, argp);
#endif

		SelectorType selectorType = lookupSelector(s->_segMan, send_obj, selector, &varp, &funcp);
		if (selectorType == kSelectorNone)
			error("Send to invalid selector 0x%x (%s) of object at %04x:%04x", 0xffff & selector, g_sci->getKernel()->getSelectorName(0xffff & selector).c_str(), PRINT_REG(send_obj));

		ExecStackType stackType = EXEC_STACK_TYPE_VARSELECTOR;
		StackPtr curSP = NULL;
		reg_t curFP = make_reg32(0, 0);
		if (selectorType == kSelectorMethod) {
			stackType = EXEC_STACK_TYPE_CALL;
			curSP = sp;
			curFP = make_reg32(funcp.getSegment(), funcp.getOffset());
			sp = CALL_SP_CARRY; // Destroy sp, as it will be carried over
		}

		if ((activeBreakpointTypes & (BREAK_SELECTOREXEC | BREAK_SELECTORREAD | BREAK_SELECTORWRITE))
		     || DebugMan.isDebugChannelEnabled(kDebugLevelScripts))
			debugSelectorCall(send_obj, selector, argc, argp, varp, funcp, s->_segMan, selectorType);

		assert(argp[0].toUint16() == argc); // The first argument is argc
		ExecStack xstack(work_obj, send_obj, curSP, argc, argp,
							kUninitializedSegment, curFP, selector, -1, -1, -1, -1,
							origin, stackType);

		if (selectorType == kSelectorVariable)
			xstack.addr.varp = varp;

		// The new stack entries should be put on the stack in reverse order
		// so that the first one is executed first
		s->_executionStack.insert(prevElementIterator, xstack);
		// Decrement the stack end pointer so that it points to our recently
		// added element, so that the next insert() places it before this one.
		--prevElementIterator;

		framesize -= (2 + argc);
		argp += argc + 1;
	}	// while (framesize > 0)

	// Perform all varselector actions at the top of the stack immediately.
	// Note that there may be some behind method selector calls as well;
	// those will get executed by op_ret later.
	_exec_varselectors(s);

	return s->_executionStack.empty() ? NULL : &(s->_executionStack.back());
}

static void addKernelCallToExecStack(EngineState *s, int kernelCallNr, int kernelSubCallNr, int argc, reg_t *argv) {
	// Add stack frame to indicate we're executing a callk.
	// This is useful in debugger backtraces if this
	// kernel function calls a script itself.
	ExecStack xstack(NULL_REG, NULL_REG, argv + argc, argc, argv - 1, kUninitializedSegment, make_reg32(0, 0),
						-1, kernelCallNr, kernelSubCallNr, -1, -1, s->_executionStack.size() - 1, EXEC_STACK_TYPE_KERNEL);
	s->_executionStack.push_back(xstack);
}

static void callKernelFunc(EngineState *s, int kernelCallNr, int argc) {
	Kernel *kernel = g_sci->getKernel();

	if (kernelCallNr >= (int)kernel->_kernelFuncs.size())
		error("Invalid kernel function 0x%x requested", kernelCallNr);

	const KernelFunction &kernelCall = kernel->_kernelFuncs[kernelCallNr];
	reg_t *argv = s->xs->sp + 1;

	if (kernelCall.signature
			&& !kernel->signatureMatch(kernelCall.signature, argc, argv)) {
		// signature mismatch, check if a workaround is available
		SciCallOrigin originReply;
		SciWorkaroundSolution solution = trackOriginAndFindWorkaround(0, kernelCall.workarounds, &originReply);
		switch (solution.type) {
		case WORKAROUND_NONE: {
			Common::String signatureDetailsStr;
			kernel->signatureDebug(signatureDetailsStr, kernelCall.signature, argc, argv);
			error("\n%s[VM] k%s[%x]: signature mismatch in %s", signatureDetailsStr.c_str(), kernelCall.name, kernelCallNr, originReply.toString().c_str());
			break;
			}
		case WORKAROUND_IGNORE: // don't do kernel call, leave acc alone
			return;
		case WORKAROUND_STILLCALL: // call kernel anyway
			break;
		case WORKAROUND_FAKE: // don't do kernel call, fake acc
			s->r_acc = make_reg(0, solution.value);
			return;
		default:
			error("unknown workaround type");
		}
	}


	// Call kernel function
	if (!kernelCall.subFunctionCount) {
		argv[-1] = make_reg(0, argc); // The first argument is argc
		addKernelCallToExecStack(s, kernelCallNr, -1, argc, argv);
		s->r_acc = kernelCall.function(s, argc, argv);

		if (g_sci->checkKernelBreakpoint(kernelCall.name))
			logKernelCall(&kernelCall, NULL, s, argc, argv, s->r_acc);
	} else {
		// Sub-functions available, check signature and call that one directly
		if (argc < 1)
			error("[VM] k%s[%x]: no subfunction ID parameter given", kernelCall.name, kernelCallNr);
		if (argv[0].isPointer())
			error("[VM] k%s[%x]: given subfunction ID is actually a pointer", kernelCall.name, kernelCallNr);

#ifdef ENABLE_SCI32
		// The Windows version of kShowMovie has subops, but the subop number
		// is put in the second parameter in SCI2.1+, even though every other
		// kcall with subops puts the subop in the first parameter. To allow use
		// of the normal subops system, we swap the arguments so the subop
		// number is in the usual place.
		if (getSciVersion() > SCI_VERSION_2 &&
			g_sci->getPlatform() == Common::kPlatformWindows &&
			strcmp(kernelCall.name, "ShowMovie") == 0) {
			assert(argc > 1);
			SWAP(argv[0], argv[1]);
		}
#endif

		const uint16 subId = argv[0].toUint16();
		// Skip over subfunction-id
		argc--;
		argv++;
		if (subId >= kernelCall.subFunctionCount)
			error("[VM] k%s: subfunction ID %d requested, but not available", kernelCall.name, subId);
		const KernelSubFunction &kernelSubCall = kernelCall.subFunctions[subId];
		if (kernelSubCall.signature && !kernel->signatureMatch(kernelSubCall.signature, argc, argv)) {
			// Signature mismatch
			SciCallOrigin originReply;
			SciWorkaroundSolution solution = trackOriginAndFindWorkaround(0, kernelSubCall.workarounds, &originReply);
			switch (solution.type) {
			case WORKAROUND_NONE: {
				Common::String signatureDetailsStr;
				kernel->signatureDebug(signatureDetailsStr, kernelSubCall.signature, argc, argv);
				int callNameLen = strlen(kernelCall.name);
				if (strncmp(kernelCall.name, kernelSubCall.name, callNameLen) == 0) {
					const char *subCallName = kernelSubCall.name + callNameLen;
					error("\n%s[VM] k%s(%s): signature mismatch in %s",
						signatureDetailsStr.c_str(), kernelCall.name, subCallName,
						originReply.toString().c_str());
				}
				error("\n%s[VM] k%s: signature mismatch in %s",
					signatureDetailsStr.c_str(), kernelSubCall.name,
					originReply.toString().c_str());
				break;
			}
			case WORKAROUND_IGNORE: // don't do kernel call, leave acc alone
				return;
			case WORKAROUND_STILLCALL: // call kernel anyway
				break;
			case WORKAROUND_FAKE: // don't do kernel call, fake acc
				s->r_acc = make_reg(0, solution.value);
				return;
			default:
				error("unknown workaround type");
			}
		}
		if (!kernelSubCall.function)
			error("[VM] k%s: subfunction ID %d requested, but not available", kernelCall.name, subId);
		argv[-1] = make_reg(0, argc); // The first argument is argc
		addKernelCallToExecStack(s, kernelCallNr, subId, argc, argv);
		s->r_acc = kernelSubCall.function(s, argc, argv);

		if (g_sci->checkKernelBreakpoint(kernelSubCall.name))
			logKernelCall(&kernelCall, &kernelSubCall, s, argc, argv, s->r_acc);
	}

	// Remove callk stack frame again, if there's still an execution stack
	if (s->_executionStack.begin() != s->_executionStack.end())
		s->_executionStack.pop_back();
}

int readPMachineInstruction(const byte *src, byte &extOpcode, int16 opparams[4]) {
	uint offset = 0;
	extOpcode = src[offset++]; // Get "extended" opcode (lower bit has special meaning)
	const byte opcode = extOpcode >> 1;	// get the actual opcode

	memset(opparams, 0, 4*sizeof(int16));

	for (int i = 0; g_sci->_opcode_formats[opcode][i]; ++i) {
		//debugN("Opcode: 0x%x, Opnumber: 0x%x, temp: %d\n", opcode, opcode, temp);
		assert(i < 3);
		switch (g_sci->_opcode_formats[opcode][i]) {

		case Script_Byte:
			opparams[i] = src[offset++];
			break;
		case Script_SByte:
			opparams[i] = (int8)src[offset++];
			break;

		case Script_Word:
			opparams[i] = READ_SCI11ENDIAN_UINT16(src + offset);
			offset += 2;
			break;
		case Script_SWord:
			opparams[i] = (int16)READ_SCI11ENDIAN_UINT16(src + offset);
			offset += 2;
			break;

		case Script_Variable:
		case Script_Property:

		case Script_Local:
		case Script_Temp:
		case Script_Global:
		case Script_Param:

		case Script_Offset:
			if (extOpcode & 1) {
				opparams[i] = src[offset++];
			} else {
				opparams[i] = READ_SCI11ENDIAN_UINT16(src + offset);
				offset += 2;
			}
			break;

		case Script_SVariable:
		case Script_SRelative:
			if (extOpcode & 1) {
				opparams[i] = (int8)src[offset++];
			} else {
				opparams[i] = (int16)READ_SCI11ENDIAN_UINT16(src + offset);
				offset += 2;
			}
			break;

		case Script_None:
		case Script_End:
			break;

		case Script_Invalid:
		default:
			error("opcode %02x: Invalid", extOpcode);
		}
	}

	// Special handling of the op_line opcode
	if (opcode == op_pushSelf) {
		// Compensate for a bug in non-Sierra compilers, which seem to generate
		// pushSelf instructions with the low bit set. This makes the following
		// heuristic fail and leads to endless loops and crashes. Our
		// interpretation of this seems correct, as other SCI tools, like for
		// example SCI Viewer, have issues with these scripts (e.g. script 999
		// in Circus Quest). Fixes bug #3038686.
		if (!(extOpcode & 1) || g_sci->getGameId() == GID_FANMADE) {
			// op_pushSelf: no adjustment necessary
		} else {
			// Debug opcode op_file, skip null-terminated string (file name)
			while (src[offset++]) {}
		}
	}

	return offset;
}

uint32 findOffset(const int16 relOffset, const Script *scr, const uint32 pcOffset) {
	uint32 offset;

	switch (g_sci->_features->detectLofsType()) {
	case SCI_VERSION_0_EARLY:
		offset = (uint16)pcOffset + relOffset;
		break;
	case SCI_VERSION_1_MIDDLE:
		offset = relOffset;
		break;
	case SCI_VERSION_1_1:
		offset = relOffset + scr->getHeapOffset();
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_3:
		// In theory this can break if the variant with a one-byte argument is
		// used. For now, assume it doesn't happen.
		offset = scr->relocateOffsetSci3(pcOffset - 2);
		break;
#endif
	default:
		error("Unknown lofs type");
	}

	return offset;
}

void run_vm(EngineState *s) {
	assert(s);

	int temp;
	reg_t r_temp; // Temporary register
	StackPtr s_temp; // Temporary stack pointer
	int16 opparams[4]; // opcode parameters

	VmHooks vmHooks;

	s->r_rest = 0;	// &rest adjusts the parameter count by this value
	// Current execution data:
	s->xs = &(s->_executionStack.back());
	ExecStack *xs_new = NULL;
	Object *obj = s->_segMan->getObject(s->xs->objp);
	Script *scr = 0;
	Script *local_script = s->_segMan->getScriptIfLoaded(s->xs->local_segment);
	int old_executionStackBase = s->executionStackBase;
	// Used to detect the stack bottom, for "physical" returns

	if (!local_script)
		error("run_vm(): program counter gone astray (local_script pointer is null)");

	s->executionStackBase = s->_executionStack.size() - 1;

	s->variablesSegment[VAR_TEMP] = s->variablesSegment[VAR_PARAM] = s->_segMan->findSegmentByType(SEG_TYPE_STACK);
	s->variablesBase[VAR_TEMP] = s->variablesBase[VAR_PARAM] = s->stack_base;

	s->_executionStackPosChanged = true; // Force initialization

#ifdef ABORT_ON_INFINITE_LOOP
	byte prevOpcode = 0xFF;
#endif

	while (1) {
		vmHooks.vm_hook_before_exec(s);

		int var_type; // See description below
		int var_number;

		g_sci->_debugState.old_pc_offset = s->xs->addr.pc.getOffset();
		g_sci->_debugState.old_sp = s->xs->sp;

		if (s->abortScriptProcessing != kAbortNone)
			return; // Stop processing

		if (s->_executionStackPosChanged) {
			scr = s->_segMan->getScriptIfLoaded(s->xs->addr.pc.getSegment());
			if (!scr)
				error("No script in segment %d",  s->xs->addr.pc.getSegment());
			s->xs = &(s->_executionStack.back());
			s->_executionStackPosChanged = false;

			obj = s->_segMan->getObject(s->xs->objp);
			local_script = s->_segMan->getScriptIfLoaded(s->xs->local_segment);
			if (!local_script) {
				error("Could not find local script from segment %x", s->xs->local_segment);
			} else {
				s->variablesSegment[VAR_LOCAL] = local_script->getLocalsSegment();
				s->variablesBase[VAR_LOCAL] = s->variables[VAR_LOCAL] = local_script->getLocalsBegin();
				s->variablesMax[VAR_LOCAL] = local_script->getLocalsCount();
				s->variablesMax[VAR_TEMP] = s->xs->sp - s->xs->fp;
				s->variablesMax[VAR_PARAM] = s->xs->argc + 1;
			}
			s->variables[VAR_TEMP] = s->xs->fp;
			s->variables[VAR_PARAM] = s->xs->variables_argp;
		}

		if (s->abortScriptProcessing != kAbortNone)
			return; // Stop processing

		g_sci->checkAddressBreakpoint(s->xs->addr.pc);

		// Debug if this has been requested:
		// TODO: re-implement sci_debug_flags
		if (g_sci->_debugState.debugging /* sci_debug_flags*/) {
			g_sci->scriptDebug();
			g_sci->_debugState.breakpointWasHit = false;
		}
		Console *con = g_sci->getSciDebugger();
		con->onFrame();

		if (s->xs->sp < s->xs->fp)
			error("run_vm(): stack underflow, sp: %04x:%04x, fp: %04x:%04x",
			PRINT_REG(*s->xs->sp), PRINT_REG(*s->xs->fp));

		s->variablesMax[VAR_TEMP] = s->xs->sp - s->xs->fp;

		if (s->xs->addr.pc.getOffset() >= scr->getBufSize())
			error("run_vm(): program counter gone astray, addr: %d, code buffer size: %d",
			s->xs->addr.pc.getOffset(), scr->getBufSize());

		// Get opcode
		byte extOpcode;
		if (!vmHooks.isActive(s))
			s->xs->addr.pc.incOffset(readPMachineInstruction(scr->getBuf(s->xs->addr.pc.getOffset()), extOpcode, opparams));
		else {
			int offset = readPMachineInstruction(vmHooks.data(), extOpcode, opparams);
			vmHooks.advance(offset);
		}
		const byte opcode = extOpcode >> 1;
		//debug("%s: %d, %d, %d, %d, acc = %04x:%04x, script %d, local script %d", opcodeNames[opcode], opparams[0], opparams[1], opparams[2], opparams[3], PRINT_REG(s->r_acc), scr->getScriptNumber(), local_script->getScriptNumber());

#ifdef ABORT_ON_INFINITE_LOOP
		if (prevOpcode != 0xFF) {
			if (prevOpcode == op_eq_  || prevOpcode == op_ne_  ||
				prevOpcode == op_gt_  || prevOpcode == op_ge_  ||
				prevOpcode == op_lt_  || prevOpcode == op_le_  ||
				prevOpcode == op_ugt_ || prevOpcode == op_uge_ ||
				prevOpcode == op_ult_ || prevOpcode == op_ule_) {
				if (opcode == op_jmp)
					error("Infinite loop detected in script %d", scr->getScriptNumber());
			}
		}

		prevOpcode = opcode;
#endif

		switch (opcode) {

		case op_bnot: // 0x00 (00)
			// Binary not
			s->r_acc = make_reg(0, 0xffff ^ s->r_acc.requireUint16());
			break;

		case op_add: // 0x01 (01)
			s->r_acc = POP32() + s->r_acc;
			break;

		case op_sub: // 0x02 (02)
			s->r_acc = POP32() - s->r_acc;
			break;

		case op_mul: // 0x03 (03)
			s->r_acc = POP32() * s->r_acc;
			break;

		case op_div: // 0x04 (04)
			// we check for division by 0 inside the custom reg_t division operator
			s->r_acc = POP32() / s->r_acc;
			break;

		case op_mod: // 0x05 (05)
			// we check for division by 0 inside the custom reg_t modulo operator
			s->r_acc = POP32() % s->r_acc;
			break;

		case op_shr: // 0x06 (06)
			// Shift right logical
			s->r_acc = POP32() >> s->r_acc;
			break;

		case op_shl: // 0x07 (07)
			// Shift left logical
			s->r_acc = POP32() << s->r_acc;
			break;

		case op_xor: // 0x08 (08)
			s->r_acc = POP32() ^ s->r_acc;
			break;

		case op_and: // 0x09 (09)
			s->r_acc = POP32() & s->r_acc;
			break;

		case op_or: // 0x0a (10)
			s->r_acc = POP32() | s->r_acc;
			break;

		case op_neg:	// 0x0b (11)
			s->r_acc = make_reg(0, -s->r_acc.requireSint16());
			break;

		case op_not: // 0x0c (12)
			s->r_acc = make_reg(0, !(s->r_acc.getOffset() || s->r_acc.getSegment()));
			// Must allow pointers to be negated, as this is used for checking whether objects exist
			break;

		case op_eq_: // 0x0d (13)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() == s->r_acc);
			break;

		case op_ne_: // 0x0e (14)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() != s->r_acc);
			break;

		case op_gt_: // 0x0f (15)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() > s->r_acc);
			break;

		case op_ge_: // 0x10 (16)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() >= s->r_acc);
			break;

		case op_lt_: // 0x11 (17)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() < s->r_acc);
			break;

		case op_le_: // 0x12 (18)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32() <= s->r_acc);
			break;

		case op_ugt_: // 0x13 (19)
			// > (unsigned)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32().gtU(s->r_acc));
			break;

		case op_uge_: // 0x14 (20)
			// >= (unsigned)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32().geU(s->r_acc));
			break;

		case op_ult_: // 0x15 (21)
			// < (unsigned)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32().ltU(s->r_acc));
			break;

		case op_ule_: // 0x16 (22)
			// <= (unsigned)
			s->r_prev = s->r_acc;
			s->r_acc  = make_reg(0, POP32().leU(s->r_acc));
			break;

		case op_bt: // 0x17 (23)
			// Branch relative if true
			if (!vmHooks.isActive(s)) {
				if (s->r_acc.getOffset() || s->r_acc.getSegment())
					s->xs->addr.pc.incOffset(opparams[0]);

				if (s->xs->addr.pc.getOffset() >= local_script->getScriptSize())
					error("[VM] op_bt: request to jump past the end of script %d (offset %d, script is %d bytes)",
						local_script->getScriptNumber(), s->xs->addr.pc.getOffset(), local_script->getScriptSize());
			} else {
				if (s->r_acc.getOffset() || s->r_acc.getSegment())
					vmHooks.advance(opparams[0]);
			}
			break;

		case op_bnt: // 0x18 (24)
			// Branch relative if not true
			if (!vmHooks.isActive(s)) {
				if (!(s->r_acc.getOffset() || s->r_acc.getSegment()))
					s->xs->addr.pc.incOffset(opparams[0]);

				if (s->xs->addr.pc.getOffset() >= local_script->getScriptSize())
					error("[VM] op_bnt: request to jump past the end of script %d (offset %d, script is %d bytes)",
						local_script->getScriptNumber(), s->xs->addr.pc.getOffset(), local_script->getScriptSize());
			} else {
				if (!(s->r_acc.getOffset() || s->r_acc.getSegment()))
					vmHooks.advance(opparams[0]);
			}
			break;

		case op_jmp: // 0x19 (25)
			if (!vmHooks.isActive(s)) {
				s->xs->addr.pc.incOffset(opparams[0]);

				if (s->xs->addr.pc.getOffset() >= local_script->getScriptSize())
					error("[VM] op_jmp: request to jump past the end of script %d (offset %d, script is %d bytes)",
						local_script->getScriptNumber(), s->xs->addr.pc.getOffset(), local_script->getScriptSize());
			} else {
				vmHooks.advance(opparams[0]);
			}
			break;

		case op_ldi: // 0x1a (26)
			// Load data immediate
			s->r_acc = make_reg(0, opparams[0]);
			break;

		case op_push: // 0x1b (27)
			// Push to stack
			PUSH32(s->r_acc);
			break;

		case op_pushi: // 0x1c (28)
			// Push immediate
			PUSH(opparams[0]);
			break;

		case op_toss: // 0x1d (29)
			// TOS (Top Of Stack) subtract
			s->xs->sp--;
			break;

		case op_dup: // 0x1e (30)
			// Duplicate TOD (Top Of Stack) element
			r_temp = s->xs->sp[-1];
			PUSH32(r_temp);
			break;

		case op_link: // 0x1f (31)
			// We shouldn't initialize temp variables at all
			//  We put special segment 0xFFFF in there, so that uninitialized reads can get detected
			for (int i = 0; i < opparams[0]; i++)
				s->xs->sp[i] = make_reg(kUninitializedSegment, 0);

			s->xs->sp += opparams[0];
			break;

		case op_call: { // 0x20 (32)
			// Call a script subroutine
			int argc = (opparams[1] >> 1) // Given as offset, but we need count
			           + 1 + s->r_rest;
			StackPtr call_base = s->xs->sp - argc;

			uint32 localCallOffset = s->xs->addr.pc.getOffset() + opparams[0];

			int final_argc = (call_base->requireUint16()) + s->r_rest;
			call_base[0] = make_reg(0, final_argc); // The first argument is argc
			ExecStack xstack(s->xs->objp, s->xs->objp, s->xs->sp,
							final_argc, call_base,
							s->xs->local_segment, make_reg32(s->xs->addr.pc.getSegment(), localCallOffset),
							NULL_SELECTOR, -1, -1, -1, localCallOffset, s->_executionStack.size() - 1,
							EXEC_STACK_TYPE_CALL);

			s->_executionStack.push_back(xstack);
			xs_new = &(s->_executionStack.back());

			s->r_rest = 0; // Used up the &rest adjustment
			s->xs->sp = call_base;

			s->_executionStackPosChanged = true;
			break;
		}

		case op_callk: { // 0x21 (33)
			// Run the garbage collector, if needed
			if (s->gcCountDown-- <= 0) {
				s->gcCountDown = s->scriptGCInterval;
				run_gc(s);
			}

			// Call kernel function
			s->xs->sp -= (opparams[1] >> 1) + 1;

			bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
			if (!oldScriptHeader)
				s->xs->sp -= s->r_rest;

			int argc = s->xs->sp[0].requireUint16();

			if (!oldScriptHeader)
				argc += s->r_rest;

			callKernelFunc(s, opparams[0], argc);

			if (!oldScriptHeader)
				s->r_rest = 0;

			// Calculate xs again: The kernel function might
			// have spawned a new VM

			xs_new = &(s->_executionStack.back());
			s->_executionStackPosChanged = true;

			// If a game is being loaded, stop processing
			if (s->abortScriptProcessing != kAbortNone)
				return; // Stop processing

			break;
		}

		case op_callb: // 0x22 (34)
			// Call base script
			temp = ((opparams[1] >> 1) + s->r_rest + 1);
			s_temp = s->xs->sp;
			s->xs->sp -= temp;

			s->xs->sp[0].incOffset(s->r_rest);
			xs_new = execute_method(s, 0, opparams[0], s_temp, s->xs->objp,
									s->xs->sp[0].getOffset(), s->xs->sp);
			s->r_rest = 0; // Used up the &rest adjustment
			if (xs_new)    // in case of error, keep old stack
				s->_executionStackPosChanged = true;
			break;

		case op_calle: // 0x23 (35)
			// Call external script
			temp = ((opparams[2] >> 1) + s->r_rest + 1);
			s_temp = s->xs->sp;
			s->xs->sp -= temp;

			s->xs->sp[0].incOffset(s->r_rest);
			xs_new = execute_method(s, opparams[0], opparams[1], s_temp, s->xs->objp,
									s->xs->sp[0].getOffset(), s->xs->sp);
			s->r_rest = 0; // Used up the &rest adjustment
			if (xs_new)  // in case of error, keep old stack
				s->_executionStackPosChanged = true;
			break;

		case op_ret: // 0x24 (36)
			// Return from an execution loop started by call, calle, callb, send, self or super
			do {
				StackPtr old_sp2 = s->xs->sp;
				StackPtr old_fp = s->xs->fp;
				ExecStack *old_xs = &(s->_executionStack.back());

				if ((int)s->_executionStack.size() - 1 == s->executionStackBase) { // Have we reached the base?
					s->executionStackBase = old_executionStackBase; // Restore stack base

					s->_executionStack.pop_back();

					s->_executionStackPosChanged = true;
					return; // "Hard" return
				}

				if (old_xs->type == EXEC_STACK_TYPE_VARSELECTOR) {
					// varselector access?
					reg_t *var = old_xs->getVarPointer(s->_segMan);
					if (old_xs->argc) { // write?
						*var = old_xs->variables_argp[1];

#ifdef ENABLE_SCI32
						updateInfoFlagViewVisible(s->_segMan->getObject(old_xs->addr.varp.obj), old_xs->addr.varp.varindex);
#endif
					} else // No, read
						s->r_acc = *var;
				}

				// Not reached the base, so let's do a soft return
				s->_executionStack.pop_back();
				s->_executionStackPosChanged = true;
				s->xs = &(s->_executionStack.back());

				if (s->xs->sp == CALL_SP_CARRY // Used in sends to 'carry' the stack pointer
				        || s->xs->type != EXEC_STACK_TYPE_CALL) {
					s->xs->sp = old_sp2;
					s->xs->fp = old_fp;
				}

			} while (s->xs->type == EXEC_STACK_TYPE_VARSELECTOR);
			// Iterate over all varselector accesses
			s->_executionStackPosChanged = true;
			xs_new = s->xs;

			break;

		case op_send: // 0x25 (37)
			// Send for one or more selectors
			s_temp = s->xs->sp;
			s->xs->sp -= ((opparams[0] >> 1) + s->r_rest); // Adjust stack

			s->xs->sp[1].incOffset(s->r_rest);
			xs_new = send_selector(s, s->r_acc, s->r_acc, s_temp,
									(int)(opparams[0] >> 1) + (uint16)s->r_rest, s->xs->sp);

			if (xs_new && xs_new != s->xs)
				s->_executionStackPosChanged = true;

			s->r_rest = 0;

			break;

		case op_info: // (38)
			if (getSciVersion() < SCI_VERSION_3)
				error("Dummy opcode 0x%x called", opcode);	// should never happen

			if (!(extOpcode & 1))
				s->r_acc = obj->getInfoSelector();
			else
				PUSH32(obj->getInfoSelector());
			break;

		case op_superP: // (39)
			if (getSciVersion() < SCI_VERSION_3)
				error("Dummy opcode 0x%x called", opcode);	// should never happen

			if (!(extOpcode & 1))
				s->r_acc = obj->getSuperClassSelector();
			else
				PUSH32(obj->getSuperClassSelector());
			break;

		case op_class: // 0x28 (40)
			// Get class address
			s->r_acc = s->_segMan->getClassAddress((unsigned)opparams[0], SCRIPT_GET_LOCK,
											s->xs->addr.pc.getSegment());
			break;

		case 0x29: // (41)
			error("Dummy opcode 0x%x called", opcode);	// should never happen
			break;

		case op_self: // 0x2a (42)
			// Send to self
			s_temp = s->xs->sp;
			s->xs->sp -= ((opparams[0] >> 1) + s->r_rest); // Adjust stack

			s->xs->sp[1].incOffset(s->r_rest);
			xs_new = send_selector(s, s->xs->objp, s->xs->objp,
									s_temp, (int)(opparams[0] >> 1) + (uint16)s->r_rest,
									s->xs->sp);

			if (xs_new && xs_new != s->xs)
				s->_executionStackPosChanged = true;

			s->r_rest = 0;
			break;

		case op_super: // 0x2b (43)
			// Send to any class
			r_temp = s->_segMan->getClassAddress(opparams[0], SCRIPT_GET_LOAD, s->xs->addr.pc.getSegment());

			if (!r_temp.isPointer())
				error("[VM]: Invalid superclass in object");
			else {
				// SCI3 sets r_acc to whatever was in EAX at the start of a
				// send. In the case of a super call this is the object ID of
				// the superclass, as determined by the interpreter, rather than
				// by the game scripts
				if (getSciVersion() == SCI_VERSION_3) {
					s->r_acc = r_temp;
				}

				s_temp = s->xs->sp;
				s->xs->sp -= ((opparams[1] >> 1) + s->r_rest); // Adjust stack

				s->xs->sp[1].incOffset(s->r_rest);
				xs_new = send_selector(s, r_temp, s->xs->objp, s_temp,
										(int)(opparams[1] >> 1) + (uint16)s->r_rest,
										s->xs->sp);

				if (xs_new && xs_new != s->xs)
					s->_executionStackPosChanged = true;

				s->r_rest = 0;
			}

			break;

		case op_rest: // 0x2c (44)
			// Pushes all or part of the parameter variable list on the stack
			// Index 0 is argc, so normally this will be called as &rest 1 to
			// forward all the arguments.
			temp = (uint16) opparams[0]; // First argument
			s->r_rest = MAX<int16>(s->xs->argc - temp + 1, 0); // +1 because temp counts the paramcount while argc doesn't

			for (; temp <= s->xs->argc; temp++)
				PUSH32(s->xs->variables_argp[temp]);

			break;

		case op_lea: // 0x2d (45)
			// Load Effective Address
			temp = (uint16) opparams[0] >> 1;
			var_number = temp & 0x03; // Get variable type

			// Get variable block offset
			r_temp.setSegment(s->variablesSegment[var_number]);
			r_temp.setOffset(s->variables[var_number] - s->variablesBase[var_number]);

			if (temp & 0x08)  // Add accumulator offset if requested
				r_temp.incOffset(s->r_acc.requireSint16());

			r_temp.incOffset(opparams[1]);  // Add index
			r_temp.setOffset(r_temp.getOffset() * 2); // variables are 16 bit
			// That's the immediate address now
			s->r_acc = r_temp;
			break;


		case op_selfID: // 0x2e (46)
			// Get 'self' identity
			s->r_acc = s->xs->objp;
			break;

		case 0x2f: // (47)
			error("Dummy opcode 0x%x called", opcode);	// should never happen
			break;

		case op_pprev: // 0x30 (48)
			// Pushes the value of the prev register, set by the last comparison
			// bytecode (eq?, lt?, etc.), on the stack
			PUSH32(s->r_prev);
			break;

		case op_pToa: // 0x31 (49)
			// Property To Accumulator
			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORREAD) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    validate_property(s, obj, opparams[0]), NULL_REG,
				                    s->_segMan, BREAK_SELECTORREAD);
			}
			s->r_acc = validate_property(s, obj, opparams[0]);
			break;

		case op_aTop: // 0x32 (50)
			{
			// Accumulator To Property
			reg_t &opProperty = validate_property(s, obj, opparams[0]);
			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORWRITE) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    opProperty, s->r_acc,
				                    s->_segMan, BREAK_SELECTORWRITE);
			}

			opProperty = s->r_acc;
#ifdef ENABLE_SCI32
			updateInfoFlagViewVisible(obj, opparams[0], true);
#endif
			break;
		}

		case op_pTos: // 0x33 (51)
			{
			// Property To Stack
			reg_t value = validate_property(s, obj, opparams[0]);
			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORREAD) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    value, NULL_REG,
				                    s->_segMan, BREAK_SELECTORREAD);
			}
			PUSH32(value);
			break;
		}

		case op_sTop: // 0x34 (52)
			{
			// Stack To Property
			reg_t newValue = POP32();
			reg_t &opProperty = validate_property(s, obj, opparams[0]);
			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORWRITE) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    opProperty, newValue,
				                    s->_segMan, BREAK_SELECTORWRITE);
			}
			opProperty = newValue;
#ifdef ENABLE_SCI32
			updateInfoFlagViewVisible(obj, opparams[0], true);
#endif
			break;
		}

		case op_ipToa: // 0x35 (53)
		case op_dpToa: // 0x36 (54)
		case op_ipTos: // 0x37 (55)
		case op_dpTos: // 0x38 (56)
			{
			// Increment/decrement a property and copy to accumulator,
			// or push to stack
			reg_t &opProperty = validate_property(s, obj, opparams[0]);
			reg_t oldValue = opProperty;

			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORREAD) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    oldValue, NULL_REG,
				                    s->_segMan, BREAK_SELECTORREAD);
			}

			if (opcode & 1)
				opProperty += 1;
			else
				opProperty -= 1;

			if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORWRITE) {
				debugPropertyAccess(obj, s->xs->objp, opparams[0], NULL_SELECTOR,
				                    oldValue, opProperty,
				                    s->_segMan, BREAK_SELECTORWRITE);
			}

#ifdef ENABLE_SCI32
			updateInfoFlagViewVisible(obj, opparams[0], true);
#endif
			if (opcode == op_ipToa || opcode == op_dpToa)
				s->r_acc = opProperty;
			else
				PUSH32(opProperty);
			break;
		}

		case op_lofsa: // 0x39 (57)
		case op_lofss: { // 0x3a (58)
			// Load offset to accumulator or push to stack

			r_temp.setSegment(s->xs->addr.pc.getSegment());
			r_temp.setOffset(findOffset(opparams[0], local_script, s->xs->addr.pc.getOffset()));
			if (r_temp.getOffset() >= scr->getBufSize())
				error("VM: lofsa/lofss operation overflowed: %04x:%04x beyond end"
						  " of script (at %04x)", PRINT_REG(r_temp), scr->getBufSize());

			if (opcode == op_lofsa)
				s->r_acc = r_temp;
			else
				PUSH32(r_temp);
			break;
		}

		case op_push0: // 0x3b (59)
			PUSH(0);
			break;

		case op_push1: // 0x3c (60)
			PUSH(1);
			break;

		case op_push2: // 0x3d (61)
			PUSH(2);
			break;

		case op_pushSelf: // 0x3e (62)
			// Compensate for a bug in non-Sierra compilers, which seem to generate
			// pushSelf instructions with the low bit set. This makes the following
			// heuristic fail and leads to endless loops and crashes. Our
			// interpretation of this seems correct, as other SCI tools, like for
			// example SCI Viewer, have issues with these scripts (e.g. script 999
			// in Circus Quest). Fixes bug #3038686.
			if (!(extOpcode & 1) || g_sci->getGameId() == GID_FANMADE) {
				PUSH32(s->xs->objp);
			} else {
				// Debug opcode op_file
			}
			break;

		case op_line: // 0x3f (63)
			// Debug opcode (line number)
			//debug("Script %d, line %d", scr->getScriptNumber(), opparams[0]);
			break;

		case op_lag: // 0x40 (64)
		case op_lal: // 0x41 (65)
		case op_lat: // 0x42 (66)
		case op_lap: // 0x43 (67)
			// Load global, local, temp or param variable into the accumulator
		case op_lagi: // 0x48 (72)
		case op_lali: // 0x49 (73)
		case op_lati: // 0x4a (74)
		case op_lapi: // 0x4b (75)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_lagi ? s->r_acc.requireSint16() : 0);
			s->r_acc = read_var(s, var_type, var_number);
			break;

		case op_lsg: // 0x44 (68)
		case op_lsl: // 0x45 (69)
		case op_lst: // 0x46 (70)
		case op_lsp: // 0x47 (71)
			// Load global, local, temp or param variable into the stack
		case op_lsgi: // 0x4c (76)
		case op_lsli: // 0x4d (77)
		case op_lsti: // 0x4e (78)
		case op_lspi: // 0x4f (79)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_lsgi ? s->r_acc.requireSint16() : 0);
			PUSH32(read_var(s, var_type, var_number));
			break;

		case op_sag: // 0x50 (80)
		case op_sal: // 0x51 (81)
		case op_sat: // 0x52 (82)
		case op_sap: // 0x53 (83)
			// Save the accumulator into the global, local, temp or param variable
		case op_sagi: // 0x58 (88)
		case op_sali: // 0x59 (89)
		case op_sati: // 0x5a (90)
		case op_sapi: // 0x5b (91)
			// Save the accumulator into the global, local, temp or param variable,
			// using the accumulator as an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_sagi ? s->r_acc.requireSint16() : 0);
			if (opcode >= op_sagi)	// load the actual value to store in the accumulator
				s->r_acc = POP32();
			write_var(s, var_type, var_number, s->r_acc);
			break;

		case op_ssg: // 0x54 (84)
		case op_ssl: // 0x55 (85)
		case op_sst: // 0x56 (86)
		case op_ssp: // 0x57 (87)
			// Save the stack into the global, local, temp or param variable
		case op_ssgi: // 0x5c (92)
		case op_ssli: // 0x5d (93)
		case op_ssti: // 0x5e (94)
		case op_sspi: // 0x5f (95)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_ssgi ? s->r_acc.requireSint16() : 0);
			write_var(s, var_type, var_number, POP32());
			break;

		case op_plusag: // 0x60 (96)
		case op_plusal: // 0x61 (97)
		case op_plusat: // 0x62 (98)
		case op_plusap: // 0x63 (99)
			// Increment the global, local, temp or param variable and save it
			// to the accumulator
		case op_plusagi: // 0x68 (104)
		case op_plusali: // 0x69 (105)
		case op_plusati: // 0x6a (106)
		case op_plusapi: // 0x6b (107)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_plusagi ? s->r_acc.requireSint16() : 0);
			s->r_acc = read_var(s, var_type, var_number) + 1;
			write_var(s, var_type, var_number, s->r_acc);
			break;

		case op_plussg: // 0x64 (100)
		case op_plussl: // 0x65 (101)
		case op_plusst: // 0x66 (102)
		case op_plussp: // 0x67 (103)
			// Increment the global, local, temp or param variable and save it
			// to the stack
		case op_plussgi: // 0x6c (108)
		case op_plussli: // 0x6d (109)
		case op_plussti: // 0x6e (110)
		case op_plusspi: // 0x6f (111)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_plussgi ? s->r_acc.requireSint16() : 0);
			r_temp = read_var(s, var_type, var_number) + 1;
			PUSH32(r_temp);
			write_var(s, var_type, var_number, r_temp);
			break;

		case op_minusag: // 0x70 (112)
		case op_minusal: // 0x71 (113)
		case op_minusat: // 0x72 (114)
		case op_minusap: // 0x73 (115)
			// Decrement the global, local, temp or param variable and save it
			// to the accumulator
		case op_minusagi: // 0x78 (120)
		case op_minusali: // 0x79 (121)
		case op_minusati: // 0x7a (122)
		case op_minusapi: // 0x7b (123)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_minusagi ? s->r_acc.requireSint16() : 0);
			s->r_acc = read_var(s, var_type, var_number) - 1;
			write_var(s, var_type, var_number, s->r_acc);
			break;

		case op_minussg: // 0x74 (116)
		case op_minussl: // 0x75 (117)
		case op_minusst: // 0x76 (118)
		case op_minussp: // 0x77 (119)
			// Decrement the global, local, temp or param variable and save it
			// to the stack
		case op_minussgi: // 0x7c (124)
		case op_minussli: // 0x7d (125)
		case op_minussti: // 0x7e (126)
		case op_minusspi: // 0x7f (127)
			// Same as the 4 ones above, except that the accumulator is used as
			// an additional index
			var_type = opcode & 0x3; // Gets the variable type: g, l, t or p
			var_number = opparams[0] + (opcode >= op_minussgi ? s->r_acc.requireSint16() : 0);
			r_temp = read_var(s, var_type, var_number) - 1;
			PUSH32(r_temp);
			write_var(s, var_type, var_number, r_temp);
			break;

		default:
			error("run_vm(): illegal opcode %x", opcode);

		} // switch (opcode)

		if (s->_executionStackPosChanged) // Force initialization
			s->xs = xs_new;

		if (s->xs != &(s->_executionStack.back())) {
			error("xs is stale (%p vs %p); last command was %02x",
					(void *)s->xs, (void *)&(s->_executionStack.back()),
					opcode);
		}
		++s->scriptStepCounter;
	}
}

reg_t *ObjVarRef::getPointer(SegManager *segMan) const {
	Object *o = segMan->getObject(obj);
	return o ? &o->getVariableRef(varindex) : 0;
}

reg_t *ExecStack::getVarPointer(SegManager *segMan) const {
	assert(type == EXEC_STACK_TYPE_VARSELECTOR);
	return addr.varp.getPointer(segMan);
}

} // End of namespace Sci
