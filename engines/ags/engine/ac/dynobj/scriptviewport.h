//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

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
