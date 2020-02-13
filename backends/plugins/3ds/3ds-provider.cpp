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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(__3DS__)

#include "backends/plugins/3ds/3ds-provider.h"
#include "backends/plugins/elf/arm-loader.h"

#include "common/debug.h"

#include <3ds.h>

extern uint32 __end__; // End of the main program in memory. Set by the linker.

static uint32 alignUp(uint32 ptr, uint32 align) {
	return (ptr + align - 1) & ~(align - 1);
}

class CTRDLObject : public ARMDLObject {
public:
	CTRDLObject():
			ARMDLObject(),
			_segmentHeapAddress(0) {
	}

protected:
	static const uint32 kPageSize = 0x1000;

	uint32 _segmentHeapAddress;

	void flushDataCache(void *ptr, uint32 len) const override {
		svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ptr, len);
	}

	void protectMemory(void *ptr, uint32 len, int prot) const override {
		debug(2, "elfloader: Protecting memory at %p, len %d with %d", ptr, len, prot);

		uint32 ctrFlags = 0;
		if (prot & PF_R) ctrFlags |= MEMPERM_READ;
		if (prot & PF_W) ctrFlags |= MEMPERM_WRITE;
		if (prot & PF_X) ctrFlags |= MEMPERM_EXECUTE;

		// The kernel expects the range to be aligned to page boundaries
		len = alignUp(len, kPageSize);

		Handle currentHandle;
		svcDuplicateHandle(&currentHandle, CUR_PROCESS_HANDLE);
		svcControlProcessMemory(currentHandle, (uint32)ptr, 0, len, MEMOP_PROT, ctrFlags);
		svcCloseHandle(currentHandle);
	}

	void *allocateMemory(uint32 align, uint32 size) override {
		assert(!_segmentHeapAddress); // At the moment we can only load a single segment

		_segmentHeapAddress = (uint32)ARMDLObject::allocateMemory(align, size);
		if (!_segmentHeapAddress) {
			return nullptr;
		}

		size = alignUp(size, kPageSize);

		// The plugin needs to be loaded near the main executable for PC-relative calls
		// to resolve. The segment is allocated on the heap which not in the +/-32 MB
		// range of the main executable. So here we map the segment address in the heap
		// to a virtual address just after the main executable.
		uint32 segmentNearAddress = alignUp((uint32)&__end__, kPageSize) + kPageSize;

		Handle currentHandle;
		svcDuplicateHandle(&currentHandle, CUR_PROCESS_HANDLE);
		Result mapResult = svcControlProcessMemory(currentHandle, segmentNearAddress, _segmentHeapAddress, size, MEMOP_MAP, MemPerm(MEMPERM_READ | MEMPERM_WRITE));
		svcCloseHandle(currentHandle);

		if (mapResult != 0) {
			warning("elfloader: unable to map segment memory (%x) near the excutable (%x)", _segmentHeapAddress, segmentNearAddress);

			ARMDLObject::deallocateMemory((void *)_segmentHeapAddress, size);
			_segmentHeapAddress = 0;
			return nullptr;
		}

		return (void *)segmentNearAddress;
	}

	void deallocateMemory(void *ptr, uint32 size) override {
		assert(_segmentHeapAddress);

		uint32 alignedSize = alignUp(size, kPageSize);

		Handle currentHandle;
		svcDuplicateHandle(&currentHandle, CUR_PROCESS_HANDLE);
		svcControlProcessMemory(currentHandle, (uint32)ptr, _segmentHeapAddress, alignedSize, MEMOP_UNMAP, MemPerm(MEMPERM_READ | MEMPERM_WRITE));
		svcCloseHandle(currentHandle);

		ARMDLObject::deallocateMemory((void *)_segmentHeapAddress, size);

		_segmentHeapAddress = 0;
	}

};

Plugin *CTRPluginProvider::createPlugin(const Common::FSNode &node) const {
	return new TemplatedELFPlugin<CTRDLObject>(node.getPath());
}

#endif // defined(DYNAMIC_MODULES) && defined(__3DS__)
