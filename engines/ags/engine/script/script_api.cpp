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

#include "ags/shared/ac/game_version.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/script/script_api.h"
#include "ags/shared/util/math.h"
#include "ags/shared/util/utf8.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

enum FormatParseResult {
	kFormatParseNone,
	kFormatParseInvalid,
	kFormatParseLiteralPercent,
	kFormatParseArgInteger,
	kFormatParseArgFloat,
	kFormatParseArgCharacter,
	kFormatParseArgString,
	kFormatParseArgPointer,

	kFormatParseArgFirst = kFormatParseArgInteger,
	kFormatParseArgLast = kFormatParseArgPointer
};

// Helper functions for getting parameter value either from script val array or va_list
inline int GetArgInt(const RuntimeScriptValue *sc_args, va_list *varg_ptr, int arg_idx) {
	if (varg_ptr)
		return va_arg(*varg_ptr, int);
	else
		return sc_args[arg_idx].IValue;
}

inline float GetArgFloat(const RuntimeScriptValue *sc_args, va_list *varg_ptr, int arg_idx) {
	// note that script variables store only floats, but va_list has floats promoted to double
	if (varg_ptr)
		return (float)va_arg(*varg_ptr, double);
	else
		return sc_args[arg_idx].FValue;
}

inline const char *GetArgPtr(const RuntimeScriptValue *sc_args, va_list *varg_ptr, int arg_idx) {
	if (varg_ptr)
		return va_arg(*varg_ptr, const char *);
	else
		return reinterpret_cast<const char *>(sc_args[arg_idx].Ptr);
}


// TODO: this implementation can be further optimised by either not calling
// snprintf but formatting values ourselves, or by using some library method
// that supports customizing, such as getting arguments in a custom way.
const char *ScriptSprintf(char *buffer, size_t buf_length, const char *format,
                          const RuntimeScriptValue *sc_args, int32_t sc_argc, va_list *varg_ptr) {
	if (!buffer || buf_length == 0) {
		cc_error("Internal error in ScriptSprintf: buffer is null");
		return "";
	}
	if (!format) {// NOTE: interpreter (usually) catches null-pointer sent as format at some stage earlier
		cc_error("Internal error in ScriptSprintf: format string is null");
		return "";
	}
	if (!varg_ptr && sc_argc > 0 && !sc_args) {
		cc_error("Internal error in ScriptSprintf: args pointer is null");
		return "";
	}

	// Expected format character count:
	// percent sign:    1
	// flag:            1
	// field width      10 (an uint32 number)
	// precision sign   1
	// precision        10 (an uint32 number)
	// length modifier  2
	// type             1
	// NOTE: although width and precision will
	// not likely be defined by a 10-digit
	// number, such case is theoretically valid.
	const size_t fmtbuf_size = 27;
	char       fmtbuf[fmtbuf_size];
	char *fmt_bufptr;
	char *fmt_bufendptr = &fmtbuf[fmtbuf_size - 1];

	char *out_ptr = buffer;
	// save 1 character for null terminator
	const char *out_endptr = buffer + buf_length - 1;
	const char *fmt_ptr = format;
	int32_t    arg_idx = 0;

	ptrdiff_t  avail_outbuf;
	int        snprintf_res;
	FormatParseResult fmt_done;

	// Parse the format string, looking for argument placeholders
	while (*fmt_ptr && out_ptr != out_endptr) {
		// Try to put argument into placeholder
		if (*fmt_ptr == '%') {
			avail_outbuf = out_endptr - out_ptr;
			fmt_bufptr = fmtbuf;
			*(fmt_bufptr++) = '%';
			snprintf_res = 0;
			fmt_done = kFormatParseNone;

			// Parse placeholder
			while (*(++fmt_ptr) && fmt_done == kFormatParseNone && fmt_bufptr != fmt_bufendptr) {
				*(fmt_bufptr++) = *fmt_ptr;
				switch (*fmt_ptr) {
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					fmt_done = kFormatParseArgInteger;
					break;
				case 'c':
					fmt_done = kFormatParseArgCharacter;
					break;
				case 'e':
				case 'E':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
				case 'a':
				case 'A':
					fmt_done = kFormatParseArgFloat;
					break;
				case 'p':
					fmt_done = kFormatParseArgPointer;
					break;
				case 's':
					fmt_done = kFormatParseArgString;
					break;
				case '%':
					// This may be a literal percent sign ('%%')
					if (fmt_bufptr - fmtbuf == 2) {
						fmt_done = kFormatParseLiteralPercent;
					}
					// ...Otherwise we reached the next placeholder
					else {
						fmt_ptr--;
						fmt_bufptr--;
						fmt_done = kFormatParseInvalid;
					}
					break;
				}
			}

			// Deal with the placeholder parsing results
			if (fmt_done == kFormatParseLiteralPercent) {
				// literal percent sign
				*(out_ptr++) = '%';
				continue;
			} else if (fmt_done >= kFormatParseArgFirst && fmt_done <= kFormatParseArgLast &&
			           (varg_ptr || arg_idx < sc_argc)) {
				// Print the actual value
				// NOTE: snprintf is called with avail_outbuf + 1 here, because we let it use our reserved
				// character for null-terminator, in case we are at the end of the buffer
				*fmt_bufptr = 0; // terminate the format buffer, we are going to use it
				switch (fmt_done) {
				case kFormatParseArgInteger:
					snprintf_res = snprintf(out_ptr, avail_outbuf + 1, fmtbuf, GetArgInt(sc_args, varg_ptr, arg_idx)); break;
				case kFormatParseArgFloat:
					snprintf_res = snprintf(out_ptr, avail_outbuf + 1, fmtbuf, GetArgFloat(sc_args, varg_ptr, arg_idx)); break;
				case kFormatParseArgCharacter:
				{
					int chr = GetArgInt(sc_args, varg_ptr, arg_idx);
					char cbuf[5]{};
					usetc(cbuf, chr);
					snprintf_res = snprintf(out_ptr, avail_outbuf + 1, "%s", cbuf);
					break;
				}
				case kFormatParseArgString:
				{
					const char *p = GetArgPtr(sc_args, varg_ptr, arg_idx);
					// Do extra checks for %s placeholder
					if (fmt_done == kFormatParseArgString && !p) {
						if (_G(loaded_game_file_version) < kGameVersion_320) {
							// explicitly put "(null)" into the placeholder
							p = "(null)";
						} else {
							cc_error("!ScriptSprintf: formatting argument %d is expected to be a string, but it is a null pointer", arg_idx + 1);
							return "";
						}
					} else if (fmt_done == kFormatParseArgString && p == buffer) {
						cc_error("!ScriptSprintf: formatting argument %d is a pointer to output buffer", arg_idx + 1);
						return "";
					}
					snprintf_res = snprintf(out_ptr, avail_outbuf + 1, fmtbuf, p);
					break;
				}
				case kFormatParseArgPointer:
					snprintf_res = snprintf(out_ptr, avail_outbuf + 1, fmtbuf, GetArgPtr(sc_args, varg_ptr, arg_idx)); break;
				default: /* should not happen */ break;
				}

				arg_idx++;
				if (snprintf_res >= 0) {
					// snprintf returns maximal number of characters, so limit it with buffer size
					out_ptr += MIN<ptrdiff_t>(snprintf_res, avail_outbuf);
					continue;
				}
				// -- pass further to invalid format case
			}

			// If format was not valid, or there are no available
			// parameters, just copy stored format buffer as it is
			size_t copy_len = MIN(MIN<ptrdiff_t>(fmt_bufptr - fmtbuf, fmtbuf_size - 1), avail_outbuf);
			memcpy(out_ptr, fmtbuf, copy_len);
			out_ptr += copy_len;
		}
		// If there's no placeholder, simply copy the character to output buffer
		else {
			*(out_ptr++) = *(fmt_ptr++);
		}
	}

	// Terminate the string
	*out_ptr = 0;
	return buffer;
}

} // namespace AGS3
