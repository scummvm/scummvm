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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/gc.h"
#ifdef ENABLE_SCI32
#include "sci/engine/guest_additions.h"
#endif
#include "sci/engine/savegame.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/drivers/gfxdriver.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"
#endif
#include "sci/graphics/maciconbar.h"
#include "sci/console.h"
#ifdef ENABLE_SCI32
#include "sci/engine/hoyle5poker.h"
#endif

namespace Sci {

reg_t kRestartGame16(EngineState *s, int argc, reg_t *argv) {
	s->shrinkStackToBase();

	s->abortScriptProcessing = kAbortRestartGame; // Force vm to abort ASAP
	return NULL_REG;
}

/** 
 * kGameIsRestarting returns the EngineState::gameIsRestarting flag.
 * If zero is passed then the flag is cleared before returning its previous value.
 *
 * kGameIsRestarting(0) is unique because it is called by (almost) every game from
 * the game loop (Game:doit, etc) and nowhere else. For that reason, we use this
 * normally simple function as our main speed throttler. It prevents unthrottled
 * games from running as fast as the CPU will allow. This creates a consistent
 * playable experience and avoids the many script bugs that occur when thousands
 * of game cycles run per second on a modern CPU.
 *
 * Early games throttle their own speed by calling kWait on every game cycle.
 * The delay passed to kWait is the game speed selected by the user. At every speed
 * except the fastest (kWait(1), or 17 ms) the kWait delay is greater than our
 * speed throttle delay (30 ms). This means that our kGameIsRestarting throttle
 * has no effect on these games other than to make the user-selected fastest speed
 * run barely faster than the next-fastest selection (33 ms). It's unclear if that
 * is intentional or a side-effect of throttling meant for unthrottled games.
 * The speed throttling code is over a decade old and its details were uncommented.
 * There have also been significant fixes to ScummVM's SCI timing code since then.
 * It may be better to remove kGameIsRestarting throttling from kWait games to
 * simplify things instead of having two overlapping throttlers run. That would
 * allow those to run at their original speed when the user selects Fastest.
 * kWait games are not the ones that run too fast at fast CPU speeds.
 *
 * If a scene in a game needs to be throttled further for compatibility then we
 * add a workaround here. We also use script patches to make unthrottled inner loops
 * call this function so that the program remains responsive and delays are
 * consistent instead of CPU-bound.
 *
 * kGameIsRestarting was removed from SCI2 but Sierra left it in the PC interpreter
 * as a no-op. Game scripts kept calling it even though it didn't do anything.
 * That allows us to keep throttling those games this way; this is why this function
 * is included in the kernel table for all SCI versions. SCI2 Mac scripts removed
 * the call so they are unthrottled by this mechanism. They currently run faster
 * than their PC versions. kGameIsRestarting was completely removed from SCI3.
 *
 * SCI2+ games (or versions) that don't call kGameIsRestarting are still limited
 * to 60fps by GfxFrameout::throttle().
 */
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv) {
	// Always return the previous flag value
	const int16 previousRestartingFlag = s->gameIsRestarting;

	// Games pass zero to clear the restarting flag from their Game:doit method on
	// each cycle. Other scripts query the restarting flag by passing no parameters.
	if (argc > 0 && argv[0].toUint16() == 0) {
		s->gameIsRestarting = GAMEISRESTARTING_NONE;
	} else {
		// Only speed throttle calls from game loops or our script patches.
		return make_reg(0, previousRestartingFlag);
	}

	uint32 neededSleep = g_sci->_speedThrottleDelay; // 30 ms (kSpeedThrottleDefaultDelay)

	// WORKAROUNDS for scripts that require specific speed throttling behavior
	switch (g_sci->getGameId()) {
	case GID_CASTLEBRAIN:
		// In Castle of Dr. Brain, memory color matching puzzle in the first
		// room (room 100), the game scripts constantly poll the state of each
		// stone when the user clicks on one. Since the scene is not animating
		// much, this results in activating and deactivating each stone very
		// quickly (together with its associated tone sound), depending on how
		// low it is in the animate list. This worked somewhat in older PCs, but
		// not in modern computers. We throttle the scene in order to allow the
		// stones to display, otherwise the game scripts reset them too soon.
		// Fixes bug #5543.
		if (s->currentRoomNumber() == 100) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
		break;
	case GID_ICEMAN:
		// In ICEMAN the submarine control room is not animating much, so it
		// runs way too fast. We calm it down even more, otherwise fighting
		// against other submarines is almost impossible.
		if (s->currentRoomNumber() == 27) {
			s->_throttleTrigger = true;
			neededSleep = 60;
		}
		break;
	case GID_SQ5:
		switch (s->currentRoomNumber()) {
		case 104:
			// Introduction: star field. Requires extra speed throttling to achieve
			// comet animations and intended timing. Comets and fast stars move at
			// unthrottled speed, but the comet's animation cycle speed is based on
			// clock time and unsynchronized with movement. On a 386/33, the comets
			// animate and become streaks, but even a 486/50 is too fast for this.
			// The comets move so fast that they reach their destination before they
			// can animate beyond their initial 1x1 pixel cel. Bug #15622
			s->_throttleTrigger = true;
			neededSleep = 90;
			break;
		case 110: {
			// Introduction: exiting the simulator. Requires extra speed throttling to
			// achieve intended timing. All actors in this room have unthrottled speed.
			// This may have been to compensate for the interpreter lag when animating
			// so many actors. Without that lag, and the CPU this scene was timed for,
			// everything moves and animates too fast. However, we must not apply extra
			// throttling to the second half of this scene, or else ego will walk slowly.
			// The original interpreter did not lag here, because it removed the earlier
			// actors from the cast. We detect this by querying the cast size. Bug #15610
			const reg_t cast = s->variables[VAR_GLOBAL][kGlobalVarCast];
			const uint16 castSize = readSelectorValue(s->_segMan, cast, SELECTOR(size));
			if (castSize != 5) { // only throttle before ego begins walking
				s->_throttleTrigger = true;
				neededSleep = 90;
			}
			break;
		}
		default:
			break;
		}
		break;

	// Don't throttle SCI1.1 speed test rooms. Prevents delays at startup.
	// We generically patch these scripts to calculate a passing result,
	// but each script performs a different test, so to speed them all up
	// it's easier to just let them run unthrottled. See: sci11SpeedTestPatch
	case GID_ECOQUEST2:     if (s->currentRoomNumber() ==  10) s->_throttleTrigger = false; break;
	case GID_FREDDYPHARKAS: if (s->currentRoomNumber() ==  28) s->_throttleTrigger = false; break;
	case GID_GK1DEMO:       if (s->currentRoomNumber() ==  17) s->_throttleTrigger = false; break;
	case GID_KQ5:           if (s->currentRoomNumber() ==  99) s->_throttleTrigger = false; break;
	case GID_KQ6:           if (s->currentRoomNumber() ==  99) s->_throttleTrigger = false; break;
	case GID_LAURABOW2:     if (s->currentRoomNumber() ==  28) s->_throttleTrigger = false; break;
	case GID_LSL6:          if (s->currentRoomNumber() ==  99) s->_throttleTrigger = false; break;
	case GID_QFG1VGA:       if (s->currentRoomNumber() == 299) s->_throttleTrigger = false; break;

	default:
		break;
	}

	s->speedThrottler(neededSleep);

	s->_eventCounter = 0;
	s->_paletteSetIntensityCounter = 0;
	return make_reg(0, previousRestartingFlag);
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

	switch (argv[0].getOffset()) {
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
		error("Unknown MemoryInfo operation: %04x", argv[0].getOffset());
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

		if (size > EngineState::kMemorySegmentMax) {
			// This was set to cut the block to 256 bytes. This should be an
			// error, as we won't restore the full block that the game scripts
			// request, thus error out instead.
			//size = EngineState::kMemorySegmentMax;
			error("kMemorySegment: Requested to save more than 256 bytes (%d)", size);
		}

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
	debugC(kDebugLevelRoom, "Entering room number %d", argv[0].toUint16());
	return s->r_acc;
}

reg_t kSetDebug(EngineState *s, int argc, reg_t *argv) {
	// WORKAROUND: For some reason, GK1 calls this unconditionally when
	// watching the intro. Older (SCI0) games call it on room change if
	// a flag is set, in which case the debugger SHOULD get activated.
	// Therefore, don't break into the debugger in GK1, but do so elsewhere.

	if (g_sci->getGameId() != GID_GK1) {
		debug("Debug mode activated");

		g_sci->getDebugger()->attach();
	}

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
	uint16 retval = 0; // Avoid spurious warning

	int mode = (argc > 0) ? argv[0].toUint16() : 0;

	// Modes 2 and 3 are supported since 0.629.
	// This condition doesn't check that exactly, but close enough.
	if (getSciVersion() == SCI_VERSION_0_EARLY && mode > 1)
		error("kGetTime called in SCI0 with mode %d (expected 0 or 1)", mode);

	switch (mode) {
	case KGETTIME_TICKS :
		retval = g_sci->getTickCount();
		debugC(kDebugLevelTime, "GetTime(elapsed) returns %d", retval);
		break;
	case KGETTIME_TIME_12HOUR :
		g_system->getTimeAndDate(loc_time);
		loc_time.tm_hour %= 12;
		if (loc_time.tm_hour == 0) {
			loc_time.tm_hour = 12;
		}
		retval = (loc_time.tm_hour << 12) | (loc_time.tm_min << 6) | (loc_time.tm_sec);
		debugC(kDebugLevelTime, "GetTime(12h) returns %d", retval);
		break;
	case KGETTIME_TIME_24HOUR :
		g_system->getTimeAndDate(loc_time);
		retval = (loc_time.tm_hour << 11) | (loc_time.tm_min << 5) | (loc_time.tm_sec >> 1);
		debugC(kDebugLevelTime, "GetTime(24h) returns %d", retval);
		break;
	case KGETTIME_DATE :
	{
		g_system->getTimeAndDate(loc_time);
		// SCI0 late: Year since 1920 (0 = 1920, 1 = 1921, etc)
		// SCI01 and newer: Year since 1980 (0 = 1980, 1 = 1981, etc)
		// Atari ST SCI0 late versions use the newer base year.
		int baseYear = 80;
		if (getSciVersion() == SCI_VERSION_0_LATE && g_sci->getPlatform() == Common::kPlatformDOS) {
			baseYear = 20;
		}
		retval = loc_time.tm_mday | ((loc_time.tm_mon + 1) << 5) | (((loc_time.tm_year - baseYear) & 0x7f) << 9);
		debugC(kDebugLevelTime, "GetTime(date) returns %d", retval);
		break;
	}
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
	case K_MEMORY_ALLOCATE_CRITICAL: {
		int byteCount = argv[1].toUint16();
		// Sierra themselves allocated at least 2 bytes more than requested.
		// Probably as a safety margin. And they also made size even.
		//
		// This behavior is required by at least these:
		//  - pq3 (multilingual) room 202
		//     when plotting crimes, allocates the returned bytes from kStrLen
		//     on "W" and "E" and wants to put a string in there, which doesn't
		//     fit of course.
		//  - lsl5 (multilingual) room 280
		//     allocates memory according to a previous kStrLen for the name of
		//     the airport ladies (bug #5478), which isn't enough
		byteCount += 2 + (byteCount & 1);

		if (!s->_segMan->allocDynmem(byteCount, "kMemory() critical", &s->r_acc)) {
			error("Critical heap allocation failed");
		}
		break;
	}
	case K_MEMORY_ALLOCATE_NONCRITICAL: {
		int byteCount = argv[1].toUint16();

		// See above
		byteCount += 2 + (byteCount & 1);

		s->_segMan->allocDynmem(byteCount, "kMemory() non-critical", &s->r_acc);
		break;
	}
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
		if (!argv[1].getSegment()) {
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
			return make_reg(0, (int16)READ_SCIENDIAN_UINT16(ref.raw));
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
			if (argv[2].getSegment()) {
				error("Attempt to poke memory reference %04x:%04x to %04x:%04x", PRINT_REG(argv[2]), PRINT_REG(argv[1]));
				return s->r_acc;
			}
			WRITE_SCIENDIAN_UINT16(ref.raw, argv[2].getOffset());		// Amiga versions are BE
		} else {
			if (ref.skipByte)
				error("Attempt to poke memory at odd offset %04X:%04X", PRINT_REG(argv[1]));
			*(ref.reg) = argv[2];
		}
		break;
	}
	default:
		break;
	}

	return s->r_acc;
}

#ifdef ENABLE_SCI32
reg_t kGetConfig(EngineState *s, int argc, reg_t *argv) {
	Common::String setting = s->_segMan->getString(argv[0]);
	reg_t data = readSelector(s->_segMan, argv[1], SELECTOR(data));

	// This function is used to get the benchmarked results stored in the
	// resource.cfg configuration file in Phantasmagoria 1. Normally,
	// the configuration file contains values stored by the installer
	// regarding audio and video settings, which are then used by the
	// executable. In Phantasmagoria, two extra executable files are used
	// to perform system benchmarks:
	// - CPUID for the CPU benchmarks, sets the cpu and cpuspeed settings
	// - HDDTEC for the graphics and CD-ROM benchmarks, sets the videospeed setting
	//
	// These settings are then used by the game scripts directly to modify
	// the game speed and graphics output. The result of this call is stored
	// in global 178. The scripts check these values against the value 425.
	// Anything below that makes Phantasmagoria awfully sluggish, so we're
	// setting everything to 500, which makes the game playable.

	setting.toLowercase();

	if (setting == "videospeed") {
		s->_segMan->strcpy_(data, "500");
	} else if (setting == "cpu") {
		// We always return the fastest CPU setting that CPUID can detect
		// (i.e. 586).
		s->_segMan->strcpy_(data, "586");
	} else if (setting == "cpuspeed") {
		s->_segMan->strcpy_(data, "500");
	} else if (setting == "language") {
		Common::String languageId = Common::String::format("%d", g_sci->getSciLanguage());
		s->_segMan->strcpy_(data, languageId.c_str());
	} else if (setting == "torindebug") {
		// Used to enable the debug mode in Torin's Passage (French).
		// If true, the debug mode is enabled.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "leakdump") {
		// An unknown setting in LSL7. Likely used for debugging.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "startroom") {
		// Debug setting in LSL7, specifies the room to start from.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "game") {
		// Hoyle 5 startup, specifies the number of the game to start.
		if (g_sci->getGameId() == GID_HOYLE5 &&
			!g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 100)) &&
			g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 700))) {
			// Special case for Hoyle 5 Bridge: only one game is included (Bridge),
			// so mimic the setting in 700.cfg and set the starting room number to 700.
			s->_segMan->strcpy_(data, "700");
		} else {
			s->_segMan->strcpy_(data, "");
		}
	} else if (setting == "laptop") {
		// Hoyle 5 startup.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "jumpto") {
		// Hoyle 5 startup.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "klonchtsee") {
		// Hoyle 5 - starting Solitaire.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "klonchtarr") {
		// Hoyle 5 - starting Solitaire.
		s->_segMan->strcpy_(data, "");
	} else if (setting == "deflang") {
		// MGDX 4-language startup.
		s->_segMan->strcpy_(data, "");
	} else {
		error("GetConfig: Unknown configuration setting %s", setting.c_str());
	}

	return argv[1];
}

// Likely modelled after the Windows 3.1 function GetPrivateProfileInt:
// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofileint
reg_t kGetSierraProfileInt(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getPlatform() != Common::kPlatformWindows) {
		return s->r_acc;
	}

	Common::String category = s->_segMan->getString(argv[0]);	// always "config"
	category.toLowercase();
	Common::String setting = s->_segMan->getString(argv[1]);
	setting.toLowercase();
	// The third parameter is the default value returned if the configuration key is missing

	if (category == "config" && setting == "videospeed") {
		// We return the same fake value for videospeed as with kGetConfig
		return make_reg(0, 500);
	}

	warning("kGetSierraProfileInt: Returning default value %d for unknown setting %s.%s", argv[2].toSint16(), category.c_str(), setting.c_str());
	return argv[2];
}

reg_t kGetWindowsOption(EngineState *s, int argc, reg_t *argv) {
	uint16 windowsOption = argv[0].toUint16();
	switch (windowsOption) {
	case 0:
		// Title bar on/off in Phantasmagoria, we return 0 (off)
		return NULL_REG;
	default:
		warning("GetWindowsOption: Unknown option %d", windowsOption);
		return NULL_REG;
	}
}

extern Common::String format(const Common::String &source, int argc, const reg_t *argv);

reg_t kPrintDebug(EngineState *s, int argc, reg_t *argv) {
	const Common::String debugString = s->_segMan->getString(argv[0]);
	debugC(kDebugLevelGame, "%s", format(debugString, argc - 1, argv + 1).c_str());
	return s->r_acc;
}

#endif

// kIconBar is really a subop of kMacPlatform for SCI1.1 Mac
reg_t kIconBar(EngineState *s, int argc, reg_t *argv) {
	// Mac versions use their own tertiary platform functions
	// to handle the outside-of-the-screen icon bar.

	// QFG1 Mac calls this function to load the Mac icon bar (of which
	// the resources do exist), but the game completely ignores it and
	// uses the standard icon bar for the game. We do the same.
	if (!g_sci->hasMacIconBar())
		return NULL_REG;

	switch (argv[0].toUint16()) {
	case 0: // InitIconBar
		g_sci->_gfxMacIconBar->initIcons(argv[1].toUint16(), &argv[2]);
		break;
	case 1: // DisposeIconBar
		warning("kIconBar(Dispose)");
		break;
	case 2: // EnableIconBar (-1 = all)
		debug(0, "kIconBar(Enable, %i)", argv[1].toSint16());
		g_sci->_gfxMacIconBar->setIconEnabled(argv[1].toSint16(), true);
		break;
	case 3: // DisableIconBar (-1 = all)
		debug(0, "kIconBar(Disable, %i)", argv[1].toSint16());
		g_sci->_gfxMacIconBar->setIconEnabled(argv[1].toSint16(), false);
		break;
	case 4: // SetIconBarIcon
		debug(0, "kIconBar(SetIcon, %d, %d)", argv[1].toUint16(), argv[2].toUint16());
		if (argv[2].toSint16() == -1)
			g_sci->_gfxMacIconBar->setInventoryIcon(argv[2].toSint16());
		break;
	default:
		error("Unknown kIconBar(%d)", argv[0].toUint16());
	}

	g_sci->_gfxMacIconBar->drawIcons();

	return NULL_REG;
}

// kMacPlatform is really a subop of kPlatform for SCI1.1+ Mac
reg_t kMacPlatform(EngineState *s, int argc, reg_t *argv) {
	// Mac versions use their own secondary platform functions
	// to do various things. Why didn't they just declare a new
	// kernel function?

	switch (argv[0].toUint16()) {
	case 0:
		// Subop 0 has changed a few times
		// In SCI1, its usage is still unknown
		// In SCI1.1, it's NOP
		if (getSciVersion() != SCI_VERSION_1_1)
			warning("Unknown SCI1 kMacPlatform(0) call");
		break;
	case 4: // Handle icon bar code
		return kIconBar(s, argc - 1, argv + 1);
	case 7: // Unknown, but always return -1
		return SIGNAL_REG;
	case 1:	// Unknown, calls QuickDraw region functions (KQ5, QFG1VGA, Dr. Brain 1)
		break;	// removed warning, as it produces a lot of spam in the console
	case 2: // Unknown, "UseNextWaitEvent" (Various)
	case 3: // Unknown, "ProcessOpenDocuments" (Various)
	case 5: // Unknown
	case 6: // Unknown, menu-related (Unused?)
		warning("Unhandled kMacPlatform(%d)", argv[0].toUint16());
		break;
	default:
		error("Unknown kMacPlatform(%d)", argv[0].toUint16());
	}

	return s->r_acc;
}

#ifdef ENABLE_SCI32
// kMacKq7InitializeSave is a subop of kMacPlatform32.
//  KQ7 Mac would display a native Save dialog with the prompt "Who's game?"
//  and store the result in a global variable inside the interpreter
//  for subsequent calls to kMacKq7SaveGame.
reg_t kMacKq7InitializeSave(EngineState *s) {
	s->_kq7MacSaveGameId = g_sci->_guestAdditions->runSaveRestore(true, s->_kq7MacSaveGameDescription);
	s->_kq7MacSaveGameId = shiftSciToScummVMSaveId(s->_kq7MacSaveGameId);
	return (s->_kq7MacSaveGameId != -1) ? TRUE_REG : NULL_REG;
}

// kMacKq7SaveGame is a subop of kMacPlatform32.
//  Saves the game using the current save id and description that's set
//  when initializing or restoring a saved game.
reg_t kMacKq7SaveGame(EngineState *s) {
	if (s->_kq7MacSaveGameId == -1) {
		error("kMacKq7SaveGame: save game hasn't been initialized");
	}

	const reg_t version = s->variables[VAR_GLOBAL][kGlobalVarVersionNew];
	const Common::String versionString = s->_segMan->getString(version);
	if (gamestate_save(s, s->_kq7MacSaveGameId, s->_kq7MacSaveGameDescription, versionString)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

// kMacKq7RestoreGame is a subop of kMacPlatform32.
//  KQ7 Mac would display a native Open dialog with the prompt "Who's game?"
//  and store the result in a global variable inside the interpreter to
//  use in subsequent calls to kMacKq7SaveGame before restoring.
reg_t kMacKq7RestoreGame(EngineState *s) {
	s->_kq7MacSaveGameId = g_sci->_guestAdditions->runSaveRestore(false, s->_kq7MacSaveGameDescription);
	s->_kq7MacSaveGameId = shiftSciToScummVMSaveId(s->_kq7MacSaveGameId);
	if (s->_kq7MacSaveGameId == -1) {
		return NULL_REG;
	}

	// gamestate_restore() resets s->_kq7MacSaveGameId and
	//  s->_kq7MacSaveGameDescription so save and restore them.
	int kq7MacSaveGameId = s->_kq7MacSaveGameId;
	Common::String kq7MacSaveGameDescription = s->_kq7MacSaveGameDescription;
	bool success = gamestate_restore(s, s->_kq7MacSaveGameId);
	s->_kq7MacSaveGameId = kq7MacSaveGameId;
	s->_kq7MacSaveGameDescription = kq7MacSaveGameDescription;

	return success ? TRUE_REG : NULL_REG;
}

// kMacInitializeSave is a subop of kMacPlatform32.
reg_t kMacInitializeSave(EngineState *s, int argc, reg_t *argv) {
	return TRUE_REG; // NULL_REG if i/o errors
}

// kMacSaveGame is a subop of kMacPlatform32.
reg_t kMacSaveGame(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelFrameOut(true); // see kSaveGame32

	const int saveId = shiftSciToScummVMSaveId(argv[1].toUint16());
	const Common::String description = s->_segMan->getString(argv[2]);
	const reg_t version = s->variables[VAR_GLOBAL][kGlobalVarVersionNew];
	const Common::String versionString = s->_segMan->getString(version);
	if (gamestate_save(s, saveId, description, versionString)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

// kMacRestoreGame is a subop of kMacPlatform32.
reg_t kMacRestoreGame(EngineState *s, int argc, reg_t *argv) {
	const int saveId = shiftSciToScummVMSaveId(argv[1].toUint16());
	if (gamestate_restore(s, saveId)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

reg_t kMacPlatform32(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0: // build cursor view map
		g_sci->_gfxCursor32->setMacCursorRemapList(argc - 1, argv + 1);
		return s->r_acc;

	case 1: // compact/purge mac memory
	case 2: // hands-off/hands-on for mac menus
		return s->r_acc;

	// Subops 3-5 are used for custom saving and restoring but they
	//  changed completely between each game that uses them.
	//
	//  KQ7:        3-5 with no parameters
	//  Shivers:    3-5 with parameters
	//  Lighthouse: 3 with sub-subops: -1, 0, and 1 (TODO)
	case 3:
		if (argc == 1) {
			return kMacKq7InitializeSave(s);
		} else if (argc == 3) {
			return kMacInitializeSave(s, argc - 1, argv + 1);
		}
		break;
	case 4:
		if (argc == 1) {
			return kMacKq7SaveGame(s);
		} else if (argc == 4) {
			return kMacSaveGame(s, argc - 1, argv + 1);
		}
		break;
	case 5:
		if (argc == 1) {
			return kMacKq7RestoreGame(s);
		} else if (argc == 3) {
			return kMacRestoreGame(s, argc - 1, argv + 1);
		}
		break;

	// Subops 6-11 are used for saving and restoring by Mother Goose only.
	case 6:
		return kMacInitializeSave(s, argc - 1, argv + 1);
	case 7:
		return kMacSaveGame(s, argc - 1, argv + 1);
	case 8:
		return kMacRestoreGame(s, argc - 1, argv + 1);
	case 9:
		return kGetSaveFiles32(s, argc - 1, argv + 1);
	case 10:
		return kMakeSaveCatName(s, argc - 1, argv + 1);
	case 11:
		return kMakeSaveFileName(s, argc - 1, argv + 1);

	// Phantasmagoria volume
	case 12:
		return g_sci->_soundCmd->kDoSoundMasterVolume(s, argc - 1, argv + 1);

	default:
		break;
	}

	error("Unknown kMacPlatform32(%d)", argv[0].toUint16());
	return s->r_acc;
}
#endif

enum kSciPlatforms {
	kSciPlatformMacintosh = 0,
	kSciPlatformDOS = 1,
	kSciPlatformWindows = 2
};

reg_t kPlatform(EngineState *s, int argc, reg_t *argv) {
	enum Operation {
		kPlatformUnknown        = 0,
		kPlatformGetPlatform    = 4,
		kPlatformIsSmallWindow  = 5,
		kPlatformIsHiRes        = 6,
		kPlatformWin311OrHigher = 7
	};

	if (argc == 0) {
		// This is called in KQ5CD with no parameters, where it seems to do some
		// graphics driver check. This kernel function didn't have subfunctions
		// then. If 0 is returned, the game functions normally, otherwise all
		// the animations show up like a slideshow (e.g. in the intro). So we
		// return 0.
		return NULL_REG;
	}

	// treat KQ6 DOS with hires graphics as Windows so that hires graphics are enabled
	bool isWindows = (g_sci->getPlatform() == Common::kPlatformWindows) ||
	                 (g_sci->getGameId() == GID_KQ6 && g_sci->getPlatform() == Common::kPlatformDOS && g_sci->useHiresGraphics());

	uint16 operation = argv[0].toUint16();
	switch (operation) {
	case kPlatformUnknown:
		// For Mac versions, kPlatform(0) with other args has more functionality. Otherwise, fall through.
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && argc > 1)
			return kMacPlatform(s, argc - 1, argv + 1);
		// fall through
	case kPlatformGetPlatform:
		if (isWindows)
			return make_reg(0, kSciPlatformWindows);
		else if (g_sci->getPlatform() == Common::kPlatformMacintosh)
			return make_reg(0, kSciPlatformMacintosh);
		else
			return make_reg(0, kSciPlatformDOS);
	case kPlatformIsSmallWindow:
		return make_reg(0, !g_sci->_gfxScreen->gfxDriver()->supportsHiResGraphics());
	case kPlatformIsHiRes:
	case kPlatformWin311OrHigher:
		return make_reg(0, isWindows);
	default:
		error("Unsupported kPlatform operation %d", operation);
	}

	return NULL_REG;
}

extern int showScummVMDialog(const Common::U32String &message, const Common::U32String &altButton = Common::U32String(), bool alignCenter = true);

#ifdef ENABLE_SCI32
reg_t kPlatform32(EngineState *s, int argc, reg_t *argv) {
	enum Operation {
		kGetPlatform   = 0,
		kGetCDSpeed    = 1,
		kGetColorDepth = 2,
		kGetCDDrive    = 3
	};

	Operation operation;
	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		if (argc == 0 || argv[0].toSint16() == 0) {
			operation = kGetPlatform;
		} else {
			return NULL_REG;
		}
	} else {
		operation = argc > 0 ? (Operation)argv[0].toSint16() : kGetPlatform;
	}

	switch (operation) {
	case kGetPlatform:
		switch (g_sci->getPlatform()) {
		case Common::kPlatformDOS:
			return make_reg(0, kSciPlatformDOS);
		case Common::kPlatformWindows:
			return make_reg(0, kSciPlatformWindows);
		case Common::kPlatformMacintosh:
			// For Mac versions, kPlatform(0) with other args has more functionality
			if (argc > 1) {
				return kMacPlatform32(s, argc - 1, argv + 1);
			} else {
				// SCI32 Mac claims to be DOS. GK1 depends on this in order to play its
				//  view-based slideshow movies. It appears that Sierra opted to change
				//  this return value instead of updating the game scripts for Mac.
				return make_reg(0, kSciPlatformDOS);
			}
		default:
			error("Unknown platform %d", g_sci->getPlatform());
		}
	case kGetColorDepth:
		if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
			return make_reg(0, /* 16-bit color */ 3);
		} else {
			return make_reg(0, /* 256 color */ 2);
		}
	case kGetCDSpeed:
		// The value `4` comes from Rama DOS resource.cfg installed in DOSBox,
		// and seems to correspond to the highest expected CD speed value
		return make_reg(0, 4);
	case kGetCDDrive:
	default:
		return NULL_REG;
	}
}

reg_t kWebConnect(EngineState *s, int argc, reg_t *argv) {
	const Common::String baseUrl = "https://web.archive.org/web/1996/";
	const Common::String gameUrl = argc > 0 ? s->_segMan->getString(argv[0]) : "http://www.sierra.com";
	return make_reg(0, g_system->openUrl(baseUrl + gameUrl));
}

reg_t kWinExec(EngineState *s, int argc, reg_t *argv) {
	return NULL_REG;
}

reg_t kWinDLL(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Common::String dllName = s->_segMan->getString(argv[1]);

	switch (operation) {
	case 0:	// load DLL
		if (dllName == "PENGIN16.DLL")
			showScummVMDialog(_("The Poker logic is hardcoded in an external DLL, and is not implemented yet. There exists some dummy logic for now, where opponent actions are chosen randomly"));

		// This is originally a call to LoadLibrary() and to the Watcom function GetIndirectFunctionHandle
		return make_reg(0, 1000);	// fake ID for loaded DLL, normally returned from Windows LoadLibrary()
	case 1: // free DLL
		// In the original, FreeLibrary() was called here for the loaded DLL
		return TRUE_REG;
	case 2:	// call DLL function
		if (dllName == "PENGIN16.DLL") {
			// Poker engine logic for Hoyle 5
			// This is originally a call to the Watcom function InvokeIndirectFunction()
			SciArray *data = s->_segMan->lookupArray(argv[2]);
			return hoyle5PokerEngine(data);
		} else {
			error("kWinDLL: Unknown DLL to invoke: %s", dllName.c_str());
			return NULL_REG;
		}
	default:
		return NULL_REG;
	}
}

#endif

reg_t kKawaHacks(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0: { // DoAlert
		showScummVMDialog(s->_segMan->getString(argv[1]));
		return NULL_REG;
	}
	case 1: { // ZaWarudo
		// Invert the color palette for the specified range.
		uint16 from = argv[1].toUint16();
		uint16 to = argv[2].toUint16();
		Palette pal = g_sci->_gfxPalette16->_sysPalette;
		for (uint16 i = from; i <= to; i++)
		{
			pal.colors[i].r = 255 - pal.colors[i].r;
			pal.colors[i].g = 255 - pal.colors[i].g;
			pal.colors[i].b = 255 - pal.colors[i].b;
		}
		g_sci->_gfxPalette16->set(&pal, true);
 		return NULL_REG;
	}
 	case 2: // SetTitleColors
		// Unused, would change the colors for plain windows' title bars.
		return NULL_REG;
	case 3: // IsDebug
 		// Return 1 if running with an internal debugger, 2 if we have AddMenu support, 3 if both.
		return make_reg(0, 3);
	default:
		break;
	}
	return NULL_REG;
}
reg_t kKawaDbugStr(EngineState *s, int argc, reg_t *argv)
{
	debug("%s", Common::String::format(s->_segMan->getString(argv[0]).c_str(), argc - 1, argv + 1).c_str());
	return NULL_REG;
}

reg_t kEmpty(EngineState *s, int argc, reg_t *argv) {
	// Placeholder for empty kernel functions which are still called from the
	// engine scripts (like the empty kSetSynonyms function in SCI1.1). This
	// differs from dummy functions because it doesn't throw an error when it
	// is called.
	return s->r_acc;
}

reg_t kStub(EngineState *s, int argc, reg_t *argv) {
	Kernel *kernel = g_sci->getKernel();
	int kernelCallNr = -1;
	int kernelSubCallNr = -1;

	Common::List<ExecStack>::const_iterator callIterator = s->_executionStack.end();
	if (callIterator != s->_executionStack.begin()) {
		callIterator--;
		ExecStack lastCall = *callIterator;
		kernelCallNr = lastCall.debugKernelFunction;
		kernelSubCallNr = lastCall.debugKernelSubFunction;
	}

	Common::String warningMsg;
	if (kernelSubCallNr == -1) {
		warningMsg = "Dummy function k" + kernel->getKernelName(kernelCallNr) +
		             Common::String::format("[%x]", kernelCallNr);
	} else {
		warningMsg = "Dummy function k" + kernel->getKernelName(kernelCallNr, kernelSubCallNr) +
		             Common::String::format("[%x:%x]", kernelCallNr, kernelSubCallNr);

	}

	warningMsg += " invoked. Params: " +
	              Common::String::format("%d", argc) + " (";

	for (int i = 0; i < argc; i++) {
		warningMsg +=  Common::String::format("%04x:%04x", PRINT_REG(argv[i]));
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
