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
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__AUDIOCLIP_H
#define __AGS_EE_AC__AUDIOCLIP_H

#include "ac/dynobj/scriptaudioclip.h"
#include "ac/dynobj/scriptaudiochannel.h"

int     AudioClip_GetFileType(ScriptAudioClip *clip);
int     AudioClip_GetType(ScriptAudioClip *clip);
int     AudioClip_GetIsAvailable(ScriptAudioClip *clip);
void    AudioClip_Stop(ScriptAudioClip *clip);
ScriptAudioChannel* AudioClip_Play(ScriptAudioClip *clip, int priority, int repeat);
ScriptAudioChannel* AudioClip_PlayFrom(ScriptAudioClip *clip, int position, int priority, int repeat);
ScriptAudioChannel* AudioClip_PlayQueued(ScriptAudioClip *clip, int priority, int repeat);

#endif // __AGS_EE_AC__AUDIOCLIP_H
