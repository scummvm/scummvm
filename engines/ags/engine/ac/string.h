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

#ifndef AGS_ENGINE_AC_STRING_H
#define AGS_ENGINE_AC_STRING_H

//include <stdarg.h>
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"

namespace AGS3 {

// Check that a supplied buffer from a text script function was not null
#define VALIDATE_STRING(strin) if (!strin) quit("!String argument was null: make sure you pass a string buffer")

int String_IsNullOrEmpty(const char *thisString);
const char *String_Copy(const char *srcString);
const char *String_Append(const char *thisString, const char *extrabit);
const char *String_AppendChar(const char *thisString, char extraOne);
const char *String_ReplaceCharAt(const char *thisString, int index, char newChar);
const char *String_Truncate(const char *thisString, int length);
const char *String_Substring(const char *thisString, int index, int length);
int String_CompareTo(const char *thisString, const char *otherString, bool caseSensitive);
int String_StartsWith(const char *thisString, const char *checkForString, bool caseSensitive);
int String_EfndsWith(const char *thisString, const char *checkForString, bool caseSensitive);
const char *String_Replace(const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive);
const char *String_LowerCase(const char *thisString);
const char *String_UpperCase(const char *thisString);
int String_GetChars(const char *texx, int index);
int StringToInt(const char *stino);
int StrContains(const char *s1, const char *s2);
int String_EndsWith(const char *thisString, const char *checkForString, bool caseSensitive);

//=============================================================================

const char *CreateNewScriptString(const char *fromText, bool reAllocate = true);
DynObjectRef CreateNewScriptStringObj(const char *fromText, bool reAllocate = true);
class SplitLines;
// Break up the text into lines restricted by the given width;
// returns number of lines, or 0 if text cannot be split well to fit in this width.
// Does additional processing, like removal of voice-over tags and text reversal if right-to-left text display is on.
size_t break_up_text_into_lines(const char *todis, SplitLines &lines, int wii, int fonnt, size_t max_lines = -1);
void check_strlen(char *ptt);
void my_strncpy(char *dest, const char *src, int len);

} // namespace AGS3

#endif
