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
#include "sci/engine/state.h"

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

int invoke_selector(EngineState *s, reg_t object, int selector_id, SelectorInvocation noinvalid, 
	StackPtr k_argp, int k_argc, const char *fname, int line, int argc, ...) {
	va_list argp;
	int i;
	int framesize = 2 + 1 * argc;
	reg_t address;
	int slc_type;
	StackPtr stackframe = k_argp + k_argc;

	stackframe[0] = make_reg(0, selector_id);  // The selector we want to call
	stackframe[1] = make_reg(0, argc); // Argument count

	slc_type = lookup_selector(s->_segMan, object, selector_id, NULL, &address);

	if (slc_type == kSelectorNone) {
		warning("Selector '%s' of object at %04x:%04x could not be invoked (%s L%d)",
		         s->_kernel->getSelectorName(selector_id).c_str(), PRINT_REG(object), fname, line);
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

	ExecStack *xstack;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, framesize, stackframe);

	xstack->sp += argc + 2;
	xstack->fp += argc + 2;

	run_vm(s, 0); // Start a new vm

	return 0;
}

static int _obj_locate_varselector(SegManager *segMan, Object *obj, Selector slc) {
	// Determines if obj explicitly defines slc as a varselector
	// Returns -1 if not found
	byte *buf;
	uint varnum;

	if (getSciVersion() < SCI_VERSION_1_1) {
		varnum = obj->getVarCount();
		int selector_name_offset = varnum * 2 + SCRIPT_SELECTOR_OFFSET;
		buf = obj->_baseObj + selector_name_offset;
	} else {
		if (!(obj->getInfoSelector().offset & SCRIPT_INFO_CLASS))
			obj = segMan->getObject(obj->getSuperClassSelector());

		buf = (byte *)obj->_baseVars;
		varnum = obj->getVariable(1).toUint16();
	}

	for (uint i = 0; i < varnum; i++)
		if (READ_LE_UINT16(buf + (i << 1)) == slc) // Found it?
			return i; // report success

	return -1; // Failed
}

SelectorType lookup_selector(SegManager *segMan, reg_t obj_location, Selector selector_id, ObjVarRef *varp, reg_t *fptr) {
	Object *obj = segMan->getObject(obj_location);
	int index;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// Early SCI versions used the LSB in the selector ID as a read/write
	// toggle, meaning that we must remove it for selector lookup.
	if (oldScriptHeader)
		selector_id &= ~1;

	if (!obj) {
		error("lookup_selector(): Attempt to send to non-object or invalid script. Address was %04x:%04x", 
				PRINT_REG(obj_location));
	}

	index = _obj_locate_varselector(segMan, obj, selector_id);

	if (index >= 0) {
		// Found it as a variable
		if (varp) {
			varp->obj = obj_location;
			varp->varindex = index;
		}
		return kSelectorVariable;
	} else {
		// Check if it's a method, with recursive lookup in superclasses
		while (obj) {
			index = obj->funcSelectorPosition(selector_id);
			if (index >= 0) {
				if (fptr)
					*fptr = obj->getFunction(index);

				return kSelectorMethod;
			} else {
				obj = segMan->getObject(obj->getSuperClassSelector());
			}
		}

		return kSelectorNone;
	}


//	return _lookup_selector_function(segMan, obj, selector_id, fptr);
}

} // End of namespace Sci
