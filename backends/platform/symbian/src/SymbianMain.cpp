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


#include <bacline.h> // CCommandLineArguments

#include "backends/platform/symbian/src/portdefs.h"
#include "base/main.h"
#include "backends/platform/symbian/src/SymbianOS.h"

extern "C"
{
// Include the snprintf and vsnprintf implementations as 'C' code.
#include "vsnprintf.h"
}

// Symbian SDL_Main implementation.
// Redirects standard io, creates Symbian specific SDL backend (inherited from main SDL).
int main(int argc, char *argv[]) {

	// Set up redirects for stdout/stderr under Symbian.
	// Code copied from SDL_main.

	// Symbian does not like any output to the console through any *print* function.
	char STDOUT_FILE[256], STDERR_FILE[256]; // Shhh, don't tell anybody :)
	strcpy(STDOUT_FILE, Symbian::GetExecutablePath());
	strcpy(STDERR_FILE, Symbian::GetExecutablePath());
	strcat(STDOUT_FILE, "scummvm.stdout.txt");
	strcat(STDERR_FILE, "scummvm.stderr.txt");

	// Flush the output in case anything is queued.
	fclose(stdout);
	fclose(stderr);

	// Redirect standard input and standard output.
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if (newfp == NULL) {	// This happens on NT.
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if (newfp) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if (newfp == NULL) {	// This happens on NT.
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if (newfp) {
			*stderr = *newfp;
		}
#endif
	}
	setbuf(stderr, NULL);	// No buffering.

	// Create our OSystem instance.
	g_system = new OSystem_SDL_Symbian();
	assert(g_system);

	// Pre initialize the backend.
	g_system->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Catch input params and pass to argv/argc.
	CCommandLineArguments *cmdline = CCommandLineArguments::NewL();
	if (!cmdline) {
		error("Failure to alloc CCommandLineArguments!");
		return -1;
	}

	argc = cmdline->Count();
	if (argc > 1) {
		debug("console arg count by CCommandLineArguments: %d", argc);
		argv = new char* [argc];
		HBufC8 *buf = HBufC8::NewMax(20); // This should suffice for most cases.
		for (TInt i = 0; i < argc; ++i) {
			TPtrC arg = cmdline->Arg(i);
			argv[i] = new char[arg.Length() + 1](); // Hold zero terminated string.
			if (arg.Length() > buf->Length())
				buf->ReAlloc(arg.Length());

			TPtr arg2((TUint16 *)arg.Ptr(), arg.Length(), arg.Length());
			TPtr8 pbuf2(buf->Des());
			pbuf2 = arg2.Collapse();
			pbuf2.Copy((TUint8 *)argv[i], arg.Length());
			debug("argv[%d]: %s", i, argv[i]);
		}
		delete buf;
	}
	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Clear argv.
	if (argc > 1) { //avoid multiple User 42 panics at exit
		for (TInt i = 0; i < argc; ++i) {
			delete[] argv[i];
		}
		delete[] argv;
	}
	delete cmdline;

	// Free OSystem.
	g_system->destroy();

	return res;
}
