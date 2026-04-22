/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "mads/madsv2/core/general.h"
#include "rail.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/kernel.h"

namespace MADS {
namespace MADSV2 {

 /* -------------------------------------------------------------------------
  * Constants
  * ---------------------------------------------------------------------- */

#define RAIL_STRUCTURE_SIZE             48
#define RAIL_STRUCTURE_WEIGHT_OFFSET    4   /* byte offset of weight[] within a rail node */

#define RAIL_WEIGHT_MASK                0x3FFF
#define RAIL_ALLOW_ILLEGAL              0xC000
#define RAIL_ALLOW_LEGAL_ONLY           0x8000

  /* -------------------------------------------------------------------------
   * Rail node structure
   *
   * Each node is RAIL_STRUCTURE_SIZE (48) bytes.  The weight array starts at
   * byte offset RAIL_STRUCTURE_WEIGHT_OFFSET (4), and is indexed by node
   * number.  Each weight entry is one word (2 bytes).
   *
   * The upper two bits of each weight word encode legality:
   *   RAIL_ALLOW_ILLEGAL    (0xC000) -- path exists, even through illegal area
   *   RAIL_ALLOW_LEGAL_ONLY (0x8000) -- path exists, legal only
   *   0x0000                         -- no path
   * The lower 14 bits (RAIL_WEIGHT_MASK) hold the actual traversal weight.
   *
   * The struct below matches the 48-byte layout exactly:
   *   4 bytes of header data before the weight array,
   *   then RAIL_MAX_NODES (12) weight words = 24 bytes,
   *   padding to reach 48 bytes total.
   * ---------------------------------------------------------------------- */
struct RailNode {
	unsigned char header[RAIL_STRUCTURE_WEIGHT_OFFSET];	/* 4 bytes */
	uint16 weight[RAIL_MAX_NODES];			/* 24 bytes (12 words) */
	byte padding[48 - 4 - 24];				/* 20 bytes padding */
};


word rail_solution_stack_pointer;
word rail_solution_stack_weight;
byte rail_visited[RAIL_MAX_NODES];
byte rail_working_stack[RAIL_MAX_NODES];
byte rail_solution_stack[RAIL_MAX_NODES];
word rail_num_nodes;
Rail *rail_base;
byte rail_active[ROOM_MAX_RAILS + 2];


void rail_connect_node(int id) {
	int count;
	int x1, y1;
	int x, y;
	word legal;
	word weight;

	x = room->rail[id].x;
	y = room->rail[id].y;

	rail_active[id] = true;

	for (count = 0; count < (int)rail_num_nodes; count++) {
		if (count != id) {
			if (rail_active[count] && rail_active[id]) {
				x1 = room->rail[count].x;
				y1 = room->rail[count].y;
				if (player.walk_anywhere) {
					legal = LEGAL;
				} else {
					legal = buffer_legal(scr_walk, room->xs, x, y, x1, y1);
				}
				weight = MIN(imath_hypot(abs(x1 - x), abs(y1 - y)), WEIGHT_MASK) | legal;
			} else {
				weight = WEIGHT_MASK | TOTALLY_ILLEGAL;
			}
		} else {
			weight = WEIGHT_MASK | TOTALLY_ILLEGAL;
		}
		room->rail[count].weight[id] = weight;
		room->rail[id].weight[count] = weight;
	}
}

static void disconnector(int alpha, int beta) {
	room->rail[alpha].weight[beta] = WEIGHT_MASK | TOTALLY_ILLEGAL;
}

void rail_disconnect_line(int from, int unto) {
	disconnector(from, unto);
	disconnector(unto, from);
}

void rail_disconnect_node(int id) {
	int count;

	rail_active[id] = false;

	for (count = 0; count < (int)rail_num_nodes; count++) {
		if (count != id) {
			rail_disconnect_line(count, id);
		}
	}
}

void rail_add_node(int id, int x, int y) {
	room->rail[id].x = x;
	room->rail[id].y = y;

	rail_active[id] = true;

	rail_connect_node(id);
}

void rail_connect_all_nodes(void) {
	int count;

	for (count = 0; count < ROOM_MAX_RAILS + 2; count++) {
		rail_active[count] = true;
	}

	for (count = 0; count < room->num_rails; count++) {
		rail_connect_node(count);
	}
}


/**
 * Used for recursively determing walk path between room rail nodes
 * @param node_id		Node currently being evaluated
 * @param weight		Cumulative weight for this solution attempt
 * @param allow_mode	Mode: RAIL_ALLOW_ILLEGAL or RAIL_ALLOW_LEGAL_ONLY
 * @param working_sp	Index into _rail_working_stack, acting as a simple
 * push-down stack pointer.
 *
 * The function is recursive and modifies the globals directly.
 */
static void recursive_check_path(int node_id,
	uint16 weight,
	uint16 allow_mode,
	int working_sp) {
	// visited[node_id] = true
	rail_visited[node_id] = 1;

	// push(node_id) onto working stack
	rail_working_stack[working_sp] = (unsigned char)node_id;
	working_sp++;

	// Point at rail[node_id]
	Rail *node = &rail_base[node_id];

	// The source node is second-to-last: index = num_nodes - 2.
	// We look up the weight from node_id to the source (from_node).
	int from_node = rail_num_nodes - 2;
	uint16 raw_weight = node->weight[from_node];

	// Check whether there is a direct legal path to the destination
	if (raw_weight & allow_mode) {
		uint16 leg_weight = raw_weight & RAIL_WEIGHT_MASK;
		uint16 total = weight + leg_weight;

		if (total < rail_solution_stack_weight) {
			// This is a better solution -- save it
			int stack_len = working_sp;  // number of bytes currently on stack
			memcpy(rail_solution_stack, rail_working_stack, stack_len);
			rail_solution_stack_pointer = (uint16)stack_len;
			rail_solution_stack_weight = total;
		}
	} else {
		// No direct path -- recurse through every unvisited intermediate node.
		// Intermediate nodes are indices 0 .. (num_nodes - 3); the last two
		// entries in the array are the destination and source nodes.
		int num_intermediate = rail_num_nodes - 2;

		for (int i = 0; i < num_intermediate; i++) {
			int test_node = i;  // loop counter - 1 in the ASM

			if (rail_visited[test_node])
				continue;

			// Check edge weight from current node to test_node
			uint16 edge_raw = node->weight[test_node];
			uint16 edge_legal = edge_raw & allow_mode;

			if (!edge_legal)
				continue;  // not a legal connection

			uint16 edge_weight = edge_raw & RAIL_WEIGHT_MASK;

			// All further legs must be strictly legal once we step through
			// an intermediate node (the ASM forces RAIL_ALLOW_LEGAL_ONLY)
			recursive_check_path(test_node,
				weight + edge_weight,
				RAIL_ALLOW_LEGAL_ONLY,
				working_sp);
		}
	}

	// visited[node_id] = false  (unwind)
	rail_visited[node_id] = 0;

	// pop() -- working_sp is a local copy so this just falls off the frame
}

void rail_check_path(int allow_one_illegal) {
	// Clear the visited array
	memset(rail_visited, 0, rail_num_nodes);

	// Initialise solution state to "no solution yet"
	rail_solution_stack_weight = RAIL_WEIGHT_MASK;
	rail_solution_stack_pointer = 0;

	// The last node in the array (index num_nodes - 1) is the starting
	// node for the search (the destination, in path terms)
	int start_node = rail_num_nodes - 1;

	uint16 allow_mode = allow_one_illegal ? RAIL_ALLOW_ILLEGAL
		: RAIL_ALLOW_LEGAL_ONLY;

	recursive_check_path(start_node, 0, allow_mode, 0);
}

} // namespace MADSV2
} // namespace MADS
