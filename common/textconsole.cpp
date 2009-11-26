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
 * $URL$
 * $Id$
 */

#include "common/textconsole.h"
#include "common/system.h"

namespace Common {

static OutputFormatter s_errorOutputFormatter = 0;

void setErrorOutputFormatter(OutputFormatter f) {
	s_errorOutputFormatter = f;
}

static ErrorHandler s_errorHandler = 0;

void setErrorHandler(ErrorHandler handler) {
	s_errorHandler = handler;
}


}	// End of namespace Common


#ifndef DISABLE_TEXT_CONSOLE

void warning(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !defined (__SYMBIAN32__)
	fputs("WARNING: ", stderr);
	fputs(buf, stderr);
	fputs("!\n", stderr);
#endif

#if defined( USE_WINDBG )
	strcat(buf, "\n");
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif
}

#endif

void NORETURN error(const char *s, ...) {
	char buf_input[STRINGBUFLEN];
	char buf_output[STRINGBUFLEN];
	va_list va;

	// Generate the full error message
	va_start(va, s);
	vsnprintf(buf_input, STRINGBUFLEN, s, va);
	va_end(va);


	// Next, give the active engine (if any) a chance to augment the message
	if (Common::s_errorOutputFormatter) {
		(*Common::s_errorOutputFormatter)(buf_output, buf_input, STRINGBUFLEN);
	} else {
		strncpy(buf_output, buf_input, STRINGBUFLEN);
	}

	buf_output[STRINGBUFLEN-3] = '\0';
	buf_output[STRINGBUFLEN-2] = '\0';
	buf_output[STRINGBUFLEN-1] = '\0';
	strcat(buf_output, "!\n");


	// Print the error message to stderr
	fputs(buf_output, stderr);

	// If there is an error handler, invoke it now
	if (Common::s_errorHandler)
		(*Common::s_errorHandler)(buf_output);

	// TODO: Add a OSystem::fatalError() method and invoke it here.
	// The default implementation would just call OSystem::quit().

#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_output_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf_output, strlen(buf_output) + 1, buf_output_unicode, sizeof(buf_output_unicode));
	OutputDebugString(buf_output_unicode);
#ifndef DEBUG
	drawError(buf_output);
#else
	int cmon_break_into_the_debugger_if_you_please = *(int *)(buf_output + 1);	// bus error
	printf("%d", cmon_break_into_the_debugger_if_you_please);			// don't optimize the int out
#endif
#else
	OutputDebugString(buf_output);
#endif
#endif

#ifdef PALMOS_MODE
	extern void PalmFatalError(const char *err);
	PalmFatalError(buf_output);
#endif

#ifdef __SYMBIAN32__
	Symbian::FatalError(buf_output);
#endif
	// Finally exit. quit() will terminate the program if g_system is present
	if (g_system)
		g_system->quit();

#if defined(SAMSUNGTV)
	// FIXME
	for (;;) {}
#else
	exit(1);
#endif
}
