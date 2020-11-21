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
#ifndef __AGS_EE_AC__STRING_H
#define __AGS_EE_AC__STRING_H

#include <stdarg.h>
#include "ac/dynobj/cc_dynamicobject.h"

// Check that a supplied buffer from a text script function was not null
#define VALIDATE_STRING(strin) if ((unsigned long)strin <= 4096) quit("!String argument was null: make sure you pass a string, not an int, as a buffer")

int String_IsNullOrEmpty(const char *thisString);
const char* String_Copy(const char *srcString);
const char* String_Append(const char *thisString, const char *extrabit);
const char* String_AppendChar(const char *thisString, char extraOne);
const char* String_ReplaceCharAt(const char *thisString, int index, char newChar);
const char* String_Truncate(const char *thisString, int length);
const char* String_Substring(const char *thisString, int index, int length);
int String_CompareTo(const char *thisString, const char *otherString, bool caseSensitive);
int String_StartsWith(const char *thisString, const char *checkForString, bool caseSensitive);
int String_EndsWith(const char *thisString, const char *checkForString, bool caseSensitive);
const char* String_Replace(const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive);
const char* String_LowerCase(const char *thisString);
const char* String_UpperCase(const char *thisString);
int String_GetChars(const char *texx, int index);
int StringToInt(const char*stino);
int StrContains (const char *s1, const char *s2);

//=============================================================================

const char* CreateNewScriptString(const char *fromText, bool reAllocate = true);
DynObjectRef CreateNewScriptStringObj(const char *fromText, bool reAllocate = true);
class SplitLines;
// Break up the text into lines restricted by the given width;
// returns number of lines, or 0 if text cannot be split well to fit in this width.
// Does additional processing, like removal of voice-over tags and text reversal if right-to-left text display is on.
size_t break_up_text_into_lines(const char *todis, SplitLines &lines, int wii, int fonnt, size_t max_lines = -1);
void check_strlen(char*ptt);
void my_strncpy(char *dest, const char *src, int len);

#endif // __AGS_EE_AC__STRING_H
