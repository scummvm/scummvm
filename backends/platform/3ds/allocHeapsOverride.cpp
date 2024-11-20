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

// This file largely reuses code from libctru
// https://github.com/devkitPro/libctru/blob/33a570b1c335cc38e50b7fef6bf4ed840bbd4ab2/libctru/source/system/allocateHeaps.c
#include <3ds.h>

#define HEAP_SPLIT_SIZE_CAP  (24 << 20) // 24MB
#define LINEAR_HEAP_SIZE_CAP (32 << 20) // 32MB

// Hack to get the hardware's FCRAM layout ID. It just werks.
// Layouts 0 through 5 are used exclusively in Old 3DS systems.
// Layouts 6 through 8 are used exclusively in New 3DS systems.
#define APPMEMTYPE (*(u32*)0x1FF80030)

extern "C" void __system_allocateHeaps(void) {
	extern char* fake_heap_start;
	extern char* fake_heap_end;
	extern u32 __ctru_heap;
	extern u32 __ctru_linear_heap;
	extern u32 __ctru_heap_size;
	extern u32 __ctru_linear_heap_size;
	Result rc;

	// Retrieve handle to the resource limit object for our process
	Handle reslimit = 0;
	rc = svcGetResourceLimit(&reslimit, CUR_PROCESS_HANDLE);
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Retrieve information about total/used memory
	s64 maxCommit = 0, currentCommit = 0;
	ResourceLimitType reslimitType = RESLIMIT_COMMIT;
	svcGetResourceLimitLimitValues(&maxCommit, reslimit, &reslimitType, 1); // for APPLICATION this is equal to APPMEMALLOC at all times
	svcGetResourceLimitCurrentValues(&currentCommit, reslimit, &reslimitType, 1);
	svcCloseHandle(reslimit);

	// Calculate how much remaining free memory is available
	u32 remaining = (u32)(maxCommit - currentCommit) &~ 0xFFF;

	__ctru_heap_size = 0;
	// New 3DS needs more linear memory than Old 3DS to boot up ScummVM; app instantly crashes otherwise.
	// 0x00A00000 bytes = 10 MiB, for Old 3DS
	// 0x01400000 bytes = 20 MiB, for New 3DS
	__ctru_linear_heap_size = APPMEMTYPE < 6 ? 0x00A00000 : 0x01400000;
	__ctru_heap_size = remaining - __ctru_linear_heap_size;

	// Allocate the application heap
	rc = svcControlMemory(&__ctru_heap, OS_HEAP_AREA_BEGIN, 0x0, __ctru_heap_size, MEMOP_ALLOC, static_cast<MemPerm>(static_cast<int>(MEMPERM_READ) | static_cast<int>(MEMPERM_WRITE)));
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Allocate the linear heap
	rc = svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, static_cast<MemPerm>(static_cast<int>(MEMPERM_READ) | static_cast<int>(MEMPERM_WRITE)));
	if (R_FAILED(rc))
		svcBreak(USERBREAK_PANIC);

	// Mappable allocator init
	mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;

}
