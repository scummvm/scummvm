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

#include "sci/engine/gc.h"
#include "common/array.h"

namespace Sci {

//#define DEBUG_GC

struct WorklistManager {
	Common::Array<reg_t> _worklist;
	reg_t_hash_map _map;

	void push(reg_t reg) {
		if (!reg.segment) // No numbers
			return;

		debugC(2, kDebugLevelGC, "[GC] Adding %04x:%04x\n", PRINT_REG(reg));

		if (_map.contains(reg))
			return; // already dealt with it

		_map.setVal(reg, true);
		_worklist.push_back(reg);
	}
};

static reg_t_hash_map *normalise_hashmap_ptrs(SegManager *segMan, reg_t_hash_map &nonnormal_map) {
	reg_t_hash_map *normal_map = new reg_t_hash_map();

	for (reg_t_hash_map::iterator i = nonnormal_map.begin(); i != nonnormal_map.end(); ++i) {
		reg_t reg = i->_key;
		SegmentObj *mobj = (reg.segment < segMan->_heap.size()) ? segMan->_heap[reg.segment] : NULL;

		if (mobj) {
			reg = mobj->findCanonicAddress(segMan, reg);
			normal_map->setVal(reg, true);
		}
	}

	return normal_map;
}


void add_outgoing_refs(void *refcon, reg_t addr) {
	WorklistManager *wm = (WorklistManager *)refcon;
	wm->push(addr);
}

reg_t_hash_map *find_all_used_references(EngineState *s) {
	SegManager *segMan = s->segMan;
	reg_t_hash_map *normal_map = NULL;
	WorklistManager wm;
	uint i;

	// Initialise
	// Init: Registers
	wm.push(s->r_acc);
	wm.push(s->r_prev);
	// Init: Value Stack
	// We do this one by hand since the stack doesn't know the current execution stack
	{
		ExecStack &xs = s->_executionStack.back();
		reg_t *pos;

		for (pos = s->stack_base; pos < xs.sp; pos++)
			wm.push(*pos);
	}

	debugC(2, kDebugLevelGC, "[GC] -- Finished adding value stack");

	// Init: Execution Stack
	Common::List<ExecStack>::iterator iter;
	for (iter = s->_executionStack.begin();
	     iter != s->_executionStack.end(); ++iter) {
		ExecStack &es = *iter;

		if (es.type != EXEC_STACK_TYPE_KERNEL) {
			wm.push(es.objp);
			wm.push(es.sendp);
			if (es.type == EXEC_STACK_TYPE_VARSELECTOR)
				wm.push(*(es.getVarPointer(s->segMan)));
		}
	}

	debugC(2, kDebugLevelGC, "[GC] -- Finished adding execution stack");

	// Init: Explicitly loaded scripts
	for (i = 1; i < segMan->_heap.size(); i++)
		if (segMan->_heap[i]
		        && segMan->_heap[i]->getType() == SEG_TYPE_SCRIPT) {
			Script *script = (Script *)segMan->_heap[i];

			if (script->getLockers()) { // Explicitly loaded?
				// Locals, if present
				wm.push(make_reg(script->_localsSegment, 0));

				// All objects (may be classes, may be indirectly reachable)
				for (uint obj_nr = 0; obj_nr < script->_objects.size(); obj_nr++) {
					wm.push(script->_objects[obj_nr].pos);
				}
			}
		}

	debugC(2, kDebugLevelGC, "[GC] -- Finished explicitly loaded scripts, done with root set\n");

	// Run Worklist Algorithm
	while (!wm._worklist.empty()) {
		reg_t reg = wm._worklist.back();
		wm._worklist.pop_back();
		if (reg.segment != s->stack_segment) { // No need to repeat this one
			debugC(2, kDebugLevelGC, "[GC] Checking %04x:%04x\n", PRINT_REG(reg));
			if (reg.segment < segMan->_heap.size() && segMan->_heap[reg.segment])
				segMan->_heap[reg.segment]->listAllOutgoingReferences(reg, &wm, add_outgoing_refs, s->resMan->sciVersion());
		}
	}

	// Normalise
	normal_map = normalise_hashmap_ptrs(segMan, wm._map);

	return normal_map;
}

struct deallocator_t {
	SegManager *segMan;
	SegmentObj *mobj;
#ifdef DEBUG_GC
	char *segnames[SEG_TYPE_MAX + 1];
	int segcount[SEG_TYPE_MAX + 1];
#endif
	reg_t_hash_map *use_map;
};

void free_unless_used(void *refcon, reg_t addr) {
	deallocator_t *deallocator = (deallocator_t *)refcon;
	reg_t_hash_map *use_map = deallocator->use_map;

	if (!use_map->contains(addr)) {
		// Not found -> we can free it
		deallocator->mobj->freeAtAddress(deallocator->segMan, addr);
#ifdef DEBUG_GC
		debugC(2, kDebugLevelGC, "[GC] Deallocating %04x:%04x\n", PRINT_REG(addr));
		deallocator->segcount[deallocator->mobj->getType()]++;
#endif
	}

}

void run_gc(EngineState *s) {
	uint seg_nr;
	deallocator_t deallocator;
	SegManager *segMan = s->segMan;

#ifdef DEBUG_GC
	debugC(2, kDebugLevelGC, "[GC] Running...\n");
	memset(&(deallocator.segcount), 0, sizeof(int) * (SEG_TYPE_MAX + 1));
#endif

	deallocator.segMan = segMan;
	deallocator.use_map = find_all_used_references(s);

	for (seg_nr = 1; seg_nr < segMan->_heap.size(); seg_nr++) {
		if (segMan->_heap[seg_nr] != NULL) {
			deallocator.mobj = segMan->_heap[seg_nr];
#ifdef DEBUG_GC
			deallocator.segnames[deallocator.mobj->getType()] = deallocator.mobj->type;	// FIXME: add a segment "name"
#endif
			deallocator.mobj->listAllDeallocatable(seg_nr, &deallocator, free_unless_used);
		}
	}

	delete deallocator.use_map;

#ifdef DEBUG_GC
	{
		int i;
		debugC(2, kDebugLevelGC, "[GC] Summary:\n");
		for (i = 0; i <= SEG_TYPE_MAX; i++)
			if (deallocator.segcount[i])
				debugC(2, kDebugLevelGC, "\t%d\t* %s\n", deallocator.segcount[i], deallocator.segnames[i]);
	}
#endif
}

} // End of namespace Sci
