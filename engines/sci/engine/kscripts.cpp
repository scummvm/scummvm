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
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

#include "common/file.h"

namespace Sci {

// Loads arbitrary resources of type 'restype' with resource numbers 'resnrs'
// This implementation ignores all resource numbers except the first one.
reg_t kLoad(EngineState *s, int argc, reg_t *argv) {
	ResourceType restype = (ResourceType)(argv[0].toUint16() & 0x7f);
	int resnr = argv[1].toUint16();

	// Request to dynamically allocate hunk memory for later use
	if (restype == kResourceTypeMemory)
		return s->_segMan->allocateHunkEntry("kLoad()", resnr);

	return make_reg(0, ((restype << 11) | resnr)); // Return the resource identifier as handle
}

reg_t kLock(EngineState *s, int argc, reg_t *argv) {
	int state = argc > 2 ? argv[2].toUint16() : 1;
	ResourceType type = (ResourceType)(argv[0].toUint16() & 0x7f);
	ResourceId id = ResourceId(type, argv[1].toUint16());

	Resource *which;

	switch (state) {
	case 1 :
		g_sci->getResMan()->findResource(id, 1);
		break;
	case 0 :
		which = g_sci->getResMan()->findResource(id, 0);

		if (which)
			g_sci->getResMan()->unlockResource(which);
		else {
			if (id.getType() == kResourceTypeInvalid)
				warning("[resMan] Attempt to unlock resource %i of invalid type %i", id.getNumber(), type);
			else
				// Happens in CD games (e.g. LSL6CD) with the message resource
				warning("[resMan] Attempt to unlock non-existant resource %s", id.toString().c_str());
		}
		break;
	}
	return s->r_acc;
}

// Unloads an arbitrary resource of type 'restype' with resource numbber 'resnr'
reg_t kUnLoad(EngineState *s, int argc, reg_t *argv) {
	if (argc >= 2) {
		ResourceType restype = (ResourceType)(argv[0].toUint16() & 0x7f);
		reg_t resnr = argv[1];

		// WORKAROUND for a broken script in room 320 in Castle of Dr. Brain.
		// Script 377 tries to free the hunk memory allocated for the saved area
		// (underbits) beneath the pop up window, which results in having the
		// window stay on screen even when it's closed. Ignore this request here.
		if (restype == kResourceTypeMemory && g_sci->getGameId() == GID_CASTLEBRAIN &&
			s->currentRoomNumber() == 320)
			return s->r_acc;

		if (restype == kResourceTypeMemory)
			s->_segMan->freeHunkEntry(resnr);

		if (argc > 2)
			warning("kUnload called with more than 2 parameters (%d)", argc);
	} else {
		warning("kUnload called with less than 2 parameters (%d) - ignoring", argc);
	}

	return s->r_acc;
}

reg_t kResCheck(EngineState *s, int argc, reg_t *argv) {
	Resource *res = NULL;
	ResourceType restype = (ResourceType)(argv[0].toUint16() & 0x7f);

	if (restype == kResourceTypeVMD) {
		char fileName[50];
		sprintf(fileName, "%d.vmd", argv[1].toUint16());
		return make_reg(0, Common::File::exists(fileName));
	}

	if ((restype == kResourceTypeAudio36) || (restype == kResourceTypeSync36)) {
		if (argc >= 6) {
			uint noun = argv[2].toUint16() & 0xff;
			uint verb = argv[3].toUint16() & 0xff;
			uint cond = argv[4].toUint16() & 0xff;
			uint seq = argv[5].toUint16() & 0xff;

			res = g_sci->getResMan()->testResource(ResourceId(restype, argv[1].toUint16(), noun, verb, cond, seq));
		}
	} else {
		res = g_sci->getResMan()->testResource(ResourceId(restype, argv[1].toUint16()));
	}

	return make_reg(0, res != NULL);
}

reg_t kClone(EngineState *s, int argc, reg_t *argv) {
	reg_t parent_addr = argv[0];
	const Object *parent_obj = s->_segMan->getObject(parent_addr);
	reg_t clone_addr;
	Clone *clone_obj; // same as Object*

	if (!parent_obj) {
		error("Attempt to clone non-object/class at %04x:%04x failed", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	debugC(2, kDebugLevelMemory, "Attempting to clone from %04x:%04x", PRINT_REG(parent_addr));

	clone_obj = s->_segMan->allocateClone(&clone_addr);

	if (!clone_obj) {
		error("Cloning %04x:%04x failed-- internal error", PRINT_REG(parent_addr));
		return NULL_REG;
	}

	*clone_obj = *parent_obj;

	// Mark as clone
	clone_obj->markAsClone();
	clone_obj->setSpeciesSelector(clone_obj->getPos());
	if (parent_obj->isClass())
		clone_obj->setSuperClassSelector(parent_obj->getPos());
	s->_segMan->getScript(parent_obj->getPos().segment)->incrementLockers();
	s->_segMan->getScript(clone_obj->getPos().segment)->incrementLockers();

	return clone_addr;
}

extern void _k_view_list_mark_free(EngineState *s, reg_t off);

reg_t kDisposeClone(EngineState *s, int argc, reg_t *argv) {
	reg_t victim_addr = argv[0];
	Clone *victim_obj = s->_segMan->getObject(victim_addr);

	if (!victim_obj) {
		error("Attempt to dispose non-class/object at %04x:%04x",
		         PRINT_REG(victim_addr));
		return s->r_acc;
	}

	if (!victim_obj->isClone()) {
		// SCI silently ignores this behaviour; some games actually depend on it
		return s->r_acc;
	}

	victim_obj->markAsFreed();

	return s->r_acc;
}

// Returns script dispatch address index in the supplied script
reg_t kScriptID(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].toUint16();
	uint16 index = (argc > 1) ? argv[1].toUint16() : 0;

	if (argv[0].segment)
		return argv[0];

	SegmentId scriptSeg = s->_segMan->getScriptSegment(script, SCRIPT_GET_LOAD);

	if (!scriptSeg)
		return NULL_REG;

	Script *scr = s->_segMan->getScript(scriptSeg);

	if (!scr->getExportsNr()) {
		// This is normal. Some scripts don't have a dispatch (exports) table,
		// and this call is probably used to load them in memory, ignoring
		// the return value. If only one argument is passed, this call is done
		// only to load the script in memory. Thus, don't show any warning,
		// as no return value is expected. If an export is requested, then
		// it will most certainly fail with OOB access.
		if (argc == 2)
			error("Script 0x%x does not have a dispatch table and export %d "
					"was requested from it", script, index);
		return NULL_REG;
	}

	if (index > scr->getExportsNr()) {
		error("Dispatch index too big: %d > %d", index, scr->getExportsNr());
		return NULL_REG;
	}

	uint16 address = scr->validateExportFunc(index);

	// Point to the heap for SCI1.1+ games
	if (getSciVersion() >= SCI_VERSION_1_1)
		address += scr->getScriptSize();

	return make_reg(scriptSeg, address);
}

reg_t kDisposeScript(EngineState *s, int argc, reg_t *argv) {
	int script = argv[0].offset;

	// Work around QfG1 graveyard bug
	if (argv[0].segment)
		return s->r_acc;

	SegmentId id = s->_segMan->getScriptSegment(script);
	Script *scr = s->_segMan->getScriptIfLoaded(id);
	if (scr) {
		if (s->_executionStack.back().addr.pc.segment != id)
			scr->setLockers(1);
	}

	script_uninstantiate(s->_segMan, script);

	if (argc != 2) {
		return s->r_acc;
	} else {
		// This exists in the KQ5CD and GK1 interpreter. We know it is used
		// when GK1 starts up, before the Sierra logo.
		warning("kDisposeScript called with 2 parameters, still untested");
		return argv[1];
	}
}

reg_t kIsObject(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].offset == SIGNAL_OFFSET) // Treated specially
		return NULL_REG;
	else
		return make_reg(0, s->_segMan->isHeapObject(argv[0]));
}

reg_t kRespondsTo(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	int selector = argv[1].toUint16();

	return make_reg(0, s->_segMan->isHeapObject(obj) && lookupSelector(s->_segMan, obj, selector, NULL, NULL) != kSelectorNone);
}

} // End of namespace Sci
