/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_fire/ags_fire.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFire {

IAGSEngine *AGSFire::_engine;

AGSFire::AGSFire() : PluginBase() {
	_engine = nullptr;

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSFire::AGS_GetPluginName() {
	return "Fire Plugin stub (ags_fire.dll)";
}

void AGSFire::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	SCRIPT_METHOD(FireAddObject);
	SCRIPT_METHOD(FirePreHeat);
	SCRIPT_METHOD(FireDisableSeeding);
	SCRIPT_METHOD(FireEnableSeeding);
	SCRIPT_METHOD(FireSetStrength);
	SCRIPT_METHOD(FireRemoveObject);
	SCRIPT_METHOD(FireUpdate);
	SCRIPT_METHOD(FireStop);
}

void AGSFire::FireAddObject(ScriptMethodParams &params) {
	//PARAMS3(int, object, int, seedSprite, int, paletteSprite)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FirePreHeat(ScriptMethodParams &params) {
	//PARAMS1(int, object)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FireDisableSeeding(ScriptMethodParams &params) {
	//PARAMS1(int, object)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FireEnableSeeding(ScriptMethodParams &params) {
	//PARAMS1(int, object)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FireSetStrength(ScriptMethodParams &params) {
	//PARAMS2(int, object, int, strength)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FireRemoveObject(ScriptMethodParams &params) {
	//PARAMS1(int, object)
	// TODO rest of the code

	params._result = 0;
}

void AGSFire::FireUpdate(ScriptMethodParams &params) {
	// TODO rest of the owl

	params._result = 0;
}

void AGSFire::FireStop(ScriptMethodParams &params) {
	// TODO rest of the owl

	params._result = 0;
}

} // namespace AGSFire
} // namespace Plugins
} // namespace AGS3
