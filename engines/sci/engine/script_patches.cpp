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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"
#include "sci/engine/features.h"
#include "sci/engine/script_patches.h"
#ifdef ENABLE_SCI32
#include "sci/engine/guest_additions.h"
#endif

#include "common/config-manager.h"
#include "common/util.h"

namespace Sci {

// IMPORTANT:
// every patch entry needs the following:
//  - script number (pretty obvious)
//
//  - apply count
//     specifies the number of times a patch is supposed to get applied.
//     Most of the time, it should be 1.
//
//  - magicDWORD + magicOffset
//     please ALWAYS put 0 for those two. Both will get filled out at runtime by the patcher.
//
//  - signature data (is used to identify certain script code, that needs patching)
//     every signature needs to contain SIG_MAGICDWORD once.
//      The following 4 bytes after SIG_MAGICDWORD - which don't have to be fixed, you may for example
//      use SIG_SELECTOR16, will get used to quickly search for a partly match before verifying that
//      the whole signature actually matches. If it's not included, the script patch
er will error() out
//      right when loading up the game.
//     If selector-IDs are included, please use SIG_SELECTOR16 + SIG_SELECTOR8 [1]. Simply
//      specify the selector that way, so that the patcher will search for the specific
//      selector instead of looking for a hardcoded value. Selectors may not be the same
//      between game versions.
//     For UINT16s either use SIG_UINT16 or SIG_SELECTOR16.
//      Macintosh versions of SCI games are using BE ordering instead of LE since SCI1.1 for UINT16s in scripts
//      By using those 2 commands, it's possible to make patches work for PC and Mac versions of the same game.
//     You may also skip bytes by using the SIG_ADDTOOFFSET command
//     Every signature data needs to get terminated using SIGNATURE_END
//
//  - patch data (is used for actually patching scripts)
//     When a match is found, the patch data will get applied.
//     Patch data is similar to signature data. Just use PATCH_SELECTOR16 + PATCH_SELECTOR8 [1]
//      for patching in selectors.
//     There are also patch specific commands.
//     Those are PATCH_GETORIGINALBYTE, which fetches a byte from the original script
//      and PATCH_GETORIGINALBYTEADJUST, which does the same but gets a second value
//      from the uint16 array and uses that value to adjust the original byte.
//     Every patch data needs to get terminated using PATCH_END
//
//  - and please always add a comment about why the patch was done and what's causing issues.
//     If possible make sure, that the patch works on localized (or just different) game versions
//      as well in case those need patching too.
//
// [1] - selectors need to get specified in selectorTable[] and ScriptPatcherSelectors-enum
//        before they can get used using the SIG_SELECTORx and PATCH_SELECTORx commands.
//        You have to use the exact same order in both the table and the enum, otherwise
//        it won't work.
//        ATTENTION: selectors will only work here, when the
y are also in SelectorCache (selector.h)

static const char *const selectorNameTable[] = {
	"cycles",       // system selector
	"seconds",      // system selector
	"init",         // system selector
	"dispose",      // system selector
	"new",          // system selector
	"curEvent",     // system selector
	"disable",      // system selector
	"doit",         // system selector
	"show",         // system selector
	"x",            // system selector
	"cel",          // system selector
	"setMotion",    // system selector
	"overlay",      // system selector
	"setPri",       // system selector - for setting priority
	"play",         // system selector
	"number",       // system selector
	"setScript",    // system selector
	"setCycle",     // system selector
	"setStep",      // system selector
	"cycleSpeed",   // system selector
	"moveSpeed",    // system selector
	"handsOff",     // system selector
	"handsOn",      // system selector
	"type",         // system selector
	"client",       // system selector
	"state",        // system selector
	"illegalBits",  // system selector
	"localize",     // Freddy Pharkas
	"roomFlags",    // Iceman
	"put",          // Police Quest 1 VGA
	"approachVerbs", // Police Quest 1 VGA
	"newRoom",      // Police Quest 3, GK1
	"register",     // Quest For Glory 1 EGA, QFG4
	"changeState",  // Quest For Glory 1 VGA, QFG4
	"hide",         // Quest For Glory 1 VGA, QFG4
	"say",          // Quest For Glory 1 VGA, QFG4
	"script",       // Quest For Glory 1 VGA
	"isEmpty",      // Quest For Glory 3
	"solvePuzzle",  // Quest For Glory 3
	"curIcon",      // Quest For Glory 3, QFG4
	"curInvIcon",   // Quest For Glory 3, QFG4
	"edgeHit",      // King's Quest 5
	"startText",    // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"startAudio",   // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"modNum",       // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"handle",       // King's Quest 6 / Laura Bow 2 / RAMA
	"add",   
       // King's Quest 6
	"givePoints",   // King's Quest 6
	"has",          // King's Quest 6, GK1
	"modeless",     // King's Quest 6 CD
	"message",      // King's Quest 6
	"forceUpd",     // Police Quest 3
	"cycler",       // Space Quest 4 / system selector
	"setCel",       // Space Quest 4, Phant2, GK1
	"addToPic",     // Space Quest 4
	"stop",         // Space Quest 4
	"canControl",   // Space Quest 4
	"looper",       // Space Quest 4
	"nMsgType",     // Space Quest 4
	"doVerb",       // Space Quest 4
	"setRegions",   // Space Quest 4
	"cursor",       // Space Quest 5
	"showSelf",     // Space Quest 5
	"claimed",      // Space Quest 5, QFG4
	"setCursor",    // Space Quest 5, QFG4
	"setSpeed",     // Space Quest 5, QFG4
	"loop",         // Laura Bow 1 Colonel's Bequest, QFG4
	"setLoop",      // Laura Bow 1 Colonel's Bequest, QFG4
	"ignoreActors", // Laura Bow 1 Colonel's Bequest
	"saveCursor",   // Laura Bow 2 CD
	"setVol",       // Laura Bow 2 CD
	"at",           // Longbow, QFG4
	"owner",        // Longbow, QFG4
	"fade",         // Longbow, Shivers
	"enable",       // Longbow, SQ6
	"alterEgo",     // LSL5
	"ticks",        // LSL6
	"normalize",    // Pepper, GK1, QFG4
	"delete",       // EcoQuest 1
	"size",         // EcoQuest 1
	"signal",       // EcoQuest 1, GK1
	"obstacles",    // EcoQuest 1
	"handleEvent",  // EcoQuest 2, Shivers
	"view",         // King's Quest 4, RAMA benchmarking, GK1, QFG4
	"tWindow",      // Camelot
#ifdef ENABLE_SCI32
	"newWith",      // SCI2 array script
	"inset",        // GK1
	"posn",         // GK1, Phant2, QFG4
	"printLang",    // GK2
	"test",         // Torin
	"get",          // Torin, GK1
	"setReal",      // GK1
	"set",          // Torin
	"clear",        // Torin
	"masterVolume", // SCI2 master volume reset
	"data",         // Phant2, QFG4
	"format",       // Phant2
	"mouseMoved",   // Phant2
	"setSize",      // Phant2
	"iconV",        // Phant2
	"track",        // Phant2
	"update",       // Phant2
	"xOff",         // Phant2
	"a
ddRespondVerb",// KQ7
	"eachElementDo",// KQ7
	"fore",         // KQ7
	"back",         // KQ7
	"font",         // KQ7
	"setHeading",   // KQ7
	"newPic",       // Lighthouse
	"start",        // Lighthouse
	"setScale",     // LSL6hires, QFG4
	"setScaler",    // LSL6hires, QFG4
	"showTitle",    // LSL6hires
	"name",         // LSL6hires
	"completed",    // LSL6hires
	"endType",      // LSL6hires
	"oSpecialSync", // LSL7
	"readWord",     // LSL7, Phant1, Torin
	"points",       // PQ4
	"select",       // PQ4
	"saveFilePtr",  // RAMA
	"priority",     // RAMA
	"plane",        // RAMA
	"getSubscriberObj", // RAMA
	"advance",      // QFG4
	"advanceCurIcon", // QFG4
	"amount",       // QFG4
	"cue",          // QFG4
	"drop",         // QFG4
	"getCursor",    // QFG4
	"heading",      // QFG4
	"retreat",      // QFG4
	"sayMessage",   // QFG4
	"setLooper",    // QFG4
	"use",          // QFG4
	"useStamina",   // QFG4
	"value",        // QFG4
	"setupExit",    // SQ6
	"vol",          // SQ6
	"walkIconItem", // SQ6
#endif
	nullptr
};

enum ScriptPatcherSelectors {
	SELECTOR_cycles = 0,
	SELECTOR_seconds,
	SELECTOR_init,
	SELECTOR_dispose,
	SELECTOR_new,
	SELECTOR_curEvent,
	SELECTOR_disable,
	SELECTOR_doit,
	SELECTOR_show,
	SELECTOR_x,
	SELECTOR_cel,
	SELECTOR_setMotion,
	SELECTOR_overlay,
	SELECTOR_setPri,
	SELECTOR_play,
	SELECTOR_number,
	SELECTOR_setScript,
	SELECTOR_setCycle,
	SELECTOR_setStep,
	SELECTOR_cycleSpeed,
	SELECTOR_moveSpeed,
	SELECTOR_handsOff,
	SELECTOR_handsOn,
	SELECTOR_type,
	SELECTOR_client,
	SELECTOR_state,
	SELECTOR_illegalBits,
	SELECTOR_localize,
	SELECTOR_roomFlags,
	SELECTOR_put,
	SELECTOR_approachVerbs,
	SELECTOR_newRoom,
	SELECTOR_register,
	SELECTOR_changeState,
	SELECTOR_hide,
	SELECTOR_say,
	SELECTOR_script,
	SELECTOR_isEmpty,
	SELECTOR_solvePuzzle,
	SELECTOR_curIcon,
	SELECTOR_curInvIcon,
	SELECTOR_edgeHit,
	SELECTOR_startText,
	SELECTOR_startAudio,
	SELECTOR_modNum,
	SELECTOR_handle,
	SELECTOR_add,
	SELECTOR_givePoints,
	SELECTOR_has,
	SELECTOR_model
ess,
	SELECTOR_message,
	SELECTOR_forceUpd,
	SELECTOR_cycler,
	SELECTOR_setCel,
	SELECTOR_addToPic,
	SELECTOR_stop,
	SELECTOR_canControl,
	SELECTOR_looper,
	SELECTOR_nMsgType,
	SELECTOR_doVerb,
	SELECTOR_setRegions,
	SELECTOR_cursor,
	SELECTOR_showSelf,
	SELECTOR_claimed,
	SELECTOR_setCursor,
	SELECTOR_setSpeed,
	SELECTOR_loop,
	SELECTOR_setLoop,
	SELECTOR_ignoreActors,
	SELECTOR_saveCursor,
	SELECTOR_setVol,
	SELECTOR_at,
	SELECTOR_owner,
	SELECTOR_fade,
	SELECTOR_enable,
	SELECTOR_alterEgo,
	SELECTOR_ticks,
	SELECTOR_normalize,
	SELECTOR_delete,
	SELECTOR_size,
	SELECTOR_signal,
	SELECTOR_obstacles,
	SELECTOR_handleEvent,
	SELECTOR_view,
	SELECTOR_tWindow
#ifdef ENABLE_SCI32
	,
	SELECTOR_newWith,
	SELECTOR_inset,
	SELECTOR_posn,
	SELECTOR_printLang,
	SELECTOR_test,
	SELECTOR_get,
	SELECTOR_setReal,
	SELECTOR_set,
	SELECTOR_clear,
	SELECTOR_masterVolume,
	SELECTOR_data,
	SELECTOR_format,
	SELECTOR_mouseMoved,
	SELECTOR_setSize,
	SELECTOR_iconV,
	SELECTOR_track,
	SELECTOR_update,
	SELECTOR_xOff,
	SELECTOR_addRespondVerb,
	SELECTOR_eachElementDo,
	SELECTOR_fore,
	SELECTOR_back,
	SELECTOR_font,
	SELECTOR_setHeading,
	SELECTOR_newPic,
	SELECTOR_start,
	SELECTOR_setScale,
	SELECTOR_setScaler,
	SELECTOR_showTitle,
	SELECTOR_name,
	SELECTOR_completed,
	SELECTOR_endType,
	SELECTOR_oSpecialSync,
	SELECTOR_readWord,
	SELECTOR_points,
	SELECTOR_select,
	SELECTOR_saveFilePtr,
	SELECTOR_priority,
	SELECTOR_plane,
	SELECTOR_getSubscriberObj,
	SELECTOR_advance,
	SELECTOR_advanceCurIcon,
	SELECTOR_amount,
	SELECTOR_cue,
	SELECTOR_drop,
	SELECTOR_getCursor,
	SELECTOR_heading,
	SELECTOR_retreat,
	SELECTOR_sayMessage,
	SELECTOR_setLooper,
	SELECTOR_use,
	SELECTOR_useStamina,
	SELECTOR_value,
	SELECTOR_setupExit,
	SELECTOR_vol,
	SELECTOR_walkIconItem
#endif
};

#ifdef ENABLE_SCI32
// It is not possible to change the directory for ScummVM save games, so disable
// the "change directory" button in the standard save dialogue
static const uint16 sci2ChangeDirSignature[] = {
	0x72, SIG_ADD
TOOFFSET(+2), // lofsa changeDirI
	0x4a, SIG_UINT16(0x0004),  // send 4
	SIG_MAGICDWORD,
	0x36,                      // push
	0x35, 0xf7,                // ldi $f7
	0x12,                      // and
	0x36,                      // push
	SIG_END
};

static const uint16 sci2ChangeDirPatch[] = {
	PATCH_ADDTOOFFSET(+3),    // lofsa changeDirI
	PATCH_ADDTOOFFSET(+3),    // send 4
	PATCH_ADDTOOFFSET(+1),    // push
	0x35, 0x00,               // ldi 0
	PATCH_END
};

// Save game script hardcodes the maximum number of save games to 20, but
// this is an artificial constraint that does not apply to ScummVM
static const uint16 sci2NumSavesSignature1[] = {
	SIG_MAGICDWORD,
	0x8b, 0x02,                    // lsl local[2]
	0x35, 0x14,                    // ldi 20
	0x22,                          // lt?
	SIG_END
};

static const uint16 sci2NumSavesPatch1[] = {
	PATCH_ADDTOOFFSET(+2),         // lsl local[2]
	0x35, 0x63,                    // ldi 99
	PATCH_END
};

static const uint16 sci2NumSavesSignature2[] = {
	SIG_MAGICDWORD,
	0x8b, 0x02,                    // lsl local[2]
	0x35, 0x14,                    // ldi 20
	0x1a,                          // eq?
	SIG_END
};

static const uint16 sci2NumSavesPatch2[] = {
	PATCH_ADDTOOFFSET(+2),         // lsl local[2]
	0x35, 0x63,                    // ldi 99
	PATCH_END
};

// Phantasmagoria & SQ6 try to initialize the first entry of an int16 array
// using an empty string, which is not valid (it should be a number)
static const uint16 sci21IntArraySignature[] = {
	0x38, SIG_SELECTOR16(newWith), // pushi newWith
	0x7a,                          // push2
	0x39, 0x04,                    // pushi $4
	0x72, SIG_ADDTOOFFSET(+2),     // lofsa string ""
	SIG_MAGICDWORD,
	0x36,                          // push
	0x51, 0x0b,                    // class IntArray
	0x4a, 0x08,                    // send $8
	SIG_END
};

static const uint16 sci21IntArrayPatch[] = {
	PATCH_ADDTOOFFSET(+6),      // push $b9; push2; pushi $4
	0x76,                       // pus
h0
	0x34, PATCH_UINT16(0x0001), // ldi 0001 (waste bytes)
	PATCH_END
};

// Most SCI32 games have a video performance benchmarking loop at the
// beginning of the game. Running this benchmark with calls to
// `OSystem::updateScreen` will often cause the benchmark to return a low value,
// which causes games to disable some visual effects. Running without calls to
// `OSystem::updateScreen` on any reasonably modern CPU will cause the benchmark
// to overflow, leading to randomly disabled effects. This patch changes the
// benchmarking code to always return the game's maximum speed value.
//
// The speed test function in script 64908 changes between games and is compiled
//  differently between versions of the same game, so to generically patch this
//  we replace the call-sites with zero, which is the fastest result. RAMA's
//  speed test function reverses this and returns a high value for fast machines
//  so it requires a different patch.
//
// Applies to: GK1, PQ4, LSL6hires, Phant1, Shivers, SQ6, RAMA
static const uint16 sci2BenchmarkSignature[] = {
	SIG_MAGICDWORD,
	0x76,                       // push0
	0x46, SIG_UINT16(0xfd8c),   // calle proc64908_0 [ speed test function ]
	      SIG_UINT16(0x0000),
	      SIG_UINT16(0x0000),
	SIG_END
};

static const uint16 sci2BenchmarkPatch[] = {
	0x35, 0x00,                 // ldi 00 [ fastest speed test result ]
	0x33, 0x04,                 // jmp 04
	PATCH_END
};

static const uint16 sci2BenchmarkReversePatch[] = {
	0x34, PATCH_UINT16(0x2710), // ldi 10000 [ fastest speed test result for RAMA ]
	0x33, 0x03,                 // jmp 03
	PATCH_END
};

// The init code that runs in many SCI32 games unconditionally resets the music
// volume, but the game should always use the volume stored in ScummVM.
// Applies to at least: LSL6hires, MGDX, PQ:SWAT, QFG4
static const uint16 sci2VolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(masterVolume), // pushi masterVolume
	0x78,                               // push1

	0x39, SIG_ADDTOOFFSET(+1),          // pushi [default volume]
	0x81, 0x01,                         // lag global[1]
	0x4a, SIG_UINT16(0x0006),           // send 6
	SIG_END
};

static const uint16 sci2VolumeResetPatch[] = {
	0x32, PATCH_UINT16(0x0008),         // jmp 8 [past volume reset]
	PATCH_END
};

// At least Gabriel Knight 1 and Police Quest 4 floppy have a broken Str::strip inside script 64918.
// The code never passes over the actual string to kStringTrim, so that would not work and also trigger
// a signature mismatch.
// Localized version of Police Quest 4 were also affected.
// Gabriel Knight although affected doesn't seem to ever call the code, so there is no reason to patch it.
// Police Quest 4 CD got this fixed.
static const uint16 sci2BrokenStrStripSignature[] = {
	SIG_MAGICDWORD,
	0x85, 0x06,                         // lat temp[6]
	0x31, 0x10,                         // bnt [jump to code that passes 2 parameters]
	0x38, SIG_UINT16(0x00c2),           // pushi 00c2 (callKernel)
	0x38, SIG_UINT16(0x0003),           // pushi 03
	0x39, 0x0e,                         // pushi 0e
	0x8d, 0x0b,                         // lst temp[0b]
	0x36,                               // push
	0x54, SIG_UINT16(0x000a),           // self 0a
	0x33, 0x0b,                         // jmp [ret]
	// 2 parameter code
	0x38, SIG_UINT16(0x00c2),           // pushi 00c2
	0x7a,                               // push2
	0x39, 0x0e,                         // pushi 0e
	0x8d, 0x0b,                         // lst temp[0b]
	0x54, SIG_UINT16(0x0008),           // self 08
	SIG_END
};

static const uint16 sci2BrokenStrStripPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x85, 0x06,                         // lat temp[6] (once more]
	PATCH_ADDTOOFFSET(+3),              // jump over pushi callKernel
	0x39, 0x04,                         // pushi 04
	0x39, 0x0e,                         // pushi 0e
	// Attention: data is 0x14 in PQ4 CD, in floppy it's 0x12
	0x67, 0x12,                         // pTos data (pas
s actual data)
	0x8d, 0x0b,                         // lst temp[0b]
	0x36,                               // push
	0x54, PATCH_UINT16(0x000c),         // self 0c
	0x48,                               // ret
	PATCH_END
};

// Torin/LSL7-specific version of sci2NumSavesSignature1/2
// Applies to at least: English CD
static const uint16 torinLarry7NumSavesSignature[] = {
	SIG_MAGICDWORD,
	0x36,       // push
	0x35, 0x14, // ldi 20
	0x20,       // ge?
	SIG_END
};

static const uint16 torinLarry7NumSavesPatch[] = {
	PATCH_ADDTOOFFSET(+1), // push
	0x35, 0x63,            // ldi 99
	PATCH_END
};

#endif

// Most SCI games run an initial speed test. The results are used to enable
//  animations and other details but some games go further. For example, LSL3
//  calculates how many times the player has to do a task using machine speed.
//
// We disable speed tests by patching them to return fixed results so that all
//  graphics are enabled and games behave consistently. This also fixes the bug
//  where tests fail on fast CPUs because their scores overflow. (bug #13529)
//
// Note that these patches are only for SCI16 games; the test was rewritten for
//  SCI32 and we have separate patches for those.
//
// The first generation of speed tests measured how many game cycles occurred
//  within a fixed amount of time and stored it in a global. Initialize this
//  to a value that always passes and skip the real initialization so that the
//  test immediately completes. This also removes a 1-2 second startup delay.
static const uint16 sci0EarlySpeedTestSignature[] = {
	0xc9, SIG_ADDTOOFFSET(+1),          // +sg machine-speed
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30,                               // bnt [ skip initialization ]
	SIG_END
};

static const uint16 sci0EarlySpeedTestPatch[] = {
	0x34, PATCH_UINT16(0x00ff),         // ldi 00ff
	0xa1, PATCH_GETORIGINALBYTE(+1),    // sag [ machine-speed = 255 ]
	0x32,  
                             // jmp [ always skip initialization ]
	PATCH_END
};

// Same as previous but the +sg instruction became +ag/push
static const uint16 sci01SpeedTestGlobalSignature[] = {
	0xc1, SIG_ADDTOOFFSET(+1),          // +ag machine-speed
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	SIG_END                             // bnt [ skip initialization ]
};

static const uint16 sci01SpeedTestGlobalPatch[] = {
	0x34, PATCH_UINT16(0x00ff),         // ldi 00ff
	0xa1, PATCH_GETORIGINALBYTE(+1),    // sag [ machine-speed = 255 ]
	0x18,                               // not [ always skip initialization ]
	PATCH_END
};

// Same as previous but a local variable is used instead of a global
static const uint16 sci01SpeedTestLocalSignature[] = {
	0xc3, SIG_ADDTOOFFSET(+1),          // +al machine-speed
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	SIG_END                             // bnt [ skip initialization ]
};

static const uint16 sci01SpeedTestLocalPatch[] = {
	0x34, PATCH_UINT16(0x00ff),         // ldi 00ff
	0xa3, PATCH_GETORIGINALBYTE(+1),    // sal [ machine-speed = 255 ]
	0x18,                               // not [ always skip initialization ]
	PATCH_END
};

// The second generation of speed tests measured how much time it takes to do a
//  fixed amount of work. Patch the duration calculation to always be zero.
//  We also disable speed throttling in these rooms in kGameIsRestarting, but
//  that's just to prevent the delay these tests create. Each game's script does
//  different work, so that part can't be generically patched.
static const uint16 sci11SpeedTestSignature[] = {
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime 00
	0x36,                               // push
	SIG_MAGICDWORD,
	0
x83, 0x01,                         // lal 01
	0x04,                               // sub    [ GetTime() - start-time ]
	0xa3, 0x00,                         // sal 00 [ local0 = test-duration ]
	SIG_END
};

static const uint16 sci11SpeedTestPatch[] = {
	0x35, 0x00,                         // ldi 00
	0x33, 0x04,                         // jmp 04 [ local0 = 0, best result ]
	PATCH_END
};

// The speed test originally used a signed comparison, causing the test to take
//  nine minutes if kGetTime rolled over during it. This could happen when
//  restarting the game. We fix this bug with an unsigned comparison, as Sierra
//  did, otherwise our patch will cause the test to hang when kGetTime is large.
static const uint16 sci0SpeedTestOverflowSignature[] = {
	SIG_MAGICDWORD,
	0x8b, 0x00,                         // lsl 00
	0x76,                               // push0
	0x43, SIG_ADDTOOFFSET(+1), 0x00,    // callk GetTime 00
	0x22,                               // lt?
	SIG_END
};

static const uint16 sci0SpeedTestOverflowPatch[] = {
	PATCH_ADDTOOFFSET(+6),
	0x2a,                               // ult?
	PATCH_END
};

// The Narrator class contains a bug that's responsible for rare random lockups
//  in almost every Messager SCI game from 1992 to 1996. The later first-person
//  and FMV games have structures that tend to survive this bug. It was finally,
//  and only, fixed in LSL7.
//
// When a message is said, either through text or audio, Narrator:say calculates
//  the game time when the message will end and stores this in Narrator:ticks.
//  Narrator:doit disposes of itself once ticks is reached unless the user has
//  already dismissed the message. When Narrator isn't saying a message it sets
//  ticks to the sentinel value -1 which prevents doit and handleEvent from
//  doing anything. The one rule of a sentinel value is that it can't appear in
//  normal data, but game time is unsigned and there's nothing preventing
//  Narrator:say from correctly calculating ticks as 65
535 (-1). At 60 ticks per
//  second, game time rolls over every 18 minutes and 12 seconds, and as each
//  rollover approaches it's an opportunity for lockup. If a message is said
//  when game time is high and Narrator:say calculates ticks as 65535 then the
//  message never ends because handleEvent ignores user input and doit won't
//  dispose of the message.
//
// We fix this by preventing Narrator:say from setting ticks to 65535 (-1). When
//  it attempts this we use 0 instead, avoiding lockup by adding 1/60th of a
//  second to the expiration time. Narrator:say changes over time but most
//  versions can be handled by a generic patch, plus another for SCI32 versions
//  that were compiled with debug instructions. A few games customized or cloned
//  Narrator and require specific patches. Others use versions that predate the
//  problem or don't use their Narrator class at all, but otherwise we patch all
//  buggy versions.
//
// Applies to: Most games that use Messager/Narrator
// Responsible method: Narrator:say
static const uint16 sciNarratorLockupSignature[] = {
	0x67, SIG_ADDTOOFFSET(+1),          // pTos ticks
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x36,                               // push
	0x81, 0x58,                         // lag 58 [ game time ]
	0x02,                               // add
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += 60 + game time ]
	0x35, 0x01,                         // ldi 01 [ true ]
	0x48,                               // ret
	SIG_END
};

static const uint16 sciNarratorLockupPatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x89, 0x58,                         // lsg 58 [ game time ]
	0x02,                               // add
	0x65, PATCH_GETORIGINALBYTE(+1),    // aTop ticks [ ticks += 60 + game time ]
	0x00,                               // bnot
	0x31, 0xfb,                         // bnt fb [ set ticks to 0 if ticks == -1 ]
	PATCH_END
};

#ifdef ENABLE
_SCI32
// Same signature/patch as above but for SCI32 games with debug line instructions.
//  Some games use both because different versions were compiled differently.
static const uint16 sciNarratorLockupLineSignature[] = {
	0x67, SIG_ADDTOOFFSET(+1),          // pTos ticks
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x36,                               // push
	0x81, 0x58,                         // lag 58 [ game time ]
	0x02,                               // add
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += 60 + game time ]
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_END
};

static const uint16 sciNarratorLockupLinePatch[] = {
	PATCH_ADDTOOFFSET(+11),
	0x00,                               // bnot
	0x31, 0xfb,                         // bnt fb [ set ticks to 0 if ticks == -1 ]
	PATCH_END
};
#endif

// ECO1 CD and SQ4 CD share an early Narrator:say variant
static const uint16 ecoquest1Sq4CdNarratorLockupSignature[] = {
	0x67, SIG_ADDTOOFFSET(+1),          // pTos ticks
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x36,                               // push
	0x81, 0x58,                         // lag 58 [ game time ]
	0x02,                               // add
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += 60 + game time ]
	0x63, SIG_ADDTOOFFSET(+1),          // pToa modeless
	0x18,                               // not
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [ skip fastCast event handling ]
	0x81, 0x54,                         // lag 54
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [ skip fastCast event handling ]
	0x81, 0x54,                         // lag 54
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [ skip fastCast event handling ]
	SIG_END
};

static const uint16 ecoquest1Sq4CdNarratorLockupPatch[] = {
	PATCH_ADDTOOFFSET(+11),
	0x00,                               // bnot
	0x30, PATCH_UINT16(0xfffa)
,         // bnt fffa [ set ticks to 0 if ticks == -1 ]
	0x63, PATCH_GETORIGINALBYTE(+12),   // pToa modeless
	0x18,                               // not
	PATCH_END
};

// Several SCI Version 1 games use a Timer class that doesn't handle kGetTime
//  rollover correctly. When Timer:set60ths is called with a tick value that
//  elapses after kGetTime rolls over from 65535 to 0, the timer instantly cues.
//  The CD versions of KQ5 and Mixed Up Mother Goose set a timer this way when
//  playing speech that can't be skipped. As rollover approaches every 18
//  minutes and 12 seconds, these messages can instantly complete and cause
//  entire scenes to randomly skip. For example, this can happen in KQ5CD's
//  Harpy and Hermit cutscenes.
//
// We fix this by replacing the Timer's tick calculation with the correct logic
//  that appears in later versions when set60ths was renamed to setTicks.
//
// Applies to: Games that use Timer:set60ths
// Responsible methods: Timer:doit, SpeakTimer:doit in KQ5CD
static const uint16 sciSignatureTimerRollover[] = {
	0x67, SIG_ADDTOOFFSET(+1),       // pTos ticksToDo
	0x63, SIG_ADDTOOFFSET(+1),       // pToa lastTime
	0x02,                            // add
	0x36,                            // push
	0x76,                            // push0
	SIG_MAGICDWORD,
	0x43, 0x42, 0x00,                // callk GetTime 00
	0x2a,                            // ult? [ ticksToDo + lastTime u< kGetTime ]
	0x2e, SIG_UINT16(0x0013),        // bt 0013
	0x67, SIG_ADDTOOFFSET(+1),       // pTos lastTime
	0x76,                            // push0
	SIG_ADDTOOFFSET(+16),
	0x30,                            // bnt [ cue client if acc == true ]
	SIG_END
};

static const uint16 sciPatchTimerRollover[] = {
	0x76,                            // push0
	0x43, 0x42, 0x00,                // callk GetTime 00
	0x36,                            // push
	0x67, PATCH_GETORIGINALBYTE(+1), // pTos ticksToDo
	0x63, PATCH_GETORIGINALBYTE(+3), // pToa lastTime
	0x02,                   
         // add
	0x04,                            // sub
	0x36,                            // push
	0x35, 0x00,                      // ldi 00
	0x1e,                            // gt? [ kGetTime - (ticksToDo + lastTime) > 0 ]
	0x33, 0x10,                      // jmp 10 [ cue client if acc == true ]
	PATCH_END
};

// Several SCI Version 1 games use a Talker class that doesn't handle kGetTime
//  rollover correctly. Talker:init calculates the message's end-time in ticks
//  (1/60ths of a second) and Talker:doit compares this to the current time
//  with a naive signed comparison. When kGetTime approaches $8000, the end-time
//  appears negative and Talker:doit prematurely closes the message.
//
// We fix this by replacing the comparison with the correct logic from later
//  versions. We restructure this to fit within the limited space:
//
//  Existing:    GetTime > ticks
//  Correct:     GetTime - ticks > 0
//  Optimized:   0 > ticks - GetTime
//
// Applies to: Castle of Dr. Brain, LSL5 PC English, SQ1
// Responsible method: Talker:doit
// Fixes bug: #15303
static const uint16 sciSignatureTalkerRollover[] = {
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x43, 0x42, 0x00,                   // callk GetTime 00
	0x36,                               // push
	0x63, SIG_ADDTOOFFSET(+1),          // pToa ticks
	0x1e,                               // gt? [ GetTime > ticks ]
	0x30, SIG_ADDTOOFFSET(+1), 0x00,    // bnt
	SIG_END
};

static const uint16 sciPatchTalkerRollover[] = {
	0x76,                               // push0
	0x67, PATCH_GETORIGINALBYTE(+6),    // pTos ticks
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime 00
	0x04,                               // sub [ ticks - GetTime ]
	0x1e,                               // gt? [ 0 > ticks - GetTime ]
	0x31, PATCH_GETORIGINALBYTE(+9),    // bnt
	PATCH_END
};

// ===========================================================================
// Conquests of Cam
elot
// At the bazaar in Jerusalem, it's possible to see a girl taking a shower.
//  If you get too close, you get warned by Merlin - if you don't get away,
//  the girl's father will kill you.
// Instead of walking there manually, it's also possible to enter "look window"
//  and ego will automatically walk to the window. It seems that this is something
//  that wasn't properly implemented, because instead of getting killed, you will
//  get an "Oops" message in Sierra SCI.
//
// This is caused by peepingTom in script 169 not getting properly initialized.
// peepingTom calls the object behind global[b9h]. This global variable is
//  properly initialized when walking there manually (method fawaz::doit).
// When you instead walk there automati

... [Content truncated]