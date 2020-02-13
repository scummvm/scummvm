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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Ultima {
namespace Nuvie {

#ifndef WITHOUT_DEBUG

// maybe make these configurable at runtime instead?
#define WITHOUT_DEBUG_TIMESTAMP_IN_HEADER
//#define WITHOUT_DEBUG_LEVEL_IN_HEADER
//#define WITHOUT_DEBUG_FUNC_IN_HEADER
//#define WITHOUT_DEBUG_FILE_LINE_IN_HEADER
//#define WITHOUT_DEBUG_NEWLINE_IN_HEADER

DebugLevelType debug(const char *func, const char *file, const int line, const bool no_header, const DebugLevelType level, const char *format, ...) {
// original
//  static const char* DebugLevelNames[]= { "EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTIFICATION", "INFORMATIONAL", "DEBUGGING" };
//  shorter, because spammy enough as is.
//  static const char* DebugLevelNames[]= { "EMERG", "ALERT", "CRIT.", "ERROR", "WARN.", "NOTE.", "INFO.", "DEBUG" };
//  shorter, because spammy enough as is.
	static const char *DebugLevelNames[] = { "!", "A", "C", "E", "W", "N", "I", "D" };
	static DebugLevelType CurrentDebugLevel = LEVEL_DEBUGGING;

	if (format == NULL) {
		CurrentDebugLevel = level;
		return CurrentDebugLevel;
	}
	if (!strcmp(format, "!!increase!!\n")) {
		unsigned char c = (unsigned char) CurrentDebugLevel;
		if (c < 7) {
			c++;
		}
		CurrentDebugLevel = (DebugLevelType) c;
	}
	if (!strcmp(format, "!!decrease!!\n")) {
		unsigned char c = (unsigned char) CurrentDebugLevel;
		if (c > 0) {
			c--;
		}
		CurrentDebugLevel = (DebugLevelType) c;
	}
	if (level > CurrentDebugLevel) {
		return CurrentDebugLevel;    // Don't call ourselves here to log something like 'message suppressed'
	}
	if (!no_header) {
#ifndef WITHOUT_DEBUG_LEVEL_IN_HEADER
		::debugN("[%s] ", DebugLevelNames[(unsigned char)level]);
#endif
#ifndef WITHOUT_DEBUG_FUNC_IN_HEADER
		::debugN("%s ", func);
#endif
#ifndef WITHOUT_DEBUG_FILE_LINE_IN_HEADER
		::debugN("%s:%d", file, line);
#endif
		::debugN("> ");
#ifndef WITHOUT_DEBUG_NEWLINE_IN_HEADER
		::debugN("\n");
#endif
	}
	
	va_list ap;
	va_start(ap, format);
	Common::String buf = Common::String::vformat(format, ap);
	::debugN("%s", buf.c_str());
	va_end(ap);

	return CurrentDebugLevel;
}

#endif /* WITHOUT_DEBUG */

/* test code / documentation.
int main(char ** argv,int argc)
{
  DEBUG(0,LEVEL_EMERGENCY,NULL); // to set the debug cut-off rather high
  DEBUG(0,LEVEL_EMERGENCY,"%d %c %s\n",1,'a',"aarrgghh..");
  DEBUG(1,LEVEL_EMERGENCY,"continuation of aarrgghh..");
  DEBUG(0,LEVEL_ALERT,"%d %c %s\n",1,'a',"RED"); // should be suppressed
  DEBUG(0,LEVEL_DEBUGGING,NULL); // to allow all messages through.
  DEBUG(0,LEVEL_DEBUGGING,"%d %c %s\n",1,'a',"debugging");
  return 1;
  }
*/

} // End of namespace Nuvie
} // End of namespace Ultima
