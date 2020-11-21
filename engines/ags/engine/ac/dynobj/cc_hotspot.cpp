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

#include "ac/dynobj/cc_hotspot.h"
#include "ac/dynobj/scripthotspot.h"
#include "ac/common_defines.h"
#include "game/roomstruct.h"

extern ScriptHotspot scrHotspot[MAX_ROOM_HOTSPOTS];

// return the type name of the object
const char *CCHotspot::GetType() {
    return "Hotspot";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCHotspot::Serialize(const char *address, char *buffer, int bufsize) {
    ScriptHotspot *shh = (ScriptHotspot*)address;
    StartSerialize(buffer);
    SerializeInt(shh->id);
    return EndSerialize();
}

void CCHotspot::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int num = UnserializeInt();
    ccRegisterUnserializedObject(index, &scrHotspot[num], this);
}