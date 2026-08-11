/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/res_man.h"
#include "engines/icb/debug.h"
#include "engines/icb/protocol.h"
#include "engines/icb/keyboard.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/zsupport.h"
#include "engines/icb/mission.h"

#include "common/textconsole.h"

namespace ICB {

// total memory game should use - i.e. total minus some for windows, .exe, etc.
uint32 memory_available = 0; // set by memory_stats

int16 res_man::Find_space(uint32 len) {
	// find a mem slot large enough to fit our resource into
	// return -1 if cannot find

	// blocks will be either free or in_use as we search fro mbase_block to child==-1

	// gets block into correct size and adjusts child downward if free or spawns a space

	int16 cur_block = 0;
	uint32 slack;
	int16 child;
	uint16 spawn;

	do {
		if (mem_list[cur_block].state == MEM_free) { // is current block a free block?
			if (mem_list[cur_block].size > len) {
				//                              free block is bigger than required so split it

				child = mem_list[cur_block].child;
				slack = mem_list[cur_block].size - len;

				mem_list[cur_block].size = len;
				total_free_memory -= len; // adjust total free

				if (child == -1) { // this is last block - so spawn the gap
					spawn = Fetch_spawn(cur_block); // spawn a new MEM_free block

					mem_list[cur_block].child = spawn; // our new baby
					mem_list[spawn].child = -1; // spawned becomes the new end block
					mem_list[spawn].size = slack; //
					mem_list[spawn].ad = mem_list[cur_block].ad + len;

					return (cur_block);
				} else if (mem_list[child].state == MEM_free) { // merge the free child downwards into the new void
					mem_list[child].ad -= slack; // address moves down
					mem_list[child].size += slack; // size goes up
					return (cur_block);
				} else if (mem_list[child].state == MEM_in_use) { // we must spawn a new block to span the new void
					spawn = Fetch_spawn(cur_block); // spawn a new MEM_free block

					mem_list[cur_block].child = spawn; // our new baby
					mem_list[spawn].child = child; // spawned is our child and our previous child becomes its child
					mem_list[child].parent = spawn; // our old child get our new child as its parent
					mem_list[spawn].size = slack; // spans the void
					mem_list[spawn].ad = mem_list[cur_block].ad + len;

					return (cur_block);
				} else { // error
					Fatal_error("ERROR: Illegal child found by Find_space [file=%s line=%u]", __FILE__, __LINE__);
				}

			} else if (mem_list[cur_block].size == len) { // right size - its a miracle
				//                              we can use this block

				total_free_memory -= len; // adjust total free

				return (cur_block); // this block is big enough - return its id
			}
		}
		cur_block = mem_list[cur_block].child; // move to next one along, the current must be floating, locked, or a NULL slot

	} while (cur_block != -1); // just got next block but the current was the end block

	return ((int16)-1);
}

uint16 res_man::Fetch_spawn(uint16 parent) {
	// spawn a new MEM_free block
	// new block has its parent, state and UID set up

	// we enter with the parent
	// we exit with id of new mem_block

	uint16 spawn = 0;

	// find a NULL slot for a new block
	while ((mem_list[spawn].state != MEM_null) && (spawn != max_mem_blocks))
		spawn++;

	if (spawn == max_mem_blocks) { // run out of blocks - stop the program. this is a major blow up and we need to alert the developer
		// Mem_debug();  //Lets get a printout of this
		Fatal_error("ERROR: ran out of mem blocks in Fetch_spawn() [file=%s line=%u]", __FILE__, __LINE__);
	}

	mem_list[spawn].parent = parent;
	mem_list[spawn].state = MEM_free;
	mem_list[spawn].age = 0;

	// increase total number of used blocks
	total_blocks++;

	return (spawn);
}

void res_man::Defrag() {
	// probably just enough space exists so do a total defrag

	int16 cur_block = 0;
	uint32 temp;
	int16 child, grandchild;

	bool8 debug_state = zdebug;
	zdebug = TRUE8;

	Tdebug("defrag.txt", "\ndefrag");

	amount_of_defrags++;

	// just to be on the safe side, finish all async stuff, we can't risk any async going to addresses we are moving about...

	do {

		Tdebug("defrag.txt", "\nlooking at bloc %d", cur_block);

		if (mem_list[cur_block].state == MEM_free) { // is current block a free block?

			Tdebug("defrag.txt", " bloc is free");

			//                      current block is free
			//                      end found? yes then finish
			if (mem_list[cur_block].child == -1) {
				Tdebug("defrag.txt", "  we are end - so end");
				zdebug = debug_state;
				return;
			}

			//       is our child also free
			//       if so it must have been caused by swap last loop
			child = mem_list[cur_block].child;

			if (mem_list[child].state == MEM_free) {
				Tdebug("defrag.txt", "  child is free");
				Tdebug("defrag.txt", "  merging %d into %d", child, cur_block);
				// merge our child into us
				grandchild = mem_list[child].child; // we get our childs child
				mem_list[cur_block].child = grandchild; // we get our childs child
				Tdebug("defrag.txt", "  our new child is %d", grandchild);
				mem_list[cur_block].size += mem_list[child].size; // we get our childs memory
				if (grandchild != -1)
					mem_list[grandchild].parent = cur_block; // we are our new childs parent

				Tdebug("defrag.txt", "  zapped %d", child);

				mem_list[child].state = MEM_null; // remove the child
				total_blocks--;

				// we could now be end block
				if (mem_list[cur_block].child == -1) {
					Tdebug("defrag.txt", "   our new child is end - so end");
					zdebug = debug_state;
					return;
				}
				// if not then next HAS to be a MEM-in-use
				child = mem_list[cur_block].child;
			}

			//                      is our child in use - i.e. a real resource
			if (mem_list[child].state == MEM_in_use) {
				Tdebug("defrag.txt", "  child is in use - we swap");
				//                      ok, child is a normal block - swap it around, so data moves to bottom

				child = mem_list[cur_block].child; // for readability

//                      physically move memory
				memcpy(mem_list[cur_block].ad, mem_list[child].ad, mem_list[child].size); // dest, src, len

				mem_list[cur_block].state = MEM_in_use; // we now have memory
				mem_list[child].state = MEM_free; // child is now free

				mem_list[cur_block].url_hash = mem_list[child].url_hash;
				mem_list[cur_block].cluster_hash = mem_list[child].cluster_hash;
				mem_list[cur_block].total_hash = mem_list[child].total_hash;

				// we get the age of the child block
				mem_list[cur_block].age = mem_list[child].age;

				//                      swap sizes
				temp = mem_list[cur_block].size;
				mem_list[cur_block].size = mem_list[child].size; // we get childs
				mem_list[child].size = temp; // child gets ours

				//                      reset address of child
				mem_list[child].ad = mem_list[cur_block].ad + mem_list[cur_block].size;
			} else
				Fatal_error("defrag confused! child is %d");
		}

		cur_block = mem_list[cur_block].child; // move to next one along, the current must be floating, locked, or a NULL slot
	} while (cur_block != -1); // just got next block but the current was the end block

	Tdebug("defrag.txt", "got to end");
	zdebug = debug_state;

	// finished - should be done.
}

void res_man::Initialise(uint32 memory_tot) {
	total_free_memory = memory_tot;
	total_pool = memory_tot; // kept for error referencing and so on

	// force to int32 word boundary
	uint8 *adj_mem = memory_base + 7;
	adj_mem = (uint8 *)((ptrdiff_t)adj_mem & ~7);

	mem_list[0].ad = adj_mem;

	Reset();

	amount_of_defrags = 0;

	Tdebug("resman.txt", "made resman - %d", total_pool);
}
//------------------------------------------------------------------------------------
void res_man::Reset() { // trash all resources

	// set all to unused
	uint32 j;
	for (j = 0; j < max_mem_blocks; j++) {
		mem_list[j].state = MEM_null;
		mem_offset_list[j].total_hash = 0;
	}

	// now setup first free block
	total_blocks = 1; // total used (free, locked or floating)
	mem_list[0].size = total_pool;
	mem_list[0].state = MEM_free;
	mem_list[0].parent = -1; // we are base - for now
	mem_list[0].child = -1; // we are the end as well
	mem_list[0].protect = 0;
	mem_list[0].age = 0;
	mem_list[0].total_hash = 0;
	//      cannot start at 0 - updated on a screen by screen basis now
	auto_time_advance = FALSE8; // default to manual time frame advance
	no_defrag = FALSE8; // default is to allow ageing out
	current_time_frame = 1;
	total_free_memory = total_pool;

	//      set to no files currently open
	number_files_open = 0;
}

res_man::~res_man() {
	Zdebug("*resman destructing*");

	delete[] memory_base;
	delete[] mem_list;
}

uint32 res_man::Fetch_old_memory(int32 number_of_cycles) {
	uint32 amount;
	int16 search;
	int32 oldest_age;

	amount = 0;
	search = 0;

	// resource must be older than this to qualify as being an oldy
	oldest_age = current_time_frame - number_of_cycles;

	while (search != -1) {
		// make sure in use
		if (mem_list[search].state == MEM_in_use) { // if a used block with a file...
			// get age
			if (mem_list[search].age < oldest_age) {
				amount += mem_list[search].size;
			}
		}

		// go to next one

		search = mem_list[search].child;
	}

	return amount;
}

// If hash or cluster_hash == NULL_HASH then the hash of url/cluster_url
// is computed and stored in hash/cluster_hash
uint8 *res_man::Res_open(const char *url, uint32 &url_hash, const char *cluster, uint32 &cluster_hash,
						 int32 compressed, // non zero if the resource is compressed
						 int32 *ret_len) {
	// make the hash names if we need to
	MakeHash(url, url_hash);
	MakeHash(cluster, cluster_hash);

	RMParams params;
	uint32 time = 0;

	params.url_hash = url_hash;
	params.cluster = cluster;
	params.cluster_hash = cluster_hash;
	params.mode = RM_LOADNOW;
	params.compressed = compressed;
	params.not_ready_yet = 0;

	if ((g_px->logic_timing) && (g_px->mega_timer)) {
		time = GetMicroTimer();
	}

	uint8 *ret = this->Internal_open(&params, ret_len);

	if ((g_px->logic_timing) && (g_px->mega_timer)) {
		time = GetMicroTimer() - time;
		g_mission->resman_logic_time += time;
	}

	return (ret);
}

void res_man::Advance_time_stamp() {
	// add one to the time frame
	// user modules must decide when appropriate to increase

	// Note, Internal open also updates this counter whenever
	// it has to load a resource into memory e.g. from CD or disc
	current_time_frame++;
}

res_man::res_man() {
	memory_base = nullptr;
	max_mem_blocks = 0;
	mem_list = nullptr;
}

res_man::res_man(uint32 memory_tot) {
	// object constructor - allocaes the memory pool for this manager

	memory_base = AllocMemory(memory_tot);

	max_mem_blocks = MAX_MEM_BLOCKS;
	mem_list = new mem[max_mem_blocks];
	mem_offset_list = new mem_offset[max_mem_blocks];
	num_mem_offsets = 0;

	// Setup everything up correctly
	Initialise(memory_tot);
}

void res_man::Construct(uint8 *base, uint32 size, mem *memList, mem_offset *offsetList, uint32 nMemBlocks) {
	// memory pool address passed in from outside
	memory_base = base;

	// mem_list pointer passed in from outside
	mem_list = memList;

	num_mem_offsets = 0;
	mem_offset_list = offsetList;

	max_mem_blocks = nMemBlocks;

	// Setup everything up correctly
	Initialise(size);
}

void res_man::Res_purge(const char *url, uint32 url_hash, const char *cluster, uint32 cluster_hash, uint32 fatal) {
	// remove a resource file from memory
	int16 child, parent, grandchild;
	int16 search;

	//      first find the file
	RMParams params;
	params.url_hash = url_hash;
	params.cluster = cluster;
	params.cluster_hash = cluster_hash;
	search = FindFile(&params);

	if (search != -1) {

		//  one less file open
		number_files_open--; // one less file in memory

		//  remove the filename
		mem_list[search].url_hash = NULL_HASH;
		mem_list[search].cluster_hash = NULL_HASH;
		mem_list[search].total_hash = NULL_HASH;

		//  regain the mem
		total_free_memory += mem_list[search].size; // pool regains this memory

		//  ** remember, although the file and its name are gone this block may remain as a MEM_free **

		child = mem_list[search].child; // this resources child
		//                              merge free child into us if possible
		if ((child != -1) && (mem_list[child].state == MEM_free)) { // we're not endblock and child is free
			grandchild = mem_list[child].child; // we get our childs child
			mem_list[search].child = grandchild; // we get our childs child
			mem_list[search].size += mem_list[child].size; // we get our childs memory

			// If the child isn't the end block update the parent of the grandchild
			if (grandchild != -1)
				mem_list[grandchild].parent = search; // we are the grandchild's new parent  **JAKE 15FEB99**

			mem_list[child].state = MEM_null; // remove the child
			total_blocks--; // one less mem block - number of files open remains the same
		}

		//                              merge into our parent if our parent is free and we're not base block - removing ourselves
		parent = mem_list[search].parent; // our parent
		if ((parent != -1) && (search != 0) && (mem_list[parent].state == MEM_free)) { // we're not the first so can we merge with our parent?
			grandchild = mem_list[search].child; // we get our childs child
			mem_list[parent].child = grandchild; // we get our childs child
			mem_list[parent].size += mem_list[search].size; // our parent gets our memory

			if (grandchild != -1)
				mem_list[grandchild].parent = parent; // we are the grandchild's new parent  **JAKE 15FEB99**

			mem_list[search].state = MEM_null; // we're gone and can be used again later

			total_blocks--; // one less block
		} else { // we can't merge into our parent because it isn't free (or we are block 0) so we become
			// a MEM_free floating block with no file
			mem_list[search].state = MEM_free; // block remains but is free to be defragged
		}

		return;
	}
	// Only throw an error if we have been told it is an error
	// In reality I think purging something that doesn't exist is fine (hey no skin of my nose)
	if (fatal)
		Fatal_error("res_purge tried to purge %s but file not in memory", url);
}

int16 res_man::OldFindFile(uint32 url_hash, uint32 cluster_hash, uint32 total_hash) {
	if (number_files_open == 0) {
		return -1;
	}

	int16 search = 0;
	mem *current_mem_block = &(mem_list[search]);

	// try to find a file in the pool
	for (;;) {
		if (current_mem_block->state == MEM_in_use) { // if a used block with a file...
			if (current_mem_block->total_hash == total_hash) {
				// check the url and cluster match (in case of identical total_hashs (1 in a billion))
				if ((current_mem_block->url_hash == url_hash) && (current_mem_block->cluster_hash == cluster_hash))
					return search;
			}
		}

		search = current_mem_block->child;

		// finished
		if (search == -1)
			return -1;

		current_mem_block = &(mem_list[search]);
	}
}

// super-star binary using one
int16 res_man::FindFile(uint32 url_hash, uint32 cluster_hash, uint32 total_hash) {
	int32 i, search;

	// have a look in super-fast list
	// we have an offset into the list
	i = FindMemOffset(total_hash);

	// if it's not there
	if (i == -1) {
		// see if actually there in the proper list
		search = OldFindFile(url_hash, cluster_hash, total_hash);

		// if it is then add it to the super list and return i
		if (search != -1) {
			AddMemOffset(total_hash, search);
		}

		// return either -1 or the new value
	} else {
		// what is search going to be
		search = mem_offset_list[i].search;

		// if the file exists in the binary table but has subsequently been set to not actually in memory (below)
		// we found it in the list, check it is correct (also check url and cluster in case of identical total_hash)
		if ((search == -1) || (total_hash != mem_list[search].total_hash) || (url_hash != mem_list[search].url_hash) || (cluster_hash != mem_list[search].cluster_hash)) {
			// it's wrong so do the slow one, then change it
			search = OldFindFile(url_hash, cluster_hash, total_hash);
			mem_offset_list[i].search = search;
		}
	}

	// so it was in the list return it

	return (int16)search;
}

void res_man::AddMemOffset(uint32 hash, int32 search) {
	// if we've filled it up then get rid of everything!!!
	if (num_mem_offsets == (int32)max_mem_blocks)
		num_mem_offsets = 0;

	// next find the location where we want to be...

	int32 i = 0;

	// if we are not last in list so need to move some down!
	if (num_mem_offsets != 0) {
		i = num_mem_offsets;

		// while the one in the list is bigger, move it up
		while ((i > 0) && (mem_offset_list[i - 1].total_hash > hash)) {
			mem_offset_list[i] = mem_offset_list[i - 1];
			i--;
		}

		// we should now have i pointing to correct place
	}

	mem_offset_list[i].total_hash = hash;
	mem_offset_list[i].search = search;

	// another mem block
	num_mem_offsets++;
}

int16 res_man::FindFile(RMParams *params) { return FindFile(params->url_hash, params->cluster_hash, MAKE_TOTAL_HASH(params->cluster_hash, params->url_hash)); }

void res_man::FindFileCluster(int32 &url_search, int32 &cluster_search, RMParams *params) {
	cluster_search = FindFile(0, params->cluster_hash, MAKE_TOTAL_HASH(params->cluster_hash, 0));
	url_search = FindFile(params->url_hash, params->cluster_hash, MAKE_TOTAL_HASH(params->cluster_hash, params->url_hash));
}

void res_man::Res_purge_all() {
	// remove ALL resources from memory
	int16 parent, search, child, grandchild;

	Zdebug("---purging ALL---");

	search = 0;
	do {
		if (mem_list[search].state == MEM_in_use) { // if a used block with a file...

			//          search is number of mem block to remove

			Zdebug(" search == %d", search);
			//                              one less file open
			number_files_open--; // one less file in memory

			mem_list[search].url_hash = NULL_HASH;
			mem_list[search].cluster_hash = NULL_HASH;
			mem_list[search].total_hash = NULL_HASH;

			//                              regain the mem
			total_free_memory += mem_list[search].size; // pool regains this memory

			//                              ** remember, although the file and its name are gone this block may remain as a MEM_free **

			child = mem_list[search].child; // this resources child

			//				merge free child into us if possible
			if ((child != -1) && (mem_list[child].state == MEM_free)) { // we're not endblock and child is free
				grandchild = mem_list[child].child; // we get our childs child
				mem_list[search].child = grandchild; // we get our childs child
				mem_list[search].size += mem_list[child].size; // we get our childs memory

				if (grandchild != -1)
					mem_list[grandchild].parent = search; // we are the grandchild's new parent  **JAKE 15FEB99**

				mem_list[child].state = MEM_null; // remove the child
				total_blocks--; // one less mem block - number of files open remains the same
			}

			//                              merge into our parent if our parent is free and we're not base block - removing ourselves
			parent = mem_list[search].parent; // our parent
			if ((parent != -1) && (search != 0) && (mem_list[parent].state == MEM_free)) { // we're not the first so can we merge with our parent?
				grandchild = mem_list[search].child; // our parent gets our child
				mem_list[parent].child = grandchild;
				mem_list[parent].size += mem_list[search].size; // our parent gets our memory

				if (grandchild != -1)
					mem_list[grandchild].parent = parent; // our child gets our parent

				mem_list[search].state = MEM_null; // we're gone and can be used again later

				total_blocks--; // one less block
			} else { // we can't merge into our parent because it isn't free (or we are block 0) so we become
				// a MEM_free floating block with no file
				mem_list[search].state = MEM_free; // block remains but is free to be defragged
			}
		}

		search = mem_list[search].child;
	} while (search != -1);

	// Reset the timer
	current_time_frame = 1;
}

uint32 res_man::Check_file_size(const char *url, uint32 url_hash, const char *cluster, uint32 cluster_hash) {
	// if the file exists and will fit into memory then return 1
	// else return 0 if not exist or not fit into memory

	uint32 size;

	size = Fetch_size(url, url_hash, cluster, cluster_hash);

	if (!size)
		return (0);

	if (total_pool > size)
		return (1);

	return (0);
}

uint8 *res_man::Internal_open(RMParams *params, int32 *ret_len) {
	// Loads if not in memory here already

	// resources cannot be locked in memory

	// if space cannot be found then ALL the resources loaded during a previous time-frame will be removed
	// as graphic resources are to be loaded en-mass when scenes begin the trash process should make a sizeable single block
	// time-frames are removed continuously until enough space is made - until the current time-frame is reached.

	// input         pointer to full path and name of resource file
	// returns       address of resource.

	int32 search = 0;         // mem block number during name search
	int32 cluster_search = 0; // mem block number where the cluster for this file is

	// check for auto time frame advance
	if (auto_time_advance)
		Advance_time_stamp();

	// is the file open already?

	// if there are files open then search to see if this file is open
	FindFileCluster(search, cluster_search, params);

	// Hey the file was found : good news
	if (search != -1) {
		if (ret_len)
			*ret_len = mem_list[search].size;

		//  inc stamp
		if (auto_time_advance) {
			// restamp the file with current time
			mem_list[search].age = current_time_frame;
			// restamp the cluster with current time (if the cluster was found)
			if (cluster_search != -1)
				mem_list[cluster_search].age = current_time_frame;
		}

		return (mem_list[search].ad);
	}

	// right we don't have the file so we better load it...

	uint8 *ptr = LoadFile(cluster_search, params);
	if (params->not_ready_yet) {
		// we are preloading the header but we are doing a loadnow so wait till it's loaded...
		if (params->mode == RM_LOADNOW) {
			while (params->not_ready_yet) {
				Fatal_error("This async shit should not happen on pc");
				ptr = LoadFile(cluster_search, params);
			}
		}
		// otherwise we're doing an async so it's alright to return 9
		else
			return nullptr;
	}

	// Pass the 8-byte aligned length back to calling function
	if (ret_len)
		*ret_len = params->len;

	if (mem_list[params->search].protect)
		return nullptr;

	return ptr;
}

// Load the specified file into the memory pool
uint8 *res_man::LoadFile(int32 &cluster_search, RMParams *params) {
	uint32 adj_len;
	const char *new_url;

	params->zipped = params->compressed; // This now defaults to the supplied parameter, rather than FALSE8;

	// file not in memory so we need to load it in

	// Note, this may also load in the cluster if the required cluster cannot be found
	// If it loads in the cluster then the cluster_search variable will need to be set
	// to point the mem_block where the cluster is loaded to
	new_url = OpenFile(cluster_search, params);

	// if we are preloading the cluster
	if (params->not_ready_yet) {
		// printf("LoadFile cluster not ready yet");
		return nullptr;
	}

	// align to 8 byte boundary in length so that next resource will adjoin legally
	// so, the file was 5 bytes int32 it would end up being 8 bytes int32
	adj_len = (params->len + 7) & ~7;

	if (adj_len >= total_pool)
		Fatal_error("adj_len >= total_pool %d >= %d hash:%s cluster:%s", adj_len, total_pool, params->url_hash, params->cluster);

	params->search = FindMemBlock(adj_len, params);

	// done loading.
	mem_list[params->search].size = adj_len; // might be useful

	// read the data in
	ReadFile(new_url, params);

	mem_list[params->search].age = current_time_frame;

	number_files_open++; // one more file in memory

	return (mem_list[params->search].ad);
}

uint32 res_man::FindMemBlock(uint32 adj_len, RMParams *params) {
	int16 grandchild, child, parent;
	int16 search = 0;
	uint32 free_mblocks = 0;

	// PSX needs to save memory by having small number of mem_blocks
	// Note : check to see if we haven't run out of mem_blocks !
	// find a NULL slot for a new block
	while ((free_mblocks != max_mem_blocks) && (mem_list[free_mblocks].state != MEM_null))
		free_mblocks++;

	if (free_mblocks == max_mem_blocks) { // run out of blocks
		search = -1;
		free_mblocks = 0; // There are not any free mem_blocks
	} else {
		search = Find_space(adj_len);
		free_mblocks = 1; // there are some free mem_blocks
	}

	// PSX needs to save memory by having small number of mem_blocks

	// try to find a mem_block large enough to fit the resource
	// if we can't ditch a whole set of resources from a previous frame
	if (search == -1) {

		// failed to find space but is this resman protected?
		if (no_defrag == TRUE8) {
			if (free_mblocks == 0) {
				Fatal_error("FindMemBlock needs to shuffle or age out but resman is locked no free mblocks");
			} else {
				Fatal_error("FindMemBlock needs to shuffle or age out but resman is locked want %d got %d", adj_len, total_free_memory); //'the end'
			}
		}

		//    could not find a big enough single block
		if ((free_mblocks == 1) && (total_free_memory >= adj_len)) {
			// enough memory does exist though
			Defrag(); // float the free space to the top
			if ((search = Find_space(adj_len)) == -1) {
				Fatal_error("%d MAJOR ERROR mem full after defrag free_mblocks %d total_free_memory %d adj_len %d", id, free_mblocks, total_free_memory, adj_len);
			}

			//                      search is the block
		} else { // not enough space free so chuck resources out until there is

			bool8 debug_state = zdebug;
			zdebug = TRUE8;

			uint16 *age_table = new uint16[MAX_MEM_BLOCKS];
			uint32 total_age = 0;
			uint32 cur_age = 0; // index in trash loop to current age to remove
			uint32 j, k;
			uint16 age, temp;

			//			build an age table
			search = 0;
			do {
				if (mem_list[search].state == MEM_in_use) {
					age = mem_list[search].age;

					// Because the time counter can wrap at 65535 then treat things in the future
					// as being in the past !
					if (age > current_time_frame) {
						mem_list[search].age = 0; // always removed this cycle because by definition this is oldest resource age...
						age = 0;
					}

					if (total_age > 0) { // got some entries in age table
						for (j = 0; j < total_age; j++) {
							if (age_table[j] == age) // this can happen - but not when using auto-age scheme
								break;
						}

						if (j == total_age) { // didn't find this items age in table
							age_table[total_age++] = age;
						}
					} else
						age_table[total_age++] = age; // first entry in table
				}

				search = mem_list[search].child; // move on to the next file
			} while (search != -1);

			if (!total_age)
				Fatal_error("failed to build an age table - not really possible");

			//			now sort the ages
			for (j = 0; j < total_age; j++)
				for (k = j + 1; k < total_age; k++)
					if (age_table[j] > age_table[k]) {
						temp = age_table[k];
						age_table[k] = age_table[j];
						age_table[j] = temp;
					}

			Tdebug("make_space.txt", "begin remove loop");

			do {
				// remove all resources of oldest_time_frame

				if (cur_age == total_age) {
					Fatal_error("ERROR - res_open cannot kill anymore old resources! Memory full! - available %dk  require %dk for [%X %s]",
					            (total_free_memory / 1024), (adj_len / 1024), params->url_hash, params->cluster);
				}

				search = 0;
				do {
					// find blocks that are in use of the age we want to remove
					// there can be free blocks in here - we ignore them as the space we be got back from a defrag

					if ((mem_list[search].state == MEM_in_use) && (mem_list[search].age == age_table[cur_age])) { // trash it
						//                one less file open
						number_files_open--; // one less file in memory

						mem_list[search].url_hash = NULL_HASH;
						mem_list[search].cluster_hash = NULL_HASH;
						mem_list[search].total_hash = NULL_HASH;

						//                                              regain the mem
						total_free_memory += mem_list[search].size; // pool regains this memory

						//                                              ** remember, although the file and its name are gone this block may remain as a
						//                                              MEM_free **

						child = mem_list[search].child; // this resources child
						//                                              merge free child into us if possible
						if ((child != -1) && (mem_list[child].state == MEM_free)) { // we're not endblock and child is free
							grandchild = mem_list[child].child; // we get our childs child
							mem_list[search].child = grandchild; // we get our childs child
							mem_list[search].size += mem_list[child].size; // we get our childs memory
							if (grandchild != -1)
								mem_list[grandchild].parent = search; // we are the grandchilds new parent

							mem_list[child].state = MEM_null; // remove the child
							total_blocks--; // one less mem block - number of files open remains the same
						}

						//                                              merge into our parent if our parent is free and we're not base block - removing
						//                                              ourselves
						parent = mem_list[search].parent; // our parent

						// the following if could also be
						if ((search != 0) && (parent != -1) &&
						    (mem_list[parent].state == MEM_free)) { // we're not the first so can we merge with our parent?
							grandchild = mem_list[search].child; // we get our childs child
							mem_list[parent].child = grandchild; // we get our childs child
							mem_list[parent].size += mem_list[search].size; // we get our childs memory
							if (grandchild != -1)
								mem_list[grandchild].parent = parent; // we are the grandchilds new parent

							mem_list[search].state = MEM_null; // we're gone and can be used again later

							total_blocks--; // one less block

							search = mem_list[parent].child; // continue the search from our parent new child - was ours remember
						} else { // we can't merge into our parent because it isn't free so we become a MEM_free floating block with no file
							mem_list[search].state = MEM_free; // block remains but is free to be defragged
							search = mem_list[search].child; // move on to the next file
						}
					} else { // move to next if current not oldest and free
						search = mem_list[search].child; // move on to the next file
					}

				} while (search != -1);

				cur_age++;

			} while (total_free_memory < adj_len);

			delete[] age_table;

			//       ok, we've made enough space
			Tdebug("make_space.txt", "made space - doing a defrag");
			Defrag();

			Tdebug("make_space.txt", "done the defrag");
			if ((search = Find_space(adj_len)) == -1)
				Fatal_error("MAJOR ERROR mem full after defrag??");

			Tdebug("make_space.txt", "Find_space %d worked", adj_len);
			// reset debugger
			zdebug = debug_state;
		}
	}
	return search;
}

uint8 *res_man::Res_alloc(uint32 url_hash, const char *cluster, uint32 cluster_hash, uint32 length) {
	RMParams params;
	params.len = length;
	params.url_hash = url_hash;
	params.cluster = cluster;
	params.cluster_hash = cluster_hash;
	params.mode = RM_LOADNOW;

	int32 search = 0;         // mem block number during name search
	int32 cluster_search = 0; // mem block number where the cluster for this resource is

	// check for auto time frame advance
	if (auto_time_advance)
		Advance_time_stamp();

	// if there are resources open then search to see if this resource is open
	FindFileCluster(search, cluster_search, &params);

	// Hey it was found
	if (search != -1) {
		//    inc stamp
		if (auto_time_advance) {
			// restamp the file with current time
			mem_list[search].age = current_time_frame;
			// restamp the cluster with current time (if the cluster was found)
			if (cluster_search != -1)
				mem_list[cluster_search].age = current_time_frame;
		}
		return (mem_list[search].ad);
	}

	// Hey it wasn't found so add it into the pool !

	// so, if the file was 5 bytes int32 it would end up being 8 bytes int32
	uint32 adj_len = (params.len + 7) & ~7;

	// haul the thing up if file is bigger than total memory available
	if (adj_len >= total_pool)
		Fatal_error("adj_len >= total_pool %d >= %d hash:%s cluster:%s", adj_len, total_pool, params.url_hash, params.cluster);

	params.search = FindMemBlock(adj_len, &params);

	// slot now being used
	mem_list[params.search].state = MEM_in_use;
	mem_list[params.search].url_hash = params.url_hash;
	mem_list[params.search].cluster_hash = params.cluster_hash;
	mem_list[params.search].total_hash = MAKE_TOTAL_HASH(params.cluster_hash, params.url_hash);
	mem_list[params.search].protect = 0;
	mem_list[params.search].size = adj_len; // might be useful

	mem_list[params.search].age = current_time_frame;

	number_files_open++; // one more file in memory

	return (mem_list[params.search].ad);
}

} // End of namespace ICB
