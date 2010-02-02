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
#include "sci/engine/selector.h"

namespace Sci {

#if 1

#define FIND_SELECTOR(_slc_) _selectorCache._slc_ = findSelector(#_slc_)
#define FIND_SELECTOR2(_slc_, _slcstr_) _selectorCache._slc_ = findSelector(_slcstr_)

#else

// The defines below can be used to construct static selector tables for games which don't have
// a vocab.997 resource, by dumping the selector table from other similar versions or games
#define FIND_SELECTOR(_slc_) _selectorCache._slc_ = findSelector(#_slc_); \
	printf("\t{ \"%s\", %d },\n", #_slc_, _selectorCache._slc_)

#define FIND_SELECTOR2(_slc_, _slcstr_) _selectorCache._slc_ = findSelector(_slcstr_); \
	printf("\t{ \"%s\", %d },\n", _slcstr_, _selectorCache._slc_)

#endif

void Kernel::mapSelectors() {
	// species
	// superClass
	// -info-
	FIND_SELECTOR(y);
	FIND_SELECTOR(x);
	FIND_SELECTOR(view);
	FIND_SELECTOR(loop);
	FIND_SELECTOR(cel);
	FIND_SELECTOR(underBits);
	FIND_SELECTOR(nsTop);
	FIND_SELECTOR(nsLeft);
	FIND_SELECTOR(nsBottom);
	FIND_SELECTOR(lsTop);
	FIND_SELECTOR(lsLeft);
	FIND_SELECTOR(lsBottom);
	FIND_SELECTOR(lsRight);
	FIND_SELECTOR(nsRight);
	FIND_SELECTOR(signal);
	FIND_SELECTOR(illegalBits);
	FIND_SELECTOR(brTop);
	FIND_SELECTOR(brLeft);
	FIND_SELECTOR(brBottom);
	FIND_SELECTOR(brRight);
	// name
	// key
	// time
	FIND_SELECTOR(text);
	FIND_SELECTOR(elements);
	// color
	// back
	FIND_SELECTOR(mode);
	// style
	FIND_SELECTOR(state);
	FIND_SELECTOR(font);
	FIND_SELECTOR(type);
	// window
	FIND_SELECTOR(cursor);
	FIND_SELECTOR(max);
	// mark
	// who
	FIND_SELECTOR(message);
	// edit
	FIND_SELECTOR(play);
	FIND_SELECTOR(number);
	FIND_SELECTOR(handle);	// nodePtr
	FIND_SELECTOR(client);
	FIND_SELECTOR(dx);
	FIND_SELECTOR(dy);
	FIND_SELECTOR2(b_movCnt, "b-moveCnt");
	FIND_SELECTOR2(b_i1, "b-i1");
	FIND_SELECTOR2(b_i2, "b-i2");
	FIND_SELECTOR2(b_di, "b-di");
	FIND_SELECTOR2(b_xAxis, "b-xAxis");
	FIND_SELECTOR2(b_incr, "b-incr");
	FIND_SELECTOR(xStep);
	FIND_SELECTOR(yStep);
	FIND_SELECTOR(moveSpeed);
	FIND_SELECTOR(canBeHere);	// cantBeHere
	FIND_SELECTOR(heading);
	FIND_SELECTOR(mover);
	FIND_SELECTOR(doit);
	FIND_SELECTOR(isBlocked);
	FIND_SELECTOR(looper);
	FIND_SELECTOR(priority);
	FIND_SELECTOR(modifiers);
	FIND_SELECTOR(replay);
	// setPri
	// at
	// next
	// done
	// width
	FIND_SELECTOR(wordFail);
	FIND_SELECTOR(syntaxFail);
	// semanticFail
	// pragmaFail
	// said
	FIND_SELECTOR(claimed);
	// value
	// save
	// restore
	// title
	// button
	// icon
	// draw
	FIND_SELECTOR2(delete_, "delete");
	FIND_SELECTOR(z);
	// -----------------------------
	FIND_SELECTOR(size);
	FIND_SELECTOR(moveDone);
	FIND_SELECTOR(vol);
	FIND_SELECTOR(pri);
	FIND_SELECTOR(min);
	FIND_SELECTOR(sec);
	FIND_SELECTOR(frame);
	FIND_SELECTOR(dataInc);
	FIND_SELECTOR(palette);
	FIND_SELECTOR(cantBeHere);
	FIND_SELECTOR(nodePtr);
	FIND_SELECTOR(flags);
	FIND_SELECTOR(points);
	FIND_SELECTOR(syncCue);
	FIND_SELECTOR(syncTime);
	FIND_SELECTOR(printLang);
	FIND_SELECTOR(subtitleLang);
	FIND_SELECTOR(parseLang);
	FIND_SELECTOR(overlay);
	FIND_SELECTOR(setCursor);
	FIND_SELECTOR(topString);
	FIND_SELECTOR(scaleSignal);
	FIND_SELECTOR(scaleX);
	FIND_SELECTOR(scaleY);

#ifdef ENABLE_SCI32
	FIND_SELECTOR(data);
	FIND_SELECTOR(picture);
	FIND_SELECTOR(plane);
	FIND_SELECTOR(top);
	FIND_SELECTOR(left);
#endif
}

reg_t read_selector(SegManager *segMan, reg_t object, Selector selector_id) {
	ObjVarRef address;

	if (lookup_selector(segMan, object, selector_id, &address, NULL) != kSelectorVariable)
		return NULL_REG;
	else
		return *address.getPointer(segMan);
}

void write_selector(SegManager *segMan, reg_t object, Selector selector_id, reg_t value) {
	ObjVarRef address;

	if ((selector_id < 0) || (selector_id > (int)((SciEngine*)g_engine)->getKernel()->getSelectorNamesSize())) {
		warning("Attempt to write to invalid selector %d of"
		         " object at %04x:%04x.", selector_id, PRINT_REG(object));
		return;
	}

	if (lookup_selector(segMan, object, selector_id, &address, NULL) != kSelectorVariable)
		warning("Selector '%s' of object at %04x:%04x could not be"
		         " written to", ((SciEngine*)g_engine)->getKernel()->getSelectorName(selector_id).c_str(), PRINT_REG(object));
	else
		*address.getPointer(segMan) = value;
}

int invoke_selector_argv(EngineState *s, reg_t object, int selector_id, SelectorInvocation noinvalid,
	int k_argc, StackPtr k_argp, int argc, const reg_t *argv) {
	int i;
	int framesize = 2 + 1 * argc;
	reg_t address;
	int slc_type;
	StackPtr stackframe = k_argp + k_argc;

	stackframe[0] = make_reg(0, selector_id);  // The selector we want to call
	stackframe[1] = make_reg(0, argc); // Argument count

	slc_type = lookup_selector(s->_segMan, object, selector_id, NULL, &address);

	if (slc_type == kSelectorNone) {
		warning("Selector '%s' of object at %04x:%04x could not be invoked",
		         s->_kernel->getSelectorName(selector_id).c_str(), PRINT_REG(object));
		if (noinvalid == kStopOnInvalidSelector)
			error("[Kernel] Not recoverable: VM was halted");
		return 1;
	}
	if (slc_type == kSelectorVariable) {
		warning("Attempting to invoke variable selector %s of object %04x:%04x",
			s->_kernel->getSelectorName(selector_id).c_str(), PRINT_REG(object));
		return 0;
	}

	for (i = 0; i < argc; i++)
		stackframe[2 + i] = argv[i]; // Write each argument

	ExecStack *xstack;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, framesize, stackframe);

	xstack->sp += argc + 2;
	xstack->fp += argc + 2;

	run_vm(s, 0); // Start a new vm

	return 0;
}

int invoke_selector(EngineState *s, reg_t object, int selector_id, SelectorInvocation noinvalid,
	int k_argc, StackPtr k_argp, int argc, ...) {
	va_list argp;
	reg_t *args = new reg_t[argc];

	va_start(argp, argc);
	for (int i = 0; i < argc; i++)
		args[i] = va_arg(argp, reg_t);
	va_end(argp);

	int retval = invoke_selector_argv(s, object, selector_id, noinvalid, k_argc, k_argp, argc, args);

	delete[] args;
	return retval;
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
