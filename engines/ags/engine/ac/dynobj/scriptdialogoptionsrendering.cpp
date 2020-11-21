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

#include "ac/dynobj/scriptdialogoptionsrendering.h"

// return the type name of the object
const char *ScriptDialogOptionsRendering::GetType() {
    return "DialogOptionsRendering";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int ScriptDialogOptionsRendering::Serialize(const char *address, char *buffer, int bufsize) {
    return 0;
}

void ScriptDialogOptionsRendering::Unserialize(int index, const char *serializedData, int dataSize) {
    ccRegisterUnserializedObject(index, this, this);
}

void ScriptDialogOptionsRendering::Reset()
{
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    hasAlphaChannel = false;
    parserTextboxX = 0;
    parserTextboxY = 0;
    parserTextboxWidth = 0;
    dialogID = 0;
    surfaceToRenderTo = nullptr;
    surfaceAccessed = false;
    activeOptionID = -1;
    chosenOptionID = -1;
    needRepaint = false;
}

ScriptDialogOptionsRendering::ScriptDialogOptionsRendering()
{
    Reset();
}
