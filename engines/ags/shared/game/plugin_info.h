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

//=============================================================================
//
// PluginInfo - a struct defining general information on game plugin.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_PLUGIN_INFO_H
#define AGS_SHARED_GAME_PLUGIN_INFO_H

#include "common/std/memory.h"
#include "ags/shared/util/string.h"

// TODO: why 10 MB limit?
#define PLUGIN_SAVEBUFFERSIZE 10247680

namespace AGS3 {
namespace AGS {
namespace Shared {

struct PluginInfo {
	// (File)name of plugin
	String      Name;
	// Custom data for plugin
	std::vector<uint8_t> Data;

	PluginInfo() = default;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
