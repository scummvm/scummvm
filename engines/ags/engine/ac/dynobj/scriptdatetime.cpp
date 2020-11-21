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

#include "ac/dynobj/scriptdatetime.h"

int ScriptDateTime::Dispose(const char *address, bool force) {
    // always dispose a DateTime
    delete this;
    return 1;
}

const char *ScriptDateTime::GetType() {
    return "DateTime";
}

int ScriptDateTime::Serialize(const char *address, char *buffer, int bufsize) {
    StartSerialize(buffer);
    SerializeInt(year);
    SerializeInt(month);
    SerializeInt(day);
    SerializeInt(hour);
    SerializeInt(minute);
    SerializeInt(second);
    SerializeInt(rawUnixTime);
    return EndSerialize();
}

void ScriptDateTime::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    year = UnserializeInt();
    month = UnserializeInt();
    day = UnserializeInt();
    hour = UnserializeInt();
    minute = UnserializeInt();
    second = UnserializeInt();
    rawUnixTime = UnserializeInt();
    ccRegisterUnserializedObject(index, this, this);
}

ScriptDateTime::ScriptDateTime() {
    year = month = day = 0;
    hour = minute = second = 0;
    rawUnixTime = 0;
}
