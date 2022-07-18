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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(PSP2)

#include <cxxabi.h>
#include <psp2/kernel/modulemgr.h>

#include "base/plugins.h"
#include "backends/plugins/psp2/psp2-plugin.h"

extern "C" {

int32 PLUGIN_getVersion();
int32 PLUGIN_getType();
int32 PLUGIN_getTypeVersion();
PluginObject *PLUGIN_getObject();

static PSP2FunctionPointers PSP2Functions = {
	PSP2FunctionPointers_VERSION,
	PLUGIN_getVersion,
	PLUGIN_getType,
	PLUGIN_getTypeVersion,
	PLUGIN_getObject,
};

// hacks to make libc work
//void* __dso_handle = (void*) &__dso_handle;
extern void *__dso_handle __attribute__((weak));

/* These magic symbols are provided by the linker.  */
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));
extern void (*__fini_array_start [])(void) __attribute__((weak));
extern void (*__fini_array_end [])(void) __attribute__((weak));

static void __libc_init_array(void) {
	size_t count, i;

	count = __preinit_array_end - __preinit_array_start;
	for (i = 0; i < count; i++) {
		__preinit_array_start[i]();
	}

	count = __init_array_end - __init_array_start;
	for (i = 0; i < count; i++) {
		__init_array_start[i]();
	}
}

static void __libc_fini_array(void) {
	size_t count, i;

	count = __fini_array_end - __fini_array_start;
	for (i = count; i > 0; i--) {
		__fini_array_start[i-1]();
	}
}

int module_stop(SceSize argc, const void *args) {
	if (&__dso_handle != nullptr) {
		__cxxabiv1::__cxa_finalize(&__dso_handle);
	}
	__libc_fini_array();
	return SCE_KERNEL_STOP_SUCCESS;
}

int module_exit() {
	if (&__dso_handle != nullptr) {
		__cxxabiv1::__cxa_finalize(&__dso_handle);
	}
	__libc_fini_array();
	return SCE_KERNEL_STOP_SUCCESS;
}

int _start(SceSize argc, void *args) __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, void *args) {
	PSP2FunctionPointers **arg = *(PSP2FunctionPointers ***)args;

	__libc_init_array();

	*arg = &PSP2Functions;

	return SCE_KERNEL_START_SUCCESS;
}

}
#endif // defined(DYNAMIC_MODULES) && defined(PSP2)
