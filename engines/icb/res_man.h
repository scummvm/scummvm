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

#ifndef ICB_RES_MAN_H
#define ICB_RES_MAN_H

#include "engines/icb/p4.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"

namespace Common {
class Mutex;
}

namespace ICB {

// Horrible forward declaration, this really should be a macro, but that makes
// debugging a real bitch
// The actual body is contained in res_man_psx.h & res_man_pc.h
// as the PC & PSX have different ways of determining if the files are identical
// i.e. PC uses strcmp, PSX uses compare the hash values
inline int32 SameUrl(const char *urla, const uint32 urla_hash, const uint32 clustera_hash, const char *urlb, const uint32 urlb_hash, const uint32 clusterb_hash);

} // End of namespace ICB (To avoid nesting during includes)

#include "engines/icb/res_man_pc.h"

namespace ICB {

// Generic stuff

extern uint32 MAX_MEM_BLOCKS;

#define MEM_null 0
#define MEM_free 1
#define MEM_in_use 2

#define RM_LOADNOW 0   // load file right now
#define RM_ASYNCLOAD 1 // async load file

#define MAKE_TOTAL_HASH(c, f) (2 * c + f)

typedef struct {
	uint32 url_hash;     // hash value of the url name
	uint32 cluster_hash; // hash value of the cluster the url beint32s to
	uint8 *ad;
	uint32 size;
	uint32 protect;
	uint32 total_hash; // 2*cluster_hash+url_hash;
	int16 parent; // who is before us
	int16 child; // who is after us
	uint16 age;
	uint8 state;
	uint8 pad1;
} mem;

// MUST BE PADDED TO 8 BYTES!!!!
typedef struct {
	uint32 total_hash;
	int32 search;
} mem_offset;

// Put the res_man params in a structure to prevent passing lots of the same arguments
// from function to function
typedef struct RMParams {
	uint32 url_hash;
	const char *cluster;
	uint32 cluster_hash;
	uint32 mode;
	int32 len;
	int32 seekpos;
	int32 zipped;
	int32 search;
	int32 compressed; // No zero if the resource is compressed
	Common::SeekableReadStream *_stream;
	uint8 not_ready_yet; // are not ready yet
} RMParams;

class res_man {
	uint32 total_pool; // fixed initial memory pool
	uint32 total_free_memory;
	uint8 *memory_base;
	uint32 total_blocks; // total memory blocks in use - 1 more than number_files_open as we have the end free block to be eaten into
	uint32 number_files_open;
	uint32 max_mem_blocks;

	mem *mem_list; // mem_list[MAX_MEM_BLOCKS];

	mem_offset *mem_offset_list;
	int32 num_mem_offsets;

	uint16 current_time_frame; // inc's each time Res_open is called and is given to resource as its age cannot be allowed to start at 0!

public:
	int32 amount_of_defrags;

	res_man();
	res_man(uint32 memory_tot);
	res_man(uint8 *base, uint32 size);
	void Construct(uint8 *base, uint32 size, mem *memList, mem_offset *memOffsets, uint32 nMemBlocks);
	~res_man();

	void AddMemOffset(uint32 total_hash, int32 search); // adds to mem list, clearing out if nesesary
	inline int32 FindMemSearch(uint32 total_hash);      // returns a search file for given total_hash, generates and adds if not in
	inline int32 FindMemOffset(uint32 total_hash);      // returns index into offsets table

public:
	void Reset(); // trash all resources

	// So that other parts of the game can use this memory
	inline uint8 *Get_memory_base();
	inline uint32 Get_memory_size();

	void Set_auto_timeframe_advance();
	void Set_to_no_defrag();

	int16 find_oldest_file();
	void Garbage_removal();

	// load whole cluster (if size is specified then load that amount of data, otherwise just load
	// the cluster header first...
	void Res_open_cluster(const char *cluster_url, uint32 &cluster_hash, int32 size = -1);

	void Res_open_mini_cluster(const char *cluster_url, uint32 &cluster_hash, const char *fake_cluster_url, uint32 &fake_cluster_hash);

	// If hash or cluster_hash == NULL_HASH then the hash of url/cluster_url
	// is computed and stored in hash/cluster_hash
	uint8 *Res_open(const char *url, uint32 &url_hash, const char *cluster_url, uint32 &cluster_hash,
	                int32 compressed = 0, // non zero if the resource is compressed
	                int32 *ret_len = NULL);
	// new function to just allocate some memory for use
	// by code which wants temporary memory but not files
	// e.g. stream player, image decompressor
	uint8 *Res_alloc(uint32 url_hash, const char *cluster, uint32 cluster_hash, uint32 length);
	// If hash or cluster_hash == NULL_HASH then the hash of url/cluster_url
	void Res_purge(const char *url, uint32 url_hash, const char *cluster, uint32 cluster_hash, uint32 fatal = 1);
	void Res_purge_all();
	uint32 Fetch_size(const char *url, uint32 url_hash, const char *cluster_url, uint32 cluster_hash);

	void Advance_time_stamp();
	uint32 Check_file_size(const char *url, uint32 url_hash, const char *cluster_url, uint32 cluster_hash);
	bool8 Test_file(const char *url);
	bool8 Test_file(const char *url, uint32 url_hash, const char *cluster_url, uint32 cluster_hash);

	uint32 Fetch_files_open();
	uint32 Fetch_total_pool_size();
	uint32 Fetch_free_memory();
	uint32 Fetch_mem_used();
	mem *Fetch_mem_list();
	uint32 Fetch_max_mem_blocks();

	uint32 Fetch_old_memory(int32 number_of_cycles);

	// For testing purposes wanted to call Defrag from console
	// so made it public function
	void Defrag();

	// To aid debugging messages
	inline void Id(int32 newId);
	inline int32 Id();

private:
	inline void MakeHash(const char *s, uint32 &h);
	inline int32 CheckHash(const char *s, const uint32 h, uint32 &h2);

	const char *OpenFile(int32 &cluster_search, RMParams *params);

	// the old linear ones
	int16 OldFindFile(uint32 url_hash, uint32 cluster_hash, uint32 total_hash);

	// new binary ones
	int16 FindFile(uint32 url_hash, uint32 cluster_hash, uint32 total_hash);

	int16 FindFile(RMParams *params);
	void FindFileCluster(int32 &url_search, int32 &cluster_search, RMParams *params);

	HEADER_NORMAL *GetFileHeader(int32 &cluster_search, RMParams *params);

	inline int32 SameFile(mem *current_mem_block, const char *url, const uint32 url_hash, const uint32 cluster_hash);

	void ReadFile(const char *new_url, RMParams *params);
	uint32 FindMemBlock(uint32 adj_len, RMParams *params);
	uint8 *AllocMemory(uint32 &memory_tot);

	void Initialise(uint32 memory_tot);

	uint8 *Internal_open(RMParams *params, int32 *ret_len = NULL);

	uint8 *LoadFile(int32 &cluster_search, RMParams *params);

	int16 Find_space(uint32 len);
	uint16 Fetch_spawn(uint16 parent);

	bool8 auto_time_advance; // if true then time stamp is automatically imcremented as a file is opened
	bool8 no_defrag; // this manager is a static one so resources cannot be purged, shuffled or aged out
	uint8 id; // used for debugging messages to say which res_man you are

	uint8 padding1;
};

// binary search the mem_offset table return -1 if not found
inline int32 res_man::FindMemOffset(uint32 hash) {
	int32 top, bottom;
	int32 i;
	uint32 current;

	if (!num_mem_offsets)
		return -1;

	top = num_mem_offsets - 1;
	i = top >> 1;
	bottom = 0;

	for (;;) {
		current = mem_offset_list[i].total_hash;
		if (hash == current)
			return i;
		else if (top == bottom)
			return -1;
		else if (hash > current) {
			bottom = i + 1;
			i = (top + bottom) >> 1;
		} else {
			top = i;
			i = (top + bottom) >> 1;
		}
	}
}

inline int32 res_man::FindMemSearch(uint32 total_hash) { // returns a search file for given total_hash, generates and adds if not in
	int32 i = FindMemOffset(total_hash);
	if (i == -1)
		return -1;
	else
		return mem_offset_list[i].search;
}

inline uint8 *res_man::Get_memory_base() {
	return memory_base;
}

inline uint32 res_man::Get_memory_size() {
	return total_pool;
}

inline void res_man::Set_auto_timeframe_advance() {
	auto_time_advance = TRUE8; // timeframe will be advanced with each res-open
}

inline void res_man::Set_to_no_defrag() {
	no_defrag = TRUE8; // stop ageing out, etc
}

inline uint32 res_man::Fetch_total_pool_size() {
	return (total_pool);
}

inline uint32 res_man::Fetch_free_memory() {
	return (total_free_memory);
}

inline uint32 res_man::Fetch_files_open() {
	return (number_files_open);
}

inline mem *res_man::Fetch_mem_list() { return mem_list; }

inline uint32 res_man::Fetch_max_mem_blocks() { return max_mem_blocks; }

inline uint32 res_man::Fetch_mem_used() {
	return (total_pool - total_free_memory);
}

inline int32 res_man::SameFile(mem *current_mem_block, const char *url, const uint32 url_hash, const uint32 cluster_hash) {
	return SameUrl(url, url_hash, cluster_hash, NULL, current_mem_block->url_hash, current_mem_block->cluster_hash);
}

inline void res_man::MakeHash(const char *s, uint32 &h) {
	// if hash value is not set then make it
	if (h == NULL_HASH)
		h = EngineHashString(s);
}

inline int32 res_man::CheckHash(const char *s, const uint32 h, uint32 &h2) {
	if (!s)
		return 1;
	h2 = EngineHashString(s);
	if (h2 != h)
		return 0;
	return 1;
}

// To aid debugging messages
inline void res_man::Id(int32 newId) { id = (uint8)newId; }

inline int32 res_man::Id() { return (int32)id; }

extern uint32 memory_available; // set by memory_stats

void Memory_stats();
void Print_console_clusters();

} // End of namespace ICB

#endif // #ifndef RES_MAN_H
