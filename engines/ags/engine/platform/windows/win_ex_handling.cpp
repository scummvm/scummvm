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

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS
#include <new.h>
#include <allegro.h>
#include <winalleg.h>
#include "ac/common.h"
#include "ac/common_defines.h"
#include "debug/debugger.h"
#include "debug/out.h"
#include "main/main.h"
#include "util/ini_util.h"

#if !AGS_PLATFORM_DEBUG
#define USE_CUSTOM_EXCEPTION_HANDLER
#endif

using namespace AGS::Shared;

extern int our_eip;
extern int eip_guinum;
extern int eip_guiobj;
extern int proper_exit;

char tempmsg[100];
char *printfworkingspace;

#ifdef USE_CUSTOM_EXCEPTION_HANDLER
void CreateMiniDump(EXCEPTION_POINTERS *pep);

extern int CustomExceptionHandler(LPEXCEPTION_POINTERS exinfo);
extern EXCEPTION_RECORD excinfo;
extern int miniDumpResultCode;

static void DisplayException() {
	String script_callstack = get_cur_script(5);
	sprintf(printfworkingspace, "An exception 0x%X occurred in ACWIN.EXE at EIP = 0x%08X; program pointer is %+d, ACI version %s, gtags (%d,%d)\n\n"
	        "AGS cannot continue, this exception was fatal. Please note down the numbers above, remember what you were doing at the time and post the details on the AGS Technical Forum.\n\n%s\n\n"
	        "Most versions of Windows allow you to press Ctrl+C now to copy this entire message to the clipboard for easy reporting.\n\n%s (code %d)",
	        excinfo.ExceptionCode, (intptr_t)excinfo.ExceptionAddress, our_eip, EngineVersion.LongString.GetCStr(), eip_guinum, eip_guiobj, script_callstack.GetCStr(),
	        (miniDumpResultCode == 0) ? "An error file CrashInfo.dmp has been created. You may be asked to upload this file when reporting this problem on the AGS Forums." :
	        "Unable to create an error dump file.", miniDumpResultCode);
	MessageBoxA(win_get_window(), printfworkingspace, "Illegal exception", MB_ICONSTOP | MB_OK);
}

int initialize_engine_with_exception_handling(
    int (initialize_engine)(const AGS::Shared::ConfigTree &startup_opts),
    const ConfigTree &startup_opts) {
	__try {
		Debug::Printf(kDbgMsg_Info, "Installing exception handler");
		return initialize_engine(startup_opts);
	} __except (CustomExceptionHandler(GetExceptionInformation())) {
		DisplayException();
		proper_exit = 1;
	}
	return EXIT_CRASH;
}
#endif // USE_CUSTOM_EXCEPTION_HANDLER


int malloc_fail_handler(size_t amountwanted) {
#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	CreateMiniDump(NULL);
#endif
	free(printfworkingspace);
	sprintf(tempmsg, "Out of memory: failed to allocate %ld bytes (at PP=%d)", amountwanted, our_eip);
	quit(tempmsg);
	return 0;
}

void setup_malloc_handling() {
	_set_new_handler(malloc_fail_handler);
	_set_new_mode(1);
	printfworkingspace = (char *)malloc(7000);
}

#endif // AGS_PLATFORM_OS_WINDOWS
