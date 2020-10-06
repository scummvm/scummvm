/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_GAME_SCRIPT
#define ICB_GAME_SCRIPT

#include "engines/icb/common/px_common.h"

namespace ICB {

class _game_script {

public:
	bool8 running_from_game_script;

	uint8 *buf;
	char cluster[ENGINE_STRING_LEN];
	char fname[ENGINE_STRING_LEN];
	uint32 fn_hash;
	uint32 cluster_hash;

	uint32 pc;

	void Restart_game_script();

	bool8 Init_game_script();

	void Run_to_bookmark(const char *name);

	void Process_game_script();

	void Fetch_next_param(char *p);

	bool8 Running_from_gamescript();

	void Fetch_next_line();
};

} // End of namespace ICB

#endif
