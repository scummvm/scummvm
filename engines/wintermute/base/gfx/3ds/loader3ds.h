/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WINTERMUTE_LOADER3DS_H
#define WINTERMUTE_LOADER3DS_H

#include "engines/wintermute/coll_templ.h"

namespace Wintermute {

class BaseGame;
class Light3D;
class Camera3D;
class Mesh3DS;

bool load3DSFile(const char *filename, BaseArray<Mesh3DS *> &meshes, BaseArray<Common::String> &meshNames,
                 BaseArray<Light3D *> &lights, BaseArray<Camera3D *> &cameras, BaseGame *gameRef);

enum Chunks3DS {
	RGB_FLOAT = 0x0010,
	RGB_BYTE = 0x0011,
	EDITOR = 0x3D3D,
	OBJECT = 0x4000,
	MESH = 0x4100,
	VERTICES = 0x4110,
	FACES = 0x4120,
	FACES_MATERIAL = 0x4130,
	MAPPING_COORDS = 0x4140,
	SMOOTHING_GROUPS = 0x4150,
	LOCAL_COORDS = 0x4160,
	LIGHT = 0x4600,
	SPOTLIGHT = 0x4610,
	LIGHT_IS_OFF = 0x4620,
	SPOT_RAY_TRACE = 0x4627,
	SPOT_SHADOW_MAP = 0x4641,
	ROLL = 0x4656,
	SPOT_RAY_TRACE_BIAS = 0x4658,
	RANGE_END = 0x465A,
	MULTIPLIER = 0x465B,
	CAMERA = 0x4700,
	MAIN = 0x4D4D,
	KEYFRAMER = 0xB000,
	AMBIENT_INFO = 0xB001,
	MESH_INFO = 0xB002,
	CAMERA_INFO = 0xB003,
	CAMERA_TARGET_INFO = 0xB004,
	OMNI_LIGHT_INFO = 0xB005,
	SPOTLIGHT_TARGET_INFO = 0xB006,
	SPOTLIGHT_INFO = 0xB007,
	NODE_HEADER = 0xB010,
	ROLL_TRACK = 0xB024
};

} // namespace Wintermute
#endif
