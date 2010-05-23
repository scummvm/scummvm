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

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/gc.h"
#include "sci/graphics/gui.h"

namespace Sci {

reg_t kRestartGame(EngineState *s, int argc, reg_t *argv) {
	s->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;

	s->shrinkStackToBase();

	script_abort_flag = 1; // Force vm to abort ASAP
	return NULL_REG;
}

/* kGameIsRestarting():
** Returns the restarting_flag in acc
*/
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, (s->restarting_flags & SCI_GAME_WAS_RESTARTED));

	if (argc) { // Only happens during replay
		if (!argv[0].toUint16()) // Set restarting flag
			s->restarting_flags &= ~SCI_GAME_WAS_RESTARTED;
	}

	uint32 neededSleep = 30;

	// WORKAROUND:
	// LSL3 calculates a machinespeed variable during game startup (right after the filthy questions)
	//  This one would go through w/o throttling resulting in having to do 1000 pushups or something
	//  Another way of handling this would be delaying incrementing of "machineSpeed" selector
	if (!strcmp(g_sci->getGameID(), "lsl3") && s->currentRoomNumber() == 290)
		s->_throttleTrigger = true;
	if (!strcmp(g_sci->getGameID(), "iceman") && s->currentRoomNumber() == 27) {
		s->_throttleTrigger = true;
		neededSleep = 60;
	}

	if (s->_throttleTrigger) {
		// Some games seem to get the duration of main loop initially and then switch of animations for the whole game
		//  based on that (qfg2, iceman). We are now running full speed initially to avoid that.
		// It seems like we dont need to do that anymore
		//if (s->_throttleCounter < 50) {
		//	s->_throttleCounter++;
		//	return s->r_acc;
		//}

		uint32 curTime = g_system->getMillis();
		uint32 duration = curTime - s->_throttleLastTime;

		if (duration < neededSleep) {
			s->_event->sleep(neededSleep - duration);
			s->_throttleLastTime = g_system->getMillis();
		} else {
			s->_throttleLastTime = curTime;
		}
		s->_throttleTrigger = false;
	}

	return s->r_acc;
}

reg_t kHaveMouse(EngineState *s, int argc, reg_t *argv) {
	return SIGNAL_REG;
}

enum kMemoryInfoFunc {
	K_MEMORYINFO_LARGEST_HEAP_BLOCK = 0, // Largest heap block available
	K_MEMORYINFO_FREE_HEAP = 1, // Total free heap memory
	K_MEMORYINFO_LARGEST_HUNK_BLOCK = 2, // Largest available hunk memory block
	K_MEMORYINFO_FREE_HUNK = 3, // Amount of free DOS paragraphs
	K_MEMORYINFO_TOTAL_HUNK = 4 // Total amount of hunk memory (SCI01)
};

reg_t kMemoryInfo(EngineState *s, int argc, reg_t *argv) {
	const uint16 size = 0x7fff;  // Must not be 0xffff, or some memory calculations will overflow

	switch (argv[0].offset) {
	case K_MEMORYINFO_LARGEST_HEAP_BLOCK:
		// In order to prevent "Memory fragmented" dialogs from
		// popping up in some games, we must return FREE_HEAP - 2 here.
		return make_reg(0, size - 2);
	case K_MEMORYINFO_FREE_HEAP:
	case K_MEMORYINFO_LARGEST_HUNK_BLOCK:
	case K_MEMORYINFO_FREE_HUNK:
	case K_MEMORYINFO_TOTAL_HUNK:
		return make_reg(0, size);

	default:
		warning("Unknown MemoryInfo operation: %04x", argv[0].offset);
	}

	return NULL_REG;
}

enum kMemorySegmentFunc {
	K_MEMORYSEGMENT_SAVE_DATA = 0,
	K_MEMORYSEGMENT_RESTORE_DATA = 1
};

reg_t kMemorySegment(EngineState *s, int argc, reg_t *argv) {
	// MemorySegment provides access to a 256-byte block of memory that remains
	// intact across restarts and restores

	switch (argv[0].toUint16()) {
	case K_MEMORYSEGMENT_SAVE_DATA: {
		if (argc < 3)
			error("Insufficient number of arguments passed to MemorySegment");
		uint16 size = argv[2].toUint16();

		if (!size)
			size = s->_segMan->strlen(argv[1]) + 1;

		if (size > EngineState::kMemorySegmentMax)
			size = EngineState::kMemorySegmentMax;

		s->_memorySegmentSize = size;

		// We assume that this won't be called on pointers
		s->_segMan->memcpy(s->_memorySegment, argv[1], size);
		break;
	}
	case K_MEMORYSEGMENT_RESTORE_DATA:
		s->_segMan->memcpy(argv[1], s->_memorySegment, s->_memorySegmentSize);
		break;
	default:
		error("Unknown MemorySegment operation %04x", argv[0].toUint16());
	}

	return argv[1];
}

reg_t kFlushResources(EngineState *s, int argc, reg_t *argv) {
	run_gc(s);
	debugC(2, kDebugLevelRoom, "Entering room number %d", argv[0].toUint16());
	return s->r_acc;
}

reg_t kSetDebug(EngineState *s, int argc, reg_t *argv) {
	printf("Debug mode activated\n");

	g_debugState.seeking = kDebugSeekNothing;
	g_debugState.runningStep = 0;
	return s->r_acc;
}

enum {
	K_NEW_GETTIME_TICKS = 0,
	K_NEW_GETTIME_TIME_12HOUR = 1,
	K_NEW_GETTIME_TIME_24HOUR = 2,
	K_NEW_GETTIME_DATE = 3
};

reg_t kGetTime(EngineState *s, int argc, reg_t *argv) {
	TimeDate loc_time;
	uint32 elapsedTime;
	int retval = 0; // Avoid spurious warning

	g_system->getTimeAndDate(loc_time);
	elapsedTime = g_system->getMillis() - s->game_start_time;

	int mode = (argc > 0) ? argv[0].toUint16() : 0;

	if (getSciVersion() <= SCI_VERSION_0_LATE && mode > 1)
		warning("kGetTime called in SCI0 with mode %d (expected 0 or 1)", mode);

	switch (mode) {
	case K_NEW_GETTIME_TICKS :
		retval = elapsedTime * 60 / 1000;
		debugC(2, kDebugLevelTime, "GetTime(elapsed) returns %d", retval);
		break;
	case K_NEW_GETTIME_TIME_12HOUR :
		retval = ((loc_time.tm_hour % 12) << 12) | (loc_time.tm_min << 6) | (loc_time.tm_sec);
		debugC(2, kDebugLevelTime, "GetTime(12h) returns %d", retval);
		break;
	case K_NEW_GETTIME_TIME_24HOUR :
		retval = (loc_time.tm_hour << 11) | (loc_time.tm_min << 5) | (loc_time.tm_sec >> 1);
		debugC(2, kDebugLevelTime, "GetTime(24h) returns %d", retval);
		break;
	case K_NEW_GETTIME_DATE :
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

reg_t kMemory(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case K_MEMORY_ALLOCATE_CRITICAL :
		if (!s->_segMan->allocDynmem(argv[1].toUint16(), "kMemory() critical", &s->r_acc)) {
			error("Critical heap allocation failed");
		}
		break;
	case K_MEMORY_ALLOCATE_NONCRITICAL :
		s->_segMan->allocDynmem(argv[1].toUint16(), "kMemory() non-critical", &s->r_acc);
		break;
	case K_MEMORY_FREE :
		if (s->_segMan->freeDynmem(argv[1])) {
			error("Attempt to kMemory::free() non-dynmem pointer %04x:%04x", PRINT_REG(argv[1]));
		}
		break;
	case K_MEMORY_MEMCPY : {
		int size = argv[3].toUint16();
		s->_segMan->memcpy(argv[1], argv[2], size);
		break;
	}
	case K_MEMORY_PEEK : {
		SegmentRef ref = s->_segMan->dereference(argv[1]);

		if (!ref.isValid() || ref.maxSize < 2) {
			// This occurs in KQ5CD when interacting with certain objects
			warning("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}
		if (ref.isRaw)
			return make_reg(0, (int16)READ_LE_UINT16(ref.raw));
		else {
			if (ref.skipByte)
				error("Attempt to peek memory at odd offset %04X:%04X", PRINT_REG(argv[1]));
			return *(ref.reg);
		}
		break;
	}
	case K_MEMORY_POKE : {
		SegmentRef ref = s->_segMan->dereference(argv[1]);

		if (!ref.isValid() || ref.maxSize < 2) {
			warning("Attempt to poke invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}

		if (ref.isRaw) {
			if (argv[2].segment) {
				error("Attempt to poke memory reference %04x:%04x to %04x:%04x", PRINT_REG(argv[2]), PRINT_REG(argv[1]));
				return s->r_acc;
			}
			WRITE_LE_UINT16(ref.raw, argv[2].offset);
		} else {
			if (ref.skipByte)
				error("Attempt to poke memory at odd offset %04X:%04X", PRINT_REG(argv[1]));
			*(ref.reg) = argv[2];
		}
		break;
	}
	}

	return s->r_acc;
}

// kIconBar is really a subop of kPlatform for SCI1.1 Mac
reg_t kIconBar(EngineState *s, int argc, reg_t *argv) {
	// TODO...

	if (argv[0].toUint16() == 4 && argv[1].toUint16() == 0)
		for (int i = 0; i < argv[2].toUint16(); i++)
			warning("kIconBar: Icon Object %d = %04x:%04x", i, PRINT_REG(argv[i + 3]));

	// Other calls seem to handle selecting/deselecting them

	return NULL_REG;
}

enum kSciPlatforms {
	kSciPlatformDOS = 1,
	kSciPlatformWindows = 2
};

enum kPlatformOps {
	kPlatformUnk0 = 0,
	kPlatformCDSpeed = 1,
	kPlatformUnk2 = 2,
	kPlatformCDCheck = 3,
	kPlatformGetPlatform = 4,
	kPlatformUnk5 = 5,
	kPlatformIsHiRes = 6,
	kPlatformIsItWindows = 7
};

reg_t kPlatform(EngineState *s, int argc, reg_t *argv) {
	bool isWindows = g_sci->getPlatform() == Common::kPlatformWindows;

	if (argc == 0 && getSciVersion() < SCI_VERSION_2) {
		// This is called in KQ5CD with no parameters, where it seems to do some graphics
		// driver check. This kernel function didn't have subfunctions then. If 0 is
		// returned, the game functions normally, otherwise all the animations show up
		// like a slideshow (e.g. in the intro). So we return 0. However, the behavior
		// changed for kPlatform with no parameters in SCI32.
		return NULL_REG;
	}

	uint16 operation = (argc == 0) ? 0 : argv[0].toUint16();

	switch (operation) {
	case kPlatformCDSpeed:
		// TODO: Returns CD Speed?
		warning("STUB: kPlatform(CDSpeed)");
		break;
	case kPlatformUnk2:
		// Always returns 2
		return make_reg(0, 2);
	case kPlatformCDCheck:
		// TODO: Some sort of CD check?
		warning("STUB: kPlatform(CDCheck)");
		break;
	case kPlatformUnk0:
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() == SCI_VERSION_1_1)
			return kIconBar(s, argc - 1, argv + 1);
		// Otherwise, fall through
	case kPlatformGetPlatform:
		return make_reg(0, (isWindows) ? kSciPlatformWindows : kSciPlatformDOS);
	case kPlatformUnk5:
		// This case needs to return the opposite of case 6 to get hires graphics
		return make_reg(0, !isWindows);
	case kPlatformIsHiRes:
		return make_reg(0, isWindows);
	case kPlatformIsItWindows:
		return make_reg(0, isWindows);
	default:
		warning("Unsupported kPlatform operation %d", operation);
	}

	return NULL_REG;
}

} // End of namespace Sci
