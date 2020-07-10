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

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/scriptdebug.h"
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
	debugN("\t{ \"%s\", %d },\n", #_slc_, _selectorCache._slc_)

#define FIND_SELECTOR2(_slc_, _slcstr_) _selectorCache._slc_ = findSelector(_slcstr_); \
	debugN("\t{ \"%s\", %d },\n", _slcstr_, _selectorCache._slc_)

#endif

void Kernel::mapSelectors() {
	// species
	// superClass
	FIND_SELECTOR2(_info_, "-info-");
	FIND_SELECTOR(y);
	FIND_SELECTOR(x);
	FIND_SELECTOR(view);
	FIND_SELECTOR(loop);
	FIND_SELECTOR(cel);
	FIND_SELECTOR(underBits);
	FIND_SELECTOR(nsTop);
	FIND_SELECTOR(nsLeft);
	FIND_SELECTOR(nsBottom);
	FIND_SELECTOR(nsRight);
	FIND_SELECTOR(lsTop);
	FIND_SELECTOR(lsLeft);
	FIND_SELECTOR(lsBottom);
	FIND_SELECTOR(lsRight);
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
	FIND_SELECTOR(mark);
	FIND_SELECTOR(sort);
	// who
	FIND_SELECTOR(message);
	// edit
	FIND_SELECTOR(play);
	FIND_SELECTOR(restore);
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
	FIND_SELECTOR(xLast);
	FIND_SELECTOR(yLast);
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
	FIND_SELECTOR(topString);
	FIND_SELECTOR(scaleSignal);
	FIND_SELECTOR(scaleX);
	FIND_SELECTOR(scaleY);
	FIND_SELECTOR(maxScale);
	FIND_SELECTOR(vanishingX);
	FIND_SELECTOR(vanishingY);
	FIND_SELECTOR(iconIndex);
	FIND_SELECTOR(select);
	FIND_SELECTOR(handsOff);
	FIND_SELECTOR(setStep);
	FIND_SELECTOR(setMotion);
	FIND_SELECTOR(cycleSpeed);
	FIND_SELECTOR(owner);
	FIND_SELECTOR(curPos);
	FIND_SELECTOR(update);

#ifdef ENABLE_SCI32
	FIND_SELECTOR(data);
	FIND_SELECTOR(picture);
	FIND_SELECTOR(bitmap);
	FIND_SELECTOR(plane);
	FIND_SELECTOR(top);
	FIND_SELECTOR(left);
	FIND_SELECTOR(bottom);
	FIND_SELECTOR(right);
	FIND_SELECTOR(seenRect);
	FIND_SELECTOR(resY);
	FIND_SELECTOR(resX);
	FIND_SELECTOR(dimmed);
	FIND_SELECTOR(fore);
	FIND_SELECTOR(back);
	FIND_SELECTOR(skip);
	FIND_SELECTOR(borderColor);
	FIND_SELECTOR(width);
	FIND_SELECTOR(fixPriority);
	FIND_SELECTOR(mirrored);
	FIND_SELECTOR(visible);
	FIND_SELECTOR(useInsetRect);
	FIND_SELECTOR(inTop);
	FIND_SELECTOR(inLeft);
	FIND_SELECTOR(inBottom);
	FIND_SELECTOR(inRight);
	FIND_SELECTOR(textTop);
	FIND_SELECTOR(textLeft);
	FIND_SELECTOR(textBottom);
	FIND_SELECTOR(textRight);
	FIND_SELECTOR(title);
	FIND_SELECTOR(titleFont);
	FIND_SELECTOR(titleFore);
	FIND_SELECTOR(titleBack);
	FIND_SELECTOR(magnifier);
	FIND_SELECTOR(frameOut);
	FIND_SELECTOR(casts);
	FIND_SELECTOR(setVol);
	FIND_SELECTOR(reSyncVol);
	FIND_SELECTOR(set);
	FIND_SELECTOR(clear);
	FIND_SELECTOR(show);
	FIND_SELECTOR(position);
	FIND_SELECTOR(musicVolume);
	FIND_SELECTOR(soundVolume);
	FIND_SELECTOR(initialOff);
	FIND_SELECTOR(setPos);
	FIND_SELECTOR(setSize);
	FIND_SELECTOR(displayValue);
	FIND_SELECTOR2(new_, "new");
	FIND_SELECTOR(mainCel);
	FIND_SELECTOR(move);
	FIND_SELECTOR(eachElementDo);
	FIND_SELECTOR(physicalBar);
	FIND_SELECTOR(init);
	FIND_SELECTOR(scratch);
	FIND_SELECTOR(num);
	FIND_SELECTOR(reallyRestore);
	FIND_SELECTOR(bookMark);
	FIND_SELECTOR(fileNumber);
	FIND_SELECTOR(description);
	FIND_SELECTOR(dispose);
	FIND_SELECTOR(masterVolume);
	FIND_SELECTOR(setCel);
	FIND_SELECTOR(value);
#endif
}

reg_t readSelector(SegManager *segMan, reg_t object, Selector selectorId) {
	ObjVarRef address;

	if (lookupSelector(segMan, object, selectorId, &address, NULL) != kSelectorVariable)
		return NULL_REG;

	if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORREAD) {
		reg_t curValue = *address.getPointer(segMan);
		debugPropertyAccess(segMan->getObject(object), object, 0, selectorId,
			                curValue, NULL_REG, segMan, BREAK_SELECTORREAD);
	}

	return *address.getPointer(segMan);
}

#ifdef ENABLE_SCI32
void updateInfoFlagViewVisible(Object *obj, int index, bool fromPropertyOp) {
	if (getSciVersion() >= SCI_VERSION_2 && obj->mustSetViewVisible(index, fromPropertyOp)) {
		obj->setInfoSelectorFlag(kInfoFlagViewVisible);
	}
}
#endif

void writeSelector(SegManager *segMan, reg_t object, Selector selectorId, reg_t value) {
	ObjVarRef address;

	if ((selectorId < 0) || (selectorId > (int)g_sci->getKernel()->getSelectorNamesSize())) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		error("Attempt to write to invalid selector %d. Address %04x:%04x, %s", selectorId, PRINT_REG(object), origin.toString().c_str());
	}

	if (lookupSelector(segMan, object, selectorId, &address, NULL) != kSelectorVariable) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		error("Selector '%s' of object could not be written to. Address %04x:%04x, %s", g_sci->getKernel()->getSelectorName(selectorId).c_str(), PRINT_REG(object), origin.toString().c_str());
	}

	if (g_sci->_debugState._activeBreakpointTypes & BREAK_SELECTORWRITE) {
		reg_t curValue = *address.getPointer(segMan);
		debugPropertyAccess(segMan->getObject(object), object, 0, selectorId,
			                curValue, value, segMan, BREAK_SELECTORWRITE);
	}

	*address.getPointer(segMan) = value;
#ifdef ENABLE_SCI32
	updateInfoFlagViewVisible(segMan->getObject(object), address.varindex);
#endif
}

void invokeSelector(EngineState *s, reg_t object, int selectorId,
	int k_argc, StackPtr k_argp, int argc, const reg_t *argv) {
	int i;
	int framesize = 2 + 1 * argc;
	int slc_type;
	StackPtr stackframe = k_argp + k_argc;

	stackframe[0] = make_reg(0, selectorId);  // The selector we want to call
	stackframe[1] = make_reg(0, argc); // Argument count

	slc_type = lookupSelector(s->_segMan, object, selectorId, NULL, NULL);

	if (slc_type == kSelectorNone) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		error("invokeSelector: Selector '%s' could not be invoked. Address %04x:%04x, %s", g_sci->getKernel()->getSelectorName(selectorId).c_str(), PRINT_REG(object), origin.toString().c_str());
	}
	if (slc_type == kSelectorVariable) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		error("invokeSelector: Attempting to invoke variable selector %s. Address %04x:%04x, %s", g_sci->getKernel()->getSelectorName(selectorId).c_str(), PRINT_REG(object), origin.toString().c_str());
	}

	for (i = 0; i < argc; i++)
		stackframe[2 + i] = argv[i]; // Write each argument

	ExecStack *xstack;

	// Now commit the actual function:
	xstack = send_selector(s, object, object, stackframe, framesize, stackframe);

	xstack->sp += argc + 2;
	xstack->fp += argc + 2;

	run_vm(s); // Start a new vm
}

SelectorType lookupSelector(SegManager *segMan, reg_t obj_location, Selector selectorId, ObjVarRef *varp, reg_t *fptr) {
	const Object *obj = segMan->getObject(obj_location);
	int index;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// Early SCI versions used the LSB in the selector ID as a read/write
	// toggle, meaning that we must remove it for selector lookup.
	if (oldScriptHeader)
		selectorId &= ~1;

	if (!obj) {
		const SciCallOrigin origin = g_sci->getEngineState()->getCurrentCallOrigin();
		error("lookupSelector: Attempt to send to non-object or invalid script. Address %04x:%04x, %s", PRINT_REG(obj_location), origin.toString().c_str());
	}

	index = obj->locateVarSelector(segMan, selectorId);

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
			index = obj->funcSelectorPosition(selectorId);
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


//	return _lookupSelector_function(segMan, obj, selectorId, fptr);
}

} // End of namespace Sci
