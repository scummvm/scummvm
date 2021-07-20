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

#include "common/util.h"
#include "ags/plugins/ags_waves/ags_waves.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

void AGSWaves::NoiseCreator(ScriptMethodParams &params) {
	//PARAMS2(int, graphic, int, setA);
}

void AGSWaves::SFX_Play(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, repeat);
}

void AGSWaves::SFX_SetVolume(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, volume);
}

void AGSWaves::SFX_GetVolume(ScriptMethodParams &params) {
	//PARAMS1(int, SFX);
	params._result = 0;
}

void AGSWaves::Music_Play(ScriptMethodParams &params) {
	//PARAMS6(int, MFX, int, repeat, int, fadeinMS, int, fadeoutMS, int, Position, bool, fixclick);
}

void AGSWaves::Music_SetVolume(ScriptMethodParams &params) {
	//PARAMS1(int, volume);
}

void AGSWaves::Music_GetVolume(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWaves::SFX_Stop(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, fademsOUT);
}

void AGSWaves::SFX_SetPosition(ScriptMethodParams &params) {
	//PARAMS4(int, SFX, int, x, int, y, int, intensity);
}

void AGSWaves::SFX_SetGlobalVolume(ScriptMethodParams &params) {
	//PARAMS1(int, volume);
}

void AGSWaves::Load_SFX(ScriptMethodParams &params) {
	//PARAMS1(int, SFX);
}

void AGSWaves::Audio_Apply_Filter(ScriptMethodParams &params) {
	//PARAMS1(int, Frequency);
}

void AGSWaves::Audio_Remove_Filter(ScriptMethodParams &params) {
}

void AGSWaves::SFX_AllowOverlap(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, allow);
}

void AGSWaves::SFX_Filter(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, enable);
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
