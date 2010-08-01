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
#include "sci/graphics/maciconbar.h"

namespace Sci {

reg_t kRestartGame(EngineState *s, int argc, reg_t *argv) {
	s->shrinkStackToBase();

	s->abortScriptProcessing = kAbortRestartGame; // Force vm to abort ASAP
	return NULL_REG;
}

/* kGameIsRestarting():
** Returns the restarting_flag in acc
*/
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv) {
	s->r_acc = make_reg(0, s->gameIsRestarting);

	if (argc) { // Only happens during replay
		if (!argv[0].toUint16()) // Set restarting flag
			s->gameIsRestarting = GAMEISRESTARTING_NONE;
	}

	uint32 neededSleep = 30;

	// WORKAROUNDS:
	switch (g_sci->getGameId()) {
	case GID_LSL3:
		// LSL3 calculates a machinespeed variable during game startup
		// (right after the filthy questions). This one would go through w/o
		// throttling resulting in having to do 1000 pushups or something. Another
		// way of handling this would be delaying incrementing of "machineSpeed"
		// selector.
		if (s->currentRoomNumber() == 290)
			s->_throttleTrigger = true;
		break;
	case GID_ICEMAN:
		// In ICEMAN the submarine control room is not animating much, so it runs way too fast
		//  we calm it down even more otherwise especially fighting against other submarines
		//  is almost impossible
		if (s->currentRoomNumber() == 27) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
		break;
	default:
		break;
	}

	s->speedThrottler(neededSleep);
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
	// The free heap size returned must not be 0xffff, or some memory
	// calculations will overflow. Crazy Nick's games handle up to 32746
	// bytes (0x7fea), otherwise they throw a warning that the memory is
	// fragmented
	const uint16 size = 0x7fea;

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
		error("Unknown MemoryInfo operation: %04x", argv[0].offset);
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

	g_sci->_debugState.seeking = kDebugSeekNothing;
	g_sci->_debugState.runningStep = 0;
	return s->r_acc;
}

enum {
	KGETTIME_TICKS = 0,
	KGETTIME_TIME_12HOUR = 1,
	KGETTIME_TIME_24HOUR = 2,
	KGETTIME_DATE = 3
};

reg_t kGetTime(EngineState *s, int argc, reg_t *argv) {
	TimeDate loc_time;
	uint32 elapsedTime;
	int retval = 0; // Avoid spurious warning

	g_system->getTimeAndDate(loc_time);
	elapsedTime = g_system->getMillis() - s->gameStartTime;

	int mode = (argc > 0) ? argv[0].toUint16() : 0;

	if (getSciVersion() <= SCI_VERSION_0_LATE && mode > 1)
		error("kGetTime called in SCI0 with mode %d (expected 0 or 1)", mode);

	switch (mode) {
	case KGETTIME_TICKS :
		retval = elapsedTime * 60 / 1000;
		debugC(2, kDebugLevelTime, "GetTime(elapsed) returns %d", retval);
		break;
	case KGETTIME_TIME_12HOUR :
		retval = ((loc_time.tm_hour % 12) << 12) | (loc_time.tm_min << 6) | (loc_time.tm_sec);
		debugC(2, kDebugLevelTime, "GetTime(12h) returns %d", retval);
		break;
	case KGETTIME_TIME_24HOUR :
		retval = (loc_time.tm_hour << 11) | (loc_time.tm_min << 5) | (loc_time.tm_sec >> 1);
		debugC(2, kDebugLevelTime, "GetTime(24h) returns %d", retval);
		break;
	case KGETTIME_DATE :
		retval = loc_time.tm_mday | ((loc_time.tm_mon + 1) << 5) | (((loc_time.tm_year + 1900) & 0x7f) << 9);
		debugC(2, kDebugLevelTime, "GetTime(date) returns %d", retval);
		break;
	default:
		error("Attempt to use unknown GetTime mode %d", mode);
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
		if (!s->_segMan->freeDynmem(argv[1])) {
			if (g_sci->getGameId() == GID_QFG1VGA) {
				// Ignore script bug in QFG1VGA, when closing any conversation dialog with esc
			} else {
				// Usually, the result of a script bug. Non-critical
				warning("Attempt to kMemory::free() non-dynmem pointer %04x:%04x", PRINT_REG(argv[1]));
			}
		}
		break;
	case K_MEMORY_MEMCPY : {
		int size = argv[3].toUint16();
		s->_segMan->memcpy(argv[1], argv[2], size);
		break;
	}
	case K_MEMORY_PEEK : {
		if (!argv[1].segment) {
			// This occurs in KQ5CD when interacting with certain objects
			warning("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
			return s->r_acc;
		}

		SegmentRef ref = s->_segMan->dereference(argv[1]);

		if (!ref.isValid() || ref.maxSize < 2) {
			error("Attempt to peek invalid memory at %04x:%04x", PRINT_REG(argv[1]));
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
			error("Attempt to poke invalid memory at %04x:%04x", PRINT_REG(argv[1]));
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

	if (argv[0].toUint16() == 4 && argv[1].toUint16() == 0) {
		for (int i = 0; i < argv[2].toUint16(); i++)
			g_sci->_gfxMacIconBar->addIcon(argv[i + 3]);

		g_sci->_gfxMacIconBar->drawIcons();		
	}

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
		// This is called in KQ5CD with no parameters, where it seems to do some
		// graphics driver check. This kernel function didn't have subfunctions
		// then. If 0 is returned, the game functions normally, otherwise all
		// the animations show up like a slideshow (e.g. in the intro). So we
		// return 0. However, the behavior changed for kPlatform with no
		// parameters in SCI32.
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
		error("Unsupported kPlatform operation %d", operation);
	}

	return NULL_REG;
}

reg_t kEmpty(EngineState *s, int argc, reg_t *argv) {
	// Placeholder for empty kernel functions which are still called from the
	// engine scripts (like the empty kSetSynonyms function in SCI1.1). This
	// differs from dummy functions because it does nothing and never throws a
	// warning when it is called.
	return s->r_acc;
}

reg_t kStub(EngineState *s, int argc, reg_t *argv) {
	Kernel *kernel = g_sci->getKernel();
	int kernelCallNr = -1;

	Common::List<ExecStack>::iterator callIterator = s->_executionStack.end();
	if (callIterator != s->_executionStack.begin()) {
		callIterator--;
		ExecStack lastCall = *callIterator;
		kernelCallNr = lastCall.debugSelector;
	}

	Common::String warningMsg = "Dummy function k" + kernel->getKernelName(kernelCallNr) +
								Common::String::printf("[%x]", kernelCallNr) +
								" invoked. Params: " +
								Common::String::printf("%d", argc) + " (";

	for (int i = 0; i < argc; i++) {
		warningMsg +=  Common::String::printf("%04x:%04x", PRINT_REG(argv[i]));
		warningMsg += (i == argc - 1 ? ")" : ", ");
	}

	warning("%s", warningMsg.c_str());
	return s->r_acc;
}

reg_t kStubNull(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	return NULL_REG;
}

reg_t kDummy(EngineState *s, int argc, reg_t *argv) {
	kStub(s, argc, argv);
	error("Kernel function was called, which was considered to be unused - see log for details");
}

} // End of namespace Sci
