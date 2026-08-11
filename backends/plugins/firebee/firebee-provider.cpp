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

#if defined(DYNAMIC_MODULES) && defined(__MINT__) && defined(__ELF__) && defined(__mcoldfire__)

#include "backends/plugins/firebee/firebee-provider.h"
#include "backends/plugins/elf/m68k-loader.h"

#include <mint/basepage.h>
#include <mint/cookie.h>
#include <mint/mintbind.h>
#include <mint/ssystem.h>

class FireBeeDLObject final : public M68KDLObject {
protected:
	void relocateSymbols(ptrdiff_t offset) override {
		// Symbols imported via `ld --just-symbols=$(EXECUTABLE)` (used by the
		// plugin link step to resolve references into the main binary) end up as
		// SHN_ABS with st_value = ELF VMA in the main binary. The atariprg loader
		// loads the main binary at _base->p_tbase, so the runtime address is
		// (ELF VMA + p_tbase). Patch SHN_ABS entries here; the base implementation
		// then handles plugin-local symbols.
		const uint32 mainOffset = (uint32)_base->p_tbase;
		Elf32_Sym *s = _symtab;
		for (uint32 c = _symbol_cnt; c--; s++) {
			if (s->st_shndx == SHN_ABS)
				s->st_value += mainOffset;
		}

		DLObject::relocateSymbols(offset);
	}

	void flushDataCache(void *ptr, uint32 len) const override {
		if (Ssystem(-1, 0L, 0L) == 0) {
			Ssystem(S_FLUSHCACHE, (long)ptr, (long)len);
		} else {		
			// ignored for now
			(void)ptr;
			(void)len;

			long oldssp = Super(SUP_SET);
			__asm__ volatile (
				"nop\n\t"	// flush store buffer
				"moveq.l #0,%%d0\n\t"	// way = 0
				"moveq.l #0,%%d1\n\t"	// set = 0
				"move.l  %%d0,%%a0\n"
				"1:\n\t"
				"cpushl  %%bc,(%%a0)\n\t"	// push + invalidate line
				"add.l   #16,%%a0\n\t"
				"addq.l  #1,%%d1\n\t"
				"cmpi.l  #512,%%d1\n\t"
				"bne.s   1b\n\t"
				"moveq.l #0,%%d1\n\t"
				"addq.l  #1,%%d0\n\t"
				"move.l  %%d0,%%a0\n\t"
				"cmpi.l  #4,%%d0\n\t"
				"bne.s   1b"
				:
				:
				: "d0", "d1", "a0", "memory", "cc"
			);
			Super((void *)oldssp);
		}
	}
};

Plugin *FireBeePluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<FireBeeDLObject>(node.getPath());
}

#endif // defined(DYNAMIC_MODULES) && defined(__MINT__) && defined(__ELF__) && defined(__mcoldfire__)
