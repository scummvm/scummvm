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

#include "bladerunner/script/init.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

void ScriptInit::SCRIPT_Initialize_Game() {
	Assign_Player_Gun_Hit_Sounds(0, 517, 518, 519);
	Assign_Player_Gun_Hit_Sounds(1, 520, 521, 522);
	Assign_Player_Gun_Hit_Sounds(2, 523, 524, 525);

	Assign_Player_Gun_Miss_Sounds(0, 526, 527, 528);
	Assign_Player_Gun_Miss_Sounds(1, 529, 530, 531);
	Assign_Player_Gun_Miss_Sounds(2, 532, 533, 534);

	Init_Globals();
	Init_Game_Flags();
	Init_Clues();
	Init_Clues2();
	Init_World_Waypoints();
	Init_SDB();
	Init_CDB();
	Init_Spinner();
	Init_Actor_Friendliness();
	Init_Combat_Aggressiveness();
	Init_Actor_Honesty();
	Init_Actor_Intelligence();
	Init_Actor_Stability();
	Init_Actor_Health();
	Init_Combat_Cover_Waypoints();
	Init_Combat_Flee_Waypoints();
	Init_Shadows();
}

void ScriptInit::Init_Globals() {

}

void ScriptInit::Init_Game_Flags() {

}

void ScriptInit::Init_Clues() {

}

void ScriptInit::Init_Clues2() {

}

void ScriptInit::Init_World_Waypoints() {

}

void ScriptInit::Init_SDB() {

}

void ScriptInit::Init_CDB() {

}

void ScriptInit::Init_Spinner() {

}

void ScriptInit::Init_Actor_Friendliness() {

}

void ScriptInit::Init_Combat_Aggressiveness() {

}

void ScriptInit::Init_Actor_Honesty() {

}

void ScriptInit::Init_Actor_Intelligence() {

}

void ScriptInit::Init_Actor_Stability() {

}

void ScriptInit::Init_Actor_Health() {

}

void ScriptInit::Init_Combat_Cover_Waypoints() {

}

void ScriptInit::Init_Combat_Flee_Waypoints() {

}

void ScriptInit::Init_Shadows() {

}



} // End of namespace BladeRunner
