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
#ifndef __AGS_EE_AC__GLOBALDISPLAY_H
#define __AGS_EE_AC__GLOBALDISPLAY_H

#include "ac/speech.h"

void Display(const char*texx, ...); // applies translation
void DisplaySimple(const char* text); // does not apply translation
void DisplayAt(int xxp,int yyp,int widd, const char*text);
void DisplayAtY (int ypos, const char *texx);
void DisplayMessage(int msnum);
void DisplayMessageAtY(int msnum, int ypos);
void DisplayTopBar(int ypos, int ttexcol, int backcol, const char *title, const char *text);
// Display a room/global message in the bar
void DisplayMessageBar(int ypos, int ttexcol, int backcol, const char *title, int msgnum);

void SetSpeechStyle (int newstyle);
void SetSkipSpeech (SkipSpeechStyle newval);
SkipSpeechStyle GetSkipSpeech();

#endif // __AGS_EE_AC__GLOBALDISPLAY_H
