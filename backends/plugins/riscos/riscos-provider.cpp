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

Plugin *RiscOSPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<RiscOSDLObject>(node.getPath());
}

#endif // defined(DYNAMIC_MODULES) && defined(RISCOS)
