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
#ifndef __AGS_EE_AC__SPEECH_H
#define __AGS_EE_AC__SPEECH_H

enum SkipSpeechStyle
{
    kSkipSpeechUndefined    = -1,
    kSkipSpeechKeyMouseTime =  0,
    kSkipSpeechKeyTime      =  1,
    kSkipSpeechTime         =  2,
    kSkipSpeechKeyMouse     =  3,
    kSkipSpeechMouseTime    =  4,
    kSkipSpeechKey          =  5,
    kSkipSpeechMouse        =  6,

    kSkipSpeechFirst        = kSkipSpeechKeyMouseTime,
    kSkipSpeechLast         = kSkipSpeechMouse
};

int user_to_internal_skip_speech(SkipSpeechStyle userval);
SkipSpeechStyle internal_skip_speech_to_user(int internal_val);

#endif // __AGS_EE_AC__SPEECH_H
