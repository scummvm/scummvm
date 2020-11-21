/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_AC_GLOBALDISPLAY_H
#define AGS_ENGINE_AC_GLOBALDISPLAY_H

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

#endif
