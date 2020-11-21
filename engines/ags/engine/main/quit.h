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
#ifndef __AGS_EE_MAIN__QUIT_H
#define __AGS_EE_MAIN__QUIT_H

enum QuitReason
{
    kQuitKind_NormalExit        = 0x01,
    kQuitKind_DeliberateAbort   = 0x02,
    kQuitKind_GameException     = 0x04,
    kQuitKind_EngineException   = 0x08,

    // user closed the window or script command QuitGame was executed
    kQuit_GameRequest   = kQuitKind_NormalExit | 0x10,

    // user pressed abort game key
    kQuit_UserAbort     = kQuitKind_DeliberateAbort | 0x20,
    
    // script command AbortGame was executed
    kQuit_ScriptAbort   = kQuitKind_GameException | 0x10,
    // game logic has generated a warning and warnings are treated as error
    kQuit_GameWarning   = kQuitKind_GameException | 0x20,
    // game logic has generated an error (often script error)
    kQuit_GameError     = kQuitKind_GameException | 0x30,

    // any kind of a fatal engine error
    kQuit_FatalError    = kQuitKind_EngineException
};

#endif // __AGS_EE_MAIN__QUIT_H
