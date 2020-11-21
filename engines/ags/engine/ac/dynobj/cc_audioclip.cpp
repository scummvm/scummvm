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

#include "ac/dynobj/cc_audioclip.h"
#include "ac/dynobj/scriptaudioclip.h"
#include "ac/gamesetupstruct.h"

extern GameSetupStruct game;

const char *CCAudioClip::GetType() {
    return "AudioClip";
}

int CCAudioClip::Serialize(const char *address, char *buffer, int bufsize) {
    ScriptAudioClip *ach = (ScriptAudioClip*)address;
    StartSerialize(buffer);
    SerializeInt(ach->id);
    return EndSerialize();
}

void CCAudioClip::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int id = UnserializeInt();
    ccRegisterUnserializedObject(index, &game.audioClips[id], this);
}
