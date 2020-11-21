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

#include "ac/dynobj/scriptstring.h"
#include "ac/string.h"
#include <stdlib.h>
#include <string.h>

DynObjectRef ScriptString::CreateString(const char *fromText) {
    return CreateNewScriptStringObj(fromText);
}

int ScriptString::Dispose(const char *address, bool force) {
    // always dispose
    if (text) {
        free(text);
        text = nullptr;
    }
    delete this;
    return 1;
}

const char *ScriptString::GetType() {
    return "String";
}

int ScriptString::Serialize(const char *address, char *buffer, int bufsize) {
    StartSerialize(buffer);
    
    auto toSerialize = text ? text : "";
    
    auto len = strlen(toSerialize);
    SerializeInt(len);
    strcpy(&serbuffer[bytesSoFar], toSerialize);
    bytesSoFar += len + 1;
    
    return EndSerialize();
}

void ScriptString::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int textsize = UnserializeInt();
    text = (char*)malloc(textsize + 1);
    strcpy(text, &serializedData[bytesSoFar]);
    ccRegisterUnserializedObject(index, text, this);
}

ScriptString::ScriptString() {
    text = nullptr;
}

ScriptString::ScriptString(const char *fromText) {
    text = (char*)malloc(strlen(fromText) + 1);
    strcpy(text, fromText);
}
