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

typedef void (*Cache_CleanInvalidateRangePtr)(void *start, void *end);

// This function enters in supervisor mode, call the ARMop and leaves the supervisor mode
static void call_Cache_CleanInvalidateRange(void *start, void *end, Cache_CleanInvalidateRangePtr f) __asm__(".call_Cache_CleanInvalidateRange");
__asm__(
	".call_Cache_CleanInvalidateRange:\n"
	"PUSH	{r4, lr}\n" // Backup r4 and lr
	"MRS	r4, cpsr\n" // Backup CPSR in R4
	"SWI    0x16\n" // OS_EnterOS
	"MOV	lr, pc\n" //
	"MOV	pc, r2\n" // Call 3rd argument (function pointer)
	"MSR	cpsr_c, r4\n" // Restore CPSR (leave SVC mode)
	"POP	{r4, pc}\n" // Restore R4 and return
);


class RiscOSDLObject : public ARMDLObject {
protected:
	void flushDataCache(void *ptr, uint32 len) const override {
		Cache_CleanInvalidateRangePtr Cache_CleanInvalidateRange;

		if (!_swix(OS_MMUControl, _IN(0)|_OUT(0), 2 | 21 << 8, &Cache_CleanInvalidateRange)) {
			call_Cache_CleanInvalidateRange(ptr, (char *)ptr + len, Cache_CleanInvalidateRange);
			return;
		}

		// OS_MMUControl 2 or Cache_CleanInvalidateRange are not supported: fallback to old inefficient OS_MMUControl 1
		_swix(OS_MMUControl, _IN(0), 1 | 1 << 28 | 1 << 30 | 1 << 31);
	}

};

Plugin *RiscOSPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<RiscOSDLObject>(node.getPath());
}

#endif // defined(DYNAMIC_MODULES) && defined(RISCOS)
