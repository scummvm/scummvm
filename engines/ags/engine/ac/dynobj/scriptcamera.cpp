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

#include "ac/dynobj/scriptcamera.h"
#include "ac/gamestate.h"
#include "util/bbop.h"

using namespace AGS::Common;

ScriptCamera::ScriptCamera(int id) : _id(id) {}

const char *ScriptCamera::GetType()
{
    return "Camera2";
}

int ScriptCamera::Dispose(const char *address, bool force)
{
    // Note that ScriptCamera is a reference to actual Camera object,
    // and this deletes the reference, while camera may remain in GameState.
    delete this;
    return 1;
}

int ScriptCamera::Serialize(const char *address, char *buffer, int bufsize)
{
    StartSerialize(buffer);
    SerializeInt(_id);
    return EndSerialize();
}

void ScriptCamera::Unserialize(int index, const char *serializedData, int dataSize)
{
    StartUnserialize(serializedData, dataSize);
    _id = UnserializeInt();
    ccRegisterUnserializedObject(index, this, this);
}

ScriptCamera *Camera_Unserialize(int handle, const char *serializedData, int dataSize)
{
    // The way it works now, we must not create a new script object,
    // but acquire one from the GameState, which keeps the first reference.
    // This is essential because GameState should be able to invalidate any
    // script references when Camera gets removed.
    const int id = BBOp::Int32FromLE(*((int*)serializedData));
    if (id >= 0)
    {
        auto scam = play.RegisterRoomCamera(id, handle);
        if (scam)
            return scam;
    }
    return new ScriptCamera(-1); // make invalid reference
}
