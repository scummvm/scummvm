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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTCONTAINERS_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTCONTAINERS_H

namespace AGS3 {

class ScriptDictBase;
class ScriptSetBase;

// Create and register new dictionary
ScriptDictBase *Dict_Create(bool sorted, bool case_sensitive);
// Unserialize dictionary from the memory stream
ScriptDictBase *Dict_Unserialize(int index, const char *serializedData, int dataSize);
// Create and register new set
ScriptSetBase *Set_Create(bool sorted, bool case_sensitive);
// Unserialize set from the memory stream
ScriptSetBase *Set_Unserialize(int index, const char *serializedData, int dataSize);

} // namespace AGS3

#endif
