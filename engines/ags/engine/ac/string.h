/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_AC_STRING_H
#define AGS_ENGINE_AC_STRING_H

//include <stdarg.h>
#include "ags/engine/ac/dynobj/cc_script_object.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

// Check that a supplied buffer from a text script function was not null
#define VALIDATE_STRING(strin) if (!strin) quit("!String argument was null: make sure you pass a string buffer")

const char *CreateNewScriptString(const char *text);
inline const char *CreateNewScriptString(const AGS::Shared::String &text) { return CreateNewScriptString(text.GetCStr()); }
char *CreateNewScriptString(size_t buf_len); // FIXME, unsafe to expose raw buf like this

int String_IsNullOrEmpty(const char *thisString);
const char *String_Copy(const char *srcString);
const char *String_Append(const char *thisString, const char *extrabit);
const char *String_AppendChar(const char *thisString, int extraOne);
const char *String_ReplaceCharAt(const char *thisString, int index, int newChar);
const char *String_Truncate(const char *thisString, int length);
const char *String_Substring(const char *thisString, int index, int length);
int String_CompareTo(const char *thisString, const char *otherString, bool caseSensitive);
int String_StartsWith(const char *thisString, const char *checkForString, bool caseSensitive);
int String_EndsWith(const char *thisString, const char *checkForString, bool caseSensitive);
const char *String_Replace(const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive);
const char *String_LowerCase(const char *thisString);
const char *String_UpperCase(const char *thisString);
int String_GetChars(const char *texx, int index);
int StringToInt(const char *stino);
int StrContains(const char *s1, const char *s2);

//=============================================================================

class SplitLines;
// Break up the text into lines restricted by the given width;
// returns number of lines, or 0 if text cannot be split well to fit in this width.
// Does additional processing, like removal of voice-over tags and text reversal if right-to-left text display is on.
// Optionally applies text direction rules (apply_direction param), otherwise leaves left-to-right always.
size_t break_up_text_into_lines(const char *todis, bool apply_direction, SplitLines &lines, int wii, int fonnt, size_t max_lines = -1);
inline size_t break_up_text_into_lines(const char *todis, SplitLines &lines, int wii, int fonnt, size_t max_lines = -1) {
	return break_up_text_into_lines(todis, true, lines, wii, fonnt, max_lines);
}
// Checks the capacity of an old-style script string buffer.
// Commonly this should return MAX_MAXSTRLEN, but there are
// cases when the buffer is a field inside one of the game structs,
// in which case this returns that field's capacity.
size_t check_scstrcapacity(const char *ptr);
// This function reports that a legacy script string was modified,
// and checks if it is an object's field in order to sync with any contemporary
// properties.
void commit_scstr_update(const char *ptr);
// Tries if the input string contains a voice-over token ("&N"),
// *optionally* fills the voice_num value (if the valid int pointer is passed),
// and returns the pointer to the text portion after the token.
// If returned pointer equals input pointer, that means that there was no token.
// voice_num must be > 0 for a valid token, it's assigned 0 if no token was found,
// or if there have been a parsing error.
const char *parse_voiceover_token(const char *text, int *voice_num);

} // namespace AGS3

#endif
