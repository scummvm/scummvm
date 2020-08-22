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

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

void Glulx::setup_vm() {
	byte buf[4 * 7];

	pc = 0;           // Clear this, so that error messages are cleaner.
	prevpc = 0;

	// Read in all the size constants from the game file header
	stream_char_handler = nullptr;
	stream_unichar_handler = nullptr;

	_gameFile.seek(gamefile_start + 8);
	if (_gameFile.read(buf, 4 * 7) != (4 * 7))
		fatal_error("The game file header is too short.");

	ramstart = Read4(buf + 0);
	endgamefile = Read4(buf + 4);
	origendmem = Read4(buf + 8);
	stacksize = Read4(buf + 12);
	startfuncaddr = Read4(buf + 16);
	origstringtable = Read4(buf + 20);
	checksum = Read4(buf + 24);

	// Set the protection range to (0, 0), meaning "off".
	protectstart = 0;
	protectend = 0;

	// Do a few sanity checks.
	if ((ramstart & 0xFF)
	        || (endgamefile & 0xFF)
	        || (origendmem & 0xFF)
	        || (stacksize & 0xFF)) {
		nonfatal_warning("One of the segment boundaries in the header is not "
		                 "256-byte aligned.");
	}

	if (endgamefile != gamefile_len) {
		nonfatal_warning("The gamefile length does not match the header "
		                 "endgamefile length.");
	}

	if (ramstart < 0x100 || endgamefile < ramstart || origendmem < endgamefile) {
		fatal_error("The segment boundaries in the header are in an impossible "
		            "order.");
	}
	if (stacksize < 0x100) {
		fatal_error("The stack size in the header is too small.");
	}

	/* Allocate main memory and the stack. This is where memory allocation
	   errors are most likely to occur. */
	endmem = origendmem;
	memmap = (byte *)glulx_malloc(origendmem);
	if (!memmap) {
		fatal_error("Unable to allocate Glulx memory space.");
	}
	stack = (byte *)glulx_malloc(stacksize);
	if (!stack) {
		glulx_free(memmap);
		memmap = nullptr;
		fatal_error("Unable to allocate Glulx stack space.");
	}
	stringtable = 0;

	// Initialize various other things in the terp.
	init_operands();
	init_serial();

	// Set up the initial machine state.
	vm_restart();

	/* If the debugger is compiled in, check that the debug data matches
	   the game. (This only prints warnings for mismatch.) */
	debugger_check_story_file();

	/* Also, set up any start-time debugger state. This may do a block-
	   and-debug, if the user has requested that. */
	debugger_setup_start_state();
}

void Glulx::finalize_vm() {
	stream_set_table(0);

	if (memmap) {
		glulx_free(memmap);
		memmap = nullptr;
	}
	if (stack) {
		glulx_free(stack);
		stack = nullptr;
	}

	final_serial();
}

void Glulx::vm_restart() {
	uint lx;
	int res;
	int bufpos;
	char buf[0x100];

	/* Deactivate the heap (if it was active). */
	heap_clear();

	/* Reset memory to the original size. */
	lx = change_memsize(origendmem, false);
	if (lx)
		fatal_error("Memory could not be reset to its original size.");

	/* Load in all of main memory. We do this in 256-byte chunks, because
	   why rely on OS stream buffering? */
	_gameFile.seek(gamefile_start);
	bufpos = 0x100;

	for (lx = 0; lx < endgamefile; lx++) {
		if (bufpos >= 0x100) {
			int count = _gameFile.read(buf, 0x100);
			if (count != 0x100) {
				fatal_error("The game file ended unexpectedly.");
			}
			bufpos = 0;
		}

		res = buf[bufpos++];
		if (lx >= protectstart && lx < protectend)
			continue;
		memmap[lx] = res;
	}
	for (lx = endgamefile; lx < origendmem; lx++) {
		memmap[lx] = 0;
	}

	/* Reset all the registers */
	stackptr = 0;
	frameptr = 0;
	pc = 0;
	prevpc = 0;
	stream_set_iosys(0, 0);
	stream_set_table(origstringtable);
	valstackbase = 0;
	localsbase = 0;

	/* Note that we do not reset the protection range. */

	/* Push the first function call. (No arguments.) */
	enter_function(startfuncaddr, 0, nullptr);

	/* We're now ready to execute. */
}

uint Glulx::change_memsize(uint newlen, bool internal) {
	uint lx;
	unsigned char *newmemmap;

	if (newlen == endmem)
		return 0;

#ifdef FIXED_MEMSIZE
	return 1;
#else /* FIXED_MEMSIZE */

	if ((!internal) && heap_is_active())
		fatal_error("Cannot resize Glulx memory space while heap is active.");

	if (newlen < origendmem)
		fatal_error("Cannot resize Glulx memory space smaller than it started.");

	if (newlen & 0xFF)
		fatal_error("Can only resize Glulx memory space to a 256-byte boundary.");

	newmemmap = (unsigned char *)glulx_realloc(memmap, newlen);
	if (!newmemmap) {
		/* The old block is still in place, unchanged. */
		return 1;
	}
	memmap = newmemmap;

	if (newlen > endmem) {
		for (lx = endmem; lx < newlen; lx++) {
			memmap[lx] = 0;
		}
	}

	endmem = newlen;

	return 0;

#endif /* FIXED_MEMSIZE */
}

uint *Glulx::pop_arguments(uint count, uint addr) {
	uint ix;
	uint argptr;
	uint *array;

#define MAXARGS (32)
	static uint statarray[MAXARGS];
	static uint *dynarray = nullptr;
	static uint dynarray_size = 0;

	if (count == 0)
		return nullptr;

	if (count <= MAXARGS) {
		/* Store in the static array. */
		array = statarray;
	} else {
		if (!dynarray) {
			dynarray_size = count + 8;
			dynarray = (uint *)glulx_malloc(sizeof(uint) * dynarray_size);
			if (!dynarray)
				fatal_error("Unable to allocate function arguments.");
			array = dynarray;
		} else {
			if (dynarray_size >= count) {
				/* It fits. */
				array = dynarray;
			} else {
				dynarray_size = count + 8;
				dynarray = (uint *)glulx_realloc(dynarray, sizeof(uint) * dynarray_size);
				if (!dynarray)
					fatal_error("Unable to reallocate function arguments.");
				array = dynarray;
			}
		}
	}

	if (!addr) {
		if (stackptr < valstackbase + 4 * count)
			fatal_error("Stack underflow in arguments.");
		stackptr -= 4 * count;
		for (ix = 0; ix < count; ix++) {
			argptr = stackptr + 4 * ((count - 1) - ix);
			array[ix] = Stk4(argptr);
		}
	} else {
		for (ix = 0; ix < count; ix++) {
			array[ix] = Mem4(addr);
			addr += 4;
		}
	}

	return array;
}

void Glulx::verify_address(uint addr, uint count) {
	if (addr >= endmem)
		fatal_error_i("Memory access out of range", addr);
	if (count > 1) {
		addr += (count - 1);
		if (addr >= endmem)
			fatal_error_i("Memory access out of range", addr);
	}
}

void Glulx::verify_address_write(uint addr, uint count) {
	if (addr < ramstart)
		fatal_error_i("Memory write to read-only address", addr);
	if (addr >= endmem)
		fatal_error_i("Memory access out of range", addr);
	if (count > 1) {
		addr += (count - 1);
		if (addr >= endmem)
			fatal_error_i("Memory access out of range", addr);
	}
}

void Glulx::verify_array_addresses(uint addr, uint count, uint size) {
	uint bytecount;
	if (addr >= endmem)
		fatal_error_i("Memory access out of range", addr);

	if (count == 0)
		return;
	bytecount = count * size;

	/* If just multiplying by the element size overflows, we have trouble. */
	if (bytecount < count)
		fatal_error_i("Memory access way too long", addr);

	/* If the byte length by itself is too long, or if its end overflows,
	   we have trouble. */
	if (bytecount > endmem || addr + bytecount < addr)
		fatal_error_i("Memory access much too long", addr);
	/* The simple length test. */
	if (addr + bytecount > endmem)
		fatal_error_i("Memory access too long", addr);
}

} // End of namespace Glulx
} // End of namespace Glk
