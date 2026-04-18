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

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(ELF_LOADER_CXA_ATEXIT) && defined(__MINT__)

#include "common/debug.h"
#include "common/textconsole.h"

#include <stdlib.h>

/**
 * Minimal __cxa_atexit / __cxa_finalize for targets whose libc lacks them
 * (e.g. mintlib on Atari).
 *
 * With -fuse-cxa-atexit, GCC registers function-local static destructors via
 * __cxa_atexit(&dtor, &obj, &__dso_handle).  The third argument tags which
 * DSO owns the registration.  On plugin unload, ELFPlugin::unloadPlugin()
 * calls __cxa_finalize(&plugin_dso_handle) to run and remove only that
 * plugin's entries -- preventing dangling atexit pointers into freed memory.
 *
 * See also the detailed comment in elf-provider.cpp.
 */

#define CXA_ATEXIT_MAX 256

struct CxaAtexitEntry {
	void (*destructor)(void *);
	void *arg;
	void *dso_handle;
};

static CxaAtexitEntry s_entries[CXA_ATEXIT_MAX];
static int s_count;

extern "C" {

// GCC with -fuse-cxa-atexit emits references to __dso_handle in every TU that
// has function-local statics with non-trivial destructors. It is normally
// provided by crtbegin.o; on targets whose startup files don't supply one,
// define it here for the main executable. Its address uniquely identifies
// the main DSO.
void *__dso_handle = &__dso_handle;

int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso_handle) {
	if (s_count >= CXA_ATEXIT_MAX) {
		warning("elfloader: cxa-atexit table full (max=%d), dropping dtor %p dso=%p",
			CXA_ATEXIT_MAX, (void *)destructor, dso_handle);
		return -1;
	}

	s_entries[s_count].destructor = destructor;
	s_entries[s_count].arg = arg;
	s_entries[s_count].dso_handle = dso_handle;
	s_count++;

	debug(2, "elfloader: cxa-atexit register[%d] dtor=%p arg=%p dso=%p",
		s_count - 1, (void *)destructor, arg, dso_handle);
	return 0;
}

void __cxa_finalize(void *dso_handle) {
	int ran = 0, matched = 0;

	for (int i = s_count - 1; i >= 0; i--) {
		if (s_entries[i].destructor == NULL)
			continue;

		if (dso_handle == NULL || s_entries[i].dso_handle == dso_handle) {
			matched++;
			void (*f)(void *) = s_entries[i].destructor;
			void *a = s_entries[i].arg;
			s_entries[i].destructor = NULL;  // mark before calling (re-entrancy)
			f(a);
			ran++;
		}
	}

	debug(2, "elfloader: cxa-atexit finalize(dso=%p): matched=%d ran=%d (total registered=%d)",
		dso_handle, matched, ran, s_count);
}

} // extern "C"

// Ensure the main binary's own statics are finalized at process exit.
// Registered early via constructor so it runs late in the atexit sequence.
static void cxa_finalize_all() {
	__cxa_finalize(NULL);
}

static void register_cxa_cleanup() __attribute__((constructor, used));
static void register_cxa_cleanup() {
	// debug() / stdio aren't ready at constructor time, so no log here.
	atexit(cxa_finalize_all);
}

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(ELF_LOADER_CXA_ATEXIT) && defined(__MINT__)
