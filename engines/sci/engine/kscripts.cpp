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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel_types.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t read_selector(SegManager *segMan, reg_t object, Selector selector_id, const char *file, int line) {
	ObjVarRef address;

	if (lookup_selector(segMan, object, selector_id, &address, NULL) != kSelectorVariable)
		return NULL_REG;
	else
		return *address.getPointer(segMan);
}

void write_selector(SegManager *segMan, reg_t object, Selector selector_id, reg_t value, const char *fname, int line) {
	ObjVarRef address;

	if ((selector_id < 0) || (selector_id > (int)((SciEngine*)g_engine)->getKernel()->getSelectorNamesSize())) {
		warning("Attempt to write to invalid selector %d of"
		         " object at %04x:%04x (%s L%d).", selector_id, PRINT_REG(object), fname, line);
		return;
	}

	if (lookup_selector(segMan, object, selector_id, &address, NULL) != kSelectorVariable)
		warning("Selector '%s' of object at %04x:%04x could not be"
		         " written to (%s L%d)", ((SciEngine*)g_engine)->getKernel()->getSelectorName(selector_id).c_str(), PRINT_REG(object), fname, line);
	else
		*address.getPointer(segMan) = value;
}

int invoke_selector(EngineState *s, reg_t object, int selector_id, SelectorInvocation noinvalid, int kfunct,
	StackPtr k_argp, int k_argc, const char *fname, int line, int argc, ...) {
	va_list argp;
	int i;
	int framesize = 2 + 1 * argc;
	reg_t address;
	int slc_type;
	StackPtr stackframe = k_argp + k_argc;

	stackframe[0] = make_reg(0, selector_id);  // The selector we want to call
	stackframe[1] = make_reg(0, argc); // Argument count

	slc_type = lookup_selector(s->segMan, object, selector_id, NULL, &address);

	if (slc_type == kSelectorNone) {
		warning("Selector '%s' of object at %04x:%04x could not be invoked (%s L%d)",
		         ((SciEngine*)g_engine)->getKernel()->getSelectorName(selector_id).c_str(), PRINT_REG(object), fname, line);
		if (noinvalid == kStopOnInvalidSelector)
			error("[Kernel] Not recoverable: VM was halted");
		return 1;
	}
	if (slc_type == kSelectorVariable) // Swallow silently
		return 0;

	va_start(argp, argc);
	for (i = 0; i < argc; i++) {
		reg_t arg = va_arg(argp, reg_t);
		stackframe[2 + i] = arg; // Write each argument
	}
	va_end(argp);

	// Write "kernel" call to the stack, for debugging:
	ExecStack *xstack;
	xstack = add_exec_stack_entry(s, NULL_REG, NULL, NULL_REG, k_argc, k_argp - 1, 0, NULL_REG,
	                              s->_executionStack.size()-1, SCI_XS_CALLEE_LOCALS);
	xstack->selector = -42 - kfunct; // Evil debugging hack to identify kernel function
	xstack->type = EXEC_STACK_TYPE_KERNEL;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, framesize, stackframe);

	xstack->sp += argc + 2;
	xstack->fp += argc + 2;

	run_vm(s, 0); // Start a new vm

	s->_executionStack.pop_back(); // Get rid of the extra stack entry

	return 0;
}

bool is_object(SegManager *segMan, reg_t object) {
	return obj_get(segMan, object) != NULL;
}

// Loads arbitrary resources of type 'restype' with resource numbers 'resnrs'
// This implementation ignores all resource numbers except the first one.
reg_t kLoad(EngineState *s, int, int argc, reg_t *argv) {
	int restype = argv[0].toUint16();
	int resnr = argv[1].toUint16();

	// Request to dynamically allocate hunk memory for later use
	if (restype == kResourceTypeMemory)
		return kalloc(s->segMan, "kLoad()", resnr);

	return make_reg(0, ((restype << 11) | resnr)); // Return the resource identifier as handle
}

reg_t kLock(EngineState *s, int, int argc, reg_t *argv) {
	int state = argc > 2 ? argv[2].toUint16() : 1;
	ResourceType type = (ResourceType)(argv[0].toUint16() & 0x7f);
	ResourceId id = ResourceId(type, argv[1].toUint16());

	Resource *which;

	switch (state) {
	case 1 :
		s->resMan->findResource(id, 1);
		break;
	case 0 :
		which = s->resMan->findResource(id, 0);

		if (which)
			s->resMan->unlockResource(which);
		else {
			if (id.type == kResourceTypeInvalid)
				warning("[resMan] Attempt to unlock resource %i of invalid type %i", id.number, type);
			else
				warning("[resMan] Attempt to unlock non-existant resource %s", id.toString().c_str());
		}
		break;
	}
	return s->r_acc;
}

// Unloads an arbitrary resource of type 'restype' with resource numbber 'resnr'
reg_t kUnLoad(EngineState *s, int, int argc, reg_t *argv) {
	int restype = argv[0].toUint16();
	reg_t resnr = argv[1];

	if (restype == kResourceTypeMemory)
		kfree(s->segMan, resnr);

	return s->r_acc;
}

reg_t kResCheck(EngineState *s, int, int argc, reg_t *argv) {
	Resource *res = NULL;
	ResourceType restype = (ResourceType)(argv[0].toUint16() & 0x7f);

	if ((restype == kResourceTypeAudio36) || (restype == kResourceTypeSync36)) {
		if (argc >= 6) {
			uint noun = argv[2].toUint16() & 0xff;
			uint verb = argv[3].toUint16() & 0xff;
			uint cond = argv[4].toUint16() & 0xff;
			uint seq = argv[5].toUint16() & 0xff;

			res = s->resMan->testResource(ResourceId(restype, argv[1].toUint16(), noun, verb, cond, seq));
		}
	} else {
		res = s->resMan->testResource(ResourceId(restype, argv[1].toUint16()));
	}

	return make_reg(0, res != NULL);
}

reg_t kClone(EngineState *s, int, int argc, reg_t *argv) {
	reg_t parent_addr = argv[0];
	Object *parent_obj = obj_get(s->segMan, parent_addr);
	reg_t clone_addr;
	Clone *clone_obj; // same as Object*

	if (!parent_obj) {
		error("Attempt to clone non-object/class at %04x:%04x failed", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	debugC(2, kDebugLevelMemory, "Attempting to clone from %04x:%04x\n", PRINT_REG(parent_addr));

	clone_obj = s->segMan->allocateClone(&clone_addr);

	if (!clone_obj) {
		error("Cloning %04x:%04x failed-- internal error", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	*clone_obj = *parent_obj;
	clone_obj->flags = 0;

	SciVersion version = s->resMan->sciVersion();	// for the selector defines

	// Mark as clone
	clone_obj->_variables[SCRIPT_INFO_SELECTOR].offset = SCRIPT_INFO_CLONE;
	clone_obj->_variables[SCRIPT_SPECIES_SELECTOR] = clone_obj->pos;
	if (IS_CLASS(parent_obj))
		clone_obj->_variables[SCRIPT_SUPERCLASS_SELECTOR] = parent_obj->pos;
	s->segMan->getScript(parent_obj->pos.segment)->incrementLockers();
	s->segMan->getScript(clone_obj->pos.segment)->incrementLockers();

	return clone_addr;
}

extern void _k_view_list_mark_free(EngineState *s, reg_t off);

reg_t kDisposeClone(EngineState *s, int, int argc, reg_t *argv) {
	SegManager *segMan = s->segMan;
	reg_t victim_addr = argv[0];
	Clone *victim_obj = obj_get(s->segMan, victim_addr);
	uint16 underBits;

	if (!victim_obj) {
		error("Attempt to dispose non-class/object at %04x:%04x",
		         PRINT_REG(victim_addr));
		return s->r_acc;
	}

	SciVersion version = s->resMan->sciVersion();	// for the selector defines

	if (victim_obj->_variables[SCRIPT_INFO_SELECTOR].offset != SCRIPT_INFO_CLONE) {
		//warning("Attempt to dispose something other than a clone at %04x", offset);
		// SCI silently ignores this behaviour; some games actually depend on it
		return s->r_acc;
	}

	underBits = GET_SEL32V(victim_addr, underBits);
	if (underBits) {
		warning("Clone %04x:%04x was cleared with underBits set", PRINT_REG(victim_addr));
	}
#if 0
	if (s->dyn_views) {  // Free any widget associated with the clone
		GfxWidget *widget = gfxw_set_id(gfxw_remove_ID(s->dyn_views, offset), GFXW_NO_ID);

		if (widget && s->bg_widgets)
			s->bg_widgets->add(GFXWC(s->bg_widgets), widget);
	}
#endif

	victim_obj->flags |= OBJECT_FLAG_FREED;

	_k_view_list_mark_free(s, victim_addr); // Free on view list, if neccessary

	return s->r_acc;
}

// Returns script dispatch address index in the supplied script
reg_t kScriptID(EngineState *s, int, int argc, reg_t *argv) {
	int script = argv[0].toUint16();
	int index = (argc > 1) ? argv[1].toUint16() : 0;

	if (argv[0].segment)
		return argv[0];

	SegmentId scriptSeg = s->segMan->getScriptSegment(script, SCRIPT_GET_LOAD);
	Script *scr;

	if (!scriptSeg)
		return NULL_REG;

	scr = s->segMan->getScript(scriptSeg);

	if (!scr->exports_nr) {
		// FIXME: Is this fatal? This occurs in SQ4CD
		warning("Script 0x%x does not have a dispatch table", script);
		return NULL_REG;
	}

	if (index > scr->exports_nr) {
		error("Dispatch index too big: %d > %d", index, scr->exports_nr);
		return NULL_REG;
	}

	return make_reg(scriptSeg, s->segMan->validateExportFunc(index, scriptSeg));
}

reg_t kDisposeScript(EngineState *s, int, int argc, reg_t *argv) {
	int script = argv[0].offset;

	// Work around QfG1 graveyard bug
	if (argv[0].segment)
		return s->r_acc;

	int id = s->segMan->getScriptSegment(script);
	Script *scr = s->segMan->getScriptIfLoaded(id);
	if (scr) {
		if (s->_executionStack.back().addr.pc.segment != id)
			scr->setLockers(1);

		// HACK for EcoQuest CD
		if (s->_gameName == "ecoquest" && script == 821) {
			warning("kDisposeScript hack for EcoQuest 1 CD: not disposing script 821");
			scr->setLockers(2);
		}
	}

	script_uninstantiate(s->segMan, script);
	s->_executionStackPosChanged = true;

	if (argc != 2) {
		return s->r_acc;
	} else {
		// This exists in the KQ5CD interpreter, but a test case hasn't been found yet
		warning("kDisposeScript called with 2 parameters, still untested");
		return argv[1];
	}
}

int is_heap_object(EngineState *s, reg_t pos) {
	Object *obj = obj_get(s->segMan, pos);
	return (obj != NULL && (!(obj->flags & OBJECT_FLAG_FREED)) && (!s->segMan->scriptIsMarkedAsDeleted(pos.segment)));
}

reg_t kIsObject(EngineState *s, int, int argc, reg_t *argv) {
	if (argv[0].offset == 0xffff) // Treated specially
		return NULL_REG;
	else
		return make_reg(0, is_heap_object(s, argv[0]));
}

reg_t kRespondsTo(EngineState *s, int, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int selector = argv[1].toUint16();

	return make_reg(0, is_heap_object(s, obj) && lookup_selector(s->segMan, obj, selector, NULL, NULL) != kSelectorNone);
}

} // End of namespace Sci
