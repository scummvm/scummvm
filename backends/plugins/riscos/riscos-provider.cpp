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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(RISCOS)

#include "backends/plugins/riscos/riscos-provider.h"
#include "backends/plugins/elf/arm-loader.h"

#include "common/debug.h"

#include <kernel.h>
#include <swis.h>

// By declaring this variable we force libunixlib to always use dynamic areas for data allocations
// This frees up space for plugins and allows to have plenty of space for data
const char *const __dynamic_da_name = "ScummVM Heap";

// HACK: These two function are part of private API in libunixlib
// They let allocate and free data in the application space where the stack is placed below 64MB
// When using malloc with big chunks we end up in memory mapped areas above 64MB
extern "C" {
extern void *__stackalloc (size_t __size);
extern void __stackfree (void *__ptr);
}

// HACK: This is needed so that standard library functions that are only
// used in plugins can be found in the main executable.
void pluginHack() {
	volatile float f = 0.0f;
	volatile double d = 0.0;
	volatile int i = 0;

	byte *b = new (std::nothrow) byte[100];

	f = tanhf(f);
	f = logf(f);
	f = log10f(f);
	f = lroundf(f);
	f = expf(f);
	f = frexpf(f, NULL);
	f = ldexpf(f, 1);
	f = fmaxf(f, f);
	f = fminf(f, f);
	f = truncf(f);

	d = nearbyint(d);

	i = strcoll("dummyA", "dummyB");

	rename("dummyA", "dummyB");

	delete[] b;
}

class RiscOSDLObject : public ARMDLObject {
protected:
	void flushDataCache(void *ptr, uint32 len) const override {
		_kernel_swi_regs regs;

		regs.r[0] = 1;
		regs.r[1] = (int)ptr;
		regs.r[2] = (int)ptr + len;

		_kernel_swi(OS_SynchroniseCodeAreas, &regs, &regs);
	}

};

/**
 * On 26-bit RISC OS, plugins need to be allocated in the first 64 MB
 * of RAM so that it can be executed. This may not be the case when using
 * the default allocators, which use dynamic areas for large allocations.
 */
class RiscOSDLObject_AS : public RiscOSDLObject {
protected:
	void *allocateMemory(uint32 align, uint32 size) override {
		// Allocate with worst case alignment in application space
		void *p = __stackalloc(size + sizeof(uintptr) + align - 1);
		void *np = (void *)(((uintptr)p + align - 1) & (-align));

		*(uintptr *)((byte *)np + size) = (uintptr)p;

		debug(8, "Allocated %p while alignment was %d: using %p", p, align, np);

		return np;
	}

	void deallocateMemory(void *ptr, uint32 size) override {
		void *p = (void *)*(uintptr *)((byte *)ptr + size);
		debug(8, "Freeing %p which was allocated at %p", ptr, p);
		__stackfree(p);
	}
};

RiscOSPluginProvider::RiscOSPluginProvider() : _is32bit(false) {
	__asm__ volatile (
		"SUBS	%[is32bit], r0, r0\n\t" /* Set at least one status flag and set is32bits to 0 */
		"TEQ	pc, pc\n\t"				/* First operand never contains flags while second one contains them in 26-bits only */
		"MOVEQ	%[is32bit], #1\n\t"		/* Set to 1 only if EQ flag is set */
		: [is32bit] "=r" (_is32bit)
		: /* no inputs */
		: "cc");
}

Plugin *RiscOSPluginProvider::createPlugin(const Common::FSNode &node) const {
	if (_is32bit) {
		return new TemplatedELFPlugin<RiscOSDLObject>(node.getPath());
	} else {
		return new TemplatedELFPlugin<RiscOSDLObject_AS>(node.getPath());
	}
}

#endif // defined(DYNAMIC_MODULES) && defined(RISCOS)
