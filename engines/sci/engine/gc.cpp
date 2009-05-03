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
//#define DEBUG_GC_VERBOSE

struct WorklistManager {
	Common::Array<reg_t> _worklist;
	reg_t_hash_map _map;

	void push(reg_t reg) {
		if (!reg.segment) // No numbers
			return;

	#ifdef DEBUG_GC_VERBOSE
		sciprintf("[GC] Adding "PREG"\n", PRINT_REG(reg));
	#endif

		if (_map.contains(reg))
			return; // already dealt with it

		_map.setVal(reg, true);
		_worklist.push_back(reg);
	}
};

static reg_t_hash_map *normalise_hashmap_ptrs(reg_t_hash_map &nonnormal_map, const Common::Array<SegInterface *> &interfaces) {
	reg_t_hash_map *normal_map = new reg_t_hash_map();

	for (reg_t_hash_map::iterator i = nonnormal_map.begin(); i != nonnormal_map.end(); ++i) {
		reg_t reg = i->_key;
		SegInterface *interfce = (reg.segment < interfaces.size()) ? interfaces[reg.segment] : NULL;

		if (interfce) {
			reg = interfce->findCanonicAddress(reg);
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
	SegManager *sm = s->seg_manager;
	Common::Array<SegInterface *> interfaces;
	reg_t_hash_map *normal_map = NULL;
	WorklistManager wm;
	uint i;

	interfaces.resize(sm->heap_size);
	for (i = 1; i < sm->heap_size; i++)
		if (sm->heap[i] == NULL)
			interfaces[i] = NULL;
		else
			interfaces[i] = sm->getSegInterface(i);

	// Initialise
	// Init: Registers
	wm.push(s->r_acc);
	wm.push(s->r_prev);
	// Init: Value Stack
	// We do this one by hand since the stack doesn't know the current execution stack
	{
		ExecStack &xs = s->_executionStack[s->execution_stack_pos];
		reg_t *pos;

		for (pos = s->stack_base; pos < xs.sp; pos++)
			wm.push(*pos);
	}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished adding value stack");
#endif

	// Init: Execution Stack
	for (i = 0; (int)i <= s->execution_stack_pos; i++) {
		ExecStack &es = s->_executionStack[i];

		if (es.type != EXEC_STACK_TYPE_KERNEL) {
			wm.push(es.objp);
			wm.push(es.sendp);
			if (es.type == EXEC_STACK_TYPE_VARSELECTOR)
				wm.push(*(es.addr.varp));
		}
	}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished adding execution stack");
#endif

	// Init: Explicitly loaded scripts
	for (i = 1; i < sm->heap_size; i++)
		if (interfaces[i]
		        && interfaces[i]->getType() == MEM_OBJ_SCRIPT) {
			Script *script = &(interfaces[i]->getMobj()->data.script);

			if (script->lockers) { // Explicitly loaded?
				int obj_nr;

				// Locals, if present
				wm.push(make_reg(script->locals_segment, 0));

				// All objects (may be classes, may be indirectly reachable)
				for (obj_nr = 0; obj_nr < script->objects_nr; obj_nr++) {
					Object *obj = script->objects + obj_nr;
					wm.push(obj->pos);
				}
			}
		}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished explicitly loaded scripts, done with root set");
#endif

	// Run Worklist Algorithm
	while (!wm._worklist.empty()) {
		reg_t reg = wm._worklist.back();
		wm._worklist.pop_back();
		if (reg.segment != s->stack_segment) { // No need to repeat this one
#ifdef DEBUG_GC_VERBOSE
			sciprintf("[GC] Checking "PREG"\n", PRINT_REG(reg));
#endif
			if (reg.segment < sm->heap_size && interfaces[reg.segment])
				interfaces[reg.segment]->listAllOutgoingReferences(s, reg, &wm, add_outgoing_refs);
		}
	}

	// Normalise
	normal_map = normalise_hashmap_ptrs(wm._map, interfaces);

	// Cleanup
	for (i = 1; i < sm->heap_size; i++)
		if (interfaces[i])
			delete interfaces[i];

	return normal_map;
}

struct deallocator_t {
	SegInterface *interfce;
#ifdef DEBUG_GC
	char *segnames[MEM_OBJ_MAX + 1];
	int segcount[MEM_OBJ_MAX + 1];
#endif
	reg_t_hash_map *use_map;
};

void free_unless_used(void *refcon, reg_t addr) {
	deallocator_t *deallocator = (deallocator_t *)refcon;
	reg_t_hash_map *use_map = deallocator->use_map;

	if (!use_map->contains(addr)) {
		// Not found -> we can free it
		deallocator->interfce->freeAtAddress(addr);
#ifdef DEBUG_GC
		sciprintf("[GC] Deallocating "PREG"\n", PRINT_REG(addr));
		deallocator->segcount[deallocator->interfce->getType()]++;
#endif
	}

}

void run_gc(EngineState *s) {
	uint seg_nr;
	deallocator_t deallocator;
	SegManager *sm = s->seg_manager;

#ifdef DEBUG_GC
	extern int c_segtable(EngineState *s);
	c_segtable(s);
	sciprintf("[GC] Running...\n");
	memset(&(deallocator.segcount), 0, sizeof(int) * (MEM_OBJ_MAX + 1));
#endif

	deallocator.use_map = find_all_used_references(s);

	for (seg_nr = 1; seg_nr < sm->heap_size; seg_nr++) {
		if (sm->heap[seg_nr] != NULL) {
			deallocator.interfce = sm->getSegInterface(seg_nr);
#ifdef DEBUG_GC
			deallocator.segnames[deallocator.interfce->getType()] = deallocator.interfce->type;
#endif
			deallocator.interfce->listAllDeallocatable(&deallocator, free_unless_used);
			delete deallocator.interfce;
		}
	}

	delete deallocator.use_map;

#ifdef DEBUG_GC
	{
		int i;
		sciprintf("[GC] Summary:\n");
		for (i = 0; i <= MEM_OBJ_MAX; i++)
			if (deallocator.segcount[i])
				sciprintf("\t%d\t* %s\n", deallocator.segcount[i], deallocator.segnames[i]);
	}
#endif
}

} // End of namespace Sci
