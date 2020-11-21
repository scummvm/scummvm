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

//=============================================================================
//
// Platform-independent Directory functions
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_DIRECTORY_H
#define AGS_SHARED_UTIL_DIRECTORY_H

#include "core/platform.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

namespace Directory
{
    // Creates new directory (if it does not exist)
    bool   CreateDirectory(const String &path);
    // Makes sure all directories in the path are created. Parent path is
    // not touched, and function must fail if parent path is not accessible.
    bool   CreateAllDirectories(const String &parent, const String &path);
    // Sets current working directory, returns the resulting path
    String SetCurrentDirectory(const String &path);
    // Gets current working directory
    String GetCurrentDirectory();
} // namespace Directory

} // namespace Common
} // namespace AGS

#endif
