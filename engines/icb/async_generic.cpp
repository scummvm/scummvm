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

#include "engines/icb/debug.h"
#include "engines/icb/session.h"
#include "engines/icb/actor.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"
#include "engines/icb/mission.h"

namespace ICB {

mcodeFunctionReturnCodes fn_preload_basics(int32 &result, int32 *params) { return (MS->fn_preload_basics(result, params)); }

	// function loads some standard anim and marker files in, designed to be used at session
	// init. this function run for a few megas (ie the ones that are on screen)
	// should ease off the preloader slightly and create less chug on psx...!
mcodeFunctionReturnCodes _game_session::fn_preload_basics(int32 &, int32 *) {
	// if this is init script, then I is not set yet, so do so now...
	I = L->voxel_info;

	rs_anims->Res_open_mini_cluster(I->base_path, I->base_path_hash, I->base_path, I->base_path_hash);

	// okay
	return (IR_CONT);
}


} // End of namespace ICB
