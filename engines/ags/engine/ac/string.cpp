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

#include "common/std/algorithm.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/util/utf8.h"
#include "ags/engine/ac/display.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/math.h"
#include "ags/globals.h"

namespace AGS3 {

int String_IsNullOrEmpty(const char *thisString) {
	if ((thisString == nullptr) || (thisString[0] == 0))
		return 1;

	return 0;
}

const char *String_Copy(const char *srcString) {
	return CreateNewScriptString(srcString);
}

const char *String_Append(const char *thisString, const char *extrabit) {
	size_t ln = strlen(thisString) + strlen(extrabit) + 1;
	char *buffer = (char *)malloc(ln);
	Common::strcpy_s(buffer, ln, thisString);
	Common::strcat_s(buffer, ln, extrabit);
	return CreateNewScriptString(buffer, false);
}

const char *String_AppendChar(const char *thisString, int extraOne) {
	char chr[5]{};
	size_t chw = usetc(chr, extraOne);
	size_t ln = strlen(thisString) + chw + 1;
	char *buffer = (char *)malloc(ln);
	Common::sprintf_s(buffer, ln, "%s%s", thisString, chr);
	return CreateNewScriptString(buffer, false);
}

const char *String_ReplaceCharAt(const char *thisString, int index, int newChar) {
	size_t len = ustrlen(thisString);
	if ((index < 0) || ((size_t)index >= len))
		quit("!String.ReplaceCharAt: index outside range of string");

	size_t off = uoffset(thisString, index);
	int uchar = ugetc(thisString + off);
	size_t remain_sz = strlen(thisString + off);
	size_t old_sz = ucwidth(uchar);
	char new_chr[5]{};
	size_t new_chw = usetc(new_chr, newChar);
	size_t total_sz = off + remain_sz + new_chw - old_sz + 1;
	char *buffer = (char *)malloc(total_sz);
	memcpy(buffer, thisString, off);
	memcpy(buffer + off, new_chr, new_chw);
	memcpy(buffer + off + new_chw, thisString + off + old_sz, remain_sz - old_sz + 1);
	return CreateNewScriptString(buffer, false);
}

const char *String_Truncate(const char *thisString, int length) {
	if (length < 0)
		quit("!String.Truncate: invalid length");
	size_t strlen = ustrlen(thisString);
	if ((size_t)length >= strlen)
		return thisString;

	size_t sz = uoffset(thisString, length);
	char *buffer = (char *)malloc(sz + 1);
	memcpy(buffer, thisString, sz);
	buffer[sz] = 0;
	return CreateNewScriptString(buffer, false);
}

const char *String_Substring(const char *thisString, int index, int length) {
	if (length < 0)
		quit("!String.Substring: invalid length");
	size_t strlen = ustrlen(thisString);
	if ((index < 0) || ((size_t)index > strlen))
		quit("!String.Substring: invalid index");
	size_t sublen = MIN((size_t)length, strlen - index);
	size_t start = uoffset(thisString, index);
	size_t end = uoffset(thisString + start, sublen) + start;
	size_t copysz = end - start;

	char *buffer = (char *)malloc(copysz + 1);
	memcpy(buffer, thisString + start, copysz);
	buffer[copysz] = 0;
	return CreateNewScriptString(buffer, false);
}

int String_CompareTo(const char *thisString, const char *otherString, bool caseSensitive) {

	if (caseSensitive) {
		return strcmp(thisString, otherString);
	} else {
		return ustricmp(thisString, otherString);
	}
}

int String_StartsWith(const char *thisString, const char *checkForString, bool caseSensitive) {

	if (caseSensitive) {
		return (strncmp(thisString, checkForString, strlen(checkForString)) == 0) ? 1 : 0;
	} else {
		return (ustrnicmp(thisString, checkForString, ustrlen(checkForString)) == 0) ? 1 : 0;
	}
}

int String_EndsWith(const char *thisString, const char *checkForString, bool caseSensitive) {
	// NOTE: we need size in bytes here
	size_t thislen = strlen(thisString), checklen = strlen(checkForString);
	if (checklen > thislen)
		return 0;

	if (caseSensitive) {
		return (strcmp(thisString + (thislen - checklen), checkForString) == 0) ? 1 : 0;
	} else {
		return (ustricmp(thisString + (thislen - checklen), checkForString) == 0) ? 1 : 0;
	}
}

const char *String_Replace(const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive) {
	char resultBuffer[STD_BUFFER_SIZE] = "";
	size_t outputSize = 0; // length in bytes
	if (caseSensitive) {
		size_t lookForLen = strlen(lookForText);
		size_t replaceLen = strlen(replaceWithText);
		for (const char *ptr = thisString; *ptr; ++ptr) {
			if (strncmp(ptr, lookForText, lookForLen) == 0) {
				memcpy(&resultBuffer[outputSize], replaceWithText, replaceLen);
				outputSize += replaceLen;
				ptr += lookForLen - 1;
			} else {
				resultBuffer[outputSize] = *ptr;
				outputSize++;
			}
		}
	} else {
		size_t lookForLen = ustrlen(lookForText);
		size_t lookForSz = strlen(lookForText); // length in bytes
		size_t replaceSz = strlen(replaceWithText); // length in bytes
		const char *p_cur = thisString;
		for (int c = ugetxc(&thisString); *p_cur; p_cur = thisString, c = ugetxc(&thisString)) {
			if (ustrnicmp(p_cur, lookForText, lookForLen) == 0) {
				memcpy(&resultBuffer[outputSize], replaceWithText, replaceSz);
				outputSize += replaceSz;
				thisString = p_cur + lookForSz;
			} else {
				usetc(&resultBuffer[outputSize], c);
				outputSize += ucwidth(c);
			}
		}
	}

	resultBuffer[outputSize] = 0; // terminate
	return CreateNewScriptString(resultBuffer, true);
}

const char *String_LowerCase(const char *thisString) {
	char *buffer = ags_strdup(thisString);
	ustrlwr(buffer);
	return CreateNewScriptString(buffer, false);
}

const char *String_UpperCase(const char *thisString) {
	char *buffer = ags_strdup(thisString);
	ustrupr(buffer);
	return CreateNewScriptString(buffer, false);
}

int String_GetChars(const char *texx, int index) {
	if (get_uformat() == U_UTF8) {
		if ((index < 0) || (index >= ustrlen(texx)))
			return 0;
		return ugetat(texx, index);
	} else {
		if ((index < 0) || (index >= (int)strlen(texx)))
			return 0;
		return texx[index];
	}
}

int String_GetLength(const char *texx) {
	return (get_uformat() == U_UTF8) ? ustrlen(texx) : strlen(texx);
}

int StringToInt(const char *stino) {
	return atoi(stino);
}

int StrContains(const char *s1, const char *s2) {
	VALIDATE_STRING(s1);
	VALIDATE_STRING(s2);
	char *tempbuf1 = ags_strdup(s1);
	char *tempbuf2 = ags_strdup(s2);
	ustrlwr(tempbuf1);
	ustrlwr(tempbuf2);

	char *offs = const_cast<char *>(ustrstr(tempbuf1, tempbuf2));

	if (offs == nullptr) {
		free(tempbuf1);
		free(tempbuf2);
		return -1;
	}

	*offs = 0;
	int at = ustrlen(tempbuf1);
	free(tempbuf1);
	free(tempbuf2);
	return at;
}

//=============================================================================

const char *CreateNewScriptString(const String &fromText) {
	return (const char *)CreateNewScriptStringObj(fromText.GetCStr(), true).second;
}

const char *CreateNewScriptString(const char *fromText, bool reAllocate) {
	return (const char *)CreateNewScriptStringObj(fromText, reAllocate).second;
}

DynObjectRef CreateNewScriptStringObj(const String &fromText) {
	return CreateNewScriptStringObj(fromText.GetCStr(), true);
}

DynObjectRef CreateNewScriptStringObj(const char *fromText, bool reAllocate) {
	ScriptString *str;
	if (reAllocate) {
		str = new ScriptString(fromText);
	} else { // TODO: refactor to avoid const casts!
		str = new ScriptString(const_cast<char *>(fromText), true);
	}
	void *obj_ptr = str->GetTextPtr();
	int32_t handle = ccRegisterManagedObject(obj_ptr, str);
	if (handle == 0) {
		delete str;
		return DynObjectRef(0, nullptr);
	}
	return DynObjectRef(handle, obj_ptr);
}

size_t break_up_text_into_lines(const char *todis, bool apply_direction, SplitLines &lines, int wii, int fonnt, size_t max_lines) {
	if (fonnt == -1)
		fonnt = _GP(play).normal_font;

	//  char sofar[100];
	if (todis[0] == '&') {
		while ((todis[0] != ' ') & (todis[0] != 0)) todis++;
		if (todis[0] == ' ') todis++;
	}
	lines.Reset();
	_G(longestline) = 0;

	// Don't attempt to display anything if the width is tiny
	if (wii < 3)
		return 0;

	int line_length;

	split_lines(todis, lines, wii, fonnt, max_lines);

	// Right-to-left just means reverse the text then
	// write it as normal
	if (apply_direction && (_GP(game).options[OPT_RIGHTLEFTWRITE] != 0))
		for (size_t rr = 0; rr < lines.Count(); rr++) {
			(get_uformat() == U_UTF8) ?
				lines[rr].ReverseUTF8() :
				lines[rr].Reverse();
			line_length = get_text_width_outlined(lines[rr].GetCStr(), fonnt);
			if (line_length > _G(longestline))
				_G(longestline) = line_length;
		} else
			for (size_t rr = 0; rr < lines.Count(); rr++) {
				line_length = get_text_width_outlined(lines[rr].GetCStr(), fonnt);
				if (line_length > _G(longestline))
					_G(longestline) = line_length;
			}
		return lines.Count();
}

void check_strlen(char *ptt) {
	_G(MAXSTRLEN) = MAX_MAXSTRLEN;
	long charstart = (intptr_t)&_GP(game).chars[0];
	long charend = charstart + sizeof(CharacterInfo) * _GP(game).numcharacters;
	if (((intptr_t)&ptt[0] >= charstart) && ((intptr_t)&ptt[0] <= charend))
		_G(MAXSTRLEN) = 30;
}

/*void GetLanguageString(int indxx,char*buffr) {
VALIDATE_STRING(buffr);
char*bptr=get_language_text(indxx);
if (bptr==NULL) Common::strcpy_s(buffr, 200, "[language string error]");
else strncpy(buffr,bptr,199);
buffr[199]=0;
}*/

void my_strncpy(char *dest, const char *src, int len) {
	// the normal strncpy pads out the string with zeros up to the
	// max length -- we don't want that
	Common::strcpy_s(dest, len + 1, src);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int (const char *thisString)
RuntimeScriptValue Sc_String_IsNullOrEmpty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(String_IsNullOrEmpty, const char);
}

// const char* (const char *thisString, const char *extrabit)
RuntimeScriptValue Sc_String_Append(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_POBJ(const char, const char, _GP(myScriptStringImpl), String_Append, const char);
}

// const char* (const char *thisString, char extraOne)
RuntimeScriptValue Sc_String_AppendChar(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_PINT(const char, const char, _GP(myScriptStringImpl), String_AppendChar);
}

// int (const char *thisString, const char *otherString, bool caseSensitive)
RuntimeScriptValue Sc_String_CompareTo(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ_PBOOL(const char, String_CompareTo, const char);
}

// int  (const char *s1, const char *s2)
RuntimeScriptValue Sc_StrContains(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ(const char, StrContains, const char);
}

// const char* (const char *srcString)
RuntimeScriptValue Sc_String_Copy(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(const char, const char, _GP(myScriptStringImpl), String_Copy);
}

// int (const char *thisString, const char *checkForString, bool caseSensitive)
RuntimeScriptValue Sc_String_EndsWith(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ_PBOOL(const char, String_EndsWith, const char);
}

// const char* (const char *texx, ...)
RuntimeScriptValue Sc_String_Format(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(String_Format, 1);
	return RuntimeScriptValue().SetDynamicObject(const_cast<char *>(CreateNewScriptString(scsf_buffer)), &_GP(myScriptStringImpl));
}

// const char* (const char *thisString)
RuntimeScriptValue Sc_String_LowerCase(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(const char, const char, _GP(myScriptStringImpl), String_LowerCase);
}

// const char* (const char *thisString, const char *lookForText, const char *replaceWithText, bool caseSensitive)
RuntimeScriptValue Sc_String_Replace(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_POBJ2_PBOOL(const char, const char, _GP(myScriptStringImpl), String_Replace, const char, const char);
}

// const char* (const char *thisString, int index, char newChar)
RuntimeScriptValue Sc_String_ReplaceCharAt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_PINT2(const char, const char, _GP(myScriptStringImpl), String_ReplaceCharAt);
}

// int (const char *thisString, const char *checkForString, bool caseSensitive)
RuntimeScriptValue Sc_String_StartsWith(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_POBJ_PBOOL(const char, String_StartsWith, const char);
}

// const char* (const char *thisString, int index, int length)
RuntimeScriptValue Sc_String_Substring(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_PINT2(const char, const char, _GP(myScriptStringImpl), String_Substring);
}

// const char* (const char *thisString, int length)
RuntimeScriptValue Sc_String_Truncate(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ_PINT(const char, const char, _GP(myScriptStringImpl), String_Truncate);
}

// const char* (const char *thisString)
RuntimeScriptValue Sc_String_UpperCase(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(const char, const char, _GP(myScriptStringImpl), String_UpperCase);
}

// FLOAT_RETURN_TYPE (const char *theString);
RuntimeScriptValue Sc_StringToFloat(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_FLOAT(const char, StringToFloat);
}

// int (char*stino)
RuntimeScriptValue Sc_StringToInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(const char, StringToInt);
}

// int (const char *texx, int index)
RuntimeScriptValue Sc_String_GetChars(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(const char, String_GetChars);
}

RuntimeScriptValue Sc_String_GetLength(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(const char, String_GetLength);
}

//=============================================================================
//
// Exclusive variadic API implementation for Plugins
//
//=============================================================================

void RegisterStringAPI() {
	ScFnRegister string_api[] = {
		{"String::IsNullOrEmpty^1", API_FN_PAIR(String_IsNullOrEmpty)},
		{"String::Format^101", Sc_String_Format},

		{"String::Append^1", API_FN_PAIR(String_Append)},
		{"String::AppendChar^1", API_FN_PAIR(String_AppendChar)},
		{"String::CompareTo^2", API_FN_PAIR(String_CompareTo)},
		{"String::Contains^1", API_FN_PAIR(StrContains)},
		{"String::Copy^0", API_FN_PAIR(String_Copy)},
		{"String::EndsWith^2", API_FN_PAIR(String_EndsWith)},
		{"String::IndexOf^1", API_FN_PAIR(StrContains)},
		{"String::LowerCase^0", API_FN_PAIR(String_LowerCase)},
		{"String::Replace^3", API_FN_PAIR(String_Replace)},
		{"String::ReplaceCharAt^2", API_FN_PAIR(String_ReplaceCharAt)},
		{"String::StartsWith^2", API_FN_PAIR(String_StartsWith)},
		{"String::Substring^2", API_FN_PAIR(String_Substring)},
		{"String::Truncate^1", API_FN_PAIR(String_Truncate)},
		{"String::UpperCase^0", API_FN_PAIR(String_UpperCase)},
		{"String::get_AsFloat", API_FN_PAIR(StringToFloat)},
		{"String::get_AsInt", API_FN_PAIR(StringToInt)},
		{"String::geti_Chars", API_FN_PAIR(String_GetChars)},
		{"String::get_Length", API_FN_PAIR(String_GetLength)},
	};

	ccAddExternalFunctions361(string_api);
}

} // namespace AGS3
