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

// Allow use of stuff in <nds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(__DS__)

#include <malloc.h>
#include <nds.h>

#include "backends/plugins/ds/ds-provider.h"
#include "backends/plugins/elf/arm-loader.h"

class DSDLObject : public ARMDLObject {
protected:
	virtual void flushDataCache(void *ptr, uint32 len) const {
		DC_FlushRange(ptr, len);
		IC_InvalidateRange(ptr, len);
	}
};

Plugin *DSPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<DSDLObject>(node.getPath());
}

void DSPluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	dirs.push_back(Common::FSNode("nitro:/plugins"));
}

#endif // defined(DYNAMIC_MODULES) && defined(__DS__)
