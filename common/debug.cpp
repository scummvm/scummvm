/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/sys.h"
#include "common/debug.h"
#include "common/str.h"

#include "engine/backend/platform/driver.h"

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "engine/backend/platform/ps2/fileio.h"

	#define fprintf				ps2_fprintf
	#define fputs(str, file)	ps2_fputs(str, file)
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "engine/backend/fs/ds/ds-fs.h"

	void	std_fprintf(FILE* handle, const char* fmt, ...);
	void	std_fflush(FILE* handle);

	#define fprintf(file, fmt, ...)				do { char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); } while(0)
	#define fputs(str, file)					DS::std_fwrite(str, strlen(str), 1, file)
	#define fflush(file)						DS::std_fflush(file)
#endif

#ifndef DISABLE_TEXT_CONSOLE

static void debugHelper(const char *s, va_list va, bool caret = true) {
	char in_buf[STRINGBUFLEN];
	char buf[STRINGBUFLEN];
	vsnprintf(in_buf, STRINGBUFLEN, s, va);

	strcpy(buf, in_buf);

	if (caret) {
		buf[STRINGBUFLEN-2] = '\0';
		strcat(buf, "\n");
	}

	fputs(buf, stdout);

#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif

	fflush(stdout);
}

void debug(const char *s, ...) {
	va_list va;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debug(int level, const char *s, ...) {
	va_list va;

	if (level > debugLevel)
		return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
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

	strcpy(buf_output, buf_input);

	// Print the error message to stderr
	fprintf(stderr, "%s!\n", buf_output);

	buf_output[STRINGBUFLEN-3] = '\0';
	buf_output[STRINGBUFLEN-2] = '\0';
	buf_output[STRINGBUFLEN-1] = '\0';
	strcat(buf_output, "!\n");


	// Print the error message to stderr
	fputs(buf_output, stderr);

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
	// Finally exit. quit() will terminate the program if g_driver is present
	if (g_driver)
		g_driver->quit();

	exit(1);
}

#ifndef DISABLE_TEXT_CONSOLE

void warning(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

#if !defined (__SYMBIAN32__)
	fprintf(stderr, "WARNING: %s!\n", buf);
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


const char *debug_levels[] = {
	"NONE",
	"NORMAL",
	"WARN",
	"ERROR",
	"FUNC",
	"BITMAP",
	"MODEL",
	"STUB",
	"SMUSH",
	"IMUSE",
	"CHORE",
	"ALL"
};

const char *debug_descriptions[] = {
	"No debug messages will be printed (default)",
	"\"Normal\" debug messages will be printed",
	"Warning debug messages will be printed",
	"Error debug messages will be printed",
	"Function (normal and stub) debug messages will be printed",
	"Bitmap debug messages will be printed",
	"Model debug messages will be printed",
	"Stub (missing function) debug messages will be printed",
	"SMUSH (video) debug messages will be printed",
	"IMUSE (audio) debug messages will be printed",
	"Chore debug messages will be printed",
	"All debug messages will be printed",
};

