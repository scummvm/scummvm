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

#include "ags/shared/core/platform.h"
//include <string.h>
#include "ags/shared/ac/game_version.h"
//#include "ags/shared/debugging/assert.h"
// File not present??
#include "common/scummsys.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/globals.h"

namespace AGS3 {

const char *ScriptVSprintf__(char *buffer, size_t buf_length, const char *format, ...) {
	va_list args;
	va_start(args, format);
	const char *res_buffer = ScriptVSprintf(buffer, buf_length, format, args);
	va_end(args);
	return res_buffer;
}

void Test_ScriptSprintf() {
	const int argi = 123;
	const float argf = 0.456F;
	const char *argcc = "string literal";
	RuntimeScriptValue params[10];
	params[0].SetInt32(argi);
	params[1].SetFloat(argf);
	params[2].SetStringLiteral(argcc);

	char ScSfBuffer[STD_BUFFER_SIZE];
	//
	// Called-from-script variant
	//
	// Correct format, extra placeholder
	const char *result =
	    ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE,
	                  "testing ScriptSprintf:\nThis is int: %10d\nThis is float: %.4f\nThis is string: '%s'\nThis placeholder will be ignored: %d",
	                  params, 3);
	assert(strcmp(result, "testing ScriptSprintf:\nThis is int:        123\nThis is float: 0.4560\nThis is string: 'string literal'\nThis placeholder will be ignored: %d") == 0);
	// Literal percent sign
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "%d%%", params, 3);
	assert(strcmp(result, "123%") == 0);
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "123%%", NULL, 0);
	assert(strcmp(result, "123%") == 0);
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "%%5d%%0.5f%%s", params, 3);
	assert(strcmp(result, "%5d%0.5f%s") == 0);

	// Invalid format
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "%zzzzzz", params, 3);
	assert(strcmp(result, "%zzzzzz") == 0);
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "%12.34%d", params, 3);
	assert(strcmp(result, "%12.34123") == 0);

	// Not enough arguments
	result = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, "%5d%0.5f%s", params, 0);
	assert(strcmp(result, "%5d%0.5f%s") == 0);

	// Not enough buffer space
	result = ScriptSprintf(ScSfBuffer, 9, "12345678%d", params, 3);
	assert(strcmp(result, "12345678") == 0);
	result = ScriptSprintf(ScSfBuffer, 11, "12345678%d", params, 3);
	assert(strcmp(result, "1234567812") == 0);
	// Not enough buffer space and not enough params
	result = ScriptSprintf(ScSfBuffer, 10, "12345678%d", params, 0);
	assert(strcmp(result, "12345678%") == 0);
	result = ScriptSprintf(ScSfBuffer, 11, "12345678%d", params, 0);
	assert(strcmp(result, "12345678%d") == 0);

	// Test null string pointer in backward-compatibility mode
	_G(loaded_game_file_version) = kGameVersion_312;
	params[0].SetStringLiteral(NULL);
	result = ScriptSprintf(ScSfBuffer, 10, "A%sB", params, 1);
	assert(strcmp(result, "A(null)B") == 0);
	_G(loaded_game_file_version) = kGameVersion_Undefined;

	//
	// Called-from-plugin variant
	// Note that since this is variadic function, number of parameters must
	// always be equal or higher than number of placeholders in buffer string.
	//
	// Correct format, matching number of arguments
	result =
	    ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE,
	                     "testing ScriptVSprintf:\nThis is int: %10d\nThis is float: %.4f\nThis is string: '%s'\n",
	                     argi, argf, argcc);
	assert(strcmp(result, "testing ScriptVSprintf:\nThis is int:        123\nThis is float: 0.4560\nThis is string: 'string literal'\n") == 0);
	// Literal percent sign
	result = ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE, "%d%%", argi);
	assert(strcmp(result, "123%") == 0);
	result = ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE, "123%%");
	assert(strcmp(result, "123%") == 0);
	result = ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE, "%%5d%%0.5f%%s");
	assert(strcmp(result, "%5d%0.5f%s") == 0);

	// Invalid format
	result = ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE, "%zzzzzz", argi, argf, argcc);
	assert(strcmp(result, "%zzzzzz") == 0);
	result = ScriptVSprintf__(ScSfBuffer, STD_BUFFER_SIZE, "%12.34%d", argi, argf, argcc);
	assert(strcmp(result, "%12.34123") == 0);

	// Not enough buffer space
	result = ScriptVSprintf__(ScSfBuffer, 9, "12345678%d", argi, argf, argcc);
	assert(strcmp(result, "12345678") == 0);
	result = ScriptVSprintf__(ScSfBuffer, 11, "12345678%d", argi, argf, argcc);
	assert(strcmp(result, "1234567812") == 0);

	// Test null string pointer in backward-compatibility mode
	_G(loaded_game_file_version) = kGameVersion_312;
	result = ScriptVSprintf__(ScSfBuffer, 10, "A%sB", NULL);
	assert(strcmp(result, "A(null)B") == 0);
	_G(loaded_game_file_version) = kGameVersion_Undefined;
}

} // namespace AGS3
