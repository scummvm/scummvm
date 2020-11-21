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

#ifndef __AC_SCRIPTVIEWPORT_H
#define __AC_SCRIPTVIEWPORT_H

#include "ac/dynobj/cc_agsdynamicobject.h"

// ScriptViewport keeps a reference to actual room Viewport in script.
struct ScriptViewport final : AGSCCDynamicObject
{
public:
    ScriptViewport(int id);
    // Get viewport index; negative means the viewport was deleted
    int GetID() const { return _id; }
    void SetID(int id) { _id = id; }
    // Reset viewport index to indicate that this reference is no longer valid
    void Invalidate() { _id = -1; }

    const char *GetType() override;
    int Dispose(const char *address, bool force) override;
    int Serialize(const char *address, char *buffer, int bufsize) override;
    void Unserialize(int index, const char *serializedData, int dataSize) override;

private:
    int _id = -1; // index of viewport in the game state array
};

// Unserialize viewport from the memory stream
ScriptViewport *Viewport_Unserialize(int handle, const char *serializedData, int dataSize);

#endif // __AC_SCRIPTVIEWPORT_H
