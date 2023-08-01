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

#include "m4/burger/walker.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

// Starting hashes for walker machines/sequences/etc */
#define WALKER_HASH 8			  // machine/data starting hash for wilbur
#define WALKER_SERIES_HASH 0
#define NUM_WALKER_SERIES  8
#define SHADOW_SERIES_HASH 8
#define NUM_SHADOW_SERIES  5

// These are the walker types
#define WALKER_WILBUR          0
#define WALKER_FLUMIX          1

// These are the shadow types
#define SHADOW_WILBUR          0
#define SHADOW_FLUMIX          1


bool Walker::walk_load_walker_and_shadow_series() {
	// TODO
	warning("TODO: walk_load_walker_and_shadow_series");
	return true;
}

machine *Walker::walk_initialize_walker() {
#if 0
	machine *m;
	int32 s;

	// Wilbur walker
	_G(player).walker_type = WALKER_WILBUR;
	_G(player).shadow_type = SHADOW_WILBUR;

	_G(globals)[GLB_TEMP_1] = _G(player).walker_type << 16;
	_G(globals)[GLB_TEMP_2] = WALKER_SERIES_HASH << 24;  // starting series hash of default walker	        GAMECTRL loads shadows starting @ 0
	_G(globals)[GLB_TEMP_3] = SHADOW_SERIES_HASH << 24;  // starting series hash of default walker shadows. GAMECTRL loads shadows starting @ 10

	// initialize with bogus data (this is for the real walker)
	s = _G(globals)[GLB_MIN_SCALE] + FixedMul((400 << 16) - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);
	_G(globals)[GLB_TEMP_4] = 320 << 16;
	_G(globals)[GLB_TEMP_5] = 400 << 16;
	_G(globals)[GLB_TEMP_6] = s;
	_G(globals)[GLB_TEMP_7] = 3 << 16;	 // facing

	m = TriggerMachineByHash(WALKER_HASH, NULL, _G(player).walker_type + WALKER_HASH, 0, player_walker_callback, false, "Wilbur Walker");

	// we need to all init sequences to happen immediately (init coordinates)
	CycleEngines(NULL, &(currentSceneDef.depth_table[0]),
		NULL, (uint8 *)&master_palette[0], inverse_pal->get_ptr(), TRUE);

	inverse_pal->release();

	return m;
#else
	error("TODO: walk_initialize_walker");
#endif
}

} // namespace Burger
} // namespace M4
