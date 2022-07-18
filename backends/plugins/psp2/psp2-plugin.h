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

#ifndef BACKENDS_PLUGINS_PSP2_PLUGIN_H
#define BACKENDS_PLUGINS_PSP2_PLUGIN_H

#if defined(DYNAMIC_MODULES) && defined(PSP2)

struct PSP2FunctionPointers {
	uint32 version;
	int32 (*PLUGIN_getVersion)();
	int32 (*PLUGIN_getType)();
	int32 (*PLUGIN_getTypeVersion)();
	PluginObject *(*PLUGIN_getObject)();
};

// Increment this when modifying the structure above
#define PSP2FunctionPointers_VERSION 1

#endif // defined(DYNAMIC_MODULES) && defined(PSP2)

#endif

