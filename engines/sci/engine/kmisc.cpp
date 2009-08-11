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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"

#include <time.h>	// FIXME: For struct tm

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/gc.h"

namespace Sci {

reg_t kRestartGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	s->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;
	s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE; // This appears to help

	shrink_execution_stack(s, s->execution_stack_base + 1);

	script_abort_flag = 1; // Force vm to abort ASAP
	return NULL_REG;
}

/* kGameIsRestarting():
** Returns the restarting_flag in acc
*/
reg_t kGameIsRestarting(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, (s->restarting_flags & SCI_GAME_WAS_RESTARTED));

	if (argc) { // Only happens during replay
		if (!argv[0].toUint16()) // Set restarting flag
			s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED;
	}

	return s->r_acc;
}

reg_t kHaveMouse(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, -1);
}

reg_t kMemoryInfo(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (argv[0].offset) {
	case 0: // Total free heap memory
	case 1: // Largest heap block available
	case 2: // Largest available hunk memory block
	case 3: // Total amount of hunk memory
	case 4: // Amount of free DOS paragraphs- SCI01
		return make_reg(0, 0x7fff); // Must not be 0xffff, or some memory calculations will overflow

	default:
		warning("Unknown MemoryInfo operation: %04x", argv[0].offset);
	}

	return NULL_REG;
}

#define SCI_MAPPED_UNKNOWN_KFUNCTIONS_NR 0x75
// kfunct_mappers below doubles for unknown kfunctions

reg_t k_Unknown(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	warning("Unhandled Unknown function %04x", funct_nr);
	return NULL_REG;
}

reg_t kFlushResources(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	run_gc(s);
	debugC(2, kDebugLevelRoom, "Entering room number %d", argv[0].toUint16());
	return s->r_acc;
}

reg_t kSetDebug(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	printf("Debug mode activated\n");

	scriptState.seeking = kDebugSeekNothing;
	scriptState.runningStep = 0;
	return s->r_acc;
}

enum {
	_K_NEW_GETTIME_TICKS = 0,
	_K_NEW_GETTIME_TIME_12HOUR = 1,
	_K_NEW_GETTIME_TIME_24HOUR = 2,
	_K_NEW_GETTIME_DATE = 3
};

reg_t kGetTime(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	tm loc_time;
	uint32 start_time;
	int retval = 0; // Avoid spurious warning

#if 0
	// Reset optimization flags: If this function is called,
	// the game may be waiting for a timeout
	s->kernel_opt_flags &= ~(KERNEL_OPT_FLAG_GOT_EVENT | KERNEL_OPT_FLAG_GOT_2NDEVENT);
#endif

	g_system->getTimeAndDate(loc_time);
	start_time = g_system->getMillis() - s->game_start_time;

	if ((s->_flags & GF_SCI0_OLDGETTIME) && argc) { // Use old semantics
		retval = (loc_time.tm_hour % 12) * 3600 + loc_time.tm_min * 60 + loc_time.tm_sec;
		debugC(2, kDebugLevelTime, "GetTime(timeofday) returns %d", retval);
		return make_reg(0, retval);
	}

	int mode = (argc > 0) ? argv[0].toUint16() : 0;

	switch (mode) {
	case _K_NEW_GETTIME_TICKS :
		retval = start_time * 60 / 1000;
		debugC(2, kDebugLevelTime, "GetTime(elapsed) returns %d", retval);
		break;
	case _K_NEW_GETTIME_TIME_12HOUR :
		retval = ((loc_time.tm_hour % 12) << 12) | (loc_time.tm_min << 6) | (loc_time.tm_sec);
		debugC(2, kDebugLevelTime, "GetTime(12h) returns %d", retval);
		break;
	case _K_NEW_GETTIME_TIME_24HOUR :
		retval = (loc_time.tm_hour << 11) | (loc_time.tm_min << 5) | (loc_time.tm_sec >> 1);
		debugC(2, kDebugLevelTime, "GetTime(24h) returns %d", retval);
		break;
	case _K_NEW_GETTIME_DATE :
		retval = loc_time.tm_mday | ((loc_time.tm_mon + 1) << 5) | (((loc_time.tm_year + 1900) & 0x7f) << 9);
		debugC(2, kDebugLevelTime, "GetTime(date) returns %d", retval);
		break;
	default:
		warning("Attempt to use unknown GetTime mode %d", mode);
		break;
	}

	return make_reg(0, retval);
}

enum {
	K_MEMORY_ALLOCATE_CRITICAL		= 1,
	K_MEMORY_ALLOCATE_NONCRITICAL   = 2,
	K_MEMORY_FREE					= 3,
	K_MEMORY_MEMCPY					= 4,
	K_MEMORY_PEEK					= 5,
	K_MEMORY_POKE					= 6
};

reg_t kMemory(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case K_MEMORY_ALLOCATE_CRITICAL :
		if (!s->seg_manager->allocDynmem(argv[1].toUint16(), "kMemory() critical", &s->r_acc)) {
			error("Critical heap allocation failed");
		}
		return s->r_acc;
		break;
	case K_MEMORY_ALLOCATE_NONCRITICAL :
		s->seg_manager->allocDynmem(argv[1].toUint16(), "kMemory() non-critical", &s->r_acc);
		break;
	case K_MEMORY_FREE :
		if (s->seg_manager->freeDynmem(argv[1])) {
			error("Attempt to kMemory::free() non-dynmem pointer %04x:%04x", PRINT_REG(argv[1]));
		}
		break;
	case K_MEMORY_MEMCPY : {
		int size = argv[3].toUint16();
		byte *dest = kernel_dereference_bulk_pointer(s, argv[1], size);
		byte *src = kernel_dereference_bulk_pointer(s, argv[2], size);

		if (dest && src)
			memcpy(dest, src, size);
		else {
			warning("Could not execute kMemory:memcpy of %d bytes:", size);
			if (!dest) {
				warning("  dest ptr (%04x:%04x) invalid/memory region too small", PRINT_REG(argv[1]));
			}
			if (!src) {
				warning("  src ptr (%04x:%04x) invalid/memory region too small", PRINT_REG(argv[2]));
			}
		}
		break;
	}
	case K_MEMORY_PEEK : {
		byte *ref = kernel_dereference_bulk_pointer(s, argv[1], 2);

		if (!ref) {
			// This occurs in KQ5CD when interacting with certain objects
			warning("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}
		if (s->seg_manager->_heap[argv[1].segment]->getType() == MEM_OBJ_LOCALS)
			return *((reg_t *) ref);
		else
			return make_reg(0, (int16)READ_LE_UINT16(ref));
		break;
	}
	case K_MEMORY_POKE : {
		byte *ref = kernel_dereference_bulk_pointer(s, argv[1], 2);

		if (!ref) {
			warning("Attempt to poke invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}

		if (s->seg_manager->_heap[argv[1].segment]->getType() == MEM_OBJ_LOCALS)
			*((reg_t *) ref) = argv[2];
		else {
			if (argv[2].segment) {
				error("Attempt to poke memory reference %04x:%04x to %04x:%04x", PRINT_REG(argv[2]), PRINT_REG(argv[1]));
				return s->r_acc;
				WRITE_LE_UINT16(ref, argv[2].offset); // ?
			}
		}
		return s->r_acc;
		break;
	}
	}

	return s->r_acc;
}

reg_t kStub(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char tmpbuf[200];
	sprintf(tmpbuf, "Unimplemented syscall: %s[%x] (", 
					((SciEngine*)g_engine)->getKernel()->getKernelName(funct_nr).c_str(), funct_nr);

	for (int i = 0; i < argc; i++) {
		char tmpbuf2[20];
		sprintf(tmpbuf2, "%04x:%04x", PRINT_REG(argv[i]));
		if (i + 1 < argc)
			strcat(tmpbuf2, ", ");
		strcat(tmpbuf, tmpbuf2);
	}
	strcat(tmpbuf, ")");

	warning("%s", tmpbuf);

	return NULL_REG;
}

reg_t kNOP(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	warning("Kernel function 0x%02x (%s) invoked: unmapped", funct_nr, ((SciEngine*)g_engine)->getKernel()->_kernelFuncs[funct_nr].orig_name.c_str());
	return NULL_REG;
}

} // End of namespace Sci
