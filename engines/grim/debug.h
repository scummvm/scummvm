/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_DEBUG_H
#define GRIM_DEBUG_H

#include "common/debug.h"
#include "common/streamdebug.h"

namespace Grim {

enum enDebugLevels {
	DEBUG_NONE = 0,
	DEBUG_NORMAL = 1,
	DEBUG_WARN = 2,
	DEBUG_ERROR = 4,
	DEBUG_LUA = 8,
	DEBUG_BITMAPS = 16,
	DEBUG_MODEL = 32,
	DEBUG_STUB = 64,
	DEBUG_SMUSH = 128,
	DEBUG_IMUSE = 256,
	DEBUG_CHORES = 512,
	DEBUG_ALL = DEBUG_NORMAL | DEBUG_WARN | DEBUG_ERROR | DEBUG_LUA | DEBUG_BITMAPS |
	            DEBUG_MODEL | DEBUG_STUB | DEBUG_SMUSH | DEBUG_IMUSE | DEBUG_CHORES
};

}

#endif
