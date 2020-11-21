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

#include "ac/dynobj/cc_region.h"
#include "ac/dynobj/scriptregion.h"
#include "ac/common_defines.h"
#include "game/roomstruct.h"

extern ScriptRegion scrRegion[MAX_ROOM_REGIONS];

// return the type name of the object
const char *CCRegion::GetType() {
    return "Region";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCRegion::Serialize(const char *address, char *buffer, int bufsize) {
    ScriptRegion *shh = (ScriptRegion*)address;
    StartSerialize(buffer);
    SerializeInt(shh->id);
    return EndSerialize();
}

void CCRegion::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int num = UnserializeInt();
    ccRegisterUnserializedObject(index, &scrRegion[num], this);
}
