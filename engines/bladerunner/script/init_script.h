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

#ifndef BLADERUNNER_SCRIPT_INIT_H
#define BLADERUNNER_SCRIPT_INIT_H

#include "bladerunner/script/script.h"

namespace BladeRunner {

class BladeRunnerEngine;

class InitScript : ScriptBase {
public:
	InitScript(BladeRunnerEngine *vm)
		: ScriptBase(vm) {
	}

	void SCRIPT_Initialize_Game();

private:
	void Init_Globals();
	void Init_Game_Flags();
	void Init_Clues();
	void Init_Clues2();
	void Init_World_Waypoints();
	void Init_SDB();
	void Init_CDB();
	void Init_Spinner();
	void Init_Actor_Friendliness();
	void Init_Actor_Combat_Aggressiveness();
	void Init_Actor_Honesty();
	void Init_Actor_Intelligence();
	void Init_Actor_Stability();
	void Init_Actor_Health();
	void Init_Combat_Cover_Waypoints();
	void Init_Combat_Flee_Waypoints();
	void Init_Shadows();
};

} // End of namespace BladeRunner

#endif
