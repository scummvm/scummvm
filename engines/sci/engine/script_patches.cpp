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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"
#include "sci/engine/features.h"
#include "sci/engine/script_patches.h"
#ifdef ENABLE_SCI32
#include "sci/engine/guest_additions.h"
#endif

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
//      the whole signature actually matches. If it's not included, the script patcher will error() out
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
//        ATTENTION: selectors will only work here, when they are also in SelectorCache (selector.h)

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
	"handsOff",     // system selector
	"handsOn",      // system selector
	"type",         // system selector
	"localize",     // Freddy Pharkas
	"roomFlags",    // Iceman
	"put",          // Police Quest 1 VGA
	"approachVerbs", // Police Quest 1 VGA, QFG4
	"newRoom",      // Police Quest 3, GK1
	"changeState",  // Quest For Glory 1 VGA, QFG4
	"hide",         // Quest For Glory 1 VGA, QFG4
	"say",          // Quest For Glory 1 VGA, QFG4
	"script",       // Quest For Glory 1 VGA
	"solvePuzzle",  // Quest For Glory 3
	"curIcon",      // Quest For Glory 3, QFG4
	"curInvIcon",   // Quest For Glory 3, QFG4
	"timesShownID", // Space Quest 1 VGA
	"startText",    // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"startAudio",   // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"modNum",       // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"handle",       // King's Quest 6 / Laura Bow 2 / RAMA
	"add",          // King's Quest 6
	"givePoints",   // King's Quest 6
	"has",          // King's Quest 6, GK1
	"modeless",     // King's Quest 6 CD
	"cycler",       // Space Quest 4 / system selector
	"setCel",       // Space Quest 4, Phant2, GK1
	"addToPic",     // Space Quest 4
	"stop",         // Space Quest 4
	"canControl",   // Space Quest 4
	"looper",       // Space Quest 4
	"nMsgType",     // Space Quest 4
	"doVerb",       // Space Quest 4
	"setRegions",   // Space Quest 4
	"setSpeed",     // Space Quest 5, QFG4
	"loop",         // Laura Bow 1 Colonel's Bequest, QFG4
	"setLoop",      // Laura Bow 1 Colonel's Bequest, QFG4
	"ignoreActors", // Laura Bow 1 Colonel's Bequest
	"setVol",       // Laura Bow 2 CD
	"at",           // Longbow, QFG4
	"owner",        // Longbow, QFG4
	"delete",       // EcoQuest 1
	"size",         // EcoQuest 1
	"signal",       // EcoQuest 1, GK1
	"obstacles",    // EcoQuest 1, QFG4
	"handleEvent",  // EcoQuest 2, Shivers
#ifdef ENABLE_SCI32
	"newWith",      // SCI2 array script
	"posn",         // GK1, Phant2, QFG4
	"printLang",    // GK2
	"view",         // RAMA benchmarking, GK1, QFG4
	"fade",         // Shivers
	"test",         // Torin
	"get",          // Torin, GK1
	"normalize",    // GK1
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
	"fore",         // KQ7
	"back",         // KQ7
	"font",         // KQ7
	"setHeading",   // KQ7
	"setScale",     // LSL6hires, QFG4
	"setScaler",    // LSL6hires, QFG4
	"readWord",     // LSL7, Phant1, Torin
	"points",       // PQ4
	"select",       // PQ4
	"addObstacle",  // QFG4
	"saveFilePtr",  // RAMA
	"priority",     // RAMA
	"plane",        // RAMA
	"state",        // RAMA
	"getSubscriberObj", // RAMA
	"advanceCurIcon", // QFG4
	"amount",       // QFG4
	"claimed",      // QFG4
	"cue",          // QFG4
	"getCursor",    // QFG4
	"heading",      // QFG4
	"moveSpeed",    // QFG4
	"register",     // QFG4
	"sayMessage",   // QFG4
	"setCursor",    // QFG4
	"setLooper",    // QFG4
	"useStamina",   // QFG4
	"value",        // QFG4
	"enable",       // SQ6
	"setupExit",    // SQ6
	"vol",          // SQ6
	"walkIconItem", // SQ6
#endif
	NULL
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
	SELECTOR_handsOff,
	SELECTOR_handsOn,
	SELECTOR_type,
	SELECTOR_localize,
	SELECTOR_roomFlags,
	SELECTOR_put,
	SELECTOR_approachVerbs,
	SELECTOR_newRoom,
	SELECTOR_changeState,
	SELECTOR_hide,
	SELECTOR_say,
	SELECTOR_script,
	SELECTOR_solvePuzzle,
	SELECTOR_curIcon,
	SELECTOR_curInvIcon,
	SELECTOR_timesShownID,
	SELECTOR_startText,
	SELECTOR_startAudio,
	SELECTOR_modNum,
	SELECTOR_handle,
	SELECTOR_add,
	SELECTOR_givePoints,
	SELECTOR_has,
	SELECTOR_modeless,
	SELECTOR_cycler,
	SELECTOR_setCel,
	SELECTOR_addToPic,
	SELECTOR_stop,
	SELECTOR_canControl,
	SELECTOR_looper,
	SELECTOR_nMsgType,
	SELECTOR_doVerb,
	SELECTOR_setRegions,
	SELECTOR_setSpeed,
	SELECTOR_loop,
	SELECTOR_setLoop,
	SELECTOR_ignoreActors,
	SELECTOR_setVol,
	SELECTOR_at,
	SELECTOR_owner,
	SELECTOR_delete,
	SELECTOR_size,
	SELECTOR_signal,
	SELECTOR_obstacles,
	SELECTOR_handleEvent
#ifdef ENABLE_SCI32
	,
	SELECTOR_newWith,
	SELECTOR_posn,
	SELECTOR_printLang,
	SELECTOR_view,
	SELECTOR_fade,
	SELECTOR_test,
	SELECTOR_get,
	SELECTOR_normalize,
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
	SELECTOR_fore,
	SELECTOR_back,
	SELECTOR_font,
	SELECTOR_setHeading,
	SELECTOR_setScale,
	SELECTOR_setScaler,
	SELECTOR_readWord,
	SELECTOR_points,
	SELECTOR_select,
	SELECTOR_addObstacle,
	SELECTOR_saveFilePtr,
	SELECTOR_priority,
	SELECTOR_plane,
	SELECTOR_state,
	SELECTOR_getSubscriberObj,
	SELECTOR_advanceCurIcon,
	SELECTOR_amount,
	SELECTOR_claimed,
	SELECTOR_cue,
	SELECTOR_getCursor,
	SELECTOR_heading,
	SELECTOR_moveSpeed,
	SELECTOR_register,
	SELECTOR_sayMessage,
	SELECTOR_setCursor,
	SELECTOR_setLooper,
	SELECTOR_useStamina,
	SELECTOR_value,
	SELECTOR_enable,
	SELECTOR_setupExit,
	SELECTOR_vol,
	SELECTOR_walkIconItem
#endif
};

#ifdef ENABLE_SCI32
// It is not possible to change the directory for ScummVM save games, so disable
// the "change directory" button in the standard save dialogue
static const uint16 sci2ChangeDirSignature[] = {
	0x72, SIG_ADDTOOFFSET(+2), // lofsa changeDirI
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
	0x76,                       // push0
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
	0x67, 0x12,                         // pTos data (pass actual data)
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
//  Narrator:say from correctly calculating ticks as 65535 (-1). At 60 ticks per
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
	0x30, PATCH_UINT16(0xfffa),         // bnt fffa [ set ticks to 0 if ticks == -1 ]
	0x63, PATCH_GETORIGINALBYTE(+12),   // pToa modeless
	0x18,                               // not
	PATCH_END
};

// ===========================================================================
// Conquests of Camelot
// At the bazaar in Jerusalem, it's possible to see a girl taking a shower.
//  If you get too close, you get warned by the father - if you don't get away,
//  he will kill you.
// Instead of walking there manually, it's also possible to enter "look window"
//  and ego will automatically walk to the window. It seems that this is something
//  that wasn't properly implemented, because instead of getting killed, you will
//  get an "Oops" message in Sierra SCI.
//
// This is caused by peepingTom in script 169 not getting properly initialized.
// peepingTom calls the object behind global[b9h]. This global variable is
//  properly initialized when walking there manually (method fawaz::doit).
// When you instead walk there automatically (method fawaz::handleEvent), that
//  global isn't initialized, which then results in the Oops-message in Sierra SCI
//  and an error message in ScummVM/SCI.
//
// We fix the script by patching in a jump to the proper code inside fawaz::doit.
// Responsible method: fawaz::handleEvent
// Fixes bug: #6402
static const uint16 camelotSignaturePeepingTom[] = {
	0x72, SIG_MAGICDWORD, SIG_UINT16(0x077e), // lofsa fawaz <-- start of proper initializion code
	0xa1, 0xb9,                      // sag global[b9h]
	SIG_ADDTOOFFSET(+571),           // ...
	0x39, 0x7a,                      // pushi 7a <-- initialization code when walking automatically
	0x78,                            // push1
	0x7a,                            // push2
	0x38, SIG_UINT16(0x00a9),        // pushi 00a9 - script 169
	0x78,                            // push1
	0x43, 0x02, 0x04,                // callk ScriptID
	0x36,                            // push
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06
	0x32, SIG_UINT16(0x0520),        // jmp [end of fawaz::handleEvent]
	SIG_END
};

static const uint16 camelotPatchPeepingTom[] = {
	PATCH_ADDTOOFFSET(+576),
	0x32, PATCH_UINT16(0xfdbd),      // jmp [to fawaz::doit] (properly init peepingTom code)
	PATCH_END
};

// If the butcher's daughter in room 62 closes her window while Arthur interacts
//  with the relic merchant then the game locks up. The script daughterAppears
//  attempts to dispose the script peepingTom, but it does so by clearing ego's
//  script no matter what it is, which breaks the game if the script is buyRelic
//  or one of the other handsOff merchant scripts.
//
// We fix this by calling peepingTom:dispose instead of clearing ego's script.
//  As this is an earlier SCI game prior to Script:dispose clearing the client
//  property, we need to do that ourselves in peepingTom:doit when Arthur turns
//  away from the window, or else peepingTom:client will still point to ego
//  after disposal, and the subsequent peepingTom:dispose will end ego's script.
//
// Applies to: All versions
// Responsible methods: daughterAppears:changeState(6), peepingTom:doit
// Fixes bug: #11025
static const uint16 camelotSignatureRelicMerchantLockup1[] = {
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x76,                               // push0
	0x81, SIG_MAGICDWORD, 0x00,         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setScript: 0 ]
	0x3a,                               // toss
	SIG_END
};

static const uint16 camelotPatchRelicMerchantLockup1[] = {
	0x39, PATCH_SELECTOR8(dispose),     // pushi dispose
	0x76,                               // push0
	0x72, PATCH_UINT16(0x01f3),         // lofsa peepingTom
	0x4a, 0x04,                         // send 04 [ peepingTom dispose: ]
	PATCH_END
};

static const uint16 camelotSignatureRelicMerchantLockup2[] = {
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x76,                               // push0
	0x81, SIG_MAGICDWORD, 0x00,         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setScript: 0 ]
	0x48,                               // ret
	SIG_END
};

static const uint16 camelotPatchRelicMerchantLockup2[] = {
	0x39, PATCH_SELECTOR8(dispose),     // pushi dispose
	0x76,                               // push0
	0x54, 0x04,                         // self 04 [ self dispose: ]
	0x76,                               // push0
	0x69, 0x08,                         // sTop client [ client = 0 ]
	PATCH_END
};

// The hunter in room 11 doesn't award soul points if you buy his furs with the
//  "buy furs" command first and "pay" second. Two points are awarded if these
//  commands are entered in the opposite order.
//
// We fix this by adding the missing function call to award the soul points as
//  Sierra did in later versions. Fortunately, the GiveMoney script contains
//  redundant code that can be replaced as it is occurs later in the method.
//
// Applies to: PC only
// Responsible method: GiveMoney:changeState(3)
// Fixes bug: #11027
static const uint16 camelotSignatureHunterMissingPoints[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0020),           // bnt 0020 [ matches PC only ]
	0x35, 0x00,                         // ldi 00
	0xa3, 0x00,                         // sal 00 [ local0 = 0 ]
	// unnecessary code which is repeated later in the method
	0x89, 0xdd,                         // lsg dd
	0x81, 0x84,                         // lag 84
	0x02,                               // add
	0xa1, 0xdd,                         // sag dd [ global221 += global132 ]
	0x35, 0x00,                         // ldi 00
	0xa1, 0x84,                         // sag 84 [ global132 = 0 ]
	SIG_END
};

static const uint16 camelotPatchHunterMissingPoints[] = {
	PATCH_ADDTOOFFSET(+7),
	0x38, PATCH_UINT16(0x0003),         // pushi 0003
	0x38, PATCH_UINT16(0x00f5),         // pushi 00f5 [ point flag 245 ]
	0x7a,                               // push2 [ soul points ]
	0x7a,                               // push2 [ +2 points ]
	0x45, 0x0a, 0x06,                   // callb proc0_10 06 [ +2 soul points ]
	PATCH_END
};

// When giving away the mule in room 56, the merchant's first long message is
//  immediately replaced by the next. mo:handleEvent displays the first and
//  starts the script getMule, which proceeds to display its own messages
//  without waiting.
//
// We fix this by adding code to getMule to wait for the merchant's message to
//  complete if you gave away the mule and a message is on screen.
//
// Applies to: All versions
// Responsible method: getMule:changeState(4)
// Fixes bug: #11026
static const uint16 camelotSignatureGiveMuleMessage[] = {
	0x30, SIG_UINT16(0x0023),           // bnt 0023 [ state 4 ]
	SIG_ADDTOOFFSET(+0x20),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x0239),           // jmp 0239 [ end of method ]
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0016),           // bnt 0016 [ state 5 ]
	0x83, 0x02,                         // lal 02   [ gave away mule? ]
	0x30, SIG_UINT16(0x000a),           // bnt 000a [ skip state 14 if mule was sold ]
	0x39, SIG_SELECTOR8(changeState),   // pushi changeState
	0x78,                               // push1
	0x39, 0x0e,                         // pushi 0e
	0x54, 0x06,                         // self 06 [ self changeState: 14 ]
	0x32, SIG_UINT16(0x0223),           // jmp 0223 [ end of method ]
	0x35, 0x01,                         // ldi 01
	0x65, 0x10,                         // aTop cycles [ cycles = 1 ]
	0x32, SIG_UINT16(0x021c),           // jmp 021c [ end of method ]
	SIG_END
};

static const uint16 camelotPatchGiveMuleMessage[] = {
	0x30, PATCH_UINT16(0x0020),         // bnt 0020 [ state 4 ]
	PATCH_ADDTOOFFSET(+0x20),
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x31, 0x1a,                         // bnt 1a [ state 5 ]
	0x83, 0x02,                         // lal 02 [ gave away mule? ]
	0x31, 0x13,                         // bnt 13 [ skip state 14 if mule was sold ]
	0x35, 0x0d,                         // ldi 0d
	0x65, 0x0a,                         // aTop state [ state = 13 ]
	0x38, PATCH_UINT16(0x0121),         // pushi talkCue [ same value in all versions ]
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x38, PATCH_UINT16(0x0122),         // pushi tS1 [ same value in all versions ]
	0x76,                               // push0
	0x81, 0x6f,                         // lag 6f
	0x4a, 0x0a,                         // send 0a [ tObj talkCue: self tS1? ]
	0x2f, 0x03,                         // bt 03   [ don't set cycles if message on screen ]
	0x78,                               // push1
	0x69, 0x10,                         // sTop cycles [ cycles = 1 ]
	PATCH_END
};

// In Fatima's house in room 64, "look room" and "look trap" respond with the
//  wrong messages due to testing the wrong flag. Flag 162 is set when falling
//  through the trap door and alters responses the next time in the room, but
//  the script tests flag 137 instead, which is set when entering the room.
//
// Sierra fixed the first flag test in Amiga and Atari ST but not the second, so
//  this patch is applied only once to those versions and twice to PC.
//
// Applies to: All versions
// Responsible method: Rm64:handleEvent
// Fixes bug: #11028
static const uint16 camelotSignatureFatimaRoomMessages[] = {
	0x78,                               // push1
	0x38, SIG_MAGICDWORD,               // pushi 0089 [ flag 137, always true ]
	      SIG_UINT16(0x0089),
	0x45, 0x09, 0x02,                   // callb proc0_9 02 [ is flag 137 set? ]
	SIG_END
};

static const uint16 camelotPatchFatimaRoomMessages[] = {
	PATCH_ADDTOOFFSET(+1),
	0x38, PATCH_UINT16(0x00a2),         // pushi 00a2 [ flag 162, set by trap ]
	PATCH_END
};

// Sheathing the sword by pressing F8 while entering or exiting a room breaks
//  the game by placing ego in an invalid state that allows walking through
//  obstacles and prevents room changes. This affects rooms that subclass eRoom
//  in areas that allow walking with sword drawn such as the monk's ruins and
//  the desert. This is most likely to occur while battling the monk.
//
// eRoom walks ego in and out of rooms in handsOff mode. It sets ego:illegalBits
//  to 0, enables ignoreActors, and sets a motion that cues when complete to
//  restore ego's properties. Sheathing the sword interrupts the motion, which
//  prevents eRoom:cue, and prematurely restores control to the user with ego in
//  the temporary state. There are almost no restrictions on sheathing because
//  it's used when input is disabled and during several handsOff scenes.
//
// We fix this by preventing sword scripts from starting when an eRoom is in the
//  middle of controlling ego. eRoom tracks this with the comingIn and goingOut
//  properties and we require that both be cleared to execute a sword command.
//
// Applies to: All versions
// Responsible method: ARTHUR:doit
// Fixes bug: #11042
static const uint16 camelotSignatureSwordSheathing[] = {
	SIG_MAGICDWORD,
	0x89, 0x7d,                         // lsg 7d [ sword-command ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq? [ sword-command == 1 (draw) ]
	0x30, SIG_UINT16(0x0013),           // bnt 0013
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x7a,                               // push2
	0x38, SIG_UINT16(0x038e),           // pushi 910d
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // callk ScriptID 04 [ ScriptID 910 0 (DrawSword) ]
	0x36,                               // push
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setScript: DrawSword ]
	0x32, SIG_UINT16(0x0085),           // jmp 0085 [ end of switch ]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq? [ sword-command == 2 (sheathe) ]
	0x30, SIG_UINT16(0x0013),           // bnt 0013
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x7a,                               // push2
	0x38, SIG_UINT16(0x038e),           // pushi 910d
	0x78,                               // push1
	0x43, 0x02, 0x04,                   // callk ScriptID 04 [ ScriptID 910 1 (SheatheSword) ]
	0x36,                               // push
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setScript: SheatheSword ]
	0x32, SIG_UINT16(0x006b),           // jmp 006b [ end of switch ]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq? [ sword-command == 3 (parry) ]
	0x30, SIG_UINT16(0x0013),           // bnt 0013
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0390),           // pushi 912d
	0x78,                               // push1
	0x43, 0x02, 0x04,                   // callk ScriptID 04 [ ScriptID 912 1 (DoParry) ]
	SIG_ADDTOOFFSET(+15),
	0x81, 0x7c,                         // lag 7c [ start of sword-command == 0 handler ]
	SIG_ADDTOOFFSET(+72),
	0x3a,                               // toss [ end of switch ]
	SIG_END
};

static const uint16 camelotPatchSwordSheathing[] = {
	0x81, 0x7d,                         // lag 7d [ sword-command ]
	0x31, 0x53,                         // bnt 53 [ sword-command == 0 handler ]
	0x39, 0x03,                         // pushi 03
	0x20,                               // ge? [ 3 >= sword-command ]
	0x31, 0x3e,                         // bnt 3e [ exit if sword-command > 3 ]
	0x7a,                               // push2
	0x89, 0x02,                         // lsg 02
	0x38, PATCH_UINT16(0x014b),         // pushi comingIn [ same value in all versions ]
	0x43, 0x07, 0x04,                   // callk RespondsTo 04 [ RespondsTo currentRoom comingIn ]
	0x31, 0x14,                         // bnt 14 [ skip eRoom checks if room isn't an eRoom ]
	0x38, PATCH_UINT16(0x014b),         // pushi comingIn
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, 0x04,                         // send 04 [ currentRoom comingIn? ]
	0x2f, 0x29,                         // bt 29   [ skip sword scripts if entering room ]
	0x38, PATCH_UINT16(0x014c),         // pushi goingOut [ same value in all versions ]
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, 0x04,                         // send 04 [ currentRoom goingOut? ]
	0x2f, 0x1f,                         // bt 1f   [ skip sword scripts if exiting room ]
	0x39, PATCH_SELECTOR8(setScript),   // pushi setScript
	0x78,                               // push1
	0x7a,                               // push2
	0x81, 0x7d,                         // lag 7d
	0x7a,                               // push2
	0x20,                               // ge? [ 2 >= sword-command ]
	0x31, 0x05,                         // bnt 05
	0x38, PATCH_UINT16(0x038e),         // pushi 910d
	0x33, 0x03,                         // jmp 03
	0x38, PATCH_UINT16(0x0390),         // pushi 912d
	0x81, 0x7d,                         // lag 7d
	0x78,                               // push1
	0x1c,                               // ne? [ sword-command != 1 ]
	0x36,                               // push
	0x43, 0x02, 0x04,                   // callk ScriptID 04 [ ScriptID (sword-command <= 2) ? 910 : 912, (sword-command != 1) ]
	0x36,                               // push
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setScript: sword-script ]
	0x48,                               // ret
	PATCH_ADDTOOFFSET(+89),
	0x48,                               // ret [ remove toss since dup instructions were removed ]
	PATCH_END
};

// When Arthur's sword is drawn, ARTHUR:doit calls kGetEvent a second time on
//  each cycle to test if a shift key is pressed, causing input events to be
//  frequently dropped. This is similar to Freddy Pharkas and QFG1VGA where this
//  technique just happened to usually work in Sierra's interpreter. We fix this
//  in the same way by using the current event instead of consuming a new one.
//
// Applies to: All versions
// Responsible method: ARTHUR:doit
// Fixes bug: #11269
static const uint16 camelotSignatureSwordEvents[] = {
	0x30, SIG_MAGICDWORD,               // bnt 0045
	      SIG_UINT16(0x0045),
	0x39, SIG_SELECTOR8(new),           // pushi new
	0x76,                               // push0
	0x51, 0x07,                         // class Event
	0x4a, 0x04,                         // send 04 [ Event new: ]
	0xa5, 0x01,                         // sat 01 [ temp1 = Event new: ]
	SIG_ADDTOOFFSET(+53),
	0x39, SIG_SELECTOR8(dispose),       // pushi dispose
	0x76,                               // push0
	0x85, 0x01,                         // lat 01
	0x4a, 0x04,                         // send 04 [ temp1 dispose: ]
	SIG_END
};

static const uint16 camelotPatchSwordEvents[] = {
	0x31, 0x46,                         // bnt 46
	0x38, PATCH_SELECTOR16(curEvent),   // pushi curEvent
	0x76,                               // push0
	0x51, 0x30,                         // class User [ User: curEvent ]
	PATCH_ADDTOOFFSET(+57),
	0x33, 0x05,                         // jmp 05 [ don't dispose event ]
	PATCH_END
};

//         script, description,                                       signature                             patch
static const SciScriptPatcherEntry camelotSignatures[] = {
	{ true,     0, "fix sword sheathing",                          1, camelotSignatureSwordSheathing,       camelotPatchSwordSheathing },
	{ true,     0, "fix sword events",                             1, camelotSignatureSwordEvents,          camelotPatchSwordEvents },
	{ true,    11, "fix hunter missing points",                    1, camelotSignatureHunterMissingPoints,  camelotPatchHunterMissingPoints },
	{ true,    62, "fix peepingTom Sierra bug",                    1, camelotSignaturePeepingTom,           camelotPatchPeepingTom },
	{ true,    64, "fix Fatima room messages",                     2, camelotSignatureFatimaRoomMessages,   camelotPatchFatimaRoomMessages },
	{ true,   158, "fix give mule message",                        1, camelotSignatureGiveMuleMessage,      camelotPatchGiveMuleMessage },
	{ true,   169, "fix relic merchant lockup (1/2)",              1, camelotSignatureRelicMerchantLockup1, camelotPatchRelicMerchantLockup1 },
	{ true,   169, "fix relic merchant lockup (2/2)",              1, camelotSignatureRelicMerchantLockup2, camelotPatchRelicMerchantLockup2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// stayAndHelp::changeState (0) is called when ego swims to the left or right
//  boundaries of room 660. Normally a textbox is supposed to get on screen
//  but the call is wrong, so not only do we get an error message the script
//  is also hanging because the cue won't get sent out
//  This also happens in sierra sci
// Applies to at least: PC-CD
// Responsible method: stayAndHelp::changeState
// Fixes bug: #5107
static const uint16 ecoquest1SignatureStayAndHelp[] = {
	0x3f, 0x01,                      // link 01
	0x87, 0x01,                      // lap param[1]
	0x65, 0x14,                      // aTop state
	0x36,                            // push
	0x3c,                            // dup
	0x35, 0x00,                      // ldi 00
	0x1a,                            // eq?
	0x31, 0x1c,                      // bnt [next state]
	0x76,                            // push0
	0x45, 0x01, 0x00,                // callb [export 1 of script 0], 00 (switching control off)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0122),        // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, SIG_SELECTOR8(init),       // pushi init
	0x39, 0x04,                      // pushi 04
	0x76,                            // push0
	0x76,                            // push0
	0x39, 0x17,                      // pushi 17
	0x7c,                            // pushSelf
	0x51, 0x82,                      // class EcoNarrator
	0x4a, 0x0c,                      // send 0c - call EcoNarrator::init(0, 0, 23, self) (BADLY BROKEN!)
	0x33,                            // jmp [end]
	SIG_END
};

static const uint16 ecoquest1PatchStayAndHelp[] = {
	0x87, 0x01,                      // lap param[1]
	0x65, 0x14,                      // aTop state
	0x36,                            // push
	0x2f, 0x22,                      // bt [next state] (this optimization saves 6 bytes)
	0x39, 0x00,                      // pushi 0 (wasting 1 byte here)
	0x45, 0x01, 0x00,                // callb [export 1 of script 0], 00 (switching control off)
	0x38, PATCH_UINT16(0x0122),      // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, PATCH_SELECTOR8(init),     // pushi init
	0x39, 0x06,                      // pushi 06
	0x39, 0x02,                      // pushi 02 (additional 2 bytes)
	0x76,                            // push0
	0x76,                            // push0
	0x39, 0x17,                      // pushi 17
	0x7c,                            // pushSelf
	0x38, PATCH_UINT16(0x0280),      // pushi 280 (additional 3 bytes)
	0x51, 0x82,                      // class EcoNarrator
	0x4a, 0x10,                      // send 10 - call EcoNarrator::init(2, 0, 0, 23, self, 640)
	PATCH_END
};

// Giving the oily shell to Superfluous when he's out of the mask runs the
//  wrong animation and skips messages in the CD version. Sierra modified
//  getInOilyShell for the CD version by adding a new state to the beginning
//  but forgot to increment the state numbers passed to self:changeState to
//  their new values, causing the script to change to the wrong states.
//
// We fix this by incrementing the state numbers passed to self:changeState.
//
// Applies to: PC CD
// Responsible method: getInOilyShell:changeState
// Fixes bug #10881
static const uint16 ecoquest1SignatureGiveOilyShell[] = {
	0x30, SIG_UINT16(0x000a),       // bnt 000a
	0x38, SIG_UINT16(0x0090),       // pushi changeState [ hard coded for CD ]
	0x78,                           // push1
	0x7a,                           // push2 [ state 2 ]
	0x54, SIG_MAGICDWORD, 0x06,     // self 06
	0x32, SIG_UINT16(0x0195),       // jmp 0195
	SIG_ADDTOOFFSET(+209),
	0x39, 0x08,                     // pushi 08 [ state 8 ]
	SIG_ADDTOOFFSET(+16),
	0x39, 0x0b,                     // pushi 0b [ state 11 ]
	SIG_END
};

static const uint16 ecoquest1PatchGiveOilyShell[] = {
	0x31, 0x0b,                     // bnt 0b
	0x38, PATCH_UINT16(0x0090),     // pushi changeState [ hard coded for CD ]
	0x78,                           // push1
	0x39, 0x03,                     // pushi 03 [ state 3 ]
	PATCH_ADDTOOFFSET(+214),
	0x39, 0x09,                     // pushi 09 [ state 9 ]
	PATCH_ADDTOOFFSET(+16),
	0x39, 0x0c,                     // pushi 0c [ state 12 ]
	PATCH_END
};

// Reading the prophecy scroll in the CD version breaks messages in at least
//  rooms 100 and 120. scrollScript:init overwrites the global that holds the
//  noun for the room's message tuples. This global was added in the CD version
//  and is set by most rooms during initialization. This pattern was mistakenly
//  applied to scrollScript which isn't a room and doesn't depend on the global.
//
// We fix this by skipping the problematic code which overwrites the global.
//
// Applies to: PC CD
// Responsible method: scrollScript:init
// Fixes bug #10883
static const uint16 ecoquest1SignatureProphecyScroll[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                     // ldi 01
	0xa1, 0xfa,                     // sag fa [ global250 = 1 ]
	SIG_END
};

static const uint16 ecoquest1PatchProphecyScroll[] = {
	0x33, 0x02,                     // jmp 02 [ don't set global250 ]
	PATCH_END
};

// The empty apartments have several broken messages in the CD version due to
//  not setting the global that holds the current room's noun, so we set it.
//
// Applies to: PC CD
// Responsible method: rm220:init
// Fixes bug #10903
static const uint16 ecoquest1SignatureEmptyApartmentMessages[] = {
	SIG_MAGICDWORD,
	0x54, 0x0c,                     // self 0c [ self setRegions: 51, addObstacle: ... ]
	0x39, SIG_SELECTOR8(init),      // pushi init
	0x76,                           // push0
	0x59, 0x01,                     // &rest 01 [ unused by ApartmentRoom:init ]
	0x57, 0x96, 0x04,               // super ApartmentRoom 04 [ super init: &rest ]
	SIG_END
};

static const uint16 ecoquest1PatchEmptyApartmentMessages[] = {
	0x35, 0x01,                     // ldi 01 [ the room's noun ]
	PATCH_ADDTOOFFSET(+3),
	0xa1, 0xfa,                     // sag fa [ global250 = 1 ]
	0x57, 0x96, 0x10,               // super ApartmentRoom 10 [ combine self and super ]
	PATCH_END
};

// The temple has a complex script bug in the CD version which can crash the
//  interpreter when solving the mosaic puzzle after loading a game that was
//  saved during the puzzle. The bug causes invalid memory access which locks up
//  Sierra's interpreter and can cause ours to fail an assertion.
//
// Room 140 has three insets and a conch shell in the middle. This room's script
//  was significantly changed in the CD version and transition animations were
//  added. Due to these changes the shell no longer renders beneath the insets
//  and so Sierra added code to hide the shell while they're displayed.
//  Unfortunately this code is incorrect and leaves the game in a state that's
//  unsafe to save. The shell is removed from the cast when showing an inset and
//  then shell:init is called when hiding. This leaves shell:underBits pointing
//  to hunk memory while temporarily not a member of the cast. Hunk memory isn't
//  persisted in saved games but underBits' values are. SCI games handle this in
//  Game:replay by clearing the underBits of cast members when restoring. Saving
//  while the puzzle is displayed causes shell:underBits' stale hunk value to
//  survive restoring. Solving the puzzle adds the shell back to the cast via
//  init followed by a call to kAnimate that accesses the potentially stale
//  shell:underBits. If the hunk segment id upon restoring in ScummVM is the
//  same as when saved then this out of bounds access will fail an assertion.
//
// We fix this by fully disposing the shell when showing an inset so that its
//  resources are cleaned up and it's safe to save the game. In order to do this
//  without changing the animation effect we set shell's disposal flag and then
//  immediately call shell:delete. This is equivalent to shell:dispose but
//  prevents hiding the shell before the transition animation takes place.
//
// Applies to: PC CD
// Responsible methods: MosaicWall:doVerb, localproc_2ab6 in script 140
// Fixes bug #10884
static const uint16 ecoquest1SignatureMosaicPuzzleFix[] = {
	0x36,                           // push     [ conchShell:owner ]
	0x34, SIG_UINT16(0x008c),       // ldi 008c [ room number ]
	0x1a,                           // eq?      [ is conchShell owned by room 140? ]
	0x30, SIG_UINT16(0x000b),       // bnt 000b [ no shell to hide ]
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(delete),    // pushi delete
	0x78,                           // push1
	0x72, SIG_UINT16(0x056a),       // lofsa shell
	0x36,                           // push
	0x81, 0x05,                     // lag 05
	0x4a, 0x06,                     // send 06 [ cast delete: shell ]
	SIG_END
};

static const uint16 ecoquest1PatchMosaicPuzzleFix[] = {
	0x89, 0x0b,                     // lsg 0b [ current room number, saves 2 bytes ]
	0x1a,                           // eq?    [ is conchShell owned by room 140? ]
	0x31, 0x0e,                     // bnt 0e [ no shell to hide, save a byte ]
	0x39, PATCH_SELECTOR8(signal),  // pushi signal
	0x78,                           // push1
	0x38, PATCH_UINT16(0xc014),     // pushi c014 [ kSignalDisposeMe | shell:signal ]
	0x39, PATCH_SELECTOR8(delete),  // pushi delete
	0x76,                           // push0
	0x72, PATCH_UINT16(0x056a),     // lofsa shell
	0x4a, 0x0a,                     // send 0a [ shell signal: c014, delete ]
	PATCH_END
};

// The column puzzle in room 160 can be put in a state that can't be completed.
//  This is a bug in the original that affects all versions.
//
// The puzzle consists of nine columns that must be rotated to their correct
//  positions in the correct order. As each column is solved it is locked. When
//  leaving the room the puzzle state is saved to globals but this state is
//  insufficient to recreate the puzzle. The game saves column positions but not
//  lock states. Instead it infers lock states from positions when restoring but
//  this is inaccurate because columns can be put in their correct positions out
//  of order. If the player leaves the room while all columns are in their
//  correct positions but before the puzzle is solved then all columns will be
//  locked when returning and the game can't be completed.
//
// The proper solution would be to save and restore lock states but it would be
//  impractical to patch in that functionality while retaining save game
//  compatibility. Instead we patch the loop that reinitializes lock states to
//  skip the last column so that it's always unlocked and the player can't get
//  stuck. This code only runs when the puzzle isn't solved and should never
//  have been able to lock the last column.
//
// Applies to: All Floppy and CD versions
// Responsible method: Local procedure 5 in script 160
// Fixes bug #10885
static const uint16 ecoquest1SignatureColumnPuzzleFix[] = {
	0x39, SIG_SELECTOR8(size),      // pushi size
	0x76,                           // push0
	0x72, SIG_ADDTOOFFSET(+2),      // lofsa columnList [ columns in solution order ]
	0x4a, 0x04,                     // send 04 [ columnList:size ]
	0x22,                           // lt? [ temp0 < columnList:size (9) ]
	0x30, SIG_ADDTOOFFSET(+2),      // bnt [ end of method ]
	0x39, SIG_MAGICDWORD,           // pushi cel
	      SIG_SELECTOR8(cel),
	0x76,                           // push0
	0x39, SIG_SELECTOR8(at),        // pushi at
	SIG_END
};

static const uint16 ecoquest1PatchColumnPuzzleFix[] = {
	0x34, PATCH_UINT16(0x0008),     // ldi 0008 [ only initialize 8 of 9 columns ]
	0x32, PATCH_UINT16(0x0002),     // jmp 0002
	PATCH_END
};

// The ocean cliffs that border rooms 320 and 321 aren't displayed in the CD
//  version. Instead they are drawn above the visible area and on more screens
//  than they should. This also occurs in the original.
//
// Cliff views 325 and 326 have y displacements greater than 127 in the floppy
//  versions. In the CD version these offsets were changed to zero. Sierra
//  attempted to compensate for this by adding rows of empty pixels to the views
//  but it appears that someone mistook the unsigned offsets for negative values
//  and added the wrong number of rows to the wrong side of the views, causing
//  the cliffs to be drawn 256 pixels higher than normal.
//
// The ocean scripts were changed to use different techniques for adding and
//  removing the cliffs but this introduced more errors. Room 321 reinitializes
//  the cliffs instead of disposing them, causing them to be redrawn on the
//  wrong screens, and room 320 disposes the eastern cliffs instead of western.
//
// We fix the cliffs by adjusting their positions by 256 and disposing of them
//  in room 321. We leave room 320's incorrect cliff disposal in place since
//  both are automatically disposed of when that room's pic changes.
//
// Applies to: PC CD
// Responsible methods: Heap in scripts 320 and 321, toEast:changeState, toWest:changeState
// Fixes bug #10893
static const uint16 ecoquest1SignatureSouthCliffsPosition[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0095),             // easternCliffs:x = 149
	SIG_UINT16(0x0033),             // easternCliffs:y = 51
	SIG_ADDTOOFFSET(+88),
	SIG_UINT16(0x0004),             // westernCliffs:x = 4
	SIG_UINT16(0x0014),             // westernCliffs:y = 20
	SIG_END
};

static const uint16 ecoquest1PatchSouthCliffsPosition[] = {
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x0133),           // easternCliffs:y = 307
	PATCH_ADDTOOFFSET(+90),
	PATCH_UINT16(0x0114),           // westernCliffs:y = 276
	PATCH_END
};

static const uint16 ecoquest1SignatureNorthCliffsPosition[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x00eb),             // easternCliffs:x = 236
	SIG_UINT16(0x0038),             // easternCliffs:y = 56
	SIG_ADDTOOFFSET(+88),
	SIG_UINT16(0x0000),             // westernCliffs:x = 0
	SIG_UINT16(0x0032),             // westernCliffs:y = 50
	SIG_END
};

static const uint16 ecoquest1PatchNorthCliffsPosition[] = {
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x0138),           // easternCliffs:y = 312
	PATCH_ADDTOOFFSET(+90),
	PATCH_UINT16(0x0132),           // westernCliffs:y = 306
	PATCH_END
};

static const uint16 ecoquest1SignatureNorthCliffsDisposal[] = {
	0x39, SIG_SELECTOR8(init),          // pushi init
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa easternCliffs or westernCliffs
	0x4a, SIG_MAGICDWORD, 0x04,         // send 04 [ cliffs init: ]
	0x38, SIG_SELECTOR16(obstacles),    // pushi obstacles
	SIG_END
};

static const uint16 ecoquest1PatchNorthCliffsDisposal[] = {
	0x39, PATCH_SELECTOR8(dispose),     // pushi dispose
	PATCH_END
};

// The Spanish version of EcoQuest accidentally shipped with temporary test code
//  that breaks the game when entering Olympia's apartment. (room 226)
//
// A message box's position was localized in the Spanish version. This message
//  occurs after saving Olympia by pumping bleach out of the window. To test
//  this change, a developer added code to forcibly run the usePump script upon
//  entering the room, but then forgot to remove it. This breaks the puzzle and
//  locks up the game upon re-entering the room.
//
// We fix this by disabling the test code that should not have been shipped.
//
// Applies to: Spanish PC Floppy
// Responsible method: rm226:init
// Fixes bug #10900
static const uint16 ecoquest1SignatureBleachPumpTest[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x39, 0x35,                         // pushi 35
	0x46, SIG_UINT16(0x0333),           // calle proc819_3 [ set recycled-bleach flag ]
	      SIG_UINT16(0x0003), 0x02,
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0x0a44),           // lofsa usePump
	0x36,                               // push
	0x54, 0x06,                         // self 06 [ self setScript: usePump ]
	SIG_END
};

static const uint16 ecoquest1PatchBleachPumpTest[] = {
	0x32, PATCH_UINT16(0x0010),         // jmp 0010 [ skip test code ]
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry ecoquest1Signatures[] = {
	{  true,   140, "CD: mosaic puzzle fix",                       2, ecoquest1SignatureMosaicPuzzleFix,        ecoquest1PatchMosaicPuzzleFix },
	{  true,   160, "CD: give superfluous oily shell",             1, ecoquest1SignatureGiveOilyShell,          ecoquest1PatchGiveOilyShell },
	{  true,   160, "CD/Floppy: column puzzle fix",                1, ecoquest1SignatureColumnPuzzleFix,        ecoquest1PatchColumnPuzzleFix },
	{  true,   220, "CD: empty apartment messages",                1, ecoquest1SignatureEmptyApartmentMessages, ecoquest1PatchEmptyApartmentMessages },
	{  true,   226, "Spanish: disable bleach pump test",           1, ecoquest1SignatureBleachPumpTest,         ecoquest1PatchBleachPumpTest },
	{  true,   320, "CD: south cliffs position",                   1, ecoquest1SignatureSouthCliffsPosition,    ecoquest1PatchSouthCliffsPosition },
	{  true,   321, "CD: north cliffs position",                   1, ecoquest1SignatureNorthCliffsPosition,    ecoquest1PatchNorthCliffsPosition },
	{  true,   321, "CD: north cliffs disposal",                   2, ecoquest1SignatureNorthCliffsDisposal,    ecoquest1PatchNorthCliffsDisposal },
	{  true,   660, "CD: bad messagebox and freeze",               1, ecoquest1SignatureStayAndHelp,            ecoquest1PatchStayAndHelp },
	{  true,   816, "CD: prophecy scroll",                         1, ecoquest1SignatureProphecyScroll,         ecoquest1PatchProphecyScroll },
	{  true,   928, "CD: EcoTalker lockup fix",                    1, ecoquest1Sq4CdNarratorLockupSignature,    ecoquest1Sq4CdNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// doMyThing::changeState (2) is supposed to remove the initial text on the
//  ecorder. This is done by reusing temp-space, that was filled on state 1.
//  this worked in sierra sci just by accident. In our sci, the temp space
//  is resetted every time, which means the previous text isn't available
//  anymore. We have to patch the code because of that.
// Fixes bug: #4993
static const uint16 ecoquest2SignatureEcorder[] = {
	0x31, 0x22,                      // bnt [next state]
	0x39, 0x0a,                      // pushi 0a
	0x5b, 0x04, 0x1e,                // lea temp[1e]
	0x36,                            // push
	SIG_MAGICDWORD,
	0x39, 0x64,                      // pushi 64
	0x39, 0x7d,                      // pushi 7d
	0x39, 0x32,                      // pushi 32
	0x39, 0x66,                      // pushi 66
	0x39, 0x17,                      // pushi 17
	0x39, 0x69,                      // pushi 69
	0x38, SIG_UINT16(0x2631),        // pushi 2631
	0x39, 0x6a,                      // pushi 6a
	0x39, 0x64,                      // pushi 64
	0x43, 0x1b, 0x14,                // callk Display
	0x35, 0x0a,                      // ldi 0a
	0x65, 0x20,                      // aTop ticks
	0x33,                            // jmp [end]
	SIG_ADDTOOFFSET(+1),             // [skip 1 byte]
	0x3c,                            // dup
	0x35, 0x03,                      // ldi 03
	0x1a,                            // eq?
	0x31,                            // bnt [end]
	SIG_END
};

static const uint16 ecoquest2PatchEcorder[] = {
	0x2f, 0x02,                      // bt [to pushi 7]
	0x3a,                            // toss
	0x48,                            // ret
	0x38, PATCH_UINT16(0x0007),      // pushi 7d (parameter count) (waste 1 byte)
	0x39, 0x0b,                      // pushi 11d (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x38, PATCH_UINT16(0x0017),      // pushi 23d (color) (waste 1 byte)
	0x43, 0x6c, 0x0e,                // callk Graph
	0x38, PATCH_UINT16(0x0005),      // pushi 5d (parameter count) (waste 1 byte)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // callk Graph
	PATCH_END
};

// Same patch as above for the ecorder introduction.
// Two workarounds are needed for this patch in workarounds.cpp (when calling
// kGraphFillBoxAny and kGraphUpdateBox), as there isn't enough space to patch
// the function otherwise.
// Fixes bug: #6467
static const uint16 ecoquest2SignatureEcorderTutorial[] = {
	0x30, SIG_UINT16(0x0023),        // bnt [next state]
	0x39, 0x0a,                      // pushi 0a
	0x5b, 0x04, 0x1f,                // lea temp[1f]
	0x36,                            // push
	SIG_MAGICDWORD,
	0x39, 0x64,                      // pushi 64
	0x39, 0x7d,                      // pushi 7d
	0x39, 0x32,                      // pushi 32
	0x39, 0x66,                      // pushi 66
	0x39, 0x17,                      // pushi 17
	0x39, 0x69,                      // pushi 69
	0x38, SIG_UINT16(0x2631),        // pushi 2631
	0x39, 0x6a,                      // pushi 6a
	0x39, 0x64,                      // pushi 64
	0x43, 0x1b, 0x14,                // callk Display
	0x35, 0x1e,                      // ldi 1e
	0x65, 0x20,                      // aTop ticks
	0x32,                            // jmp [end]
	// 2 extra bytes, jmp offset
	SIG_END
};

static const uint16 ecoquest2PatchEcorderTutorial[] = {
	0x31, 0x23,                      // bnt [next state] (save 1 byte)
	// The parameter count below should be 7, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,                            // push1 (parameter count)
	//0x39, 0x07,                    // pushi 7d (parameter count)
	0x39, 0x0b,                      // pushi 11d (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x39, 0x17,                      // pushi 23d (color)
	0x43, 0x6c, 0x0e,                // callk Graph
	// The parameter count below should be 5, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,                            // push1 (parameter count)
	//0x39, 0x05,                    // pushi 5d (parameter count)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // callk Graph
	// We are out of bytes to patch at this point,
	// so we skip 494 (0x1ee) bytes to reuse this code:
	// ldi 1e
	// aTop 20
	// jmp 030e (jump to end)
	0x32, PATCH_UINT16(0x01ee),      // jmp 494d
	PATCH_END
};

// Clicking an icon during the icon bar tutorial in room 100 sends messages to
//  an uninitialized temporary variable. This is supposed to be the dispatched
//  Event object that's passed around earlier in the call stack. In Sierra's
//  interpreter that's what happened to be at this location and it worked.
//
// We fix this by using the global variable that stores the current Event object
//  instead of the uninitialized temp variable that accidentally points to it.
//  User:handleEvent sets this global before dispatching an event.
//
// Applies to: All versions
// Responsible methods: iconWalk:select, iconLook:select, ...
// Fixes bug: #11081
static const uint16 ecoquest2SignatureIconBarTutorial[] = {
	0x7a,                               // push2
	0x38, SIG_SELECTOR16(handleEvent),  // pushi handleEvent
	SIG_MAGICDWORD,
	0x8d, 0x01,                         // lst 01  [ uninitialized ]
	0x4a, 0x08,                         // send 08 [ EventHandler firstTrue: handleEvent temp1 ]
	SIG_END
};

static const uint16 ecoquest2PatchIconBarTutorial[] = {
	PATCH_ADDTOOFFSET(+4),
	0x89, 0x18,                         // lsg 18 [ current event ]
	PATCH_END
};

// The electronic organizer and password paper reappear in room 500 after they
//  fall into the water when entering the canoe. rm500:init only tests if these
//  items are in inventory. It should have also tested the canoe flag like room
//  530 does to prevent the vacuum from reappearing.
//
// We fix this by only adding an item to the room if its InvI:owner is zero.
//  This is initially zero, then set to ego when getting an item, and finally
//  set to negative one when the item is removed from inventory.
//
// Applies to: All versions
// Responsible method: rm500:init
// Fixes bug: #11135
static const uint16 ecoquest2SignatureRoom500Items[] = {
	0x38, SIG_ADDTOOFFSET(+2),          // pushi test
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x0b,                         // pushi 0b
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 11 ]
	0xa5, 0x00,                         // sat 00
	0x38, SIG_ADDTOOFFSET(+2),          // pushi test
	0x78,                               // push1
	0x39, 0x04,                         // pushi 04
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 4 ]
	0xa5, 0x01,                         // sat 01
	0x38, SIG_ADDTOOFFSET(+2),          // pushi test
	0x78,                               // push1
	0x39, 0x17,                         // pushi 17
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 23 ]
	0xa5, 0x02,                         // sat 02
	0x38, SIG_ADDTOOFFSET(+2),          // pushi test
	SIG_ADDTOOFFSET(+636),
	0x38, SIG_SELECTOR16(has),          // pushi has
	0x78,                               // push1
	0x39, 0x15,                         // pushi 15
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego has: 21 ]
	0x18,                               // not
	0x31, 0x13,                         // bnt 13 [ don't initialize theOrganizer ]
	SIG_ADDTOOFFSET(+236),
	0x38, SIG_SELECTOR16(has),          // pushi has
	0x78,                               // push1
	0x39, 0x0b,                         // pushi 0b
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego has: 11 ]
	0x18,                               // not
	0x30, SIG_UINT16(0x0058),           // bnt 0058 [ don't initialize paper ]
	SIG_END
};

static const uint16 ecoquest2PatchRoom500Items[] = {
	0x39, PATCH_SELECTOR8(at),          // pushi at
	0x3c,                               // dup [ push at, saves 1 byte ]
	0x78,                               // push1
	0x39, 0x15,                         // pushi 15
	0x38, PATCH_GETORIGINALUINT16(+1),  // pushi test
	0x3c,                               // dup [ push test, saves 2 bytes ]
	0x3c,                               // dup [ push test, saves 2 bytes ]
	0x3c,                               // dup [ push test, saves 2 bytes ]
	0x78,                               // push1
	0x39, 0x0b,                         // pushi 0b
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 11 ]
	0xa5, 0x00,                         // sat 00
	0x78,                               // push1
	0x39, 0x04,                         // pushi 04
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 4 ]
	0xa5, 0x01,                         // sat 01
	0x78,                               // push1
	0x39, 0x17,                         // pushi 17
	0x81, 0x96,                         // lag 96
	0x4a, 0x06,                         // send 06 [ cibolaFlags test: 23 ]
	0xa5, 0x02,                         // sat 02
	PATCH_ADDTOOFFSET(+636),
	0x81, 0x09,                         // lag 09
	0x4a, 0x06,                         // send 06 [ Inv at: 21 ]
	0x38, PATCH_SELECTOR16(owner),      // pushi owner
	0x76,                               // push0
	0x4a, 0x04,                         // send 04 [ organizer owner? ]
	0x78,                               // push1
	0x2f, 0x13,                         // bt 13 [ don't initialize theOrganizer ]
	PATCH_ADDTOOFFSET(+236),
	0x39, 0x0b,                         // pushi 0b
	0x81, 0x09,                         // lag 09
	0x4a, 0x06,                         // send 06 [ Inv at: 11 ]
	0x38, PATCH_SELECTOR16(owner),      // pushi owner
	0x76,                               // push0
	0x4a, 0x04,                         // send 04 [ password owner? ]
	0x2f, 0x58,                         // bt 58 [ don't initialize paper ]
	PATCH_END
};

// The Ecorder cursor only highlights over one of the four Victoria lilies, even
//  though they all respond to Ecorder clicks. Each lily has a doit method that
//  highlights the cursor but three of them never execute because they are
//  added to the room pic. This removes them from the cast and prevents doit.
//
// We fix this by removing the addToPic calls so the lilies remain in the cast.
//
// Applies to: All versions
// Responsible methods: lilly1:init, lilly2:init, lilly3:init
// Fixes bug: #5552
static const uint16 ecoquest2SignatureEcorderLily[] = {
	0x38, SIG_MAGICDWORD,               // pushi addToPic
	      SIG_SELECTOR16(addToPic),
	0x76,                               // push0
	0x54, 0x04,                         // self 04 [ self addToPic: ]
	SIG_END
};

static const uint16 ecoquest2PatchEcorderLily[] = {
	0x32, PATCH_UINT16(0x0003),         // jmp 0003
	PATCH_END
};

// Objects that you can hide behind in rooms 530 and 560 all have messages that
//  are supposed to display when clicking Do after Gonzales leaves camp, but
//  their doVerb methods are missing a call to super:doVerb. We fix this by
//  patching the doVerb methods to call super:doVerb instead of doing nothing.
//
// Applies to: All versions
// Responsible methods: crates:doVerb, barrel1-4:doVerb, refuse:doVerb in 530,
//  bullldozer:doVerb, barrel1-5:doVerb, crates:doVerb in 560
static const uint16 ecoquest2SignatureCampMessages1[] = {
	0x30, SIG_UINT16(0x0033),           // bnt 0033 [ end of method ]
	SIG_ADDTOOFFSET(+10),
	0x30, SIG_UINT16(0x0026),           // bnt 0026 [ end of method ]
	SIG_ADDTOOFFSET(+8),
	0x30, SIG_UINT16(0x001b),           // bnt 001b [ end of method ]
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x39, SIG_MAGICDWORD, 0x03,         // pushi 03
	0x72, SIG_UINT16(0x00cc),           // lofsa sRunHide
	SIG_ADDTOOFFSET(+10),
	0x38, SIG_SELECTOR16(doVerb),       // pushi doVerb
	SIG_END
};

static const uint16 ecoquest2PatchCampMessages1[] = {
	0x30, PATCH_UINT16(0x002a),         // bnt 002a [ super doVerb: verb ]
	PATCH_ADDTOOFFSET(+10),
	0x30, PATCH_UINT16(0x001d),         // bnt 001d [ super doVerb: verb ]
	PATCH_ADDTOOFFSET(+8),
	0x30, PATCH_UINT16(0x0012),         // bnt 0012 [ super doVerb: verb ]
	PATCH_END
};

static const uint16 ecoquest2SignatureCampMessages2[] = {
	0x30, SIG_UINT16(0x001b),           // bnt 001b [ end of method ]
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x39, SIG_MAGICDWORD, 0x03,         // pushi 03
	0x72, SIG_UINT16(0x0154),           // lofsa sRunHide
	SIG_ADDTOOFFSET(+10),
	0x38, SIG_SELECTOR16(doVerb),       // pushi doVerb
	SIG_END
};

static const uint16 ecoquest2PatchCampMessages2[] = {
	0x30, PATCH_UINT16(0x0012),         // bnt 0012 [ super doVerb: verb ]
	PATCH_END
};

//          script, description,                                        signature                          patch
static const SciScriptPatcherEntry ecoquest2Signatures[] = {
	{  true,     0, "icon bar tutorial",                            10, ecoquest2SignatureIconBarTutorial, ecoquest2PatchIconBarTutorial },
	{  true,    50, "initial text not removed on ecorder",           1, ecoquest2SignatureEcorder,         ecoquest2PatchEcorder },
	{  true,   333, "initial text not removed on ecorder tutorial",  1, ecoquest2SignatureEcorderTutorial, ecoquest2PatchEcorderTutorial },
	{  true,   500, "room 500 items reappear",                       1, ecoquest2SignatureRoom500Items,    ecoquest2PatchRoom500Items },
	{  true,   530, "missing camp messages",                         6, ecoquest2SignatureCampMessages1,   ecoquest2PatchCampMessages1 },
	{  true,   560, "missing camp messages",                         7, ecoquest2SignatureCampMessages2,   ecoquest2PatchCampMessages2 },
	{  true,   702, "ecorder not highlighting lilies",               3, ecoquest2SignatureEcorderLily,     ecoquest2PatchEcorderLily },
	{  true,   928, "Narrator lockup fix",                           1, sciNarratorLockupSignature,        sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Fan-made games
// Attention: Try to make script patches as specific as possible

// CascadeQuest::autosave in script 994 is called various times to auto-save.
//  It uses a fixed slot (999) for this purpose. This doesn't work in ScummVM,
//  because we do not let scripts save directly into specific slots, but
//  instead use virtual slots / detect scripts wanting to create a new slot.
// We patch the code to use slot 99 instead. kSaveGame also checks for Cascade
//  Quest, and if slot 99 is asked for, it will then use the actual slot 0,
//  which is the official ScummVM auto-save slot.
//
// Responsible method: CascadeQuest::autosave
// Fixes bug: #7007
static const uint16 fanmadeSignatureCascadeQuestFixAutoSaving[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x03e7),        // pushi 3E7 (999d) -> save game slot 999
	0x74, SIG_UINT16(0x06f8),        // lofss "AutoSave"
	0x89, 0x1e,                      // lsg global[1e]
	0x43, 0x2d, 0x08,                // callk SaveGame
	SIG_END
};

static const uint16 fanmadePatchCascadeQuestFixAutoSaving[] = {
	0x38, PATCH_UINT16((SAVEGAMEID_OFFICIALRANGE_START - 1)), // fix slot
	PATCH_END
};

// EventHandler::handleEvent in Demo Quest has a bug, and it jumps to the
// wrong address when an incorrect word is typed, therefore leading to an
// infinite loop. This script bug was not apparent in SSCI, probably because
// event handling was slightly different there, so it was never discovered.
// Fixes bug: #5120
static const uint16 fanmadeSignatureDemoQuestInfiniteLoop[] = {
	0x38, SIG_UINT16(0x004c),        // pushi 004c
	0x39, 0x00,                      // pushi 00
	0x87, 0x01,                      // lap param[1]
	0x4b, 0x04,                      // send 04
	SIG_MAGICDWORD,
	0x18,                            // not
	0x30, SIG_UINT16(0x002f),        // bnt 002f  [06a5]    --> jmp ffbc  [0664] --> BUG! infinite loop
	SIG_END
};

static const uint16 fanmadePatchDemoQuestInfiniteLoop[] = {
	PATCH_ADDTOOFFSET(+10),
	0x30, PATCH_UINT16(0x0032),      // bnt 0032  [06a8] --> pushi 004c
	PATCH_END
};

// This patch is for a bug that first appeared in the LSL3 volume dialog and was
//  then copied into the templates included with SCI Studio and SCI Companion,
//  causing it to appear in fan games. See larry3SignatureVolumeSlider.
//
// Applies to: Fan games built with the SCI Studio / SCI Companion SCI0 template
// Responsible method: TheMenuBar:handleEvent
static const uint16 fangameSignatureVolumeSlider[] = {
	0x39, SIG_SELECTOR8(doit),       // pushi doit
	SIG_ADDTOOFFSET(+1),             // push1 [ opcode 79 instead of 78 in some games ]
	SIG_ADDTOOFFSET(+1),             // push2 [ opcode 7b instead of 7a in some games ]
	SIG_MAGICDWORD,
	0x39, 0x08,                      // pushi 08 [ volume ]
	0x8d, 0x03,                      // lst 03   [ uninitialized variable ]
	0x43, 0x31, 0x04,                // callk DoSound 04 [ set volume and return previous ]
	SIG_END
};

static const uint16 fangamePatchVolumeSlider[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x01,                      // pushi 01
	0x38, PATCH_UINT16(0x0008),      // pushi 0008 [ volume ]
	0x43, 0x31, 0x02,                // callk DoSound 02 [ return volume ]
	PATCH_END
};

//          script, description,                                      signature                                  patch
static const SciScriptPatcherEntry fanmadeSignatures[] = {
	{  true,   994, "Cascade Quest: fix auto-saving",              1, fanmadeSignatureCascadeQuestFixAutoSaving, fanmadePatchCascadeQuestFixAutoSaving },
	{  true,   997, "SCI Template: fix volume slider",             1, fangameSignatureVolumeSlider,              fangamePatchVolumeSlider },
	{  true,   999, "Demo Quest: infinite loop on typo",           1, fanmadeSignatureDemoQuestInfiniteLoop,     fanmadePatchDemoQuestInfiniteLoop },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// WORKAROUND
// Freddy Pharkas intro screen
// Sierra used inner loops for the scaling of the 2 title views.
// Those inner loops don't call kGameIsRestarting, which is why
// we do not update the screen and we also do not throttle.
//
// This patch fixes this and makes it work.
// Applies to at least: English PC-CD
// Responsible method: sTownScript::changeState(1), sTownScript::changeState(3) (script 110)
static const uint16 freddypharkasSignatureIntroScaling[] = {
	0x38, SIG_ADDTOOFFSET(+2),       // pushi (setLoop) (009b for PC CD)
	0x78,                            // push1
	SIG_ADDTOOFFSET(+1),             // push0 for first code, push1 for second code
	0x38, SIG_ADDTOOFFSET(+2),       // pushi (setStep) (0143 for PC CD)
	0x7a,                            // push2
	0x39, 0x05,                      // pushi 05
	0x3c,                            // dup
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa (view)
	SIG_MAGICDWORD,
	0x4a, 0x1e,                      // send 1e
	0x35, 0x0a,                      // ldi 0a
	0xa3, 0x02,                      // sal local[2]
	// start of inner loop
	0x8b, 0x02,                      // lsl local[2]
	SIG_ADDTOOFFSET(+43),            // skip almost all of inner loop
	0xa3, 0x02,                      // sal local[2]
	0x33, 0xcf,                      // jmp [inner loop start]
	SIG_END
};

static const uint16 freddypharkasPatchIntroScaling[] = {
	// remove setLoop(), objects in heap are already prepared, saves 5 bytes
	0x38,
	PATCH_GETORIGINALUINT16(+6),     // pushi (setStep)
	0x7a,                            // push2
	0x39, 0x05,                      // pushi 05
	0x3c,                            // dup
	0x72,
	PATCH_GETORIGINALUINT16(+13),    // lofsa (view)
	0x4a, 0x18,                      // send 18 - adjusted
	0x35, 0x0a,                      // ldi 0a
	0xa3, 0x02,                      // sal local[2]
	// start of new inner loop
	0x39, 0x00,                      // pushi 00
	0x43, 0x2c, 0x00,                // callk GameIsRestarting (add this to trigger our speed throttler)
	PATCH_ADDTOOFFSET(+47),          // skip almost all of inner loop
	0x33, 0xca,                      // jmp [inner loop start]
	PATCH_END
};

// PointsSound::check waits for a signal. If no signal is received, it'll call
//   kDoSound(0x0d) which is a dummy in sierra sci. ScummVM and will use acc
//   (which is not set by the dummy) to trigger sound disposal. This somewhat
//   worked in sierra sci because the sample was already playing in the sound
//   driver. In our case, that would also stop the sample from playing, so we
//   patch it out. The "score" code is already buggy and sets volume to 0 when
//   playing.
// Applies to at least: English PC-CD
// Responsible method: PointsSound::check in script 0
// Fixes bug: #5059
static const uint16 freddypharkasSignatureScoreDisposal[] = {
	0x67, 0x32,                      // pTos 32 (selector theAudCount)
	0x78,                            // push1
	SIG_MAGICDWORD,
	0x39, 0x0d,                      // pushi 0d
	0x43, 0x75, 0x02,                // callk DoAudio
	0x1c,                            // ne?
	0x31,                            // bnt [skip disposal]
	SIG_END
};

static const uint16 freddypharkasPatchScoreDisposal[] = {
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	PATCH_END
};

// In script 235, rm235::init and sEnterFrom500 disable icon 7+8 of iconbar (CD
//  only). When picking up the canister after placing it down, the scripts will
//  disable all the other icons. This results in IconBar::disable doing endless
//  loops even in sierra sci because there is no enabled icon left. We remove
//  the disabling of icon 8 (which is help). This fixes the issue.
// Applies to at least: English PC-CD
// Responsible method: rm235::init and sEnterFrom500::changeState
// Fixes bug: #5245
static const uint16 freddypharkasSignatureCanisterHang[] = {
	0x38, SIG_SELECTOR16(disable),   // pushi disable
	0x7a,                            // push2
	SIG_MAGICDWORD,
	0x39, 0x07,                      // pushi 07
	0x39, 0x08,                      // pushi 08
	0x81, 0x45,                      // lag global[45]
	0x4a, 0x08,                      // send 08 (call IconBar::disable(7, 8))
	SIG_END
};

static const uint16 freddypharkasPatchCanisterHang[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                            // push1
	PATCH_ADDTOOFFSET(+2),
	0x33, 0x00,                      // jmp 0 (waste 2 bytes)
	PATCH_ADDTOOFFSET(+3),
	0x06,                            // send 06 (call IconBar::disable(7))
	PATCH_END
};

// In script 215, lowerLadder::doit and highLadder::doit actually process
//   keyboard presses when the ladder is on the screen in that room. They
//   strangely also call kGetEvent. Because the main User::doit also calls
//   kGetEvent, it's pure luck, where the event will hit. It's the same issue
//   as in QfG1VGA. If you turn DOSBox to max cycles and click around for ego,
//   sometimes clicks also won't get registered. Strangely it's not nearly
//   as bad as in our sci, but these differences may be caused by timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent
//   call.
// Applies to at least: English PC-CD, German Floppy, English Mac
// Responsible method: lowerLadder::doit and highLadder::doit
// Fixes bug: #5060
static const uint16 freddypharkasSignatureLadderEvent[] = {
	0x39, SIG_MAGICDWORD,
	SIG_SELECTOR8(new),              // pushi new
	0x76,                            // push0
	0x38, SIG_SELECTOR16(curEvent),  // pushi curEvent
	0x76,                            // push0
	0x81, 0x50,                      // lag global[50]
	0x4a, 0x04,                      // send 04 - read User::curEvent
	0x4a, 0x04,                      // send 04 - call curEvent::new
	0xa5, 0x00,                      // sat temp[0]
	0x38, SIG_SELECTOR16(localize),  // pushi localize
	0x76,                            // push0
	0x4a, 0x04,                      // send 04 (call curEvent::localize)
	SIG_END
};

static const uint16 freddypharkasPatchLadderEvent[] = {
	0x34, 0x00, 0x00,                // ldi 0000 (waste 3 bytes, overwrites first 2 pushes)
	PATCH_ADDTOOFFSET(+8),
	0xa5, 0x00,                      // sat temp[0] (waste 2 bytes, overwrites 2nd send)
	PATCH_ADDTOOFFSET(+2),
	0x34, 0x00, 0x00,                // ldi 0000
	0x34, 0x00, 0x00,                // ldi 0000 (waste 6 bytes, overwrites last 3 opcodes)
	PATCH_END
};

// In the Macintosh version of Freddy Pharkas, kRespondsTo is broken for
// property selectors. They hacked the script to work around the issue,
// so we revert the script back to using the values of the DOS script.
// Applies to at least: English Mac
// Responsible method: publicfpInv::drawInvWindow in script 15
static const uint16 freddypharkasSignatureMacInventory[] = {
	SIG_MAGICDWORD,
	0x39, 0x23,                      // pushi 23
	0x39, 0x74,                      // pushi 74
	0x78,                            // push1
	0x38, SIG_UINT16(0x0174),        // pushi 0174 (on mac it's actually 0x01, 0x74)
	0x85, 0x15,                      // lat temp[15]
	SIG_END
};

static const uint16 freddypharkasPatchMacInventory[] = {
	0x39, 0x02,                      // pushi 02 (now matches the DOS version)
	PATCH_ADDTOOFFSET(+23),
	0x39, 0x04,                      // pushi 04 (now matches the DOS version)
	PATCH_END
};

// WORKAROUND
// FPFP Mac has an easter egg with a script bug that accidentally works in
//  Sierra's interpreter. Clicking Talk on a small part of the mine in room 270
//  triggers it. The script macThing plays macSound and waits for it to finish,
//  but macSound:loop is set to -1, indicating that it should loop forever.
//  ScummVM loops the sound and so macThing never advances to the next state and
//  the user never regains control. Sierra's interpreter cues the script after
//  the first play and doesn't loop the sound, despite macSound:loop.
//
// We work around this by setting macSound:loop correctly on the heap so that it
//  only plays once and macThing proceeds.
//
// This buggy script didn't break in the original because the Mac interpreter
//  didn't support looping sounds. It always played sounds just once and then 
//  signaled when they were complete and ignored the value of the loop property.
//  This was most apparent in the KQ6 Mac port. All the room music, which was
//  designed to loop, stopped abruptly after a minute in a room and then
//  jarringly returned when changing rooms.
//
// Applies to: Mac Floppy
// Responsible method: Heap in script 270
// Fixes bug #7065
static const uint16 freddypharkasSignatureMacEasterEgg[] = {
	SIG_MAGICDWORD,                 // macSound
	SIG_UINT16(0x0b89),             // number = 2953
	SIG_UINT16(0x007f),             // vol = 127
	SIG_UINT16(0x0000),             // priority = 0
	SIG_UINT16(0xffff),             // loop = -1 [ loop sound forever ]
	SIG_END
};

static const uint16 freddypharkasPatchMacEasterEgg[] = {
	PATCH_ADDTOOFFSET(+6),
	PATCH_UINT16(0x0001),           // loop = 1 [ play sound once ]
	PATCH_END
};

// FPFP Mac is missing view 844 of Hop Singh leaving town, breaking the scene.
//  This occurs when going to the desert (room 200) after the restaurant closes
//  but before act 3 ends. This would also crash the original so we just disable
//  this minor optional scene.
//
// Applies to: Mac Floppy
// Responsible method: rm200:init
// Fixes bug #10954
static const uint16 freddypharkasSignatureMacHopSingh[] = {
	0x89, 0x77,                     // lsg 77
	0x35, 0x13,                     // ldi 13
	0x1a,                           // eq? [ did restaurant just close? ]
	0x31, 0x46,                     // bnt 46 [ skip hop singh scene ]
	SIG_ADDTOOFFSET(+0x41),
	SIG_MAGICDWORD,
	0x72, 0x01, 0xd0,               // lofsa hopSingh [ hard-coded big endian for mac ]
	0x4a, 0x20,                     // send 20 [ hopSingh init: ... setScript: sLeaveTown ]
	SIG_END
};

static const uint16 freddypharkasPatchMacHopSingh[] = {
	0x33, 0x4b,                     // jmp 4b [ always skip hop singh scene ]
	PATCH_END
};

// At the start of act 4 the church key is removed from inventory but reappears
//  in the church door. The door script attempts to prevent this by not drawing
//  the key in act 4 but the verb handler is missing this check. Looking at the
//  door in act 4 still brings up the inset with the key. Sierra fixed this in
//  Mac but forgot to include the fix in the CD version a year later.
//
// We fix this by replacing a duplicate inventory check with an act 4 check so
//  that the key no longer appears in the inset and can't be picked up again.
//
// Applies to: PC Floppy, PC CD
// Responsible method: inDoorInset:init
// Fixes bug #10975
static const uint16 freddypharkasSignatureChurchKey[] = {
	SIG_MAGICDWORD,
	0x76,                           // push0
	0x59, 0x01,                     // &rest 01
	0x57, SIG_ADDTOOFFSET(+1), 0x04,// super Inset 04 [ super: init &rest ]
	0x38, SIG_SELECTOR16(has),      // pushi has
	0x78,                           // push1
	0x39, 0x06,                     // pushi 06
	0x81, 0x00,                     // lag 00
	0x4a, 0x06,                     // send 06 [ ego has: 6 (church key) ]
	SIG_END
};

static const uint16 freddypharkasPatchChurchKey[] = {
	PATCH_ADDTOOFFSET(+6),
	0x89, 0x78,                     // lsg 78 [ act number ]
	0x35, 0x04,                     // ldi 04
	0x20,                           // ge?
	0x33, 0x03,                     // jmp 03
	PATCH_END
};

// After leaving the desk letter in the grave, the letter reappears in the desk.
//  The desk script only checks if the letter is in inventory. Sierra started to
//  fix this in the CD version by setting a new flag but forgot to check it.
//
// We fix this by testing Letter's owner, if -1 then it is in the grave.
//
// Applies to: All versions
// Responsible method: deskDrawer:doVerb(1)
// Fixes bug #10975
static const uint16 freddypharkasSignatureDeskLetter[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0055),       // bnt 0055
	0x38, SIG_SELECTOR16(has),      // pushi has
	0x78,                           // push1
	0x39, 0x1f,                     // pushi 1f
	0x81, 0x00,                     // lag 00
	0x4a, 0x06,                     // send 06 [ ego has: 31 (Letter) ]
	0x18,                           // not
	0x31, 0x1f,                     // bnt 1f
	0x78,                           // push1
	0x39, 0x31,                     // pushi 31
	0x45, 0x02, 0x02,               // callb proc0_2 02 [ is flag 49 set? ]
	0x31, 0x17,                     // bnt 17
	0x38, SIG_ADDTOOFFSET(+2),      // pushi stopUpd
	0x76,                           // push0
	0x81, 0x00,                     // lag 00
	0x4a, 0x04,                     // send 04 [ ego stopUpd: (optimization) ]
	0x38, SIG_ADDTOOFFSET(+2),      // pushi setInset
	0x78,                           // push1
	0x72, SIG_UINT16(0x0522),       // lofsa inLetterInset
	0x36,                           // push
	0x81, 0x02,                     // lag 02
	0x4a, 0x06,                     // send 06  [ rm610 setInset: inLetterInset ]
	0x32, SIG_UINT16(0x008f),       // jmp 008f [ end of method ]
	0x78,                           // push1
	0x39, 0x31,                     // pushi 31
	0x45, 0x02, 0x02,               // callb proc0_2 02 [ is flag 49 set? ]
	0x31, 0x11,                     // bnt 11 [ drawer is closed ]
	SIG_END
};

static const uint16 freddypharkasPatchDeskLetter[] = {
	0x31, 0x56,                     // bnt 56
	0x78,                           // push1
	0x39, 0x31,                     // pushi 31
	0x45, 0x02, 0x02,               // callb proc0_2 02 [ is flag 49 set? ]
	0x31, 0x3e,                     // bnt 3e  [ drawer is closed ]
	0x38, PATCH_SELECTOR16(has),    // pushi has
	0x78,                           // push1
	0x39, 0x1f,                     // pushi 1f
	0x81, 0x00,                     // lag 00
	0x4a, 0x06,                     // send 06 [ ego has: 31 (Letter) ]
	0x2f, 0x21,                     // bt 21   [ drawer is open and empty ]
	0x39, PATCH_SELECTOR8(at),      // pushi at
	0x78,                           // push1
	0x39, 0x1f,                     // pushi 1f
	0x81, 0x09,                     // lag 09
	0x4a, 0x06,                     // send 06 [ fpInv at: 31 (Letter) ]
	0x38, PATCH_SELECTOR16(owner),  // pushi owner
	0x76,                           // push0
	0x4a, 0x04,                     // send 04 [ Letter owner? ]
	0x39, 0xff,                     // pushi ff
	0x1a,                           // eq?
	0x2f, 0x0d,                     // bt 0d   [ drawer is open and empty ]
	0x38, PATCH_GETORIGINALUINT16(+33), // pushi setInset
	0x78,                           // push1
	0x74, PATCH_UINT16(0x0522),     // lofss inLetterInset
	0x81, 0x02,                     // lag 02
	0x4a, 0x06,                     // send 06 [ rm610 setInset: inLetterInset ]
	0x3a,                           // toss
	0x48,                           // ret
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry freddypharkasSignatures[] = {
	{  true,     0, "CD: score early disposal",                    1, freddypharkasSignatureScoreDisposal, freddypharkasPatchScoreDisposal },
	{  true,    15, "Mac: broken inventory",                       1, freddypharkasSignatureMacInventory,  freddypharkasPatchMacInventory },
	{  true,   110, "intro scaling workaround",                    2, freddypharkasSignatureIntroScaling,  freddypharkasPatchIntroScaling },
	{  false,  200, "Mac: skip broken hop singh scene",            1, freddypharkasSignatureMacHopSingh,   freddypharkasPatchMacHopSingh },
	{  true,   235, "CD: canister pickup hang",                    3, freddypharkasSignatureCanisterHang,  freddypharkasPatchCanisterHang },
	{  true,   270, "Mac: easter egg hang",                        1, freddypharkasSignatureMacEasterEgg,  freddypharkasPatchMacEasterEgg },
	{  true,   310, "church key reappears",                        1, freddypharkasSignatureChurchKey,     freddypharkasPatchChurchKey },
	{  true,   320, "ladder event issue",                          2, freddypharkasSignatureLadderEvent,   freddypharkasPatchLadderEvent },
	{  true,   610, "desk letter reappears",                       1, freddypharkasSignatureDeskLetter,    freddypharkasPatchDeskLetter },
	{  true,   928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,          sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// During Bridge, Declarer_Second_NT:think performs a bitwise or against an
//  object due to a script typo. This operation is supposed to be against
//  (bridgeHand:highCard):rank but instead it's against bridgeHand:highCard.
//  ThirdSeat_Trump:think has a correct version of this. Declarer_Second_NT must
//  have just been missing the word "rank" in the original script.
//
// We fix this by inserting the missing rank code. To make room we remove the
//  call to self:checkFinCard. It's called immediately before this patch and its
//  result is stored in local1 so we just use that. Hoyle5 also has this bug.
//
// Applies to at least: English PC
// Responsible method: Declarer_Second_NT:think
// Fixes bug #11163
static const uint16 hoyle4SignatureBridgeArithmetic[] = {
	0x36,                               // push [ bridgeHand:highCard ]
	0x34, SIG_UINT16(0x0f00),           // ldi 0f00
	0x14,                               // or [ error: bridgeHand:highCard is an object ]
	0x36,                               // push
	0x63, 0x42,                         // pToa pard
	0x4a, 0x08,                         // send 08 [ pard hasCard: theSuitLead (bridgeHand:highCard | 0f00) ]
	SIG_MAGICDWORD,
	0x2f, 0x1d,                         // bt 1d
	0x83, 0x03,                         // lal 03
	0x2f, 0x19,                         // bt 19
	0x38, SIG_ADDTOOFFSET(+2),          // pushi rank
	0x76,                               // push0
	0x38, SIG_ADDTOOFFSET(+2),          // pushi checkFinCard
	0x78,                               // push1
	0x67, 0x48,                         // pTos theSuitLead
	0x54, 0x06,                         // self 06 [ self checkFinCard: theSuitLead ]
	SIG_END
};

static const uint16 hoyle4PatchBridgeArithmetic[] = {
	0x38, PATCH_GETORIGINALUINT16(+17), // pushi rank
	0x76,                               // push0
	0x4a, 0x04,                         // send 04 [ bridgeHand:highCard rank? ]
	0x36,                               // push
	0x34, PATCH_UINT16(0x0f00),         // ldi 0f00
	0x14,                               // or
	0x36,                               // push
	0x63, 0x42,                         // pToa pard
	0x4a, 0x08,                         // send 08 [ pard hasCard: theSuitLead ((bridgeHand:highCard):rank | 0f00) ]
	0x2f, 0x17,                         // bt 17
	0x83, 0x03,                         // lal 03
	0x2f, 0x13,                         // bt 13
	0x38, PATCH_GETORIGINALUINT16(+17), // pushi rank
	0x76,                               // push0
	0x83, 0x01,                         // lal 01 [ set to "self checkFinCard: theSuitLead" earlier ]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle4Signatures[] = {
	{  true,   733, "bridge arithmetic against object ",           1, hoyle4SignatureBridgeArithmetic,  hoyle4PatchBridgeArithmetic },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Hoyle 5

// Several scripts in Hoyle5 contain a subroutine which spins on kGetTime until
// a certain number of ticks elapse. Since this wastes CPU and makes ScummVM
// unresponsive, the kWait kernel function (which was removed in SCI2) is
// reintroduced for Hoyle5, and the spin subroutines are patched here to call
// that function instead.
// Applies to at least: English Demo
static const uint16 hoyle5SignatureSpinLoop[] = {
	SIG_MAGICDWORD,
	0x76,                         // push0
	0x43, 0x79, SIG_UINT16(0x00), // callk GetTime, $0
	0x36,                         // push
	0x87, 0x01,                   // lap param[1]
	0x02,                         // add
	0xa5, 0x00,                   // sat temp[0]
	SIG_END
};

static const uint16 hoyle5PatchSpinLoop[] = {
	0x78,                                     // push1
	0x8f, 0x01,                               // lsp param[1]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, $2
	0x48,                                     // ret
	PATCH_END
};

// While playing Old Maid (room 200), a repeated typo in the game script
// means that `setScale` is called accidentally instead of `setScaler`.
// In SSCI this did not do much because the first argument happened to be
// smaller than the y-position of `ego`, but in ScummVM the first argument is
// larger and so a debug message "y value less than vanishingY" is displayed.
// This is the same issue as with LSL6 hires.
static const uint16 hoyle5SetScaleSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setScale), // pushi setScale ($14b)
	0x38, SIG_UINT16(0x05),         // pushi 5
	0x51, 0x2c,                     // class Scaler
	SIG_END
};

static const uint16 hoyle5PatchSetScale[] = {
	0x38, PATCH_SELECTOR16(setScaler), // pushi setScaler ($14f)
	PATCH_END
};

// There are two derived collections of Hoyle Classic Games:
// 1) The Hoyle Children's Collection, which includes the following games:
// - Crazy Eights (script 100)
// - Old Maid (script 200)
// - Checkers (script 1200)
// 2) Hoyle Bridge, which includes the following games:
// - Bridge (script 700)
// In these two collections, the scripts for the other games have been removed.
// Choosing any other game than the above results in a "No script found" error.
// The original game did not show the game selection screen, as there were
// direct shortucts to each game.
// Since we do show the game selection screen, we remove all the games
// which from the ones below, which are not included in each version:
// - Crazy Eights (script 100)
// - Old Maid (script 200)
// - Hearts (script 300)
// - Gin Rummy (script 400)
// - Cribbage (script 500)
// - Klondike / Solitaire (script 600)
// - Bridge (script 700)
// - Poker (script 1100)
// - Checkers (script 1200)
// - Backgammon (script 1300)
static const uint16 hoyle5SignatureCrazyEights[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x9c, 0x01,      // lofsa chooseCrazy8s
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureOldMaid[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x2c, 0x02,      // lofsa chooseOldMaid
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureHearts[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0xdc, 0x03,      // lofsa chooseHearts
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureGinRummy[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0xbc, 0x02,      // lofsa chooseGinRummy
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureCribbage[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x4c, 0x03,      // lofsa chooseCribbage
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureKlondike[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0xfc, 0x04,      // lofsa chooseKlondike
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureBridge[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x6c, 0x04,      // lofsa chooseBridge
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignaturePoker[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x8c, 0x05,      // lofsa choosePoker
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureCheckers[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0x1c, 0x06,      // lofsa chooseCheckers
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5SignatureBackgammon[] = {
	SIG_MAGICDWORD,
	0x38, 0x8e, 0x00,      // pushi 008e
	0x76,                  // push0
	0x38, 0xf0, 0x02,      // pushi 02f0
	0x76,                  // push0
	0x72, 0xac, 0x06,      // lofsa chooseBackgammon
	0x4a, 0x08, 0x00,      // send  0008
	SIG_END
};

static const uint16 hoyle5PatchDisableGame[] = {
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	PATCH_END
};

// During Bridge, Declarer_Second_NT:think performs a bitwise or against an
//  object due to a script typo. This script bug is also in Hoyle4, see its
//  patch notes above for more detail.
//
// Applies to at least: English PC
// Responsible method: Declarer_Second_NT:think
// Fixes bug #11173
static const uint16 hoyle5SignatureBridgeArithmetic[] = {
	0x36,                               // push [ bridgeHand:highCard ]
	0x34, SIG_UINT16(0x0f00),           // ldi 0f00
	0x14,                               // or [ error: bridgeHand:highCard is an object ]
	0x36,                               // push
	0x63, 0x44,                         // pToa pard
	0x4a, SIG_UINT16(0x0008),           // send 08 [ pard hasCard: theSuitLead (bridgeHand:highCard | 0f00) ]
	0x2f, 0x26,                         // bt 26
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_MAGICDWORD,
	0x83, 0x03,                         // lal 03
	0x2f, 0x1f,                         // bt 1f
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x38,                               // pushi rank
	SIG_END
};

static const uint16 hoyle5PatchBridgeArithmetic[] = {
	0x38, PATCH_GETORIGINALUINT16(+24), // pushi rank
	0x76,                               // push0
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ bridgeHand:highCard rank? ]
	0x38, PATCH_UINT16(0x0f00),         // pushi 0f00
	0x14,                               // or
	0x36,                               // push
	0x63, 0x44,                         // pToa pard
	0x4a, PATCH_UINT16(0x0008),         // send 08 [ pard hasCard: theSuitLead ((bridgeHand:highCard):rank | 0f00) ]
	0x2f, 0x20,                         // bt 20
	0x83, 0x03,                         // lal 03
	0x2f, 0x1c,                         // bt 1c
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5Signatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   200, "fix setScale calls",                         11, hoyle5SetScaleSignature,          hoyle5PatchSetScale },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   733, "bridge arithmetic against object ",           1, hoyle5SignatureBridgeArithmetic,  hoyle5PatchBridgeArithmetic },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5ChildrensCollectionSignatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   200, "fix setScale calls",                         11, hoyle5SetScaleSignature,          hoyle5PatchSetScale },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   975, "disable Gin Rummy",                           1, hoyle5SignatureGinRummy,          hoyle5PatchDisableGame },
	{  true,   975, "disable Cribbage",                            1, hoyle5SignatureCribbage,          hoyle5PatchDisableGame },
	{  true,   975, "disable Klondike",                            1, hoyle5SignatureKlondike,          hoyle5PatchDisableGame },
	{  true,   975, "disable Bridge",                              1, hoyle5SignatureBridge,            hoyle5PatchDisableGame },
	{  true,   975, "disable Poker",                               1, hoyle5SignaturePoker,             hoyle5PatchDisableGame },
	{  true,   975, "disable Hearts",                              1, hoyle5SignatureHearts,            hoyle5PatchDisableGame },
	{  true,   975, "disable Backgammon",                          1, hoyle5SignatureBackgammon,        hoyle5PatchDisableGame },
	SCI_SIGNATUREENTRY_TERMINATOR
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5BridgeSignatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   733, "bridge arithmetic against object ",           1, hoyle5SignatureBridgeArithmetic,  hoyle5PatchBridgeArithmetic },
	{  true,   975, "disable Gin Rummy",                           1, hoyle5SignatureGinRummy,          hoyle5PatchDisableGame },
	{  true,   975, "disable Cribbage",                            1, hoyle5SignatureCribbage,          hoyle5PatchDisableGame },
	{  true,   975, "disable Klondike",                            1, hoyle5SignatureKlondike,          hoyle5PatchDisableGame },
	{  true,   975, "disable Poker",                               1, hoyle5SignaturePoker,             hoyle5PatchDisableGame },
	{  true,   975, "disable Hearts",                              1, hoyle5SignatureHearts,            hoyle5PatchDisableGame },
	{  true,   975, "disable Backgammon",                          1, hoyle5SignatureBackgammon,        hoyle5PatchDisableGame },
	{  true,   975, "disable Crazy Eights",                        1, hoyle5SignatureCrazyEights,       hoyle5PatchDisableGame },
	{  true,   975, "disable Old Maid",                            1, hoyle5SignatureOldMaid,           hoyle5PatchDisableGame },
	{  true,   975, "disable Checkers",                            1, hoyle5SignatureCheckers,          hoyle5PatchDisableGame },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 1

// `daySixBeignet::changeState(4)` is called when the cop goes outside. It sets
// cycles to 220. This is a CPU-speed dependent value and not usually enough
// time to get to the door, so patch it to 22 seconds.
//
// Applies to at least: English PC-CD, German PC-CD, English Mac
static const uint16 gk1Day6PoliceBeignetSignature1[] = {
	0x35, 0x04,                    // ldi 4
	0x1a,                          // eq?
	0x30, SIG_ADDTOOFFSET(+2),     // bnt [next state check]
	0x38, SIG_SELECTOR16(dispose), // pushi dispose
	0x76,                          // push0
	0x72, SIG_ADDTOOFFSET(+2),     // lofsa deskSarg
	0x4a, SIG_UINT16(0x04),        // send 4
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0xdc),        // ldi 220
	0x65, SIG_ADDTOOFFSET(+1),     // aTop cycles ($1a for PC, $1c for Mac)
	0x32,                          // jmp [end]
	SIG_END
};

static const uint16 gk1Day6PoliceBeignetPatch1[] = {
	PATCH_ADDTOOFFSET(+16),
	0x34, PATCH_UINT16(0x16),                   // ldi 22
	0x65, PATCH_GETORIGINALBYTEADJUST(+20, +2), // aTop seconds ($1c for PC, $1e for Mac)
	PATCH_END
};

// On day 6 when the cop is outside for the beignet, walking through the
// swinging door will also reset the puzzle timer so the player has 200 cycles
// to get through the area before the cop returns. This is a CPU-speed
// dependent value and not usually enough time to get to the door, so we patch
// it to 20 seconds instead.
//
// Applies to at least: English PC-CD, German PC-CD, English Mac
// Responsible method: sInGateWithPermission::changeState(0)
// Fixes bug: #9805
static const uint16 gk1Day6PoliceBeignetSignature2[] = {
	0x72, SIG_ADDTOOFFSET(+2),    // lofsa daySixBeignet
	0x1a,                         // eq?
	0x31, 0x0d,                   // bnt [skip set cycles]
	0x38, SIG_SELECTOR16(cycles), // pushi cycles
	0x78,                         // push1
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0xc8),       // pushi 200
	0x72,                         // lofsa
	SIG_END
};

static const uint16 gk1Day6PoliceBeignetPatch2[] = {
	PATCH_ADDTOOFFSET(+6),
	0x38, PATCH_SELECTOR16(seconds), // pushi seconds
	0x78,                            // push1
	0x38, PATCH_UINT16(0x14),        // pushi 20
	PATCH_END
};

// `sargSleeping::changeState(8)` is called when the cop falls asleep and sets
// the puzzle timer to 220 cycles. This is CPU-speed dependent and not usually
// enough time to get to the door, so patch it to 22 seconds instead.
//
// Applies to at least: English PC-CD, German PC-CD, English Mac
static const uint16 gk1Day6PoliceSleepSignature[] = {
	0x35, 0x08,                // ldi 8
	0x1a,                      // eq?
	0x31, SIG_ADDTOOFFSET(+1), // bnt [next state check]
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0xdc),    // ldi 220
	0x65, SIG_ADDTOOFFSET(+1), // aTop cycles ($1a for PC, $1c for Mac)
	0x32,                      // jmp [end]
	SIG_END
};

static const uint16 gk1Day6PoliceSleepPatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x34, PATCH_UINT16(0x16),                  // ldi 22
	0x65, PATCH_GETORIGINALBYTEADJUST(+9, +2), // aTop seconds (1c for PC, 1e for Mac)
	PATCH_END
};

// The beignet vendor's speed at the police station is CPU dependent, causing
//  him to zoom to his position in room 230 on modern machines at an unintended
//  high speed that's inconsistent with the room. Unlike the other actors,
//  theVendor:moveSpeed is set to 0, which advances his motion on every game
//  cycle instead of tethering his speed to ticks. On a machine from 1993 this
//  was much slower which is why his movement sound effect is five seconds long.
//
// We fix this by setting theVendor:moveSpeed to a time-based speed that matches
//  the room scene. This doesn't affect puzzle timing, it's just cosmetic.
//
// Applies to: All versions
// Responsible method: heap in script 231
// Fixes bug: #11892
static const uint16 gk1BeignetVendorSpeedSignature[] = {
	SIG_MAGICDWORD,             // theVendor
	SIG_UINT16(0x0006),         // xStep = 6
	SIG_UINT16(0x0302),         // origStep = 770
	SIG_UINT16(0x0000),         // moveSpeed = 0
	SIG_END
};

static const uint16 gk1BeignetVendorSpeedPatch[] = {
	PATCH_ADDTOOFFSET(+4),
	PATCH_UINT16(0x0006),       // moveSpeed = 6
	PATCH_END
};

// At the start of day 5, when the player already has the veve but still needs
// to get the drum book, the drum book dialogue with Grace is played twice in
// a row, and then the veve dialogue gets played again even though it was
// already played during day 4.
//
// The duplicate drum book dialogue happens because it is triggered once in
// `GetTheVeve::changeState(0)` and then again in `GetTheVeve::changeState(11)`.
// The re-run of the veve dialogue happens because the game gives the player
// the drum book in `GetTheVeVe::changeState(1)`, then *after* doing so, checks
// if the player has the drum book and runs the veve dialogue if so.
//
// We fix both of these issues by skipping the has-drum-book check if the player
// just got the drum book in 'GetTheVeve::changeState(1)'.
// Doing this causes the game to jump from state 1 to state 12, which bypasses
// the duplicate drum book dialogue in state 11, as well as the veve dialogue
// trigger in the has-drum-book check.
//
// More notes: The veve newspaper item is inventory 9. The drum book is
//             inventory 14. The flag for veve research is 36, the flag for drum
//             research is 73.
//
// Special thanks, credits and kudos to sluicebox on IRC, who did a ton of
// research on this and even found this game bug originally.
//
// Applies to at least: English PC-CD, German PC-CD
static const uint16 gk1Day5DrumBookDialogueSignature[] = {
	0x31, 0x0b,                         // bnt [skip giving player drum book code]
	0x38, SIG_SELECTOR16(get),          // pushi get ($200)
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x0e,                         // pushi $e
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x06),             // send 6 - GKEgo::get($e)
	// end of giving player drum book code
	0x38, SIG_SELECTOR16(has),          // pushi has ($202)
	0x78,                               // push1
	0x39, 0x0e,                         // pushi $e
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x06),             // send 6 - GKEgo::has($e)
	0x18,                               // not
	0x30, SIG_UINT16(0x25),             // bnt [veve newspaper code]
	SIG_END
};

static const uint16 gk1Day5DrumBookDialoguePatch[] = {
	0x31, 0x0d,                         // bnt [skip giving player drum book code] adjusted
	PATCH_ADDTOOFFSET(+11),             // skip give player drum book original code
	0x33, 0x0d,                         // jmp [over the check inventory for drum book code]
	// check inventory for drum book
	0x38, PATCH_SELECTOR16(has),        // pushi has ($202)
	0x78,                               // push1
	0x39, 0x0e,                         // pushi $e
	0x81, 0x00,                         // lag global[0]
	0x4a, PATCH_UINT16(0x0006),         // send 6 - GKEgo::has($e)
	0x2f, 0x23,                         // bt [veve newspaper code] (adjusted, saves 2 bytes)
	PATCH_END
};

// When Gabriel goes to the phone, the script softlocks at
// `startOfDay5::changeState(32)`.
//
// Applies to at least: English PC-CD, German PC-CD, English Mac
static const uint16 gk1Day5PhoneFreezeSignature[] = {
	0x4a,                             // send ...
	SIG_MAGICDWORD, SIG_UINT16(0x0c), // ... $c
	0x35, 0x03,                       // ldi 3
	0x65, SIG_ADDTOOFFSET(+1),        // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),        // jmp [end]
	0x3c,                             // dup
	0x35, 0x21,                       // ldi $21
	SIG_END
};

static const uint16 gk1Day5PhoneFreezePatch[] = {
	PATCH_ADDTOOFFSET(+3),                     // send $c
	0x35, 0x06,                                // ldi 6
	0x65, PATCH_GETORIGINALBYTEADJUST(+6, +6), // aTop ticks
	PATCH_END
};

// When Gabriel is grabbing a vine, his saying "I can't believe I'm doing
// this..." is cut off. We change it so the scripts wait for the audio.
//
// This is not supposed to be applied to the Floppy version.
//
// Applies to at least: English PC-CD, German PC-CD, Spanish PC-CD
// Responsible method: vineSwing::changeState(1)
// Fixes bug: #9820
static const uint16 gk1Day9VineSwingSignature[] = {
	0x38, SIG_UINT16(0x0004),         // pushi $4
	0x51, 0x17,                       // class CT
	0x36,                             // push
	0x39, 0x0b,                       // pushi $b
	0x78,                             // push1
	0x7c,                             // pushSelf
	0x81, 0x00,                       // lag global[$0]
	0x4a, SIG_UINT16(0x0020),         // send $20
	0x38, SIG_SELECTOR16(setMotion),  // pushi setMotion
	0x78,                             // push1
	0x76,                             // push0
	0x72, SIG_UINT16(0x0412),         // lofsa guard1
	0x4a, SIG_UINT16(0x0006),         // send $6
	0x38, SIG_SELECTOR16(say),        // pushi say
	0x38, SIG_UINT16(0x0004),         // pushi $4
	SIG_MAGICDWORD,
	0x39, 0x07,                       // pushi $7
	0x39, 0x08,                       // pushi $8
	0x39, 0x10,                       // pushi $10
	0x78,                             // push1
	0x81, 0x5b,                       // lag global[$5b]
	0x4a, SIG_UINT16(0x000c),         // send $c
	SIG_END
};

static const uint16 gk1Day9VineSwingPatch[] = {
	0x38, PATCH_UINT16(0x0003),         // pushi $3
	0x51, 0x17,                         // class CT
	0x36,                               // push
	0x39, 0x0b,                         // pushi $b
	0x78,                               // push1
	0x81, 0x00,                         // lag global[$0]
	0x4a, PATCH_UINT16(0x001e),         // send $20
	0x38, PATCH_SELECTOR16(setMotion),  // pushi setMotion
	0x78,                               // push1
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0412),         // lofsa guard1
	0x4a, PATCH_UINT16(0x0006),         // send $6
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x38, PATCH_UINT16(0x0005),         // pushi $5
	0x39, 0x07,                         // pushi $7
	0x39, 0x08,                         // pushi $8
	0x39, 0x10,                         // pushi $10
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[$5b]
	0x4a, PATCH_UINT16(0x000e),         // send $c
	PATCH_END
};

// The mummies on day 9 move without animating if ego exits to the north before
//  the first one finishes standing. This also occurs in Sierra's interpreter.
//
// The 12 outer rooms of the African mound all take place in room 710, which
//  reinitializes its contents on each room change. Each room's mummy is guard1
//  repositioned with a different view. When the mummies come to life,
//  keyWorks:changeState(6) starts guard1's standing animation after which
//  state 7 initializes guard1 for chasing ego. Ego however can leave before
//  guard1 finishes standing, preventing state 7 from occurring. The script for
//  exiting to the north assumes state 7 has run, otherwise guard1 remains on
//  the wrong view with no cycler or looper in subsequent rooms.
//
// This bug is due to the script rightWay only partially initializing guard1 for
//  chasing as opposed to wrongWay and backTrack which fully initialize. We fix
//  this by replacing rightWay's partial initialization with the full version
//  from keyWorks state 7. There are two versions of this patch due to
//  significant differences between floppy and CD versions of this script.
//
// This patch is not applied to the NRS versions of this script, which address
//  this bug by disabling control until guard1 finishes standing, giving the
//  player less time to escape.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac
// Responsible method: rightWay:changeState(1)
// Fixes bug: #10828
static const uint16 gk1MummyAnimateFloppySignature[] = {
	0x39, SIG_SELECTOR8(view),          // pushi view [ full guard1 init ]
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x02c5),           // pushi 709d
	SIG_ADDTOOFFSET(+674),
	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion [ partial guard1 init ]
	0x38, SIG_UINT16(0x0004),           // pushi 0004
	0x51, 0x70,                         // class PChase
	0x36,                               // push
	0x89, 0x00,                         // lsg global[0]
	0x39, 0x0f,                         // pushi 0f
	SIG_END
};

static const uint16 gk1MummyAnimateFloppyPatch[] = {
	PATCH_ADDTOOFFSET(+680),
	0x39, PATCH_SELECTOR8(view),        // pushi view [ waste 6 stack items to be compatible with ]
	0x76,                               // push0      [  the send instruction in full guard1 init ]
	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x39, 0x00,                         // pushi 00
	0x32, PATCH_UINT16(0xfd4b),         // jmp -693d [ continue full guard1 init in keyWorks state 7 ]
	PATCH_END
};

static const uint16 gk1MummyAnimateCDSignature[] = {
	0x39, SIG_SELECTOR8(view),          // pushi view [ full guard1 init ]
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x02c5),           // pushi 709d
	SIG_ADDTOOFFSET(+750),
	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion [ partial guard1 init ]
	0x38, SIG_UINT16(0x0004),           // pushi 0004
	0x51, 0x70,                         // class PChase
	0x36,                               // push
	0x89, 0x00,                         // lsg global[0]
	0x39, 0x0f,                         // pushi 0f
	SIG_END
};

static const uint16 gk1MummyAnimateCDPatch[] = {
	PATCH_ADDTOOFFSET(+756),
	0x39, PATCH_SELECTOR8(view),        // pushi view [ waste 6 stack items to be compatible with ]
	0x76,                               // push0      [  the send instruction in full guard1 init ]
	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x39, 0x00,                         // pushi 00
	0x32, PATCH_UINT16(0xfcff),         // jmp -769d [ continue full guard1 init in keyWorks state 7 ]
	PATCH_END
};

// In GK1, the `view` selector is used to store view numbers in some cases and
// object references to Views in other cases. `Interrogation::dispose` compares
// an object stored in the `view` selector with a number (which is not valid)
// because its checks are in the wrong order. The check order was fixed in the
// CD version, so just do what the CD version does.
//
// TODO: Check if English Mac is affected too and if this patch applies
// Applies to at least: English Floppy
static const uint16 gk1InterrogationBugSignature[] = {
	SIG_MAGICDWORD,
	0x65, 0x4c,                      // aTop $4c
	0x67, 0x50,                      // pTos $50
	0x34, SIG_UINT16(0x2710),        // ldi $2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 8 [05a0]
	0x67, 0x50,                      // pTos $50
	0x34, SIG_UINT16(0x2710),        // ldi $2710
	0x04,                            // sub
	0x65, 0x50,                      // aTop $50
	0x63, 0x50,                      // pToa $50
	0x31, 0x15,                      // bnt $15 [05b9]
	0x39, SIG_SELECTOR8(view),       // pushi view ($e)
	0x76,                            // push0
	0x4a, SIG_UINT16(0x04),          // send 4
	0xa5, 0x00,                      // sat temp[0]
	0x38, SIG_SELECTOR16(dispose),   // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa $50
	0x4a, SIG_UINT16(0x04),          // send 4
	0x85, 0x00,                      // lat temp[0]
	0x65, 0x50,                      // aTop $50
	SIG_END
};

static const uint16 gk1InterrogationBugPatch[] = {
	0x65, 0x4c,                      // aTop $4c
	0x63, 0x50,                      // pToa $50
	0x31, 0x15,                      // bnt $15 [05b9]
	0x39, PATCH_SELECTOR8(view),     // pushi view ($e)
	0x76,                            // push0
	0x4a, PATCH_UINT16(0x04),        // send 4
	0xa5, 0x00,                      // sat temp[0]
	0x38, PATCH_SELECTOR16(dispose), // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa $50
	0x4a, PATCH_UINT16(0x04),        // send 4
	0x85, 0x00,                      // lat temp[0]
	0x65, 0x50,                      // aTop $50
	0x67, 0x50,                      // pTos $50
	0x34, PATCH_UINT16(0x2710),      // ldi $2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 8 [05b9]
	0x67, 0x50,                      // pTos $50
	0x34, PATCH_UINT16(0x2710),      // ldi $2710
	0x04,                            // sub
	0x65, 0x50,                      // aTop $50
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm.
// In Madame Cazanoux's house, when Gabriel is leaving, he is placed on
// the edge of the walkable area initially. This leads to a failure in
// the pathfinding algorithm, and the pathfinding area is then ignored,
// so Gabriel goes straight to the door by walking through the wall.
// This is an edge case, which was apparently acceptable in SSCI. We
// change the upper border of the walk area slightly, so that Gabriel
// can be placed inside, and the pathfinding algorithm works correctly.
//
// Responsible method: rm280:init
// Fixes bug: #9770
static const uint16 gk1CazanouxPathfindingSignature[] = {
	SIG_MAGICDWORD,
	0x78,                            // push1 x = 1
	0x38, SIG_UINT16(0x0090),        // pushi y = 144
	0x38, SIG_UINT16(0x00f6),        // pushi x = 246
	0x38, SIG_UINT16(0x0092),        // pushi y = 146
	0x38, SIG_UINT16(0x00f2),        // pushi x = 242
	0x39, 0x69,                      // pushi y = 105
	0x39, 0x7c,                      // pushi x = 124
	0x39, 0x68,                      // pushi y = 104
	0x39, 0x56,                      // pushi x = 86
	0x39, 0x6f,                      // pushi y = 111
	0x39, 0x45,                      // pushi x = 69
	0x39, 0x7c,                      // pushi y = 124
	0x39, 0x2e,                      // pushi x = 46
	0x38, SIG_UINT16(0x0081),        // pushi y = 129
	SIG_END
};

static const uint16 gk1CazanouxPathfindingPatch[] = {
	PATCH_ADDTOOFFSET(+15),
	0x39, 0x7c,                      // pushi x = 124
	0x39, 0x67,                      // pushi y = 103 (was 104)
	PATCH_END
};

// GK1 english pc floppy locks up on day 10 in the honfour (room 800) when
//  using the keycard on an unlocked door's keypad. This is due to mistakenly
//  calling handsOff instead of handsOn. Sierra fixed this in floppy patch 1.0a
//  and all other versions.
//
// We fix this by changing handsOff to handsOn and passing 0 as the caller
//  to gkMessager:say since the script disposes itself.
//
// Applies to: English PC Floppy only
// Responsible method: sUnlockDoor:changeState(2)
// Fixes bug: #10767
static const uint16 gk1HonfourUnlockDoorSignature[] = {
	0x7c,                           // pushSelf
	0x81, 0x5b,                     // lag global[5b]
	0x4a, SIG_MAGICDWORD,           // send e [ gkMessager:say ... self ]
	SIG_UINT16(0x000e),
	0x38, SIG_UINT16(0x0216),       // pushi 0216 [ handsOff ]
	SIG_END
};

static const uint16 gk1HonfourUnlockDoorPatch[] = {
	0x76,                           // push0
	0x81, 0x5b,                     // lag global[5b]
	0x4a, PATCH_UINT16(0x000e),     // send e [ gkMessager:say ... 0 ]
	0x38, PATCH_UINT16(0x0217),     // pushi 0217 [ handsOn ]
	PATCH_END
};

// GK1 english pc floppy locks up on day 2 when using the binoculars to view
//  room 410 when the artist's drawing blows away. This is particularly bad
//  because when using the binoculars you can't use the mouse to access the
//  control panel to restore.
//
// We fix this as Sierra did in later versions by not allowing the drawing to
//  blow away when viewing through binoculars. To make room for this patch
//  we remove initializing juggler:cycleSpeed to 6 as this is redundant.
//  juggler is a Prop and Prop:cycleSpeed's initial value is 6.
//
// Applies to: English PC Floppy
// Responsible method: neJackson:init
// Fixes bug: #10797
static const uint16 gk1Day2BinocularsLockupSignature[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x01d6),           // bnt 01d6 [ english pc floppy 1.0 only ]
	0x38, SIG_SELECTOR16(init),         // pushi init
	0x76,                               // push0
	0x38, SIG_SELECTOR16(cycleSpeed),   // pushi cycleSpeed
	0x78,                               // push1
	0x39, 0x06,                         // pushi 06
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x78,                               // push1
	0x51, 0x15,                         // class Fwd
	0x36,                               // push
	0x72, SIG_UINT16(0x02b0),           // lofsa juggler
	0x4a, SIG_UINT16(0x0010),           // send 10 [ juggler: init, cycleSpeed: 6, setCycle: Fwd ]
	0x38, SIG_SELECTOR16(init),         // pushi init
	0x76,                               // push0
	0x72, SIG_UINT16(0x0538),           // lofsa easel
	0x4a, SIG_UINT16(0x0004),           // send 4 [ easel: init ]
	SIG_END
};

static const uint16 gk1Day2BinocularsLockupPatch[] = {
	PATCH_ADDTOOFFSET(+6),
	0x3c,                               // dup
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x78,                               // push1
	0x51, 0x15,                         // class Fwd
	0x36,                               // push
	0x72, PATCH_UINT16(0x02b0),         // lofsa juggler
	0x4a, PATCH_UINT16(0x000a),         // send a [ juggler: init, setCycle Fwd ]
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0538),         // lofsa easel
	0x4a, PATCH_UINT16(0x0004),         // send 4 [ easel: init ]

	0x89, 0x0c,                         // lsg global[0c] [ previous room ]
	0x34, PATCH_UINT16(0x0190),         // ldi 0190 [ overlook ]
	0x1c,                               // ne?
	0x31, 0x09,                         // bnt 09 [ drawing doesn't blow away ]
	PATCH_END
};

// GK1 english pc floppy has a missing-points bug on day 5 in room 240.
//  Showing Mosely the veve sketch and Hartridge's notes awards 2 points
//  but not if you show the notes before the veve.
//  Sierra fixed this in floppy patch 1.0b and all other versions.
//
// We fix this by awarding 2 points when showing the veve second.
//
// Applies to: English PC Floppy
// Responsible method: showMoselyPaper:changeState(5)
// Fixes bug: #10763
static const uint16 gk1Day5MoselyVevePointsSignature[] = {
	0x78,                                   // push1
	0x39, 0x1b,                             // pushi 1b
	0x47, 0x0d, 0x00, SIG_UINT16(0x0002),   // calle [export 0 of script 13], 02 [ is flag 1b set? ]
	0x30, SIG_UINT16(0x001e),               // bnt 001e [ haven't shown notes yet ]
	0x78,                                   // push1
	0x39, 0x1a,                             // pushi 1a
	0x47, 0x0d, 0x01, SIG_UINT16(0x0002),   // calle [export 1 of script 13], 02 [ set flag 1a ]
	0x38, SIG_UINT16(0x00f2),               // pushi 00f2 [ say ]
	0x38, SIG_UINT16(0x0005),               // pushi 0005
	0x39, 0x11,                             // pushi 11 [ noun ]
	SIG_MAGICDWORD,
	0x39, 0x10,                             // pushi 10 [ verb ]
	0x39, 0x38,                             // pushi 38 [ cond ]
	0x76,                                   // push0
	0x7c,                                   // pushSelf
	0x81, 0x5b,                             // lag global[5b] [ GkMessager ]
	0x4a, SIG_UINT16(0x000e),               // send 000e [ GkMessager:say ]
	0x32, SIG_UINT16(0x0013),               // jmp 0013
	0x38, SIG_UINT16(0x00f2),               // pushi 00f2 [ say ]
	SIG_END
};

static const uint16 gk1Day5MoselyVevePointsPatch[] = {
	0x38, PATCH_UINT16(0x00f2),             // pushi 00f2 [ say ]
	0x39, 0x05,                             // pushi 05
	0x39, 0x11,                             // pushi 11 [ noun ]
	0x39, 0x10,                             // pushi 10 [ verb ]
	0x78,                                   // push1
	0x39, 0x1b,                             // pushi 1b
	0x47, 0x0d, 0x00, PATCH_UINT16(0x0002), // calle [export 0 of script 13], 02 [ is flag 1b set? ]
	0x31, 0x20,                             // bnt 20 [ pushi 37, continue GkMessager:say ]
	0x38, PATCH_UINT16(0x02fa),             // pushi 02fa [ getPoints ]
	0x7a,                                   // push2
	0x38, PATCH_UINT16(0xfc19),             // pushi fc19 [ no flag ]
	0x7a,                                   // push2 [ 2 points ]
	0x81, 0x00,                             // lag global[0]
	0x4a, PATCH_UINT16(0x0008),             // send 8 [ GKEgo:getPoints -999 2 ]
	0x78,                                   // push1
	0x39, 0x1a,                             // pushi 1a
	0x47, 0x0d, 0x01, PATCH_UINT16(0x0002), // calle [export 1 of script 13], 02 [ set flag 1a ]
	0x39, 0x38,                             // pushi 38 [ cond ]
	0x33, 0x09,                             // jmp 9 [ continue GkMessager:say ]
	PATCH_END
};

// When turning on the museum's air conditioner prior to day 5 in room 260, the
//  timing is off and speech is interrupted. Some parts of the sequence run at
//  game speed and others don't, which at high speeds eliminates pauses between
//  dialogue. Dr. John's "We have air conditioning, you see" speech is cut off
//  at all speeds.
//
// We fix this by setting ego's speed to its default (6) during this sequence
//  and waiting for the messages to complete before proceeding. flipTheSwitch
//  restores ego's speed at the end of the script, even though it never sets it.
//
// Applies to: All CD versions
// Responsible method: flipTheSwitch:changeState
// Fixes bug: #11219
static const uint16 gk1AirConditionerSpeechSignature[] = {
	0x30, SIG_UINT16(0x0020),               // bnt 0020 [ state 1 ]
	SIG_ADDTOOFFSET(+26),
	0x4a, SIG_UINT16(0x000c),               // send 0c
	0x32, SIG_UINT16(0x0409),               // jmp 0409 [ end of method ]
	0x3c,                                   // dup
	0x35, SIG_MAGICDWORD, 0x01,             // ldi 01
	0x1a,                                   // eq?
	0x30, SIG_UINT16(0x0056),               // bnt 0056 [ state 2 ]
	SIG_ADDTOOFFSET(+24),
	0x4a, SIG_UINT16(0x001a),               // send 1a [ GKEgo view: 265 ... ]
	SIG_ADDTOOFFSET(+33),
	0x4a, SIG_UINT16(0x000c),               // send 0c
	0x32, SIG_UINT16(0x03c0),               // jmp 03c0 [ end of method ]
	SIG_ADDTOOFFSET(+620),
	0x7a,                                   // push2
	SIG_ADDTOOFFSET(+3),
	0x7c,                                   // pushSelf
	0x81, 0x00,                             // lag 00
	0x4a, SIG_UINT16(0x0014),               // send 14 [ GKEgo ... setCycle: End self ]
	SIG_ADDTOOFFSET(+8),
	0x38, SIG_UINT16(0x0004),               // pushi 0004
	SIG_ADDTOOFFSET(+10),
	0x4a, SIG_UINT16(0x000c),               // send 0c  [ gkMessager say: 28 8 7 4 ]
	0x32, SIG_UINT16(0x012f),               // jmp 012f [ end of method ]
	SIG_ADDTOOFFSET(+3),
	0x38, SIG_UINT16(0x0004),               // pushi 0004
	SIG_ADDTOOFFSET(+9),
	0x4a, SIG_UINT16(0x000c),               // send 0c  [ gkMessager say: 28 8 8 4 ]
	0x32, SIG_UINT16(0x011a),               // jmp 011a [ end of method ]
	SIG_ADDTOOFFSET(+6),
	0x38, SIG_SELECTOR16(stop),             // pushi stop [ stop snake sound ]
	SIG_END
};

static const uint16 gk1AirConditionerSpeechPatch[] = {
	0x30, PATCH_UINT16(0x001c),             // bnt 001c [ state 1 ]
	PATCH_ADDTOOFFSET(+26),
	0x33, 0x47,                             // jmp 47 [ send 0c / end of method ]
	0x3c,                                   // dup
	0x18,                                   // not
	0x1a,                                   // eq?
	0x31, 0x5c,                             // bnt 5c [ state 2 ]
	0x38, PATCH_SELECTOR16(cycleSpeed),     // pushi cycleSpeed
	0x78,                                   // push1
	0x39, 0x06,                             // pushi 06
	PATCH_ADDTOOFFSET(+24),
	0x4a, PATCH_UINT16(0x0020),             // send 20 [ GKEgo cycleSpeed: 6 view: 265 ... ]
	PATCH_ADDTOOFFSET(+659),
	0x78,                                   // push1
	PATCH_ADDTOOFFSET(+3),
	0x80, PATCH_UINT16(0x0000),             // lag 0000
	0x4a, PATCH_UINT16(0x0012),             // send 12 [ GKEgo ... setCycle: End ]
	PATCH_ADDTOOFFSET(+8),
	0x38, PATCH_UINT16(0x0005),             // pushi 0005
	PATCH_ADDTOOFFSET(+10),
	0x7c,                                   // pushSelf
	0x4a, PATCH_UINT16(0x000e),             // send 0e [ gkMessager say: 28 8 7 4 self ]
	0x33, 0x1b,                             // jmp 1b  [ stop snake sound ]
	PATCH_ADDTOOFFSET(+3),
	0x38, PATCH_UINT16(0x0005),             // pushi 0005
	PATCH_ADDTOOFFSET(+9),
	0x7c,                                   // pushSelf
	0x4a, PATCH_UINT16(0x000e),             // send 0e [ gkMessager say: 28 8 8 4 self ]
	0x33, 0x06,                             // jmp 06  [ stop snake sound ]
	PATCH_END
};

// The day 5 snake attack has speed, audio, and graphics problems.
//  These occur in all versions and also in Sierra's interpreter.
//
// Gabriel automatically walks cautiously in the darkened museum while looking
//  around and saying lines, then a snake drops on him. Depending on the game's
//  speed setting, the audio for "Why is it so dark in here?" is interrupted as
//  much as halfway through by the next line, "Dr. John, hello?". The cautious
//  walk animation runs at game speed, which can be fast, then abruptly changes
//  to 10 (33%) when the snake drops, which looks off. Ego doesn't even reach
//  the snake and instead stops short and warps 17 pixels to the right when the
//  drop animation starts.
//
// We fix all of this. Initializing ego's speed to 10 solves the interrupted
//  speech and inconsistent speed. It feels like this was the intended pacing.
//  The snake-warping isn't a speed issue, ego's animation frames for this
//  scene simply fall short of the snake's location. To fix that we start ego
//  a little farther in the room and increase ego's final position so that he
//  ends up directly under the snake and transitions to the drop animation
//  smoothly. Finally, we initialize ego on the room's first cycle instead of
//  second so that ego doesn't materialize after the room is already displayed.
//
// This patch works with pc floppy and cd even though they have different
//  snakeAttack scripts. Floppy doesn't have speech to interrupt but it
//  has the same issues.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: snakeAttack:changeState
// Fixes bug: #10793
static const uint16 gk1Day5SnakeAttackSignature1[] = {
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 1
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0048),           // bnt 0048 [ state 2 ]
	0x35, 0x01,                         // ldi 1 [ free bytes ]
	0x39, SIG_SELECTOR8(view),          // pushi view
	0x78,                               // push1
	0x38, SIG_UINT16(0x0107),           // pushi 0107
	0x38, SIG_SELECTOR16(setCel),       // pushi setCel
	0x78,                               // push1
	0x76,                               // push0
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x76,                               // push0
	0x39, SIG_SELECTOR8(signal),        // pushi signal
	0x78,                               // push1
	0x39, SIG_SELECTOR8(signal),        // pushi signal
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x0004),           // send 4 [ GKEgo:signal? ]
	SIG_ADDTOOFFSET(+18),
	0x39, 0x64,                         // pushi 64 [ initial x ]
	SIG_END
};

static const uint16 gk1Day5SnakeAttackPatch1[] = {
	0x39, PATCH_SELECTOR8(view),        // pushi view [ begin initializing ego in state 0 ]
	0x78,                               // push1
	0x33, 0x07,                         // jmp 07 [ continue initializing ego in state 0 ]
	0x3c,                               // dup
	0x18,                               // not [ acc = 1 ]
	0x1a,                               // eq?
	0x65, 0x1a,                         // aTop cycles [ just set cycles to 1 in state 1 ]
	0x33, 0x48,                         // jmp 47 [ state 2 ]
	0x38, PATCH_UINT16(0x0107),         // pushi 0107
	0x39, PATCH_SELECTOR8(cel),         // pushi cel
	0x78,                               // push1
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(setLoop),    // pushi setLoop
	0x78,                               // push1
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(signal),      // pushi signal
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0a
	PATCH_ADDTOOFFSET(+5),
	0x4a, PATCH_UINT16(0x000a),         // send a [ GKEgo:signal?, cycleSpeed = a ]
	PATCH_ADDTOOFFSET(+18),
	0x39, 0x70,                         // pushi 70 [ new initial x ]
	PATCH_END
};

// This just changes ego's second x coordinate but unfortunately that promotes it to 16 bits
static const uint16 gk1Day5SnakeAttackSignature2[] = {
	SIG_MAGICDWORD,
	0x39, 0x7a,                         // pushi 7a [ x for second walking loop ]
	0x39, 0x7c,                         // pushi 7c
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x0022),           // send 22
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	SIG_END
};

static const uint16 gk1Day5SnakeAttackPatch2[] = {
	0x38, PATCH_UINT16(0x008b),         // pushi 008b [ new x for second walking loop ]
	0x39, 0x7c,                         // pushi 7c
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	0x4a, PATCH_UINT16(0x0022),         // send 22
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_END
};

// When entering the police station (room 230) sGabeEnters sets ego speed
//  to 4 for the door animation but fails to restore it to the game speed
//  by calling GKEgo:normalize. This leaves ego at 75% speed until doing
//  something that does call normalize.
//
// We fix this by calling GKEgo:normalize after Gabriel finishes walking
//  through the door in sGabeEnters:changeState(4). This replaces setting
//  GKEgo:ignoreActors to 0 but that's okay because normalize does that.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: sGabeEnters:changeState(4)
// Fixes bug: #10780
static const uint16 gk1PoliceEgoSpeedFixSignature[] = {
	0x38, SIG_MAGICDWORD,               // pushi ignoreActors
	      SIG_SELECTOR16(ignoreActors),
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x000c),           // send c [ GKEgo: ..., ignoreActors: 0 ]
	SIG_END
};

static const uint16 gk1PoliceEgoSpeedFixPatch[] = {
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x39, 0x00,                         // pushi 00
	0x81, 0x00,                         // lag global[0]
	0x4a, PATCH_UINT16(0x000a),         // send a [ GKEgo: ..., normalize ]
	PATCH_END
};

// When exiting the drugstore (room 250) egoExits sets ego speed to 15
//  (slowest) for the door animation but fails to restore it to game
//  speed by calling GKEgo:normalize. This leaves ego slow until doing
//  something that does call normalize.
//
// We fix this by calling GKEgo:normalize after the door animation.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: egoExits:changeState
// Fixes bug: #10780
static const uint16 gk1DrugStoreEgoSpeedFixSignature[] = {
	0x30, SIG_UINT16(0x003f),           // bnt 003f [ state 1 ]
	SIG_ADDTOOFFSET(+60),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x0012),           // jmp 12 [ end of method ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt c [ end of method ]
	0x38, SIG_SELECTOR16(newRoom),      // pushi newRoom
	0x78,                               // push1
	0x38, SIG_UINT16(0x00c8),           // pushi 00c8 [ map ]
	0x81, 0x02,                         // lag global[2]
	0x4a, SIG_UINT16(0x0006),           // send 6 [ rm250:newRoom = map ]
	0x3a,                               // toss
	SIG_END
};

static const uint16 gk1DrugStoreEgoSpeedFixPatch[] = {
	0x3a,                               // toss
	0x31, 0x3d,                         // bnt 3d [ state 1 ]
	PATCH_ADDTOOFFSET(+60),
	0x48,                               // ret
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, PATCH_UINT16(0x0004),         // send 4 [ GKEgo:normalize ]
	0x38, PATCH_SELECTOR16(newRoom),    // pushi newRoom
	0x78,                               // push1
	0x38, PATCH_UINT16(0x00c8),         // pushi 00c8 [ map ]
	0x81, 0x02,                         // lag global[2]
	0x4a, PATCH_UINT16(0x0006),         // send 6 [ rm250:newRoom = map ]
	PATCH_END
};

// GK1 CD version cuts off Grace's speech when hanging up the phone on day 1.
//  This is a timing issue that also occurs in the original.
//
// startingCartoon:changeState(12) plays Grace's final phone message but doesn't
//  synchronize it with the script. Instead ego goes through a series of movements
//  that advance the state while Grace is speaking. Once the sequence is complete
//  Grace hangs up the phone and starts her next message which interrupts the
//  previous one. There is no mechanism to make sure that Grace's message has
//  first completed and so it cut offs the last one or two words. The timing only
//  worked in the original on slower machines that weren't able to run the
//  sequence at full speed.
//
// We fix this by adding a delay to startingCartoon:changeState(18) so that
//  Grace's speech has time to complete. This scene occurs before game speed
//  can be set and it plays at a consistent speed on ScummVM.
//
// This patch is only applied to CD versions. Floppies have a different script.
//
// Applies to: All CD versions
// Responsible method: startingCartoon:changeState(18)
// Fixes bug: #10787
static const uint16 gk1Day1GracePhoneSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x12,                 // ldi 12
	0x1a,                       // eq?
	0x31, 0x2c,                 // bnt 2c
	SIG_ADDTOOFFSET(+28),
	0x38, SIG_UINT16(0x0003),   // pushi 0003
	0x51, 0x69,                 // class Osc
	0x36,                       // push
	0x78,                       // push1
	0x7c,                       // pushSelf
	0x81, 0x00,                 // lag global[0]
	0x4a, SIG_UINT16(0x0024),   // send 24 [ GKEgo: ... setCycle: Osc 1 self ]
	0x32, SIG_ADDTOOFFSET(+2),  // jmp [ end of method ]
	SIG_END
};

static const uint16 gk1Day1GracePhonePatch[] = {
	PATCH_ADDTOOFFSET(+33),
	0x7a,                       // push2
	0x51, 0x69,                 // class Osc
	0x36,                       // push
	0x78,                       // push1
	0x81, 0x00,                 // lag global[0]
	0x4a, PATCH_UINT16(0x0022), // send 22 [ GKEgo: ... setCycle: Osc 1 ]

	// advance to the next state in 6 seconds instead of when Gabriel finishes
	//  taking a sip of coffee, which takes 2 seconds, giving Grace's speech
	//  an extra 4 seconds to complete.
	0x35, 0x06,                 // ldi 06
	0x65, 0x1c,                 // aTop seconds

	0x3a,                       // toss
	0x48,                       // ret
	PATCH_END
};

// French and Spanish CD versions contain an active debugging hotkey, ALT+N,
//  which brings up a series of unskippable bug-reporting dialogs and
//  eventually writes files to disk and crashes non-release builds due to
//  an uninitialized read. This hotkey is always active and not hidden
//  behind the game's debug mode flag so we just patch it out.
//
// Applies to: French and Spanish PC CD
// Responsible method: GK:handleEvent
// Fixes bug: #10781
static const uint16 gk1SysLoggerHotKeySignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x3100),       // ldi 3100 [ ALT+N ]
	0x1a,                           // eq?
	0x31,                           // bnt
	SIG_END
};

static const uint16 gk1SysLoggerHotKeyPatch[] = {
	PATCH_ADDTOOFFSET(+4),
	0x33,                           // jmp
	PATCH_END
};

// After interrogating Gran in room 380, the room is re-initialized incorrectly.
//  Clicking on objects while seated causes Gabriel to briefly flicker into
//  standing and other frames. After standing, the knitting basket can be walked
//  through. These are script bugs which also occur in Sierra's interpreter.
//
// Ego is initialized incorrectly by rm380:init when returning from interrogation
//  (room 50). Several properties are wrong and it's bad luck that it works as
//  well as it does or Sierra would have noticed. For comparison, the scripts
//  egoEnters and sitDown do it correctly. rm380:init first initializes ego for
//  walking and then applies only some of the properties for sitting in the chair.
//
// This leaves ego in a walking/sitting state with several problems:
//  - signal flag kSignalDoesntTurn isn't set
//  - cycler is set to StopWalk instead of none
//  - loop/cel is set to 2 0 instead of 0 5
//
// rm380:init sets ego's loop/cel to 0 5 (Gabriel sitting) but the unexpected
//  StopWalk immediately changes this to 2 0 (Gabriel starts talking) which went
//  unnoticed because those two frames are similar. This is why Gabriel's hand
//  is slightly raised when returning from interrogation. The flickering is due
//  to ego attempting to turn to face items while sitting due to kSignalDoesntTurn
//  not being set.
//
// We fix the flickering by passing a second parameter to GKEgo:setLoop which
//  causes kSignalDoesntTurn to be set, preventing ego from attempting to face
//  objects being clicked, just as egoEnters and sitDown do. We fix the knitting
//  basket by adding its obstacle polygon to the room even when returning from
//  interrogation, which Sierra forgot to do.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: rm380:init
// Fixes bug: #9760, #10707
static const uint16 gk1GranRoomInitSignature[] = {
	0x38, SIG_SELECTOR16(setCel),       // pushi setCel
	0x78,                               // push1
	0x39, 0x05,                         // pushi 05
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x76,                               // push0 [ loop: 0 ]
	0x38, SIG_SELECTOR16(init),         // pushi init
	0x76,                               // push0
	0x38, SIG_SELECTOR16(posn),         // pushi posn
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00af),           // pushi 00af
	0x39, 0x75,                         // pushi 75
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x001e),           // send 1e [ GKEgo: ... setCel: 5, setLoop: 0 ... ]
	0x35, 0x01,                         // ldi 1
	0xa3, 0x00,                         // sal local[0] [ 1, a non-zero value indicates ego is sitting ]
	SIG_END
};

static const uint16 gk1GranRoomInitPatch[] = {
	0x39, PATCH_SELECTOR8(cel),         // pushi cel [ use cel instead of equivalent setCel to save a byte ]
	0x78,                               // push1
	0x39, 0x05,                         // pushi 05
	0x38, PATCH_SELECTOR16(setLoop),    // pushi setLoop
	0x7a,                               // push2
	0x76,                               // push0 [ loop: 0 ]
	0x78,                               // push1 [ 2nd param tells setLoop to set kSignalDoesntTurn ]
	0x38, PATCH_SELECTOR16(init),       // pushi init
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(posn),       // pushi posn
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x00af),         // pushi 00af
	0x39, 0x75,                         // pushi 75
	0x81, 0x00,                         // lag global[0]
	0xa3, 0x00,                         // sal local[0] [ setting a non-zero object instead of 1 saves 2 bytes ]
	0x4a, PATCH_UINT16(0x0020),         // send 20 [ GKEgo: ... cel: 5, setLoop: 0 1 ... ]
	0x33, 0x87,                         // jmp -79 [ add knitting basket obstacle to room ]
	PATCH_END
};

// After phoning Wolfgang on day 7, Gabriel is placed beyond room 220's obstacle
//  boundary and can walk through walls and behind the room. This also occurs in
//  the original. The script inconsistently uses accessible and inaccessible
//  positions for placing ego next to the phone. We patch all instances to use
//  the accessible position.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible methods: rm220:init, useThePhone:changeState(0)
// Fixes bug: #10853
static const uint16 gk1EgoPhonePositionSignature[] = {
	SIG_MAGICDWORD,
	0x39, 0x68,                         // pushi 68 [ x: 104 ]
	0x39, 0x7e,                         // pushi 7e [ y: 126 ]
	SIG_END
};

static const uint16 gk1EgoPhonePositionPatch[] = {
	0x39, 0x6b,                         // pushi 6b [ x: 107 ]
	0x39, 0x7c,                         // pushi 7c [ y: 124 ]
	PATCH_END
};

// Restarting the game doesn't reset the current inventory item in the icon bar.
//  The previously selected item can then be used on day 1.
//
// Room 93 restarts the game and resets inventory by setting each item's owner
//  to zero. It makes no attempt to reset the icon bar. We fix this by instead
//  calling GKEgo:put on each item and passing zero for the new owner, as this
//  handles updating the icon bar when dropping an item. The "state" property is
//  no longer cleared for items but that's okay because it's never set or used.
//
// Applies to: All versions
// Responsible method: doTheRestart:changeState(0)
// Fixes bug: #11222
static const uint16 gk1RestartInventorySignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(owner),            // pushi owner
	0x78,                                   // push1
	0x76,                                   // push0
	0x39, SIG_SELECTOR8(state),             // pushi state
	0x78,                                   // push1
	0x76,                                   // push0
	0x39, SIG_SELECTOR8(at),                // pushi at
	0x78,                                   // push1
	0x8b, 0x00,                             // lsl 00
	0x81, 0x09,                             // lag 09
	0x4a, SIG_UINT16(0x0006),               // send 06 [ GKInventory at: local0 ]
	0x4a, SIG_UINT16(0x000c),               // send 0c [ item owner: 0 state: 0 ]
	SIG_END
};

static const uint16 gk1RestartInventoryPatch[] = {
	0x38, PATCH_SELECTOR16(put),            // pushi put
	0x7a,                                   // push2
	0x8b, 0x00,                             // lsl 00
	0x76,                                   // push0
	0x81, 0x00,                             // lag 00
	0x4a, PATCH_UINT16(0x0008),             // send 08 [ GKEgo put: local0 0 ]
	0x33, 0x08,                             // jmp 08
	PATCH_END
};

// On day 6 in Jackson Square, if you never gave Madame Lorelei her veil on an
//  earlier day, then looking at her empty booth says that she's sitting there.
//  Clicking Operate also says the wrong message. The logic in the booth and
//  chair doVerb methods doesn't consider that this optional action might not
//  occur by day 6. We add the missing day check by overwriting a redundant
//  local variable test.
//
// Applies to: All versions
// Responsible methods: booth:doVerb, chair1:doVerb, chair2:doVerb
static const uint16 gk1EmptyBoothMessageSignature[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x001a),               // bnt 001a [ skip lorelei message ]
	0x83, 0x01,                             // lal 01   [ always zero ]
	0x18,                                   // not
	0x30, SIG_UINT16(0x0014),               // bnt 0014 [ skip lorelei message ]
	SIG_END
};

static const uint16 gk1EmptyBoothMessagePatch[] = {
	0x31, 0x1b,                             // bnt 1b [ skip lorelei message ]
	0x89, 0x7b,                             // lsg 7b
	0x35, 0x05,                             // ldi 05
	0x24,                                   // le?    [ day <= 5 ]
	0x31, 0x14,                             // bnt 14 [ skip lorelei message ]
	PATCH_END
};

// Madame Lorelei has an obscure timer bug that can cause events to repeat and
//  award duplicate points. When exiting the conversation with her by selecting
//  "Er...nothing.", fortuneTeller:cue resets the dance timer incorrectly.
//  loreleiTimer's client is the room. When it fires, nwJackson:cue tests flags
//  and room conditions before running the sLoreleiDance script, otherwise it
//  resets the timer for 5 seconds. When fortuneTeller:cue resets the timer it
//  changes the client to sLoreleiDance, bypassing the checks in nwJackson:cue.
//  Madame Lorelei can then dance and drop her veil a second time while Gabriel
//  already has it, among other edge cases.
//
// We fix this by setting the room as the client when resetting the timer. This
//  requires calling dispose first since the timer is already running, which
//  might have been what led to this script being written differently.
//
// Applies to: All versions
// Responsible method: fortuneTeller:cue
static const uint16 gk1LoreleiDanceTimerSignature[] = {
	0x30, SIG_UINT16(0x0005),               // bnt 0005
	0xc3, SIG_MAGICDWORD, 0x04,             // +al 04
	0x32, SIG_UINT16(0x001f),               // jmp 001f
	0x3c,                                   // dup
	0x35, 0x10,                             // ldi 10 [ "Er...nothing." ]
	0x1a,                                   // eq?
	0x30, SIG_UINT16(0x0018),               // bnt 0018
	0x38, SIG_SELECTOR16(setReal),          // pushi setReal
	0x7a,                                   // push2
	0x72, SIG_ADDTOOFFSET(+2),              // lofsa sLoreleiDance
	0x36,                                   // push
	SIG_ADDTOOFFSET(+13),
	0x4a, SIG_UINT16(0x0008),               // send 08 [ loreleiTimer setReal: sLoreleiDance ... ]
	SIG_END
};

static const uint16 gk1LoreleiDanceTimerPatch[] = {
	0x30, PATCH_UINT16(0x0004),             // bnt 0004
	PATCH_ADDTOOFFSET(+2),
	0x33, 0x20,                             // jmp 20
	0x3c,                                   // dup
	0x35, 0x10,                             // ldi 10 [ "Er...nothing." ]
	0x1a,                                   // eq?
	0x31, 0x1a,                             // bnt 1a
	0x38, PATCH_SELECTOR16(dispose),        // pushi dispose
	0x76,                                   // push0
	0x38, PATCH_SELECTOR16(setReal),        // pushi setReal
	0x7a,                                   // push2
	0x89, 0x02,                             // lsg 02
	PATCH_ADDTOOFFSET(+13),
	0x4a, PATCH_UINT16(0x000c),             // send 0c [ loreleiTimer dispose: setReal: nwJackson ... ]
	PATCH_END
};

// When walking between rooms in Jackson Square, the cursor is often initialized
//  to the wrong view, even though it's really the Walk cursor. This seemingly
//  random event with many variations is due to a bug in the ExitFeature class.
//
// ExitFeature is responsible for swapping the cursor with an Exit cursor when
//  the mouse is over it and then restoring afterwards. The previous cursor is
//  stored in ExitFeature:lastCursor. The first problem is that ExitFeature:init
//  initializes lastCursor to the current cursor even though the mouse hasn't
//  touched it yet. The second problem is that ExitFeature:dispose restores the
//  cursor to lastCursor if the current cursor is any Exit cursor, including
//  ones it's not responsible for. Entering Jackson Square from the cathedral
//  causes all three ExitFeatures in the room to initialize lastCursor to
//  theWaitCursor (the shield). When exiting the room, the ExitFeatures are
//  disposed in the order they were added, which means northExit disposes first.
//  northExit:lastCursor is still theWaitCursor unless it was moused over.
//  Exiting to another Jackson Square room will cause northExit:dispose to check
//  if the cursor is globeCursor, which represents all Exit cursors, and if so
//  then it will incorrectly restore the cursor to theWaitCursor, preventing
//  the ExitFeature responsible for the room change from correctly restoring.
//
// We fix this by patching ExitFeature:dispose to only restore the cursor if
//  its view matches the Exit cursor that it's responsible for.
//
// Applies to: All versions
// Responsible method: ExitFeature:dispose
static const uint16 gk1ExitFeatureCursorSignature[] = {
	0x89, 0x13,                             // lsg 13 [ current-cursor ]
	0x7a,                                   // push2
	0x76,                                   // push0
	0x78,                                   // push1
	0x43, 0x02, SIG_UINT16(0x0004),         // callk ScriptID 0 1 [ globeCursor ]
	0x1a,                                   // eq?
	0x31, 0x0b,                             // bnt 0b [ skip if current-cursor isn't an exit cursor ]
	0x38, SIG_SELECTOR16(setCursor),        // pushi setCursor
	0x78,                                   // push1
	0x67, SIG_ADDTOOFFSET(+1),              // pTos lastCursor
	0x81, 0x01,                             // lag 01
	0x4a, SIG_UINT16(0x0006),               // send 06 [ GK1 setCursor: lastCursor ]
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(delete),            // pushi delete
	0x78,                                   // push1
	0x7c,                                   // pushSelf
	0x81, 0xce,                             // lag ce
	0x4a, SIG_UINT16(0x0006),               // send 06 [ gk1Exits delete: self ]
	0x48,                                   // ret
	SIG_ADDTOOFFSET(+372),
	0x38, SIG_SELECTOR16(setCursor),        // pushi setCursor
	0x78,                                   // push1
	0x67, SIG_ADDTOOFFSET(+1),              // pTos lastCursor
	0x81, 0x01,                             // lag 01
	0x4a, SIG_UINT16(0x0006),               // send 06 [ GK1 setCursor: lastCursor ]
	0x35, 0x01,                             // ldi 01
	0x65, SIG_ADDTOOFFSET(+1),              // aTop eCursor [ eCursor = 1 ]
	0x48,                                   // ret
	SIG_END
};

static const uint16 gk1ExitFeatureCursorPatch[] = {
	0x39, PATCH_SELECTOR8(delete),          // pushi delete
	0x78,                                   // push1
	0x7c,                                   // pushSelf
	0x81, 0xce,                             // lag ce
	0x4a, PATCH_UINT16(0x0006),             // send 06 [ gk1Exits delete: self ]
	0x39, PATCH_SELECTOR8(view),            // pushi view
	0x76,                                   // push0
	0x81, 0x13,                             // lag 13  [ current-cursor ]
	0x4a, PATCH_UINT16(0x0004),             // send 04 [ current-cursor: view? ]
	0x67, PATCH_GETORIGINALBYTEADJUST(+17, -2), // pTos cursor
	0x1a,                                   // eq?     [ current-cursor:view == self:cursor ]
	0x2e, PATCH_UINT16(0x017e),             // bt 017e [ GK1 setCursor: lastCursor ]
	0x48,                                   // ret
	PATCH_END
};

// The Windows CD version never plays its AVI videos during the bayou ritual,
//  instead it runs the view-based slide shows for the floppy versions. The
//  ritual script contains the normal code for playing its AVI and SEQ files
//  depending on kPlatform, just like every video script in the game, except
//  that the initial floppy flag test has been replaced with another kPlatform
//  call which prevents the real platform test from executing.
//
// It's unclear if this is a script bug or why it would be intentional, but
//  the end result is that selecting Windows as the platform excludes videos
//  from this one scene whereas selecting DOS doesn't, so we patch the platform
//  tests back to floppy tests like everywhere else and enable the AVI videos.
//
// Applies to: All CD versions, though only English versions support Windows
// Responsible method: roomScript:changeState
// Fixes bug: #9807
static const uint16 gk1BayouRitualAviSignature[] = {
	0x76,                                   // push0
	0x43, 0x68, SIG_UINT16(0x0000),         // callk Platform
	SIG_MAGICDWORD,
	0x36,                                   // push
	0x35, 0x01,                             // ldi 01 [ DOS ]
	0x1c,                                   // ne?
	SIG_END
};

static const uint16 gk1BayouRitualAviPatch[] = {
	0x78,                                   // push1
	0x38, PATCH_UINT16(0x01d6),             // pushi 01d6     [ flag 470 ]
	0x47, 0x0d, 0x00, PATCH_UINT16(0x0002), // calle proc13_0 [ is floppy flag set? ]
	PATCH_END
};

// On day 6, an envelope is dropped off in the bookstore after 20 seconds, but
//  if the game is in the middle of a message sequence then it can lockup.
//  When a timer expires, bookstore:cue tests a number of properties to make
//  sure that it's not interrupting anything, but unlike other rooms such as
//  nwJackson:cue it doesn't test if a message is being said. Looking at Grace
//  triggers one of many message sequences which can prevent ego from completing
//  his turn to face the door, leaving dropTheEnvelope stuck in handsOff mode.
//
// We fix this by adding a test to bookstore:cue to verify that a message isn't
//  being said, just like nwJackson:cue. We make room for this by overwriting a
//  redundant handsOff call. This also prevents the florist script from starting
//  in the middle of a message, as this could have similar conflicts.
//
// Applies to: All versions
// Responsible method: bookstore:cue
static const uint16 gk1Day6EnvelopeSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(state),             // pushi state
	0x76,                                   // push0
	0x51, SIG_ADDTOOFFSET(+1),              // class CueObj
	0x4a, SIG_UINT16(0x0004),               // send 04 [ CueObj state? ]
	0x18,                                   // not
	0x31, SIG_ADDTOOFFSET(+1),              // bnt [ reset timer ]
	0x38, SIG_SELECTOR16(handsOff),         // pushi handsOff
	0x76,                                   // push0
	0x81, 0x01,                             // lag 01
	0x4a, SIG_UINT16(0x0004),               // send 04 [ GK1 handsOff: ]
	SIG_END
};

static const uint16 gk1Day6EnvelopePatch[] = {
	PATCH_ADDTOOFFSET(+8),
	0x2f, PATCH_GETORIGINALBYTEADJUST(+10, +1), // bt [ reset timer ]
	0x39, PATCH_SELECTOR8(size),                // pushi size
	0x76,                                       // push0
	0x81, 0x54,                                 // lag 54
	0x4a, PATCH_UINT16(0x0004),                 // send 04 [ talkers size? ]
	0x2f, PATCH_GETORIGINALBYTEADJUST(+10, -9), // bt [ reset timer ]
	PATCH_END
};

// GK1 Mac is missing view 56, which is the close-up of the talisman. Clicking
//  Look on the talisman from inventory is supposed to display an inset with
//  view 56 and say a message, but instead this would crash the Mac interpreter.
//
// We fix this by skipping the talisman inset when view 56 isn't present in the
//  Mac version. The default verb handler still says the talisman message.
//
// Applies to: Mac Floppy
// Responsible method: talisman:doVerb
static const uint16 gk1MacTalismanInsetSignature[] = {
	0x31, 0x3e,                             // bnt 3e [ super doVerb: verb &rest ]
	0x39, SIG_SELECTOR8(hide),              // pushi hide
	0x78,                                   // push1
	0x78,                                   // push1
	0x81, 0x09,                             // lag 09
	0x4a, SIG_UINT16(0x0006),               // send 06 [ GKInventory hide: 1 ] 
	0x39, SIG_SELECTOR8(doit),              // pushi doit
	0x38, SIG_MAGICDWORD,                   // pushi 0008
	      SIG_UINT16(0x0008),
	0x39, 0x38,                             // pushi 38 [ talisman view ]
	SIG_END
};

static const uint16 gk1MacTalismanInsetPatch[] = {
	0x33,                                   // jmp [ super doVerb: verb &rest ]
	PATCH_END
};

// Narrator lockup fix for GK1 CD / Mac, see sciNarratorLockupSignature.
//  The custom code in these versions overlaps with the generic patch signature
//  so we enable the correct one based on game version and platform.
static const uint16 gk1NarratorLockupSignature[] = {
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += game time ]
	0x33, 0x0b,                         // jmp 0b
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

static const uint16 gk1NarratorLockupPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x33, 0x0a,                         // jmp 0a
	PATCH_ADDTOOFFSET(+5),
	0x89, 0x58,                         // lsg 58 [ game time ]
	0x02,                               // add
	0x65, PATCH_GETORIGINALBYTE(+1),    // aTop ticks [ ticks += 60 + game time ]
	0x00,                               // bnot
	0x31, 0xfb,                         // bnt fb [ set ticks to 0 if ticks == -1 ]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry gk1Signatures[] = {
	{  true,     0, "remove alt+n syslogger hotkey",               1, gk1SysLoggerHotKeySignature,      gk1SysLoggerHotKeyPatch },
	{  true,    17, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true,    21, "fix ExitFeature cursor restore",              1, gk1ExitFeatureCursorSignature,    gk1ExitFeatureCursorPatch },
	{  false,   24, "mac: fix missing talisman view",              1, gk1MacTalismanInsetSignature,     gk1MacTalismanInsetPatch },
	{  true,    51, "fix interrogation bug",                       1, gk1InterrogationBugSignature,     gk1InterrogationBugPatch },
	{  true,    93, "fix inventory on restart",                    1, gk1RestartInventorySignature,     gk1RestartInventoryPatch },
	{  true,   210, "fix day 6 envelope lockup",                   2, gk1Day6EnvelopeSignature,         gk1Day6EnvelopePatch },
	{  true,   211, "fix day 1 grace phone speech timing",         1, gk1Day1GracePhoneSignature,       gk1Day1GracePhonePatch },
	{  true,   212, "fix day 5 drum book dialogue error",          1, gk1Day5DrumBookDialogueSignature, gk1Day5DrumBookDialoguePatch },
	{  true,   212, "fix day 5 phone softlock",                    1, gk1Day5PhoneFreezeSignature,      gk1Day5PhoneFreezePatch },
	{  true,   220, "fix ego phone position",                      2, gk1EgoPhonePositionSignature,     gk1EgoPhonePositionPatch },
	{  true,   230, "fix day 6 police beignet timer issue (1/2)",  1, gk1Day6PoliceBeignetSignature1,   gk1Day6PoliceBeignetPatch1 },
	{  true,   230, "fix day 6 police beignet timer issue (2/2)",  1, gk1Day6PoliceBeignetSignature2,   gk1Day6PoliceBeignetPatch2 },
	{  true,   230, "fix day 6 police sleep timer issue",          1, gk1Day6PoliceSleepSignature,      gk1Day6PoliceSleepPatch },
	{  true,   230, "fix police station ego speed",                1, gk1PoliceEgoSpeedFixSignature,    gk1PoliceEgoSpeedFixPatch },
	{  true,   231, "fix beignet vendor speed",                    1, gk1BeignetVendorSpeedSignature,   gk1BeignetVendorSpeedPatch },
	{  true,   240, "fix day 5 mosely veve missing points",        1, gk1Day5MoselyVevePointsSignature, gk1Day5MoselyVevePointsPatch },
	{  true,   250, "fix ego speed when exiting drug store",       1, gk1DrugStoreEgoSpeedFixSignature, gk1DrugStoreEgoSpeedFixPatch },
	{  true,   260, "fix air conditioner speech timing",           1, gk1AirConditionerSpeechSignature, gk1AirConditionerSpeechPatch },
	{  true,   260, "fix day 5 snake attack (1/2)",                1, gk1Day5SnakeAttackSignature1,     gk1Day5SnakeAttackPatch1 },
	{  true,   260, "fix day 5 snake attack (2/2)",                1, gk1Day5SnakeAttackSignature2,     gk1Day5SnakeAttackPatch2 },
	{  true,   280, "fix pathfinding in Madame Cazanoux's house",  1, gk1CazanouxPathfindingSignature,  gk1CazanouxPathfindingPatch },
	{  true,   380, "fix Gran's room obstacles and ego flicker",   1, gk1GranRoomInitSignature,         gk1GranRoomInitPatch },
	{  true,   410, "fix day 2 binoculars lockup",                 1, gk1Day2BinocularsLockupSignature, gk1Day2BinocularsLockupPatch },
	{  true,   420, "fix day 6 empty booth message",               6, gk1EmptyBoothMessageSignature,    gk1EmptyBoothMessagePatch },
	{  true,   420, "fix lorelei dance timer",                     1, gk1LoreleiDanceTimerSignature,    gk1LoreleiDanceTimerPatch },
	{  true,   480, "win: play day 6 bayou ritual avi videos",     3, gk1BayouRitualAviSignature,       gk1BayouRitualAviPatch },
	{  true,   710, "fix day 9 vine swing speech playing",         1, gk1Day9VineSwingSignature,        gk1Day9VineSwingPatch },
	{  true,   710, "fix day 9 mummy animation (floppy)",          1, gk1MummyAnimateFloppySignature,   gk1MummyAnimateFloppyPatch },
	{  true,   710, "fix day 9 mummy animation (cd)",              1, gk1MummyAnimateCDSignature,       gk1MummyAnimateCDPatch },
	{  true,   800, "fix day 10 honfour unlock door lockup",       1, gk1HonfourUnlockDoorSignature,    gk1HonfourUnlockDoorPatch },
	{ false, 64928, "floppy: Narrator lockup fix",                 1, sciNarratorLockupSignature,       sciNarratorLockupPatch },
	{ false, 64928, "cd/mac: Narrator lockup fix",                 1, gk1NarratorLockupSignature,       gk1NarratorLockupPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 2

// GK2's inventory scrolls smoothly when the mouse is held down in the original
//  due to an inner loop in ScrollButton:track, but this causes slow scrolling
//  in our interpreter since we throttle kFrameOut. The script's inner loop is
//  itself throttled by ScrollButton:moveDelay, which is set to 25 and limits
//  event processing to every 25th iteration. Removing this delay results in
//  smooth scrolling as in the original.
//
// Applies to: All versions
// Responsible method: ScrollButton:track
static const uint16 gk2InventoryScrollSpeedSignature[] = {
	SIG_MAGICDWORD,
	0x63, 0x9c,                 // pToa moveDelay [ 25 ]
	0xa5, 0x02,                 // sat 02
	SIG_END
};

static const uint16 gk2InventoryScrollSpeedPatch[] = {
	0x35, 0x01,                 // ldi 01
	PATCH_END
};

// The down scroll button in GK2 jumps up a pixel on mousedown because there is
//  a send to scrollSelections using an immediate value 1, which means to scroll
//  up by 1 pixel. This patch fixes the send to scrollSelections by passing the
//  button's delta instead of 1. The Italian version's vocab.997 is missing the
//  scrollSelections selector so this patch avoids referencing it. Two versions
//  are necessary to accomodate scripts compiled with and without line numbers.
//
// Applies to: All versions
// Responsible method: ScrollButon:track
// Fixes bug: #9648
static const uint16 gk2InventoryScrollDirSignature1[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x63, 0x98,                         // pToa client
	0x4a, SIG_UINT16(0x0006),           // send 06 [ client scrollSelections: 1 ]
	0x7e,                               // line
	SIG_END
};

static const uint16 gk2InventoryScrollDirPatch1[] = {
	0x66, PATCH_UINT16(0x009a),         // pTos delta
	0x62, PATCH_UINT16(0x0098),         // pToa client
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ client scrollSelections: delta ]
	PATCH_END
};

static const uint16 gk2InventoryScrollDirSignature2[] = {
	0x78,                               // push1
	0x63, 0x98,                         // pToa client
	0x4a, SIG_MAGICDWORD,               // send 06 [ client scrollSelections: 1 ]
	      SIG_UINT16(0x0006),
	0x35, 0x02,                         // ldi 02
	0x65, 0x56,                         // aTop cel
	SIG_END
};

static const uint16 gk2InventoryScrollDirPatch2[] = {
	0x67, 0x9a,                         // pTos delta
	0x63, 0x98,                         // pToa client
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ client scrollSelections: delta ]
	0x7a,                               // push2
	0x69, 0x56,                         // sTop cel
	PATCH_END
};

// The init code 'GK2::init' that runs when GK2 starts up unconditionally resets
// the music volume to 63, but the game should always use the volume stored in
// ScummVM.
// Applies to: All versions
// Fixes bug: #9700
static const uint16 gk2VolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x3f, // ldi $3f
	0xa1, 0x4c, // sag global[$4c] (music volume)
	SIG_END
};

static const uint16 gk2VolumeResetPatch[] = {
	0x33, 0x02,  // jmp 2 [past volume changes]
	PATCH_END
};

// GK2 has custom video benchmarking code that needs to be disabled in a local
//  procedure called from GK2:init. It sets the game's detailLevel and returns
//  a value which is assigned to GK2:speedRating and never used. The maximum
//  detailLevel the game recognizes is six so we just set it to that.
//
// Applies to: All versions
// Responsible method: GK2:init
static const uint16 gk2BenchmarkSignature[] = {
	0x76,                       // push0
	0x40, SIG_ADDTOOFFSET(+2),  // call speed test proc 
	      SIG_MAGICDWORD,
		  SIG_UINT16(0x0000),
	0x65, 0x28,                 // aTop speedRating
	SIG_END
};

static const uint16 gk2BenchmarkPatch[] = {
	0x35, 0x06,                 // ldi 06
	0x65, 0x18,                 // aTop _detailLevel
	0x33, 0x02,                 // jmp 02
	PATCH_END
};

// GK2 has a complex sound bug which causes seemingly random lockups when
//  changing rooms in many areas including the Herrenchiemse Museum, the Hunt 
//  Club, and St. Georg Church. This also occurs in the original.
//
// SoundManager continuously plays an array of sounds provided to its play
//  method. Sounds play in a random order with a random delay of five to ten
//  seconds in between. SoundManager is attached to soundRegion and survives
//  room changes. Rooms that set a new playlist call play on initialization.
//  The problem is that SoundManager:play doesn't clear its delay timer. If play
//  is called during a delay then the timer continues and expires during the
//  next sound. This is noticeable throughout the game when background music is
//  randomly interrupted by different music. Many room scripts change rooms by
//  calling SoundManager:fade in handsOff mode and proceeding once they've been
//  cued. If a stray SoundManager timer expires while a script is waiting for
//  fade to complete then SoundManager:cue will play the next sound, overwrite
//  gk2Music:client with itself, and the waiting script will never cue.
//
// We fix this by clearing SoundManager's timer state in SoundManager:play.
//  This prevents the delay timer from ever running while music is playing.
//
// Applies to: All versions
// Responsible method: SoundManager:play
static const uint16 gk2SoundManagerLockupSignature1[] = {
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 00
	0x65, 0x34,                         // aTop cleanup
	SIG_END
};

static const uint16 gk2SoundManagerLockupPatch1[] = {
	0x35, 0x00,                         // ldi 00
	0x64, PATCH_UINT16(0x001e),         // aTop seconds
	0x64, PATCH_UINT16(0x0012),         // aTop scratch
	PATCH_END
};

static const uint16 gk2SoundManagerLockupSignature2[] = {
	0x87, SIG_MAGICDWORD, 0x00,         // lap 00
	0x18,                               // not
	0x31, 0x10,                         // bnt 10 [ skip debug message ]
	0x78,                               // push1
	0x72,                               // lofsa "WARNING: 0 args passed to SoundManager!"
	SIG_END
};

static const uint16 gk2SoundManagerLockupPatch2[] = {
	0x35, 0x00,                         // ldi 00
	0x65, 0x1e,                         // aTop seconds
	0x65, 0x12,                         // aTop scratch
	0x32, PATCH_UINT16(0x0014),         // jmp 0014 [ skip debug message ]
	PATCH_END
};

// Clicking on Frau Miller in room 810 after exhausting her topics and then
//  clicking on anything else can lockup or crash the game. rm810:newRoom fades
//  the music before transitioning to room 8110, which takes several seconds.
//  The game doesn't disable input during this period and if the player begins
//  another action then rm810:cue can unexpectedly interrupt it. If Grace is
//  walking then the room will reload in a handsOff state. Other edge cases
//  include setting the room number to zero and subsequently crashing.
//
// We fix this by calling handsOff so that the player can't interrupt the Frau
//  Miller room transition while waiting for the music to fade, which is
//  consistent with the exit to the map.
//
// Applies to: All versions
// Responsible method: rm810:newRoom
static const uint16 gk2FrauMillerLockupSignature[] = {
	SIG_MAGICDWORD,
	0x39, 0x03,                         // pushi 03
	0x8f, 0x01,                         // lsp 01
	0x38, SIG_UINT16(0x1fae),           // pushi 1fae
	0x38, SIG_UINT16(0x0320),           // pushi 0320
	0x46, SIG_UINT16(0xfde7),           // calle proc64999_5 [ OneOf newRoomNumber 8110 800 ]
	      SIG_UINT16(0x0005),
	      SIG_UINT16(0x0006),
	0x31,                               // bnt [ don't fade music ]
	SIG_END
};

static const uint16 gk2FrauMillerLockupPatch[] = {
	0x8f, 0x01,                         // lsp 01
	0x34, PATCH_UINT16(0x1fae),         // ldi 1fae
	0x24,                               // le? [ newRoomNumber <= 8110 ]
	0x31, PATCH_GETORIGINALBYTEADJUST(+18, +11), // bnt [ don't fade music ]
	0x38, PATCH_SELECTOR16(handsOff),   // pushi handsOff
	0x39, 0x00,                         // pushi 00
	0x80, PATCH_UINT16(0x0001),         // lag 0001
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ GK2 handsOff: ]
	PATCH_END
};

// GK2 1.0 contains a deadend bug in chapter 3. Exhausting Leber's topics before
//  reading Grace's letter prevents returning to the police station to ask about
//  the Black Wolf, which is necessary to complete the chapter.
//
// We fix this as Sierra did by adding a flag 218 test so that the police
//  station doesn't close before Leber has been asked about the Black Wolf.
//
// Applies to: English PC 1.0
// Responsible method: rm3210:dispose
static const uint16 gk2PoliceStationDeadendSignature[] = {
	0x78,                               // push1
	0x38, SIG_UINT16(0x00dd),           // pushi 00dd
	0x47, 0x0b, 0x00, SIG_MAGICDWORD,   // calle proc11_0 [ is flag 221 set? ]
	      SIG_UINT16(0x002),
	0x31, 0x51,                         // bnt 51 [ skip closing police station ]
	SIG_END
};

static const uint16 gk2PoliceStationDeadendPatch[] = {
	0x80, PATCH_UINT16(0x00a3),         // lag 00a3 [ flags 208-223 ]
	0x39, 0x24,                         // pushi 24
	0x12,                               // and
	0x39, 0x24,                         // pushi 24
	0x1a,                               // eq? [ are flags 218 and 221 set? ]
	PATCH_END
};

// In chapter 3, Xaver can be asked about the Black Wolf before learning about
//  the Black Wolf from Grace's letter. The tBlackWolf topic in room 4320 is
//  missing the readyFlagNum value of 514 that the other tBlackWolf topics in
//  chapter 3 have, so we set it.
//
// Applies to: All versions
// Responsible method: heap in script 4320
static const uint16 gk2XaverBlackWolfSignature[] = {
	SIG_MAGICDWORD,             // tBlackWolf
	SIG_UINT16(0x010e),         // sceneNum = 270
	SIG_UINT16(0x00f0),         // flagNum = 240
	SIG_UINT16(0x0000),         // readyFlagNum = 0
	SIG_END
};

static const uint16 gk2XaverBlackWolfkPatch[] = {
	PATCH_ADDTOOFFSET(+4),
	PATCH_UINT16(0x0202),       // readyFlagNum = 514
	PATCH_END
};

// Chapter 4 has a bug in many versions of GK2 that is effectively a deadend.
//  Asking Georg about "Ludwig's letter to the Conductor" is required to finish
//  the chapter. This topic becomes available after looking at the "Ludwig and
//  Wagner" plaque in Herrenchiemsee and then clicking again to read it aloud.
//  The problem is that the rest of the game only cares about looking at the 
//  plaque. If Herrenchiemsee is completed without reading the plaque then the
//  Hint feature claims everything is done and the player appears to be stuck.
//
// We fix this as Sierra did by making Georg's letter topic available upon just
//  looking at the plaque, which is consistent with the rest of the scripts.
//  Although Sierra advertised this fix in the readme for GK2PAT 1.11, the patch
//  file seems to be missing, but appears in later versions and the GOG release.
//
// Applies to: English PC 1.0, 1.1, 1.11 Patch, Mac
// Responsible method: Heap in script 8520
static const uint16 gk2GeorgLetterTopicSignature[] = {
	SIG_MAGICDWORD,             // tLtr2Conductor
	SIG_UINT16(0x0211),         // sceneNum = 529
	SIG_UINT16(0x012a),         // flagNum = 298
	SIG_UINT16(0x0283),         // readyFlagNum = 643
	SIG_END
};

static const uint16 gk2GeorgLetterTopicPatch[] = {
	PATCH_ADDTOOFFSET(+4),
	PATCH_UINT16(0x026f),       // readyFlagNum = 623
	PATCH_END
};

// In early versions of GK2, clicking on the holy water basket after using the
//  holy water locks up the game. The script is supposed to test the flag that's
//  set when getting the water but some code mistakenly tests inventory instead.
//  We fix this as Sierra did by replacing the inventory tests with flag tests.
//
// Applies to: English PC 1.0, Mac
// Responsible methods: waterBasket:handleEvent, waterBasket:doVerb
static const uint16 gk2HolyWaterLockupSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(has),  // pushi has
	0x78,                       // push1
	0x39, 0x3e,                 // pushi 3e
	0x81, 0x00,                 // lag 00
	0x4a, SIG_UINT16(0x0006),   // send 06 [ GraceEgo has: 62 (invBottleOfWater) ]
	SIG_END
};

static const uint16 gk2HolyWaterLockupPatch[] = {
	0x38, PATCH_UINT16(0x0001), // pushi 0001
	0x38, PATCH_UINT16(0x0476), // pushi 0476
	0x47, 0x0b, 0x00,           // calle proc11_0 [ is flag 1142 set? ]
	      PATCH_UINT16(0x0002),
	PATCH_END
};

// In early versions of GK2, Neuschwanstein castle can flash when clicking the
//  Hint button even after completing everything. The Hint script tests too many
//  flags including one whose value is random since it toggles back and forth
//  between two tape messages. We remove these flag tests as Sierra did.
//
// Applies to: English PC 1.0, Mac
// Responsible method: local procedure #0 in script 800
static const uint16 gk2NeuschwansteinHintSignature1[] = {
	SIG_MAGICDWORD,
	0x78,                       // push1
	0x38, SIG_UINT16(0x024d),   // pushi 024d
	0x47, 0x0b, 0x00,           // calle proc11_0 [ is flag 589 set? ]
	SIG_UINT16(0x0002),
	SIG_END
};

static const uint16 gk2NeuschwansteinHintSignature2[] = {
	SIG_MAGICDWORD,
	0x78,                       // push1
	0x38, SIG_UINT16(0x024e),   // pushi 024e
	0x47, 0x0b, 0x00,           // calle proc11_0 [ is flag 590 set? ]
	SIG_UINT16(0x0002),
	SIG_END
};

static const uint16 gk2NeuschwansteinHintSignature3[] = {
	SIG_MAGICDWORD,
	0x78,                       // push1
	0x38, SIG_UINT16(0x0250),   // pushi 0250
	0x47, 0x0b, 0x00,           // calle proc11_0 [ is flag 592 set? ]
	SIG_UINT16(0x0002),
	SIG_END
};

static const uint16 gk2NeuschwansteinHintPatch[] = {
	0x35, 0x01,                 // ldi 01
	0x33, 0x05,                 // jmp 05
	PATCH_END
};

// Clicking an inventory item on the Wagner paintings in rooms 8616 and 8617
//  causes a missing message error. The paintings only have responses for the
//  "Do" verb but painting:doVerb passes the incoming verb to gk2Messager:say
//  without any filtering. We fix this by always playing the "Do" message.
//
// Applies to: All versions
// Responsible methods: painting:doVerb in scripts 8616 and 8617
static const uint16 gk2WagnerPaintingMessageSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(say),  // pushi say
	0x38, SIG_UINT16(0x0006),   // pushi 0006
	0x67, SIG_ADDTOOFFSET(+1),  // pTos noun
	0x8f, 0x01,                 // lsp 01 [ verb ]
	SIG_END
};

static const uint16 gk2WagnerPaintingMessagePatch[] = {
	PATCH_ADDTOOFFSET(+8),
	0x39, 0x3e,                 // pushi 3e [ "Do" verb ]
	PATCH_END
};

// The game-over rooms 665 and 666 draw a pic over everything by setting the
//  default plane's priority to 202, but this is already inventoryBorderPlane's
//  priority. In our interpreter this causes a border fragment to be drawn above
//  the pics. This worked by luck in Sierra's interpreter because it sorts on
//  memory ID when planes have the same priority. In ScummVM the renderer
//  guarantees a sort order based on the creation order of the planes. The
//  default plane is created first and drawn before inventoryBorderPlane.
//
// We fix this by increasing the plane priority in the game-over rooms.
//
// Applies to: All versions
// Responsible methods: gabeNews:init, uDie:init
// Fixes bug: #11298
static const uint16 gk2GameOverPrioritySignature[] = {
	0x39, SIG_SELECTOR8(priority),  // pushi priority
	SIG_MAGICDWORD,
	0x78,                           // push1
	0x38, SIG_UINT16(0x00ca),       // pushi 00ca
	0x81, 0x03,                     // lag 03
	0x4a, SIG_UINT16(0x0012),       // send 12 [ Plane ... priority: 202 ]
	SIG_END
};

static const uint16 gk2GameOverPriorityPatch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x38, PATCH_UINT16(0x00cb),     // pushi 00cb [ priority: 203 ]
	PATCH_END
};

// GK2 fans have created patches that add subtitles to the entire game. There
//  are at least English and Spanish patch sets. Sierra added the subtitle
//  feature solely for the Portuguese version. The fan patches include these
//  subtitle scripts, replace the Portuguese resources and embedded script
//  strings, and configure Sierra's interpreter to use the Portuguese language
//  through RESOURCE.CFG. This sets GK2:printLang which the scripts test for
//  Portuguese in order to activate subtitles.
//
// The subtitle patches are compatible with ScummVM except for the requirement
//  that GK2:printLang equals Portuguese (351) since we don't use RESOURCE.CFG.
//  We fix this by patching the GK2:printLang tests to always activate subtitles
//  when a sync resource is present for synchronizing text to video playback.
//
// Applies to: PC versions with a subtitle fan-patch applied
// Responsible methods: Any that test GK2:printLang for Portuguese
// Fixes bugs: #9677, #11282
static const uint16 gk2SubtitleCompatibilitySignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(printLang), // pushi printLang
	0x76,                           // push0
	0x81, 0x01,                     // lag 01
	0x4a, SIG_UINT16(0x0004),       // send 04 [ GK2 printLang? ]
	SIG_END
};

static const uint16 gk2SubtitleCompatibilityPatch[] = {
	0x34, PATCH_UINT16(0x015f),     // ldi 015f [ K_LANG_PORTUGUESE ]
	0x33, 0x03,                     // jmp 03
	PATCH_END
};

//          script, description,                                              signature                         patch
static const SciScriptPatcherEntry gk2Signatures[] = {
	{  true,     0, "disable volume reset on startup",                     1, gk2VolumeResetSignature,           gk2VolumeResetPatch },
	{  true,     0, "disable video benchmarking",                          1, gk2BenchmarkSignature,             gk2BenchmarkPatch },
	{  true,    23, "fix inventory scroll speed",                          2, gk2InventoryScrollSpeedSignature,  gk2InventoryScrollSpeedPatch },
	{  true,    23, "fix inventory scroll direction",                      1, gk2InventoryScrollDirSignature1,   gk2InventoryScrollDirPatch1 },
	{  true,    23, "fix inventory scroll direction (no line numbers)",    1, gk2InventoryScrollDirSignature2,   gk2InventoryScrollDirPatch2 },
	{  true,    37, "fix sound manager lockup",                            1, gk2SoundManagerLockupSignature1,   gk2SoundManagerLockupPatch1 },
	{  true,    37, "fix sound manager lockup (no line numbers)",          1, gk2SoundManagerLockupSignature2,   gk2SoundManagerLockupPatch2 },
	{  true,   665, "fix game-over priority",                              1, gk2GameOverPrioritySignature,      gk2GameOverPriorityPatch },
	{  true,   666, "fix game-over priority",                              1, gk2GameOverPrioritySignature,      gk2GameOverPriorityPatch },
	{  true,   800, "fix neuschwanstein hint (1/3)",                       1, gk2NeuschwansteinHintSignature1,   gk2NeuschwansteinHintPatch },
	{  true,   800, "fix neuschwanstein hint (2/3)",                       1, gk2NeuschwansteinHintSignature2,   gk2NeuschwansteinHintPatch },
	{  true,   800, "fix neuschwanstein hint (3/3)",                       1, gk2NeuschwansteinHintSignature3,   gk2NeuschwansteinHintPatch },
	{  true,   810, "fix frau miller lockup",                              1, gk2FrauMillerLockupSignature,      gk2FrauMillerLockupPatch },
	{  true,  1020, "fix holy water lockup",                               2, gk2HolyWaterLockupSignature,       gk2HolyWaterLockupPatch },
	{  true,  3210, "fix police station deadend",                          1, gk2PoliceStationDeadendSignature,  gk2PoliceStationDeadendPatch },
	{  true,  4320, "fix xaver black wolf topic",                          1, gk2XaverBlackWolfSignature,        gk2XaverBlackWolfkPatch },
	{  true,  8520, "fix georg letter topic",                              1, gk2GeorgLetterTopicSignature,      gk2GeorgLetterTopicPatch },
	{  true,  8616, "fix wagner painting message",                         2, gk2WagnerPaintingMessageSignature, gk2WagnerPaintingMessagePatch },
	{  true,  8617, "fix wagner painting message",                         2, gk2WagnerPaintingMessageSignature, gk2WagnerPaintingMessagePatch },
	{  true, 64928, "Narrator lockup fix",                                 1, sciNarratorLockupSignature,        sciNarratorLockupPatch },
	{  true, 64928, "Narrator lockup fix",                                 1, sciNarratorLockupLineSignature,    sciNarratorLockupLinePatch },
	{  true, 64990, "increase number of save games (1/2)",                 1, sci2NumSavesSignature1,            sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",                 1, sci2NumSavesSignature2,            sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",                     1, sci2ChangeDirSignature,            sci2ChangeDirPatch },
	{ false,     0, "subtitle patch compatibility",                        3, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false,    11, "subtitle patch compatibility",                        7, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false,    12, "subtitle patch compatibility",                        5, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false,    91, "subtitle patch compatibility",                        7, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false,   200, "subtitle patch compatibility",                        1, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false,  1300, "subtitle patch compatibility",                        1, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	{ false, 64924, "subtitle patch compatibility",                        1, gk2SubtitleCompatibilitySignature, gk2SubtitleCompatibilityPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// When spotting the destroyer, timing problems prevent completing the bridge
//  scene at fast game speeds.
//
// In the control room, room 25, ego and the captain go to the bridge, room 28,
//  where they spot ships in an effectively automatic scene. When this completes
//  they return to the control room, the captain falls, and the player regains
//  control of ego and has to walk to the control panel. This entire sequence
//  has to be completed within 400 game cycles or the destroyer kills the sub,
//  but the bridge timing is in wall time and has at least 25 seconds of delays,
//  which at faster speeds is longer than 400 cycles. The bridge also animates
//  during messages, causing the timer to run while reading, so even at slower
//  speeds the game can illogically end before the ships are revealed.
//
// There are several problems here but the real bug is that the timer starts
//  before the player has control. We fix this by disabling the timer during the
//  bridge and resetting it to 120 game cycles when the player regains control.
//  This preserves the original timer duration in the control room, where the
//  real timed action is, and is compatible with existing saved games. When the
//  timer expires, subMarineScript:changeState(9) no longer ends the game if
//  subMarine:roomFlags flag 2 isn't set, which captainfallsScript sets at the
//  same time that it now calls subMarineScript:changeState(8).
//
// Applies to: All versions
// Responsible methods: subMarineScript:changeState, captainfallsScript:changeState
// Fixes bug #11017
static const uint16 icemanDestroyerTimer1Signature[] = {
	0x30, SIG_UINT16(0x0022),           // bnt 0022 [ state 8 ]
	SIG_ADDTOOFFSET(+0x1f),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x0074),           // jmp 0074 [ end of method ]
	0x3c,                               // dup
	0x35, 0x08,                         // ldi 08
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0008),           // bnt 0008 [ state 9 ]
	0x34, SIG_UINT16(0x0190),           // ldi 0190
	0x65, 0x10,                         // aTop cycles [ cycles = 400 ]
	0x32, SIG_UINT16(0x0065),           // jmp 0065 [ end of method ]
	0x3c,                               // dup
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0023),           // bnt 0023 [ state 15 ]
	0x8f, 0x00,                         // lsp 00
	0x35, 0x02,                         // ldi 02
	0x22,                               // lt?      [ didn't reach control panel? ]
	0x30, SIG_UINT16(0x0014),           // bnt 0014 [ skip death if reached control panel ]
	SIG_END
};

static const uint16 icemanDestroyerTimer1Patch[] = {
	0x30, PATCH_UINT16(0x001f),         // bnt 001f [ state 8 ]
	PATCH_ADDTOOFFSET(+0x1f),
	0x3c,                               // dup
	0x35, 0x08,                         // ldi 08
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 04 [ state 9 ]
	0x35, 0x78,                         // ldi 78
	0x65, 0x10,                         // aTop cycles [ cycles = 120 ]
	0x3c,                               // dup
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x31, 0x2c,                         // bnt 2c [ state 15 ]
	0x38, PATCH_SELECTOR16(roomFlags),  // pushi roomFlags
	0x76,                               // push0
	0x63, 0x08,                         // pToa client
	0x4a, 0x04,                         // send 04 [ subMarine roomFlags? ]
	0x7a,                               // push2  [ flag 2 set when captain falls ]
	0x12,                               // and    [ has captain fallen? ]
	0x31, 0x19,                         // bnt 19 [ skip death if captain hasn't fallen ]
	0x8f, 0x00,                         // lsp 00
	0x22,                               // lt?    [ didn't reach control panel? ]
	0x31, 0x14,                         // bnt 14 [ skip death if reached control panel ]
	PATCH_END
};

static const uint16 icemanDestroyerTimer2Signature[] = {
	// print four messages
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0187),           // pushi 0187
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x47, 0xff, 0x00, 0x04,             // calle proc255_0 [ print 391 2 ]
	SIG_ADDTOOFFSET(+20),               // [ print 391 3, print 391 4 ]
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0187),           // pushi 0187
	0x39, 0x05,                         // pushi 05
	0x47, 0xff, 0x00, 0x04,             // calle proc255_0 [ print 391 5 ]
	SIG_END
};

static const uint16 icemanDestroyerTimer2Patch[] = {
	// print four messages using a loop
	0x35, 0x02,                         // ldi 02
	0xa7, 0x01,                         // sap 01
	0x8f, 0x01,                         // lsp 01
	0x35, 0x05,                         // ldi 05
	0x24,                               // le?    [ loop while 2 <= param1 <= 5 ]
	0x31, 0x0e,                         // bnt 0e [ exit loop ]
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0187),         // pushi 0187
	0x8f, 0x01,                         // lsp 01
	0x47, 0xff, 0x00, 0x04,             // calle proc255_0 [ print 391 param1 ]
	0xcf, 0x01,                         // +sp 01 [ increment and push param1 ]
	0x33, 0xed,                         // jmp ed [ continue loop ]
	// reset subMarineScript timer
	0x39, PATCH_SELECTOR8(script),      // pushi script
	0x76,                               // push0
	0x51, 0x5c,                         // class subMarine
	0x4a, 0x04,                         // send 04 [ subMarine script? ]
	0x39, PATCH_SELECTOR8(changeState), // pushi changeState
	0x78,                               // push1
	0x39, 0x08,                         // pushi 08
	0x4a, 0x06,                         // send 06 [ subMarineScript changeState: 8 ]
	PATCH_END
};

// At the pier in Honolulu, room 23, "climb down" causes ego to bypass boarding
//  procedure, walk through the air, climb down the hatch, and get stuck in the
//  submarine without triggering a room change. There is no "climb up" command.
//
// Boarding requires asking the officer permission. comeAboardScript gives him
//  the orders, runs downTheHatchScript, and changes to room 31 when finished.
//  downTheHatchScript only walks ego to the hatch and runs the climb animation.
//  "climb down" simply runs downTheHatchScript and nothing else, leaving the
//  room in a broken state by running this intermediate script out of context.
//
// We patch "climb down" to respond with the message for other hatch commands.
//
// Applies to: All versions
// Responsible method: hatch:handleEvent
// Fixes bug #11039
static const uint16 icemanClimbDownHatchSignature[] = {
	0x7a,                               // push2
	SIG_MAGICDWORD,
	0x39, 0x17,                         // pushi 17
	0x39, 0x18,                         // pushi 18
	0x47, 0xff, 0x00, 0x04,             // calle proc255_0 04 [ "You must follow proper boarding procedure." ]
	0x32, SIG_UINT16(0x0021),           // jmp 0021 [ end of method ]
	SIG_ADDTOOFFSET(+22),
	0x39, SIG_SELECTOR8(setScript),     // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0xfc24),           // lofsa downTheHatchScript
	SIG_END
};

static const uint16 icemanClimbDownHatchPatch[] = {
	PATCH_ADDTOOFFSET(+34),
	0x33, 0xdc,                         // jmp dc [ "You must follow proper boarding procedure." ]
	PATCH_END
};

//         script, description,                                       signature                                      patch
static const SciScriptPatcherEntry icemanSignatures[] = {
	{ true,    23, "climb down hatch",                             1, icemanClimbDownHatchSignature,                 icemanClimbDownHatchPatch },
	{ true,   314, "destroyer timer (1/2)",                        1, icemanDestroyerTimer1Signature,                icemanDestroyerTimer1Patch },
	{ true,   391, "destroyer timer (2/2)",                        1, icemanDestroyerTimer2Signature,                icemanDestroyerTimer2Patch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Island of Dr. Brain

// Narrator lockup fix, see sciNarratorLockupSignature.
//  Island of Dr. Brain contains an early version of Narrator with the lockup
//  bug so it requires its own patch.
static const uint16 islandBrainNarratorLockupSignature[] = {
	0x67, SIG_ADDTOOFFSET(+1),          // pTos ticks
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x36,                               // push
	0x89, 0x56,                         // lsg 56 [ game time ]
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime 00
	0x02,                               // add
	0x02,                               // add
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += 60 + game time + GetTime ]
	0x35, 0x01,                         // ldi 01 [ true ]
	0x48,                               // ret
	SIG_END
};

static const uint16 islandBrainNarratorLockupPatch[] = {
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime 00
	0x39, 0x3c,                         // pushi 3c
	0x02,                               // add
	0x67, PATCH_GETORIGINALBYTE(+1),    // pTos ticks
	0x02,                               // add
	0x89, 0x56,                         // lsg 56 [ game time ]
	0x02,                               // add
	0x65, PATCH_GETORIGINALBYTE(+1),    // aTop ticks [ ticks = GetTime + 60 + game time + ticks ]
	0x00,                               // bnot
	0x31, 0xfb,                         // bnt fb [ set ticks to 0 if ticks == -1 ]
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry islandBrainSignatures[] = {
	{  true,   928, "Narrator lockup fix",                         1, islandBrainNarratorLockupSignature,       islandBrainNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// In KQ4 1.000.111, falling down the lower stairs in room 90 sends a message to
//  a non-object, which also crashes the original. It appears that the fragment
//  of code which instantiated the Sound object was accidentally deleted from
//  the script. This line was intact in the previous version and in later
//  versions it was rewritten to use a new local Sound object. We fix this by
//  jumping from the broken code into the sound code in fallingToDeath, which
//  correctly plays the same sound.
//
// Applies to: PC 1.000.111
// Responsible method: fallingDown:changeState(0)
static const uint16 kq4SignatureFallDownStairs[] = {
	SIG_MAGICDWORD,
	0x83, 0x01,                      // lal 01  [ not set, zero ]
	0x4a, 0x10,                      // send 10 [ local1 number: 51 loop: 1 play: ]
	SIG_ADDTOOFFSET(+0x0108),
	0x38, SIG_SELECTOR16(new),       // pushi new
	0x76,                            // push0
	0x51, 0x31,                      // class Sound
	0x4a, 0x04,                      // send 04  [ Sound new: ]
	0x4a, 0x10,                      // send 10  [ sound number: 51 loop: 1 play: ]
	0x32, SIG_UINT16(0x0040),        // jmp 0040 [ end of method ]
	SIG_END
};

static const uint16 kq4PatchFallDownStairs[] = {
	0x32, PATCH_UINT16(0x0109),      // jmp 0109
	PATCH_ADDTOOFFSET(+0x0111),
	0xa2, PATCH_UINT16(0x0001),      // sal 0001 [ local1 = the new sound ]
	0x4a, 0x10,                      // send 10  [ local1 number: 51 loop: 1 play: ]
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq4Signatures[] = {
	{  true,    90, "fall down stairs",                            1, kq4SignatureFallDownStairs,               kq4PatchFallDownStairs },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// At least during the harpy scene, export 29 of script 0 is called and has an
//  issue where temp[3] won't get inititialized, but is later used to set
//  master volume. This makes SSCI set the volume to max. We fix the procedure,
//  so volume won't get modified in those cases.
//
// Applies to at least: PC CD
// Responsible method: export 29 in script 0
// Fixes bug: #5209
static const uint16 kq5SignatureCdHarpyVolume[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0191),        // lag global[191h]
	0x18,                            // not
	0x30, SIG_UINT16(0x002c),        // bnt [jump further] (jumping, if global[191h] is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, SIG_UINT16(0x0191),        // sag global[191h] (setting to 1)
	0x38, SIG_UINT16(0x017b),        // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume)
	0x38, SIG_UINT16(0x017b),        // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0x36,                            // push
	0x35, 0x04,                      // ldi 04
	0x20,                            // ge? (followed by bnt)
	SIG_END
};

static const uint16 kq5PatchCdHarpyVolume[] = {
	0x38, PATCH_UINT16(0x022f),      // pushi 022f (selector theVol) (3 new bytes)
	0x76,                            // push0 (1 new byte)
	0x51, 0x88,                      // class SpeakTimer (2 new bytes)
	0x4a, 0x04,                      // send 04 (2 new bytes) -> read SpeakTimer::theVol
	0xa5, 0x03,                      // sat temp[3] (2 new bytes)
	0x80, PATCH_UINT16(0x0191),      // lag global[191h]
	// saving 1 byte due optimization
	0x2e, PATCH_UINT16(0x0023),      // bt [jump further] (jumping, if global[191h] is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, PATCH_UINT16(0x0191),      // sag global[191h] (setting to 1)
	0x38, PATCH_UINT16(0x017b),      // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume)
	// saving 8 bytes due removing of duplicate code
	0x39, 0x04,                      // pushi 04 (saving 1 byte due swapping)
	0x22,                            // lt? (because we switched values)
	PATCH_END
};

// The witchCage object in script 200 is broken and claims to have 12
// properties instead of the 8 it should have because it is a Cage.
// Additionally its top,left,bottom,right properties are set to 0 rather
// than the right values. We fix the object by setting the right values.
// If they are all zero, this causes an impossible position check in
// witch::cantBeHere and an infinite loop when entering room 22.
//
// This bug is accidentally not triggered in SSCI because the invalid number
// of variables effectively hides witchCage::doit, causing this position check
// to be bypassed entirely.
// See also the warning+comment in Object::initBaseObject
//
// Applies to at least: PC CD, English PC floppy
// Responsible method: heap in script 200
// Fixes bug: #4964
static const uint16 kq5SignatureWitchCageInit[] = {
	SIG_UINT16(0x0000),         // top
	SIG_UINT16(0x0000),         // left
	SIG_UINT16(0x0000),         // bottom
	SIG_UINT16(0x0000),         // right
	SIG_UINT16(0x0000),         // extra property #1
	SIG_MAGICDWORD,
	SIG_UINT16(0x007a),         // extra property #2
	SIG_UINT16(0x00c8),         // extra property #3
	SIG_UINT16(0x00a3),         // extra property #4
	SIG_END
};

static const uint16 kq5PatchWitchCageInit[] = {
	PATCH_UINT16(0x0000),       // top
	PATCH_UINT16(0x007a),       // left
	PATCH_UINT16(0x00c8),       // bottom
	PATCH_UINT16(0x00a3),       // right
	PATCH_END
};

// The multilingual releases of KQ5 hang right at the end during the magic
// battle with Mordack. It seems additional code was added to wait for signals,
// but the signals are never set and thus the game hangs. We disable that code,
// so that the battle works again. This also happened in the original
// interpreter. We must not change similar code, that happens before.
//
// Applies to at least: French PC floppy, German PC floppy, Spanish PC floppy
// Responsible method: stingScript::changeState, dragonScript::changeState, snakeScript::changeState
static const uint16 kq5SignatureMultilingualEndingGlitch[] = {
	SIG_MAGICDWORD,
	0x89, 0x57,                      // lsg global[57h]
	0x35, 0x00,                      // ldi 0
	0x1a,                            // eq?
	0x18,                            // not
	0x30, SIG_UINT16(0x0011),        // bnt [skip signal check]
	SIG_ADDTOOFFSET(+8),             // skip globalSound::prevSignal get code
	0x36,                            // push
	0x35, 0x0a,                      // ldi 0Ah
	SIG_END
};

static const uint16 kq5PatchMultilingualEndingGlitch[] = {
	PATCH_ADDTOOFFSET(+6),
	0x32,                            // jmp (replace bnt)
	PATCH_END
};

// In the final battle, the DOS version uses signals in the music to handle
// timing, while in the Windows version another method is used and the GM
// tracks do not contain these signals. The original kq5 interpreter used
// global[400] to distinguish between Windows (1) and DOS (0) versions.
//
// We replace the global[400] checks with a fixed true. This is toggled with
// enablePatch() below when alternative Windows GM MIDI tracks are used.
//
// Instead, we could have set global[400], but this has the possibly unwanted
// side effects of switching to black&white cursors (which also needs complex
// changes to GameFeatures::detectsetCursorType()) and breaking savegame
// compatibilty between the DOS and Windows CD versions of KQ5.
//
// TODO: Investigate those side effects more closely.
// Applies to at least: Win CD
// Responsible method: mordOneScript::changeState(1), dragonScript::changeState(1),
//                     fireScript::changeState() in script 124
// Fixes bug: #6251
static const uint16 kq5SignatureWinGMSignals[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0190),        // lag global[400]
	0x18,                            // not
	0x30, SIG_UINT16(0x001b),        // bnt 0x001b
	0x89, 0x57,                      // lsg global[87]
	SIG_END
};

static const uint16 kq5PatchWinGMSignals[] = {
	0x34, PATCH_UINT16(0x0001),      // ldi 0x0001
	PATCH_END
};

// During the introduction at Crispin's house in later floppy versions, Graham's
//  initial message immediately disappears. This is another KQ5 sound regression
//  caused by a broken signal that a script depends on.
//
// Sound 80's first signal is supposed to be set after 781 ticks to trigger the
//  second message in room 109, but starting in the French version the sound
//  changed to set this signal on the first tick. This causes the first message
//  to miss the signal and display for too long until the second signal occurs
//  and closes the first message and immediately closes the second.
//
// We fix this by replacing the first signal wait with a 13 second delay.
//  781 ticks / 60 = 13 seconds and 1 tick. All subsequent signals in sound 80
//  are correct. Sierra noticed this bug in the German Amiga Floppy version, but
//  instead of fixing the broken sound, they peppered this script with similar
//  delays except for the one place that needed it. This covers up that the
//  wrong signals are still triggering messages with unintentional timings, but
//  the results are acceptable. Due to this messy history, and that it involves
//  multiple versions of multiple resources, we only enable this patch on game
//  versions known to disappear Graham's message.
//
// Applies to: French PC Floppy, Italian PC Floppy, English Mac Floppy,
//             English Amiga Floppy
// Responsible method: a2s5Script:changeState(2)
// Fixes bug: #11543
static const uint16 kq5SignatureCrispinIntroSignal[] = {
	SIG_MAGICDWORD,
	0x36,                            // push
	0x35, 0x0a,                      // ldi 0a
	0x22,                            // lt? [ globalMusic:prevSignal < 10 ]
	0x30, SIG_UINT16(0x0002),        // bnt 0002
	0x6d, 0x0a,                      // dpToa state
	0x35, 0x01,                      // ldi 01
	0x65, 0x10,                      // aTop cycles
	SIG_END
};

static const uint16 kq5PatchCrispinIntroSignal[] = {
	0x38, PATCH_SELECTOR16(seconds), // pushi seconds
	0x78,                            // push1
	0x39, 0x0d,                      // pushi 0d
	0x54, 0x06,                      // self 06 [ self seconds: 13 ]
	0x32, PATCH_UINT16(0x0002),      // jmp 0002
	PATCH_END
};

//          script, description,                                      signature                  patch
static const SciScriptPatcherEntry kq5Signatures[] = {
	{  true,     0, "CD: harpy volume change",                     1, kq5SignatureCdHarpyVolume,            kq5PatchCdHarpyVolume },
	{ false,   109, "Crispin intro signal",                        1, kq5SignatureCrispinIntroSignal,       kq5PatchCrispinIntroSignal },
	{  true,   124, "Multilingual: Ending glitching out",          3, kq5SignatureMultilingualEndingGlitch, kq5PatchMultilingualEndingGlitch },
	{ false,   124, "Win: GM Music signal checks",                 4, kq5SignatureWinGMSignals,             kq5PatchWinGMSignals },
	{  true,   200, "CD: witch cage init",                         1, kq5SignatureWitchCageInit,            kq5PatchWitchCageInit },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// In the garden (room 480), when giving the milk bottle to one of the babies,
// script 481 starts a looping a baby cry sound (cryMusic). However, that
// particular sound has an overriden check() method that explicitly restarts
// the sound, even if it's set to be looped. Thus the same sound is played
// twice, squelching all other sounds.
//
// We just rip out the unnecessary check() method, thereby stopping the sound
// from constantly restarting (since it's being looped anyway), thus the normal
// game speech can work while the baby cry sound is heard.
//
// Applies to at least: PC-CD
// Responsible method: cryMusic::check in script 481
// Fixes bug: #4955
static const uint16 kq6SignatureDuplicateBabyCry[] = {
	SIG_MAGICDWORD,
	0x83, 0x00,                      // lal local[0]
	0x31, 0x1e,                      // bnt 1e [07f4]
	0x78,                            // push1
	0x39, 0x04,                      // pushi 04
	0x43, 0x75, 0x02,                // callk DoAudio[75], 02
	SIG_END
};

static const uint16 kq6PatchDuplicateBabyCry[] = {
	0x48,                            // ret
	PATCH_END
};

// The inventory of King's Quest 6 is buggy. When it grows too large,
//  it will get split into 2 pages. Switching between those pages will
//  grow the stack, because it's calling itself per switch.
// Which means after a while ScummVM will bomb out because the stack frame
//  will be too large. This patch fixes the buggy script.
//
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: KqInv::showSelf in script 907
// Fixes bug: #5681
static const uint16 kq6SignatureInventoryStackFix[] = {
	0x67, 0x30,                         // pTos state
	0x34, SIG_UINT16(0x2000),           // ldi 2000
	0x12,                               // and
	0x18,                               // not
	0x31, 0x04,                         // bnt [not first refresh]
	0x35, 0x00,                         // ldi 00
	SIG_MAGICDWORD,
	0x65, 0x1e,                         // aTop curIcon
	0x67, 0x30,                         // pTos state
	0x34, SIG_UINT16(0xdfff),           // ldi dfff
	0x12,                               // and
	0x65, 0x30,                         // aTop state
	0x38, SIG_SELECTOR16(show),         // pushi show (e1h for KQ6CD)
	0x78,                               // push1
	0x87, 0x00,                         // lap param[0]
	0x31, 0x04,                         // bnt [use global for show]
	0x87, 0x01,                         // lap param[1]
	0x33, 0x02,                         // jmp [use param for show]
	0x81, 0x00,                         // lag global[0]
	0x36,                               // push
	0x54, 0x06,                         // self 06 (KqInv::show)
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [exit menu code] (0x08 for PC, 0x07 for mac)
	0x39, 0x39,                         // pushi 39
	0x76,                               // push0
	0x54, 0x04,                         // self 04 (KqInv::doit)
	SIG_END                             // followed by jmp (0x32 for PC, 0x33 for mac)
};

static const uint16 kq6PatchInventoryStackFix[] = {
	0x67, 0x30,                         // pTos state
	0x3c,                               // dup (1 more byte, needed for patch)
	0x3c,                               // dup (1 more byte, saves 1 byte later)
	0x34, PATCH_UINT16(0x2000),         // ldi 2000
	0x12,                               // and
	0x2f, 0x02,                         // bt [not first refresh] - saves 3 bytes in total
	0x65, 0x1e,                         // aTop curIcon
	0x00,                               // bnot (neg, either 2000 or 0000 in acc, this will create dfff or ffff) - saves 2 bytes
	0x12,                               // and
	0x65, 0x30,                         // aTop state
	0x38, PATCH_GETORIGINALUINT16(+22), // pushi show
	0x78,                               // push1
	0x87, 0x00,                         // lap param[0]
	0x31, 0x04,                         // bnt [call show using global[0]]
	0x8f, 0x01,                         // lsp param[1], save 1 byte total with lsg global[0] combined
	0x33, 0x02,                         // jmp [call show using param[1]]
	0x89, 0x00,                         // lsg global[0], save 1 byte total, see above
	0x54, 0x06,                         // self 06 (call x::show)
	0x31, PATCH_GETORIGINALBYTEADJUST(+39, +6), // bnt [menu exit code] (0x0e for pc, 0x0d for mac)
	0x34, PATCH_UINT16(0x2000),         // ldi 2000
	0x12,                               // and
	0x2f, 0x05,                         // bt [to ret]
	0x39, 0x39,                         // pushi 39
	0x76,                               // push0
	0x54, 0x04,                         // self 04 (self::doit)
	0x48,                               // ret (saves 2 bytes for PC, 1 byte for mac)
	PATCH_END
};

// The "Drink Me" bottle code doesn't repaint the AddToPics elements to the
//  screen, when Alexander returns back from the effect of the bottle.
//  It's pretty strange that Sierra didn't find this bug, because it occurs
//  when drinking the bottle right on the screen, where the bottle is found.
// This bug also occurs in Sierra SCI.
//
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: drinkMeScript::changeState in script 87
// Fixes bug: #5252
static const uint16 kq6SignatureDrinkMeFix[] = {
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x0f,                         // ldi 0f
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x00a4),           // bnt [skip to next check]
	SIG_ADDTOOFFSET(+161),
	0x32, SIG_UINT16(0x007f),           // jmp [return]
	0x3c,                               // dup
	0x35, 0x10,                         // ldi 10
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt [skip to next check]
	0x35, 0x03,                         // ldi 03
	0x65, 0x1a,                         // aTop (cycles)
	0x32, SIG_UINT16(0x0072),           // jmp [return]
	0x3c,                               // dup
	0x35, 0x11,                         // ldi 11
	0x1a,                               // eq?
	0x31, 0x13,                         // bnt [skip to next check]
	SIG_ADDTOOFFSET(+20),
	0x35, 0x12,                         // ldi 12
	SIG_ADDTOOFFSET(+23),
	0x35, 0x13,                         // ldi 13
	SIG_END
};

static const uint16 kq6PatchDrinkMeFix[] = {
	PATCH_ADDTOOFFSET(+4),
	0x30, PATCH_UINT16(0x00b1),         // bnt 00b1 [ check for 11h code ]
	PATCH_ADDTOOFFSET(+161),
	0x39, PATCH_SELECTOR8(doit),        // pushi doit
	0x76,                               // push0
	0x81, 0x0a,                         // lag global[0a]
	0x4a, 0x04,                         // send 04 (call addToPics::doit)
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_ADDTOOFFSET(+8),              // skip to check 11h code
	0x35, 0x10,                         // ldi 10 (instead of 11)
	PATCH_ADDTOOFFSET(+23),             // skip to check 12h code
	0x35, 0x11,                         // ldi 11 (instead of 12)
	PATCH_ADDTOOFFSET(+23),             // skip to check 13h code
	0x35, 0x12,                         // ldi 12 (instead of 13)
	PATCH_END
};

// KQ6 Mac is missing pic 981 and crashes when drinking the "Drink Me" bottle.
//  This also crashed the original. Pic 981 is a black background and it's only
//  used in this scene. Pic 98 is also a black background and used when the game
//  starts and everywhere else. We restore this scene by switching to pic 98.
//
// This patch is only enabled on Mac as the script is the same in all versions.
//  The pics have different palettes and applying it to PC would disable the
//  palette cycling between red and black during this scene. KQ6 Mac didn't do
//  these palette cycling effects as it didn't include any palette resources.
//
// Applies to: English Mac
// Responsible method: drinkMeScript:changeState(0)
static const uint16 kq6SignatureMacDrinkMePic[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x03d5),           // pushi 981d
	0x39, 0x09,                         // pushi 09
	0x43, 0x08, 0x04,                   // callk DrawPic 04
	SIG_END
};

static const uint16 kq6PatchMacDrinkMePic[] = {
	0x38, PATCH_UINT16(0x0062),         // pushi 98d
	PATCH_END
};

// During the common Game Over cutscene, one of the guys says "Tickets, only",
//  but the subtitle says "Tickets, please". Normally people wouldn't have
//  noticed, but ScummVM supports audio + subtitles in this game at the same
//  time. This is caused by a buggy message, which really has this text + audio
//  attached.
// We assume that "Tickets, only" (the audio) is the correct one. There is
//  another message with "Tickets, only" in both text and audio. We change
//  message 1, 0, 1, 1 to message 5, 0, 0, 2 to fix this issue.
// This mismatch also occurs in Sierra SCI.
//
// Applies to at least: PC-CD
// Responsible method: modeLessScript::changeState(0) in script 640
static const uint16 kq6SignatureTicketsOnly[] = {
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	SIG_MAGICDWORD,
	0x31, 0x2b,                         // bnt [skip over state 0]
	0x39, 0x1e,                         // pushi font (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, SIG_UINT16(0x009a),           // pushi posn
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c8),           // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, SIG_UINT16(0x00ab),           // pushi say
	0x39, 0x05,                         // pushi 05 (parameter count for say)
	0x76,                               // push0
	0x78,                               // push1
	0x76,                               // push0
	0x78,                               // push1
	0x78,                               // push1
	SIG_END
};

static const uint16 kq6PatchTicketsOnly[] = {
	0x32, PATCH_UINT16(0x0000),         // jmp (waste 3 bytes)
	0x2f, 0x2c,                         // bt [skip over state 0] (saves 1 byte)
	0x39, 0x1e,                         // pushi font (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, PATCH_UINT16(0x009a),         // pushi posn
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x00c8),         // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, PATCH_UINT16(0x00ab),         // pushi say
	0x39, 0x05,                         // pushi 05 (parameter count for say)
	0x76,                               // push0
	0x39, 0x05,                         // pushi 05
	0x76,                               // push0
	0x76,                               // push0
	0x7a,                               // push2
	PATCH_END
};

// Looking at the ribbon in inventory says that there's a hair even after it's
//  been removed. This occurs after the hair has been put in the skull or is on
//  a different inventory page than the ribbon.
//
// The ribbon's Look handler has incorrect logic for determining if it contains
//  a hair. It fails to test flag 143 which is set when getting the hair and so
//  it displays the wrong message. The Do handler tests all the necessary flags.
//  This bug probably would have been noticed except that both verb handlers
//  also test inventory for hair, which is redundant as testing flags is enough,
//  but it causes the right message some of the time. Testing inventory is wrong
//  because possessing the hair is temporary, which is why the bug emerges after
//  it's used, and it's broken because testing inventory across pages doesn't
//  work in KQ6. ego:has returns false for any item on another page when the
//  inventory window is open. As inventory increases the ribbon and hair end up
//  on different pages and ribbon:doVerb can no longer see it.
//
// We fix the message by changing ribbon:doVerb(1) to test flag 143 like doVerb(5).
//  This requires overwriting one of the redundant inventory tests.
//
// Beauty's clothes also have a hair and clothes:doVerb(1) has similar issues
//  but it happens to work. Those items are always on the same page due to their
//  low item numbers and the clothes are removed from inventory before the hair.
//
// Applies to: PC CD, PC Floppy, Mac Floppy
// Responsible method: ribbon:doVerb(1) in script 907
// Fixes bug: #10801
static const uint16 kq6SignatureLookRibbonFix[] = {
	0x30, SIG_ADDTOOFFSET(+2),          // bnt [ verb != Look ]
	0x38, SIG_SELECTOR16(has),          // pushi has
	0x78,                               // push1
	0x39, 0x04,                         // pushi 04
	0x81, SIG_MAGICDWORD, 0x00,         // lag global[0]
	0x4a, 0x06,                         // send 6 [ ego:has 4 (beauty's hair) ]
	0x2e,                               // bt [ continue hair tests ]
	SIG_END
};

static const uint16 kq6PatchLookRibbonFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                               // push1
	0x38, PATCH_UINT16(0x008f),         // pushi 008f
	0x46, PATCH_UINT16(0x0391),         // calle [export 0 of script 13], 02 [ is flag 8f set? ]
	      PATCH_UINT16(0x0000), 0x02,
	PATCH_END
};

// KQ6 CD introduced a bug in the wallflower dance in room 480. The dance is
//  supposed to last until the music ends but in Text mode it stops after only
//  three seconds once the user gains control. This isn't usually enough time
//  to get the hole in the wall. This bug also occurs in Sierra's interpreter.
//
// wallFlowerDance was changed in the CD version for Speech mode but broke Text.
//  In Text mode, changeState(9) creates a dialog with Print, which blocks, and
//  then sets ticks to 12. Meanwhile, wallFlowerDance:handleEvent cues if an
//  event is received in state 9. A mouse click starts a 12 tick race which
//  handleEvent wins, cueing before the countdown expires, and so the countdown
//  expires on state 10, skipping ahead to the three second fadeout. Closing the
//  dialog with the keyboard works because Dialog claims keyboard events when
//  blocking, preventing wallFlowerDance:handleEvent from receiving and cueing.
//
// We fix this by setting the Print dialog to modeless as it was in the floppy
//  version and removing the countdown. wallFlowerDance:handleEvent now receives
//  all events and is the only one responsible for advancing state 9 to 10 in
//  Text mode. This patch does not affect audio modes Speech and Both.
//
// Applies to: PC CD
// Responsible method: wallFlowerDance:changeState(9) in script 480
// Fixes bug: #10811
static const uint16 kq6CDSignatureWallFlowerDanceFix[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(init),          // pushi init
	0x76,                               // push0
	0x51, 0x15,                         // class Print [ Print: ... init ]
	0x4a, 0x24,                         // send 24
	0x35, 0x0c,                         // ldi 0c
	0x65, 0x20,                         // aTop ticks
	0x32, SIG_UINT16(0x00d0),           // jmp 00d0 [ end of method ]
	SIG_END
};

static const uint16 kq6CDPatchWallFlowerDanceFix[] = {
	0x38, PATCH_SELECTOR16(modeless),   // pushi modeless
	0x78,                               // push1
	0x78,                               // push1
	0x39, PATCH_SELECTOR8(init),        // pushi init
	0x76,                               // push0
	0x51, 0x15,                         // class Print [ Print: ... modeless: 1, init ]
	0x4a, 0x2a,                         // send 2a
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_END
};

// In room 300 at the bottom of the logic cliffs, clicking Walk while Alexander
//  randomly wobbles on lower steps causes him to float around the room. Scripts
//  attempt to prevent this by ignoring Walk when ego:view is 301 and ego:loop
//  is 3, which is wobbling animation, but this is incomplete because egoWobbles
//  also animates with view 3011. The rest of the cliff rooms directly test if
//  egoWobbles is running and so they don't have this bug. egoWobbles isn't
//  exported so room 300 is left testing ego's state.
//
// We fix this by adding the missing test for view 3011 so that the Walk verb is
//  successfully ignored whenever egoWobbles is running.
//
// Applies to: All versions
// Responsible methods: beach:doVerb, WalkFeature:doVerb
static const uint16 kq6SignatureCliffStepFloatFix[] = {
	0x36,                               // push
	0x34, SIG_UINT16(0x012d),           // ldi 012d
	0x1a,                               // eq?      [ ego:view == 301 ]
	0x31, 0x12,                         // bnt 12   [ not wobbling ]
	0x39, 0x03,                         // pushi 03 [ loop ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04  [ ego loop? ]
	0x36,                               // push
	0x35, SIG_MAGICDWORD, 0x03,         // ldi 03
	0x1a,                               // eq?      [ ego:loop == 3 ]
	0x31, 0x05,                         // bnt 05   [ not wobbling ]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_ADDTOOFFSET(+2),          // jmp      [ end of method ]
	0x76,                               // push0    [ y ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04  [ ego y? ]
	0x36,                               // push
	0x35, 0x26,                         // ldi 26
	0x22,                               // lt?      [ ego:y < 38 ]
	SIG_ADDTOOFFSET(+17),
	0x32,                               // jmp      [ end of method ]
	SIG_END
};

static const uint16 kq6PatchCliffStepFloatFix[] = {
	0x38, PATCH_UINT16(0x0bc3),         // pushi 0bc3
	0x1a,                               // eq?      [ ego:view == 3011 ]
	0x2f, 0x12,                         // bt 12    [ wobbling ]
	0x60,                               // pprev
	0x34, PATCH_UINT16(0x012d),         // ldi 012d
	0x1a,                               // eq?      [ ego:view == 301 ]
	0x31, 0x0d,                         // bnt 0d   [ not wobbling ]
	0x39, 0x03,                         // pushi 03
	0x3c,                               // dup      [ loop ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04  [ ego loop? ]
	0x1a,                               // eq?      [ ego:loop == 3 ]
	0x31, 0x02,                         // bnt 02   [ not wobbling ]
	0x33, 0x1a,                         // jmp 1a   [ end of method ]
	0x76,                               // push0    [ y ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04  [ ego y? ]
	0x39, 0x26,                         // pushi 26
	0x20,                               // ge?      [ 38 >= ego:y ]
	PATCH_END
};

// Similar to the patched bug above, clicking an item on the beach in room 300
//  while wobbling on the bottom step causes Alexander to float around the room.
//  The doVerb methods for the flower and the feather incorrectly test ego:y
//  using a less than comparison instead of less than or equals to, which is
//  what the rest of the methods in this script do.
//
// Applies to: All versions
// Responsible methods: stench:doVerb, feather:doVerb
static const uint16 kq6SignatureCliffItemFloatFix[] = {
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35, 0x69,                         // ldi 69
	0x22,                               // lt?
	SIG_END
};

static const uint16 kq6PatchCliffItemFloatFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x24,                               // le?
	PATCH_END
};

// After casting the Make Rain spell, collecting the baby's tears a second time
//  awards a duplicate point. The script getBabyTears is missing a flag check to
//  prevent this. The falling water from Beast's fountain doesn't have this bug
//  because it tests flag 14 before awarding its point. Flag 14 is set when the
//  Druids catch Alexander. We add the missing flag check.
//
// Applies to: All versions
// Responsible method: getBabyTears:changeState(4)
static const uint16 kq6SignatureDuplicateBabyTearsPoint[] = {
	0x31, 0x0f,                         // bnt 0f [ state 4 ]
	SIG_ADDTOOFFSET(+12),
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0040),           // bnt 0040
	0x89, 0xa1,                         // lsg a1
	0x35, SIG_MAGICDWORD, 0x04,         // ldi 04
	0x14,                               // or
	0xa1, 0xa1,                         // sag a1
	0x38, SIG_SELECTOR16(givePoints),   // pushi givePoints
	0x78,                               // push1
	0x78,                               // push1
	0x81, 0x01,                         // lag 01
	0x4a, 0x06,                         // send 06 [ Kq6 givePoints: 1 ]
	SIG_ADDTOOFFSET(+45),
	0x32,                               // jmp [ end of method ]
	SIG_END
};

static const uint16 kq6PatchDuplicateBabyTearsPoint[] = {
	0x31, 0x0c,                         // bnt 0c [ state 4 ]
	PATCH_ADDTOOFFSET(+12),
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x31, 0x44,                         // bnt 44
	0x81, 0xa1,                         // lag a1
	0x14,                               // or
	0xa1, 0xa1,                         // sag a1
	0x81, 0x89,                         // lag 89 [ flags ]
	0x7a,                               // push2  [ flag 14 ]
	0x12,                               // and    [ has storm occurred? ]
	0x2f, 0x09,                         // bt 09  [ skip point ]
	PATCH_ADDTOOFFSET(+54),
	0x48,                               // ret    [ prevent toss, stack is empty ]
	0x00,                               // bnot
	0x00,                               // bnot
	PATCH_END
};

// Clicking the lamp on the Baby Tears when the lamp already contains the sacred
//  water awards the tears even if the babies aren't crying, allowing the puzzle
//  to be bypassed. The script tests for the sacred water, even though the water
//  shouldn't affect this sequence, and its only function is to skip the crying
//  test which is always necessary. We fix this by disabling the water test.
//
// Applies to: All versions
// Responsible method: Brat:doVerb
static const uint16 kq6SignatureGetBabyTears[] = {
	SIG_MAGICDWORD,
	0x89, 0xa1,                         // lsg a1 [ lamp flags ]
	0x35, 0x02,                         // ldi 02 [ sacred water ]
	0x12,                               // and    [ is sacred water in lamp? ]
	SIG_END
};

static const uint16 kq6PatchGetBabyTears[] = {
	0x39, 0x00,                         // pushi 00 [ disable sacred water test ]
	PATCH_END
};

// KQ6 truncates messages longer than 400 characters in the CD and Mac versions.
//  This is most prominent when reading Cassima's letter to Alexander. When the
//  Messager class was upgraded to support audio, a 400 character buffer was
//  added to read messages into, but KQ6 has messages as long as 602 characters.
//  This limitation is in other games with this same Messager class. LB2, for
//  example, has the same class but no messages beyond 400 characters.
//
// We fix this by increasing the temporary variables in Messager:sayNext from
//  202 to 303 to accommodate the longest message. This patch implicitly depends
//  on the Audio+Subtitle patches below, as they coincidentally alter sayNext
//  in a way that this patch would otherwise have to. The CD version uses
//  temp201, which we would normally have to patch to temp302 since we're
//  expanding the buffer before it, but the Audio+Subtitle patches happen to
//  remove or disable all use of this variable.
//
// Applies to: PC CD, Macintosh
// Responsible method: Messager:sayNext
// Fixes bug: #10682
static const uint16 kq6SignatureTruncatedMessagesFix[] = {
	SIG_MAGICDWORD,
	0x3f, 0xca,                         // link ca
	0x87, 0x00,                         // lap 00
	0x31, 0x18,                         // bnt 18
	0x39, 0x07,                         // pushi 07
	0x76,                               // push0
	0x8f, 0x01,                         // lsp 01
	0x8f, 0x02,                         // lsp 02
	0x8f, 0x03,                         // lsp 03
	0x8f, 0x04,                         // lsp 04
	0x8f, 0x05,                         // lsp 05
	0x5b, 0x04, 0x01,                   // lea 04 01
	0x36,                               // push
	0x43, 0x7c, 0x0e,                   // callk Message 0e
	0xa5, 0x00,                         // sat 00
	0x33, 0x0b,                         // jmp 0b
	SIG_ADDTOOFFSET(+9),
	0xa5, 0x00,                         // sat 00
	SIG_END
};

static const uint16 kq6PatchTruncatedMessagesFix[] = {
	0x3e, PATCH_UINT16(0x012f),         // link 012f
	0x87, 0x00,                         // lap 00
	0x31, 0x17,                         // bnt 17
	0x39, 0x07,                         // pushi 07
	0x76,                               // push0
	0x8f, 0x01,                         // lsp 01
	0x8f, 0x02,                         // lsp 02
	0x8f, 0x03,                         // lsp 03
	0x8f, 0x04,                         // lsp 04
	0x8f, 0x05,                         // lsp 05
	0x5b, 0x04, 0x01,                   // lea 04 01
	0x36,                               // push
	0x43, 0x7c, 0x0e,                   // callk Message 0e
	0x32, PATCH_UINT16(0x0009),         // jmp 0009
	PATCH_END
};

// Clicking Look or Do on the secret passage peephole outside of Cassima's room
//  multiple times can lockup the game and crash the original.
//
// The bug is in the Feature and CueObj classes but room 800 and cassimaScript
//  happen to expose it. Clicking on a Feature such as the hole causes ego to
//  walk towards it, then face it, wait three cycles, and then call doVerb.
//  Feature:handleEvent calls ego:setMotion and then has CueObj orchestrate the
//  rest of the sequence. If ego is instead already within approachDist and
//  facing the clicked Feature then handleEvent sets CueObj to its final state
//  so that doVerb is called immediately. handleEvent resets CueObj's state and
//  stops any ego movement to prevent a conflict from an earlier call but it
//  doesn't stop ego's looper from turning to face the Feature. Clicking on a
//  Feature while ego is within approachDist and in the process of turning to
//  face that Feature from a previous click causes both clicks to generate calls
//  to doVerb, and out of order. In room 800 that causes cassimaScript to run a
//  a second time and interrupt the first which breaks the game in various ways.
//
// We fix room 800 by adding a check to prevent running a peephole script if
//  there's already a script running.
//
// Applies to: All versions
// Responsible method: chink:doVerb
static const uint16 kq6SignatureCassimaSecretPassage[] = {
	SIG_MAGICDWORD,
	0x67, 0x1a,                         // pTos noun         
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x31, 0x12,                         // bnt 12
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x78,                               // push1
	0x38, SIG_UINT16(0x0321),           // pushi 0321
	0x43, 0x02, 0x02,                   // callk ScriptID 02 [ ScriptID 801 ] 
	0x36,                               // push
	0x81, 0x02,                         // lag 02
	0x4a, 0x06,                         // send 06 [ rm800 setScript: cassimaScript ]
	0x33, 0x10,                         // jmp 10
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	SIG_ADDTOOFFSET(+5),
	0x43, 0x02, 0x02,                   // callk ScriptID 02 [ ScriptID 802 ] 
	SIG_END
};

static const uint16 kq6PatchCassimaSecretPassage[] = {
	0x38, PATCH_SELECTOR16(script),     // pushi script
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, 0x04,                         // send 04 [ rm800 script? ]
	0x31, 0x01,                         // bnt 01
	0x48,                               // ret [ do nothing if script is running ]
	0x67, 0x1a,                         // pTos noun         
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x31, 0x07,                         // bnt 07
	0x78,                               // push1
	0x78,                               // push1
	0x38, PATCH_UINT16(0x0321),         // pushi 0321
	0x33, 0x05,                         // jmp 05 [ callk ScriptID 02 ]
	PATCH_END
};

// WORKAROUND
//
// Dangling Participle and Rotten Tomato are inventory items that can talk, but
//  their mouths are animated by inner loops that call kDrawCel with unthrottled
//  inner inner loops that spin to create a delay between frames. This prevents
//  updating the screen and responding to input. We replace the spin loops with
//  calls to kGameIsRestarting, which detects and throttles these calls so that
//  the speed is reasonable, the screen updates, and the game is responsive.
//
// Applies to: All versions
// Responsible method: participle:doVerb, tomato:doVerb
static const uint16 kq6SignatureTalkingInventory[] = {
	0x35, 0x00,                         // ldi 00
	0xa5, 0x01,                         // sat 01
	0x8d, 0x01,                         // lst 01
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x1b58),           // ldi 1b58
	0x22,                               // lt? [ temp1 < 7000 ]
	SIG_END
};

static const uint16 kq6PatchTalkingInventory[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x00,                         // pushi 00
	0x43, 0x2c, 0x00,                   // callk GameIsRestarting [ custom throttling ]
	0x34, PATCH_UINT16(0x0000),         // ldi 0000 [ exit loop ]
	PATCH_END
};

// Exiting the pawnshop while the Genie's eye is glinting locks up the game.
//  Unlike the bookstore, the pawnshop script fails to dispose the "eye" object
//  if it's in the middle of animating, causing the door animation in the next
//  room to loop forever. Sierra added a simple workaround in later versions to
//  prevent this: the eye no longer glints when ego is close the pawnshop exit.
//  We apply this same workaround to vulnerable versions.
//
// Applies to: English, French, and German PC Floppy, English Mac Floppy,
//             English PC CD 1.000.000
// Responsible method: genieBrowseScr:changeState(3)
static const uint16 kq6SignaturePawnshopGenieEye[] = {
	0x39, 0x03,                         // pushi 03
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x76,                               // push0
	0x7a,                               // push2
	0x43, 0x3c, 0x04,                   // callk Random [ Random 0 2 ]
	0x36,                               // push
	0x76,                               // push0
	0x78,                               // push1
	0x46, SIG_UINT16(0x03e7),           // calle proc999_5 [ OneOf (Random 0 2) 0 1 ]
	      SIG_UINT16(0x0005), 0x06,
	SIG_END
};

static const uint16 kq6PatchPawnshopGenieEye[] = {
	0x39, 0x43,                         // pushi 43
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04 [ ego x? ]
	0x22,                               // lt?     [ 67 < ego:x ]
	0x31, 0x06,                         // bnt 06  [ skip glint ]
	0x7a,                               // push2
	0x76,                               // push0
	0x7a,                               // push2
	0x43, 0x3c, 0x04,                   // callk Random [ Random 0 2 ]
	PATCH_END
};

// During the wedding close-up with the Vizier and the Genie in room 740,
//  clicking almost any of the wrong lamps on the Genie results in the wrong
//  message sequence. Alexander says "I have this lamp, 'princess'" twice
//  instead of the Genie responding after the first message. The script passes
//  the wrong sequence number in the second message tuple.
//
// Applies to: All versions
// Responsible method: genieHead:doVerb
static const uint16 kq6SignatureWeddingGenieLampMessage[] = {
	0x39, SIG_SELECTOR8(add),           // pushi add
	0x39, 0x05,                         // pushi 05
	0x67, SIG_ADDTOOFFSET(+1),          // pTos modNum
	0x67, SIG_ADDTOOFFSET(+1),          // pTos noun
	SIG_MAGICDWORD,
	0x39, 0x39,                         // pushi 39 [ verb ]
	0x76,                               // push0    [ cond ]
	0x78,                               // push1    [ wrong seq ]
	SIG_END
};

static const uint16 kq6PatchWeddingGenieLampMessage[] = {
	PATCH_ADDTOOFFSET(+11),
	0x7a,                               // push2 [ correct seq ]
	PATCH_END
};

// Clicking Look on the first catacombs room (405) or the dead-end room (407)
//  while in text-mode breaks the CD and Spanish versions. The message boxes
//  never leave the screen, even after restarting. The doVerb methods in these
//  rooms are missing the necessary "return true" statements that were added to
//  all other rooms in the CD version. In earlier versions, Messager:say always
//  set the accumulator to non-zero and these statements weren't necessary.
//
// Applies to: PC CD, Spanish PC Floppy
// Responsible methods: rm405:doVerb(1), rm407:doVerb(1)
static const uint16 kq6SignatureRoom405LookMessage[] = {
	0x33, 0x1f,                         // jmp 1f
	SIG_ADDTOOFFSET(+18),
	0x33, 0x0b,                         // jmp 0b
	SIG_ADDTOOFFSET(+8),
	SIG_MAGICDWORD,
	0x57, 0x93, 0x06,                   // super LabRoom 06 [ super doVerb: param1 &rest ]
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 kq6PatchRoom405LookMessage[] = {
	0x14,                               // or [ acc |= 1  ]
	0x48,                               // ret
	PATCH_ADDTOOFFSET(+18),
	0x14,                               // or [ acc |= 1  ]
	0x48,                               // ret
	PATCH_END
};

static const uint16 kq6SignatureRoom407LookMessage[] = {
	0x33, 0x0b,                         // jmp 0b
	SIG_ADDTOOFFSET(+8),
	SIG_MAGICDWORD,
	0x57, 0x93, 0x06,                   // super LabRoom 06 [ super doVerb: param1 &rest ]
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 kq6PatchRoom407LookMessage[] = {
	0x14,                               // or [ acc |= 1  ]
	0x48,                               // ret
	PATCH_END
};

// After lighting the torch in the dark catacombs room and then re-entering,
//  using items such as a book results in a message that the room is too dark.
//  rm406:scriptCheck tests for darkness with a local variable that is set when
//  lighting the torch instead of a persistent global. We could change the
//  darkness test but it turns out that this method is never called in the dark.
//  KQ6Room:setScript only calls scriptCheck when ego:view is 900 and that is
//  only true once the torch is lit.
//
// We fix this by patching rm406:scriptCheck to always allow items since the
//  condition it tries to prevent can never occur.
//
// Applies to: All versions
// Responsible method: rm406:scriptCheck
static const uint16 kq6SignatureDarkRoomInventory[] = {
	0x3f, 0x01,                         // link 01
	0x35, 0x00,                         // ldi 00
	0xa5, 0x00,                         // sat temp0
	0x8b, SIG_MAGICDWORD, 0x01,         // lsl 01
	0x35, 0x64,                         // ldi 64
	0x22,                               // lt? [ has lightItUp not run yet? ]
	SIG_END
};

static const uint16 kq6PatchDarkRoomInventory[] = {
	0x35, 0x01,                         // ldi 01
	0x48,                               // ret [ return true, allow items ]
	PATCH_END
};

// The Girl In The Tower theme plays from a CD audio track during the credits.
//  In ScummVM this requires an actual CD, or a mounted CD image, or extracting
//  the audio to a "track1" file ahead of time. If the track can't be played
//  then there is silence for the entire credits, but the game includes a 5 MB
//  Audio version to fall back on when CD audio is unavailable. The script plays
//  this when the CD audio driver fails to load, though this never occurs in our
//  our implementation and doesn't address a missing track.
//
// We ensure that the credits theme always plays by patching the script to also
//  fall back on the Audio version when CD audio playback fails.
//
// Applies to: PC CD
// Responsible method: sCredits:init
static const uint16 kq6CDSignatureGirlInTheTowerPlayback[] = {
	SIG_MAGICDWORD,
	0x39, 0x05,                         // pushi 05
	0x39, 0x0a,                         // pushi 0a   [ kSciAudioCD ]
	0x7a,                               // push2      [ play ]
	0x7a,                               // push2      [ track ]
	0x76,                               // push0      [ start ]
	0x38, SIG_UINT16(0x00ec),           // pushi 00ec [ end ]
	0x43, 0x75, 0x0a,                   // callk DoAudio 0a
	0x33,                               // jmp [ skip Audio version ]
	SIG_END
};

static const uint16 kq6CDPatchGirlInTheTowerPlayback[] = {
	PATCH_ADDTOOFFSET(+13),
	0x2f,                               // bt [ skip Audio version if CD audio succeeded ]
	PATCH_END
};

// Audio + subtitles support - SHARED! - used for King's Quest 6 and Laura Bow 2.
//  This patch gets enabled when the user selects "both" in the ScummVM
//  "Speech + Subtitles" menu. We currently use global[98d] to hold a kMemory
//  pointer.
// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (always use text branch)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport1[] = {
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	SIG_MAGICDWORD,
	0x31, 0x13,                         // bnt [audio call]
	0x38, SIG_SELECTOR16(modNum),       // pushi modNum
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport1[] = {
	PATCH_ADDTOOFFSET(+5),
	0x33, 0x13,                         // jmp [audio call]
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (allocate audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport2[] = {
	0x7a,                               // push2
	0x78,                               // push1
	0x39, 0x0c,                         // pushi 0c
	0x43, SIG_MAGICDWORD, 0x72, 0x04,   // callk Memory
	0xa5, 0xc9,                         // sat temp[c9]
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport2[] = {
	PATCH_ADDTOOFFSET(+7),
	0xa1, 0x62,                         // sag global[62] (global[98d])
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (release audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport3[] = {
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	SIG_MAGICDWORD,
	0x8d, 0xc9,                         // lst temp[c9]
	0x43, 0x72, 0x04,                   // callk Memory
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport3[] = {
	PATCH_ADDTOOFFSET(+3),
	0x89, 0x62,                         // lsg global[62] (global[98d])
	PATCH_END
};

// startText call gets acc = 0 for text-only and acc = 2 for audio+text
// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Narrator::say (use audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport4[] = {
	// set caller property code
	0x31, 0x08,                         // bnt [set acc to 0 for caller]
	0x87, 0x02,                         // lap param[2]
	0x31, 0x04,                         // bnt [set acc to 0 for caller]
	0x87, 0x02,                         // lap param[2]
	0x33, 0x02,                         // jmp [set caller]
	0x35, 0x00,                         // ldi 00
	0x65, 0x68,                         // aTop caller
	// call startText + startAudio code
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x12,                               // and
	0x31, 0x08,                         // bnt [skip code]
	0x38, SIG_SELECTOR16(startText),    // pushi startText
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x54, 0x06,                         // self 06
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31, 0x08,                         // bnt [skip code]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(startAudio),   // pushi startAudio
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x54, 0x06,                         // self 06
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport4[] = {
	0x31, 0x02,                         // bnt [set caller]
	0x87, 0x02,                         // lap param[2]
	0x65, 0x68,                         // aTop caller
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x12,                               // and
	0x31, 0x11,                         // bnt [skip startText code]
	0x81, 0x5a,                         // lag global[5a]
	0x7a,                               // push2
	0x12,                               // and
	0x33, 0x03,                         // jmp 3 [skip unused bytes]
	PATCH_ADDTOOFFSET(+22),
	0x89, 0x62,                         // lsg global[62] (global[98d])
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Talker::display/Narrator::say (remove reset saved mouse cursor code)
//  code would screw over mouse cursor
static const uint16 kq6laurabow2CDSignatureAudioTextSupport5[] = {
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 00
	0x65, 0x82,                         // aTop saveCursor
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport5[] = {
	0x18, 0x18, 0x18, 0x18,             // (waste bytes)
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes text window placement, when in "dual" mode
// Applies to at least: PC-CD
// Patched method: Kq6Talker::init
static const uint16 kq6CDSignatureAudioTextSupport1[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [jump-to-text-code]
	0x78,                               // push1
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport1[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes low-res portrait staying on screen for hi-res mode
// Applies to at least: PC-CD
// Patched method: Talker::startText
//  this method is called by Narrator::say and acc is 0 for text-only and 2 for dual mode (audio+text)
static const uint16 kq6CDSignatureAudioTextSupport2[] = {
	SIG_MAGICDWORD,
	0x3f, 0x01,                         // link 01
	0x63, 0x8a,                         // pToa viewInPrint
	0x18,                               // not
	0x31, 0x06,                         // bnt [skip following code]
	0x38, SIG_UINT16(0x00e1),           // pushi 00e1
	0x76,                               // push0
	0x54, 0x04,                         // self 04
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport2[] = {
	PATCH_ADDTOOFFSET(+2),
	0x67, 0x8a,                         // pTos viewInPrint
	0x14,                               // or
	0x2f,                               // bt [skip following code]
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes special windows, used for example in...
//   The Pawn shop (room 280), when the man in a robe complains about no more
//    mints.
//   Room 300 at the cliffs (aka copy protection), when Alexander falls down
//    the cliffs (closes automatically, but too late).
//   Room 210, when Alexander gives the ring to the nightingale (these need a
//    mouse click).
//
//  We have to change even more code because the game uses PODialog class for
//   text windows and myDialog class for audio. Both are saved to
//   KQ6Print::dialog.
//
//  Changing KQ6Print::dialog is disabled for now, because it has side-effects
//   (breaking game over screens).
//
//  Original comment:
//  Sadly PODialog is created during KQ6Print::addText, myDialog is set during
//   KQ6Print::showSelf, which is called much later and KQ6Print::addText
//   requires KQ6Print::dialog to be set, which means we have to set it before
//   calling addText for audio mode, otherwise the user would have to click to
//   get those windows disposed.
//
// Applies to at least: PC-CD
// Patched method: KQ6Print::say
static const uint16 kq6CDSignatureAudioTextSupport3[] = {
	0x31, 0x6e,                         // bnt [to text code]
	SIG_ADDTOOFFSET(+85),
	SIG_MAGICDWORD,
	0x8f, 0x01,                         // lsp param[1]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt [code to set property repressText to 1]
	0x38,                               // pushi (selector addText)
	SIG_ADDTOOFFSET(+9),                // skip addText-calling code
	0x33, 0x10,                         // jmp [to ret]
	0x35, 0x01,                         // ldi 01
	0x65, 0x2e,                         // aTop repressText
	0x33, 0x0a,                         // jmp [to ret]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport3[] = {
	0x31, 0x68,                         // bnt (adjust branch to reuse audio mode addText-calling code)
	PATCH_ADDTOOFFSET(+85),             // (right at the MAGIC_DWORD)
	// check, if text is supposed to be shown. If yes, skip the follow-up check (param[1])
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x12,                               // and
	0x2f, 0x07,                         // bt [skip over param check]
	// original code, checks param[1]
	0x8f, 0x01,                         // lsp param[1]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x10,                         // bnt [code to set property repressText to 1], adjusted
	// waste 5 bytes instead of using myDialog class for now
	// setting myDialog class all the time causes game over screens to misbehave (bug #9771)
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	// use myDialog class, so that text box automatically disappears (this is not done for text only mode, like in the original)
	//0x72, 0x0e, 0x00,                   // lofsa myDialog
	//0x65, 0x12,                         // aTop dialog
	// followed by original addText-calling code
	0x38,
	PATCH_GETORIGINALUINT16(+95),       // pushi (addText)
	0x78,                               // push1
	0x8f, 0x02,                         // lsp param[2]
	0x59, 0x03,                         // &rest 03
	0x54, 0x06,                         // self 06
	0x48,                               // ret

	0x35, 0x01,                         // ldi 01
	0x65, 0x2e,                         // aTop repressText
	0x48,                               // ret
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes text-window size for hires portraits mode
//   Otherwise at least at the end some text-windows will be way too small
// Applies to at least: PC-CD
// Patched method: Talker::init
static const uint16 kq6CDSignatureAudioTextSupport4[] = {
	SIG_MAGICDWORD,
	0x63, 0x94,                         // pToa raving
	0x31, 0x0a,                         // bnt [no rave code]
	0x35, 0x00,                         // ldi 00
	SIG_ADDTOOFFSET(+6),                // skip reset of bust, eyes and mouth
	0x33, 0x24,                         // jmp [to super class code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport4[] = {
	PATCH_ADDTOOFFSET(+12),
	0x33, PATCH_GETORIGINALBYTEADJUST(+13, -6), // (adjust jump to also include setSize call)
	PATCH_END
};

//  Fixes text window placement, when dual mode is active (Guards in room 220)
// Applies to at least: PC-CD
// Patched method: tlkGateGuard1::init & tlkGateGuard2::init
static const uint16 kq6CDSignatureAudioTextSupportGuards[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	SIG_END                             // followed by bnt for Guard1 and bt for Guard2
};

static const uint16 kq6CDPatchAudioTextSupportGuards[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x02,                         // ldi 02
	0x1c,                               // ne?
	PATCH_END
};

//  Fixes text window placement, when portrait+text is shown (Stepmother in room 250)
// Applies to at least: PC-CD
// Patched method: tlkStepmother::init
static const uint16 kq6CDSignatureAudioTextSupportStepmother[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportJumpAlways[] = {
	PATCH_ADDTOOFFSET(+4),
	0x1a,                               // eq?
	PATCH_END
};

//  Fixes "Girl In The Tower" to get played in dual mode as well
//  Also changes credits to use CD audio for dual mode.
//
// Applies to at least: PC-CD
// Patched method: rm740::cue (script 740), sCredits::init (script 52)
static const uint16 kq6CDSignatureAudioTextSupportGirlInTheTower[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportGirlInTheTower[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

//  Fixes dual mode for scenes with Azure and Ariel (room 370)
//   Effectively same patch as the one for fixing "Girl In The Tower"
// Applies to at least: PC-CD
// Patched methods: rm370::init, caughtAtGateCD::changeState, caughtAtGateTXT::changeState, toLabyrinth::changeState
// Fixes bug: #6750
static const uint16 kq6CDSignatureAudioTextSupportAzureAriel[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportAzureAriel[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

// Fixes Alexander's yell in dual mode when the Minotaur knocks him into the
//  firey pit in room 440.
//
// Applies to: PC-CD
// Patched method: hornSwaggled:changeState
static const uint16 kq6CDSignatureAudioTextSupportMinotaurLair[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	SIG_ADDTOOFFSET(+41),
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1c,                               // ne?
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportMinotaurLair[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_ADDTOOFFSET(+43),
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Adds another button state for the text/audio button. We currently use the "speech" view for "dual" mode.
// View 947, loop 9, cel 0+1 -> "text"
// View 947, loop 8, cel 0+1 -> "speech"
// View 947, loop 12, cel 0+1 -> "dual" (this view is injected by us into the game)
// Applies to at least: PC-CD
// Patched method: iconTextSwitch::show, iconTextSwitch::doit
static const uint16 kq6CDSignatureAudioTextMenuSupport[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt [set text view]
	0x35, 0x08,                         // ldi 08
	0x65, 0x14,                         // aTop loop
	0x33, 0x04,                         // jmp [skip over text view]
	0x35, 0x09,                         // ldi 09
	0x65, 0x14,                         // aTop loop
	SIG_ADDTOOFFSET(+102),              // skip to iconTextSwitch::doit code
	0x89, 0x5a,                         // lsg global[5a]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt [set text mode]
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	0x33, 0x0a,                         // jmp [skip over text mode code]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [skip over text ode code]
	0x35, 0x01,                         // ldi 01
	0xa1, 0x5a,                         // sag global[5a]
	0x3a,                               // toss
	0x67, 0x14,                         // pTos loop
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [set text view]
	0x35, 0x08,                         // ldi 08
	0x33, 0x02,                         // jmp [skip text view]
	0x35, 0x09,                         // ldi 09
	0x65, 0x14,                         // aTop loop
	SIG_END
};

static const uint16 kq6CDPatchAudioTextMenuSupport[] = {
	PATCH_ADDTOOFFSET(+13),
	0x33, 0x79,                         // jmp [to new text+dual code]
	PATCH_ADDTOOFFSET(+104),            // seek to iconTextSwitch::doit
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x02,                               // add
	0xa1, 0x5a,                         // sag global[5a]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x1e,                               // gt?
	0x31, 0x03,                         // bnt [skip over]
	0x78,                               // push1
	0xa9, 0x5a,                         // ssg global[5a]
	0x33, 0x17,                         // jmp [iconTextSwitch::show call]
	// additional code for iconTextSwitch::show
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [dual mode]
	0x35, 0x09,                         // ldi 09
	0x33, 0x02,                         // jmp [skip over dual mode]
	0x35, 0x0c,                         // ldi 0c (view 947, loop 12, cel 0+1 is our "dual" view, injected by view.cpp)
	0x65, 0x14,                         // aTop loop
	0x32, PATCH_UINT16(0xff75),         // jmp [back to iconTextSwitch::show]
	PATCH_END
};

// When caught by guard dogs in the castle, sometimes their music doesn't stop.
//  Sound 710 continues playing in the dungeon and afterwards, drowning out the
//  real room music. This script bug also occurs in the original. It's a
//  regression in the CD version and subsequent localized floppy versions.
//
// When changing rooms, CastleRoom:newRoom fades out sound 710 if it's already
//  playing, which it detects by testing if globalSound2:prevSignal != -1.
//  This worked in the original floppy versions but the CD version introduced a
//  newer Sound class with different behavior. prevSignal is no longer reset to
//  0 by every Sound:play. This prevents CastleRoom:newRoom from detecting and
//  stopping the music when globalSound2:prevSignal is -1 from an earlier sound.
//
// We fix this by testing globalSound2:handle instead of prevSignal. handle is
//  always set while a sound is being played and always cleared afterwards. This
//  is the same bug as in LB2CD's Act 5 finale music, and the same fix.
//
// Applies to: PC CD, Italian PC Floppy, Spanish PC Floppy
// Responsible method: CastleRoom:newRoom
// Fixes bug: #11746
static const uint16 kq6SignatureGuardDogMusic[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00ac),           // pushi prevSignal [ hard-coded for affected versions ]
	0x76,                               // push0
	0x81, 0x67,                         // lag 67  [ globalSound2 ]
	0x4a, 0x04,                         // send 04 [ globalSound2 prevSignal? ]
	0x36,                               // push
	0x35, 0xff,                         // ldi ff
	SIG_END
};

static const uint16 kq6PatchGuardDogMusic[] = {
	0x38, PATCH_SELECTOR16(handle),     // pushi handle
	PATCH_ADDTOOFFSET(+6),
	0x35, 0x00,                         // ldi 00
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq6Signatures[] = {
	{  true,    52, "CD: Girl In The Tower playback",                 1, kq6CDSignatureGirlInTheTowerPlayback,     kq6CDPatchGirlInTheTowerPlayback },
	{  true,    80, "fix guard dog music",                            1, kq6SignatureGuardDogMusic,                kq6PatchGuardDogMusic },
	{  true,    87, "fix Drink Me bottle",                            1, kq6SignatureDrinkMeFix,                   kq6PatchDrinkMeFix },
	{ false,    87, "Mac: Drink Me pic",                              1, kq6SignatureMacDrinkMePic,                kq6PatchMacDrinkMePic },
	{  true,   281, "fix pawnshop genie eye",                         1, kq6SignaturePawnshopGenieEye,             kq6PatchPawnshopGenieEye },
	{  true,   300, "fix floating off steps",                         2, kq6SignatureCliffStepFloatFix,            kq6PatchCliffStepFloatFix },
	{  true,   300, "fix floating off steps",                         2, kq6SignatureCliffItemFloatFix,            kq6PatchCliffItemFloatFix },
	{  true,   405, "fix catacombs room message",                     1, kq6SignatureRoom405LookMessage,           kq6PatchRoom405LookMessage },
	{  true,   406, "fix catacombs dark room inventory",              1, kq6SignatureDarkRoomInventory,            kq6PatchDarkRoomInventory },
	{  true,   407, "fix catacombs room message",                     1, kq6SignatureRoom407LookMessage,           kq6PatchRoom407LookMessage },
	{  true,   480, "CD: fix wallflower dance",                       1, kq6CDSignatureWallFlowerDanceFix,         kq6CDPatchWallFlowerDanceFix },
	{  true,   480, "fix getting baby tears",                         1, kq6SignatureGetBabyTears,                 kq6PatchGetBabyTears },
	{  true,   481, "fix duplicate baby cry",                         1, kq6SignatureDuplicateBabyCry,             kq6PatchDuplicateBabyCry },
	{  true,   481, "fix duplicate baby tears point",                 1, kq6SignatureDuplicateBabyTearsPoint,      kq6PatchDuplicateBabyTearsPoint },
	{  true,   640, "fix 'Tickets, only' message",                    1, kq6SignatureTicketsOnly,                  kq6PatchTicketsOnly },
	{  true,   745, "fix wedding genie lamp message",                 1, kq6SignatureWeddingGenieLampMessage,      kq6PatchWeddingGenieLampMessage },
	{  true,   800, "fix Cassima secret passage peephole",            1, kq6SignatureCassimaSecretPassage,         kq6PatchCassimaSecretPassage },
	{  true,   907, "fix inventory stack leak",                       1, kq6SignatureInventoryStackFix,            kq6PatchInventoryStackFix },
	{  true,   907, "fix hair detection for ribbon's look msg",       1, kq6SignatureLookRibbonFix,                kq6PatchLookRibbonFix },
	{  true,   907, "talking inventory workaround",                   4, kq6SignatureTalkingInventory,             kq6PatchTalkingInventory },
	{  true,   924, "CD/Mac: fix truncated messages",                 1, kq6SignatureTruncatedMessagesFix,         kq6PatchTruncatedMessagesFix },
	{  true,   928, "Narrator lockup fix",                            1, sciNarratorLockupSignature,               sciNarratorLockupPatch },
	// King's Quest 6 and Laura Bow 2 share basic patches for audio + text support
	// *** King's Quest 6 audio + text support ***
	{ false,   924, "CD: audio + text support KQ6&LB2 1",             1, kq6laurabow2CDSignatureAudioTextSupport1,     kq6laurabow2CDPatchAudioTextSupport1 },
	{ false,   924, "CD: audio + text support KQ6&LB2 2",             1, kq6laurabow2CDSignatureAudioTextSupport2,     kq6laurabow2CDPatchAudioTextSupport2 },
	{ false,   924, "CD: audio + text support KQ6&LB2 3",             1, kq6laurabow2CDSignatureAudioTextSupport3,     kq6laurabow2CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support KQ6&LB2 4",             1, kq6laurabow2CDSignatureAudioTextSupport4,     kq6laurabow2CDPatchAudioTextSupport4 },
	{ false,   928, "CD: audio + text support KQ6&LB2 5",             2, kq6laurabow2CDSignatureAudioTextSupport5,     kq6laurabow2CDPatchAudioTextSupport5 },
	{ false,   909, "CD: audio + text support KQ6 1",                 2, kq6CDSignatureAudioTextSupport1,              kq6CDPatchAudioTextSupport1 },
	{ false,   928, "CD: audio + text support KQ6 2",                 1, kq6CDSignatureAudioTextSupport2,              kq6CDPatchAudioTextSupport2 },
	{ false,   104, "CD: audio + text support KQ6 3",                 1, kq6CDSignatureAudioTextSupport3,              kq6CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support KQ6 4",                 1, kq6CDSignatureAudioTextSupport4,              kq6CDPatchAudioTextSupport4 },
	{ false,  1009, "CD: audio + text support KQ6 Guards",            2, kq6CDSignatureAudioTextSupportGuards,         kq6CDPatchAudioTextSupportGuards },
	{ false,  1027, "CD: audio + text support KQ6 Stepmother",        1, kq6CDSignatureAudioTextSupportStepmother,     kq6CDPatchAudioTextSupportJumpAlways },
	{ false,    52, "CD: audio + text support KQ6 Girl In The Tower", 1, kq6CDSignatureAudioTextSupportGirlInTheTower, kq6CDPatchAudioTextSupportGirlInTheTower },
	{ false,   740, "CD: audio + text support KQ6 Girl In The Tower", 1, kq6CDSignatureAudioTextSupportGirlInTheTower, kq6CDPatchAudioTextSupportGirlInTheTower },
	{ false,   370, "CD: audio + text support KQ6 Azure & Ariel",     6, kq6CDSignatureAudioTextSupportAzureAriel,     kq6CDPatchAudioTextSupportAzureAriel },
	{ false,   441, "CD: audio + text support KQ6 Minotaur lair",     1, kq6CDSignatureAudioTextSupportMinotaurLair,   kq6CDPatchAudioTextSupportMinotaurLair },
	{ false,   903, "CD: audio + text support KQ6 menu",              1, kq6CDSignatureAudioTextMenuSupport,           kq6CDPatchAudioTextMenuSupport },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Kings Quest 7

// KQ7's subtitles were left unfinished in the shipped game, so there are
// several problems when enabling them from the ScummVM launcher:
//
// 1. `kqMessager::findTalker` tries to determine which class to use for
//    displaying subtitles using the talker number of each message, but the
//    talker data is often bogus (e.g. princess messages normally use talker 7,
//    but talker 99 (which is for the narrator) is used for her messages at the
//    start of chapter 2), so it can't be used.
// 2. Some display classes render subtitles at the top or middle of the game
//    area, blocking the view of what is on the screen.
// 3. In some areas, the colors of the subtitles are changed arbitrarily (e.g.
//    pink/purple at the start of chapter 2).
//
// To work around these problems, we always use KQTalker, force the text area to
// the bottom of the game area, and force it to always use black & white, which
// are guaranteed to not be changed by game scripts.
//
// We make 2 changes to KQNarrator::init and one to Narrator::say.
//
// Applies to at least: PC CD 1.4 English, 1.51 English, 1.51 German, 2.00 English
static const uint16 kq7SubtitleFixSignature1[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(fore), // pushi fore ($25)
	0x78,                      // push1
	0x39, 0x06,                // pushi 6 - sets fore to 6
	0x39, SIG_SELECTOR8(back), // pushi back ($26)
	0x78,                      // push1
	0x78,                      // push1 - sets back to 1
	0x39, SIG_SELECTOR8(font), // pushi font ($2a)
	0x78,                      // push1
	0x89, 0x16,                // lsg global[$16] - sets font to global[$16]
	0x7a,                      // push2 (y)
	0x78,                      // push1
	0x76,                      // push0 - sets y to 0
	0x54, SIG_UINT16(0x0018),  // self $18
	SIG_END
};

static const uint16 kq7SubtitleFixPatch1[] = {
	0x33, 0x12, // jmp [skip special init code]
	PATCH_END
};

// Applies to at least: PC CD 1.51 English, 1.51 German, 2.00 English
static const uint16 kq7SubtitleFixSignature2[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[$5a]
	0x35, 0x02,                         // ldi 2
	0x12,                               // and
	0x31, 0x1e,                         // bnt [skip audio volume code]
	0x38, SIG_SELECTOR16(masterVolume), // pushi masterVolume (0212h for 2.00, 0219h for 1.51)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1]
	0x4a, SIG_UINT16(0x0004),           // send 4
	0x65, 0x32,                         // aTop curVolume
	0x38, SIG_SELECTOR16(masterVolume), // pushi masterVolume (0212h for 2.00, 0219h for 1.51)
	0x78,                               // push1
	0x67, 0x32,                         // pTos curVolume
	0x35, 0x02,                         // ldi 2
	0x06,                               // mul
	0x36,                               // push
	0x35, 0x03,                         // ldi 3
	0x08,                               // div
	0x36,                               // push
	0x81, 0x01,                         // lag global[1]
	0x4a, SIG_UINT16(0x0006),           // send 6
	// end of volume code
	0x35, 0x01,                         // ldi 1
	0x65, 0x28,                         // aTop initialized
	SIG_END
};

static const uint16 kq7SubtitleFixPatch2[] = {
	PATCH_ADDTOOFFSET(+5),    // skip to bnt
	0x31, 0x1b,               // bnt [skip audio volume code]
	PATCH_ADDTOOFFSET(+15),   // right after "aTop curVolume / pushi masterVolume / push1"
	0x7a,                     // push2
	0x06,                     // mul (saves 3 bytes in total)
	0x36,                     // push
	0x35, 0x03,               // ldi 3
	0x08,                     // div
	0x36,                     // push
	0x81, 0x01,               // lag global[1]
	0x4a, PATCH_UINT16(0x06), // send 6
	// end of volume code
	0x35, 0x76,               // ldi 118
	0x65, 0x16,               // aTop y
	0x78,                     // push1 (saves 1 byte)
	0x69, 0x28,               // sTop initialized
	PATCH_END
};

// Applies to at least: PC CD 1.51 English, 1.51 German, 2.00 English
static const uint16 kq7SubtitleFixSignature3[] = {
	SIG_MAGICDWORD,
	0x63, 0x28,                 // pToa initialized
	0x18,                       // not
	0x31, 0x07,                 // bnt [skip init code]
	0x38, SIG_SELECTOR16(init), // pushi init ($8e for 2.00, $93 for 1.51)
	0x76,                       // push0
	0x54, SIG_UINT16(0x0004),   // self 4
	// end of init code
	0x8f, 0x00,                 // lsp param[0]
	0x35, 0x01,                 // ldi 1
	0x1e,                       // gt?
	0x31, 0x08,                 // bnt [set acc to 0]
	0x87, 0x02,                 // lap param[2]
	0x31, 0x04,                 // bnt [set acc to 0]
	0x87, 0x02,                 // lap param[2]
	0x33, 0x02,                 // jmp [over set acc to 0 code]
	0x35, 0x00,                 // ldi 00
	0x65, 0x18,                 // aTop caller
	SIG_END
};

static const uint16 kq7SubtitleFixPatch3[] = {
	PATCH_ADDTOOFFSET(+2),              // skip over "pToa initialized code"
	0x2f, 0x0c,                         // bt [skip init code] - saved 1 byte
	0x38, PATCH_GETORIGINALUINT16(+6),  // pushi init
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),         // self 4
	// additionally set background color here (5 bytes)
	0x34, PATCH_UINT16(0x00ff),         // ldi 255
	0x65, 0x2e,                         // aTop back
	// end of init code
	0x8f, 0x00,                         // lsp param[0]
	0x35, 0x01,                         // ldi 1 (this may get optimized to get another byte)
	0x1e,                               // gt?
	0x31, 0x04,                         // bnt [set acc to 0]
	0x87, 0x02,                         // lap param[2]
	0x2f, 0x02,                         // bt [over set acc to 0 code]
	PATCH_END
};

// KQ7 has custom video benchmarking code that needs to be disabled in a subroutine
// that is called by KQ7CD::init; see sci2BenchmarkSignature
static const uint16 kq7BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class Actor
	0x4a, SIG_UINT16(0x0004),  // send 4
	0xa5, 0x00,                // sat temp[0]
	0x39, SIG_SELECTOR8(view), // pushi view ($e)
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0xfdd4),  // pushi 64980
	SIG_END
};

static const uint16 kq7BenchmarkPatch[] = {
	0x34, PATCH_UINT16(10000), // ldi 10000
	0x48,                      // ret
	PATCH_END
};

// When attempting to use an inventory item on an object that does not interact
// with that item, the game briefly displays an X cursor. It does this by
// spinning for 90000 cycles, which makes the duration dependent on CPU speed,
// maxes out the CPU for no reason, and keeps the engine from polling for
// events (which may make the window appear nonresponsive to the OS).
//
// We replace the loop with a call to kWait().
//
// Applies to at least: KQ7 English 2.00b
// Responsible method: KQ7CD::pragmaFail in script 0
static const uint16 kq7PragmaFailSpinSignature[] = {
	0x35, 0x00,               // ldi 0
	0xa5, 0x02,               // sat temp[2]
	SIG_MAGICDWORD,
	0x8d, 0x02,               // lst temp[2]
	0x35, 0x03,               // ldi 3
	0x22,                     // lt?
	SIG_END
};

static const uint16 kq7PragmaFailSpinPatch[] = {
	0x78,                                     // push1
	0x39, 0x12,                               // pushi 18 (~300ms)
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x33, 0x16,                               // jmp [to setCursor]
	PATCH_END
};

// Room 6100 creates an extra ambrosia, usually floating in upper left of the
//  screen, due to an off by one error. The script's local arrays contain four
//  ambrosia coordinates but the loop that accesses them iterates five times.
//
// Applies to: All versions
// Responsible method: rm6100:init
// Fixes bug #9790
static const uint16 kq7ExtraAmbrosiaSignature[] = {
	SIG_MAGICDWORD,
	0x8d, 0x00,                 // lst 00
	0x35, 0x04,                 // ldi 04
	0x24,                       // le?
	SIG_END
};

static const uint16 kq7ExtraAmbrosiaPatch[] = {
	PATCH_ADDTOOFFSET(+4),
	0x22,                       // lt?
	PATCH_END
};

// In KQ7 1.4, after giving the statue to the snake oil salesman, the curtain is
//  drawn on top of ego when walking in front of the wagon. The script doesn't
//  dispose of the salesman and this leaves his final cel stuck on the screen.
//  We add the missing call to snakeSalesman:dispose.
//
// Applies to: English PC 1.4
// Responsible method: giveStatue:changeState
// Fixes bug: #10221
static const uint16 kq7SnakeOilSalesmanSignature[] = {
	0x38, SIG_SELECTOR16(setHeading),   // pushi setHeading
	SIG_ADDTOOFFSET(+0x281),
	0x72, SIG_UINT16(0x15b4),           // lofsa snakeSalesman
	SIG_ADDTOOFFSET(+0x3f),
	0x3c,                               // dup
	0x35, SIG_MAGICDWORD, 0x0c,         // ldi 0c
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0010),           // bnt 0010 [ state 13 ]
	0x38, SIG_SELECTOR16(setHeading),   // pushi setHeading
	0x7a,                               // pushi2
	0x38, SIG_UINT16(0x00b4),           // pushi 00b4
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0008),           // send 08  [ KQEgo setHeading: 180 self ]
	0x32, SIG_UINT16(0x0017),           // jmp 0017 [ end of method ]
	SIG_END
};

static const uint16 kq7SnakeOilSalesmanPatch[] = {
	PATCH_ADDTOOFFSET(+0x02cd),
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x76,                               // push0
	0x72, PATCH_UINT16(0x15b4),         // lofsa snakeSalesman
	0x4a, PATCH_UINT16(0x0004),         // send 04  [ snakeSalesman: dispose ]
	0x32, PATCH_UINT16(0xfd26),         // jmp fd26 [ KQEgo setHeading and end of method ] 
	PATCH_END
};

// In KQ7 1.65c, when Chicken Petite appears in room 5300 after leaving the
//  china shop, the script sends a message to a non-object. Several script and
//  export numbers changed from the previous version, but chickenCartoon2 wasn't
//  updated to call ScriptID with the new values. Instead, the stale values
//  return a procedure address instead of the chickenTimerScript object.
//
// We fix this by using chickenTimerScript's correct script and export numbers.
//
// Applies to: English PC 1.65c, French PC 1.65c, probably German 1.65c
// Responsible method: chickenCartoon2:changeState(9)
// Fixes bug: #11575
static const uint16 kq7ChickenCartoonSignature[] = {
	0x38, SIG_MAGICDWORD, SIG_UINT16(0x14b4), // pushi 14b4
	0x39, 0x03,                               // pushi 03
	0x43, 0x02, SIG_UINT16(0x0004),           // callk ScriptID [ ScriptID 5300 3 ]
	SIG_ADDTOOFFSET(+0x037b),
	// this part of the signature matches 1.65c while excluding earlier versions
	0x38, SIG_UINT16(0x14b5),                 // pushi 14b5
	0x39, 0x03,                               // pushi 03
	0x43, 0x02, SIG_UINT16(0x0004),           // callk ScriptID [ ScriptID 5301 3 ]
	SIG_END
};

static const uint16 kq7ChickenCartoonPatch[] = {
	0x38, PATCH_UINT16(0x14b6),               // pushi 14b6 [ script 5302 ]
	0x39, 0x16,                               // pushi 16   [ export 22   ]
	PATCH_END
};

// KQ7 allows a maximum of 10 save games but English version 2.00 introduced a
//  script bug which removed the check that enforces this. We add the missing
//  check so that the game doesn't break. Sierra later released English version
//  2.00b whose only change was to fix this.
//
// Applies to: English PC 2.00
// Responsible method: startBut:doVerb
static const uint16 kq7TooManySavesSignature[] = {
	0x47, 0x15, 0x00, SIG_UINT16(0x0000),   // calle proc21_0 [ get save count ]
	0xa5, 0x00,                             // sat 00 [ unused ]
	SIG_MAGICDWORD,
	0x35, 0x00,                             // ldi 00
	0x31, 0x16,                             // bnt 16 [ allow save ]
	SIG_END
};

static const uint16 kq7TooManySavesPatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x36,                                   // push
	0x35, 0x0a,                             // ldi 0a
	0x20,                                   // ge? [ save count >= 10 ]
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq7Signatures[] = {
	{  true,     0, "disable video benchmarking",                  1, kq7BenchmarkSignature,                    kq7BenchmarkPatch },
	{  true,     0, "remove hardcoded spin loop",                  1, kq7PragmaFailSpinSignature,               kq7PragmaFailSpinPatch },
	{  true,    30, "fix allowing too many saves",                 1, kq7TooManySavesSignature,                 kq7TooManySavesPatch },
	{  true,  5300, "fix snake oil salesman disposal",             1, kq7SnakeOilSalesmanSignature,             kq7SnakeOilSalesmanPatch },
	{  true,  5301, "fix chicken cartoon",                         1, kq7ChickenCartoonSignature,               kq7ChickenCartoonPatch },
	{  true,  6100, "fix extra ambrosia",                          1, kq7ExtraAmbrosiaSignature,                kq7ExtraAmbrosiaPatch },
	{  true,    31, "enable subtitles (1/3)",                      1, kq7SubtitleFixSignature1,                 kq7SubtitleFixPatch1 },
	{  true, 64928, "enable subtitles (2/3)",                      1, kq7SubtitleFixSignature2,                 kq7SubtitleFixPatch2 },
	{  true, 64928, "enable subtitles (3/3)",                      1, kq7SubtitleFixSignature3,                 kq7SubtitleFixPatch3 },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,               sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Lighthouse

// When going to room 5 (sierra logo & menu room) from room 380 (the credits
// room), the game tries to clear flags from 0 (global[116] bit 0) to 1423
// (global[204] bit 15), but global[201] is not a flag global (it holds a
// reference to theInvisCursor). This patch stops clearing after flag 1359
// (global[200] bit 15). Hopefully that is good enough to not break the game.
// Applies to at least: English 1.0c & 2.0a
static const uint16 lighthouseFlagResetSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x58f), // ldi 1423
	0x24,                    // le?
	SIG_END
};

static const uint16 lighthouseFlagResetPatch[] = {
	0x34, PATCH_UINT16(0x54f), // ldi 1359
	PATCH_END
};

// When doing a system check on the portal computer in the lighthouse, the game
// counts up to 1024MB, one megabyte at a time. In SSCI, this count speed would
// be video speed dependent, but with our frame rate throttler, it takes 17
// seconds. So, replace this slowness with a much faster POST that is more
// accurate to the original game.
// Applies to at least: US English 1.0c
static const uint16 lighthouseMemoryCountSignature[] = {
	SIG_MAGICDWORD,
	0x8d, 0x02,             // lst temp[2]
	0x35, 0x0a,             // ldi 10
	0x24,                   // le?
	0x31, 0x3b,             // bnt [to second digit overflow]
	SIG_ADDTOOFFSET(+4),    // ldi, sat
	0x8d, 0x03,             // lst temp[3]
	0x35, 0x0a,             // ldi 10
	SIG_END
};

static const uint16 lighthouseMemoryCountPatch[] = {
	PATCH_ADDTOOFFSET(+2), // lst temp[2]
	0x35, 0x02,            // ldi 2
	PATCH_ADDTOOFFSET(+9), // le?, bnt, ldi, sat, lst
	0x35, 0x02,            // ldi 2
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry lighthouseSignatures[] = {
	{  true,     5, "fix bad globals clear after credits",         1, lighthouseFlagResetSignature,     lighthouseFlagResetPatch },
	{  true,   360, "fix slow computer memory counter",            1, lighthouseMemoryCountSignature,   lighthouseMemoryCountPatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,       sciNarratorLockupPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// When Robin hands out the scroll to Marion and then types his name using the
// hand code, the German version's script contains a typo (likely a copy/paste
// error), and the local procedure that shows each letter is called twice. The
// The procedure expects a letter arg and returns no value, so the first call
// takes its letter and feeds an undefined value to the second call. Thus the
// kStrCat() within the procedure reads a random pointer and crashes.
//
// We patch all of the 5 doubled local calls (one for each letter typed from
// "R", "O", "B", "I", "N") to be the same as the English version.
// Applies to at least: German floppy
// Responsible method: giveScroll::changeState(19,21,23,25,27) in script 210
// Fixes bug: #5264
static const uint16 longbowSignatureShowHandCode[] = {
	0x78,                            // push1 (1 call arg)
                                     //
	0x78,                            // push1 (1 call arg)
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa (letter that was typed)
	0x36,                            // push
	0x40, SIG_ADDTOOFFSET(+2), 0x02, // call [localproc], 02
                                     //
	0x36,                            // push (the result is an arg for the next call)
	0x40, SIG_ADDTOOFFSET(+2), SIG_MAGICDWORD, 0x02, // call [localproc], 02
                                     //
	0x38, SIG_SELECTOR16(setMotion), // pushi setMotion (0x11c in Longbow German)
	0x39, SIG_SELECTOR8(x),          // pushi x (0x04 in Longbow German)
	0x51, 0x1e,                      // class MoveTo
	SIG_END
};

static const uint16 longbowPatchShowHandCode[] = {
	0x39, 0x01,                      // pushi 1 (combine the two push1's in one, like in the English version)
	PATCH_ADDTOOFFSET(+3),           // leave the lofsa untouched
	// The following will remove the first push & call
	0x32, PATCH_UINT16(0x0002),      // jmp 02 [to the second push & call]
	0x35, 0x00,                      // ldi 0 (waste 2 bytes)
	PATCH_END
};

// When walking through the forest, arithmetic errors may occur at "random".
// The scripts try to add a value and a pointer to the object "berryBush".
//
// This is caused by a local variable overflow.
//
// The scripts create berry bush objects dynamically. The array storage for
// those bushes may hold a total of 8 bushes. But sometimes 10 bushes
// are created. This overwrites 2 additional locals in script 225 and
// those locals are used normally for value lookups.
//
// Changing the total of bushes could cause all sorts of other issues,
// that's why I rather patched the code, that uses the locals for a lookup.
// Which means it doesn't matter anymore when those locals are overwritten.
//
// Applies to at least: English PC floppy, German PC floppy, English Amiga floppy
// Responsible method: export 2 of script 225
// Fixes bug: #6751
static const uint16 longbowSignatureBerryBushFix[] = {
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x03,                      // ldi 03h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x002d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x04,                      // ldi 04h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0025),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x05,                      // ldi 05h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x001d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x06,                      // ldi 06h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0015),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x18,                      // ldi 18h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x000d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x19,                      // ldi 19h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0005),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x1a,                      // ldi 1Ah
	0x1a,                            // eq?
	// jump location for the "bt" instructions
	0x30, SIG_UINT16(0x0011),        // bnt [skip over follow up code, to offset 0c35]
	// 55 bytes until here
	0x85, 0x00,                      // lat temp[0]
	SIG_MAGICDWORD,
	0x9a, SIG_UINT16(0x0110),        // lsli local[110h] -> 110h points normally to 110h / 2Bh
	// 5 bytes
	0x7a,                            // push2
	SIG_END
};

static const uint16 longbowPatchBerryBushFix[] = {
	PATCH_ADDTOOFFSET(+4),           // keep: lsg global[70h], ldi 03h
	0x22,                            // lt? (global < 03h)
	0x2f, 0x42,                      // bt [skip over all the code directly]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x06,                      // ldi 06h
	0x24,                            // le? (global <= 06h)
	0x2f, 0x0e,                      // bt [to kRandom code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x18,                      // ldi 18h
	0x22,                            // lt? (global < 18h)
	0x2f, 0x34,                      // bt [skip over all the code directly]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x1a,                      // ldi 1Ah
	0x24,                            // le? (global <= 1Ah)
	0x31, 0x2d,                      // bnt [skip over all the code directly]
	// 28 bytes, 27 bytes saved
	// kRandom code
	0x85, 0x00,                      // lat temp[0]
	0x2f, 0x05,                      // bt [skip over case 0]
	// temp[0] == 0
	0x38, PATCH_UINT16(0x0110),      // pushi 0110h - that's what's normally at local[110h]
	0x33, 0x18,                      // jmp [kRandom call]
	// check temp[0] further
	0x78,                            // push1
	0x1a,                            // eq?
	0x31, 0x05,                      // bnt [skip over case 1]
	// temp[0] == 1
	0x38, PATCH_UINT16(0x002b),      // pushi 002Bh - that's what's normally at local[111h]
	0x33, 0x0f,                      // jmp [kRandom call]
	// temp[0] >= 2
	0x8d, 0x00,                      // lst temp[0]
	0x35, 0x02,                      // ldi 02
	0x04,                            // sub
	0x9a, PATCH_UINT16(0x0112),      // lsli local[112h] -> look up value in 2nd table
	                                 // this may not be needed at all and was just added for safety reasons
	// waste 9 spare bytes
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	PATCH_END
};

// The camp (room 150) has a bug that can prevent the outlaws from ever rescuing
//  the boys at sunset on day 5 or 6. The rescue occurs when entering camp as an
//  abbey monk after leaving town exactly 3 times but this assumes that the
//  counter can't exceed this. Wearing a different disguise can increment the
//  counter beyond 3 at which point sunset can never occur.
//
// We fix this by patching the counter tests to greater than or equals. This
//  makes them consistent with the other scripts that test this global variable.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible method: local procedure #3 in script 150
// Fixes bug: #10839
static const uint16 longbowSignatureCampSunsetFix[] = {
	SIG_MAGICDWORD,
	0x89, 0x8e,                     // lsg global[8e] [ times left town ]
	0x35, 0x03,                     // ldi 03
	0x1a,                           // eq?
	SIG_END
};

static const uint16 longbowPatchCampSunsetFix[] = {
	PATCH_ADDTOOFFSET(+4),
	0x20,                        	// ge?
	PATCH_END
};

// The town map (room 260) has a bug that can send Robin to the wrong room.
//  Loading the map from town on day 5 or 6 automatically sends Robin to camp
//  (room 150) after leaving town more than twice. The intent is to start the
//  sunset scene where the outlaws rescue the boys, but the map doesn't test the
//  correct sunset conditions and can load an empty camp, even on the wrong day.
//
// We fix this by changing the map's logic to match the camp's by requiring the
//  abbey monk disguise to be worn and the rescue flag to not be set.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible method: rm260:init
// Fixes bug: #10839
static const uint16 longbowSignatureTownMapSunsetFix[] = {
	SIG_MAGICDWORD,
	0x39, 0x05,                     // pushi 05
	0x81, 0x82,                     // lag global[82] [ day ]
	0x24,                           // le?
	0x30, SIG_UINT16(0x0089),       // bnt 0089 [ no sunset if day < 5 ]
	0x60,                           // pprev
	0x35, 0x06,                     // ldi 06
	0x24,                           // le?
	0x30, SIG_UINT16(0x0082),       // bnt 0082 [ no sunset if day > 6 ]
	0x89, 0x8e,                     // lsg global[8e]
	0x35, 0x01,                     // ldi 01
	SIG_END
};

static const uint16 longbowPatchTownMapSunsetFix[] = {
	0x89, 0x7e,                     // lsg global[7e] [ current disguise ]
	0x35, 0x05,                     // ldi 05 [ abbey monk ]
	0x1c,                           // ne?
	0x2f, 0x06,                     // bt 06 [ no sunset if disguise != abbey monk ]
	0x78,                           // push1
	0x39, 0x38,                     // pushi 38
	0x45, 0x05, 0x02,               // callb [export 5 of script 0], 02 [ is rescue flag set? ]
	0x2e, PATCH_UINT16(0x0081),     // bt 0081 [ no sunset if rescue flag is set ]
	0x81, 0x8e,                     // lag global[8e]
	0x78,                           // push1 [ save a byte ]
	PATCH_END
};

// Ending day 5 or 6 by choosing to attack the castle fails to set the rescue
//  flag which tells the next day what to do. This flag is set when rescuing
//  the boys yourself and when the outlaws rescue them at sunset. Without this
//  flag, the sunset rescue can repeat the next day and break the game.
//
// We fix this by setting the flag when returning the boys to their mother in
//  room 250 after the attack.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible method: boysSaved:changeState(0)
// Fixes bug: #10839
static const uint16 longbowSignatureRescueFlagFix[] = {
	0x3c,                           // dup
	0x35, 0x00,                     // ldi 00
	0x1a,                           // eq?
	0x30, SIG_MAGICDWORD,           // bnt 0003 [ state 1 ]
	      SIG_UINT16(0x0003),
	0x32, SIG_UINT16(0x025b),       // jmp 025b [ end of method ]
	SIG_END
};

static const uint16 longbowPatchRescueFlagFix[] = {
	0x2f, 0x08,                     // bt 08 [ state 1 ]
	0x78,                           // push1
	0x39, 0x38,                     // pushi 38
	0x45, 0x06, 0x02,               // callb [export 6 of script 0], 02 [ set rescue flag ]
	0x3a,                           // toss
	0x48,                           // ret
	PATCH_END
};

// On day 7, Tuck can appear at camp to say that the widow wants to see you when
//  she really doesn't. This scene is only supposed to occur if you haven't
//  received the net but the script only tests if the net is currently in
//  inventory, which it isn't if you've already used it or are in disguise.
//
// We fix this by testing the net's owner instead of inventory. If net:owner is
//  non-zero then it's in inventory or in your cave or has been used.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible method: local procedure #3 in script 150
// Fixes bug: #10847
static const uint16 longbowSignatureTuckNetFix[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x03a2),       // bnt 03a2 [ end of method ]
	0x38, SIG_SELECTOR16(has),      // pushi has
	0x78,                           // push1
	0x39, 0x04,                     // pushi 04
	0x81, 0x00,                     // lag global[0]
	0x4a, 0x06,                     // send 6 [ ego: has 4 ]
	0x18,                           // not
	0x30, SIG_UINT16(0x0394),       // bnt 0394 [ end of method if net not in inventory ]
	0x78,                           // push1
	0x39, 0x47,                     // pushi 47
	0x45, 0x05, 0x02,               // callb [export 5 of script 0], 02 [ is flag 47 set? ]
	0x18,                           // not
	0x30, SIG_UINT16(0x038a),       // bnt 038a [ end of method ]
	SIG_ADDTOOFFSET(+60),
	0x32, SIG_UINT16(0x034b),       // jmp 034b [ end of method ]
	SIG_END
};

static const uint16 longbowPatchTuckNetFix[] = {
	0x31, 0x55,                     // bnt 55 [ skip scene, save a byte ]
	0x39, PATCH_SELECTOR8(at),      // pushi at
	0x78,                           // push1
	0x39, 0x04,                     // pushi 04
	0x81, 0x09,                     // lag global[9]
	0x4a, 0x06,                     // send 6 [ Inv: at 4 ]
	0x38, PATCH_SELECTOR16(owner),  // pushi owner
	0x76,                           // push0
	0x4a, 0x04,                     // send 4 [ net: owner? ]
	0x2f, 0x44,                     // bt 44 [ skip scene if net:owner != 0 ]
	0x78,                           // push1
	0x39, 0x47,                     // pushi 47
	0x45, 0x05, 0x02,               // callb [export 5 of script 0], 02 [ is flag 47 set? ]
	0x2f, 0x3c,                     // bt 3c [ skip scene, save 2 bytes ]
	PATCH_END
};

// On day 9, room 350 outside the cobbler's hut is initialized incorrectly if
//  disguised as a monk. The entrance to the hut is broken and several minor
//  messages are incorrect. This is due to the room's script assuming that the
//  only disguises that day are yeoman and merchant. A monk disguise causes some
//  tests to pass and others to fail, leaving the room in an inconsistent state.
//
// We fix this by changing the yeoman disguise tests in the script to include
//  the monk disguises. The disguise global is set to 4 for yeoman and 5 or 6
//  for monk disguises so we patch the tests to be greater than or equals to.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible methods: rm350:init, lobbsHut:doVerb, lobbsDoor:doVerb,
//                      lobbsCover:doVerb, tailorDoor:doVerb
// Fixes bug: #10834
static const uint16 longbowSignatureCobblerHut[] = {
	SIG_MAGICDWORD,
	0x89, 0x7e,                     // lsg global[7e] [ current disguise ]
	0x35, 0x04,                     // ldi 04 [ yeoman ]
	0x1a,                           // eq?    [ is current disguise yeoman? ]
	SIG_END
};

static const uint16 longbowPatchCobblerHut[] = {
	PATCH_ADDTOOFFSET(+4),
	0x20,                           // ge? [ is current disguise yeoman or monk? ]
	PATCH_END
};

// The Amiga version of room 530 adds a broken fDrunk:onMe method which prevents
//  messages when clicking on the drunk on the floor of the pub and causes a
//  signature mismatch on every click in the room. fDrunk:onMe passes an Event
//  object as an integer x coordinate and an uninitialized parameter as a y
//  coordinate to kOnControl. This is a signature mismatch and would cause onMe
//  to return false on every click and prevent hit testing from dispatching
//  events to fDrunk. It's unclear why Sierra added this method to this one
//  Feature in the room. Even if it worked, Feature:onMe already does this.
//
// We fix this by replacing fDrunk:onMe's contents with a call to super:onMe
//  which calls kOnControl correctly and does proper hit testing, making its
//  behavior consistent with the DOS version, which doesn't override onMe.
//
// Applies to: English Amiga Floppy
// Responsible method: fDrunk:onMe
// Fixes bug: #9688
static const uint16 longbowSignatureAmigaPubFix[] = {
	SIG_MAGICDWORD,
	0x67, 0x20,                     // pTos onMeCheck
	0x39, 0x03,                     // pushi 03
	0x39, 0x04,                     // pushi 04
	0x8f, 0x01,                     // lsp param[1]
	0x8f, 0x02,                     // lsp param[2]
	0x43, 0x4e, 0x06,               // callk OnControl, 6
	SIG_END
};

static const uint16 longbowPatchAmigaPubFix[] = {
	0x38, PATCH_UINT16(0x00c4),     // pushi 00c4 [ onMe, hard-coded for amiga ]
	0x76,                           // push0
	0x59, 0x01,                     // &rest 1
	0x57, 0x2c, 0x04,               // super Feature, 4 [ super: onMe &rest ]
	0x48,                           // ret
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm
// When the guards kick Robin out of archery room 320 the game locks up due to
//  pathfinding algorithm differences. Ours sends ego in the wrong direction,
//  colliding with a guard, and preventing the script from continuing.
//
// Applies to: English PC Floppy, German PC Floppy, English Amiga Floppy
// Responsible method: takeHimOut:changeState(1)
// Fixes bug: #10896
static const uint16 longbowSignatureArcherPathfinding[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00c8),       // pushi 00c8 [ y = 200 ]
	0x7c,                           // pushSelf
	0x81, 0x00,                     // lag 00
	0x4a, 0x0c,                     // send 0c [ ego setMotion: PolyPath (ego x?) 200 self ]
	SIG_END
};

static const uint16 longbowPatchArcherPathfinding[] = {
	0x38, PATCH_UINT16(0x00c4),     // pushi 00c4 [ y = 196 ]
	PATCH_END
};

// Longbow 1.0 has two random but common game-breaking bugs: Green Man's riddle
//  scene never ends and the Sheriff's men catch Robin too quickly when sweeping
//  the forest. Both are due to reusing an uninitialized global variable.
//
// Global 137 is used by the abbey hedge maze to store ego's cel during room
//  transitions. Exiting the maze leaves this as a random value between 0 and 5.
//  The forest sweep also uses this global but as a counter it expects to start
//  at 0. It increments as Robin changes rooms during a sweep until it reaches a
//  a maximum and he is caught. This is usually 7 but in some rooms it's only 3.
//  A high initial value can make this sequence impossible. rm180:doit also
//  tests the sweep counter and doesn't allow scripts to respond to a hand code
//  when greater than 2. This breaks the riddle scene after the first answer.
//
// We fix this by clearing global 137 at the start of days 1-7 and 11 so that
//  stale hedge maze values from days 5/6 and 10 don't affect the day 7 riddles
//  or the sweeps on days 9 and 12. Ideally we could just clear this at the
//  start of each day but there's no day initialization script. Instead we add
//  our day-specific code to Robin's cave (room 140), similar to Sierra's patch
//  and later versions.
//
// Applies to: English PC Floppy 1.0
// Responsible method: localproc_001a in script 140
// Fixes bug #5036
static const uint16 longbowSignatureGreenManForestSweepFix[] = {
	0x89, SIG_MAGICDWORD, 0x82,     // lsg 82 [ day ]
	0x35, 0x01,                     // ldi 01
	0x1a,                           // eq?
	0x30, SIG_UINT16(0x0019),       // bnt 0019 [ skip horn init ]
	0x38, SIG_SELECTOR16(has),      // pushi has
	0x78,                           // push1
	0x78,                           // push1
	0x81, 0x00,                     // lag 00
	0x4a, 0x06,                     // send 06 [ ego has: 1 ]
	0x18,                           // not
	0x30, SIG_UINT16(0x000c),       // bnt 000c [ skip horn init ]
	0x39, SIG_SELECTOR8(init),      // pushi init
	0x76,                           // push0
	0x38, SIG_ADDTOOFFSET(+2),      // pushi stopUpd
	0x76,                           // push0
	0x72, SIG_UINT16(0x19b2),       // lofsa horn
	0x4a, 0x08,                     // send 08 [ horn init: stopUpd: ]
	0x89, 0x7e,                     // lsg 7e
	0x35, 0x00,                     // ldi 00
	0x1a,                           // eq?
	0x2e, SIG_UINT16(0005),         // bt 0005
	SIG_ADDTOOFFSET(+19),
	0x39, SIG_SELECTOR8(init),      // push init
	0x76,                           // push0
	0x38, SIG_ADDTOOFFSET(+2),      // pushi stopUpd
	0x76,                           // push0
	0x72, SIG_UINT16(0x1912),       // lofsa bow
	SIG_END
};

static const uint16 longbowPatchGreenManForestSweepFix[] = {
	0x39, 0x07,                     // pushi 07
	0x81, 0x82,                     // lag 82 [ day ]
	0x22,                           // lt?
	0x31, 0x06,                     // bnt 06
	0x60,                           // pprev  [ day ]
	0x35, 0x0b,                     // ldi 0b
	0x1c,                           // ne?
	0x2f, 0x02,                     // bt 02
	0xa1, 0x89,                     // sag 89 [ sweep-count = 0 if day <= 7 or day == 11 ]
	0x81, 0x82,                     // lag 82 [ day ]
	0x78,                           // push1
	0x1a,                           // eq?
	0x31, 0x10,                     // bnt 10 [ skip horn init ]
	0x38, PATCH_SELECTOR16(has),    // pushi has
	0x78,                           // push1
	0x78,                           // push1
	0x81, 0x00,                     // lag 00
	0x4a, 0x06,                     // send 06 [ ego has: 1 ]
	0x2f, 0x05,                     // bt 05 [ skip horn init ]
	0x72, PATCH_UINT16(0x19b2),     // lofsa horn
	0x33, 0x1a,                     // jmp 1c [ continue horn init ]
	0x81, 0x7e,                     // lag 7e
	0x31, 0x08,                     // bnt 08
	PATCH_ADDTOOFFSET(+19),
	0x72, PATCH_UINT16(0x1912),     // lofsa bow
	0x39, PATCH_SELECTOR8(init),    // push init
	0x76,                           // push0
	0x38, PATCH_GETORIGINALUINT16(+25), // pushi stopUpd
	0x76,                           // push0
	PATCH_END
};

// After rescuing Fulk in the Amiga version, rescueOfFulk stores the boat speed
//  in a temporary variable during one state and expects it to still be there in
//  a later state, which only worked by accident in Sierra's interpreter. This
//  Amiga tweak was made so that on slower machines the boat would animate after
//  Fulk and Robin leave the screen. We fix this by using the script's register
//  property for storage instead of a temporary variable.
//
// Applies to: English Amiga Floppy
// Responsible method: rescueOfFulk:changeState
// Fixes bug: #11137
static const uint16 longbowSignatureAmigaFulkRescue[] = {
	SIG_MAGICDWORD,
	0xa5, 0x00,                     // sat 00
	0x89, 0x57,                     // lsg 87
	SIG_ADDTOOFFSET(+10),
	0x8d, 0x00,                     // lst 00
	SIG_ADDTOOFFSET(+635),
	0x8d, 0x00,                     // lst 00
	SIG_END
};

static const uint16 longbowPatchAmigaFulkRescue[] = {
	0x65, 0x1a,                     // aTop register
	PATCH_ADDTOOFFSET(+12),
	0x67, 0x1a,                     // pTos register
	PATCH_ADDTOOFFSET(+635),
	0x67, 0x1a,                     // pTos register
	PATCH_END
};

// The Amiga version has an unusual speed test which takes 10 seconds to run in
//  ScummVM, causing the test to assume a slow machine speed and reduce details
//  throughout the game. We disable the speed test and its long delay before the
//  the Sierra logo so that the fastest machine speed is used.
//
// Applies to: English Amiga Floppy
// Responsible method: speedScript:changeState
static const uint16 longbowSignatureAmigaSpeedTest[] = {
	// state 1
	0x32, SIG_UINT16(0x0164),       // jmp 0164 [ end of method ]
	SIG_ADDTOOFFSET(+0xe9),
	// state 2
	SIG_MAGICDWORD,
	0x35, 0x02,                     // ldi 02   [ fastest machine speed ]
	0x32, SIG_UINT16(0x000f),       // jmp 000f [ set machine speed ]
	SIG_END
};

static const uint16 longbowPatchAmigaSpeedTest[] = {
	0x32, PATCH_UINT16(0x00e9),     // jmp 00e9 [ skip test, use fastest machine speed ]
	PATCH_END
};

// When Robin is sentenced to death, King Richard and Robin discuss Marian's
//  death even if she is alive and just finished testifying at the trial.
//
// At the end of the game, troub:init calculates which ending to use based on
//  the ransom raised, the number of outlaws left, interactions with others, and
//  whether or not Marian lived. These calculations are complex and the worst of
//  the four endings is to be hung. Room 422 handles this ending, but unlike the
//  other trial scripts, it doesn't test any flags. Instead it always displays
//  messages that begin with Marian's death even though this ending is possible
//  when Marian is alive.
//
// We fix this by skipping the messages that discuss Marian's death if she is
//  alive. We make room for this by overwriting redundant code that disables the
//  icon bar. The icon bar is already disabled by the previous room, and even if
//  it weren't, the subsequent call to HandsOff would have disabled it.
//
// Applies to: All versions
// Responsible method: hanging:init
static const uint16 longbowSignatureMarianMessagesFix[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(disable),  // pushi disable
	0x39, 0x08,                     // pushi 08
	0x76,                           // push0
	0x78,                           // push1
	0x7a,                           // push2
	0x39, 0x03,                     // pushi 03
	0x39, 0x04,                     // pushi 04
	0x39, 0x05,                     // pushi 05
	0x39, 0x06,                     // pushi 06
	0x39, 0x07,                     // pushi 07
	0x81, 0x45,                     // lag 45
	0x4a, 0x14,                     // send 14 [ IconBar disable: 1 2 3 4 5 6 7 ]
	SIG_END
};

static const uint16 longbowPatchMarianMessagesFix[] = {
	0x76,                           // push0
	0x39, 0x3e,                     // pushi 3e      [ flag 62 ]
	0x45, 0x05, 0x02,               // callb proc0_5 [ is Marian alive? ]
	0x31, 0x0e,                     // bnt 0e
	0x38, PATCH_UINT16(0x0093),     // pushi 0093
	0xab, 0x0a,                     // ssl 0a [ start message sequence at 147 ]
	0x76,                           // push0
	0xab, 0x0c,                     // ssl 0c [ stop sequence after first message ]
	0x33, 0x04,                     // jmp 04
	PATCH_END
};

//          script, description,                                      signature                                patch
static const SciScriptPatcherEntry longbowSignatures[] = {
	{  true,   140, "green man riddles and forest sweep fix",      1, longbowSignatureGreenManForestSweepFix,  longbowPatchGreenManForestSweepFix },
	{  true,   150, "day 5/6 camp sunset fix",                     2, longbowSignatureCampSunsetFix,           longbowPatchCampSunsetFix },
	{  true,   150, "day 7 tuck net fix",                          1, longbowSignatureTuckNetFix,              longbowPatchTuckNetFix },
	{  true,   210, "hand code crash",                             5, longbowSignatureShowHandCode,            longbowPatchShowHandCode },
	{  true,   225, "arithmetic berry bush fix",                   1, longbowSignatureBerryBushFix,            longbowPatchBerryBushFix },
	{  true,   250, "day 5/6 rescue flag fix",                     1, longbowSignatureRescueFlagFix,           longbowPatchRescueFlagFix },
	{  true,   260, "day 5/6 town map sunset fix",                 1, longbowSignatureTownMapSunsetFix,        longbowPatchTownMapSunsetFix },
	{  true,   320, "day 8 archer pathfinding workaround",         1, longbowSignatureArcherPathfinding,       longbowPatchArcherPathfinding },
	{  true,   350, "day 9 cobbler hut fix",                      10, longbowSignatureCobblerHut,              longbowPatchCobblerHut },
	{  true,   422, "marian messages fix",                         1, longbowSignatureMarianMessagesFix,       longbowPatchMarianMessagesFix },
	{  true,   530, "amiga pub fix",                               1, longbowSignatureAmigaPubFix,             longbowPatchAmigaPubFix },
	{  true,   600, "amiga fulk rescue fix",                       1, longbowSignatureAmigaFulkRescue,         longbowPatchAmigaFulkRescue },
	{  true,   803, "amiga speed test",                            1, longbowSignatureAmigaSpeedTest,          longbowPatchAmigaSpeedTest },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 1 (Spanish)
//
// It seems originally the Spanish version of Larry 1 used some beta code at
// least for the man wearing a barrel, who walks around in front of the casino.
// The script inside the resource files even uses a class, that does not exist
// inside those resource files, which causes a hard error.
// The patch files included with the Spanish version (300.scr,300.tex, 927.scr)
// add this class, but at least inside ScummVM a write to a non-existent selector
// happens right after the player tries to buy an apple from that man.
//
// In the original English releases (2.0+2.1) this was handled differently.
// Which is why this script patch changes that code to work just like in the English release.
//
// Attention: for at least some release of this game, view 302 (man wearing a barrel) is fully
//            broken! Which also causes a crash. The original interpreter crashes as well.
//            The only way to fix this is to dump that view from another release of Larry 1
//            and then use the view patch file on this release.
//
// Applies to at least: Spanish floppy
// Responsible method: sBuyApple::changeScript(2)
// Fixes bug: #10240
static const uint16 larry1SignatureBuyApple[] = {
	// end of state 0
	0x35, 0x01,                      // ldi 01
	0x65, 0x10,                      // aTop cycles
	0x32, SIG_UINT16(0x0248),        // jmp [ret]
	0x3c,                            // dup
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x30, SIG_UINT16(0x0007),        // bnt [step 2 check]
	// state 1 code
	0x35, 0x01,                      // ldi 01
	0x65, 0x10,                      // aTop cycles
	0x32, SIG_UINT16(0x023a),        // jmp [ret]
	0x3c,                            // dup
	0x35, 0x02,                      // ldi 02
	0x1a,                            // eq?
	0x30, SIG_UINT16(0x0036),        // bnt [step 3 check]
	// state 2 code
	0x35, 0x02,                      // ldi 02
	0x38, SIG_UINT16(0x0091),        // pushi setCycle
	0x78,                            // push1
	0x51, 0x18,                      // class Walk
	0x36,                            // push
	0x38, SIG_UINT16(0x0126),        // pushi setAvoider
	0x78,                            // push1
	0x51, SIG_ADDTOOFFSET(+1),       // class PAvoider (original 0x25, w/ patch file 0x6d)
	0x36,                            // push
	0x38, SIG_UINT16(0x0116),        // pushi setMotion
	SIG_MAGICDWORD,
	0x39, 0x04,                      // pushi 04
	0x51, 0x24,                      // class PolyPath
	0x36,                            // push
	0x39, 0x04,                      // pushi 04
	0x76,                            // push0
	0x72, SIG_UINT16(0x0f4e),        // lofsa aAppleMan
	0x4a, 0x04,                      // send 04
	0x36,                            // push
	0x35, 0x1d,                      // ldi 1Dh
	0x02,                            // add
	0x36,                            // push
	0x39, 0x03,                      // pushi 03
	0x76,                            // push0
	0x72, SIG_UINT16(0x0f4e),        // lofsa aAppleMan
	0x4a, 0x04,                      // send 04
	0x36,                            // push
	0x7c,                            // pushSelf
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x18,                      // send 18h
	0x32, SIG_UINT16(0x01fd),        // jmp [ret]
	SIG_END
};

static const uint16 larry1PatchBuyApple[] = {
	PATCH_ADDTOOFFSET(+11),
	0x2f, 0xf3,                        // bt [jump to end of step 1 code], saves 8 bytes
	0x3c,                              // dup
	0x35, 0x02,                        // ldi 02
	0x1a,                              // eq?
	0x31, 0x3f,                        // bnt [step 3 check]
	0x38, PATCH_UINT16(0x00e1),        // pushi distanceTo
	0x78,                              // push1
	0x72, PATCH_UINT16(0x0f4e),        // lofsa sAppleMan
	0x36,                              // push
	0x81, 0x00,                        // lag global[0]
	0x4a, 0x06,                        // send 06
	0x36,                              // push
	0x35, 0x1e,                        // ldi 1Eh
	0x1e,                              // gt?
	0x31, 0xdb,                        // bnt [jump to end of step 1 code]
	0x38, PATCH_SELECTOR16(setCycle),  // pushi setCycle
	0x78,                              // push1
	0x51, 0x18,                        // class Walk
	0x36,                              // push
	0x38, PATCH_SELECTOR16(setMotion), // pushi setMotion
	0x39, 0x04,                        // pushi 04
	0x51, 0x24,                        // class PolyPath
	0x36,                              // push
	0x39, 0x04,                        // pushi 04
	0x76,                              // push0
	0x72, PATCH_UINT16(0x0f4e),        // lofsa aAppleMan
	0x4a, 0x04,                        // send 04
	0x36,                              // push
	0x35, 0x1d,                        // ldi 1Dh
	0x02,                              // add
	0x36,                              // push
	0x39, 0x03,                        // pushi 03
	0x76,                              // push0
	0x72, PATCH_UINT16(0x0f4e),        // lofsa aAppleMan
	0x4a, 0x04,                        // send 04
	0x36,                              // push
	0x7c,                              // pushSelf
	0x81, 0x00,                        // lag global[0]
	0x4a, 0x12,                        // send 12h
	PATCH_END
};

//          script, description,                               signature                patch
static const SciScriptPatcherEntry larry1Signatures[] = {
	{  true,   300, "Spanish: buy apple from barrel man",    1, larry1SignatureBuyApple, larry1PatchBuyApple },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 2
// On the plane, Larry is able to wear the parachute. This grants 4 points.
// In early versions of LSL2, it was possible to get "unlimited" points by
//  simply wearing it multiple times.
// They fixed it in later versions by remembering, if the parachute was already
//  used before.
// But instead of adding it properly, it seems they hacked the script / forgot
//  to replace script 0 as well, which holds information about how many global
//  variables are allocated at the start of the game.
// The script tries to read an out-of-bounds global variable, which somewhat
//  "worked" in SSCI, but ScummVM/SCI doesn't allow that.
// That's why those points weren't granted here at all.
// We patch to use global[5a], which seems to be unused in the whole game.
// Applies to at least: English floppy
// Responsible method: rm63Script::handleEvent
// Fixes bug: #6346
static const uint16 larry2SignatureWearParachutePoints[] = {
	0x35, 0x01,                      // ldi 01
	0xa1, SIG_MAGICDWORD, 0x8e,      // sag global[8e]
	0x80, SIG_UINT16(0x01e0),        // lag global[1e0]
	0x18,                            // not
	0x30, SIG_UINT16(0x000f),        // bnt [don't give points]
	0x35, 0x01,                      // ldi 01
	0xa0, 0xe0, 0x01,                // sag global[1e0]
	SIG_END
};

static const uint16 larry2PatchWearParachutePoints[] = {
	PATCH_ADDTOOFFSET(+4),
	0x80, PATCH_UINT16(0x005a),      // lag global[5a]
	PATCH_ADDTOOFFSET(+6),
	0xa0, PATCH_UINT16(0x005a),      // sag global[5a]
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry larry2Signatures[] = {
	{  true,    63, "plane: no points for wearing parachute",      1, larry2SignatureWearParachutePoints, larry2PatchWearParachutePoints },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 3

// Disable the LSL3 speed test by always setting the machine speed to 40 (PC AT)
//  so that all graphics are enabled and the weight room behaves reasonably.
//  40 is the minimum value that enables everything such as incrementing the
//  score and the lighting effects in rooms 390, 430, and 431. The weight room
//  (room 380) uses the machine speed to calculate how many exercises are
//  required and the results would be much too high (in the thousands) if the
//  speed test were to run unthrottled at modern speeds.
//
// Applies to: All versions
// Responsible method: rm290:doit
// Fixes bug: #11967
static const uint16 larry3SignatureSpeedTest[] = {
	SIG_MAGICDWORD,
	0x8b, 0x00,                      // lsl 00
	0x76,                            // push0
	0x43, SIG_ADDTOOFFSET(+1), 0x00, // callk GetTime 00
	0x22,                            // lt? [ is speed test complete? ]
	0x30,                            // bnt
	SIG_END
};

static const uint16 larry3PatchSpeedTest[] = {
	0x35, 0x28,                      // ldi 28
	0xa1, 0x7b,                      // sag 7b [ machine speed = 40 ]
	0x33, 0x04,                      // jmp 04 [ complete speed test ]
	PATCH_END
};

// The LSL3 volume dialog initialize its slider to the current volume by calling
//  kDoSoundMasterVolume, but it passes an uninitialized variable as an extra
//  parameter. This changes the volume instead of just querying it, leaving the
//  slider out of sync with the abruptly changed volume.
//
// We remove the uninitialized parameter so that this code correctly queries the
//  volume instead of setting it. This was fixed in later versions but the buggy
//  one was used as the basis for SCI Studio's template script, which is also
//  included with SCI Companion, and so this bug lives on in fan games.
//
// Applies to: English PC, English Amiga, English Atari ST
// Responsible method: TheMenuBar:handleEvent
static const uint16 larry3SignatureVolumeSlider[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(doit),       // pushi doit
	0x78,                            // push1
	0x7a,                            // push2
	0x39, 0x08,                      // pushi 08 [ volume ]
	0x8d, 0x01,                      // lst 01   [ uninitialized variable ]
	0x43, 0x31, 0x04,                // callk DoSound 04 [ set volume and return previous ]
	SIG_END
};

static const uint16 larry3PatchVolumeSlider[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x01,                      // pushi 01
	0x38, PATCH_UINT16(0x0008),      // pushi 0008 [ volume ]
	0x43, 0x31, 0x02,                // callk DoSound 02 [ return volume ]
	PATCH_END
};

//          script, description,                                      signature                     patch
static const SciScriptPatcherEntry larry3Signatures[] = {
	{  true,   290, "disable speed test",                          1, larry3SignatureSpeedTest,     larry3PatchSpeedTest },
	{  true,   997, "fix volume slider",                           1, larry3SignatureVolumeSlider,  larry3PatchVolumeSlider },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 5
// In Miami the player can call the green card telephone number and get
//  green card including limo at the same time in the English 1.000 PC release.
// This results later in a broken game in case the player doesn't read
//  the second telephone number for the actual limousine service, because
//  in that case it's impossible for the player to get back to the airport.
//
// We disable the code, that is responsible to make the limo arrive.
//
// This bug was fixed in the European (dual language) versions of the game.
//
// Applies to at least: English PC floppy (1.000)
// Responsible method: sPhone::changeState(40)
static const uint16 larry5SignatureGreenCardLimoBug[] = {
	0x7a,                               // push2
	SIG_MAGICDWORD,
	0x39, 0x07,                         // pushi 07
	0x39, 0x0c,                         // pushi 0Ch
	0x45, 0x0a, 0x04,                   // callb [export 10 of script 0], 04
	0x78,                               // push1
	0x39, 0x26,                         // pushi 26h (limo arrived flag)
	0x45, 0x07, 0x02,                   // callb [export 7 of script 0], 02 (sets flag)
	SIG_END
};

static const uint16 larry5PatchGreenCardLimoBug[] = {
	PATCH_ADDTOOFFSET(+8),
	0x34, PATCH_UINT16(0x0000),         // ldi 0000 (dummy)
	0x34, PATCH_UINT16(0x0000),         // ldi 0000 (dummy)
	PATCH_END
};

// In one of the conversations near the end (to be exact - room 380 and the text
//  about using champagne on Reverse Biaz - only used when you actually did that
//  in the game), the German text is too large, causing the textbox to get too large.
// Because of that the talking head of Patti is drawn over the textbox. A translation oversight.
// Applies to at least: German floppy
// Responsible method: none, position of talker object on screen needs to get modified
static const uint16 larry5SignatureGermanEndingPattiTalker[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x006e),                 // object pattiTalker::x (110)
	SIG_UINT16(0x00b4),                 // object pattiTalker::y (180)
	SIG_ADDTOOFFSET(+469),              // verify that it's really the German version
	0x59, 0x6f, 0x75,                   // (object name) "You"
	0x23, 0x47, 0x44, 0x75,             // "#GDu"
	SIG_END
};

static const uint16 larry5PatchGermanEndingPattiTalker[] = {
	PATCH_UINT16(0x005a),               // object pattiTalker::x (90)
	PATCH_END
};

//          script, description,                                      signature                               patch
static const SciScriptPatcherEntry larry5Signatures[] = {
	{  true,   280, "English-only: fix green card limo bug",       1, larry5SignatureGreenCardLimoBug,        larry5PatchGreenCardLimoBug },
	{  true,   380, "German-only: Enlarge Patti Textbox",          1, larry5SignatureGermanEndingPattiTalker, larry5PatchGermanEndingPattiTalker },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// This is called on every death dialog. Problem is at least the German
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption. This patch
//  moves the title text around, so this overflow doesn't happen anymore. We
//  would otherwise get a crash calling for invalid views (this happens of
//  course also in sierra sci).
// Applies to at least: German PC-CD
// Responsible method: unknown
static const uint16 larry6SignatureDeathDialog[] = {
	SIG_MAGICDWORD,
	0x3e, SIG_UINT16(0x0133),        // link 0133 (offset 0x20)
	0x35, 0xff,                      // ldi ff
	0xa3, 0x00,                      // sal local[0]
	SIG_ADDTOOFFSET(+680),           // ...
	0x8f, 0x01,                      // lsp param[1] (offset 0x2cf)
	0x7a,                            // push2
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea temp[010e]
	0x36,                            // push
	0x43, 0x7c, 0x0e,                // callk Message[7c], 0e
	SIG_ADDTOOFFSET(+90),            // ...
	0x38, SIG_UINT16(0x00d6),        // pushi 00d6 (offset 0x335)
	0x78,                            // push1
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea temp[010e]
	0x36,                            // push
	SIG_ADDTOOFFSET(+76),            // ...
	0x38, SIG_UINT16(0x00cd),        // pushi 00cd (offset 0x38b)
	0x39, 0x03,                      // pushi 03
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea temp[010e]
	0x36,
	SIG_END
};

static const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,                                 // link 0200
	PATCH_ADDTOOFFSET(+687),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea temp[0140]
	PATCH_ADDTOOFFSET(+98),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea temp[0140]
	PATCH_ADDTOOFFSET(+82),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea temp[0140]
	PATCH_END
};

//          script, description,                                      signature                   patch
static const SciScriptPatcherEntry larry6Signatures[] = {
	{  true,    82, "death dialog memory corruption",              1, larry6SignatureDeathDialog, larry6PatchDeathDialog },
	{  true,   928, "Narrator lockup fix",                         1, sciNarratorLockupSignature, sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Leisure Suit Larry 6 Hires

// When entering room 270 (diving board) from room 230, a typo in the game
// script means that `setScale` is called accidentally instead of `setScaler`.
// In SSCI this did not do much because the first argument happened to be
// smaller than the y-position of `ego`, but in ScummVM the first argument is
// larger and so a debug message "y value less than vanishingY" is displayed.
static const uint16 larry6HiresSetScaleSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setScale), // pushi setScale ($14b)
	0x38, SIG_UINT16(0x0005),       // pushi 5
	0x51, 0x2c,                     // class Scaler
	SIG_END
};

static const uint16 larry6HiresSetScalePatch[] = {
	0x38, PATCH_SELECTOR16(setScaler), // pushi setScaler ($14f)
	PATCH_END
};

// The init code that runs when LSL6hires starts up unconditionally resets the
// master music volume to 12 (and the volume dial to 11), but the game should
// always use the volume stored in ScummVM.
// Applies to at least: English CD
// Fixes bug: #9700
static const uint16 larry6HiresVolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x0b,                         // ldi $0b
	0xa1, 0xc2,                         // sag global[$c2]
	SIG_END
};

static const uint16 larry6HiresVolumeResetPatch[] = {
	0x32, PATCH_UINT16(0x0001),         // jmp 1 [past volume change]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry larry6HiresSignatures[] = {
	{  true,    71, "disable volume reset on startup (1/2)",       1, sci2VolumeResetSignature,         sci2VolumeResetPatch },
	{  true,    71, "disable volume reset on startup (2/2)",       1, larry6HiresVolumeResetSignature,  larry6HiresVolumeResetPatch },
	{  true,    71, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true,   270, "fix incorrect setScale call",                 1, larry6HiresSetScaleSignature,     larry6HiresSetScalePatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,       sciNarratorLockupPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Leisure Suit Larry 7

// The init code that runs when LSL7 starts up unconditionally resets the audio
// volumes to defaults, but the game should always use the volume stored in
// ScummVM. This patch is basically identical to the patch for Torin, except
// that they left line numbers in the LSL7 scripts and changed the music volume.
// Applies to at least: English CD
static const uint16 larry7VolumeResetSignature1[] = {
	SIG_MAGICDWORD,
	0x35, 0x41,                         // ldi $41
	0xa1, 0xe3,                         // sag global[$e3] (music volume)
	0x7e, SIG_ADDTOOFFSET(+2),          // (line whatever)
	0x35, 0x3c,                         // ldi $3c
	0xa1, 0xe4,                         // sag global[$e4] (sfx volume)
	0x7e, SIG_ADDTOOFFSET(+2),          // (line whatever)
	0x35, 0x64,                         // ldi $64
	0xa1, 0xe5,                         // sag global[$e5] (speech volume)
	SIG_END
};

static const uint16 larry7VolumeResetPatch1[] = {
	0x33, 0x10,                         // jmp [past volume resets]
	PATCH_END
};

// The init code that runs when LSL7 starts up unconditionally resets the
// audio volumes to values stored in larry7.prf, but the game should always use
// the volume stored in ScummVM. This patch is basically identical to the patch
// for Torin, except that they left line numbers in the LSL7 scripts.
// Applies to at least: English CD
static const uint16 larry7VolumeResetSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(readWord),     // pushi readWord
	0x76,                               // push0
	SIG_ADDTOOFFSET(+6),                // ...
	0xa1, 0xe3,                         // sag global[$e3] (music volume)
	SIG_ADDTOOFFSET(+3),                // (line whatever)
	SIG_ADDTOOFFSET(+10),               // ...
	0xa1, 0xe4,                         // sag global[$e4] (sfx volume)
	SIG_ADDTOOFFSET(+3),                // (line whatever)
	SIG_ADDTOOFFSET(+10),               // ...
	0xa1, 0xe5,                         // sag global[$e5] (speech volume)
	SIG_END
};

static const uint16 larry7VolumeResetPatch2[] = {
	PATCH_ADDTOOFFSET(+10),
	0x18, 0x18,                         // (waste bytes)
	PATCH_ADDTOOFFSET(+3),              // (line whatever)
	PATCH_ADDTOOFFSET(+10),             // ...
	0x18, 0x18,                         // (waste bytes)
	PATCH_ADDTOOFFSET(+3),              // (line whatever)
	PATCH_ADDTOOFFSET(+10),             // ...
	0x18, 0x18,                         // (waste bytes)
	PATCH_END
};

// In room 540 of Leisure Suit Larry 7, when using the cheese maker,
// `soMakeCheese::changeState(6)` incorrectly pushes `self` as the end cel
// instead of a cel number to the End cycler. In SSCI, this bad argument would
// get corrected down to the final cel in the loop by `CycleCueList::init`, but
// because ScummVM currently always sorts numbers higher than objects, the
// comparison fails and the cel number is not corrected, so the cycler never
// calls back and the game softlocks.
// Here, we fix the call so a proper cel number is given for the second argument
// instead of a bogus object pointer.
//
// Applies to at least: English PC-CD, German PC-CD
static const uint16 larry7MakeCheeseCyclerSignature[] = {
	0x38, SIG_UINT16(0x04), // pushi 4
	0x51, 0xc4,             // class End
	0x36,                   // push
	SIG_MAGICDWORD,
	0x7c,                   // pushSelf
	0x39, 0x04,             // pushi 4
	0x7c,                   // pushSelf
	SIG_END
};

static const uint16 larry7MakeCheeseCyclerPatch[] = {
	0x39, 0x04, // pushi 4 - save 1 byte
	0x51, 0xc4, // class End
	0x36,       // push
	0x7c,       // pushSelf
	0x39, 0x04, // pushi 4
	0x39, 0x10, // pushi $10 (last cel of view 54007, loop 0)
	PATCH_END
};

// During the cheese maker cutscene, `soMakeCheese::changeState(2)` sets the
// priority of ego to 500 to draw him over the cheese maker, but this is also
// above the guillotine (view 54000, cel 7, priority 400), so ego gets
// incorrectly drawn on top of the guillotine as well. The cheese maker has a
// priority of 373, so use priority 374 instead of 500.
// Applies to at least: English PC-CD, German PC-CD
// Responsible method: soMakeCheese::changeState(2) in script 540
static const uint16 larry7MakeCheesePrioritySignature[] = {
	0x38, SIG_SELECTOR16(setPri),    // pushi setPri
	SIG_MAGICDWORD,
	0x78,                            // push1
	0x38, SIG_UINT16(0x01f4),        // pushi 500
	SIG_END
};

static const uint16 larry7MakeCheesePriorityPatch[] = {
	PATCH_ADDTOOFFSET(+4),           // pushi setPri, push1
	0x38, PATCH_UINT16(0x0176),      // pushi 374
	PATCH_END
};

// LSL7 tries to reset the message type twice at startup, first with a default
// value in script 0, then with a stored value from larry7.prf (if that file
// exists) or the same default value (if it does not) in script 64000. Since
// message type sync relies on the game only setting this value once at startup,
// we must stop the second attempt or the value from ScummVM will be
// overwritten.
// Applies to at least: English CD
static const uint16 larry7MessageTypeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x02, // ldi 2
	0xa1, 0x5a, // sag global[$5a]
	SIG_END
};

static const uint16 larry7MessageTypeResetPatch[] = {
	0x33, 0x02, // jmp [past reset]
	PATCH_END
};

//          script, description,                                signature                           patch
static const SciScriptPatcherEntry larry7Signatures[] = {
	{  true,     0, "disable message type reset on startup", 1, larry7MessageTypeResetSignature,    larry7MessageTypeResetPatch },
	{  true,   540, "fix make cheese cutscene (cycler)",     1, larry7MakeCheeseCyclerSignature,    larry7MakeCheeseCyclerPatch },
	{  true,   540, "fix make cheese cutscene (priority)",   1, larry7MakeCheesePrioritySignature,  larry7MakeCheesePriorityPatch },
	{  true, 64000, "disable volume reset on startup (1/2)", 1, larry7VolumeResetSignature1,        larry7VolumeResetPatch1 },
	{  true, 64000, "disable volume reset on startup (2/2)", 1, larry7VolumeResetSignature2,        larry7VolumeResetPatch2 },
	{  true, 64866, "increase number of save games",         1, torinLarry7NumSavesSignature,       torinLarry7NumSavesPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Laura Bow 1 - Colonel's Bequest
//
// This is basically just a broken easter egg in Colonel's Bequest.
// A plane can show up in room 4, but that only happens really rarely.
// Anyway the Sierra developer seems to have just entered the wrong loop,
// which is why the statue view is used instead (loop 0).
// We fix it to use the correct loop.
//
// This is only broken in the PC version. It was fixed for Amiga + Atari ST.
//
// Credits to OmerMor, for finding it.
//
// Applies to at least: English PC Floppy
// Responsible method: room4::init
static const uint16 laurabow1SignatureEasterEggViewFix[] = {
	0x78,                               // push1
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x39, 0x03,                         // pushi 3 (loop 3, view only has 3 loops)
	SIG_END
};

static const uint16 laurabow1PatchEasterEggViewFix[] = {
	PATCH_ADDTOOFFSET(+7),
	0x02,                               // (change loop to 2)
	PATCH_END
};

// When oiling the armor or opening the visor of the armor, the scripts first
//  check if Laura/ego is near the armor and if she is not, they will move her
//  to the armor. After that, further code is executed.
//
// The current location is checked by a ego::inRect() call.
//
// The given rect for the inRect call inside openVisor::changeState was made
//  larger for Atari ST/Amiga versions. We change the PC version to use the
//  same rect.
//
// Additionally, the coordinate that Laura is moved to (152, 107) may not be
//  reachable depending on where Laura was when "use oil on helmet of armor"
//  or "open visor of armor" got entered. Bad coordinates such as (82, 110),
//  cause collisions and effectively an endless loop, effectively freezing the
//  game. The user is only able to restore a previous game.
//
//  We change the destination coordinate to (152, 110), which seems to be
//   reachable all the time.
//
// The following patch fixes the rect for the PC version of the game.
//
// Applies to at least: English PC Floppy
// Responsible method: openVisor::changeState (script 37)
// Fixes bug: #7119
static const uint16 laurabow1SignatureArmorOpenVisorFix[] = {
	0x39, 0x04,                         // pushi 04
	SIG_MAGICDWORD,
	0x39, 0x6a,                         // pushi 6a (106d)
	0x38, SIG_UINT16(0x0096),           // pushi 0096 (150d)
	0x39, 0x6c,                         // pushi 6c (108d)
	0x38, SIG_UINT16(0x0098),           // pushi 0098 (152d)
	SIG_END
};

static const uint16 laurabow1PatchArmorOpenVisorFix[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x68,                         // pushi 68 (104d)   (-2)
	0x38, PATCH_UINT16(0x0094),         // pushi 0094 (148d) (-2)
	0x39, 0x6f,                         // pushi 6f (111d)   (+3)
	0x38, PATCH_UINT16(0x009a),         // pushi 009a (154d) (+2)
	PATCH_END
};

// This here fixes the destination coordinate (exact details are above).
//
// Applies to at least: English PC Floppy, English Atari ST Floppy, English Amiga Floppy
// Responsible method: openVisor::changeState, oiling::changeState (script 37)
// Fixes bug: #7119
static const uint16 laurabow1SignatureArmorMoveToFix[] = {
	SIG_MAGICDWORD,
	0x36,                               // push
	0x39, 0x6b,                         // pushi 6B (107d)
	0x38, SIG_UINT16(0x0098),           // pushi 98 (152d)
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	SIG_END
};

static const uint16 laurabow1PatchArmorMoveToFix[] = {
	PATCH_ADDTOOFFSET(+1),
	0x39, 0x6e,                         // pushi 6E (110d) - adjust x, so that no collision can occur anymore
	PATCH_END
};

// In some cases like for example when the player oils the arm of the armor,
// command input stays disabled, even when the player exits fast enough, so
// that Laura doesn't die.
//
// This is caused by the scripts only enabling control (directional movement),
// but do not enable command input as well.
//
// This bug also happens, when using the original interpreter. It was fixed for
// the Atari ST + Amiga versions of the game.
//
// Applies to at least: English PC Floppy
// Responsible method: 2nd subroutine in script 37, called by oiling::changeState(7)
// Fixes bug: #7154
static const uint16 laurabow1SignatureArmorOilingArmFix[] = {
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x1a5c),           // lofsa "Can" - offsets are not skipped to make sure only the PC version gets patched
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x19a1),           // lofsa "Visor"
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x194a),           // lofsa "note"
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x18f3),           // lofsa "valve"
	0x4a, 0x04,                         // send 04
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x02,                         // ldi 02
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x0014),           // bnt [to ret]
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x05,                         // ldi 05
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x000c),           // bnt [to ret]
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x06,                         // ldi 06
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x0004),           // bnt [to ret]
	// followed by code to call script 0 export to re-enable controls and call setMotion
	SIG_END
};

static const uint16 laurabow1PatchArmorOilingArmFix[] = {
	PATCH_ADDTOOFFSET(+3),              // skip over pushi 89h
	0x3c,                               // dup
	0x3c,                               // dup
	0x3c,                               // dup
	// saves a total of 6 bytes
	0x76,                               // push0
	0x72, PATCH_UINT16(0x1a59),         // lofsa "Can"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, PATCH_UINT16(0x19a1),         // lofsa "Visor"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, PATCH_UINT16(0x194d),         // lofsa "note"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, PATCH_UINT16(0x18f9),         // lofsa "valve" 18f3
	0x4a, 0x04,                         // send 04
	// new code to enable input as well, needs 9 spare bytes
	0x38, PATCH_UINT16(0x00e2),         // pushi canInput
	0x78,                               // push1
	0x78,                               // push1
	0x51, 0x2b,                         // class User
	0x4a, 0x06,                         // send 06 -> call User::canInput(1)
	// original code, but changed a bit to save some more bytes
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x02,                         // ldi 02
	0x04,                               // sub
	0x31, 0x12,                         // bnt [to ret]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x04,                               // sub
	0x31, 0x0c,                         // bnt [to ret]
	0x78,                               // push1
	0x1a,                               // eq?
	0x2f, 0x08,                         // bt [to ret]
	// saves 7 bytes, we only need 3, so waste 4 bytes
	0x35, 0x00,                         // ldi 0
	0x35, 0x00,                         // ldi 0
	PATCH_END
};

// Jeeves lights the chapel candles (room 58) in act 2 but they don't stay
//  lit when re-entering until the next act. This is due to Room58:init
//  incorrectly testing the global variable that tracks Jeeves' act 2 state.
//
// We fix this by changing the test from if global[155] equals 11, which it
//  never does, to if it's greater than 11. The global is set to 12 in
//  lightCandles:changeState(11) and it continues to increment as Jeeves'
//  chore sequence progresses, ending with 17.
//
// Applies to: DOS, Amiga, Atari ST
// Responsible method: Room58:init
// Fixes bug: #10743
static const uint16 laurabow1SignatureChapelCandlesPersistence[] = {
	SIG_MAGICDWORD,
	0x89, 0x9b,                         // lsg global[155] [ Jeeves' act 2 state ]
	0x35, 0x0b,                         // ldi b
	0x1a,                               // eq?
	SIG_END
};

static const uint16 laurabow1PatchChapelCandlesPersistence[] = {
	PATCH_ADDTOOFFSET(+4),
	0x1e,                               // gt?
	PATCH_END
};

// LB1 DOS doesn't acknowledge Lillian's presence in room 44 when she's sitting
//  on the bed in act 4. Look, talk, etc respond that she's not there.
//  This is due to not setting global[195] which tracks who is in the room.
//  We fix this by setting the global as Amiga and Atari ST versions do.
//
// Applies to: DOS only
// Responsible method: Room44:init
// Fixes bug: #10742
static const uint16 laurabow1SignatureLillianBedFix[] = {
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x10f8),           // lofsa suit2 [ only matches DOS version ]
	0x4a, 0x14,                         // send 14
	SIG_ADDTOOFFSET(+8),
	0x89, 0x76,                         // lsg global[118]
	0x35, 0x02,                         // ldi 2
	0x12,                               // and
	0x30, SIG_UINT16(0x000d),           // bnt d [ haven't seen Lillian in study ]
	0x35, 0x01,                         // ldi 1
	SIG_END
};

static const uint16 laurabow1PatchLillianBedFix[] = {
	PATCH_ADDTOOFFSET(+13),
	0x81, 0x76,                         // lag global[118]
	0x7a,                               // push2
	0x12,                               // and
	0x31, 0x0f,                         // bnt f [ haven't seen Lillian in study ]
	0x35, 0x20,                         // ldi 20 [ Lillian ]
	0xa1, 0xc3,                         // sag global[195] [ set Lillian as in the room ]
	PATCH_END
};

// Entering Laura's bedroom resets the cursor position to the upper right corner
//  of the screen as if the game were starting. Room44:init calls kSetCursor to
//  initialize the game because it's the first room where the user has control,
//  but this part of the script is missing the startup test and so it happens
//  every time. We fix this by adding the missing startup test.
//
// Applies to: All versions
// Responsible method: Room44:init
static const uint16 laurabow1SignatureRoom44CursorFix[] = {
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 00
	0xa1, 0xbe,                         // sag be [ global190 = 0 ]
	0x39, SIG_SELECTOR8(init),          // pushi init
	0x76,                               // push0
	0x57, 0x37, 0x04,                   // super Rm 04 [ super init: ]
	SIG_ADDTOOFFSET(+10),
	0x43, 0x28, 0x08,                   // callk SetCursor 08 [ SetCursor 997 1 300 0 ]
	SIG_ADDTOOFFSET(+439),
	0x89, 0xa5,                         // lsg a5
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x30,                               // bnt
	SIG_END
};

static const uint16 laurabow1PatchRoom44CursorFix[] = {
	0x39, PATCH_SELECTOR8(init),        // pushi init
	0x76,                               // push0
	0x57, 0x37, 0x04,                   // super Rm 04 [ super init: ]
	0x81, 0xcb,                         // lag cb [ global203 == 0 when game is starting ]
	0x2f, 0x0d,                         // bt 0d  [ skip SetCursor if game already started ]
	PATCH_ADDTOOFFSET(+452),
	0x76,                               // push0
	0xa9, 0xbe,                         // ssg be [ global190 = 0 ]
	0x81, 0xa5,                         // lag a5
	0x2e,                               // bt
	PATCH_END
};

// When you tell Lilly about Gertie in room 35, Lilly will then walk to the
// left and off the screen. If Laura (ego) is in the way, the whole game will
// basically block and you won't be able to do anything except saving or
// restoring the game.
//
// If this happened already, the player can enter "send Lillian ignoreActors 1"
// inside the debugger to fix this situation.
//
// This issue is very difficult to solve, because Lilly also walks diagonally
// after walking to the left right under the kitchen table. This means that
// even if we added a few more rectangle checks, there could still be spots,
// where the game would block.
//
// Also the mover "PathOut" is used for Lillian instead of the regular
// "MoveTo", which would avoid other actors by itself.
//
// So instead we set Lilly to ignore other actors during that cutscene, which
// is the least invasive solution.
//
// Applies to at least: English PC Floppy, English Amiga Floppy, English Atari ST Floppy
// Responsible method: goSee::changeState(1) in script 236
// Fixes bug: (happened during GOG Let's Play)
static const uint16 laurabow1SignatureTellLillyAboutGerieBlockingFix1[] = {
	0x7a,                               // puah2
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00c1),           // pushi 00C1h
	0x38, SIG_UINT16(0x008f),           // pushi 008Fh
	0x38, SIG_SELECTOR16(ignoreActors), // pushi ignoreActors
	0x78,                               // push1
	0x76,                               // push0
	SIG_END
};

static const uint16 laurabow1PatchTellLillyAboutGertieBlockingFix1[] = {
	PATCH_ADDTOOFFSET(+11),             // skip over until push0
	0x78,                               // push1 (change push0 to push1)
	PATCH_END
};

// a second patch to call Lillian::ignoreActors(1) on goSee::changeState(9) in script 236
static const uint16 laurabow1SignatureTellLillyAboutGerieBlockingFix2[] = {
	0x3c,                               // dup
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x003f),           // bnt [ret]
	0x39, SIG_ADDTOOFFSET(+1),          // pushi view
	0x78,                               // push1
	0x38, SIG_UINT16(0x0203),           // pushi 203h (515d)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi posn
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c9),           // pushi C9h (201d)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0084),           // pushi 84h (132d)
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa Lillian (different offsets for different platforms)
	0x4a, 0x0e,                         // send 0Eh
	SIG_END
};

static const uint16 laurabow1PatchTellLillyAboutGertieBlockingFix2[] = {
	0x38, PATCH_SELECTOR16(ignoreActors), // pushi ignoreActors
	0x78,                                 // push1
	0x76,                                 // push0
	0x33, 0x00,                           // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET(+19),               // skip over until send
	0x4a, 0x14,                           // send 14h
	PATCH_END
};

// LB1 contains over 20 commands which lockup the game if entered while ego is
//  colliding with an obstacle. Opening and moving closets in room 43 are prime
//  examples. They are all symptoms of a global bug.

// Every cycle, CB1:doit checks to see if ego appears to be walking and blocked,
//  and if so it stops ego's motion and sets ego's view to 11 (standing). If ego
//  has just collided with an obstacle but is still displaying view 1 (walking)
//  when a command is entered which disables input and sets ego's motion without
//  setting an avoider then CB1:doit will stop that new motion at the start of
//  the next cycle and lockup the game. This occurs in part because CB1:doit
//  tests potentially stale values that the new motion hasn't yet had a chance
//  to set. Scripts which set an avoider aren't vulnerable because CB1:doit
//  won't stop ego if one is set. Other SCI games don't have this kind of code
//  in their Game's doit and so they don't have this bug.

// We fix this by clearing the kSignalHitObstacle flag whenever Act:setMotion is
//  called with a new motion. This causes CB1:doit's subsequent call to
//  ego:isBlocked to return false, instead of a stale true value, preventing
//  CB1:doit from stopping the new motion before it's had a chance to start.
//  Should the new motion actually be blocked then the interpreter will then set
//  the flag when processing the motion as usual. This patch closes the short
//  window during which this value is stale and CB1:doit tests it.

// Applies to: DOS, Amiga, Atari ST and occurs in Sierra's interpreter.
// Responsible method: Act:setMotion
// Fixes bug: #10733
static const uint16 laurabow1SignatureObstacleCollisionLockupsFix[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x002f),           // bnt 2f
	0x38, SIG_UINT16(0x00a3),           // pushi a3 [ startUpd ]
	0x76,                               // push0
	0x54, 0x04,                         // self 4
	0x7a,                               // push2 [ -info- ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param[1]
	0x4a, 0x04,                         // send 4
	0x36,                               // push
	0x34, SIG_UINT16(0x8000),           // ldi 8000
	0x12,                               // and
	0x30, SIG_UINT16(0x000a),           // bnt a
	0x39, 0x56,                         // pushi 56 [ new ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param[1]
	0x4a, 0x04,                         // send 4
	0x32, SIG_UINT16(0x0002),           // jmp 2
	0x87, 0x01,                         // lap param[1]
	0x65, 0x4c,                         // aTop mover
	0x39, 0x57,                         // pushi 57 [ init ]
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x59, 0x02,                         // &rest 2
	0x63, 0x4c,                         // pToa mover
	0x4a, 0x06,                         // send 6
	0x32, SIG_UINT16(0x0004),           // jmp 4
	0x35, 0x00,                         // ldi 0
	SIG_END
};

static const uint16 laurabow1PatchObstacleCollisionLockupsFix[] = {
	0x31, 0x32,                         // bnt 32 [ save 1 byte ]

	0x63, 0x1c,                         // pToa signal
	0x38, PATCH_UINT16(0xfbff),         // pushi fbff
	0x12,                               // and
	0x65, 0x1c,                         // aTop signal [ clear kSignalHitObstacle (0400) ]

	0x38, PATCH_UINT16(0x00a3),         // pushi a3 [ startUpd ]
	0x76,                               // push0
	0x54, 0x04,                         // self 4
	0x7a,                               // push2 [ -info- ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param[1]
	0x4a, 0x04,                         // send 4
	0x38, PATCH_UINT16(0x8000),         // pushi 8000 [ save 1 byte ]
	0x12,                               // and
	0x31, 0x09,                         // bnt 9 [ save 1 byte ]
	0x39, 0x56,                         // pushi 56 [ new ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param[1]
	0x4a, 0x04,                         // send 4
	0x33, 0x02,                         // jmp 2 [ save 1 byte ]
	0x87, 0x01,                         // lap param[1]
	0x65, 0x4c,                         // aTop mover
	0x39, 0x57,                         // pushi 57 [ init ]
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x59, 0x02,                         // &rest 2
	0x63, 0x4c,                         // pToa mover
	0x4a, 0x06,                         // send 6
	0x48,                               // ret  [ save 4 bytes ]
	PATCH_END
};

// Laura can get stuck walking up the attic stairs diagonally in room 47 and
//  lockup the game. This also occurs in the original. Room47:doit loads the
//  attic when ego is on control $10 at the base of the stairs and facing north.
//  Room47:handleEvent prevents the user from moving ego when on control $10.
//  Walking up the stairs diagonally puts ego in control $10 facing left or
//  right and so the room never changes and the user never regains control.
//
// We fix this by allowing ego to face any direction except south to trigger the
//  attic room change. This also fixes an edge case that allows walking through
//  the staircase wall into Clarence's room.
//
// Applies to: DOS, Amiga, Atari ST
// Responsible method: Room47:doit
// Fixes bug: #9949
static const uint16 laurabow1SignatureAtticStairsLockupFix[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(loop),          // pushi loop
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 4 [ ego:loop? ]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03 [ facing north ]
	0x1a,                               // eq?
	SIG_END
};

static const uint16 laurabow1PatchAtticStairsLockupFix[] = {
	PATCH_ADDTOOFFSET(+8),
	0x35, 0x02,                         // ldi 02 [ facing south ]
	0x1c,                               // ne?
	PATCH_END
};

// Laura can get stuck at the top of the left stairs in room 47 and lockup the
//  game. This also occurs in the original. There is a 30x2 control area at the
//  top of the stairs in which Room47:handleEvent prevents input. This assumes
//  that ego can't be interrupted when walking through the area, but there is a
//  notch in the left wall that ego can collide with, leaving ego stuck with
//  input disabled. The right wall doesn't have a notch.
//
// We fix this by allowing input at the top of the stairs. Up and down movements
//  are allowed when on the staircase's control area ($0200) and we extend that
//  to include the top of the stairs ($0800).
//
// Applies to: DOS, Amiga, Atari ST
// Responsible method: Room47:handleEvent
// Fixes bug #10879
static const uint16 laurabow1SignatureLeftStairsLockupFix[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x0200),           // ldi 0200 [ left stairs ]
	0x1a,                               // eq? [ is ego entirely on the stairs? ]
	SIG_END
};

static const uint16 laurabow1PatchLeftStairsLockupFix[] = {
	0x34, PATCH_UINT16(0x0a00),         // ldi 0a00 [ left stairs | top of left stairs ]
	0x12,                               // and [ is ego touching the stairs or the top? ]
	PATCH_END
};

// LB1's fingerprint copy protection randomly rejects the correct answer and may
//  even fail to draw a fingerprint. myCopy:init selects the fingerprint by
//  generating random loop and cel numbers for view 553, but it passes incorrect
//  ranges to kRandom. If kRandom returns the maximum value then the loop or cel
//  overflow and a different image is displayed than what was intended.
//
// We correct the ranges from 0-600 and 1-1000 to 0-599 and 0-999 so that
//  invalid cel 6 and loop 10 are never used after the script divides by 10.
//
// Applies to: DOS, Amiga, Atari ST
// Responsible method: myCopy:init
static const uint16 laurabow1SignatureCopyProtectionRandomFix[] = {
	0x38, SIG_UINT16(0x0258),           // pushi 600d
	SIG_ADDTOOFFSET(+10),
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x03e8),           // pushi 1000d
	SIG_END
};

static const uint16 laurabow1PatchCopyProtectionRandomFix[] = {
	0x38, PATCH_UINT16(0x0257),         // pushi 599d
	PATCH_ADDTOOFFSET(+10),
	0x76,                               // push0
	0x38, PATCH_UINT16(0x03e7),         // pushi 999d
	PATCH_END
};

// At the end of the game a dialog asks if you'd like to review your notes.
//  Pressing the escape key dismisses the dialog and locks up the game with the
//  menu and input disabled. The script is missing a handler for this result.
//  We fix this by repeating the dialog when escape is pressed, just like the
//  other dialogs do in room 786. Sierra fixed this in later versions.
//
// Applies to: DOS
// Responsible method: Notes:changeState(1)
static const uint16 laurabow1SignatureReviewNotesDialog[] = {
	0x39, 0x0f,                         // pushi 0f
	SIG_ADDTOOFFSET(+0x20),
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x47, 0xff, 0x00, 0x1e,             // calle proc255_0 [ print dialog ]
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	SIG_END
};

static const uint16 laurabow1PatchReviewNotesDialog[] = {
	PATCH_ADDTOOFFSET(+0x27),
	0x31, 0xd7,                         // bnt -29 [ repeat dialog if escape pressed ]
	0x36,                               // push
	0x78,                               // push1
	PATCH_END
};

//          script, description,                                signature                                             patch
static const SciScriptPatcherEntry laurabow1Signatures[] = {
	{  true,     4, "easter egg view fix",                      1, laurabow1SignatureEasterEggViewFix,                laurabow1PatchEasterEggViewFix },
	{  true,    37, "armor open visor fix",                     1, laurabow1SignatureArmorOpenVisorFix,               laurabow1PatchArmorOpenVisorFix },
	{  true,    37, "armor move to fix",                        2, laurabow1SignatureArmorMoveToFix,                  laurabow1PatchArmorMoveToFix },
	{  true,    37, "allowing input, after oiling arm",         1, laurabow1SignatureArmorOilingArmFix,               laurabow1PatchArmorOilingArmFix },
	{  true,    44, "lillian bed fix",                          1, laurabow1SignatureLillianBedFix,                   laurabow1PatchLillianBedFix },
	{  true,    44, "room 44 cursor fix",                       1, laurabow1SignatureRoom44CursorFix,                 laurabow1PatchRoom44CursorFix },
	{  true,    47, "attic stairs lockup fix",                  1, laurabow1SignatureAtticStairsLockupFix,            laurabow1PatchAtticStairsLockupFix },
	{  true,    47, "left stairs lockup fix",                   3, laurabow1SignatureLeftStairsLockupFix,             laurabow1PatchLeftStairsLockupFix },
	{  true,    58, "chapel candles persistence",               1, laurabow1SignatureChapelCandlesPersistence,        laurabow1PatchChapelCandlesPersistence },
	{  true,   236, "tell Lilly about Gertie blocking fix 1/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix1, laurabow1PatchTellLillyAboutGertieBlockingFix1 },
	{  true,   236, "tell Lilly about Gertie blocking fix 2/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix2, laurabow1PatchTellLillyAboutGertieBlockingFix2 },
	{  true,   414, "copy protection random fix",               1, laurabow1SignatureCopyProtectionRandomFix,         laurabow1PatchCopyProtectionRandomFix },
	{  true,   786, "review notes dialog fix",                  1, laurabow1SignatureReviewNotesDialog,               laurabow1PatchReviewNotesDialog },
	{  true,   998, "obstacle collision lockups fix",           1, laurabow1SignatureObstacleCollisionLockupsFix,     laurabow1PatchObstacleCollisionLockupsFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Laura Bow 2
//
// Moving away the painting in the room with the hidden safe is problematic
//  for the CD version of the game. safePic::doVerb gets triggered by the mouse-click.
// This method sets local[0] as signal, which is only meant to get handled, when
//  the player clicks again to move the painting back. This signal is processed by
//  the room doit-script.
// That doit-script checks safePic::cel to be not equal 0 and would then skip over
//  the "close painting" trigger code. On very fast computers this script may
//  get called too early (which is the case when running under ScummVM and when
//  running the game using Sierra SCI in DOS-Box with cycles 15000) and thinks
//  that it's supposed to move the painting back. Which then results in the painting
//  getting moved to its original position immediately (which means it won't be possible
//  to access the safe behind it).
//
// We patch the script, so that we check for cel to be not equal 4 (the final cel) and
//  we also reset the safePic-signal immediately as well.
//
// In the floppy version Laura's coordinates are checked directly in rm560::doit
//  and as soon as she moves, the painting will automatically move to its original position.
//  This is not the case for the CD version of the game. The painting will only "move" back,
//  when the player actually exits the room and re-enters.
//
// Applies to at least: English PC-CD
// Responsible method: rm560::doit
// Fixes bug: #6460
static const uint16 laurabow2CDSignaturePaintingClosing[] = {
	0x39, 0x04,                         // pushi 04 (cel)
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0231),           // pushi 0231h (561)
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // callk ScriptID, 04 (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x18,                               // not
	0x31, 0x21,                         // bnt [exit]
	0x38, SIG_UINT16(0x0283),           // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // callk ScriptID, 04 (get export 0 of script 32)
	0x4a, 0x04,                         // send 04 (get sHeimlich::room)
	0x36,                               // push
	0x81, 0x0b,                         // lag global[b] (current room)
	0x1c,                               // ne?
	0x31, 0x0e,                         // bnt [exit]
	0x35, 0x00,                         // ldi 00
	0xa3, 0x00,                         // sal local[0] (reset safePic signal)
	SIG_END
};

static const uint16 laurabow2CDPatchPaintingClosing[] = {
	PATCH_ADDTOOFFSET(+2),
	0x3c,                               // dup (1 additional byte)
	0x76,                               // push0
	0x3c,                               // dup (1 additional byte)
	0xab, 0x00,                         // ssl local[0] (reset safePic signal)
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0231),         // pushi 0231h (561)
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // callk ScriptID, 04 (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x1a,                               // eq?
	0x31, 0x1d,                         // bnt [exit]
	0x38, PATCH_UINT16(0x0283),         // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // callk ScriptID, 04 (get export 0 of script 32)
	0x4a, 0x04,                         // send 04 (get sHeimlich::room)
	0x36,                               // push
	0x81, 0x0b,                         // lag global[b] (current room)
	0x1a,                               // eq? (2 opcodes changed, to save 2 bytes)
	0x2f, 0x0a,                         // bt [exit]
	PATCH_END
};

// In the CD version the system menu is disabled for certain rooms. LB2::handsOff is called,
//  when leaving the room (and in other cases as well). This method remembers the disabled
//  icons of the icon bar. In the new room LB2::handsOn will get called, which then enables
//  all icons, but also disabled the ones, that were disabled before.
//
// Because of this behaviour certain rooms, that should have the system menu enabled, have
//  it disabled, when entering those rooms from rooms, where the menu is supposed to be
//  disabled.
//
// We patch this by injecting code into LB2::newRoom (which is called right after a room change)
//  and reset the global variable there, that normally holds the disabled buttons.
//
// This patch may cause side-effects and it's difficult to test, because it affects every room
//  in the game. At least for the intro, the speakeasy and plenty of rooms in the beginning it
//  seems to work correctly.
//
// Applies to at least: English PC-CD
// Responsible method: LB2::newRoom, LB2::handsOff, LB2::handsOn
// Fixes bug: #6440
static const uint16 laurabow2CDSignatureFixProblematicIconBar[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00f1),           // pushi 00f1 (disable) - hardcoded, we only want to patch the CD version
	0x76,                               // push0
	0x81, 0x45,                         // lag global[45]
	0x4a, 0x04,                         // send 04
	SIG_END
};

static const uint16 laurabow2CDPatchFixProblematicIconBar[] = {
	0x35, 0x00,                      // ldi 00
	0xa1, 0x74,                      // sag global[74]
	0x35, 0x00,                      // ldi 00 (waste bytes)
	0x35, 0x00,                      // ldi 00
	PATCH_END
};

// LB2 CD responds with the wrong message when asking Yvette about Tut in acts 3+.
//
// aYvette:doVerb(6) tests flag 134, which is set when Pippin dies, to determine
//  which Tut message to display but they got it backwards. One of the messages
//  has additional dialogue about Pippin's murder.
//
// This is a regression introduced by Sierra when they fixed a bug from the floppy
//  versions where asking Yvette about Tut in act 2 responds with the message
//  about Pippin's murder which hasn't occurred yet, bug #10723. Sierra correctly
//  fixed that in Yvette:doVerb in script 93, which applies to act 2, but then went
//  on to add incorrect code to aYvette:doVerb in script 90, which applies to the
//  later acts after the murder.
//
// We fix this by reversing the flag test so that the correct message is displayed.
//
// Applies to: CD version, at least English
// Responsible method: aYvette:doVerb
// Fixes bug: #10724
static const uint16 laurabow2CDSignatureFixYvetteTutResponse[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x010f),           // ldi 010f [ tut ]
	0x1a,                               // eq? [ asked about tut? ]
	0x30, SIG_UINT16(0x0036),           // bnt 0036
	0x78,                               // push1
	0x38, SIG_UINT16(0x0086),           // pushi 0086 [ pippin-dead flag ]
	0x45, 0x02, 0x02,                   // callb [export 2 of script 0], 02 [ is pippin-dead flag set? ]
	0x30, SIG_UINT16(0x0016),           // bnt 0016 [ pippin-dead message ]
	SIG_END
};

static const uint16 laurabow2CDPatchFixYvetteTutResponse[] = {
	PATCH_ADDTOOFFSET(+14),
	0x2e,                               // bt (replace bnt)
	PATCH_END
};

// When entering the main musem party room (w/ the golden Egyptian head), Laura
// is walking a bit into the room automatically. If you press a mouse button
// while this is happening, you will get stuck inside that room and won't be
// able to exit it anymore.
//
// Users, who played the game w/ a previous version of ScummVM can simply enter
// the debugger and then enter "send rm350 script 0:0", which will fix the
// script state.
//
// This is caused by the user controls not being locked at that point. Pressing
// a button will cause the cue from the PolyPath walker to never happen, which
// then causes sEnterSouth to never dispose itself.
//
// User controls are locked in the previous room 335, but controls are unlocked
// by frontDoor::cue. We do not want to change this, because it could have
// side-effects. We instead add another LB2::handsOff call inside the script
// responsible for Laura's walk into the room (sEnterSouth::changeState(0).
//
// Applies to at least: English PC-CD, English PC-Floppy, German PC-Floppy
// Responsible method: sEnterSouth::changeState
// Fixes bug: (no bug report, from GOG forum post)
static const uint16 laurabow2SignatureMuseumPartyFixEnteringSouth1[] = {
	0x3c,                              // dup
	0x35, 0x00,                        // ldi 00
	0x1a,                              // eq?
	0x30, SIG_UINT16(0x0097),          // bnt [state 1 code]
	SIG_ADDTOOFFSET(+141),             // skip to end of follow-up code
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x008d for CD, 0x007d for floppy)
	0x35, 0x01,                        // ldi 01
	0x65, 0x1a,                        // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x0086 for CD, 0x0076 for floppy)
	// state 1 code
	0x3c,                              // dup
	0x35, 0x01,                        // ldi 01
	0x1a,                              // eq?
	SIG_MAGICDWORD,
	0x31, 0x05,                        // bnt [state 2 code]
	0x35, 0x00,                        // ldi 00
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x007b for CD, 0x006b for floppy)
	// state 2 code
	0x3c,                              // dup
	SIG_END
};

static const uint16 laurabow2PatchMuseumPartyFixEnteringSouth1[] = {
	0x2e, PATCH_UINT16(0x00a6),        // bt [state 2 code] (we skip state 1, because it's a NOP anyways)
	// state 0 processing
	0x32, PATCH_UINT16(0x0097),        // jmp 151d [after this ret]
	PATCH_ADDTOOFFSET(+149),           // skip to end of follow-up code
	// save 1 byte by replacing jump to [ret] into straight toss/ret
	0x3a,                              // toss
	0x48,                              // ret

	// additional code, that gets called right at the start of step 0 processing
	0x18,                              // not -- this here is where pushi handsOff will be inserted by the second patch
	0x18,                              // not    offset and handsOff is different for floppy + CD, that's why we do this
	0x18,                              // not    floppy also does not have a selector table, so we can't go by "handsOff" name
	0x18,                              // not
	0x76,                              // push0
	0x81, 0x01,                        // lag global[1]
	0x4a, 0x04,                        // send 04
	0x32, PATCH_UINT16(0xff5e),        // jmp [back to start of step 0 processing]
	PATCH_END
};

// Second patch, which only inserts pushi handsOff inside our new code. There
// is no other way to do this except making 2 full patches for floppy + CD,
// because handsOff/handsOn is not the same value between floppy + CD *and*
// floppy doesn't even have a vocab, so we can't figure out the id by
// ourselves.
static const uint16 laurabow2SignatureMuseumPartyFixEnteringSouth2[] = {
	0x18,                              // our injected code
	0x18,
	0x18,
	SIG_ADDTOOFFSET(+92),              // skip to the handsOn code, that we are interested in
	0x38, SIG_ADDTOOFFSET(+2),         // pushi handsOn (0x0189 for CD, 0x024b for floppy)
	0x76,                              // push0
	0x81, 0x01,                        // lag global[1]
	0x4a, 0x04,                        // send 04
	0x38, SIG_ADDTOOFFSET(+2),         // pushi 0274h
	SIG_MAGICDWORD,
	0x78,                              // push1
	0x38, SIG_UINT16(0x033f),          // pushi 033f
	SIG_END
};

static const uint16 laurabow2PatchMuseumPartyFixEnteringSouth2[] = {
	0x38, PATCH_GETORIGINALUINT16ADJUST(+96, -1), // pushi handsOff (@handsOn - 1)
	PATCH_END
};

// Opening/Closing the east door in the pterodactyl room doesn't check, if it's
//  locked and will open/close the door internally even when it is.
//
// It will get wired shut later in the game by Laura Bow and will be "locked"
//  because of this. We patch in a check for the locked state. We also add
//  code, that will set the "locked" state in case our eastDoor-wired-global is
//  set. This makes the locked state effectively persistent.
//
// Applies to at least: English PC-CD, English PC-Floppy
// Responsible method (CD): eastDoor::doVerb
// Responsible method (Floppy): eastDoor::<noname300>
// Fixes bug: #6458 (partly, see additional patch below)
static const uint16 laurabow2SignatureFixWiredEastDoor[] = {
	0x30, SIG_UINT16(0x0022),           // bnt [skip hand action]
	0x67, SIG_ADDTOOFFSET(+1),          // pTos (CD: doorState, Floppy: state)
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x08,                         // bnt [close door code]
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x63,                         // pushi 63h
	0x45, 0x04, 0x02,                   // callb [export 4 of script 0], 02 (sets door-bitflag)
	0x33, 0x06,                         // jmp [super-code]
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x45, 0x03, 0x02,                   // callb [export 3 of script 0], 02 (resets door-bitflag)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi (CD: 011dh, Floppy: 012ch)
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x59, 0x02,                         // rest 02
	0x57, SIG_ADDTOOFFSET(+1), 0x06,    // super (CD: LbDoor, Floppy: Door), 06
	0x33, 0x0b,                         // jmp [ret]
	SIG_END
};

static const uint16 laurabow2PatchFixWiredEastDoor[] = {
	0x31, 0x23,                         // bnt [skip hand action] (saves 1 byte)
	0x81, 0x61,                         // lag global[97d] (get our eastDoor-wired-global)
	0x31, 0x04,                         // bnt [skip setting locked property]
	0x35, 0x01,                         // ldi 01
	0x65, 0x6a,                         // aTop locked (set eastDoor::locked to 1)
	0x63, 0x6a,                         // pToa locked (get eastDoor::locked)
	0x2f, 0x17,                         // bt [skip hand action]
	0x63, PATCH_GETORIGINALBYTE(+4),    // pToa (CD: doorState, Floppy: state)
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x2f, 0x05,                         // bt [close door code]
	0x45, 0x04, 0x02,                   // callb [export 4 of script 0], 02 (sets door-bitflag)
	0x33, 0x0b,                         // jmp [super-code]
	0x45, 0x03, 0x02,                   // callb [export 3 of script 0], 02 (resets door-bitflag)
	0x33, 0x06,                         // jmp [super-code]
	PATCH_END
};

// We patch in code, so that our eastDoor-wired-global will get set to 1.
//  This way the wired-state won't get lost when exiting room 430.
//
// Applies to at least: English PC-CD, English PC-Floppy
// Responsible method (CD): sWireItShut::changeState
// Responsible method (Floppy): sWireItShut::<noname144>
// Fixes bug: #6458 (partly, see additional patch above)
static const uint16 laurabow2SignatureRememberWiredEastDoor[] = {
	SIG_MAGICDWORD,
	0x33, 0x27,                         // jmp [ret]
	0x3c,                               // dup
	0x35, 0x06,                         // ldi 06
	0x1a,                               // eq?
	0x31, 0x21,                         // bnt [skip step]
	SIG_END
};

static const uint16 laurabow2PatchRememberWiredEastDoor[] = {
	PATCH_ADDTOOFFSET(+2),              // skip jmp [ret]
	0x34, PATCH_UINT16(0x0001),         // ldi 0001
	0xa1, PATCH_UINT16(0x0061),         // sag global[97d] (set our eastDoor-wired-global)
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm
// It's possible to walk through the closed door in room 448 and enter the crate
//  room before act 5 due to differences in our pathfinding algorithm from Sierra's.
//  This edge case appears to be due to one of the door's points being one pixel
//  within the room's walkable boundary, allowing ego to walk through this edge
//  from certain positions. We work around this by moving the door's point by
//  two pixels so that it's outside the room's walkable boundary.
//
// Applies to: All Floppy and CD versions
// Responsible method: transomDoor:createPoly
// Fixes bug: #9952
static const uint16 laurabow2SignatureFixArmorHallDoorPathfinding[] = {
	SIG_MAGICDWORD,
	0x39, 0x50,                         // pushi 50 [ x =  80 ]
	0x39, 0x7d,                         // pushi 7d [ y = 125 ]
	SIG_END
};

static const uint16 laurabow2PatchFixArmorHallDoorPathfinding[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x7f,                         // pushi 7f [ y = 127 ]
	PATCH_END
};

// The crate room (room 460) in act 5 locks up the game if you enter from the
//  elevator (room 660), swing the hanging crate, and then attempt to leave
//  back through the elevator door.
//
// The state of the wall crate that blocks the elevator door is tracked by
//  setting local[0] to 1 when you push it out of the way, but Sierra forgot
//  to reinitialize local[0] when you re-enter via the elevator door, causing
//  it to be out of sync with the room state. When you then swing the hanging
//  crate, sSwingIt:changeState(6) tests local[0] to see which polygon it
//  should set as the room's obstacle and incorrectly uses the one that blocks
//  both doors. Attempting to use the elevator door then locks up the game as
//  the obstacle polygon prevents ego from reaching the destination.
//
// Someone noticed that local[0] wasn't always initialized as
//  shoveCrate:doVerb(4) tests both local[0] and the previous room to see if it
//  was the elevator.
//
// We fix this by setting local[0] to 1 if the previous room was the elevator
//  during sSwingIt:changeState(3), just in time before it gets tested in
//  sSwingIt:changeState(6). Luckily for us, the handlers for states 3 and 4
//  don't do anything but load zero, making them two consecutive conditions
//  of no-ops. By merging them into a single condition for state 3 we have
//  a whopping 13 bytes available to add code to set local[0] correctly.
//
// Applies to: All Floppy and CD versions
// Fixes bug: #10701
static const uint16 laurabow2SignatureFixCrateRoomEastDoorLockup[] = {
	0x1a,                               // eq? [ state 3? ]
	SIG_MAGICDWORD,
	0x31, 0x05,                         // bnt [ state 4 ]
	0x35, 0x00,                         // ldi 0
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ exit switch. floppy: b3, cd: bb ]
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 4
	0x1a,                               // eq? [ state 4? ]
	0x31, 0x05,                         // bnt [ state 5 ]
	SIG_END
};

static const uint16 laurabow2PatchFixCrateRoomEastDoorLockup[] = {
	PATCH_ADDTOOFFSET(+1),              // eq? [ state 3? ]
	0x31, 0x10,                         // bnt [ state 5 ]
	0x89, 0x0c,                         // lsg global[0c] [ previous room # ]
	0x34, PATCH_UINT16(0x0294),         // ldi 660d [ elevator room # ]
	0x1a,                               // eq?
	0x8b, 0x00,                         // lsl local[0]
	0x02,                               // add
	0xa3, 0x00,                         // sal local[0] [ local[0] += (global[0c] == 660d) ]
	PATCH_END
};

// Ego can get stuck in the elevator (room 660) by walking to the lower left.
//  This also happens in Sierra's interpreter. We adjust the room's obstacle
//  polygon so that ego can't reach the problematic corner positions.
//  This is a heap patch for the coordinates used in poly2660a:points.
//
// Applies to: All Floppy and CD versions
// Fixes bug: #10702
static const uint16 laurabow2SignatureFixElevatorLockup[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x008b),                 // x = 139d
	SIG_UINT16(0x0072),                 // y = 114d
	SIG_UINT16(0x007b),                 // x = 123d
	SIG_UINT16(0x008d),                 // y = 141d
	SIG_END
};

static const uint16 laurabow2PatchFixElevatorLockup[] = {
	PATCH_UINT16(0x008f),               // x = 143d
	PATCH_ADDTOOFFSET(+4),
	PATCH_UINT16(0x00aa),               // y = 170d
	PATCH_END
};

// The act 4 back rub scene in Yvette's (room 550) locks up the game when
//  entered from Carrington's (room 560) instead of the hallway (room 510).
//
// The difference is that entering from the hallway sets the room script to
//  eRS (Enter Room Script) and entering from Carrington's doesn't set any
//  room script. When sBackRubInterrupted moves ego off screen to the south,
//  lRS (Leave Room Script) is run by LBRoom:doit if a room script isn't set,
//  and lRS:changState(0) calls handsOff. Since control is already disabled,
//  this unexpected second handsOff causes handsOn(1) to restore the disabled
//  state in the hallway and the user never regains control.
//
// We fix this by setting sBackRubInterrupted as the room's script instead of
//  backRub's script in backRub:doVerb/<noname300>(0). The script executes the
//  same but having it set as the room script prevents LBRoom:doit from running
//  lRS which prevents the extra handsOff. This patch overwrites backRub's
//  default verb handler but that's okay because that code never executes.
//  doVerb is only called by sBackRubViewing:changeState(6) which passes verb 0.
//  The entire scene is in handsOff mode so the user can't send any verbs.
//
// Affects: All Floppy and CD versions
// Responsible method: backRub:doVerb/<noname300> in script 550
// Fixes bug: #10729
static const uint16 laurabow2SignatureFixBackRubEastEntranceLockup[] = {
	SIG_MAGICDWORD,
	0x31, 0x0c,                         // bnt 0c    [ unused default verb handler ]
	0x38, SIG_UINT16(0x0092),           // pushi 0092 [ setScript/<noname146> ]
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa sBackRubInterrupted [ cd: 0c94, floppy: 0c70 ]
	0x36,                               // push
	0x54, 0x06,                         // self 6 [ self:setScript sBackRubInterrupted ]
	0x33, 0x09,                         // jmp 9  [ exit switch ]
	0x38, SIG_ADDTOOFFSET(+2),          // pushi doVerb/<noname300> [ cd: 011d, floppy: 012c ]
	SIG_END
};

static const uint16 laurabow2PatchFixBackRubEastEntranceLockup[] = {
	PATCH_ADDTOOFFSET(+10),
	0x81, 0x02,                         // lag global[2] [ rm550 ]
	0x4a, 0x06,                         // send 6 [ rm550:setScript sBackRubInterrupted ]
	0x32, PATCH_UINT16(0x0006),         // jmp 6  [ exit switch ]
	PATCH_END
};

// The act 4 back rub scene in Yvette's (room 550) doesn't draw the typewriter,
//  desk lamp, and wastebasket when returning from Laura's close-up. These views
//  are added to the room pic and so they are disposed of when the pic changes.
//  This also occurs in Sierra's interpreter.
//
// We fix this by removing the addToPic call from these views so that they are
//  members of the cast like the other views in the room and survive pic change.
//
// Applies to: All Floppy and CD versions
// Responsible method: rm550:init/<noname110>
// Fixes bug #10894
static const uint16 laurabow2SignatureFixDisappearingDeskItems[] = {
	SIG_MAGICDWORD,
	0x39, 0x64,                         // pushi 64
	0x39, 0x7e,                         // pushi 7e
	0x38, SIG_ADDTOOFFSET(+2),          // pushi addToPic
	0x76,                               // push0
	SIG_ADDTOOFFSET(+3),
	0x4a, 0x20,                         // send 20 [ typewriter ... addToPic: ]
	SIG_ADDTOOFFSET(+26),
	0x38, SIG_ADDTOOFFSET(+2),          // pushi addToPic
	0x76,                               // push0
	SIG_ADDTOOFFSET(+3),
	0x4a, 0x18,                         // send 18 [ deskLamp ... addToPic: ]
	SIG_ADDTOOFFSET(+27),
	0x38, SIG_ADDTOOFFSET(+2),          // pushi addToPic
	0x76,                               // push0
	SIG_ADDTOOFFSET(+17),
	0x4a, 0x16,                         // send 16 [ wasteBasket ... addToPic: ... ]
	SIG_END
};

static const uint16 laurabow2PatchFixDisappearingDeskItems[] = {
	PATCH_ADDTOOFFSET(+4),
	0x32, PATCH_UINT16(0x0001),         // jmp 0001
	PATCH_ADDTOOFFSET(+4),
	0x4a, 0x1c,                         // send 1c [ init typewriter without addToPic ]
	PATCH_ADDTOOFFSET(+26),
	0x32, PATCH_UINT16(0x0001),         // jmp 0001
	PATCH_ADDTOOFFSET(+4),
	0x4a, 0x14,                         // send 14 [ init deskLamp without addToPic ]
	PATCH_ADDTOOFFSET(+27),
	0x32, PATCH_UINT16(0x0001),         // jmp 0001
	PATCH_ADDTOOFFSET(+18),
	0x4a, 0x12,                         // send 12 [ init wasteBasket without addToPic ]
	PATCH_END
};

// LB2 Floppy 1.0 doesn't initialize act 4 correctly when triggered by finding
//  the dagger, causing the act 4 scene in Yvette's (room 550) to lockup.
//
// The Yvette/Olympia/Steve scene in act 4 (rooms 550 and 510) expects
//  global[111] to be 11. This global tracks Yvette's state throughout acts 3
//  and 4, incrementing as you listen to her conversations and witness her
//  scenes. Some of these are optional, so at the end of act 3 it can be less
//  than 11. rm510:init initializes global[111] to 11 when act 4 is triggered
//  by reporting Ernie's death, but no such initialization occurs when act 4 is
//  triggered by finding the dagger (rooms 610 and 620). What happens when the
//  global isn't 11 depends on its value. Some values, such as 8, cause the act
//  4 scene to never complete and never restore control to the user.
//
// We fix this the way Sierra did in floppy 1.1 and cd versions by setting
//  global[111] to 11 in actBreak:init when act 4 starts so that it's always
//  initialized.
//
// Applies to: Floppy 1.0 English only
// Responsible method: actBreak:<noname150> which is really init
// Fixes bug: #10716
static const uint16 laurabow2SignatureFixAct4Initialization[] = {
	SIG_MAGICDWORD,
	0xa3, 0x08,                         // sal local[8]   [ 1.0 floppy only ]
	0x89, 0x0c,                         // lsg global[0c] [ previous room ]
	0x34, SIG_UINT16(0x026c),           // ldi 026c       [ room 620 ]
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt 5
	0x34, SIG_UINT16(0x0262),           // ldi 0262 [ room 610 ]
	0x33, 0x03,                         // jmp 3
	0x34, SIG_UINT16(0x01fe),           // ldi 01fe [ room 510 ]
	0xa3, 0x00,                         // sal local[0]   [ (previous room == 620) ? 610 : 510 ]
	0x33, 0x2d,                         // jmp 2d   [ exit switch ]
	SIG_END
};

static const uint16 laurabow2PatchFixAct4Initialization[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x0b,                         // ldi 0b
	0xa1, 0x6f,                         // sag global[6f] [ global[111d] = 11 ]
	0x89, 0x0c,                         // lsg global[0c] [ previous room ]
	0x34, PATCH_UINT16(0x026c),         // ldi 026c       [ room 620 ]
	0x1a,                               // eq?
	0x39, 0x64,                         // pushi 64
	0x06,                               // mul
	0x38, PATCH_UINT16(0x01fe),         // pushi 01fe
	0x02,                               // add      [ acc = ((previous room == 620) * 100) + 510 ]
	0x32, PATCH_UINT16(0x0013),         // jmp 0013 [ jmp to: sal 0, jmp exit switch ]
	PATCH_END
};

// The armor exhibit rooms (440 and 448) have event handlers that fail to handle
//  all events, preventing messages from being displayed.
//
// Both armor rooms implement handleEvent to handle joystick events in certain
//  situations, but they only pass move events on to super:handleEvent, blocking
//  all other event types. Clicking on either room does nothing even though both
//  have messages to respond with. This also prevents messages when clicking on
//  Pippin Carter's armor inset. Sierra fixed the armor problem after the first
//  floppy release by adding code to detect and handle the inset's events,
//  instead of handling all events, leaving the room messages broken.
//
// We fix this by handling verb events in both rooms. This fixes room messages
//  in all versions and fixes armor inset messages in English floppy 1.0.
//
// Applies to: All Floppy and CD versions
// Responsible methods: rm440:handleEvent/<noname133>, rm448:handleEvent/<noname133>
// Fixes bugs #10709, #10895
static const uint16 laurabow2SignatureHandleArmorRoomEvents[] = {
	0x87, 0x01,                         // lap 01
	0x4a, 0x04,                         // send 04 [ event type? ]
	SIG_MAGICDWORD,
	0x36,                               // push
	0x34, SIG_UINT16(0x1000),           // ldi 1000 [ move event ]
	0x12,                               // and
	0x31,                               // bnt [ don't handle event ]
	SIG_END
};

static const uint16 laurabow2PatchHandleArmorRoomEvents[] = {
	PATCH_ADDTOOFFSET(+5),
	0x34, PATCH_UINT16(0x5000),         // ldi 5000 [ move event | verb event ]
	PATCH_END
};

// The "bugs with meat" in the basement hallway (room 600) can lockup the game
//  if they appear while ego is leaving the room through one of the doors.
//
// bugsWithMeat cues after 5 seconds in the room and runs sDoMeat if no room
//  script is set. sDoMeat:changeState(0) calls handsOff. Ego might already be
//  leaving through the north door in handsOff mode, which is managed by lRS
//  (Leave Room Script), which doesn't prevent sDoMeat from running because lRS
//  isn't set as the room script. If the door is animating when the timer goes
//  off then ego will continue to Wolfe's (room 650) and the unexpected second
//  handsOff will cause handsOn(1) to restore the disabled state and the user
//  will never regain control. If sDoMeat runs after the door animates then
//  ego's movement will be interrupted and the door will be left open and broken.
//  Similar problems occur with the other door in the room.
//
// We fix this by patching bugsWithMeat:cue from testing if the room has no
//  script to instead testing if the user has control before running sDoMeat.
//  All of the room's scripts call handsOff in state 0 and handsOn in their
//  final state so this change just extends the interruption test to include
//  other handsOff scripts.
//
// The signature and patch are duplicated for floppy and cd versions due to
//  User:canControl having different selector values between versions, floppy
//  versions not including selector names, and User:canControl's selector
//  values not appearing in the script being patched.
//
// Applies to: All Floppy and CD versions
// Responsible method: bugsWithMeat:cue/<noname145>
// Fixes bug: #10730
static const uint16 laurabow2FloppySignatureFixBugsWithMeat[] = {
	SIG_MAGICDWORD,
	0x57, 0x32, 0x06,                   // super Actor[32], 6 [ floppy: 32, cd: 31 ]
	0x3a,                               // toss
	0x48,                               // ret [ end of bugsWithMeat:<noname300> aka doVerb ]
	0x38, SIG_UINT16(0x008e),           // pushi 008e [ <noname142> aka script ]
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] [ rm600 ]
	0x4a, 0x04,                         // send 4
	0x31, 0x0e,                         // bnt 0e [ run sDoMeat if not rm600:<noname142>? ]
	SIG_END
};

static const uint16 laurabow2FloppyPatchFixBugsWithMeat[] = {
	PATCH_ADDTOOFFSET(+5),
	0x38, PATCH_UINT16(0x00ed),         // pushi 00ed [ <noname237> aka canControl ]
	0x76,                               // push0
	0x81, 0x50,                         // lag global[50] [ User ]
	0x4a, 0x04,                         // send 4
	0x2f, 0x0e,                         // bt 0e [ run sDoMeat if User:<noname237>? ]
	PATCH_END
};

// cd version of the above signature/patch
static const uint16 laurabow2CDSignatureFixBugsWithMeat[] = {
	SIG_MAGICDWORD,
	0x57, 0x31, 0x06,                   // super Actor[31], 6 [ floppy: 32, cd: 31 ]
	0x3a,                               // toss
	0x48,                               // ret [ end of bugsWithMeat:doVerb ]
	0x38, SIG_UINT16(0x008e),           // pushi 008e [ script ]
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] [ rm600 ]
	0x4a, 0x04,                         // send 4
	0x31, 0x0e,                         // bnt 0e [ run sDoMeat if not rm600:script? ]
	SIG_END
};

static const uint16 laurabow2CDPatchFixBugsWithMeat[] = {
	PATCH_ADDTOOFFSET(+5),
	0x38, PATCH_UINT16(0x00f6),         // pushi 00f6 [ canControl ]
	0x76,                               // push0
	0x81, 0x50,                         // lag global[50] [ User ]
	0x4a, 0x04,                         // send 4
	0x2f, 0x0e,                         // bt 0e [ run sDoMeat if User:canControl? ]
	PATCH_END
};

// LB2 CD ends act 5 in the middle of the finale music instead of waiting for
//  it to complete. This is a script bug and occurs in Sierra's interpreter.
//
// When catching the killer in room 480, sWrapMusic is used to play the chomp
//  sound followed by the finale music. sWrapMusic uses localSound to play both
//  resources. sOrileyCaught:doit waits for the music to complete by testing
//  localSound:prevSignal for -1 before proceeding to act 6. This worked in
//  floppy versions.
//
// The problem is that CD versions include a newer Sound class with different
//  behavior. This new Sound:play doesn't call Sound:init on subsequent plays.
//  Sound:init is what initializes prevSignal to 0, and so localSound:prevSignal
//  is no longer re-initialized from -1 to 0 after playing the chomp, causing
//  sOrileyCaught:doit to treat the music as having immediately completed.
//
// We fix this by changing sOrileyCaught:doit to instead test localSound:handle
//  to determine if the music has completed. Sound:handle is always set when
//  playing and cleared when stopped or disposed.
//
// Applies to: All CD versions
// Responsible method: sOrileyCaught:doit
// Fixes bug: #10808
static const uint16 laurabow2CDSignatureFixAct5FinaleMusic[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00ab),           // pushi 00ab [ prevSignal ]
	0x76,                               // push0
	0x72, SIG_UINT16(0x083a),           // lofsa localSound
	0x4a, 0x04,                         // send 4
	0x36,                               // push
	0x35, 0xff,                         // ldi ff
	SIG_END
};

static const uint16 laurabow2CDPatchFixAct5FinaleMusic[] = {
	0x38, PATCH_SELECTOR16(handle),     // pushi handle
	PATCH_ADDTOOFFSET(+7),
	0x35, 0x00,                         // ldi 00
	PATCH_END
};

// LB2 does a speed test during startup (room 28) to determine the initial
//  detail level and to use for pacing later scenes. Even with the highest
//  score the detail level is only set to medium instead of highest like
//  other SCI games.
//
// Platforms such as iOS can introduce a lag during game initialization that
//  causes the speed test to occasionally get the lowest score, causing
//  detail to be initialized to lowest and subsequent scenes such as the act 5
//  chase scene to go very slow.
//
// We patch startGame:doit to ignore the score and always set the highest detail
//  level and cpu speed so that detail needn't be manually increased and act 5
//  behaves consistently. This also helps touchscreen devices as the game's
//  detail slider is prohibitively difficult to manually set to highest without
//  switching from the default touch mode.
//
// Applies to: All Floppy and CD versions
// Responsible method: startGame:doit/<noname57>
// Fixes bug: #10761
static const uint16 laurabow2SignatureDisableSpeedTest[] = {
	0x89, 0x57,                         // lsg global[57] [ speed test result ]
	SIG_MAGICDWORD,
	0x35, 0x03,                         // ldi 03 [ low-speed threshold ]
	0x24,                               // le?
	0x31, 0x04,                         // bnt 04
	0x35, 0x01,                         // ldi 01 [ lowest detail ]
	0x33, 0x0d,                         // jmp 0d
	0x89, 0x57,                         // lsg global[57] [ speed test result ]
	SIG_END
};

static const uint16 laurabow2PatchDisableSpeedTest[] = {
	0x38, PATCH_UINT16(0x0005),         // pushi 0005
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x06,                         // send 06 [ LB2:detailLevel = 5, max detail ]
	0x35, 0x0f,                         // ldi 0f
	0xa1, 0x57,                         // sag global[57] [ global[57] = f, max cpu speed ]
	0x33, 0x10,                         // jmp 10 [ continue init ]
	PATCH_END
};

// LB2CD reduces the music volume significantly during the introduction when
//  characters talk while disembarking the ship in room 120. This is done so
//  that their speech can be heard but it also occurs in text-only mode.
//
// Interestingly, this is the only script that manually reduces volume during
//  speech, as it's a workaround for bugs in the Narrator and Talker scripts.
//  They're supposed to automatically reduce music volume, but this scheme fails
//  when multiple appear at once, which seems to only occur in the introduction.
//
// We patch the introduction to skip the volume reduction when in text-only mode
//  so that the music doesn't abruptly go away and come back.
//
// Applies to: All CD versions
// Responsible method: sDisembark:changeState
// Fixes bug #10916
static const uint16 laurabow2CDSignatureIntroVolumeChange[] = {
	0x31, 0x2a,                         // bnt 2a [ state 3 ]
	SIG_ADDTOOFFSET(+2),
	0x31, 0x1f,                         // bnt 1f
	SIG_ADDTOOFFSET(+28),
	0x32, SIG_UINT16(0x00f7),           // jmp 00f7 [ end of method ]
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles [ cycles = 2 ]
	0x32, SIG_UINT16(0x00f0),           // jmp 00f0 [ end of method ]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x25,                         // bnt 25 [ state 4 ]
	0x38, SIG_SELECTOR16(setVol),       // pushi setVol
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x28,                         // pushi 28
	0x81, 0x66,                         // lag 66
	0x4a, 0x06,                         // send 06 [ gameMusic1 setVol: 40 ]
	SIG_END
};

static const uint16 laurabow2CDPatchIntroVolumeChange[] = {
	0x31, 0x25,                         // bnt 25 [ state 3 ]
	PATCH_ADDTOOFFSET(+2),
	0x31, 0x1e,                         // bnt 1e
	PATCH_ADDTOOFFSET(+28),
	0x3a,                               // toss
	0x48,                               // ret
	0x7a,                               // push2
	0x69, 0x1a,                         // sTop cycles [ cycles = 2 ]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x2a,                         // bnt 2a [ state 4 ]
	0x89, 0x5a,                         // lsg 5a [ message mode ]
	0x1a,                               // eq?    [ is text-only mode? ]
	0x2f, 0x0a,                         // bt 0a  [ skip volume reduction ]
	PATCH_END
};

// Laura Bow 2 CD resets the audio mode to speech on init/restart
//  We already sync the settings from ScummVM (see SciEngine::syncIngameAudioOptions())
//  and this script code would make it impossible to see the intro using "dual" mode w/o using debugger command
//  That's why we remove the corresponding code
// Patched method: LB2::init, rm100::init
static const uint16 laurabow2CDSignatureAudioTextSupportModeReset[] = {
	SIG_MAGICDWORD,
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextSupportModeReset[] = {
	0x34, PATCH_UINT16(0x0001),         // ldi 0001 (waste bytes)
	0x18,                               // not (waste bytes)
	PATCH_END
};

// Directly use global[5a] for view-cel id
//  That way it's possible to use a new "dual" mode view in the game menu
// View 995, loop 13, cel 0 -> "text"
// View 995, loop 13, cel 1 -> "speech"
// View 995, loop 13, cel 2 -> "dual" (this view is injected by us into the game)
// Patched method: gcWin::open
static const uint16 laurabow2CDSignatureAudioTextMenuSupport1[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x36,                               // push
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextMenuSupport1[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x01,                         // ldi 01
	0x04,                               // sub
	PATCH_END
};

//  Adds another button state for the text/audio button. We currently use the "speech" view for "dual" mode.
// Patched method: iconMode::doit
static const uint16 laurabow2CDSignatureAudioTextMenuSupport2[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x3c,                               // dup
	0x1a,                               // eq?
	0x31, 0x0a,                         // bnt [set text mode]
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	0x35, 0x01,                         // ldi 01
	0xa5, 0x00,                         // sat temp[0]
	0x33, 0x0e,                         // jmp [draw cel code]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x08,                         // bnt [draw cel code]
	0x35, 0x01,                         // ldi 01
	0xa1, 0x5a,                         // sag global[5a]
	0x35, 0x00,                         // ldi 00
	0xa5, 0x00,                         // sat temp[0]
	0x3a,                               // toss
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextMenuSupport2[] = {
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x02,                               // add
	0xa1, 0x5a,                         // sag global[5a]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x1e,                               // gt?
	0x31, 0x03,                         // bnt [skip over]
	0x78,                               // push1
	0xa9, 0x5a,                         // ssg global[5a]
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x04,                               // sub
	0xa5, 0x00,                         // sat temp[0] [ calculate global[5a] - 1 to use as view cel id ]
	0x33, 0x07,                         // jmp [draw cel code, don't do toss]
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry laurabow2Signatures[] = {
	{  true,   560, "CD: painting closing immediately",               1, laurabow2CDSignaturePaintingClosing,            laurabow2CDPatchPaintingClosing },
	{  true,     0, "CD: fix problematic icon bar",                   1, laurabow2CDSignatureFixProblematicIconBar,      laurabow2CDPatchFixProblematicIconBar },
	{  true,    90, "CD: fix yvette's tut response",                  1, laurabow2CDSignatureFixYvetteTutResponse,       laurabow2CDPatchFixYvetteTutResponse },
	{  true,   350, "CD/Floppy: museum party fix entering south 1/2", 1, laurabow2SignatureMuseumPartyFixEnteringSouth1, laurabow2PatchMuseumPartyFixEnteringSouth1 },
	{  true,   350, "CD/Floppy: museum party fix entering south 2/2", 1, laurabow2SignatureMuseumPartyFixEnteringSouth2, laurabow2PatchMuseumPartyFixEnteringSouth2 },
	{  true,   430, "CD/Floppy: make wired east door persistent",     1, laurabow2SignatureRememberWiredEastDoor,        laurabow2PatchRememberWiredEastDoor },
	{  true,   430, "CD/Floppy: fix wired east door",                 1, laurabow2SignatureFixWiredEastDoor,             laurabow2PatchFixWiredEastDoor },
	{  true,   448, "CD/Floppy: fix armor hall door pathfinding",     1, laurabow2SignatureFixArmorHallDoorPathfinding,  laurabow2PatchFixArmorHallDoorPathfinding },
	{  true,   460, "CD/Floppy: fix crate room east door lockup",     1, laurabow2SignatureFixCrateRoomEastDoorLockup,   laurabow2PatchFixCrateRoomEastDoorLockup },
	{  true,  2660, "CD/Floppy: fix elevator lockup",                 1, laurabow2SignatureFixElevatorLockup,            laurabow2PatchFixElevatorLockup },
	{  true,   550, "CD/Floppy: fix back rub east entrance lockup",   1, laurabow2SignatureFixBackRubEastEntranceLockup, laurabow2PatchFixBackRubEastEntranceLockup },
	{  true,   550, "CD/Floppy: fix disappearing desk items",         1, laurabow2SignatureFixDisappearingDeskItems,     laurabow2PatchFixDisappearingDeskItems },
	{  true,    26, "Floppy: fix act 4 initialization",               1, laurabow2SignatureFixAct4Initialization,        laurabow2PatchFixAct4Initialization },
	{  true,   440, "CD/Floppy: handle armor room events",            1, laurabow2SignatureHandleArmorRoomEvents,        laurabow2PatchHandleArmorRoomEvents },
	{  true,   448, "CD/Floppy: handle armor hall room events",       1, laurabow2SignatureHandleArmorRoomEvents,        laurabow2PatchHandleArmorRoomEvents },
	{  true,   600, "Floppy: fix bugs with meat",                     1, laurabow2FloppySignatureFixBugsWithMeat,        laurabow2FloppyPatchFixBugsWithMeat },
	{  true,   600, "CD: fix bugs with meat",                         1, laurabow2CDSignatureFixBugsWithMeat,            laurabow2CDPatchFixBugsWithMeat },
	{  true,   480, "CD: fix act 5 finale music",                     1, laurabow2CDSignatureFixAct5FinaleMusic,         laurabow2CDPatchFixAct5FinaleMusic },
	{  true,    28, "CD/Floppy: disable speed test",                  1, laurabow2SignatureDisableSpeedTest,             laurabow2PatchDisableSpeedTest },
	{  true,   120, "CD: disable intro volume change in text mode",   1, laurabow2CDSignatureIntroVolumeChange,          laurabow2CDPatchIntroVolumeChange },
	{  true,   928, "Narrator lockup fix",                            1, sciNarratorLockupSignature,                     sciNarratorLockupPatch },
	// King's Quest 6 and Laura Bow 2 share basic patches for audio + text support
	{ false,   924, "CD: audio + text support 1",                     1, kq6laurabow2CDSignatureAudioTextSupport1,       kq6laurabow2CDPatchAudioTextSupport1 },
	{ false,   924, "CD: audio + text support 2",                     1, kq6laurabow2CDSignatureAudioTextSupport2,       kq6laurabow2CDPatchAudioTextSupport2 },
	{ false,   924, "CD: audio + text support 3",                     1, kq6laurabow2CDSignatureAudioTextSupport3,       kq6laurabow2CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support 4",                     1, kq6laurabow2CDSignatureAudioTextSupport4,       kq6laurabow2CDPatchAudioTextSupport4 },
	{ false,   928, "CD: audio + text support 5",                     2, kq6laurabow2CDSignatureAudioTextSupport5,       kq6laurabow2CDPatchAudioTextSupport5 },
	{ false,     0, "CD: audio + text support disable mode reset",    1, laurabow2CDSignatureAudioTextSupportModeReset,  laurabow2CDPatchAudioTextSupportModeReset },
	{ false,   100, "CD: audio + text support disable mode reset",    1, laurabow2CDSignatureAudioTextSupportModeReset,  laurabow2CDPatchAudioTextSupportModeReset },
	{ false,    24, "CD: audio + text support LB2 menu 1",            1, laurabow2CDSignatureAudioTextMenuSupport1,      laurabow2CDPatchAudioTextMenuSupport1 },
	{ false,    24, "CD: audio + text support LB2 menu 2",            1, laurabow2CDSignatureAudioTextMenuSupport2,      laurabow2CDPatchAudioTextMenuSupport2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Mother Goose SCI1/SCI1.1
// MG::replay somewhat calculates the savedgame-id used when saving again
//  this doesn't work right and we remove the code completely.
//  We set the savedgame-id directly right after restoring in kRestoreGame.
//  We also draw the background picture in here instead.
//  This Mixed Up Mother Goose draws the background picture before restoring,
//  instead of doing it properly in MG::replay. This fixes graphic issues,
//  when restoring from GMM.
//
// Applies to at least: English SCI1 CD, English SCI1.1 floppy, Japanese FM-Towns
// Responsible method: MG::replay (script 0)
static const uint16 mothergoose256SignatureReplay[] = {
	0x7a,                            // push2
	0x78,                            // push1
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x43, 0x70, 0x04,                // callk MemorySegment
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x76,                            // push0
	0x43, 0x62, 0x04,                // callk StrAt
	0xa1, 0xaa,                      // sag global[AAh]
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x78,                            // push1
	0x43, 0x62, 0x04,                // callk StrAt
	0x36,                            // push
	0x35, 0x20,                      // ldi 20
	0x04,                            // sub
	0xa1, SIG_ADDTOOFFSET(+1),       // sag global[57h], for FM-Towns: sag global[9Dh]
	// 35 bytes
	0x39, 0x03,                      // pushi 03
	0x89, SIG_ADDTOOFFSET(+1),       // lsg global[1Dh], for FM-Towns: lsg global[1Eh]
	0x76,                            // push0
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x7a,                            // push2
	0x43, 0x62, 0x04,                // callk StrAt
	0x36,                            // push
	0x35, 0x01,                      // ldi 01
	0x04,                            // sub
	0x36,                            // push
	0x43, 0x62, 0x06,                // callk StrAt
	// 22 bytes
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x39, 0x03,                      // pushi 03
	0x43, 0x62, 0x04,                // callk StrAt
	// 10 bytes
	0x36,                            // push
	0x35, SIG_MAGICDWORD, 0x20,      // ldi 20
	0x04,                            // sub
	0xa1, 0xb3,                      // sag global[B3h]
	// 6 bytes
	SIG_END
};

static const uint16 mothergoose256PatchReplay[] = {
	0x39, 0x06,                      // pushi 06
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(200),         // pushi 200d
	0x38, PATCH_UINT16(320),         // pushi 320d
	0x76,                            // push0
	0x76,                            // push0
	0x43, 0x15, 0x0c,                // callk SetPort -> set picture port to full screen
	// 15 bytes
	0x39, 0x04,                      // pushi 04
	0x3c,                            // dup
	0x76,                            // push0
	0x38, PATCH_UINT16(255),         // pushi 255d
	0x76,                            // push0
	0x43, 0x6f, 0x08,                // callk Palette -> set intensity to 0 for all colors
	// 11 bytes
	0x7a,                            // push2
	0x38, PATCH_UINT16(800),         // pushi 800
	0x76,                            // push0
	0x43, 0x08, 0x04,                // callk DrawPic -> draw picture 800
	// 8 bytes
	0x39, 0x06,                      // pushi 06
	0x39, 0x0c,                      // pushi 0Ch
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(200),         // pushi 200
	0x38, PATCH_UINT16(320),         // pushi 320
	0x78,                            // push1
	0x43, 0x6c, 0x0c,                // callk Graph -> send everything to screen
	// 16 bytes
	0x39, 0x06,                      // pushi 06
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(156),         // pushi 156d
	0x38, PATCH_UINT16(258),         // pushi 258d
	0x39, 0x03,                      // pushi 03
	0x39, 0x04,                      // pushi 04
	0x43, 0x15, 0x0c,                // callk SetPort -> set picture port back
	// 17 bytes
	0x34, PATCH_UINT16(0x0000),      // ldi 0000 (dummy)
	0x34, PATCH_UINT16(0x0000),      // ldi 0000 (dummy)
	PATCH_END
};

// when saving, it also checks if the savegame ID is below 13.
//  we change this to check if below 113 instead
//
// Applies to at least: English SCI1 CD, English SCI1.1 floppy, Japanese FM-Towns
// Responsible method: Game::save (script 994 for SCI1), MG::save (script 0 for SCI1.1)
static const uint16 mothergoose256SignatureSaveLimit[] = {
	0x89, SIG_MAGICDWORD, 0xb3,      // lsg global[b3]
	0x35, 0x0d,                      // ldi 0d
	0x20,                            // ge?
	SIG_END
};

static const uint16 mothergoose256PatchSaveLimit[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x0d + SAVEGAMEID_OFFICIALRANGE_START, // ldi 113d
	PATCH_END
};

// Clicking a button on the main menu as the screen fades in causes a message to
//  be sent to a non-object when coming from the Sierra logo. This also crashes
//  the original. If returning from a menu then the previous button is clicked.
//
// After the main menu screen fades in, IconBar:doit polls for events in a loop
//  until a mouse click or key press occurs over a button. choices:show then
//  calls highlightedIcon:message to run the correct action. IconBar:doit
//  updates this property on each iteration unless the event is a mouse click.
//  If a button is clicked before polling begins then IconBar:doit exits on its
//  first iteration without setting highlightedIcon. This property is never
//  reset and its stale value can get reused when returning from a menu.
//
// These problems would be simple to fix in the event polling loop, but that
//  code is in the IconBar and GameControls classes that affect each screen.
//  Instead we surround the loop with fixes. The event queue is now drained of
//  mouse clicks and highlightedIcon is cleared before polling. Afterwards,
//  highlightedIcon is verified and the loop is repeated if it's not set. This
//  discards clicks during fade-in, resets state when canceling a subsequent
//  menu, and prevents timing edge cases from crashing should a click ever
//  register on the loop's first iteration. We make room for this by overwriting
//  the code that initializes the cursor when kHaveMouse reports no mouse, as
//  that doesn't apply to ScummVM. The CD version's menu was rewritten and
//  doesn't have these problems.
//
// Applies to: English PC Floppy
// Responsible method: choices:show
// Fixes bug #9681
static const uint16 mothergoose256SignatureMainMenuCrash[] = {
	0x43, SIG_MAGICDWORD, 0x27, 0x00,   // callk HaveMouse
	0x31, 0x0e,                         // bnt 0e [ no mouse ]
	SIG_ADDTOOFFSET(+0x0b),
	0x32, SIG_UINT16(0x0036),           // jmp 0036 [ skip no-mouse code ]
	SIG_ADDTOOFFSET(+0x1a),
	// no mouse
	0x76,                               // push0
	0x63, 0x1e,                         // pToa curIcon
	0x4a, 0x04,                         // send 04
	0x04,                               // sub
	0x36,                               // push
	0x35, 0x02,                         // ldi 02
	0x08,                               // div
	0x02,                               // add
	0x36,                               // push
	0x39, 0x08,                         // pushi 08
	0x76,                               // push0
	0x63, 0x1e,                         // pToa curIcon
	0x4a, 0x04,                         // send 04
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x04,                               // sub
	// event polling loop
	0x36,                               // push
	0x81, 0x01,                         // lag 01
	0x4a, 0x0c,                         // send 0c
	0x39, SIG_SELECTOR8(doit),          // pushi doit
	0x76,                               // push0
	0x39, SIG_SELECTOR8(hide),          // pushi hide
	0x76,                               // push0
	0x54, 0x08,                         // self 08 [ self doit: hide: ]
	SIG_END
};

static const uint16 mothergoose256PatchMainMenuCrash[] = {
	PATCH_ADDTOOFFSET(+3),
	0x33, 0x00,                         // jmp 00 [ always run mouse code ]
	PATCH_ADDTOOFFSET(+0x0b),
	0x32, PATCH_UINT16(0x001a),         // jmp 001a [ skip no-mouse code ]
	PATCH_ADDTOOFFSET(+0x1a),
	0x76,                               // push0
	0x69, 0x20,                         // sTop highlightedIcon [ highlightedIcon = 0 ]
	0x39, PATCH_SELECTOR8(new),         // pushi new
	0x78,                               // push1
	0x39, 0x03,                         // pushi 03 [ mouse down/up ]
	0x51, 0x07,                         // class Event
	0x4a, 0x06,                         // send 06 [ Event new: 3 ]
	0x39, PATCH_SELECTOR8(dispose),     // pushi dispose
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(type),        // pushi type
	0x76,                               // push0
	0x4a, 0x08,                         // send 08 [ event dispose: type? ]
	0x2f, 0xed,                         // bt ed [ loop until no more mouse down/up events ]
	0x39, PATCH_SELECTOR8(doit),        // pushi doit
	0x76,                               // push0
	0x54, 0x04,                         // self 04 [ self doit: ]
	0x63, 0x20,                         // pToa highlightedIcon
	0x31, 0xf7,                         // bnt f7 [ repeat event loop until highlightedIcon is set ]
	PATCH_ADDTOOFFSET(+3),
	0x54, 0x04,                         // self 04 [ self hide: ]
	PATCH_END
};

//          script, description,                                      signature                             patch
static const SciScriptPatcherEntry mothergoose256Signatures[] = {
	{  true,     0, "replay save issue",                           1, mothergoose256SignatureReplay,        mothergoose256PatchReplay },
	{  true,     0, "save limit dialog (SCI1.1)",                  1, mothergoose256SignatureSaveLimit,     mothergoose256PatchSaveLimit },
	{  true,   994, "save limit dialog (SCI1)",                    1, mothergoose256SignatureSaveLimit,     mothergoose256PatchSaveLimit },
	{  true,    90, "main menu button crash",                      1, mothergoose256SignatureMainMenuCrash, mothergoose256PatchMainMenuCrash },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Mixed-up Mother Goose Deluxe

// The game uses pic 10005 to render the Sierra logo, but then it also
// initialises a logo object with view 502 on the same priority as the pic. In
// the original interpreter, it is dumb luck which is drawn first (based on the
// order of the memory IDs), though usually the pic is drawn first because not
// many objects have been created at the start of the game. In ScummVM, the
// renderer guarantees a sort order based on the creation order of screen items,
// and since the view is created after the pic, it wins and is drawn on top.
// This patch stops the view object from being created at all.
//
// Applies to at least: English CD from King's Quest Collection
// Responsible method: sShowLogo::changeState
static const uint16 mothergooseHiresLogoSignature[] = {
	0x38, SIG_SELECTOR16(init),      // pushi init ($8e)
	SIG_MAGICDWORD,
	0x76,                            // push0
	0x72, SIG_UINT16(0x0082),        // lofsa logo[82]
	0x4a, SIG_UINT16(0x0004),        // send $4
	SIG_END
};

static const uint16 mothergooseHiresLogoPatch[] = {
	0x33, 0x08, // jmp [past bad logo init]
	PATCH_END
};

// After finishing the rhyme at the fountain, a horse will appear and walk
// across the screen. The priority of the horse is set too high, so it is
// rendered in front of the fountain instead of behind the fountain. This patch
// corrects the priority so the horse draws behind the fountain.
//
// Applies to at least: English CD from King's Quest Collection
// Responsible method: rhymeScript::changeState
static const uint16 mothergooseHiresHorseSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(setPri), // pushi setPri ($4a)
	0x78,                        // push1
	0x38, SIG_UINT16(0x00b7),    // pushi $b7
	SIG_END
};

static const uint16 mothergooseHiresHorsePatch[] = {
	PATCH_ADDTOOFFSET(+3),    // pushi setPri, push1
	0x38, PATCH_UINT16(0x59), // pushi $59
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry mothergooseHiresSignatures[] = {
	{  true,     0, "disable volume reset on startup (1/2)",       2, sci2VolumeResetSignature,         sci2VolumeResetPatch },
	{  true,    90, "disable volume reset on startup (2/2)",       1, sci2VolumeResetSignature,         sci2VolumeResetPatch },
	{  true,   108, "fix bad logo rendering",                      1, mothergooseHiresLogoSignature,    mothergooseHiresLogoPatch },
	{  true,   318, "fix bad horse z-index",                       1, mothergooseHiresHorseSignature,   mothergooseHiresHorsePatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Phantasmagoria

// Phantasmagoria persists audio volumes in the save games, but ScummVM manages
// game volumes through the launcher, so stop the game from overwriting the
// ScummVM volumes with volumes from save games.
// Applies to at least: English CD
// Fixes bug: #9700
static const uint16 phant1SavedVolumeSignature[] = {
	0x7a,                           // push2
	0x39, 0x08,                     // pushi 8
	0x38, SIG_SELECTOR16(readWord), // pushi readWord ($20b)
	0x76,                           // push0
	0x72, SIG_UINT16(0x13c),        // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),         // send 4
	SIG_MAGICDWORD,
	0xa1, 0xbc,                     // sag global[$bc]
	0x36,                           // push
	0x43, 0x76, SIG_UINT16(0x04),   // callk DoAudio[76], 4
	0x7a,                           // push2
	0x76,                           // push0
	0x38, SIG_SELECTOR16(readWord), // pushi readWord ($20b)
	0x76,                           // push0
	0x72, SIG_UINT16(0x13c),        // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),         // send 4
	0xa1, 0xbb,                     // sag global[$bb]
	0x36,                           // push
	0x43, 0x75, SIG_UINT16(0x04),   // callk DoSound[75], 4
	SIG_END
};

static const uint16 phant1SavedVolumePatch[] = {
	0x32, PATCH_UINT16(0x0024),     // jmp [skip the whole sig, to prefFile::close]
	PATCH_END
};

// Phantasmagoria performs an incomplete initialisation of a rat view when
// exiting the alcove in the basement any time after chapter 3 when flag 26 is
// not set. This causes the rat view to be rendered with the same origin and
// priority as the background picture for the game plane, triggering last ditch
// sorting of the screen items in the renderer. This happens to work OK most of
// the time in SSCI because the last ditch sort uses memory handle indexes, and
// the index of the rat seems to usually end up below the index for the
// background pic, so the rat's screen item is submitted before the background,
// ensuring that the background palette overrides the rat view's palette. In
// ScummVM, last ditch sorting operates using the creation order of screen
// items, so the rat ends up always sorting above the background, which causes
// the background palette to get replaced by the rat palette, which corrupts the
// background. This patch stops the game script from initialising the bad rat
// view entirely.
// Applies to at least: English CD, French CD
// Fixes bug: #9957
static const uint16 phant1RatSignature[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x39, 0x1a,                         // pushi $1a
	0x45, 0x03, SIG_UINT16(0x0002),     // callb [export 3 of script 0], 02
	0x18,                               // not
	0x31, 0x18,                         // bnt $18
	SIG_END
};

static const uint16 phant1RatPatch[] = {
	0x33, 0x20, // jmp [past rat condition + call]
	PATCH_END
};

// In Phantasmagoria the cursor's hover state will not trigger on any of the
// buttons in the main menu after returning to the main menu from a game, or
// when choosing "Quit" on the main menu and then cancelling the quit in the
// confirmation dialogue, until another button has been hovered and unhovered
// once.
// This happens because the quit confirmation dialogue creates its own
// event handling loop which prevents the main event loop from handling the
// cursor leaving the button (which would reset global[193] to 0), and the
// dialogue does not reset global[193] itself, so it remains at 2 until a new
// button gets hovered and unhovered.
// There is not enough space in the confirmation dialogue code to add a reset
// of global[193], so we just remove the check entirely, since it is only used
// to avoid resetting the cursor's view on every mouse movement, and this
// button type is only used on the main menu and the in-game control panel.
//
// Applies to at least: English CD
// Fixes bug: #9977
static const uint16 phant1RedQuitCursorSignature[] = {
	SIG_MAGICDWORD,
	0x89, 0xc1,                   // lsg global[193]
	0x35, 0x02,                   // ldi 02
	SIG_END
};

static const uint16 phant1RedQuitCursorPatch[] = {
	0x33, 0x05,                   // jmp [past global[193] check]
	PATCH_END
};

// In chapter 5, the wine casks in room 20200 shimmer to indicate that they have
//  a video to play but a script bug usually prevents them from being clicked.
//  The code that determines whether to set a hotspot on the "spikot" object
//  is incomplete and out of sync with the code that displays the shimmer and
//  determines which script to run when the casks are clicked. As a result, the
//  shimmering casks can't be clicked in chapter 5 if their contents were tasted
//  in an earlier act. In chapter 6 the shimmering casks can be clicked again.
//
// We fix this by rewriting the logic that sets the hotspot to match the rest of
//  the code in the room. The casks can now always be clicked when they shimmer.
//  To make room we remove a call to approachVerbs: 0 as that has no effect.
//
// Applies to: All versions
// Responsible method: rm20200:init
static const uint16 phant1WineCaskHotspotSignature[] = {
	0x89, 0x6a,                           // lsg 6a
	0x35, 0x06,                           // ldi 06
	0x1a,                                 // eq? [ is chapter 6? ]
	0x31, 0x09,                           // bnt 09
	SIG_MAGICDWORD,
	0x78,                                 // push1
	0x38, SIG_UINT16(0x00d8),             // pushi 00d8 [ flag 216 ]
	0x45, 0x03, SIG_UINT16(0x0002),       // callb proc0_3 [ seen barrel video? ]
	0x18,                                 // not
	0x2f, 0x0f,                           // bt 0f
	0x89, 0x6a,                           // lsg 6a 
	0x35, 0x06,                           // ldi 06
	0x1c,                                 // ne? [ is not chapter 6? ]
	0x31, 0x21,                           // bnt 21 [ skip hotspot ]
	0x78,                                 // push1
	0x39, 0x1d,                           // pushi 1d [ flag 29 ]
	0x45, 0x03, SIG_UINT16(0x0002),       // callb proc0_3 [ tasted wine? ]
	0x18,                                 // not
	0x31, 0x17,                           // bnt 17 [ skip hotspot ]
	0x38, SIG_SELECTOR16(init),           // pushi init
	0x76,                                 // push0
	0x38, SIG_SELECTOR16(approachVerbs),  // pushi approachVerbs
	0x78,                                 // push1
	0x76,                                 // push0
	SIG_ADDTOOFFSET(+11),
	0x4a, SIG_UINT16(0x0012),             // send 12 [ spikot init: approachVerbs: 0 ... ]
	SIG_END
};

static const uint16 phant1WineCaskHotspotPatch[] = {
	0x78,                                 // push1
	0x38, PATCH_UINT16(0x00d8),           // pushi 00d8 [ flag 216 ]
	0x45, 0x03, PATCH_UINT16(0x0002),     // callb proc0_3 [ seen barrel video? ]
	0x2f, 0x30,                           // bt 30 [ skip hotspot ]
	0x39, 0x06,                           // pushi 06
	0x81, 0x6a,                           // lag 6a
	0x04,                                 // sub
	0x31, 0x17,                           // bnt 17 [ show hotspot if chapter 6 ]
	0x78,                                 // push1
	0x1a,                                 // eq?
	0x31, 0x0a,                           // bnt 0a [ skip mirror test if not chapter 5 ]
	0x78,                                 // push1
	0x38, PATCH_UINT16(0x0123),           // pushi 0123 [ flag 291 ]
	0x45, 0x03, PATCH_UINT16(0x0002),     // callb proc0_3 [ seen mirror video? ]
	0x2f, 0x09,                           // bt 09 [ set hotspot ]
	0x78,                                 // push1
	0x39, 0x1d,                           // pushi 1d [ flag 29 ]
	0x45, 0x03, PATCH_UINT16(0x0002),     // callb proc0_3 [ tasted wine? ]
	0x2f, 0x12,                           // bt 12 [ skip hotspot ]
	0x38, PATCH_SELECTOR16(init),         // pushi init
	0x76,                                 // push0
	PATCH_ADDTOOFFSET(+11),
	0x4a, PATCH_UINT16(0x000c),           // send 0c [ spikot init: ... ]
	PATCH_END
};

// The darkroom in the chapter 7 chase, room 45950, has a bug which deletes the
//  chase history file and leaves the game in a state that can't be completed.
//
// Every action during the chase is written to chase.dat in order to support the
//  review feature which plays back the entire sequence. rm45950:init tests
//  several conditions to see how it should initialize the file. If a previous
//  chase.dat exists and the current game was initially started in chapter 7
//  then an unusual code path is taken which fails to clear flag 134. This flag
//  tells the room that the chase is starting. Upon returning for the book with
//  this flag incorrectly set, rm45950:init deletes chase.dat and all history is
//  lost. Upon playback, items obtained prior to the darkroom will be skipped
//  and become unobtainable, such as the glass shard.
//
// We fix this by clearing flag 134 in the unusual code path so that room 45950
//  never tries to initialize the chase history upon returning.
//
// Applies to: All versions
// Responsible method: rm45950:init
static const uint16 phant1DeleteChaseFileSignature[] = {
	0x32, SIG_UINT16(0x0148),       // jmp 0148 [ end of method ]
	SIG_ADDTOOFFSET(+0x36),
	0x78,                           // push1
	0x38, SIG_MAGICDWORD,           // pushi 0086
	      SIG_UINT16(0x0086),
	0x45, 0x02, SIG_UINT16(0x0002), // callb proc0_2 [ clear flag 134 ]
	0x32, SIG_UINT16(0x0107),       // jmp 0107 [ end of method ]
	SIG_END
};

static const uint16 phant1DeleteChaseFilePatch[] = {
	0x32, PATCH_UINT16(0x0036),     // jmp 0036 [ clear flag 134 ]
	PATCH_END
};

// Quitting ScummVM, returning to the launcher, or terminating the game in any
//  way outside of the game's menus during the chapter 7 chase leaves the saved
//  game in a broken state which will crash ScummVM when loaded. The chase was
//  programmed with the expectation that it could never end under unexpected
//  circumstances, which is a bold stance for a Sierra game.
//
// Each saved game consists of the file phantsg.#, and once the chase beings, an
//  additional chase file that records all actions for playback. This is named
//  chase.dat when the chase is running and chasedun.# when it isn't. The file
//  is renamed when transitioning to and from the main menu. The existence of
//  chasedun.# tells the main menu that it should put the game in the chapter 7
//  section. All of this depends on the chase formally ending through scripts
//  that can restore the file name. If the game terminates without this cleanup
//  then the chase file is never renamed and can't be associated with its slot.
//  The save will then be incorrectly demoted to a non-chapter 7 save. Upon
//  attempting to load this broken save, a chase script will attempt to play the
//  missing chase.dat, seek beyond the beginning, and fail an assertion in a
//  stream class. This never picks up the chase.dat from the previous run due to
//  a startup script which deletes stray chase.dat files.
//
// We fix this by copying chasedun.# to chase.dat instead of renaming it. This
//  leaves the original file intact along with phantsg.# in case the game
//  terminates without the scripts getting a chance to cleanup the file system.
//  This patch is currently the only known caller of kFileIOCopy.
//
// Applies to: All versions
// Responsible method: Exported procedure #6 in script 45951
static const uint16 phant1CopyChaseFileSignature[] = {
	0x39, 0x03,                     // pushi 03
	0x39, 0x0b,                     // pushi 0b [ kFileIORename ]
	0x39, SIG_SELECTOR8(data),      // pushi data
	0x76,                           // push0
	0x85, 0x00,                     // lat 00
	0x4a, SIG_UINT16(0x0004),       // send 04 [ temp0 data? ]
	0x36,                           // push    [ "chasedun.#" ]
	0x39, SIG_MAGICDWORD,           // pushi data
	      SIG_SELECTOR8(data),
	0x76,                           // push0
	0x85, 0x02,                     // lat 02
	0x4a, SIG_UINT16(0x0004),       // send 04 [ temp2 data? ]
	0x36,                           // push    [ "chase.dat" ]
	0x43, 0x5d, SIG_UINT16(0x0006), // callk FileIO 06
	SIG_END
};

static const uint16 phant1CopyChaseFilePatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x0c,                     // pushi 0c [ kFileIOCopy ]
	PATCH_END
};

// During the chase, the west exit in room 46980 has incorrect logic which kills
//  the player if they went to the crypt with the crucifix, among other bugs.
//
// Room 46980 takes place in the chapel and connects the secret passages to the
//  crypt. The player initially enters from the west and the crypt is to the
//  east. The west exit has incorrect logic with several consequences, but the
//  harshest is that if the player came from the crypt without beads then Don is
//  waiting for them. This is wrong because if the player has the crucifix then
//  there are no beads in the game, and also because Don is still in the crypt
//  where the player pushed a statue on him in the previous room.
//
// Sierra eventually fixed this by removing the beads from the equation and
//  swapping the transposed previous room test, but the fix only appears in the
//  Italian version, which was the final CD release. We replace the incorrect
//  logic with Sierra's final version.
//
// Applies to: All versions except Italian
// Responsible method: westExit:doVerb
static const uint16 phant1ChapelWestExitSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(has),      // pushi has
	0x78,                           // push1
	0x39, 0x0f,                     // pushi 0f
	0x81, 0x00,                     // lag 00
	0x4a, SIG_UINT16(0x0006),       // send 06 [ ego has: 15 ]
	0x2f, 0x06,                     // bt 06
	0x89, 0x0c,                     // lsg 0c
	0x34, SIG_UINT16(0xb680),       // ldi b680
	0x1a,                           // eq? [ previous room == 46720 ]
	SIG_END
};

static const uint16 phant1ChapelWestExitPatch[] = {
	0x32, PATCH_UINT16(0x000a),     // jmp 000a [ skip inventory check ]
	PATCH_ADDTOOFFSET(+15),
	0x1c,                           // ne? [ previous room != 46720 ]
	PATCH_END
};

// When reviewing the chapter 7 chase, the game fails to reset the flag that's
//  set when Adrian stabs Don and escapes the nursery. This can only happen once
//  and so the stale flag causes the review feature to play the wrong animation
//  of Don overpowering Adrian instead of Adrian stabbing Don and escaping.
//
// We fix this as Sierra did in the Italian version by clearing flag 18 during
//  the chase initialization.
//
// Applies to: All versions except Italian
// Responsible method: sChaseBegin:changeState(0)
static const uint16 phant1ResetStabDonFlagSignature[] = {
	SIG_MAGICDWORD,
	0x78,                           // push1
	0x38, SIG_UINT16(0x019f),       // pushi 019f
	SIG_ADDTOOFFSET(+4),
	0x7e, SIG_ADDTOOFFSET(+2),      // line
	0x78,                           // push1
	0x39, 0x7b,                     // pushi 7b
	0x45, 0x02, SIG_UINT16(0x0002), // callb proc0_2 [ clear flag 123 ]
	0x7e, SIG_ADDTOOFFSET(+2),      // line
	0x89, 0x0c,                     // lsg 0c
	0x34, SIG_UINT16(0x0384),       // ldi 0384
	0x1a,                           // eq?
	0x18,                           // not
	SIG_END
};

static const uint16 phant1ResetStabDonFlagPatch[] = {
	PATCH_ADDTOOFFSET(+8),
	0x78,                               // push1
	0x39, 0x7b,                         // pushi 7b
	0x45, 0x02, PATCH_UINT16(0x0002),   // callb proc0_2 [ clear flag 123 ]
	0x78,                               // push1
	0x39, 0x12,                         // pushi 12
	0x45, 0x02, PATCH_UINT16(0x0002),	// callb proc0_2 [ clear flag 18 ]
	0x89, 0x0c,                         // lsg 0c
	0x34, PATCH_UINT16(0x0384),         // ldi 0384
	0x1c,                               // ne?
	PATCH_END
};

// The censorship for videos 1920 and 2020 are out of sync and the latter has
//  an incorrect coordinate. The frame numbers for the blobs are wrong and so
//  they appear during normal frames but not during the gore. These videos play
//  in room 40100 when attempting to open or unbar the door. We fix the frame
//  numbers to be in sync with the videos and use the correct coordinate from
//  video 1920 which has the same censored frames.
//
// Applies to: All versions
// Responsible method: myList:init
static const uint16 phant1Video1920CensorSignature[] = {
	0x38, SIG_UINT16(0x00dd),       // pushi 221 [ blob 1 start frame ]
	SIG_ADDTOOFFSET(+43),
	0x39, SIG_MAGICDWORD, 0xff,     // pushi -1
	0x38, SIG_UINT16(0x00e1),       // pushi 225 [ blob 1 end frame ]
	SIG_ADDTOOFFSET(+20),
	0x38, SIG_UINT16(0x00fb),       // pushi 251 [ blob 2 start frame ]
	SIG_ADDTOOFFSET(+46),
	0x38, SIG_UINT16(0x0117),       // pushi 279 [ blob 2 end frame ]
	SIG_END
};

static const uint16 phant1Video1920CensorPatch[] = {
	0x38, PATCH_UINT16(0x00c6),     // pushi 198 [ blob 1 start frame ]
	PATCH_ADDTOOFFSET(+45),
	0x38, PATCH_UINT16(0x00ca),     // pushi 202 [ blob 1 end frame ]
	PATCH_ADDTOOFFSET(+20),
	0x38, PATCH_UINT16(0x00e4),     // pushi 228 [ blob 2 start frame ]
	PATCH_ADDTOOFFSET(+46),
	0x38, PATCH_UINT16(0x00fe),     // pushi 254 [ blob 2 end frame ]
	PATCH_END
};

static const uint16 phant1Video2020CensorSignature[] = {
	0x38, SIG_UINT16(0x014f),       // pushi 335 [ blob 1 start frame ]
	SIG_ADDTOOFFSET(+18),
	0x38, SIG_UINT16(0x0090),       // pushi 144 [ blob 1 left coordinate ]
	SIG_ADDTOOFFSET(+23),
	0x39, SIG_MAGICDWORD, 0xff,     // pushi -1
	0x38, SIG_UINT16(0x0153),       // pushi 339 [ blob 1 end frame ]
	SIG_ADDTOOFFSET(+20),
	0x38, SIG_UINT16(0x0160),       // pushi 352 [ blob 2 start frame ]
	SIG_ADDTOOFFSET(+46),
	0x38, SIG_UINT16(0x016a),       // pushi 362 [ blob 2 end frame ]
	SIG_END
};

static const uint16 phant1Video2020CensorPatch[] = {
	0x38, PATCH_UINT16(0x012f),     // pushi 303 [ blob 1 start frame ]
	PATCH_ADDTOOFFSET(+18),
	0x38, PATCH_UINT16(0x0042),     // pushi 66  [ blob 1 left coordinate ]
	PATCH_ADDTOOFFSET(+25),
	0x38, PATCH_UINT16(0x0133),     // pushi 307 [ blob 1 end frame ]
	PATCH_ADDTOOFFSET(+20),
	0x38, PATCH_UINT16(0x014c),     // pushi 332 [ blob 2 start frame ]
	PATCH_ADDTOOFFSET(+46),
	0x38, PATCH_UINT16(0x0168),     // pushi 360 [ blob 2 end frame ]
	PATCH_END
};

// The Mac version is hard-coded to use a low video speed value. This defaults
//  the video size to half-screen and causes low frame-rate versions of several
//  videos to play, even though better versions are included. PC versions read
//  config values written by a benchmark in the game's installer. We address
//  that by returning a high video speed value of 500 in kGetSierraProfileInt
//  and kGetConfig, and so we patch the Mac value to 500 as well.
//
// Applies to: Mac version only
// Responsible method: Scary:init
static const uint16 phant1MacVideoQualitySignature[] = {
	0x34, SIG_UINT16(0x0190),       // ldi 0190
	SIG_MAGICDWORD,
	0xa1, 0xb2,                     // sag b2 [ video speed = 400 ]
	0x36,                           // push
	0x34, SIG_UINT16(0x01a9),       // ldi 01a9
	0x1e,                           // gt?
	SIG_END
};

static const uint16 phant1MacVideoQualityPatch[] = {
	0x34, PATCH_UINT16(0x01f4),     // ldi 01f4 [ 500 ]
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry phantasmagoriaSignatures[] = {
	{  true,     0, "mac: set high video quality",                 1, phant1MacVideoQualitySignature,  phant1MacVideoQualityPatch },
	{  true,    23, "make cursor red after clicking quit",         1, phant1RedQuitCursorSignature,    phant1RedQuitCursorPatch },
	{  true,    26, "fix video 1920 censorship",                   1, phant1Video1920CensorSignature,  phant1Video1920CensorPatch },
	{  true,    26, "fix video 2020 censorship",                   1, phant1Video2020CensorSignature,  phant1Video2020CensorPatch },
	{  true,    33, "disable video benchmarking",                  1, sci2BenchmarkSignature,          sci2BenchmarkPatch },
	{  true,   901, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,  1111, "ignore audio settings from save game",        1, phant1SavedVolumeSignature,      phant1SavedVolumePatch },
	{  true, 20200, "fix broken rat init in sEnterFromAlcove",     1, phant1RatSignature,              phant1RatPatch },
	{  true, 20200, "fix chapter 5 wine cask hotspot",             1, phant1WineCaskHotspotSignature,  phant1WineCaskHotspotPatch },
	{  true, 45950, "fix chase file deletion",                     1, phant1DeleteChaseFileSignature,  phant1DeleteChaseFilePatch },
	{  true, 45950, "reset stab don flag",                         1, phant1ResetStabDonFlagSignature, phant1ResetStabDonFlagPatch },
	{  true, 45951, "copy chase file instead of rename",           1, phant1CopyChaseFileSignature,    phant1CopyChaseFilePatch },
	{  true, 46980, "fix chapel chase west exit",                  1, phant1ChapelWestExitSignature,   phant1ChapelWestExitPatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,      sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Phantasmagoria 2

// The game uses a spin loop when navigating to and from Curtis's computer in
// the office, and when entering passwords, which causes the mouse to appear
// unresponsive. Replace the spin loop with a call to ScummVM kWait.
// Applies to at least: US English
// Responsible method: Script 3000 localproc 2ee4, script 63019 localproc 4f04
static const uint16 phant2WaitParam1Signature[] = {
	SIG_MAGICDWORD,
	0x35, 0x00, // ldi 0
	0xa5, 0x00, // sat temp[0]
	0x8d, 0x00, // lst temp[0]
	0x87, 0x01, // lap param[1]
	SIG_END
};

static const uint16 phant2WaitParam1Patch[] = {
	0x78,                                     // push1
	0x8f, 0x01,                               // lsp param[1]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x48,                                     // ret
	PATCH_END
};

// The interface bars at the top and bottom of the screen fade in and out when
// hovered over. This fade is performed by a script loop that calls kFrameOut
// directly and uses global[227] as the fade delta for each frame. It normally
// is set to 1, which means that these fades are quite slow. We replace the use
// of global[227] with an immediate value for a reasonable fade speed.
// Applies to at least: US English
static const uint16 phant2SlowIFadeSignature[] = {
	0x43, 0x21, SIG_UINT16(0x0000),     // callk FrameOut, 0
	SIG_MAGICDWORD,
	0x67, 0x03,                         // pTos 03 (scratch)
	0x81, 0xe3,                         // lag global[227]
	SIG_END
};

static const uint16 phant2SlowIFadePatch[] = {
	PATCH_ADDTOOFFSET(+6),              // skip to lag
	0x35, 0x05,                         // ldi 5
	PATCH_END
};

// The game uses a spin loop during music transitions which causes the mouse to
// appear unresponsive during scene changes. Replace the spin loop with a call
// to ScummVM kWait.
// Applies to at least: US English
// Responsible method: P2SongPlyr::wait4Fade
static const uint16 phant2Wait4FadeSignature[] = {
	SIG_MAGICDWORD,
	0x76,                               // push0
	0x43, 0x79, SIG_UINT16(0x0000),     // callk GetTime, 0
	0xa5, 0x01,                         // sat temp[1]
	0x78,                               // push1
	SIG_END
};

static const uint16 phant2Wait4FadePatch[] = {
	0x78,                                     // push1
	0x8d, 0x00,                               // lst temp[0]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x48,                                     // ret
	PATCH_END
};

// When reading the VERSION file, Phant2 sends a Str object instead of a
// reference to a string (kernel signature violation), and flips the file handle
// and size arguments, so the version file data never actually makes it into the
// game.
// Applies to at least: Phant2 US English CD
static const uint16 phant2GetVersionSignature[] = {
	0x36,                         // push
	0x35, 0xff,                   // ldi $ff
	0x1c,                         // ne?
	0x31, 0x0e,                   // bnt $e
	0x39, 0x04,                   // pushi 4
	0x39, 0x05,                   // pushi 5
	SIG_MAGICDWORD,
	0x89, 0x1b,                   // lsg global[$1b]
	0x8d, 0x05,                   // lst temp[5]
	0x39, 0x09,                   // pushi 9
	0x43, 0x5d, SIG_UINT16(0x08), // callk FileIO, 8
	0x7a,                         // push2
	0x78,                         // push1
	0x8d, 0x05,                   // lst temp[5]
	0x43, 0x5d, SIG_UINT16(0x04), // callk FileIO, 4
	0x35, 0x01,                   // ldi 1
	0xa1, 0xd8,                   // sag global[$d8]
	SIG_END
};

static const uint16 phant2GetVersionPatch[] = {
	0x39, 0x04,                     // pushi 4
	0x39, 0x05,                     // pushi 5
	0x81, 0x1b,                     // lag global[$1b]
	0x39, PATCH_SELECTOR8(data),    // pushi data
	0x76,                           // push0
	0x4a, PATCH_UINT16(0x0004),     // send 4
	0x36,                           // push
	0x39, 0x09,                     // pushi 9
	0x8d, 0x05,                     // lst temp[5]
	0x43, 0x5d, PATCH_UINT16(0x08), // callk FileIO, 8
	0x7a,                           // push2
	0x78,                           // push1
	0x8d, 0x05,                     // lst temp[5]
	0x43, 0x5d, PATCH_UINT16(0x04), // callk FileIO, 4
	0x78,                           // push1
	0xa9, 0xd8,                     // ssg global[$d8]
	PATCH_END
};

// The game uses a spin loop when displaying the success animation of the ratboy
// puzzle, which causes the mouse to appear unresponsive. Replace the spin loop
// with a call to ScummVM kWait.
// Applies to at least: US English
static const uint16 phant2RatboySignature[] = {
	0x8d, 0x01,                   // lst temp[1]
	0x35, 0x1e,                   // ldi $1e
	0x22,                         // lt?
	SIG_MAGICDWORD,
	0x31, 0x17,                   // bnt $17 [0c3d]
	0x76,                         // push0
	0x43, 0x79, SIG_UINT16(0x00), // callk GetTime, 0
	SIG_END
};

static const uint16 phant2RatboyPatch[] = {
	0x78,                                     // push1
	0x35, 0x1e,                               // ldi $1e
	0x36,                                     // push
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, $2
	0x33, 0x14,                               // jmp [to next outer loop]
	PATCH_END
};

// Phant2 has separate in-game volume controls for handling movie volume and
// in-game volume (misleadingly labelled "music volume"), but really needs the
// in-game volume to always be significantly lower than the movie volume in
// order for dialogue in movies to be consistently audible, so patch the
// in-game volume slider to limit it to our maximum.
// Applies to at least: US English
// Fixes bug: #10165
static const uint16 phant2AudioVolumeSignature[] = {
	SIG_MAGICDWORD,
	0x39, 0x7f,           // pushi 127 (clientMax value)
	0x39, 0x14,           // pushi 20  (clientPageSize value)
	SIG_ADDTOOFFSET(+10), // skip other init arguments
	0x51, 0x5e,           // class P2ScrollBar
	SIG_ADDTOOFFSET(+3),  // skip send
	0xa3, 0x06,           // sal local[6] (identifies correct slider)
	SIG_END
};

static const uint16 phant2AudioVolumePatch[] = {
	0x39, kPhant2VolumeMax,              // pushi (our custom volume max)
	0x39, 0x14 * kPhant2VolumeMax / 127, // pushi (ratio of original value)
	PATCH_END
};

// When censorship is disabled the game sticks <PROTECTED> at the end of every
// save game name, and when it is enabled it pads the save game name with a
// bunch of spaces. This is annoying and not helpful, so just disable all of
// this nonsense.
// Applies to at least: US English
// Fixes bug: #10035
static const uint16 phant2SaveNameSignature1[] = {
	SIG_MAGICDWORD,
	0x57, 0x4b, SIG_UINT16(0x06), // super SREdit, 6
	0x63,                         // pToa (plane)
	SIG_END
};

static const uint16 phant2SaveNamePatch1[] = {
	PATCH_ADDTOOFFSET(+4), // super SREdit, 6
	0x48,                  // ret
	PATCH_END
};

static const uint16 phant2SaveNameSignature2[] = {
	SIG_MAGICDWORD,
	0xa5, 0x00,                  // sat temp[0]
	0x39, SIG_SELECTOR8(format), // pushi format
	SIG_END
};

static const uint16 phant2SaveNamePatch2[] = {
	PATCH_ADDTOOFFSET(+2), // sat temp[0]
	0x33, 0x68,            // jmp [past name mangling]
	PATCH_END
};

// Phant2-specific version of sci2NumSavesSignature1/2
// Applies to at least: English CD
static const uint16 phant2NumSavesSignature1[] = {
	SIG_MAGICDWORD,
	0x8d, 0x01, // lst temp[1]
	0x35, 0x14, // ldi 20
	0x1a,       // eq?
	SIG_END
};

static const uint16 phant2NumSavesPatch1[] = {
	PATCH_ADDTOOFFSET(+2), // lst temp[1]
	0x35, 0x63,            // ldi 99
	PATCH_END
};

static const uint16 phant2NumSavesSignature2[] = {
	SIG_MAGICDWORD,
	0x8d, 0x00, // lst temp[0]
	0x35, 0x14, // ldi 20
	0x22,       // lt?
	SIG_END
};

static const uint16 phant2NumSavesPatch2[] = {
	PATCH_ADDTOOFFSET(+2), // lst temp[0]
	0x35, 0x63,            // ldi 99
	PATCH_END
};

// The game script responsible for handling document scrolling in the computer
// interface uses a spin loop to wait for 10 ticks every time the document
// scrolls. This makes scrolling janky and makes the mouse appear
// non-responsive. Eliminating the delay entirely makes scrolling with the arrow
// buttons a little too quick; a delay of 3 ticks is an OK middle-ground between
// allowing mostly fluid motion with mouse dragging and reasonably paced
// scrolling holding down the arrows. Preferably, ScrollbarArrow::handleEvent or
// ScrollbarArrow::action would only send cues once every N ticks whilst being
// held down, but unfortunately the game was not programmed to do this.
// Applies to at least: US English
static const uint16 phant2SlowScrollSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x0a,                         // ldi 10
	0x22,                               // lt?
	0x31, 0x17,                         // bnt [end of loop]
	0x76,                               // push0
	0x43, 0x79, SIG_UINT16(0x0000),     // callk GetTime, 0
	SIG_END
};

static const uint16 phant2SlowScrollPatch[] = {
	0x78,                                     // push1
	0x39, 0x03,                               // pushi 3
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x33, 0x13,                               // jmp [end of loop]
	PATCH_END
};

// WynNetDoco::open calls setSize before it calls posn, but the values set by
// posn are used by setSize, so the left/top coordinates of the text and note
// fields is wrong for the first render of a document or email. (Incidentally,
// these fields are the now-seen rect fields, and the game is doing a very bad
// thing by touching these manually and then relying on the values instead of
// asking the kernel.) This is most noticeable during chapters 1 and 3 when the
// computer is displaying scary messages, since every time the scary message is
// rendered the text fields re-render at the top-left corner of the screen.
// Applies to at least: US English
// Fixes bug: #10036
static const uint16 phant2BadPositionSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(setSize), // pushi setSize
	0x76,                         // push0
	0x39, SIG_SELECTOR8(init),    // pushi init
	0x78,                         // pushi 1
	0x89, 0x03,                   // lsg global[3]
	0x39, SIG_SELECTOR8(posn),    // pushi posn
	0x7a,                         // push2
	0x66, SIG_ADDTOOFFSET(+2),    // pTos (x position)
	0x66, SIG_ADDTOOFFSET(+2),    // pTos (y position)
	SIG_END
};

static const uint16 phant2BadPositionPatch[] = {
	0x39, PATCH_SELECTOR8(posn),        // pushi posn
	0x7a,                               // push2
	0x66, PATCH_GETORIGINALUINT16(12),  // pTos (x position)
	0x66, PATCH_GETORIGINALUINT16(15),  // pTos (y position)
	0x39, PATCH_SELECTOR8(setSize),     // pushi setSize
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(init),        // pushi init
	0x78,                               // pushi 1
	0x89, 0x03,                         // lsg global[3]
	PATCH_END
};

// WynDocuStore::refresh resets the cel of the open folder and document icons,
// so they don't end up being rendered as closed folder/document icons, but it
// forgets to actually update the icon's View with the kernel, so they render
// as closed for the first render after a refresh anyway. This is most
// noticeable during chapters 1 and 3 when the computer is displaying scary
// messages, since every time the scary message is rendered the icons re-render
// as closed.
// Applies to at least: US English
static const uint16 phant2BadIconSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setCel), // pushi setCel
	0x78,                         // push1
	0x78,                         // push1
	0x38, SIG_SELECTOR16(iconV),  // pushi iconV
	0x76,                         // push0
	0x62, SIG_ADDTOOFFSET(+2),    // pToa curFolder/curDoco
	0x4a, SIG_UINT16(0x0004),     // send 4
	0x4a, SIG_UINT16(0x0006),     // send 6
	SIG_END
};

static const uint16 phant2BadIconPatch[] = {
	PATCH_ADDTOOFFSET(+5),          // pushi setCel, push1, push1
	0x38, PATCH_SELECTOR16(update), // pushi update
	0x76,                           // push0
	0x4a, PATCH_UINT16(0x000a),     // send 10
	0x33, 0x04,                     // jmp [past unused bytes]
	PATCH_END
};

// The left and right arrows move inventory items a pixel more than each
// inventory item is wide, which causes the inventory to creep to the left by
// one pixel per scrolled item.
// Applies to at least: US English
// Fixes bug: #10037
static const uint16 phant2InvLeftDeltaSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0042),                 // delta
	SIG_UINT16(0x0019),                 // moveDelay
	SIG_END
};

static const uint16 phant2InvLeftDeltaPatch[] = {
	PATCH_UINT16(0x0041),               // delta
	PATCH_END
};

static const uint16 phant2InvRightDeltaSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0xffbe),                 // delta
	SIG_UINT16(0x0019),                 // moveDelay
	SIG_END
};

static const uint16 phant2InvRightDeltaPatch[] = {
	PATCH_UINT16(0xffbf),               // delta
	PATCH_END
};

// The first inventory item is put too far to the right, which causes wide items
// to get cut off on the right side of the inventory.
// Applies to at least: US English
static const uint16 phant2InvOffsetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x26,                         // ldi 38
	0x64, SIG_SELECTOR16(xOff),         // aTop xOff
	SIG_END
};

static const uint16 phant2InvOffsetPatch[] = {
	0x35, 0x1d,                         // ldi 29
	PATCH_END
};

// The text placement of "File" and "Note" content inside DocuStore File
// Retrieval System makes some letters especially "g" overlap the
// corresponding box. Set by 'WynNetDoco::open'.
// We fix this by changing the position of those 2 inside the heap of
// subclass 'WynNetDoco' slightly.
// Applies to at least: English CD, Japanese CD, German CD
// Fixes bug: #10034
static const uint16 phant2DocuStoreFileNotePlacementSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0046),   // nameX
	SIG_UINT16(0x000a),   // nameY
	SIG_ADDTOOFFSET(+10), // skip over nameMsg*
	SIG_UINT16(0x0046),   // noteX
	SIG_UINT16(0x001e),   // noteY
	SIG_END
};

static const uint16 phant2DocuStoreFileNotePlacementPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x0006),   // new nameY
	PATCH_ADDTOOFFSET(+12),
	PATCH_UINT16(0x001b),   // new noteY
	PATCH_END
};

// The text placement of "From" and "Subject" content inside DocuStore.
// We fix this by changing the position inside the heap of subclass
// 'WynNetEmail' slightly.
// For this one, we also fix the horizontal placement.
static const uint16 phant2DocuStoreEmailPlacementSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0049),   // nameX
	SIG_UINT16(0x0008),   // nameY
	SIG_ADDTOOFFSET(+10), // skip over nameMsg*
	SIG_UINT16(0x0049),   // noteX
	SIG_UINT16(0x001c),   // noteY
	SIG_END
};

static const uint16 phant2DocuStoreEmailPlacementPatch[] = {
	PATCH_UINT16(0x0050),   // new nameX
	PATCH_UINT16(0x0006),   // new nameY
	PATCH_ADDTOOFFSET(+10),
	PATCH_UINT16(0x0050),   // new noteX
	PATCH_UINT16(0x001b),   // new noteY
	PATCH_END
};

// The scrollbars in the DocuStore computer crash when clicking on the thumb,
//  dragging the cursor off of the scrollbar, and then releasing. This calls
//  Thumb:action with no event parameter, which Thumb:action expects, but after
//  handling this it proceeds to use the uninitialized parameter anyway.
//  DItem:track then errors by sending a message to this non-object.
//
// We fix this by passing the current event instead of the potentially
//  nonexistent event parameter to DItem:track. This bug also occurs in the
//  volume scrollbars in the control panel, which this patch also fixes.
//
// Applies to at least: English PC, French PC, probably all versions
// Responsible method: Thumb:action
// Fixes bug: #10391
static const uint16 phant2ComputerScrollbarCrashSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x38, SIG_SELECTOR16(track),        // pushi track
	0x78,                               // push1
	0x8f, 0x01,                         // lsp 01
	0x54, SIG_UINT16(0x0006),           // self 06 [ self track: param1 ]
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_MAGICDWORD,
	0x76,                               // push0
	0x43, 0x21, SIG_UINT16(0x0000),     // callk FrameOut
	0x7e,                               // line
	SIG_END
};

static const uint16 phant2ComputerScrollbarCrashPatch[] = {
	0x38, PATCH_SELECTOR16(track),      // pushi track
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(curEvent),   // pushi curEvent
	0x76,                               // push0
	0x80, PATCH_UINT16(0x0050),         // lag 0050
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ p2User curEvent? ]
	0x36,                               // push
	0x54, PATCH_UINT16(0x0006),         // self 06 [ self track: (p2User curEvent?) ]
	0x76,                               // push0
	0x43, 0x21, PATCH_UINT16(0x0000),   // callk FrameOut
	PATCH_END
};

// The volume scrollbar arrows on the control panel don't respond to clicks.
//  ScrollbarArrow:handleEvent only calls its action method if the selection
//  flag (2) of its state property is set. DItem:track sets and clears this flag
//  by calling DItem:hilite if the mouse has moved since the previous cycle.
//  This normally works because Cast:handleEvent calls the track method of each
//  member of its mouser collection, but User:doit only passes these events on
//  if User:hogs is empty. Loading the control panel adds its button to the hogs
//  stack which prevents DItem:track from being called and updating the flag.
//  
// We fix this by setting the global mouse-moved flag whenever a scrollbar arrow
//  is pressed. ScrollbarArrow:handleEvent already calls DItem:track, so setting
//  this flag before then causes it to always evaluate the incoming event and
//  update the selection state flag accordingly.
//
// Applies to at least: English PC, French PC, probably all versions
// Responsible method: ScrollbarArrow:handleEvent
// Fixes bug: #10164
static const uint16 phant2ScrollbarArrowSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x67, SIG_SELECTOR8(state),         // pTos state
	0x35, 0x20,                         // ldi 20
	0x12,                               // and [ is arrow pressed? ]
	SIG_MAGICDWORD,
	0x18,                               // not
	0x18,                               // not
	0x31, 0x64,                         // bnt 64
	0x7e,                               // line
	SIG_END
};

static const uint16 phant2ScrollbarArrowPatch[] = {
	0x67, PATCH_SELECTOR8(state),       // pTos state
	0x35, 0x20,                         // ldi 20
	0x12,                               // and [ is arrow pressed? ]
	0x31, 0x6c,                         // bnt 6c
	0x38, PATCH_SELECTOR16(mouseMoved), // pushi mouseMoved
	0x78,                               // push1
	0x78,                               // push1
	0x80, PATCH_UINT16(0x0050),         // lag 0050
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ p2User mouseMoved: 1 ]
	PATCH_END
};

//          script, description,                                      signature                                  patch
static const SciScriptPatcherEntry phantasmagoria2Signatures[] = {
	{  true,     0, "speed up interface fades",                    3, phant2SlowIFadeSignature,                  phant2SlowIFadePatch },
	{  true,     0, "fix bad arguments to get game version",       1, phant2GetVersionSignature,                 phant2GetVersionPatch },
	{  true,  3000, "replace spin loop in alien password window",  1, phant2WaitParam1Signature,                 phant2WaitParam1Patch },
	{  true,  4081, "replace spin loop after ratboy puzzle",       1, phant2RatboySignature,                     phant2RatboyPatch },
	{  true, 63001, "fix inventory left scroll delta",             1, phant2InvLeftDeltaSignature,               phant2InvLeftDeltaPatch },
	{  true, 63001, "fix inventory right scroll delta",            1, phant2InvRightDeltaSignature,              phant2InvRightDeltaPatch },
	{  true, 63001, "fix inventory wrong initial offset",          1, phant2InvOffsetSignature,                  phant2InvOffsetPatch },
	{  true, 63004, "limit in-game audio volume",                  1, phant2AudioVolumeSignature,                phant2AudioVolumePatch },
	{  true, 63016, "replace spin loop during music fades",        1, phant2Wait4FadeSignature,                  phant2Wait4FadePatch },
	{  true, 63019, "replace spin loop during computer load",      1, phant2WaitParam1Signature,                 phant2WaitParam1Patch },
	{  true, 63019, "replace spin loop during computer scrolling", 1, phant2SlowScrollSignature,                 phant2SlowScrollPatch },
	{  true, 63019, "fix bad doc/email name & memo positioning",   2, phant2BadPositionSignature,                phant2BadPositionPatch },
	{  true, 63019, "fix bad folder/doc icon refresh",             2, phant2BadIconSignature,                    phant2BadIconPatch },
	{  true, 63019, "fix file and note content placement",         1, phant2DocuStoreFileNotePlacementSignature, phant2DocuStoreFileNotePlacementPatch },
	{  true, 63019, "fix email content placement",                 1, phant2DocuStoreEmailPlacementSignature,    phant2DocuStoreEmailPlacementPatch },
	{  true, 64926, "fix computer scrollbar crash",                1, phant2ComputerScrollbarCrashSignature,     phant2ComputerScrollbarCrashPatch },
	{  true, 64926, "fix volume scrollbar arrows",                 1, phant2ScrollbarArrowSignature,             phant2ScrollbarArrowPatch },
	{  true, 64990, "remove save game name mangling (1/2)",        1, phant2SaveNameSignature1,                  phant2SaveNamePatch1 },
	{  true, 64990, "increase number of save games (1/2)",         1, phant2NumSavesSignature1,                  phant2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         2, phant2NumSavesSignature2,                  phant2NumSavesPatch2 },
	{  true, 64994, "remove save game name mangling (2/2)",        1, phant2SaveNameSignature2,                  phant2SaveNamePatch2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Pepper's Adventures in Time

//          script, description,                                         signature                            patch
static const SciScriptPatcherEntry pepperSignatures[] = {
	{  true,   928, "Narrator lockup fix",                            1, sciNarratorLockupSignature,          sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Police Quest 1 VGA

// When briefing is about to start in room 15, other officers will get into the room too.
// When one of those officers gets into the way of ego, they will tell the player to sit down.
// But control will be disabled right at that point. Ego may then go to his seat by himself,
// or more often than not will just stand there. The player is unable to do anything.
//
// Sergeant Dooley will then enter the room. Tell the player to sit down 3 times and after
// that it's game over.
//
// Because the Sergeant is telling the player to sit down, one has to assume that the player
// is meant to still be in control. Which is why this script patch removes disabling of player control.
//
// The script also tries to make ego walk to the chair, but it fails because it gets stuck with other
// actors. So I guess the safest way is to remove all of that and let the player do it manually.
//
// The responsible method seems to use a few hardcoded texts, which is why I have to assume that it's
// not used anywhere else. I also checked all scripts and couldn't find any other calls to it.
//
// This of course also happens when using the original interpreter.
//
// Scripts work like this: manX::doit (script 134) triggers gab::changeState, which then triggers rm015::notify
//
// Applies to at least: English floppy
// Responsible method: gab::changeState (script 152)
// Fixes bug: #5865
static const uint16 pq1vgaSignatureBriefingGettingStuck[] = {
	0x76,                                // push0
	0x45, 0x02, 0x00,                    // callb [export 2 of script 0], 00 (disable control)
	0x38, SIG_ADDTOOFFSET(+2),           // pushi notify
	0x76,                                // push0
	0x81, 0x02,                          // lag global[2] (get current room)
	0x4a, 0x04,                          // send 04
	SIG_MAGICDWORD,
	0x8b, 0x02,                          // lsl local[2]
	0x35, 0x01,                          // ldi 01
	0x02,                                // add
	SIG_END
};

static const uint16 pq1vgaPatchBriefingGettingStuck[] = {
	0x33, 0x0a,                      // jmp [to lsl local[2], skip over export 2 and ::notify]
	PATCH_END                        // rm015::notify would try to make ego walk to the chair
};

// When at the police station, you can put or get your gun from your locker.
// The script, that handles this, is buggy. It disposes the gun as soon as
//  you click, but then waits 2 seconds before it also closes the locker.
// Problem is that it's possible to click again, which then results in a
//  disposed object getting accessed. This happened to work by pure luck in
//  SSCI.
// This patch changes the code, so that the gun is actually given away
//  when the 2 seconds have passed and the locker got closed.
// Applies to at least: English floppy
// Responsible method: putGun::changeState (script 341)
// Fixes bug: #5705, #6400
static const uint16 pq1vgaSignaturePutGunInLockerBug[] = {
	0x35, 0x00,                      // ldi 00
	0x1a,                            // eq?
	0x31, 0x25,                      // bnt [next state check]
	SIG_ADDTOOFFSET(+22),            // [skip 22 bytes]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(put),       // pushi put
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - ego::put(0)
	0x35, 0x02,                      // ldi 02
	0x65, 0x1c,                      // aTop 1c (set timer to 2 seconds)
	0x33, 0x0e,                      // jmp [end of method]
	0x3c,                            // dup (state check)
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x08,                      // bnt [end of method]
	0x39, SIG_SELECTOR8(dispose),    // pushi dispose
	0x76,                            // push0
	0x72, SIG_UINT16(0x0088),        // lofsa 0088
	0x4a, 0x04,                      // send 04 - locker::dispose
	SIG_END
};

static const uint16 pq1vgaPatchPutGunInLockerBug[] = {
	PATCH_ADDTOOFFSET(+3),
	0x31, 0x1c,                      // bnt [next state check]
	PATCH_ADDTOOFFSET(+22),
	0x35, 0x02,                      // ldi 02
	0x65, 0x1c,                      // aTop 1c (set timer to 2 seconds)
	0x33, 0x17,                      // jmp [end of method]
	0x3c,                            // dup (state check)
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x11,                      // bnt [end of method]
	0x38, PATCH_SELECTOR16(put),     // pushi put
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - ego::put(0)
	PATCH_END
};

// When restoring a saved game, which was made while driving around,
//  the game didn't redraw the map. This also happened in Sierra SCI.
//
// The map is a picture resource and drawn over the main picture.
//  This is called an "overlay" in SCI. This wasn't implemented properly.
//  We fix it by actually implementing it properly.
//
// Applies to at least: English floppy
// Responsible method: rm500::init, changeOverlay::changeState (script 500)
// Fixes bug: #5016
static const uint16 pq1vgaSignatureMapSaveRestoreBug[] = {
	0x39, 0x04,                          // pushi 04
	SIG_ADDTOOFFSET(+2),                 // skip either lsg global[f9] or pTos register
	SIG_MAGICDWORD,
	0x38, 0x64, 0x80,                    // pushi 8064
	0x76,                                // push0
	0x89, 0x28,                          // lsg global[28]
	0x43, 0x08, 0x08,                    // callk DrawPic, 8
	SIG_END
};

static const uint16 pq1vgaPatchMapSaveRestoreBug[] = {
	0x38, PATCH_SELECTOR16(overlay), // pushi overlay
	0x7a,                            // push2
	0x89, 0xf9,                      // lsg global[f9]
	0x39, 0x64,                      // pushi 64 (no transition)
	0x81, 0x02,                      // lag global[2] (current room object)
	0x4a, 0x08,                      // send 08
	0x18,                            // not (waste byte)
	PATCH_END
};

// In the first release of PQ1VGA, looking at objects while sitting in the car
//  outside of Carol's breaks the game. The objects set Look as an approachVerb,
//  causing ego to float towards them without leaving the car and initializing.
//
// We fix this as Sierra did by removing Look from all approachVerbs in room 30.
//
// Applies to: English Floppy without 30.HEP and 30.SCR
// Responsible methods: door:init, harleys:init, willySign:init, carolSign:init,
//                      carolWindow:init, weeds:init, alley:init, mat:init
// Fixes bug: #5826
static const uint16 pq1vgaSignatureFloatOutsideCarols1[] = {
	0x38, SIG_SELECTOR16(approachVerbs), // pushi approachVerbs
	SIG_MAGICDWORD,
	0x78,                                // push1
	0x78,                                // push1
	0x54, 0x06,                          // self 06 [ self approachVerbs: 1 ]
	SIG_END
};

static const uint16 pq1vgaPatchFloatOutsideCarols1[] = {
	0x32, PATCH_UINT16(0x0004),          // jmp 0004 [ don't set approachVerbs ]
	PATCH_END
};

static const uint16 pq1vgaSignatureFloatOutsideCarols2[] = {
	0x38, SIG_SELECTOR16(approachVerbs), // pushi approachVerbs
	SIG_MAGICDWORD,
	0x7a,                                // push2
	0x78,                                // push1
	0x39, 0x04,                          // pushi 04
	0x54, 0x08,                          // self 08 [ self approachVerbs: 1 4 ]
	SIG_END
};

static const uint16 pq1vgaPatchFloatOutsideCarols2[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x01,                          // pushi 01
	PATCH_ADDTOOFFSET(+2),
	0x54, 0x06,                          // self 06 [ self approachVerbs: 4 ]
	PATCH_END
};

//          script, description,                                         signature                            patch
static const SciScriptPatcherEntry pq1vgaSignatures[] = {
	{  true,    30, "float outside carol's (1/2)",                    7, pq1vgaSignatureFloatOutsideCarols1,  pq1vgaPatchFloatOutsideCarols1 },
	{  true,    30, "float outside carol's (2/2)",                    1, pq1vgaSignatureFloatOutsideCarols2,  pq1vgaPatchFloatOutsideCarols2 },
	{  true,   152, "getting stuck while briefing is about to start", 1, pq1vgaSignatureBriefingGettingStuck, pq1vgaPatchBriefingGettingStuck },
	{  true,   341, "put gun in locker bug",                          1, pq1vgaSignaturePutGunInLockerBug,    pq1vgaPatchPutGunInLockerBug },
	{  true,   500, "map save/restore bug",                           2, pq1vgaSignatureMapSaveRestoreBug,    pq1vgaPatchMapSaveRestoreBug },
	{  true,   928, "Narrator lockup fix",                            1, sciNarratorLockupSignature,          sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Police Quest 3

// The player can give the locket to Marie on day 6, which was supposed to grant
// 10 points. Sadly no version did so, so it was not possible to complete the game
// with a perfect score (460 points).

// Those 10 points are mentioned in the official Sierra hint book for day 6,
// which is why we consider this to be accurate.

// This bug occurs of course also, when using the original interpreter.

// We fix this issue by granting those 10 points.

// Applies to at least: English PC floppy, English Amiga, German PC floppy
// Responsible method: giveLocket::changeState(1), script 36
// Fixes bug: #9862
static const uint16 pq3SignatureGiveLocketPoints[] = {
	// selectors hardcoded in here, it seems all game versions use the same selector ids
	0x39, 0x20,                          // pushi 20h (state)
	0x78,                                // push1
	0x78,                                // push1
	0x39, 0x43,                          // pushi 43h (at)
	0x78,                                // push1
	SIG_MAGICDWORD,
	0x39, 0x25,                          // pushi 25h
	0x81, 0x09,                          // lag global[9]
	0x4a, 0x06,                          // send 06 - Inv::at(25h)
	0x4a, 0x06,                          // send 06 - locket::state(1)
	0x38, SIG_UINT16(0x009b),            // pushi 009bh (owner)
	0x76,                                // push0
	0x39, 0x43,                          // pushi 43h (at)
	0x78,                                // push1
	0x39, 0x25,                          // pushi 25h
	0x81, 0x09,                          // lag global[9]
	0x4a, 0x06,                          // send 06 - Inv:at(25h)
	0x4a, 0x04,                          // send 04 - locket::owner
	SIG_END
};

static const uint16 pq3PatchGiveLocketPoints[] = {
	// new code for points, 9 bytes
	0x7a,                                // push2
	0x38, PATCH_UINT16(0x00ff),          // pushi 0x00ff - using last flag slot, seems to be unused
	0x39, 0x0a,                          // pushi 10d - 10 points
	0x45, 0x06, 0x04,                    // callb [export 6 of script 0], 4
	// original code
	0x39, 0x20,                          // pushi 20h (state)
	0x78,                                // push1
	0x78,                                // push1
	0x39, 0x43,                          // pushi 43h (at)
	0x78,                                // push1
	0x39, 0x25,                          // pushi 25h
	0x81, 0x09,                          // lag global[9]
	0x4a, 0x06,                          // send 06 - Inv::at(25h)
	0x4a, 0x06,                          // send 06 - locket::state(1)
	// optimized code, saving 9 bytes
	0x38, PATCH_UINT16(0x009b),          // pushi 009bh (owner)
	0x76,                                // push0
	0x4a, 0x04,                          // send 04 - locket::owner
	PATCH_END
};

// The doctor's mouth moves too fast in room 36. doctorMouth:cyleSpeed is set to
//  one, the maximum speed, unlike any other inset in the game. Most insets use
//  the default speed of six and almost all the rest use an even slower speed.
//  We set the doctor's mouth to the default speed.
//
// Applies to: All versions
// Responsible method: insetDoctor:init
// Fixes bug: #10255
static const uint16 pq3SignatureDoctorMouthSpeed[] = {
	0x38, SIG_MAGICDWORD,                // pushi cyleSpeed
	      SIG_SELECTOR16(cycleSpeed),
	0x78,                                // push1
	0x78,                                // push1
	SIG_ADDTOOFFSET(+13),
	0x4a, 0x1c,                          // send 1c [ doctorMouth ... cycleSpeed: 1 ... ]
	SIG_END
};

static const uint16 pq3PatchDoctorMouthSpeed[] = {
	0x32, PATCH_UINT16(0x0002),          // jmp 0002
	PATCH_ADDTOOFFSET(+15),
	0x4a, 0x16,                          // send 16 [ don't set cycleSpeed, use default (6) ]
	PATCH_END
};

// The house fire on day six reoccurs if you return to the hospital. Flag 66
//  triggers the fire sequence and is always set when leaving the hospital on
//  day 6. It's then cleared when arriving at the fire.
//
// We add a test for flag 57, which is set at the fire, so that flag 66 isn't
//  set a second time. This is also what Sierra did in later versions.
//
// Applies to: English PC VGA Floppy
// Responsible method: outHospital:changeState(6)
// Fixes bug: #11089
static const uint16 pq3SignatureHouseFireRepeats[] = {
	0x30, SIG_UINT16(0x0068),            // bnt 0068 [ state 7 ]
	SIG_ADDTOOFFSET(+82),
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0006),            // bnt 0006 [ don't set fire-started flag ]
	0x78,                                // push1
	0x39, 0x42,                          // pushi 42 [ flag 66 ]
	0x45, 0x09, 0x02,                    // callb proc0_9 [ set fire-started flag ]
	0x38, SIG_SELECTOR16(newRoom),       // pushi newRoom
	0x78,                                // push1
	0x39, 0x19,                          // pushi 19
	0x81, 0x02,                          // lag 02
	0x4a, 0x06,                          // send 06 [ rm033 newRoom: 25 ]
	0x32, SIG_UINT16(0x004c),            // jmp 004c [ end of method ]
	0x3c,                                // dup
	0x35, 0x07,                          // ldi 07
	0x1a,                                // eq?
	0x30, SIG_UINT16(0x0007),            // bnt 0007 [ state 8 ]
	0x35, 0x01,                          // ldi 01
	0x65, 0x10,                          // aTop cycles
	0x32, SIG_UINT16(0x003e),            // jmp 003e [ end of method ]
	SIG_END
};

static const uint16 pq3PatchHouseFireRepeats[] = {
	0x30, PATCH_UINT16(0x006c),          // bnt 006c [ state 7 ]
	PATCH_ADDTOOFFSET(+82),
	0x31, 0x0e,                          // bnt 0e [ don't set fire-started flag ]
	0x78,                                // push1
	0x39, 0x39,                          // pushi 39 [ flag 57 ]
	0x45, 0x0a, 0x02,                    // callb proc0_10 [ have you been to the fire? ]
	0x2f, 0x06,                          // bt 06 [ don't set fire-started flag ]
	0x78,                                // push1
	0x39, 0x42,                          // pushi 42 [ flag 66 ]
	0x45, 0x09, 0x02,                    // callb proc0_9 [ set fire-started flag ]
	0x38, PATCH_SELECTOR16(newRoom),     // pushi newRoom
	0x78,                                // push1
	0x39, 0x19,                          // pushi 19
	0x81, 0x02,                          // lag 02
	0x4a, 0x06,                          // send 06 [ rm033 newRoom: 25 ]
	0x3c,                                // dup
	0x35, 0x07,                          // ldi 07
	0x1a,                                // eq?
	0x31, 0x04,                          // bnt 04 [ state 8 ]
	0x35, 0x01,                          // ldi 01
	0x65, 0x10,                          // aTop cycles
	PATCH_END
};

//          script, description,                                 signature                     patch
static const SciScriptPatcherEntry pq3Signatures[] = {
	{  true, 33, "prevent house fire repeating",              1, pq3SignatureHouseFireRepeats, pq3PatchHouseFireRepeats },
	{  true, 36, "give locket missing points",                1, pq3SignatureGiveLocketPoints, pq3PatchGiveLocketPoints },
	{  true, 36, "doctor mouth speed",                        1, pq3SignatureDoctorMouthSpeed, pq3PatchDoctorMouthSpeed },
	SCI_SIGNATUREENTRY_TERMINATOR
};


#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Police Quest 4

// Add support for simultaneous speech & subtitles to the in-game UI.
// The original game code has code paths for lo-res mode but only creates the
// buttons in hi-res mode, so the lo-res code paths are removed to gain more
// space for the patch.
// Applies to: English CD
// Responsible method: iconText::init, iconText::select
static const uint16 pq4CdSpeechAndSubtitlesSignature[] = {
	// iconText::init
	0x76,                         // push0
	0x43, 0x22, SIG_UINT16(0x00), // callk IsHiRes
	0x18,                         // not
	0x31, 0x05,                   // bnt [skip next 2 opcodes, when hires]
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x2661),     // ldi 9825
	0x65, 0x78,                   // aTop mainView
	0x89, 0x5a,                   // lsg global[$5a]
	0x35, 0x01,                   // ldi 1
	0x12,                         // and
	0x30, SIG_UINT16(0x1b),       // bnt [when in speech mode]
	0x76,                         // push0
	0x43, 0x22, SIG_UINT16(0x00), // callk IsHiRes
	SIG_ADDTOOFFSET(+45),         // skip over the remaining code
	0x38, SIG_SELECTOR16(init),   // pushi init ($93)
	0x76,                         // push0
	0x59, 0x01,                   // &rest 01
	0x57, 0x8f, SIG_UINT16(0x04), // super GCItem, 4
	0x48,                         // ret

	// iconText::select
	0x38, SIG_SELECTOR16(select), // pushi select ($1c4)
	0x76,                         // push0
	0x59, 0x01,                   // &rest 01
	0x57, 0x8f, SIG_UINT16(0x04), // super GCItem, 4
	0x89, 0x5a,                   // lsg global[$5a]
	0x35, 0x02,                   // ldi 2
	0x12,                         // and
	0x30, SIG_UINT16(0x001f),     // bnt [to currently-in-text-mode code]
	SIG_ADDTOOFFSET(+67),         // skip over the rest
	0x48,                         // ret
	SIG_END
};

// Note: global 0x5a may contain the following values:
// 1: subtitles
// 2: speech
// 3: both
static const uint16 pq4CdSpeechAndSubtitlesPatch[] = {
	// iconText::init
	0x76,                           // push0
	0x41, 0x02, PATCH_UINT16(0x00), // call [our new subroutine which sets view+loop+cel], 0
	0x33, 0x40,                     // jmp [to original init, super GCItem call]
	// new code for setting view+loop+cel
	//
	0x89, 0x5a,                     // lsg global[$5a]
	0x35, 0x03,                     // ldi 3
	0x1a,                           // eq?
	0x31, 0x05,                     // bnt [skip over the view modification code]
	//
	// Mode 3: speech+subtitles
	0x34, PATCH_UINT16(0x2aec),     // ldi 10988 (our new injected view, speech + subtitles)
	0x33, 0x03,                     // jmp [skip over the view modification code]
	// Mode 1: subtitles
	0x34, PATCH_UINT16(0x2aeb),     // ldi 10987 (subtitles view)
	//
	// common code for speech+subtitles / subtitles mode
	0x65, 0x78,                     // aTop mainView
	0x76,                           // push0
	0x69, 0x7a,                     // sTop mainLoop
	//
	0x89, 0x5a,                     // lsg global[$5a]
	0x35, 0x02,                     // ldi 2
	0x1a,                           // eq?
	0x31, 0x09,                     // bnt [skip over follow up code]
	//
	// Mode 2: speech
	0x34, PATCH_UINT16(0x2ae6),     // ldi 10982 (speech view)
	0x65, 0x78,                     // aTop mainView
	0x35, 0x0f,                     // ldi 15
	0x65, 0x7a,                     // aTop mainLoop
	0x48,                           // ret
	//
	PATCH_ADDTOOFFSET(+38),         // skip to iconText::select

	// iconText::select
	PATCH_ADDTOOFFSET(+10),         // skip over the super code
	0xc1, 0x5a,                     // +ag global[$5a]
	0xa1, 0x5a,                     // sag global[$5a]
	0x36,                           // push
	0x35, 0x04,                     // ldi 4
	0x28,                           // uge?
	0x31, 0x03,                     // bnt [skip over follow up code]
	0x78,                           // push1
	0xa9, 0x5a,                     // ssg global[$5a]
	0x76,                           // push0
	0x41, 0x99, PATCH_UINT16(0x00), // call [our new subroutine which sets view+loop+cel, effectively -103], 0
	0x33, 0x2f,                     // jmp [to end of original select, show call]
	PATCH_END
};

// When showing the red shoe to Barbie, after showing the police badge but
// before exhausting the normal dialogue tree, the game plays the expected
// dialogue but fails to award points or set an internal flag indicating this
// interaction has occurred (which is needed to progress in the game).
//
// This is because the game checks global[$9a] (dialogue progress flag) instead
// of local[3] (badge shown flag) when interacting with Barbie. The game uses
// the same `shoeShoe::changeState(0)` method for showing the shoe to the young
// woman at the bar earlier in the game, and checks local[3] then, so just
// check local[3] in both cases to prevent the game from appearing to be in an
// unwinnable state just because the player interacted in the "wrong" order.
//
// Applies to at least: English floppy, German floppy, English CD, German CD
// Fixes bug: #9849
static const uint16 pq4BittyKittyShowBarieRedShoeSignature[] = {
	// stripper::noun check is for checking, if police badge was shown
	SIG_MAGICDWORD,
	0x89, 0x9a,                         // lsg global[$9a]
	0x35, 0x02,                         // ldi 2
	0x1e,                               // gt?
	0x30, SIG_UINT16(0x0028),           // bnt [skip 2 points code]
	0x39, SIG_SELECTOR8(points),        // pushi points ($61)
	SIG_END
};

static const uint16 pq4BittyKittyShowBarbieRedShoePatch[] = {
	0x83, 0x03,                         // lal local[3]
	0x30, PATCH_UINT16(0x002b),         // bnt [skip 2 points code]
	0x33, 0x01,                         // jmp 1 (waste some bytes)
	PATCH_END
};

// In PQ4, scripts for the city hall action sequences use `ticks`. These
// continue to count down even during inventory interaction, so if the user is
// unable to find the correct inventory item quickly enough for the sequence,
// the game will immediately end with a "game over" once they close the
// inventory and the main game loop resumes. This can seem like a game bug, so
// we change these sequences to use `seconds`, which only tick down by 1 when
// the game returns to the main loop and the wall time has changed, even if many
// seconds have actually elapsed. However, since `seconds` uses absolute
// hardware clock time with a granularity of 1 second, "one" second can actually
// be less than one second if the timer is set in between hardware clock
// seconds, so the values are increased slightly from their equivalent tick
// values to compensate for this.
//
// TODO: The object structure changed in PQ4CD so ticks moved from 0x20 to 0x22.
// Additional signatures/patches will need to be added for CD version.
//
// Applies to at least: English Floppy, German floppy
// Responsible method: metzAttack::changeState(2) - 120 ticks (player needs to draw gun)
//                     stickScr::changeState(0) - 180 ticks (player needs to tell enemy to drop gun)
//                     dropStick::changeState(5) - 120 ticks (player needs to tell enemy to turn around)
//                     turnMetz::changeState(5) - 600/420 ticks (player needs to cuff Metz)
//                     all in script 390
static const uint16 pq4FloppyCityHallDrawGunTimerSignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x08), // send 8
	0x32,                   // jmp [ret]
	SIG_ADDTOOFFSET(+8),    // skip over some code
	0x35, 0x78,             // ldi $78 (120)
	0x65, 0x20,             // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallDrawGunTimerPatch[] = {
	PATCH_ADDTOOFFSET(+12), // send 8, jmp, skip over some code
	0x35, 0x05,             // ldi 5 (120t/2s -> 5s)
	0x65, 0x1c,             // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallTellEnemyDropWeaponTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0xb4), // ldi $b4 (180)
	0x65, 0x20,             // aTop ticks
	0x32, SIG_UINT16(0x5e), // jmp [to ret]
	SIG_END
};

static const uint16 pq4FloppyCityHallTellEnemyDropWeaponTimerPatch[] = {
	0x34, PATCH_UINT16(0x05), // ldi 5 (180t/3s -> 5s)
	0x65, 0x1c,               // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallTellEnemyTurnAroundTimerSignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x04), // send 4
	0x35, 0x78,             // ldi $78 (120)
	0x65, 0x20,             // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallTellEnemyTurnAroundTimerPatch[] = {
	PATCH_ADDTOOFFSET(+3), // send 4
	0x35, 0x03,            // ldi 3 (120t/2s -> 3s)
	0x65, 0x1c,            // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallCuffEnemyTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x0258), // ldi $258 (600)
	0x65, 0x20,               // aTop ticks
	SIG_ADDTOOFFSET(+3),
	0x34, SIG_UINT16(0x01a4), // ldi $1a4 (420)
	0x65, 0x20,               // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallCuffEnemyTimerPatch[] = {
	0x34, PATCH_UINT16(0x0a), // ldi 10 (600t/10s)
	0x65, 0x1c,               // aTop seconds
	PATCH_ADDTOOFFSET(+3),
	0x34, PATCH_UINT16(0x07), // ldi 7 (420t/7s)
	0x65, 0x1c,               // aTop seconds
	PATCH_END
};

// The end game action sequence also uses ticks instead of seconds. See the
// description of city hall action sequence issues for more information.
//
// Applies to at least: English Floppy, German floppy, English CD
// Responsible method: comeInLast::changeState(11)
static const uint16 pq4LastActionHeroTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x012c),  // ldi $12c (300)
	0x65, SIG_ADDTOOFFSET(+1), // aTop ticks ($20 for floppy, $22 for CD)
	SIG_END
};

static const uint16 pq4LastActionHeroTimerPatch[] = {
	0x34, PATCH_UINT16(0x0005),               // ldi 5 (300t/5s)
	0x65, PATCH_GETORIGINALBYTEADJUST(4, -4), // aTop seconds
	PATCH_END
};

//          script, description,                                          signature                                           patch
static const SciScriptPatcherEntry pq4Signatures[] = {
	{  true,     6, "disable video benchmarking",                      1, sci2BenchmarkSignature,                             sci2BenchmarkPatch },
	{  true,     9, "add speech+subtitles to in-game UI",              1, pq4CdSpeechAndSubtitlesSignature,                   pq4CdSpeechAndSubtitlesPatch },
	{  true,   315, "fix missing points showing barbie the red shoe",  1, pq4BittyKittyShowBarieRedShoeSignature,             pq4BittyKittyShowBarbieRedShoePatch },
	{  true,   390, "change floppy city hall use gun timer",           1, pq4FloppyCityHallDrawGunTimerSignature,             pq4FloppyCityHallDrawGunTimerPatch },
	{  true,   390, "change floppy city hall say 'drop weapon' timer", 1, pq4FloppyCityHallTellEnemyDropWeaponTimerSignature, pq4FloppyCityHallTellEnemyDropWeaponTimerPatch },
	{  true,   390, "change floppy city hall say 'turn around' timer", 1, pq4FloppyCityHallTellEnemyTurnAroundTimerSignature, pq4FloppyCityHallTellEnemyTurnAroundTimerPatch },
	{  true,   390, "change floppy city hall use handcuffs timer",     1, pq4FloppyCityHallCuffEnemyTimerSignature,           pq4FloppyCityHallCuffEnemyTimerPatch },
	{  true,   755, "change last action sequence timer",               1, pq4LastActionHeroTimerSignature,                    pq4LastActionHeroTimerPatch },
	{  true, 64918, "fix Str::strip in floppy version",                1, sci2BrokenStrStripSignature,                        sci2BrokenStrStripPatch },
	{  true, 64928, "Narrator lockup fix",                             1, sciNarratorLockupSignature,                         sciNarratorLockupPatch },
	{  true, 64990, "increase number of save games (1/2)",             1, sci2NumSavesSignature1,                             sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",             1, sci2NumSavesSignature2,                             sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",                 1, sci2ChangeDirSignature,                             sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Police Quest: SWAT

// The init code that runs when PQ:SWAT starts up unconditionally resets the
// master sound volume to 127, but the game should always use the volume stored
// in ScummVM.
// Applies to at least: English CD
// Fixes bug: #9700
static const uint16 pqSwatVolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(masterVolume), // pushi masterVolume
	0x78,                               // push1
	0x39, 0x7f,                         // pushi $7f
	0x54, SIG_UINT16(0x0006),           // self 6
	SIG_END
};

static const uint16 pqSwatVolumeResetPatch[] = {
	0x32, PATCH_UINT16(0x0006),         // jmp 6 [past volume reset]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry pqSwatSignatures[] = {
	{  true,     0, "disable volume reset on startup (1/2)",       1, pqSwatVolumeResetSignature,        pqSwatVolumeResetPatch },
	{  true,     1, "disable volume reset on startup (2/2)",       1, sci2VolumeResetSignature,          sci2VolumeResetPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
//  At the healer's house there is a bird's nest up on the tree.
//   The player can throw rocks at it until it falls to the ground.
//   The hero will then grab the item, that is in the nest.
//
//  When running is active, the hero will not reach the actual destination
//   and because of that, the game will get stuck.
//
//  We just change the coordinate of the destination slightly, so that walking,
//   sneaking and running work.
//
//  This bug was fixed by Sierra at least in the Japanese PC-9801 version.
// Applies to at least: English floppy (1.000, 1.012)
// Responsible method: pickItUp::changeState (script 54)
// Fixes bug: #6407
static const uint16 qfg1egaSignatureThrowRockAtNest[] = {
	0x4a, 0x04,                         // send 04 (nest::x)
	0x36,                               // push
	SIG_MAGICDWORD,
	0x35, 0x0f,                         // ldi 0f (15d)
	0x02,                               // add
	0x36,                               // push
	SIG_END
};

static const uint16 qfg1egaPatchThrowRockAtNest[] = {
	PATCH_ADDTOOFFSET(+3),
	0x35, 0x12,                         // ldi 12 (18d)
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry qfg1egaSignatures[] = {
	{  true,    54, "throw rock at nest while running",            1, qfg1egaSignatureThrowRockAtNest,     qfg1egaPatchThrowRockAtNest },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 215 of qfg1vga pointBox::doit actually processes button-presses
//   during fighting with monsters. It strangely also calls kGetEvent. Because
//   the main User::doit also calls kGetEvent it's pure luck, where the event
//   will hit. It's the same issue as in freddy pharkas and if you turn DOSBox
//   to max cycles, sometimes clicks also won't get registered. Strangely it's
//   not nearly as bad as in our sci, but these differences may be caused by
//   timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
// Applies to at least: English floppy
// Responsible method: pointBox::doit
// Fixes bug: #5038
static const uint16 qfg1vgaSignatureFightEvents[] = {
	0x39, SIG_MAGICDWORD,
	SIG_SELECTOR8(new),                 // pushi new
	0x76,                               // push0
	0x51, 0x07,                         // class Event
	0x4a, 0x04,                         // send 04 - call Event::new
	0xa5, 0x00,                         // sat temp[0]
	0x78,                               // push1
	0x76,                               // push0
	0x4a, 0x04,                         // send 04 - read Event::x
	0xa5, 0x03,                         // sat temp[3]
	0x76,                               // push0 (selector y)
	0x76,                               // push0
	0x85, 0x00,                         // lat temp[0]
	0x4a, 0x04,                         // send 04 - read Event::y
	0x36,                               // push
	0x35, 0x0a,                         // ldi 0a
	0x04,                               // sub (poor mans localization) ;-)
	SIG_END
};

static const uint16 qfg1vgaPatchFightEvents[] = {
	0x38, PATCH_SELECTOR16(curEvent), // pushi curEvent (15a)
	0x76,                            // push0
	0x81, 0x50,                      // lag global[50]
	0x4a, 0x04,                      // send 04 - read User::curEvent -> needs one byte more than previous code
	0xa5, 0x00,                      // sat temp[0]
	0x78,                            // push1
	0x76,                            // push0
	0x4a, 0x04,                      // send 04 - read Event::x
	0xa5, 0x03,                      // sat temp[3]
	0x76,                            // push0 (selector y)
	0x76,                            // push0
	0x85, 0x00,                      // lat temp[0]
	0x4a, 0x04,                      // send 04 - read Event::y
	0x39, 0x00,                      // pushi 00
	0x02,                            // add (waste 3 bytes) - we don't need localization, User::doit has already done it
	PATCH_END
};

// Script 814 of QFG1VGA is responsible for showing dialogs. However, the death
// screen message shown when the hero dies in room 64 (ghost room) is too large
// (254 chars long). Since the window header and main text are both stored in
// temp space, this is an issue, as the scripts read the window header, then the
// window text, which erases the window header text because of its length. To
// fix that, we allocate more temp space and move the pointer used for the
// window header a little bit, wherever it's used in script 814.
// Fixes bug: #6139.

// Patch 1: Increase temp space
static const uint16 qfg1vgaSignatureTempSpace[] = {
	SIG_MAGICDWORD,
	0x3f, 0xba,                         // link 0xba
	0x87, 0x00,                         // lap param[0]
	SIG_END
};

static const uint16 qfg1vgaPatchTempSpace[] = {
	0x3f, 0xca,                         // link 0xca
	PATCH_END
};

// Patch 2: Move the pointer used for the window header a little bit
static const uint16 qfg1vgaSignatureDialogHeader[] = {
	SIG_MAGICDWORD,
	0x5b, 0x04, 0x80,                   // lea temp[0x80]
	0x36,                               // push
	SIG_END
};

static const uint16 qfg1vgaPatchDialogHeader[] = {
	0x5b, 0x04, 0x90,                   // lea temp[0x90]
	PATCH_END
};

// When clicking on the crusher in room 331, Ego approaches him to talk to him,
// an action that is handled by moveToCrusher::changeState in script 331. The
// scripts set Ego to move close to the crusher, but when Ego is sneaking instead
// of walking, the target coordinates specified by script 331 are never reached,
// as Ego is making larger steps, and never reaches the required spot. This is an
// edge case that can occur when Ego is set to sneak. Normally, when clicking on
// the crusher, ego is supposed to move close to position 79, 165. We change it
// to 85, 165, which is not an edge case thus the freeze is avoided.
// Fixes bug: #6180
static const uint16 qfg1vgaSignatureMoveToCrusher[] = {
	SIG_MAGICDWORD,
	0x51, 0x1f,                         // class Motion
	0x36,                               // push
	0x39, 0x4f,                         // pushi 4f (79 - x)
	0x38, SIG_UINT16(0x00a5),           // pushi 00a5 (165 - y)
	0x7c,                               // pushSelf
	SIG_END
};

static const uint16 qfg1vgaPatchMoveToCrusher[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x55,                         // pushi 55 (85 - x)
	PATCH_END
};

// Clicking "Do" on Crusher in room 331 while standing near the card table locks
//  up the game. This is due to a script bug which also occurs in the original.
//  This is unrelated to bug #6180 in which clicking "Do" on Crusher while
//  sneaking also locks up.
//
// rm331:doit sets ego's script to cardScript when ego enters a rectangle around
//  the card table and sets ego's script to none when exiting. This assumes that
//  ego can't have a different script set. Clicking "Do" on Crusher sets ego's
//  script to moveToCrusher. If moveToCrusher causes ego to enter or exit the
//  table's rectangle then the script will be stopped. When ego reaches Crusher
//  he will have no script to continue the sequence and the game will be stuck
//  in handsOff mode.
//
// We fix this by skipping the card table code in rm331:doit if ego already has
//  a script other than cardScript. This prevents the card game from interfering
//  with running scripts such as moveToCrusher.
//
// This bug was fixed in the Macintosh version by changing the card game to no
//  longer involve setting ego's script and removing the code from rm331:doit.
//
// Applies to: PC Floppy
// Responsible method: rm331:doit
// Fixes bug: #10826
static const uint16 qfg1vgaSignatureCrusherCardGame[] = {
	SIG_MAGICDWORD,
	0x63, 0x12,                         // pToa script
	0x31, 0x02,                         // bnt 02
	0x33, 0x28,                         // jmp 28 [ card table location tests ]
	0x38, SIG_SELECTOR16(script),       // pushi script
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 4 [ ego:script? ]
	0x31, 0x04,                         // bnt 04
	0x35, 0x00,                         // ldi 00 [ does nothing ]
	0x33, 0x1a,                         // jmp 1a [ card table location tests ]
	SIG_ADDTOOFFSET(+113),
	0x39, SIG_SELECTOR8(doit),          // pushi doit [ pc version only ]
	SIG_END
};

static const uint16 qfg1vgaPatchCrusherCardGame[] = {
	0x38, PATCH_SELECTOR16(script),     // pushi script
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 4 [ ego:script? ]
	0x31, 0x06,                         // bnt 06
	0x74, PATCH_UINT16(0x0ee4),         // lofss cardScript
	0x1a,                               // eq?
	0x31, 0x75,                         // bnt 75 [ skip card table location tests ]
	0x63, 0x12,                         // pToa script
	0x2f, 0x1a,                         // bt 1a [ card table location tests ]
	PATCH_END
};

// Same pathfinding bug as above, where Ego is set to move to an impossible
// spot when sneaking. In GuardsTrumpet::changeState, we change the final
// location where Ego is moved from 111, 111 to 116, 116.
// target coordinate is really problematic here.
//
// 114, 114 works when the speed slider is all the way up, but doesn't work
// when the speed slider is not.
//
// It seems that this bug was fixed by Sierra for the Macintosh version.
//
// Applies to at least: English PC floppy
// Responsible method: GuardsTrumpet::changeState(8)
// Fixes bug: #6248
static const uint16 qfg1vgaSignatureMoveToCastleGate[] = {
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	SIG_MAGICDWORD,
	0x36,                               // push
	0x39, 0x6f,                         // pushi 6f (111d)
	0x3c,                               // dup (111d) - coordinates 111, 111
	0x7c,                               // pushSelf
	SIG_END
};

static const uint16 qfg1vgaPatchMoveToCastleGate[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x74,                         // pushi 74 (116d), changes coordinates to 116, 116
	PATCH_END
};

// Typo in the original Sierra scripts
// Looking at a cheetaur resulted in a text about a Saurus Rex
// The code treats both monster types the same.
// Applies to at least: English floppy
// Responsible method: smallMonster::doVerb
// Fixes bug: #6249
static const uint16 qfg1vgaSignatureCheetaurDescription[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x01b8),           // ldi 01b8
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt 16
	0x38, SIG_SELECTOR16(say),          // pushi say (0127h)
	0x39, 0x06,                         // pushi 06
	0x39, 0x03,                         // pushi 03
	0x78,                               // push1
	0x39, 0x12,                         // pushi 12 -> monster type Saurus Rex
	SIG_END
};

static const uint16 qfg1vgaPatchCheetaurDescription[] = {
	PATCH_ADDTOOFFSET(+14),
	0x39, 0x11,                         // pushi 11 -> monster type cheetaur
	PATCH_END
};

// In the "funny" room (Yorick's room) in QfG1 VGA, pulling the chain and
//  then pressing the button on the right side of the room results in
//  a broken game. This also happens in SSCI.
// Problem is that the Sierra programmers forgot to disable the door that
//  gets opened by pulling the chain. So when ego falls down and then
//  rolls through the door, one method thinks that the player walks through
//  it and acts that way and the other method is still doing the roll animation.
// The timer that closes the door (door11) is local[5]. Setting it to 1 during
//  happyFace::changeState(0) stops door11::doit from calling goTo6::init, so
//  the whole issue is stopped from happening.
//
// Applies to at least: English floppy
// Responsible method: happyFace::changeState, door11::doit
// Fixes bug: #6181
static const uint16 qfg1vgaSignatureFunnyRoomFix[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0025),           // bnt 0025 [next state]
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal local[4e]
	SIG_END
};

static const uint16 qfg1vgaPatchFunnyRoomFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x2e, PATCH_UINT16(0x0029),         // bt 0029 [next state] - saves 4 bytes
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal local[4e]
	0xa3, 0x05,                         // sal local[5] (set to 1)
	0xa3, 0x05,                         // and again to make absolutely sure (actually to waste 2 bytes)
	PATCH_END
};

// In Yorick's room, room 96, walking in certain spots in front of the rightmost
//  door locks up the game. This also occurs in Sierra's interpreter.
//
// rm96:doit runs the script goTo2 when ego enters a rect in front of the door.
//  This rect is low enough that ego can collide with the door's boundary
//  obstacle on the right and prevent goTo2 from restoring control to the user.
//
// We fix this by raising the bottom of the door rect. Sierra fixed this bug in
//  the Mac version by rewriting the door code, switching to control areas, and
//  tweaking the sizes and locations of all the relevant objects.
//
// Applies to: PC Floppy
// Responsible method: rm96:doit
// Fixes bug #6410
static const uint16 qfg1vgaSignatureYorickDoorTwoRect[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0135),               // pushi 0135 [ x = 309 ]
	0x39, 0x64,                             // pushi 64   [ y = 100 ]
	0x38, SIG_UINT16(0x013f),               // pushi 013f [ x = 319 ]
	0x39, 0x70,                             // pushi 70   [ y = 112 ]
	SIG_END
};

static const uint16 qfg1vgaPatchYorickDoorTwoRect[] = {
	PATCH_ADDTOOFFSET(+8),
	0x39, 0x6d,                             // pushi 6d [ y = 109 ]
	PATCH_END
};

// The player is able to buy (and also steal) potions in the healer's hut
//  Strangely Sierra delays the actual buy/get potion code for 60 ticks
//  Why they did that is unknown. The code is triggered anyway only after
//  the relevant dialog boxes are closed.
//
// This delay causes problems in case the user quickly enters the inventory.
// That's why we change the amount of ticks to 1, so that the remaining states
//  are executed right after the dialog boxes are closed.
//
// Applies to at least: English floppy
// Responsible method: cueItScript::changeState
// Fixes bug: #6706
static const uint16 qfg1vgaSignatureHealerHutNoDelay[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt 07 [next state]
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c (60 ticks)
	0x65, 0x20,                         // aTop ticks
	0x32,                               // jmp [end of method]
	SIG_END
};

static const uint16 qfg1vgaPatchHealerHutNoDelay[] = {
	PATCH_ADDTOOFFSET(+9),
	0x35, 0x01,                         // ldi 01 (1 tick only, so that execution will resume as soon as dialog box is closed)
	PATCH_END
};

// When following the white stag, you can actually enter the 2nd room from the
//  mushroom/fairy location, which results in ego entering from the top. When
//  you then throw a dagger at the stag, one animation frame will stay on
//  screen, because of a script bug.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: stagHurt::changeState
// Fixes bug: #6135
static const uint16 qfg1vgaSignatureWhiteStagDagger[] = {
	0x87, 0x01,                         // lap param[1]
	0x65, 0x14,                         // aTop state
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt [next parameter check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb [export 2 of script 0], 0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(say),          // pushi say (0127h)
	0x39, 0x05,                         // pushi 05
	0x39, 0x03,                         // pushi 03
	0x39, 0x51,                         // pushi 51h
	0x76,                               // push0
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[5Bh] -> qg1Messager
	0x4a, 0x0e,                         // send 0Eh -> qg1Messager::say(3, 51h, 0, 0, stagHurt)
	0x33, 0x12,                         // jmp [end of method]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt [end of method]
	0x38,                               // pushi...
	SIG_ADDTOOFFSET(+11),
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 qfg1vgaPatchWhiteStagDagger[] = {
	PATCH_ADDTOOFFSET(+4),
	0x2f, 0x05,                         // bt [next check] (state != 0)
	// state = 0 code
	0x35, 0x01,                         // ldi 1
	0x65, 0x1a,                         // aTop cycles
	0x48,                               // ret
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt [state = 2 code]
	// state = 1 code
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb [export 2 of script 0], 0
	0x38, PATCH_SELECTOR16(say),        // pushi say (0127h)
	0x39, 0x05,                         // pushi 05
	0x39, 0x03,                         // pushi 03
	0x39, 0x51,                         // pushi 51h
	0x76,                               // push0
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[5Bh] -> qg1Messager
	0x4a, 0x0e,                         // send 0Eh -> qg1Messager::say(3, 51h, 0, 0, stagHurt)
	0x48,                               // ret
	// state = 2 code
	PATCH_ADDTOOFFSET(+13),
	0x48,                               // ret (remove toss)
	PATCH_END
};

// The dagger range has a script bug that can freeze the game or cause Brutus
// to kill hero even after Brutus dies.
//
// When Bruno leaves, a 300 tick countdown starts. If hero kills Brutus or
// leaves room 73 within those 300 ticks, then the game is left in a broken
// state. For the rest of the game, if hero ever returns to the dagger range
// from the east or west during the first half of the day, then the game will
// freeze or Brutus, dead or not, will kill hero.
//
// Special thanks, credits and kudos to sluicebox, who did a ton of research on
// this and even found this game bug originally.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: brutusWaits::changeState
// Fixes bug: #9558
static const uint16 qfg1vgaSignatureBrutusScriptFreeze[] = {
	0x78,                               // push1
	0x38, SIG_UINT16(0x0144),           // pushi 144h (324d)
	0x45, 0x05, 0x02,                   // callb [export 5 of script 0], 2
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x012c),           // ldi 12Ch (300d)
	0x65, 0x20,                         // aTop ticks
	SIG_END
};

static const uint16 qfg1vgaPatchBrutusScriptFreeze[] = {
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 7 bytes)
	0x35, 0x00,                         // ldi 0
	0x35, 0x00,                         // ldi 0
	PATCH_END
};

// Patch the speed test so that it always ends up at the highest level. This
//  improves the detail in Yorick's room (96), and slightly changes the timing
//  in other rooms. This is compatible with PC and Mac versions which use
//  significantly different tests and calculations.
//
// Applies to: PC Floppy, Mac Floppy
// Responsible method: speedTest:changeState(2)
static const uint16 qfg1vgaSignatureSpeedTest[] = {
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime, 0
	SIG_MAGICDWORD,
	0x36,                               // push
	0x83, 0x01,                         // lal 01
	0x04,                               // sub
	0xa3, 0x00,                         // sal 00
	SIG_END
};

static const uint16 qfg1vgaPatchSpeedTest[] = {
	0x35, 0x00,                         // ldi 00 [ local0 = 0, the best result ]
	0x33, 0x04,                         // jmp 04
	PATCH_END
};

// QFG1VGA has a bug where exceeding the weight limit during certain scenes
//  breaks the character screen for the rest of the game. Picking mushrooms,
//  searching cheetaurs, and fetching the seed are among the vulnerable actions.
//
// When adding inventory, ego:get displays a warning if the new items exceed the
//  weight limit. If this happens while qfgMessager is displaying a message
//  then both will display at the same time but only one will be disposed. This
//  leaves an extra entry in the kernel's window list for the rest of the game.
//  kDisplay then sends text to the wrong window, breaking the character screen
//  and others, and prevents the player from ever viewing their stats.
//
// We fix this by adding a check to ego:get that skips displaying messages if a
//  dialog already exists. This is what Sierra did in the Mac version after
//  reverting the scene-specific patches they issued for the PC version.
//
// Applies to: PC Floppy
// Responsible method: ego:get
// Fixes bug: #10942
static const uint16 qfg1vgaSignatureInventoryWeightWarn[] = {
	0x8f, 0x00,                         // lsp 00
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 04
	0x35, 0x01,                         // ldi 01
	0x33, 0x02,                         // jmp 02
	0x87, 0x02,                         // lap 02
	0xa5, SIG_MAGICDWORD, 0x01,         // sat 01
	0x38, SIG_UINT16(0x024d),           // pushi amount [ hard-coded for PC ]
	0x76,                               // push0
	0x85, 0x00,                         // lat 00
	0x4a, 0x04,                         // send 04 [ temp0 amount? ]
	0xa5, 0x02,                         // sat 02
	SIG_ADDTOOFFSET(+0x0092),
	0x8d, 0x01,                         // lst 01
	SIG_END
};

static const uint16 qfg1vgaPatchInventoryWeightWarn[] = {
	0x87, 0x00,                         // lap 00
	0x78,                               // push1 [ save 1 byte ]
	0x1a,                               // eq?
	0x2f, 0x02,                         // bt 02 [ save 4 bytes ]
	0x87, 0x02,                         // lap 02
	0xa5, 0x01,                         // sat 01
	0x38, PATCH_UINT16(0x024d),         // pushi amount [ hard-coded for PC ]
	0x76,                               // push0
	0x85, 0x00,                         // lat 00
	0x4a, 0x04,                         // send 04 [ temp0 amount? ]
	0xa5, 0x02,                         // sat 02
	0x81, 0x19,                         // lag 19  [ dialog ]
	0x2e, PATCH_UINT16(0x0092),         // bt 0092 [ skip messages if dialog ]
	PATCH_END
};

// The baby antwerps in room 78 lockup the game if they get in ego's way when
//  exiting south. They also crash the interpreter if they wander too far off
//  the screen. These problems also occur in Sierra's interpreter.
//
// The antwerps are controlled by the Wander motion which randomly moves them
//  around. There's nothing stopping them from moving past the southern edge of
//  the screen and they tend to do so. When ego moves south of 180, sExitAll
//  disables control and moves ego off screen, but if an antwerp is in the way
//  then ego stops and the user never regains control. Once an antwerp has
//  escaped the screen it continues to wander until its position overflows
//  several minutes later. This freezes Sierra's interpreter and currently
//  causes ours to fail an assertion due to constructing an invalid Rect.
//
// We fix both problems by not allowing antwerps south of 180 so that they
//  remain on screen and can't block ego's exit. This is consistent with
//  room 85 where they also appear but without a southern exit. The Wander
//  motion is only used by antwerps and the sparkles above Yorick in room 96,
//  so it can be safely patched to enforce a southern limit. We make room for
//  this patch by replacing Wander's calculations with their known results,
//  since the default Wander:distance of 30 is always used, and by overwriting
//  Wander:onTarget which no script calls and just returns zero.
//
// Applies to: PC Floppy, Mac Floppy
// Responsible method: Wander:setTarget
// Fixes bug: #9564
static const uint16 qfg1vgaSignatureAntwerpWander[] = {
	SIG_MAGICDWORD,
	0x3f, 0x01,                             // link 01
	0x78,                                   // push1
	0x76,                                   // push0
	0x63, 0x12,                             // pToa client
	0x4a, 0x04,                             // send 4
	0x36,                                   // push
	0x67, 0x30,                             // pTos distance
	0x7a,                                   // push2
	0x76,                                   // push0
	0x67, 0x30,                             // pTos distance
	0x35, 0x02,                             // ldi 02
	0x06,                                   // mul
	0xa5, 0x00,                             // sat temp[0] [ distance * 2 ]
	0x36,                                   // push
	0x43, 0x3c, 0x04,                       // callk Random, 4
	0x04,                                   // sub
	0x02,                                   // add
	0x65, 0x16,                             // aTop x [ x = client:x + (distance - Random(0, temp[0])) ]
	0x76,                                   // push0
	0x76,                                   // push0
	0x63, 0x12,                             // pToa client
	0x4a, 0x04,                             // send 4
	0x36,                                   // push
	0x67, 0x30,                             // pTos distance
	0x7a,                                   // push2
	0x76,                                   // push0
	0x8d, 0x00,                             // lst temp[0]
	0x43, 0x3c, 0x04,                       // callk Random, 4
	0x04,                                   // sub
	0x02,                                   // add
	0x65, 0x18,                             // aTop y [ y = client:y + (distance - Random(0, temp[0])) ]
	0x48,                                   // ret
	0x35, 0x00,                             // ldi 00 [ start of Wander:onTarget, returns 0 and isn't called ]
	SIG_END
};

static const uint16 qfg1vgaPatchAntwerpWander[] = {
	0x78,                                   // push1
	0x76,                                   // push0
	0x63, 0x12,                             // pToa client
	0x4a, 0x04,                             // send 4
	0x36,                                   // push
	0x39, 0x1e,                             // pushi 1e
	0x7a,                                   // push2
	0x76,                                   // push0
	0x39, 0x3c,                             // pushi 3c
	0x43, 0x3c, 0x04,                       // callk Random, 4
	0x04,                                   // sub
	0x02,                                   // add
	0x65, 0x16,                             // aTop x [ x = client:x + (30d - Random(0, 60d)) ]
	0x76,                                   // push0
	0x76,                                   // push0
	0x63, 0x12,                             // pToa client
	0x4a, 0x04,                             // send 4
	0x36,                                   // push
	0x39, 0x1e,                             // pushi 1e
	0x7a,                                   // push2
	0x76,                                   // push0
	0x39, 0x3c,                             // pushi 3c
	0x43, 0x3c, 0x04,                       // callk Random, 4
	0x04,                                   // sub
	0x02,                                   // add
	0x7a,                                   // push2
	0x36,                                   // push
	0x38, PATCH_UINT16(0x00b4),             // pushi 00b4
	0x46, PATCH_UINT16(0x03e7),             // calle [export 2 of script 999], 4 [ Min ]
	      PATCH_UINT16(0x0002), 0x04,
	0x65, 0x18,                             // aTop y [ y = Min(client:y + (30d - Random(0, 60d))), 180d) ]
	PATCH_END
};

// QFG1VGA Mac disables all controls when the antwerp falls in room 78, killing
//  the player by not allowing them to defend themselves.
//
// The antwerp falls in rooms 78 and 85, and the only way to survive is to hold
//  up a weapon. These two antwerp scripts were identical in the PC version and
//  enabled all menu icons even though most of them couldn't really be used.
//  Sierra attempted to improve this in Mac by only enabling the inventory
//  icons but instead disabled everything in room 78 by not calling the enable
//  procedure.
//
// We fix this by calling the enable procedure like the script in room 85 does.
//
// Applies to: Mac Floppy
// Responsible method: antwerped:changeState(1)
// Fixes bug: #10856
static const uint16 qfg1vgaSignatureMacAntwerpControls[] = {
	0x30, SIG_UINT16(0x0033),               // bnt 0033 [ state 1 ]
	SIG_ADDTOOFFSET(+48),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x014e),               // jmp 014e [ end of method ]
	0x3c,                                   // dup
	0x35, 0x01,                             // ldi 01
	0x1a,                                   // eq?
	0x30, SIG_UINT16(0x0033),               // bnt 0033 [ state 2 ]
	0x38, SIG_UINT16(0x00f9),               // pushi 00f9 [ canControl, hard coded for Mac ]
	SIG_END
};

static const uint16 qfg1vgaPatchMacAntwerpControls[] = {
	0x30, PATCH_UINT16(0x0030),             // bnt 0030 [ state 1 ]
	PATCH_ADDTOOFFSET(+48),
	0x3c,                                   // dup
	0x35, 0x01,                             // ldi 01
	0x1a,                                   // eq?
	0x31, 0x37,                             // bnt 37 [ state 2 ]
	0x76,                                   // push0
	0x45, 0x03, 0x00,                       // callb [export 3 of script 0], 00 [ enable all input ]
	PATCH_END
};

// The Mac version's Sierra logo and introduction are often skipped when using a
//  mouse. This is a bug in the original that accidentally relies on slower
//  machines. In DOS these scenes could be skipped by pressing Enter. Sierra
//  updated this to include the mouse, but they did this by accepting any event
//  type, including mouse-up. These rooms load in response to mouse-down and if
//  they finish loading before the button is released then they are skipped.
//
// We fix this by excluding mouse-up events from these room event handlers.
//
// Applies to: Mac Floppy
// Responsible methods: LogoRoom:handleEvent, intro:handleEvent
// Fixes bug: #10937
static const uint16 qfg1vgaSignatureMacLogoIntroSkip[] = {
	0x4a, 0x04,                             // send 04 [ event type? ]
	0x31, SIG_ADDTOOFFSET(+1),              // bnt [ skip if event:type == none (0) ]
	0x39, SIG_ADDTOOFFSET(+1),              // pushi claimed
	SIG_MAGICDWORD,
	0x78,                                   // push1
	0x78,                                   // push1
	0x87, 0x01,                             // lap 01
	0x4a, 0x06,                             // send 06 [ event claimed: 1 ]
	SIG_END
};

static const uint16 qfg1vgaPatchMacLogoIntroSkip[] = {
	0x39, PATCH_GETORIGINALBYTE(+5),        // pushi claimed
	0x78,                                   // push1
	0x78,                                   // push1
	0x4a, 0x0a,                             // send 0a [ event type? claimed: 1 ]
	0x38, PATCH_UINT16(0x00fd),             // pushi 00fd
	0x12,                                   // and
	0x31, PATCH_GETORIGINALBYTEADJUST(+3, -8), // bnt [ skip if event:type == none (0) or mouse-up (2)]
	PATCH_END
};

// The Thieves' Guild cashier in room 332 stops responding to verbs when he
//  reappears at his window. This is due to heGoes:changeState(1) disposing and
//  deleting borisThief once he's out of sight, indirectly deleting his actions
//  object borisTeller which handles verbs. borisTeller is only initialized in
//  rm332:init and this leaves the player unable to purchase or fence items.
//
// We fix this by toggling borisThief's visibility with the hide and show
//  methods instead of disposing and re-initializing.
//
// Applies to: PC Floppy, Mac Floppy
// Responsible methods: heComes:changeState, heGoes:changeState
// Fixes bug #10939
static const uint16 qfg1vgaSignatureThievesGuildCashier[] = {
	0x30, SIG_UINT16(0x0024),               // bnt 0024 [ state 1 ]
	SIG_ADDTOOFFSET(+31),
	SIG_MAGICDWORD,
	0x4a, 0x20,                             // send 20  [ borisThief ... init: ... ]
	0x32, SIG_UINT16(0x002b),               // jmp 002b [ end of method ]
	0x3c,                                   // dup
	0x35, 0x01,                             // ldi 01
	0x1a,                                   // eq?
	0x30, SIG_UINT16(0x0019),               // bnt 0019 [ state 2 ]
	SIG_ADDTOOFFSET(+82),
	0x39, SIG_SELECTOR8(dispose),           // pushi dispose
	0x76,                                   // push0
	0x39, SIG_SELECTOR8(delete),            // pushi delete
	SIG_ADDTOOFFSET(+4),
	0x4a, 0x08,                             // send 08 [ borisThief dispose: delete: ]
	SIG_END
};

static const uint16 qfg1vgaPatchThievesGuildCashier[] = {
	0x30, PATCH_UINT16(0x0025),             // bnt 0025 [ state 1 ]
	PATCH_ADDTOOFFSET(+31),
	0x38, PATCH_SELECTOR16(show),           // pushi show
	0x76,                                   // push0
	0x4a, 0x24,                             // send 24 [ borisThief ... init: ... show: ]
	0x3c,                                   // dup
	0x35, 0x01,                             // ldi 01
	0x1a,                                   // eq?
	0x31, 0x19,                             // bnt 19 [ state 2 ]
	PATCH_ADDTOOFFSET(+82),
	0x39, PATCH_SELECTOR8(hide),            // pushi hide
	0x32, PATCH_UINT16(0x0000),             // jmp 0000
	PATCH_ADDTOOFFSET(+4),
	0x4a, 0x04,                             // send 04 [ borisThief hide: ]
	PATCH_END
};

// When entering the great hall (room 141), the Mac version stores ego's speed
//  in a temp variable in egoEnters:changeState(0) and expects that value to be
//  there in state 6 when restoring ego's speed. We patch the script to use its
//  register instead so that it works and doesn't do an uninitialized read.
//
// Applies to: Mac Floppy
// Responsible method: egoEnters:changeState
// Fixes bug: #10945
static const uint16 qfg1vgaSignatureMacEnterGreatHall[] = {
	SIG_MAGICDWORD,
	0x4a, 0x04,                             // send 04 [ ego cycleSpeed? ]
	0xa5, 0x00,                             // sat 00
	SIG_ADDTOOFFSET(+140),
	0x8d, 0x00,                             // lst 00
	SIG_END
};

static const uint16 qfg1vgaPatchMacEnterGreatHall[] = {
	PATCH_ADDTOOFFSET(+2),
	0x65, 0x24,                             // aTop register
	PATCH_ADDTOOFFSET(+140),
	0x67, 0x24,                             // pTos register
	PATCH_END
};

// When fighting the giant in room 58, the Mac version stores the weapon in a
//  temp variable in giantFights:changeState(2) and expects that value to be
//  there in later states to determine whether or not to run sword animation.
//  We patch the script to use the local variable that holds the weapon instead
//  so that this works and doesn't do an uninitialized read.
//
// Applies to: Mac Floppy
// Responsible method: giantFights:changeState
// Fixes bug: #10948
static const uint16 qfg1vgaSignatureMacGiantFight[] = {
	SIG_MAGICDWORD,
	0x8d, 0x00,                             // lst 00 [ temp0 set to 0 in state 2 if local5 == 1 ]
	0x35, 0x00,                             // ldi 00
	0x1a,                                   // eq?
	SIG_END
};

static const uint16 qfg1vgaPatchMacGiantFight[] = {
	0x8b, 0x05,                             // lsl 05
	0x35, 0x01,                             // ldi 01
	PATCH_END
};

// Dag-Nab-It, the dagger game in room 340, mistakenly leaves inventory enabled.
//  Using a throwable item, such as a dagger, locks up the game.
//
// We fix this by disabling inventory controls on this screen. Sierra attempted
//  to do this in the Mac version but introduced a new bug which we also fix.
//
// Applies to: PC Floppy
// Responsible method: dagnabitScript:changeState
// Fixes bug: #10958
static const uint16 qfg1vgaSignatureDagnabitInventory[] = {
	0x38, SIG_SELECTOR16(disable),          // pushi disable
	0x39, 0x03,                             // pushi 03
	0x78,                                   // push1
	0x39, SIG_MAGICDWORD, 0x05,             // pushi 05
	0x39, 0x09,                             // pushi 09
	0x81, 0x45,                             // lag 45
	0x4a, 0x0a,                             // send 0a [ mainIconBar disable: 1 5 9 ]
	0x35, 0x01,                             // ldi 01
	0xa3, 0x13,                             // sal 13
	0x32, SIG_UINT16(0x0278),               // jmp 0278 [ end of method ]
	SIG_ADDTOOFFSET(+625),
	0x38, SIG_SELECTOR16(changeState),      // pushi changeState
	0x78,                                   // push1
	0x78,                                   // push1
	SIG_END
};

static const uint16 qfg1vgaPatchDagnabitInventory[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x05,                             // pushi 05
	PATCH_ADDTOOFFSET(+5),
	0x39, 0x07,                             // pushi 07 [ "use" inventory icon ]
	0x39, 0x08,                             // pushi 08 [ inventory ]
	0x81, 0x45,                             // lag 45
	0x4a, 0x0e,                             // send 0e [ mainIconBar disable: 1 5 9 7 8 ]
	0x78,                                   // push1
	0xab, 0x13,                             // ssl 13
	PATCH_ADDTOOFFSET(+629),
	0x76,                                   // push0 [ state 0 re-disables inventory ]
	PATCH_END
};

// The Mac version of Dag-Nab-It, the dagger game in room 340, introduced a bug
//  that sends a message to a non-object when clicking during the start.
//
// The PC version left inventory enabled and Sierra fixed this in Mac. Sierra
//  also attempted to clear the inventory cursor, but this was done in a way
//  that leaves the icon bar in an illegal state. mainIconBar:curInvIcon is
//  set to zero but mainIconBar:curIcon remains set to the "use" icon item.
//  Clicking anywhere during the initial two seconds causes IconBar:handleEvent
//  to query curInvIcon:message but since curInvIcon is zero this is an error.
//
// We fix this with a deceptively simple patch that prevents the "use" icon from
//  ending up as mainIconBar:curIcon. rm340:init runs a complex sequence of icon
//  disabling and enabling. Patching a redundant mainIconBar:disable to include
//  "use" prevents the subsequent call to handsOff from cycling through enabled
//  icons and landing on "use" as the new curIcon, preventing the illegal state.
//
// Applies to: Mac Floppy
// Responsible method: rm340:init
// Fixes bug: #10958
static const uint16 qfg1vgaSignatureMacDagnabitIconBar[] = {
	0x38, SIG_SELECTOR16(disable),          // pushi disable
	0x39, 0x03,                             // pushi 03
	0x78,                                   // push1
	SIG_MAGICDWORD,
	0x39, 0x05,                             // pushi 05
	0x39, 0x08,                             // pushi 08
	0x81, 0x45,                             // lag 45
	0x4a, 0x0a,                             // send 0a [ mainIconBar disable: 1 5 8 ]
	SIG_END
};

static const uint16 qfg1vgaPatchMacDagnabitIconBar[] = {
	PATCH_ADDTOOFFSET(+6),
	0x39, 0x07,                             // pushi 07 [ "use" inventory icon ]
	PATCH_END
};

// Drinking water in room 87 after talking to the healer about falling water
//  shows two messages at once. The second message overwrites the first before
//  it is shown. We add a state for the second message as in the Mac version.
//
// Applies to: PC Floppy
// Responsible method: drinkWater:changeState
// Fixes bug: #11086
static const uint16 qfg1vgaSignatureDrinkWaterMessage[] = {
	0x31, SIG_MAGICDWORD, 0x29,             // bnt 29 [ state 3 handler ]
	0x38, SIG_SELECTOR16(say),              // pushi say
	0x39, 0x05,                             // pushi 05
	0x39, 0x07,                             // pushi 07
	0x76,                                   // push0
	0x76,                                   // push0
	0x78,                                   // push1
	0x7c,                                   // pushSelf
	0x81, 0x5b,                             // lag 5b
	0x4a, 0x0e,                             // send 0e [ qg1Messager say: 7 0 0 1 self ]
	0x78,                                   // push1
	0x38, SIG_UINT16(0x00c9),               // pushi 00c9 [ flag 201 ]
	0x45, 0x07, 0x02,                       // callb proc0_7 [ talked to healer about water? ]
	0x31, 0x42,                             // bnt 42 [ skip second message ]
	SIG_ADDTOOFFSET(+9),
	0x7a,                                   // push2 [ message seq: 2 ]
	SIG_ADDTOOFFSET(+8),
	0x35, 0x03,                             // ldi 03 [ state 3 ]
	SIG_ADDTOOFFSET(+18),
	0x35, 0x04,                             // ldi 04 [ state 4 ]
	SIG_END
};

static const uint16 qfg1vgaPatchDrinkWaterMessage[] = {
	0x2f, 0x14,                             // bt 14  [ show first message in state 2 ]
	0x3c,                                   // dup
	0x35, 0x03,                             // ldi 03 [ state 3 ]
	0x1a,                                   // eq?
	0x31, 0x23,                             // bnt 23 [ state 4 handler ]
	0x78,                                   // push1
	0x38, PATCH_UINT16(0x00c9),             // pushi 00c9 [ flag 201 ]
	0x45, 0x07, 0x02,                       // callb proc0_7 [ talked to healer about water? ]
	0x2f, 0x05,                             // bt 05 [ show second message in state 3 ]
	0x78,                                   // push1
	0x69, 0x1a,                             // sTop cycles [ cycles = 1 ]
	0x3a,                                   // toss
	0x48,                                   // ret
	0x3c,                                   // dup
	0x35, 0x01,                             // ldi 01
	0x04,                                   // sub
	PATCH_ADDTOOFFSET(+9),
	0x36,                                   // push [ message seq: state - 1 ]
	PATCH_ADDTOOFFSET(+8),
	0x35, 0x04,                             // ldi 04 [ state 4 ]
	PATCH_ADDTOOFFSET(+18),
	0x35, 0x05,                             // ldi 05 [ state 5 ]
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry qfg1vgaSignatures[] = {
	{  true,     0, "inventory weight warning",                    1, qfg1vgaSignatureInventoryWeightWarn, qfg1vgaPatchInventoryWeightWarn },
	{  true,    41, "moving to castle gate",                       1, qfg1vgaSignatureMoveToCastleGate,    qfg1vgaPatchMoveToCastleGate },
	{  true,    55, "healer's hut, no delay for buy/steal",        1, qfg1vgaSignatureHealerHutNoDelay,    qfg1vgaPatchHealerHutNoDelay },
	{  true,    58, "mac: giant fight",                            6, qfg1vgaSignatureMacGiantFight,       qfg1vgaPatchMacGiantFight },
	{  true,    73, "brutus script freeze glitch",                 1, qfg1vgaSignatureBrutusScriptFreeze,  qfg1vgaPatchBrutusScriptFreeze },
	{  true,    77, "white stag dagger throw animation glitch",    1, qfg1vgaSignatureWhiteStagDagger,     qfg1vgaPatchWhiteStagDagger },
	{  true,    78, "mac: enable antwerp controls",                1, qfg1vgaSignatureMacAntwerpControls,  qfg1vgaPatchMacAntwerpControls },
	{  true,    87, "drink water message",                         1, qfg1vgaSignatureDrinkWaterMessage,   qfg1vgaPatchDrinkWaterMessage },
	{  true,    96, "funny room script bug fixed",                 1, qfg1vgaSignatureFunnyRoomFix,        qfg1vgaPatchFunnyRoomFix },
	{  true,    96, "yorick door #2 lockup fixed",                 1, qfg1vgaSignatureYorickDoorTwoRect,   qfg1vgaPatchYorickDoorTwoRect },
	{  true,   141, "mac: enter great hall",                       1, qfg1vgaSignatureMacEnterGreatHall,   qfg1vgaPatchMacEnterGreatHall },
	{  true,   200, "mac: intro mouse-up fix",                     1, qfg1vgaSignatureMacLogoIntroSkip,    qfg1vgaPatchMacLogoIntroSkip },
	{  true,   210, "cheetaur description fixed",                  1, qfg1vgaSignatureCheetaurDescription, qfg1vgaPatchCheetaurDescription },
	{  true,   215, "fight event issue",                           1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   216, "weapon master event issue",                   1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   299, "speedtest",                                   1, qfg1vgaSignatureSpeedTest,           qfg1vgaPatchSpeedTest },
	{  true,   331, "moving to crusher",                           1, qfg1vgaSignatureMoveToCrusher,       qfg1vgaPatchMoveToCrusher },
	{  true,   331, "moving to crusher from card game",            1, qfg1vgaSignatureCrusherCardGame,     qfg1vgaPatchCrusherCardGame },
	{  true,   332, "thieves' guild cashier fix",                  1, qfg1vgaSignatureThievesGuildCashier, qfg1vgaPatchThievesGuildCashier },
	{  true,   340, "dagnabit inventory fix",                      1, qfg1vgaSignatureDagnabitInventory,   qfg1vgaPatchDagnabitInventory },
	{  true,   340, "mac: dagnabit icon bar fix",                  1, qfg1vgaSignatureMacDagnabitIconBar,  qfg1vgaPatchMacDagnabitIconBar },
	{  true,   603, "mac: logo mouse-up fix",                      1, qfg1vgaSignatureMacLogoIntroSkip,    qfg1vgaPatchMacLogoIntroSkip },
	{  true,   814, "window text temp space",                      1, qfg1vgaSignatureTempSpace,           qfg1vgaPatchTempSpace },
	{  true,   814, "dialog header offset",                        3, qfg1vgaSignatureDialogHeader,        qfg1vgaPatchDialogHeader },
	{  true,   928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,          sciNarratorLockupPatch },
	{  true,   970, "antwerps wandering off-screen",               1, qfg1vgaSignatureAntwerpWander,       qfg1vgaPatchAntwerpWander },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// This is a very complicated bug.
// When the player encounters an enemy in the desert while riding a saurus and
//  later tries to get back on it by entering "ride", the game will not give
//  control back to the player.
//
// This is caused by script mountSaurus getting triggered twice. Once by
//  entering the command "ride" and then a second time by a proximity check.
//
// Both are calling mountSaurus::init() in script 20. This one disables
//  controls. Then mountSaurus::changeState() from script 660 is triggered.
//  Finally, mountSaurus::changeState(5) calls mountSaurus::dispose(), also in
//  script 20, which re-enables controls.
//
// A fix is difficult to implement. The code in script 20 is generic and used
//  by multiple objects
//
// An early attempt changed the responsible vars (global[102], global[161])
//  during mountSaurus::changeState(5). This worked for controls, but
//  mountSaurus::init changes a few selectors of ego as well, which won't get
//  restored in that situation, which then messes up room changes and other
//  things.
//
// Instead we change sheepScript::changeState(2) in script 665.
//
// Note: This could cause issues in case there is a cutscene, where ego is
//  supposed to get onto the saurus using sheepScript.
//
// Applies to at least: English PC Floppy, English Amiga Floppy
// Responsible method: mountSaurus::changeState(), mountSaurus::init(), mountSaurus::dispose()
// Fixes bug: #5156
static const uint16 qfg2SignatureSaurusFreeze[] = {
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 2
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0043),           // bnt [ret]
	0x76,                               // push0
	SIG_ADDTOOFFSET(+61),               // skip to dispose code
	0x39, SIG_SELECTOR8(dispose),       // pushi dispose
	0x76,                               // push0
	0x54, 0x04,                         // self 04
	SIG_END
};

static const uint16 qfg2PatchSaurusFreeze[] = {
	0x81, 0x66,                         // lag global[66h]
	0x2e, PATCH_UINT16(0x0040),         // bt [to dispose code]
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	PATCH_END
};

// The Jackalmen combat code has at least one serious issue.
//
// Jackalmen may attack in groups. This is handled by 2 globals.
//  global[136h]: amount of Jackalmen still alive.
//  global[137h]: amount of Jackalmen killed so far during combat.
//
// After combat has ended, global[137h] is subtracted from global[136h]. BUT
// when the player manages to hit the last enemy AFTER defeating it during its
// death animation (yes, that is possible - don't ask), the code is called a
// second time. Subtracting global[137h] twice, which will make global[136h]
// negative and will then create an inconsistent state. Some variables will
// show that there is still an enemy, while others don't. The game will crash
// when leaving the room. The original interpreter would show the infamous
// "Oops, you did something we weren't expecting..."
//
// TODO: Check, if patch works for 1.000. That version surely has the same bug.
// Applies to at least: English Floppy (1.102+1.105)
// Responsible method: jackalMan::die (script 695)
// Fixes bug: #10218
static const uint16 qfg2SignatureOopsJackalMen[] = {
	SIG_MAGICDWORD,
	0x8b, 0x00,                         // lsl local[0]
	0x35, 0x00,                         // ldi 0
	0x22,                               // lt?
	0x30, SIG_UINT16(0x000b),           // bnt [Jackalman death animation code]
	0x38, SIG_ADDTOOFFSET(+2),          // pushi (die)
	0x76,                               // push0
	0x57, 0x66, 0x04,                   // super Monster, 4
	0x48,                               // ret
	0x32, SIG_UINT16(0x001a),           // jmp (seems to be a compiler bug)
	// Jackalman death animation code
	0x83, 0x00,                         // lal local[0]
	0x18,                               // not
	0x30, SIG_UINT16(0x0003),           // bnt [make next enemy walk in]
   SIG_END
};

static const uint16 qfg2PatchOopsJackalMen[] = {
	0x80, PATCH_UINT16(0x0136),         // lag global[136h]
	0x31, 0x0e,                         // bnt [skip everything] - requires 5 extra bytes
	0x8b, 0x00,                         // lsl local[0]
	0x35, 0x00,                         // ldi 0
	0x22,                               // lt?
	0x31, 0x08,                         // bnt [Jackalman death animation code] (save 1 byte)
	0x38, PATCH_GETORIGINALUINT16(+9),  // pushi (die)
	0x76,                               // push0
	0x57, 0x66, 0x04,                   // super Monster, 4
	0x48,                               // ret
	// Jackalman death animation code
	0x83, 0x00,                         // lal local[0]
	0x18,                               // not
	0x31, 0x03,                         // bnt [make next enemy walk in] (save 1 byte)
	PATCH_END
};

// Script 944 in QFG2 contains the FileSelector system class, used in the
// character import screen. This gets incorrectly called constantly, whenever
// the user clicks on a button in order to refresh the file list. This was
// probably done because it would be easier to refresh the list whenever the
// user inserted a new floppy disk, or changed directory. The problem is that
// the script has a bug, and it invalidates the text of the entries in the
// list. This has a high probability of breaking, as the user could change the
// list very quickly, or the garbage collector could kick in and remove the
// deleted entries. We don't allow the user to change the directory, thus the
// contents of the file list are constant, so we can avoid the constant file
// and text entry refreshes whenever a button is pressed, and prevent possible
// crashes because of these constant quick object reallocations.
// Fixes bug: #5096
static const uint16 qfg2SignatureImportDialog[] = {
	0x63, SIG_MAGICDWORD, 0x20,         // pToa text
	0x30, SIG_UINT16(0x000b),           // bnt [next state]
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	0x36,                               // push
	0x43, 0x72, 0x04,                   // callk Memory, 4
	0x35, 0x00,                         // ldi 00
	0x65, 0x20,                         // aTop text
	SIG_END
};

static const uint16 qfg2PatchImportDialog[] = {
	PATCH_ADDTOOFFSET(+5),
	0x48,                               // ret
	PATCH_END
};

// Quest For Glory 2 character import doesn't properly set the character type
//  in versions 1.102 and below, which makes all imported characters a fighter.
//
// Sierra released an official patch. However the fix is really easy to
//  implement on our side, so we also patch the flaw in here in case we find it.
//
// The version released on GOG is 1.102 without this patch applied, so us
//  patching it is quite useful.
//
// Applies to at least: English Floppy
// Responsible method: importHero::changeState
// Fixes bug: inside versions 1.102 and below
static const uint16 qfg2SignatureImportCharType[] = {
	0x35, 0x04,                         // ldi 04
	0x90, SIG_UINT16(0x023b),           // lagi global[23Bh]
	0x02,                               // add
	0x36,                               // push
	0x35, 0x04,                         // ldi 04
	0x08,                               // div
	0x36,                               // push
	0x35, 0x0d,                         // ldi 0D
	0xb0, SIG_UINT16(0x023b),           // sagi global[023Bh]
	0x8b, 0x1f,                         // lsl local[1Fh]
	0x35, 0x05,                         // ldi 05
	SIG_MAGICDWORD,
	0xb0, SIG_UINT16(0x0150),           // sagi global[0150h]
	0x8b, 0x02,                         // lsl local[02h]
	SIG_END
};

static const uint16 qfg2PatchImportCharType[] = {
	0x80, PATCH_UINT16(0x023f),         // lag global[23Fh] <-- patched to save 2 bytes
	0x02,                               // add
	0x36,                               // push
	0x35, 0x04,                         // ldi 04
	0x08,                               // div
	0x36,                               // push
	0xa8, PATCH_UINT16(0x0248),         // ssg global[0248h] <-- patched to save 2 bytes
	0x8b, 0x1f,                         // lsl local[1Fh]
	0xa8, PATCH_UINT16(0x0155),         // ssg global[0155h] <-- patched to save 2 bytes
	// new code, directly from the official sierra patch file
	0x83, 0x01,                         // lal local[01h]
	0xa1, 0xbb,                         // sag global[BBh]
	0xa1, 0x73,                         // sag global[73h]
	PATCH_END
};

//          script, description,                                      signature                    patch
static const SciScriptPatcherEntry qfg2Signatures[] = {
	{  true,   665, "getting back on saurus freeze fix",           1, qfg2SignatureSaurusFreeze,   qfg2PatchSaurusFreeze },
	{  true,   695, "Oops Jackalmen fix",                          1, qfg2SignatureOopsJackalMen,  qfg2PatchOopsJackalMen },
	{  true,   805, "import character type fix",                   1, qfg2SignatureImportCharType, qfg2PatchImportCharType },
	{  true,   944, "import dialog continuous calls",              1, qfg2SignatureImportDialog,   qfg2PatchImportDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Patch for the import screen in QFG3, same as the one for QFG2 above
static const uint16 qfg3SignatureImportDialog[] = {
	0x63, SIG_MAGICDWORD, 0x2a,         // pToa text
	0x31, 0x0b,                         // bnt [next state]
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	0x36,                               // push
	0x43, 0x72, 0x04,                   // callk Memory, 4
	0x35, 0x00,                         // ldi 00
	0x65, 0x2a,                         // aTop text
	SIG_END
};

static const uint16 qfg3PatchImportDialog[] = {
	PATCH_ADDTOOFFSET(+4),
	0x48,                               // ret
	PATCH_END
};

// Patch for the Woo dialog option in Uhura's conversation.
//
// Problem: The Woo dialog option (0xffb5) is negative, and therefore
//  treated as an option opening a submenu. This leads to uhuraTell::doChild
//  being called, which calls hero::solvePuzzle and then proceeds with
//  Teller::doChild to open the submenu. However, there is no actual submenu
//  defined for option -75 since -75 does not show up in uhuraTell::keys.
//  This will cause Teller::doChild to run out of bounds while scanning through
//  uhuraTell::keys.
//
// Strategy: there is another conversation option in uhuraTell::doChild calling
//  hero::solvePuzzle (0xfffc) which does a ret afterwards without going to
//  Teller::doChild. We jump to this call of hero::solvePuzzle to get that same
//  behaviour.
//
// Applies to at least: English, German, Italian, French, Spanish Floppy
// Responsible method: uhuraTell::doChild
// Fixes bug: #5172
static const uint16 qfg3SignatureWooDialog[] = {
	SIG_MAGICDWORD,
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb6,                         // ldi b6
	0x1a,                               // eq?
	0x2f, 0x05,                         // bt 05
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0x9b,                         // ldi 9b
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt 0c
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi solvePuzzle (0297)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialog[] = {
	PATCH_ADDTOOFFSET(+0x29),
	0x33, 0x11,                         // jmp [to 0x6a2, the call to hero::solvePuzzle for 0xFFFC]
	PATCH_END
};

// Alternative version, with uint16 offsets, for GOG release of QfG3.
static const uint16 qfg3SignatureWooDialogAlt[] = {
	SIG_MAGICDWORD,
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb6,                         // ldi b6
	0x1a,                               // eq?
	0x2e, SIG_UINT16(0x0005),           // bt 05
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0x9b,                         // ldi 9b
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x000c),           // bnt 0c
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi solvePuzzle (0297)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialogAlt[] = {
	PATCH_ADDTOOFFSET(+44),
	0x33, 0x12,                         // jmp [to 0x708, the call to hero::solvePuzzle for 0xFFFC]
	PATCH_END
};

// When exporting characters at the end of Quest for Glory 3, the underlying
//  code has issues with values above 9999.
//  For further study: https://github.com/Blazingstix/QFGImporter/blob/master/QFGImporter/QFGImporter/QFG3.txt
//
// If a value is above 9999, parts or even the whole character file will get
//  corrupted. We calculate the checksum and add extra code to lower such
//  values to 9999.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: saveHero::changeState
// Fixes bug: #6807
static const uint16 qfg3SignatureExportChar[] = {
	0x35, SIG_ADDTOOFFSET(+1),          // ldi 00 / ldi 01 (2 loops, we patch both)
	0xa5, 0x00,                         // sat temp[0] [contains index to data]
	0x8d, 0x00,                         // lst temp[0]
	SIG_MAGICDWORD,
	0x35, 0x2c,                         // ldi 2c
	0x22,                               // lt? (index above or equal to 2Ch (44d)?)
	0x31, 0x23,                         // bnt [exit loop]
	// from this point it's actually useless code, maybe a sci compiler bug
	0x8d, 0x00,                         // lst temp[0]
	0x35, 0x01,                         // ldi 01
	0x02,                               // add
	0x9b, 0x00,                         // lsli local[0] ---------- load local[0 + ACC] onto stack
	0x8d, 0x00,                         // lst temp[0]
	0x35, 0x01,                         // ldi 01
	0x02,                               // add
	0xb3, 0x00,                         // sali local[0] ---------- save stack to local[0 + ACC]
	// end of useless code
	0x8b, SIG_ADDTOOFFSET(+1),          // lsl local[36h/37h] ----- load local[36h/37h] onto stack
	0x8d, 0x00,                         // lst temp[0]
	0x35, 0x01,                         // ldi 01
	0x02,                               // add
	0x93, 0x00,                         // lali local[0] ---------- load local[0 + ACC] into ACC
	0x02,                               // add -------------------- add ACC + stack and put into ACC
	0xa3, SIG_ADDTOOFFSET(+1),          // sal local[36h/37h] ----- save ACC to local[36h/37h]
	0x8d, 0x00,                         // lst temp[0] ------------ temp[0] to stack
	0x35, 0x02,                         // ldi 02
	0x02,                               // add -------------------- add 2 to stack
	0xa5, 0x00,                         // sat temp[0] ------------ save ACC to temp[0]
	0x33, 0xd6,                         // jmp [loop]
	SIG_END
};

static const uint16 qfg3PatchExportChar[] = {
	PATCH_ADDTOOFFSET(+11),
	0x85, 0x00,                         // lat temp[0]
	0x9b, 0x01,                         // lsli local[0] + 1 ------ load local[ ACC + 1] onto stack
	0x3c,                               // dup
	0x34, PATCH_UINT16(0x2710),         // ldi 2710h (10000d)
	0x2c,                               // ult? ------------------- is value smaller than 10000?
	0x2f, 0x0a,                         // bt [jump over]
	0x3a,                               // toss
	0x38, PATCH_UINT16(0x270f),         // pushi 270fh (9999d)
	0x3c,                               // dup
	0x85, 0x00,                         // lat temp[0]
	0xba, PATCH_UINT16(0x0001),         // ssli local[0] + 1 ------ save stack to local[ACC + 1] (UINT16 to waste 1 byte)
	// jump offset
	0x83, PATCH_GETORIGINALBYTE(+26),   // lal local[37h/36h] ----- load local[37h/36h] into ACC
	0x02,                               // add -------------------- add local[37h/36h] + data value
	PATCH_END
};

// Quest for Glory 3 doesn't properly import the character type of QFG1
//  character files. This issue was never addressed. It's caused by Sierra
//  reading data directly from the local area, which is only set by QFG2
//  import data, instead of reading the properly set global variable.
//
// We fix it, by also directly setting the local variable.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: importHero::changeState(4)
static const uint16 qfg3SignatureImportQfG1Char[] = {
	SIG_MAGICDWORD,
	0x82, SIG_UINT16(0x0238),           // lal local[0x0238]
	0xa0, SIG_UINT16(0x016a),           // sag global[0x016a]
	0xa1, 0x7d,                         // sag global[0x7d]
	0x35, 0x01,                         // ldi 01
	0x99, 0xfb,                         // lsgi global[0xfb]
	SIG_END
};

static const uint16 qfg3PatchImportQfG1Char[] = {
	PATCH_ADDTOOFFSET(+8),
	0xa3, 0x01,                         // sal local[1]
	0x89, 0xfc,                         // lsg global[0xfc] (save 2 bytes vs global[0xfb + 1])
	PATCH_END
};

// The chief in his hut (room 640) is not drawn using the correct priority,
//  which results in a graphical glitch. This is a game bug and also happens
//  in Sierra's SCI. We adjust priority accordingly to fix it.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: heap in script 640
// Fixes bug: #5173
static const uint16 qfg3SignatureChiefPriority[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0002),                 // yStep     0x0002
	SIG_UINT16(0x0281),                 // view      0x0281
	SIG_UINT16(0x0000),                 // loop      0x0000
	SIG_UINT16(0x0000),                 // cel       0x0000
	SIG_UINT16(0x0000),                 // priority  0x0000
	SIG_UINT16(0x0000),                 // underbits 0x0000
	SIG_UINT16(0x1000),                 // signal    0x1000
	SIG_END
};

static const uint16 qfg3PatchChiefPriority[] = {
	PATCH_ADDTOOFFSET(+8),
	PATCH_UINT16(0x000a),               // priority  0x000A (10d)
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x1010),               // signal    0x1010 (set fixed priority flag)
	PATCH_END
};

// There are 3 points that can't be achieved in the game. They should've been
// awarded for telling Rakeesh and Kreesha (room 285) about the Simabni
// initiation.
// However the array of posibble messages the hero can tell in that room
// (local[156]) is missing the "Tell about Initiation" message (#31) which
// awards these points.
// This patch adds the message to that array, thus allowing the hero to tell
// that message (after completing the initiation) and gain the 3 points.
// A side effect of increasing the local[156] array is that the next local
// array is shifted and shrinks in size from 4 words to 3. The patch changes
// the 2 locations in the script that reference that array, to point to the new
// location (local[$aa] --> local[$ab]). It is safe to shrink the 2nd array to
// 3 words because only the first element in it is ever used.
//
// Note: You have to re-enter the room in case a saved game was loaded from a
// previous version of ScummVM and that saved game was made inside that room.
//
// Applies to: English, French, German, Italian, Spanish and the GOG release.
// Responsible method: heap in script 285
// Fixes bug: #7086
static const uint16 qfg3SignatureMissingPoints1[] = {
	// local[$9c] = [0 -41 -76 1 -30 -77 -33 -34 -35 -36 -37 -42 -80 999]
	// local[$aa] = [0 0 0 0]
	SIG_UINT16(0x0000),                 //   0 START MARKER
	SIG_MAGICDWORD,
	SIG_UINT16(0xffd7),                 // -41 "Greet"
	SIG_UINT16(0xffb4),                 // -76 "Say Good-bye"
	SIG_UINT16(0x0001),                 //   1 "Tell about Tarna"
	SIG_UINT16(0xffe2),                 // -30 "Tell about Simani"
	SIG_UINT16(0xffb3),                 // -77 "Tell about Prisoner"
	SIG_UINT16(0xffdf),                 // -33 "Dispelled Leopard Lady"
	SIG_UINT16(0xffde),                 // -34 "Tell about Leopard Lady"
	SIG_UINT16(0xffdd),                 // -35 "Tell about Leopard Lady"
	SIG_UINT16(0xffdc),                 // -36 "Tell about Leopard Lady"
	SIG_UINT16(0xffdb),                 // -37 "Tell about Village"
	SIG_UINT16(0xffd6),                 // -42 "Greet"
	SIG_UINT16(0xffb0),                 // -80 "Say Good-bye"
	SIG_UINT16(0x03e7),                 // 999 END MARKER
	SIG_ADDTOOFFSET(+2),                // local[$aa][0]
	SIG_END
};

static const uint16 qfg3PatchMissingPoints1[] = {
	PATCH_ADDTOOFFSET(+14),
	PATCH_UINT16(0xffe1),               // -31 "Tell about Initiation"
	PATCH_UINT16(0xffde),               // -34 "Tell about Leopard Lady"
	PATCH_UINT16(0xffdd),               // -35 "Tell about Leopard Lady"
	PATCH_UINT16(0xffdc),               // -36 "Tell about Leopard Lady"
	PATCH_UINT16(0xffdb),               // -37 "Tell about Village"
	PATCH_UINT16(0xffd6),               // -42 "Greet"
	PATCH_UINT16(0xffb0),               // -80 "Say Good-bye"
	PATCH_UINT16(0x03e7),               // 999 END MARKER
	PATCH_GETORIGINALUINT16(+28),       // local[$aa][0]
	PATCH_END
};

static const uint16 qfg3SignatureMissingPoints2a[] = {
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 0
	0xb3, 0xaa,                         // sali local[$aa]
	SIG_END
};

static const uint16 qfg3SignatureMissingPoints2b[] = {
	SIG_MAGICDWORD,
	0x36,                               // push
	0x5b, 0x02, 0xaa,                   // lea local[$aa]
	SIG_END
};

static const uint16 qfg3PatchMissingPoints2[] = {
	PATCH_ADDTOOFFSET(+3),
	0xab,                               // local[$ab] (replace local[$aa])
	PATCH_END
};

// Partly WORKAROUND:
// During combat, the game is not properly throttled. That's because the game uses
// an inner loop for combat and does not iterate through the main loop.
// It also doesn't call kGameIsRestarting. This may get fixed properly at some point
// by rewriting the speed throttler.
//
// Additionally Sierra set the cycle speed of the hero to 0. Which explains
// why the actions of the hero are so incredibly fast. This issue also happened
// in the original interpreter, when the computer was too powerful.
//
// Applies to at least: English, French, German, Italian, Spanish PC floppy
// Responsible method: combatControls::dispatchEvent (script 550) + WarriorObj in heap
// Fixes bug: #6247
static const uint16 qfg3SignatureCombatSpeedThrottling1[] = {
	0x3f, 0x03,                         // link 3d (these temp vars are never used)
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime, 0d
	0xa1, 0x58,                         // sag global[88]
	0x36,                               // push
	0x83, 0x01,                         // lal local[1]
	SIG_ADDTOOFFSET(+3),                // ...
	SIG_MAGICDWORD,
	0x89, 0xd2,                         // lsg global[210]
	0x35, 0x00,                         // ldi 0
	0x1e,                               // gt?
	SIG_ADDTOOFFSET(+6),                // ...
	0xa3, 0x01,                         // sal local[1]
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling1[] = {
	0x76,                               // push0  (no link, freed +2 bytes)
	0x43, 0x42, 0x00,                   // callk GetTime, 0d
	0xa1, 0x58,                         // sag global[88] (no push, leave time in acc)
	0x8b, 0x01,                         // lsl local[1] (stack up the local instead, freed +1 byte)
	0x1c,                               // ne?
	0x31, 0x0c,                         // bnt 12d [after sal]
                                        //
	0x81, 0xd2,                         // lag global[210] (load into acc instead of stack)
	0x76,                               // push0 (push0 instead of ldi 0, freed +1 byte)
	0x22,                               // lt? (flip the comparison)
	0x31, 0x06,                         // bnt 6d [after sal]
                                        //
	0xe1, 0xd2,                         // -ag global[210]
	0x81, 0x58,                         // lag global[88]
	0xa3, 0x01,                         // sal local[1]

	0x76,                               // push0 (0 call args)
	0x43, 0x2c, 0x00,                   // callk GameIsRestarting, 0d (add this to trigger our speed throttler)
	PATCH_END
};

static const uint16 qfg3SignatureCombatSpeedThrottling2[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(12),                     // priority 12
	SIG_UINT16(0x0000),                 // underbits 0
	SIG_UINT16(0x4010),                 // signal 4010h
	SIG_ADDTOOFFSET(+18),
	SIG_UINT16(0x0000),                 // scaleSignal 0
	SIG_UINT16(128),                    // scaleX 128
	SIG_UINT16(128),                    // scaleY 128
	SIG_UINT16(128),                    // maxScale 128
	SIG_UINT16(0x0000),                 // cycleSpeed 0
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling2[] = {
	PATCH_ADDTOOFFSET(+32),
	PATCH_UINT16(0x0005),               // set cycleSpeed to 5
	PATCH_END
};

// In room #750, when the hero enters from the top east path (room #755), it
// could go out of the contained-access polygon bounds, and be able to travel
// freely in the room.
// The reason is that the cutoff y value (42) that determines whether the hero
// enters from the top or bottom path is inaccurate: it's possible to enter the
// top path from as low as y=45.
// This patch changes the cutoff to be 50 which should be low enough.
// It also changes the position in which the hero enters from the top east path
// as the current location is hidden behind the tree.
//
// Applies to: English, French, German, Italian, Spanish and the GOG release.
// Responsible method: enterEast::changeState (script 750)
// Fixes bug: #6693
static const uint16 qfg3SignatureRoom750Bounds1[] = {
	// (if (< (ego y?) 42)
	0x76,                               // push0 (y)
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x04,                         // send 4
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35,   42,                         // ldi 42 (if ego.y < 42)
	0x22,                               // lt?
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds1[] = {
	// (if (< (ego y?) 50)
	PATCH_ADDTOOFFSET(+8),
	  50,                               // 50 (replace 42)
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds2[] = {
	// (ego x: 294 y: 39)
	0x78,                               // push1 (x)
	0x78,                               // push1
	0x38, SIG_UINT16(294),              // pushi 294
	0x76,                               // push0 (y)
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x1d,                         // pushi 29
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x0c,                         // send 12
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds2[] = {
	// (ego x: 320 y: 39)
	PATCH_ADDTOOFFSET(+3),
	PATCH_UINT16(320),                  // 320 (replace 294)
	PATCH_ADDTOOFFSET(+3),
	  39,                               // 39 (replace 29)
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds3[] = {
	// (ego setMotion: MoveTo 282 29 self)
	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion (0x133)
	0x39, 0x04,                         // pushi 4
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	0x38, SIG_UINT16(282),              // pushi 282
	SIG_MAGICDWORD,
	0x39,   29,                         // pushi 29
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x0c,                         // send 12
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds3[] = {
	// (ego setMotion: MoveTo 309 35 self)
	PATCH_ADDTOOFFSET(+9),
	PATCH_UINT16(309),                  // 309 (replace 282)
	PATCH_ADDTOOFFSET(+1),
	  35,                               // 35 (replace 29)
	PATCH_END
};

// When putting the last instance of an item in the chest in room 310 or 430,
//  holding the enter key causes a message to be sent to a non-object.
//
// This bug is similar to the Dag-Nab-It bug in QFG1VGA Mac. A script tries to
//  clear the inventory cursor by setting mainIconBar:curInvIcon to zero without
//  updating curIcon. This briefly places the icon bar in an illegal state which
//  causes mainIconBar:handleEvent to error when enter is pressed.
//
// We fix this by setting mainIconBar:curIcon to theWalkIcon to prevent the
//  illegal state where curInvIcon is zero while curIcon equals useIconItem.
//  useCode:init has two similar code paths with this bug.
//
// Applies to: All versions
// Responsible method: useCode:init
// Fixes bug: #11196
static const uint16 qfg3SignatureChestIconBar[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say [ "You put it in the chest." ]
	0x39, 0x06,                         // pushi 06
	0x78,                               // push1
	0x39, 0x06,                         // pushi 06
	0x39, 0x04,                         // pushi 04
	0x78,                               // push1
	0x76,                               // push0
	0x39, 0x1d,                         // pushi 1d
	0x81, 0x5b,                         // lag 5b
	0x4a, 0x10,                         // send 10 [ qg3Messager say: 1 6 4 1 0 29 ]
	0x38, SIG_SELECTOR16(curInvIcon),   // pushi curInvIcon
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x45,                         // lag 45
	SIG_ADDTOOFFSET(+0x8b),
	0x38, SIG_SELECTOR16(curInvIcon),   // pushi curInvIcon
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x45,                         // lag 45
	0x4a, SIG_MAGICDWORD, 0x06,         // send 06 [ mainIconBar curInvIcon: 0 ]
	0x38, SIG_SELECTOR16(say),          // pushi say [ "You put it in the chest." ]
	SIG_END
};

static const uint16 qfg3PatchChestIconBar[] = {
	0x39, PATCH_SELECTOR8(at),          // pushi at
	0x78,                               // push1
	0x78,                               // push1
	0x81, 0x45,                         // lag 45
	0x4a, 0x06,                         // send 06 [ mainIconBar at: 1 ]
	0x38, PATCH_SELECTOR16(curInvIcon), // pushi curInvIcon
	0x78,                               // push1
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(curIcon),    // pushi curIcon
	0x78,                               // push1
	0x36,                               // push
	0x81, 0x45,                         // lag 45
	0x4a, 0x0c,                         // send 0c [ mainIconBar curInvIcon: 0 curIcon: theWalkIcon ]
	0x32, PATCH_UINT16(0x0094),         // jmp 0094 [ "You put it in the chest." ]
	PATCH_ADDTOOFFSET(+0x8b),
	0x32, PATCH_UINT16(0xff59),         // jmp ff59 [ mainIconBar curInvIcon: 0 curIcon: theWalkIcon ]
	PATCH_END
};

// Entering the jungle close to the Leopardman village and then exiting with
//  Johari causes the game to display corrupt message boxes and divide by zero.
//  Manu's script has the same bugs which could potentially be triggered.
//
// The script fromJungle in room 170 sets local0 to the result of calculations
//  based on ego's initial distance to his destination. The scripts walkJohari
//  and walkManu divide by local0. fromJungle checks to see if it's set local0
//  to zero, but instead of addressing this, it displays a series of messages
//  all at once and corrupts the screen before continuing on to the error.
//
// We fix this by not setting local0 to zero and disabling the broken messages.
//  The message code is unnecessary since later in the same game cycle a local
//  procedure displays the messages correctly. This is similar to the fix in the
//  QFG3 Unofficial Update: https://github.com/AshLancer/QFG3-Fan-Patch
//
// Applies to: All versions
// Responsible method: fromJungle:changeState(0)
// Fixes bug: #11216
static const uint16 qfg3SignatureJohariManuMapBugs[] = {
	SIG_MAGICDWORD,
	0xa3, 0x00,                         // sal 00 [ local0 = result ]
	0x36,                               // push
	0x35, 0x01,                         // ldi 01
	0x22,                               // lt? [ local0 < 1 ]
	0x31,                               // bnt [ skip broken messages ]
	SIG_END
};

static const uint16 qfg3PatchJohariManuMapBugs[] = {
	0x2f, 0x02,                         // bt 02
	0x35, 0x01,                         // ldi 01
	0xa3, 0x00,                         // sal 00 [ local0 = result ? result : 1 ]
	0x33,                               // jmp    [ always skip broken messages  ]
	PATCH_END
};

// The NRS fan-patch, which is included with the GOG release, has a script bug
//  which errors when angering the Guardian in room 770. This can be triggered
//  by taking a second gem. The patch changes ego's moveSpeed from 0 to 2 in the
//  script kickHimOut but it also mistakenly changes another 0 to 2 in the same
//  line of code. Ego:setMotion(0) becomes Ego:setMotion(2) and errors because
//  Actor:setMotion expects an object when passed anything other than 0.
//
// We fix this by reverting the accidental setMotion patch.
//
// Applies to: Any version with NRS patches 770.HEP/SCR, such as GOG
// Responsible method: kickHimOut:changeState(0)
// Fixes bug: #11411
static const uint16 qfg3SignatureNrsAngerGuardian[] = {
	0x38, SIG_MAGICDWORD,               // pushi setMotion
	      SIG_SELECTOR16(setMotion),
	0x78,                               // push1
	0x7a,                               // push2
	SIG_END
};

static const uint16 qfg3PatchNrsAngerGuardian[] = {
	PATCH_ADDTOOFFSET(+4),
	0x76,                               // push0
	PATCH_END
};

//          script, description,                                      signature                    patch
static const SciScriptPatcherEntry qfg3Signatures[] = {
	{  true,   944, "import dialog continuous calls",                     1, qfg3SignatureImportDialog,           qfg3PatchImportDialog },
	{  true,   440, "dialog crash when asking about Woo",                 1, qfg3SignatureWooDialog,              qfg3PatchWooDialog },
	{  true,   440, "dialog crash when asking about Woo",                 1, qfg3SignatureWooDialogAlt,           qfg3PatchWooDialogAlt },
	{  true,    52, "export character save bug",                          2, qfg3SignatureExportChar,             qfg3PatchExportChar },
	{  true,    54, "import character from QfG1 bug",                     1, qfg3SignatureImportQfG1Char,         qfg3PatchImportQfG1Char },
	{  true,   640, "chief in hut priority fix",                          1, qfg3SignatureChiefPriority,          qfg3PatchChiefPriority },
	{  true,   285, "missing points for telling about initiation heap",   1, qfg3SignatureMissingPoints1,         qfg3PatchMissingPoints1 },
	{  true,   285, "missing points for telling about initiation script", 1, qfg3SignatureMissingPoints2a,	      qfg3PatchMissingPoints2 },
	{  true,   285, "missing points for telling about initiation script", 1, qfg3SignatureMissingPoints2b,        qfg3PatchMissingPoints2 },
	{  true,   550, "combat speed throttling script",                     1, qfg3SignatureCombatSpeedThrottling1, qfg3PatchCombatSpeedThrottling1 },
	{  true,   550, "combat speed throttling heap",                       1, qfg3SignatureCombatSpeedThrottling2, qfg3PatchCombatSpeedThrottling2 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds1,         qfg3PatchRoom750Bounds1 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds2,         qfg3PatchRoom750Bounds2 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds3,         qfg3PatchRoom750Bounds3 },
	{  true,    29, "icon bar crash when using chest",                    1, qfg3SignatureChestIconBar,           qfg3PatchChestIconBar },
	{  true,   170, "johari/manu map crash and message bugs",             2, qfg3SignatureJohariManuMapBugs,      qfg3PatchJohariManuMapBugs },
	{  true,   770, "NRS: anger guardian crash",                          1, qfg3SignatureNrsAngerGuardian,       qfg3PatchNrsAngerGuardian },
	{  true,   928, "Narrator lockup fix",                                1, sciNarratorLockupSignature,          sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Quest for Glory 4

// ===========================================================================
// Cranium's TRAP screen in room 380 incorrectly creates an int array for
// string data.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: trap::init() in script 83
// Fixes bug: #10766
static const uint16 qfg4TrapArrayTypeSignature[] = {
	0x38, SIG_SELECTOR16(new),          // pushi new
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0080),           // pushi 128d (array size)
	0x51, SIG_ADDTOOFFSET(+1),          // class IntArray (CD=0x0b. floppy=0x0a)
	0x4a, SIG_UINT16(0x0006),           // send 6
	SIG_END
};

static const uint16 qfg4TrapArrayTypePatch[] = {
	PATCH_ADDTOOFFSET(+4),
	0x38, PATCH_UINT16(0x0100),               // pushi 256d (array size)
	0x51, PATCH_GETORIGINALBYTEADJUST(8, +2), // class ByteArray (CD=0x0d, floppy=0x0c)
	PATCH_END
};

// QFG4 has custom video benchmarking code inside a subroutine, which is called
// by 'glryInit::init', that needs to be disabled. See: sci2BenchmarkSignature.
//
// Applies to at least: English CD, English floppy, German Floppy
// Responsible method: localproc_0010() in script 1
static const uint16 qfg4BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new),          // pushi new
	0x76,                               // push0
	0x51, SIG_ADDTOOFFSET(+1),          // class View
	0x4a, SIG_UINT16(0x0004),           // send 4
	0xa5, 0x00,                         // sat temp[0]
	0x39, SIG_SELECTOR8(view),          // pushi view
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x270f),           // pushi $270f (9999)
	SIG_END
};

static const uint16 qfg4BenchmarkPatch[] = {
	0x35, 0x01,                         // ldi 1
	0xa1, 0xbf,                         // sag global[191]
	0x48,                               // ret
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm.
// At the inn, there is a path that goes off screen. In our pathfinding
// algorithm, we move all the pathfinding points so that they are within
// the visible area. However, two points of the path are outside the
// screen, so moving them will place them both on top of each other,
// thus creating an impossible pathfinding area. This makes the
// pathfinding algorithm ignore the walkable area when the hero moves
// up the ladder to his room. We therefore move one of the points
// slightly, so that it is already within the visible screen, so that
// the walkable polygon is valid, and the pathfinding algorithm can
// work properly.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: rm320::init() in script 320
// Fixes bug: #10693
static const uint16 qfg4InnPathfindingSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0154),  // pushi x = 340
	0x39, 0x77,                // pushi y = 119
	0x38, SIG_UINT16(0x0114),  // pushi x = 276
	0x39, 0x31,                // pushi y = 49
	0x38, SIG_UINT16(0x00fc),  // pushi x = 252
	0x39, 0x30,                // pushi y = 48
	0x38, SIG_UINT16(0x00a5),  // pushi x = 165
	0x39, 0x55,                // pushi y = 85
	0x38, SIG_UINT16(0x00c0),  // pushi x = 192
	0x39, 0x55,                // pushi y = 85
	0x38, SIG_UINT16(0x010b),  // pushi x = 267
	0x39, 0x34,                // pushi y = 52
	0x38, SIG_UINT16(0x0144),  // pushi x = 324
	0x39, 0x77,                // pushi y = 119
	SIG_END
};

static const uint16 qfg4InnPathfindingPatch[] = {
	PATCH_ADDTOOFFSET(+30),
	0x38, PATCH_UINT16(0x013f), // pushi x = 319 (was 324)
	0x39, 0x77,                 // pushi y = 119
	PATCH_END
};

// When autosave is enabled, Glory::save() (script 0) deletes savegame files in
// a loop, while disk space is insufficient for a new save, or while there are
// 20+ saves. Since ScummVM handles slots differently and allows far more
// slots, this deletes all but the most recent 19 manual saves, merely by
// walking from room to room!
//
// Ironically, kGetSaveFiles() (kfile.cpp) and the debugger's 'list_files'
// command rely on listSavegames() (file.cpp), which specifically omits the
// autosave slot, so the script will only ever delete manual saves. And the
// space check doesn't take into account the reduced demand when overwriting an
// existing autosave.
//
// No good can come of this loop. So we skip it entirely. If the disk truly is
// out of space, a message box will complain, and the player can delete saves
// voluntarily.
//
// Note: Glory::save() contains another space freeing loop, but it might be
// unreachable.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: Glory::save() in script 0
// Fixes bug: #10758
static const uint16 qfg4AutosaveSignature[] = {
	0x30, SIG_ADDTOOFFSET(+2),          // bnt ?? [end the loop]
	0x78,                               // push1 (1 call arg)
                                        //
	0x39, SIG_SELECTOR8(data),          // pushi data
	0x76,                               // push0
	SIG_ADDTOOFFSET(+2),                // (CD="lag global[29]", floppy="lat temp[6]")
	0x4a, SIG_UINT16(0x0004),           // send 4d
	0x36,                               // push
	SIG_MAGICDWORD,
	0x43, 0x3f, SIG_UINT16(0x0002),     // callk CheckFreeSpace, 2d
	0x18,                               // not
	0x2f, 0x05,                         // bt 05 [skip other OR condition]
	0x8d, 0x09,                         // lst temp[9] (savegame file count)
	0x35, 0x14,                         // ldi 20d
	0x20,                               // ge?
	SIG_END
};

static const uint16 qfg4AutosavePatch[] = {
	0x32, // ...                        // jmp [end the loop]
	PATCH_END
};

// The swamp areas have typos where a Grooper object is passed to
// View::setLoop(), a method which expects an integer to store in the "loop"
// property. This leads to arithmetic crashes later. We change it to
// Actor::setLooper().
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method:
//                     Script 440
//                         sToWater::changeState(3)
//                     Script 530, 535
//                         sGlideFromTuff::changeState(1)
//                         sGoGlide::changeState(2)
//                         sFromWest::changeState(0)
//                         sFromSouth::changeState(0)
//                     Script 541, 542, 543
//                         sGlideFromTuff::changeState(1)
//                         sFromEast::changeState(0)
//                         sFromNorth::changeState(0)
//                         sFromWest::changeState(0)
//                         sFromSouth::changeState(0)
//                     Script 545
//                         sCombatEnter::changeState(0)
//                         sGlideFromTuff::changeState(2)
//                         sFromNorth::changeState(0)
//                         sFromEast::changeState(0)
//                         sFromWest::changeState(0)
// Fixes bug: #10777
static const uint16 qfg4SetLooperSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x51, 0x5a,                         // class Grooper
	SIG_END
};

static const uint16 qfg4SetLooperPatch1[] = {
	0x38, PATCH_SELECTOR16(setLooper),  // pushi setLooper
	PATCH_END
};

// As above, except it's an exported subclass of Grooper: stopGroop.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sJumpWater::changeState(3), sToJump::changeState(2) in script 10
// Fixes bug: #10777
static const uint16 qfg4SetLooperSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x7a,                               // push2 (2 call args)
	0x39, 0x1c,                         // pushi 28d
	0x78,                               // push1
	0x43, 0x02, SIG_UINT16(0x0004),     // callk ScriptID, 4d (ScriptID 28 1)
	SIG_END
};

static const uint16 qfg4SetLooperPatch2[] = {
	0x38, PATCH_SELECTOR16(setLooper),  // pushi setLooper
	PATCH_END
};

// The panel showing time of day gets stuck displaying consecutive moonrises.
// Despite time actually advancing, the sun will never rise again because
// local[5] is set to 1 for night UI and never resets until hero moves to
// another room.
//
// temp[0] is not used in this method. Thus we can safely ignore and reuse the
// code block that manipulates it in order to fix this bug.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: showTime::init() in script 7
// Fixes bug: #10775
static const uint16 qfg4MoonriseSignature[] = {
	SIG_MAGICDWORD,
	0x81, 0x7a,                         // lag global[122]
	0xa5, 0x00,                         // sat temp[0]
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x06,                         // ldi 6d
	0x1c,                               // ne?
	0x2f, 0x06,                         // bt 6d [skip remaining OR conditions]
	0x89, 0x78,                         // lsg global[120]
	0x34, SIG_UINT16(0x01f4),           // ldi 500d
	0x1e,                               // gt?
	0x31, 0x02,                         // bnt 2d [skip the if block]
	0xc5, 0x00,                         // +at temp[0]
	SIG_END
};

static const uint16 qfg4MoonrisePatch[] = {
	0x35, 0x00,                         // ldi 0 (reset the is-night var)
	0xa3, 0x05,                         // sal local[5]
	0x33, 0x0f,                         // jmp 15d [skip waste bytes]
	PATCH_END
};

// Visiting the inn after rescuing Igor sets a plot flag. Such flags are tested
// on subsequent visits to decide the dialogue options when clicking MOUTH on
// hero. That particular check neglects time of day, allowing hero to talk to
// an empty room after midnight... and get responses from the absent innkeeper.
//
// The inn's init() has a series of cond blocks to boil down all the checks
// into values for local[2], representing discrete situations. Then there's a
// switch block in sInitShit() that acts on those values, making arrival
// announcements and setting new flags.
//
// "So Dmitri says the gypsy didn't really kill Igor after all." sets flag 132.
// "I must thank you for saving our Tanya." sets flag 134.
//
// There are two bugged situations. When you have flag 132 and haven't gotten
// 134 yet, local[2] = 11. When you get flag 134, local[2] = 12. Neither
// of them consider the time of day, talking as if the innkeeper were always
// present.
//
// A day in QFG4 is broken up into 3-hour spans: 6,7,0,1,2,3,4,5. Where 6 is
// midnight. The sun rises in 0 and sets in 4. Current span is global[123]. The
// innkeeper sprite is not around from midnight to morning.
//
// To make room, we optimize block 10's time check:
// "t <= 3 || t is in [4, 5]" becomes "t <= 5". No need for a lengthy call
// to do a simple comparison. Conceptually it meant, "daytime and evening".
//
// That gap is used to insert similar pre-midnight checks before block 11 and
// block 12. This will sync them with the sprite's schedule. Ideally, the
// sprite never would've been scheduled separately in the first place.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: rm320::init() in script 320
// Fixes bug: #10753
static const uint16 qfg4AbsentInnkeeperSignature[] = {
	SIG_MAGICDWORD,                     // (block 10, partway through)
	0x31, 0x1c,                         // bnt 28d [block 11]
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x03,                         // ldi 3d
	0x24,                               // le?
                                        // (~~ junk begins ~~)
	0x2f, 0x0f,                         // bt 15d [after the calle]
	0x39, 0x03,                         // pushi 3d (3 call args)
	0x89, 0x7b,                         // lsg global[123] (needle value)
	0x39, 0x04,                         // pushi 4d (haystack values...)
	0x39, 0x05,                         // pushi 5d
	0x46, SIG_UINT16(0xfde7), SIG_UINT16(0x0005), SIG_UINT16(0x0006), // calle [export 5 of script 64999], 6d (is needle in haystack?))
                                        // (~~ junk ends ~~)
	0x31, 0x04,                         // bnt 4d [block 11]
	0x35, 0x0a,                         // ldi 10d
	0x33, 0x29,                         // jmp 41d [done, local[2]=acc]
                                        //
	SIG_ADDTOOFFSET(+25),               // (...block 11...) (patch ends after this)
	SIG_ADDTOOFFSET(+14),               // (...block 12...)
	SIG_ADDTOOFFSET(+2),                // (...else 0...)
	0xa3, 0x02,                         // sal local[2] (all blocks set acc and jmp here)
	SIG_END
};

static const uint16 qfg4AbsentInnkeeperPatch[] = {
	0x31, 0x0e,                         // bnt 14d [block 11]
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d (make it t <= 5)
	0x24,                               // le?
                                        // (*snip*)
	0x31, 0x07,                         // bnt 7d [block 11]
	0x35, 0x0a,                         // ldi 10d
	0x33, 0x3a,                         // jmp 58d [done, local[2]=acc]
                                        //
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
                                        // (14 freed bytes remain.)
                                        // (use 7 freed bytes to prepend block 11)
                                        // (and shift all of block 11 up by 7 bytes)
                                        // (use that new gap below to prepend block 12)
                                        //
                                        // (block 11, prepend a time check)
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d
	0x24,                               // le?
	0x31, 0x19,                         // bnt 25d [block 12]
                                        // (block 11, original ops shift up)
	0x78,                               // push1 (1 call arg)
	0x38, PATCH_UINT16(0x0084),         // pushi 132d
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb [export 4 of script 0], 2d (test flag 132)
	0x31, 0x0f,                         // bnt 15d [next block]
	0x78,                               // push1 (1 call arg)
	0x38, PATCH_UINT16(0x0086),         // pushi 134d
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb [export 4 of script 0], 2d (test flag 134)
	0x18,                               // not
	0x31, 0x04,                         // bnt 4d [block 12]
	0x35, 0x0b,                         // ldi 11d
	0x33, 0x17,                         // jmp 23d [done, local[2]=acc]
                                        //
                                        // (block 12, prepend a time check)
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d
	0x24,                               // le?
	0x31, 0x0e,                         // bnt 14d [else block]
                                        // (block 12, original ops continue here)
  	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm.
// When entering flipped stairways from the upper door, hero is initially
// placed outside the walkable area. As a result, hero will float around
// inappropriately in stairways leading to Tanya's room (620) and to the iron
// safe (624).
//
// The polygon's first and final points are the top of the stairs. It's quite
// narrow up there, and the final segment doesn't trace the wall very well. We
// move the final point down and over to round out the path. Point 0 takes 19's
// original place. Point 1 takes 0's original place.
//
// Disregard the responsible method's misleading name.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: rm620Code::init() in script 633
// Fixes bug: #10757
static const uint16 qfg4StairwayPathfindingSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00e2),           // pushi 226d (point 0 is top-left)
	0x39, 0x20,                         // pushi 32d
	0x38, SIG_UINT16(0x00ed),           // pushi 237d (point 1 is below on left)
	0x39, 0x26,                         // pushi 38d
	SIG_ADDTOOFFSET(+87),               // ...
	0x38, SIG_UINT16(0x00e9),           // pushi 233d (point 19 is top-right)
	0x39, 0x20,                         // pushi 32d
	SIG_END
};

static const uint16 qfg4StairwayPathfindingPatch[] = {
	0x38, PATCH_UINT16(0x00e9),         // pushi 233d (point 0 gets 19's coords)
	0x39, 0x20,                         // pushi 32d
	0x38, PATCH_UINT16(0x00e2),         // pushi 226d (point 1 gets 0's coords)
	0x39, 0x20,                         // pushi 32d
	PATCH_ADDTOOFFSET(+87),             // ...
	0x38, PATCH_UINT16(0x00fd),         // pushi 253d (point 19 hugs the wall)
	0x39, 0x2b,                         // pushi 43d
	PATCH_END
};

// Whenever levitate is cast, a cryptic error message appears in-game.
// "<Prop setScale:> y value less than vanishingY"
//
// There are typos where hero is passed to Prop::setScale(), a method which
// expects integers. We change it to Prop::setScaler().
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sLevitate::changeState(3) in script 31
//                     sLevitating::changeState(0) in script 800 (CD only)
// Fixes bug: #10726
static const uint16 qfg4SetScalerSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setScale),     // pushi setScale
	0x78,                               // push1
	0x89, 0x00,                         // lsg global[0] (hero)
	SIG_END
};

static const uint16 qfg4SetScalerPatch[] = {
	0x38, PATCH_SELECTOR16(setScaler),  // pushi setScaler
	PATCH_END
};

// The castle's crest-operated bookshelf has an unconditional HAND message
// which always says, "you haven't found the trigger yet," even after it's
// open.
//
// We schedule the walk-out script (sLeaveSecretly) at the end of the opening
// script (sSecret) to force hero to leave immediately, preventing any
// interaction with an open bookshelf.
//
// An automatic exit is consistent with the other bookshelf passage rooms:
// Chandelier (662) and EXIT (661).
//
// Clobbers Glory::handsOn() and sSecret::dispose() to do Room::setScript().
// Both of them are made redundant by setScript's built-in disposal and
// sLeaveSecretly's immediate use of Glory::handsOff().
//
// This patch has two variants, toggled to match the detected edition with
// enablePatch() below. Aside from the patched lofsa value, they are identical.
//
// Applies to at least: English CD
// Responsible method: sSecret::changeState(4) in script 663
// Fixes bug: #10756
static const uint16 qfg4CrestBookshelfCDSignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x003e),           // send 62d
	0x36,                               // push
	SIG_ADDTOOFFSET(+5),                // ...
	0x38, SIG_SELECTOR16(handsOn),      // pushi handsOn (begin clobbering)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1] (Glory)
	0x4a, SIG_UINT16(0x0004),           // send 4d
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 4d
	SIG_END
};

static const uint16 qfg4CrestBookshelfCDPatch[] = {
	PATCH_ADDTOOFFSET(+9),
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x72, PATCH_UINT16(0x01a4),         // lofsa sLeaveSecretly
	0x36,                               // push
	0x81, 0x02,                         // lag global[2] (rm663)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
	PATCH_END
};

// Applies to at least: English floppy, German floppy
static const uint16 qfg4CrestBookshelfFloppySignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x003e),           // send 62d
	0x36,                               // push
	SIG_ADDTOOFFSET(+5),                // ...
	0x38, SIG_SELECTOR16(handsOn),      // pushi handsOn (begin clobbering)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1] (Glory)
	0x4a, SIG_UINT16(0x0004),           // send 4d
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 4d
	SIG_END
};

static const uint16 qfg4CrestBookshelfFloppyPatch[] = {
	PATCH_ADDTOOFFSET(+9),
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x72, PATCH_UINT16(0x018c),         // lofsa sLeaveSecretly
	0x36,                               // push
	0x81, 0x02,                         // lag global[2] (rm663)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
	PATCH_END
};

// Modifies room 663's sLeaveSecretly to avoid obstacles.
//
// Originally intended to start when hero arrives at a doorMat region, room
// 663's walk-out script (sLeaveSecretly) ignores obstacles. The crest
// bookshelf patch repurposes this script and requires collision detection to
// exit properly, walking around the open bookshelf.
//
// Class numbers for MoveTo and PolyPath differ between CD vs floppy editions.
// Their intervals happen to be the same, so we simply offset whatever is
// there.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sLeaveSecretly::changeState(1) in script 663
// Fixes bug: #10756
static const uint16 qfg4CrestBookshelfMotionSignature[] = {
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	SIG_MAGICDWORD,
	0x39, 0x1d,                         // pushi x = 29d
	0x38, SIG_UINT16(0x0097),           // pushi y = 151d
	SIG_END
};

static const uint16 qfg4CrestBookshelfMotionPatch[] = {
	0x51, PATCH_GETORIGINALBYTEADJUST(+1, +6), // class PolyPath
	PATCH_END
};

// In the crest bookshelf room (663) connected to the upper door of the
// bat-infested stairway, peering through the keyhole *always* reports bats.
//
// As you kill the bats, global plot flags are set. Normally flags 331-334
// would be checked via proc0_4(). They're in a bitmask, among other flags, so
// we can check all simultaneously (0000000000011110).
//
// global[520] & 30 == 30
//
// Patch 1: There was no space for this in the responsible method. Instead, we
//  rewrite a different method that became obsolete after the crest bookshelf
//  patch: sCloseSecretDoor::changeState().
//
// Patch 2: We modify sPeepingTom to call our rewritten sCloseSecretDoor. This
//  has two variants, toggled to match the detected edition with enablePatch()
//  below. Aside from the patched lofsa value, they are identical.
//
// Requires patch: qfg4CrestBookshelf (CD or Floppy)
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sPeepingTom::changeState(1) in script 663
// Fixes bug: #10789
static const uint16 qfg4UpperPeerBatsSignature1[] = {
	0x87, 0x01,                         // lap param[1]
	SIG_ADDTOOFFSET(+41),               // ...
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x0006),           // send 6d
	0x35, 0x1e,                         // ldi 30d
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks
	SIG_ADDTOOFFSET(+9),                // ...
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	SIG_END
};

static const uint16 qfg4UpperPeerBatsPatch1[] = {
	0x38, PATCH_SELECTOR16(say),        // pushi say (decide the message as args are stacked up)
	0x39, 0x06,                         // pushi 6d
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x009b),         // pushi 155d

	0x39, 0x1e,                         // pushi 30d (stack up for eq)
	0x3c,                               // dup (stack up another for AND)
	0x80, PATCH_UINT16(0x0208),         // lag global[520] (plot flags bitmask)
	0x12,                               // and
	0x1a,                               // eq? (Were all dead bat flags set?)
	0x2f, 0x04,                         // bt 4d [after this jmp]
	0x39, 0x1d,                         // pushi 29d (bat message)
	0x33, 0x02,                         // jmp 2d [after deciding message]

	0x39, 0x1b,                         // pushi 27d (killed all bats, generic message)

	0x78,                               // push1
	0x76,                               // push0 (don't cue() afterward)
	0x38, PATCH_UINT16(0x0280),         // pushi 640d
	0x81, 0x5b,                         // lag global[91] (gloryMessager)
	0x4a, PATCH_UINT16(0x0010),         // send 16d
	0x48,                               // ret
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (erase 3 bytes to keep disasm aligned)
	PATCH_END
};

// Applies to at least: English CD
static const uint16 qfg4UpperPeerBatsCDSignature2[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	SIG_ADDTOOFFSET(+3),
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x38, SIG_UINT16(0x009b),           // pushi 155d
	0x39, 0x1d,                         // pushi 29d (bat message)
	SIG_ADDTOOFFSET(+7),
	0x4a, SIG_UINT16(0x0010),           // send 16d (say: 2 155 29 1 self 640)
	PATCH_END
};

static const uint16 qfg4UpperPeerBatsCDPatch2[] = {
	0x38, PATCH_SELECTOR16(changeState), // pushi changeState
	0x78,                               // push1
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0176),         // lofsa sCloseSecretDoor
	0x4a, PATCH_UINT16(0x0006),         // send 6d (call the rewritten method)
	0x38, PATCH_SELECTOR16(cue),        // pushi cue
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),         // self 4d (self-cue)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	PATCH_END
};

// Applies to at least: English floppy, German floppy
static const uint16 qfg4UpperPeerBatsFloppySignature2[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	SIG_ADDTOOFFSET(+3),
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x38, SIG_UINT16(0x009b),           // pushi 155d
	0x39, 0x1d,                         // pushi 29d (bat message)
	SIG_ADDTOOFFSET(+7),
	0x4a, SIG_UINT16(0x0010),           // send 16d (say: 2 155 29 1 self 640)
	PATCH_END
};

static const uint16 qfg4UpperPeerBatsFloppyPatch2[] = {
	0x38, PATCH_SELECTOR16(changeState), // pushi changeState
	0x78,                               // push1
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0160),         // lofsa sCloseSecretDoor
	0x4a, PATCH_UINT16(0x0006),         // send 6d (call the rewritten method)
	0x38, PATCH_SELECTOR16(cue),        // pushi cue
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),         // self 4d (self-cue)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	PATCH_END
};

// In the room (644) connected to the lower door of the bat-infested stairway,
// peering through the keyhole *always* reports bats.
//
// As you kill the bats, global plot flags are set. Normally flags 331-334
// would be checked via proc0_4(). They're in a bitmask, among other flags, so
// we can check all simultaneously (0000000000011110).
//
// global[520] & 30 == 30
//
// Room 644 has an IF-ELSE deciding between largely redundant calls to
// gloryMessager::say(). We make room by combining them.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sPeepingTom::changeState(1) in script 644
// Fixes bug: #10789
static const uint16 qfg4LowerPeerBatsSignature[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1 x (check if hero's near the left door)
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, SIG_UINT16(0x0004),           // send 4d
	0x36,                               // push
	0x35, 0x3c,                         // ldi 60d
	0x22,                               // lt?
	0x31, 0x18,                         // bnt 24d [else right door w/ bats]
	0x38, SIG_SELECTOR16(say),          // pushi say (left door, generic message)
	SIG_ADDTOOFFSET(+14),               // ...
	0x81, 0x5b,                         // lag global[91] (gloryMessager)
	0x4a, SIG_UINT16(0x0010),           // send 16d (say: 2 155 27 1 self 640)
	0x33, SIG_ADDTOOFFSET(+1),          // jmp [end the case]
	SIG_ADDTOOFFSET(+22),               // (right door, say(), 3rd arg is 29 for bat message)
	0x33, SIG_ADDTOOFFSET(+1),          // jmp [end the case]
	SIG_END
};

static const uint16 qfg4LowerPeerBatsPatch[] = {
	0x38, PATCH_SELECTOR16(say),        // pushi say (decide the message as args are stacked up)
	0x39, 0x06,                         // pushi 6d
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x009b),         // pushi 155d

	0x78,                               // push1 x (check if left door)
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0x36,                               // push
	0x35, 0x3c,                         // ldi 60d
	0x22,                               // lt?
	0x31, 0x04,                         // bnt 4d [after this jmp]
	0x39, 0x1b,                         // pushi 27d (left door, generic message)
	0x33, 0x10,                         // jmp 16d [after deciding message]

	0x39, 0x1e,                         // pushi 30d (stack up for eq)
	0x3c,                               // dup (stack up another for AND)
	0x80, PATCH_UINT16(0x0208),         // lag global[520] (plot flags bitmask)
	0x12,                               // and
	0x1a,                               // eq? (Were all dead bat flags set?)
	0x2f, 0x04,                         // bt 4d [after this jmp]
	0x39, 0x1d,                         // pushi 29d (right door, bat message)
	0x33, 0x02,                         // jmp 2d [after deciding message]

	0x39, 0x1b,                         // pushi 27d (right door, killed all bats, generic message)

	0x78,                               // push1
	0x7c,                               // pushSelf
	0x38, PATCH_UINT16(0x0280),         // pushi 640d
	0x81, 0x5b,                         // lag global[91] (gloryMessager)
	0x4a, PATCH_UINT16(0x0010),         // send 16d
	0x33, PATCH_GETORIGINALBYTEADJUST(60, +7), // jmp [end the case]
	PATCH_END
};

// The castle's great hall (630) has a doorMat region that intermittently sends
// hero back to the room they just left (barrel room) the instant they arrive.
//
// Entry from room 623 starts hero at (0, 157), the edge of the doorMat. We
// shrink the region by 2 pixels. Then sEnterTheRoom moves hero safely across.
// The region is a rectangle. Point 0 is top-left. Point 3 is bottom-left.
//
// Does not apply to English floppy 1.0. It lacked a western doorMat entirely.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: vClosentDoor::init() in script 630
// Fixes bug: #10731
static const uint16 qfg4GreatHallEntrySignature[] = {
	SIG_MAGICDWORD,
	0x76,                               // push0 (point 0)
	0x38, SIG_UINT16(0x0088),           // pushi 136d
	SIG_ADDTOOFFSET(+10),               // ...
	0x76,                               // push0 0d (point 3)
	0x38, SIG_UINT16(0x00b4),           // pushi 180d
	SIG_END
};

static const uint16 qfg4GreatHallEntryPatch[] = {
	0x7a,                               // push2
	PATCH_ADDTOOFFSET(+13),             // ...
	0x7a,                               // push2
	PATCH_END
};

// In QFG4, the kernel func SetNowSeen() returns void - meaning it doesn't
// modify the accumulator to store a result. Yet math was performed on it!
//
// The function updates boundary box properties of a given View object, prior
// to collision tests. IF (collision detection is warranted, and IF those tests
// are true), THEN respond to the collision.
//
// SetNowSeen() was inserted in the middle of the IF block's list of conditions.
// That way it can be short-circuited, and it runs before the tests.
//
// Problem: void functions make no promise about their truth value. After the
// call, acc will be *whatever* happened to already be in there. This is bad.
//
// "(| (SetNowSeen horror) $0001)"
//
// Someone wrapped the func in a bitwise OR against 1. Thus *every* value is
// guaranteed to become non-zero, always true. And the IF block won't break.
//
// In later SCI2 versions, SetNowSeen() would change to return a boolean.
//
// Whether a lucky confusion or ugly hack, the wrapped void IF condition works.
// When an object leaks into the accumulator. SSCI doesn't mind OR'ing it, too.
// ScummVM detects unsafe arithmetic and crashes. ScummVM needs proper numbers.
//
// "Invalid arithmetic operation (bitwise OR - params: 002e:1694 and 0000:0001)"
//
// We leave the OR wrapper. When the call returns, we manually feed the OR a
// literal 1. Same effect. The IF block goes on evaluating conditions.
//
// Wraith, Vorpal Bunny, and Badder scripts are not affected.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method:
// (ego1, combat hero) Script 41 - xSlash::doit(), xDuck::doit(), xParryLow::doit()
// (ego1, combat hero) Script 810 - slash::doit()
//          (Revenant) Script 830 - revenantForward::doit()
//            (Wyvern) Script 835 - doRSlash::doit(), doLSlash::doit(), tailAttack::doit()
//          (Chernovy) Script 840 - doLSlash::doit(), doRSlash::doit()
//        (Pit Horror) Script 855 - wipeSpell::doit()
//         (Necrotaur) Script 870 - attackLeft::doit(), attackRight::doit(),
//                                  headAttack::doit(), hurtMyself::changeState(1)
// Fixes bug: #10138, #10419, #10710, #10814
static const uint16 qfg4ConditionalVoidSignature[] = {
	SIG_MAGICDWORD,
	0x43, 0x0a, SIG_UINT16(0x0002),     // callk SetNowSeen, 2d (update bounds for a stacked View)
	0x36,                               // push (void func didn't set acc!)
	0x35, 0x01,                         // ldi 1d
	0x14,                               // or (whatever that was, make it non-zero)
	SIG_END
};

static const uint16 qfg4ConditionalVoidPatch[] = {
	PATCH_ADDTOOFFSET(+4),              //
	0x78,                               // push1 (feed OR a literal 1)
	PATCH_END
};

// In the graveyard rescuing Igor, ropes are briefly obscured by crypt pillars
// in the background Pic. The Pic assigns a priority to the pillars for depth.
// Ropes are initialized without priority. Then there's a setPri() call.
//
// The floppy edition's Actor::doit() readily calls UpdateScreenItem(). Thus it
// promptly responds to the new priority, bringing the ropes to the front.
//
// The CD edition changed Actor to require a bit flag on the "signal" property
// before it would call UpdateScreenItem(). So the CD edition graphics don't
// update until much later, when the ropes begin an animation.
//
// We patch the heap for script 500 (the graveyard) to give rope1 and rope2
// that "signal" bit as soon as they're created. This'll be toggled with
// enablePatch() below to only apply to the CD edition.
//
// Applies to at least: English CD
// Responsible method: Actor::doit() in script 64998
// Fixes bug: #10751
static const uint16 qfg4GraveyardRopeSignature1[] = {
	SIG_MAGICDWORD,                     // (rope1 properties)
	SIG_UINT16(0x0064),                 // x = 100d
	SIG_UINT16(0xfff6),                 // y = -10d
	SIG_ADDTOOFFSET(+24),               // ...
	SIG_UINT16(0x01f6),                 // view = 502d
	SIG_ADDTOOFFSET(+8),                // ...
	SIG_UINT16(0x6000),                 // signal = 0x6000
	SIG_END
};

static const uint16 qfg4GraveyardRopePatch1[] = {
	PATCH_ADDTOOFFSET(+38),
	PATCH_UINT16(0x6001),               // signal = 0x6001
	PATCH_END
};

static const uint16 qfg4GraveyardRopeSignature2[] = {
	SIG_MAGICDWORD,                     // (rope2 properties)
	SIG_UINT16(0x007f),                 // x = 127d
	SIG_UINT16(0xfffb),                 // y = -5d
	SIG_ADDTOOFFSET(+24),               // ...
	SIG_UINT16(0x01f6),                 // view = 502d
	SIG_ADDTOOFFSET(+8),                // ...
	SIG_UINT16(0x6000),                 // signal = 0x6000
	SIG_END
};

static const uint16 qfg4GraveyardRopePatch2[] = {
	PATCH_ADDTOOFFSET(+38),
	PATCH_UINT16(0x6001),               // signal = 0x6001
	PATCH_END
};

// Rooms 622 and 623 play an extra door sound when entering. They both
// delegate to script 645. It schedules sEnter, which indeed has an extra
// sound. The CD edition removed the line. We remove it, too.
//
// Applies to at least: English floppy, German floppy
// Responsible method: sEnter::changeState(4) in script 645
// Fixes bug: #10827
static const uint16 qfg4DoubleDoorSoundSignature[] = {
	0x35, 0x04,                         // ldi 4d (state 4)
	SIG_ADDTOOFFSET(+3),                // ...
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(play),          // pushi play
	0x76,                               // push0
	0x72, SIG_UINT16(0x0376),           // lofsa doorSound
	0x4a, SIG_UINT16(0x0004),           // send 4d
	SIG_END
};

static const uint16 qfg4DoubleDoorSoundPatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x33, 0x07,                         // jmp 7d [skip waste bytes]
	PATCH_END
};

// In the castle's iron safe room (643), the righthand door may send hero west
// instead of east - if it was oiled before it was opened (not picked).
//
// The room uses local[2] to remember which door it last decided was nearest.
// The proximity check when opening the right door doesn't reliably set
// local[2]. The assignment was buried inside an IF block testing the oiled
// flag to decide whether the door should squeak. So if the door's been oiled,
// local[2] is not set. If hero had entered the safe from from the west,
// rm643::init() would set local[2] to the left door, and sOpenTheDoor would
// remember LEFT as it decided where to send hero to next.
//
// We move the local[2] assignment out of the IF block, to always run.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sOpenTheDoor::changeState(0) in script 643
// Fixes bug: #10829
static const uint16 qfg4SafeDoorEastSignature[] = {
	SIG_MAGICDWORD,                     // (else block, right door)
	0x78,                               // push1 (1 call arg)
	0x38, SIG_UINT16(0x00d7),           // pushi 215d (right door oiled flag)
	0x45, 0x04, SIG_UINT16(0x0002),     // callb [export 4 of script 0], 2d (test flag 215)
	0x18,                               // not
	0x31, SIG_ADDTOOFFSET(+1),          // bnt ?? [end the else block]
                                        //
	0x35, 0x00,                         // ldi 0
	0xa3, 0x02,                         // sal local[2]
	SIG_END
};

static const uint16 qfg4SafeDoorEastPatch[] = {
	0x35, 0x00,                         // ldi 0
	0xa3, 0x02,                         // sal local[2]
                                        //
	0x78,                               // push1 (1 call arg)
	0x38, PATCH_UINT16(0x00d7),         // pushi 215d (right door oiled flag)
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb [export 4 of script 0], 2d (test flag 215)
	0x18,                               // not
	0x31, PATCH_GETORIGINALBYTEADJUST(10, -4), // bnt ?? [end the else block]
	PATCH_END
};

// In the castle's iron safe room (643), plot flags are mixed up. When hero
// oils either door, the other door's flag is set. Adjacent rooms oil their
// respective doors properly from the outside. We switch the flags inside.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: vBackDoor::doVerb(32), vLeftDoor::doVerb(32) in script 643
// Fixes bug: #10829
static const uint16 qfg4SafeDoorOilSignature[] = {
	0x35, 0x20,                         // ldi 32d (vBackDoor::doVerb(oil), right door)
	SIG_ADDTOOFFSET(+5),                // ...
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00d6),           // pushi 214d (left oiled flag!?)
	0x45, 0x02, SIG_UINT16(0x0002),     // callb [export 2 of script 0], 2d (set flag 214)

	SIG_ADDTOOFFSET(+152),              // ...

	0x35, 0x20,                         // ldi 32d (vLeftDoor::doVerb(oil), left door)
	SIG_ADDTOOFFSET(+5),                // ...
	0x38, SIG_UINT16(0x00d7),           // pushi 215d (right oiled flag!?)
	0x45, 0x02, SIG_UINT16(0x0002),     // callb [export 2 of script 0], 2d (set flag 215)
	SIG_END
};

static const uint16 qfg4SafeDoorOilPatch[] = {
	PATCH_ADDTOOFFSET(+7),
	0x38, PATCH_UINT16(0x00d7),         // pushi 215d (right door, set right oiled flag)
	PATCH_ADDTOOFFSET(+4+152+7),
	0x38, PATCH_UINT16(0x00d6),         // pushi 214d (left door, set left oiled flag)
	PATCH_END
};

// Waking after a dream by the staff in town (room 270) prevents the room from
// creating a doorMat at nightfall, if hero rests repeatedly. The town gate
// closes at night. Without the doorMat, hero isn't prompted to climb over the
// gate. Instead, hero casually walks south and gets stuck in the next room
// behind the closed gate.
//
// Since hero wakes in the morning, sAfterTheDream disposes any existing
// doorMat. It neglects to reset local[2], which toggles rm270::doit()'s
// constant checks for nightfall to replace the doorMat.
//
// We cache an object lookup and use the spare bytes to reset local[2].
//
// Note: There was never any sunrise detection. If hero rests repeatedly until
// morning, the doorMat will linger to needlessly prompt about climbing the
// then-open gate. Harmless. The prompt sets global[423] (1=climb, 2=levitate).
// The gate room only honors that global at night, so hero will simply walk
// through. Heroes unable to climb/levitate would be denied until they re-enter
// the room.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sAfterTheDream::changeState(2) in script 270
// Fixes bug: #10830
static const uint16 qfg4DreamGateSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(heading),       // pushi heading
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa fSouth
	0x4a, SIG_UINT16(0x0004),           // send 4d
	0x31, 0x1a,                         // bnt 26d [skip disposing/nulling] (no heading)
                                        //
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x39, SIG_SELECTOR8(heading),       // pushi heading
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa fSouth
	0x4a, SIG_UINT16(0x0004),           // send 4d (accumulate heading)
	0x4a, SIG_UINT16(0x0004),           // send 4d (dispose heading)
                                        //
	0x39, SIG_SELECTOR8(heading),       // pushi heading
	0x78,                               // push1
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa fSouth
	0x4a, SIG_UINT16(0x0006),           // send 6d (set fSouth's heading to null)
	SIG_END
};

static const uint16 qfg4DreamGatePatch[] = {
	0x3f, 0x01,                         // link 1d (cache heading for reuse)
	0x39, PATCH_SELECTOR8(heading),     // pushi heading
	0x76,                               // push0
	0x72, PATCH_GETORIGINALUINT16(4),   // lofsa fSouth
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0xa5, 0x00,                         // sat temp[0]
	0x31, 0x13,                         // bnt 19d [skip disposing/nulling] (no heading)
                                        //
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x76,                               // push0
	0x85, 0x00,                         // lat temp[0]
	0x4a, PATCH_UINT16(0x0004),         // send 4d (dispose: heading:)
                                        //
	0x39, PATCH_SELECTOR8(heading),     // pushi heading
	0x78,                               // push1
	0x76,                               // push0
	0x72, PATCH_GETORIGINALUINT16(4),   // lofsa fSouth
	0x4a, PATCH_UINT16(0x0006),         // send 6d (set fSouth's heading to null)
                                        //
	0x76,                               // push0
	0xab, 0x02,                         // ssl local[2] (let doit() watch for nightfall)
	PATCH_END
};

// When approaching the town gate at night in room 270, dismissing the menu
//  often doesn't work and instead repeats the gate message and menu. Upon
//  entering the gate's doormat, sTo290Night moves hero up by 6 pixels, assuming
//  that this places him outside the doormat. That assumption is usually wrong
//  since it depends on hero's start position, walk/run mode, and game speed.
//
// We fix this by moving hero one pixel above the doormat instead.
//
// Applies to: All versions
// Responsible method: sTo290Night:changeState(1)
// Fixes bug: #10995
static const uint16 qfg4TownGateDoormatSignature[] = {
	0x7a,                               // push2 [ y ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0004),           // send 04 [ hero:y? ]
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35, 0x06,                         // ldi 06
	0x04,                               // sub
	0x36,                               // push [ hero:y - 6 ]
	SIG_END
};

static const uint16 qfg4TownGateDoormatPatch[] = {
	0x38, PATCH_UINT16(0x00b4),         // pushi 180d [ 1 pixel above doormat ]
	0x33, 0x07,                         // jmp 07
	PATCH_END
};

// Some inventory item properties leak across restarts. Most conspicuously, the
// torch icon appears pre-lit after a restart, if it had been lit before.
//
// script 16 - thePiepan (item #28): loop, cel, value
// script 35 - theBroom (item #39): cel, value
// script 35 - theTorch (item #44): cel
//
// Glory::restart() tries to revert a bunch of globals to their original state.
// Each value was individually loaded into acc and assigned (ldi+sag, ldi+sag,
// ldi+sag, etc).
//
// One range of globals could be distilled to multiples of 45. We optimize
// those with a loop. Another range was arbitrary. We stack up those values all
// at once, then loop over the stack to pop(), assign, and do the next global.
// We use the freed bytes to reset the 3 items' properties with a subroutine.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: Glory::restart() in script 0
// Fixes bug: #10768
static const uint16 qfg4RestartSignature[] = {
	SIG_MAGICDWORD,
	0x76,                               // push0 (this range is multiples of 45)
	0x35, 0x01,                         // ldi 1d
	0xb1, 0x90,                         // sagi (global[144 + 1] = 0)
	SIG_ADDTOOFFSET(+40),               // ...
	0x38, SIG_UINT16(0x013b),           // pushi 315d
	SIG_ADDTOOFFSET(+2),                // ldi ?? (array index here was typoed)
	0xb1, 0x90,                         // sagi (global[144 + ?] = 315)

	0x35, 0x14,                         // ldi 20d (this assignment doesn't fit a pattern)
	0xa1, 0xc6,                         // sag global[198]

	0x35, 0x02,                         // ldi 2d (this range has arbitrary values)
	0xa0, SIG_UINT16(0x016f),           // sag global[367]
	SIG_ADDTOOFFSET(+95),               // ...
	0x35, 0x0a,                         // ldi 10d
	0xa0, SIG_UINT16(0x0183),           // sag global[387]
	SIG_END
};

static const uint16 qfg4RestartPatch[] = {
                                        // (loop to assign multiples of 45)
	0x76,                               // push0
	0xad, 0x00,                         // sst temp[0]
                                        //
	0x8d, 0x00,                         // lst temp[0] (global[144 + n+1] = n*45)
	0x35, 0x08,                         // ldi 8d
	0x22,                               // lt?
	0x31, 0x0c,                         // bnt 12d [end the loop]
	0x8d, 0x00,                         // lst temp[0]
	0x35, 0x2d,                         // ldi 45d
	0x06,                               // mul
	0x36,                               // push (temp[0] * 45)
	0xc5, 0x00,                         // +at temp[0]
	0xb1, 0x90,                         // sagi (global[144 + temp[0]])
	0x33, 0xed,                         // jmp -19d (loop)
                                        // (that loop freed +30 bytes)

	0x35, 0x14,                         // ldi 20d (leave this assignment as-is)
	0xa1, 0xc6,                         // sag global[198]

                                        // (stack up arbitrary values; then loop to assign them)
	0x7a,                               // push2     (global[367] = 2)
	0x3c,                               // dup       (global[368] = 2)
	0x39, 0x03,                         // pushi 3d  (global[369] = 3)
	0x3c,                               // dup       (global[370] = 3)
	0x3c,                               // dup       (global[371] = 3)
	0x39, 0x04,                         // pushi 4d  (global[372] = 4)
	0x39, 0x05,                         // pushi 5d  (global[373] = 5)
	0x3c,                               // dup       (global[374] = 5)
	0x39, 0x06,                         // pushi 6d  (global[375] = 6)
	0x39, 0x07,                         // pushi 7d  (global[376] = 7)
	0x39, 0x08,                         // pushi 8d  (global[377] = 8)
	0x3c,                               // dup       (global[378] = 8)
	0x39, 0x05,                         // pushi 5d  (global[379] = 5)
	0x39, 0x0a,                         // pushi 10d (global[380] = 10)
	0x39, 0x0f,                         // pushi 15d (global[381] = 15)
	0x39, 0x14,                         // pushi 20d (global[382] = 20)
	0x39, 0x06,                         // pushi 6d  (global[383] = 6)
	0x39, 0x08,                         // pushi 8d  (global[384] = 8)
	0x39, 0x07,                         // pushi 7d  (global[385] = 7)
	0x39, 0x0a,                         // pushi 10d (global[386] = 10)
	0x3c,                               // dup       (global[387] = 10)
                                        //
	0x39, 0x15,                         // pushi 21d (pop() and set, backward from 20 to 0)
	0xad, 0x00,                         // sst temp[0]
                                        //
	0xed, 0x00,                         // -st temp[0]
	0x35, 0x00,                         // ldi 0
	0x20,                               // ge?
	0x31, 0x07,                         // bnt 7d [end the loop]
	0x85, 0x00,                         // lat temp[0]
	0xb8, PATCH_UINT16(0x016f),         // ssgi (global[367 + n] = pop())
	0x33, 0xf2,                         // jmp -14d (loop)
                                        // (that loop freed +52 bytes)

                                        // (reset properties for a few items)
	0x33, 0x1f,                         // jmp 31d [skip subroutine declaration]
	0x38, PATCH_SELECTOR16(loop),       // pushi loop
	0x78,                               // push1
	0x8f, 0x02,                         // lsp param[2] (loop varies)
	0x38, PATCH_SELECTOR16(cel),        // pushi cel
	0x78,                               // push1
	0x8f, 0x03,                         // lsp param[3] (cel varies)
	0x38, PATCH_SELECTOR16(value),      // pushi value (weight)
	0x78,                               // push1
	0x7a,                               // push2 (these items all weigh 2)
                                        //
	0x39, PATCH_SELECTOR8(at),          // pushi at
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x81, 0x09,                         // global[9] (gloryInv)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
                                        //
	0x4a, PATCH_UINT16(0x0012),         // send 18d
	0x48,                               // ret

	0x39, 0x03,                         // pushi 3d (call has 3 args)
	0x39, 0x1c,                         // pushi 28d (thePiePan)
	0x7a,                               // push2 (loop)
	0x39, 0x0a,                         // pushi 10d (cel)
	0x40, PATCH_UINT16(0xffd5), PATCH_UINT16(0x0006), // call [-43], 6d

	0x39, 0x03,                         // pushi 3d (call has 3 args)
	0x39, 0x27,                         // pushi 39d (theBroom)
	0x39, 0x0a,                         // pushi 10d (loop)
	0x76,                               // push0 (cel)
	0x40, PATCH_UINT16(0xffc9), PATCH_UINT16(0x0006), // call [-55], 6d

	0x39, 0x03,                         // pushi 3d (call has 3 args)
	0x39, 0x2c,                         // pushi 44d (theTorch)
	0x39, 0x08,                         // pushi 8d (loop)
	0x39, 0x09,                         // pushi 9d (cel)
	0x40, PATCH_UINT16(0xffbc), PATCH_UINT16(0x0006), // call [-68], 6d

	0x33, 0x0a,                         // jmp 10d [skip waste bytes]
	PATCH_END
};

// At the squid monolith (room 800), using the grapnel on the eastern ledge
// disposes hero's scaler to freeze hero's size. A scaler dynamically shrinks
// hero into the horizon as y-pos increases. It makes sense that hero should
// maintain their size while climbing a vertical rope.
//
// Problem: After climbing the rope, both standing on the ledge and back on the
// ground, hero's scaler will remain null. An exception will occur if a script
// calls Prop::setScaler(hero) while hero's scaler is null. Casting Trigger on
// the monolith, from either location, does it. As will climbing down, then
// casting Levitate. Both spells have auras intended to fit hero's size. They
// expect hero to have a scaler, not null.
//
// Ideally the climb script would've swapped in a dummy scaler, then swapped
// the original scaler back upon return to ground level. Implementing that with
// patches would be messy. There's no room to patch setScaler() itself to
// broadly tolerate nulls. That'd avoid exceptions but wouldn't restore normal
// scaling after a climb.
//
// As a last resort, we simply leave the original scaler on hero, erasing the
// setScale() call that would freeze hero's size. The hero shrinks/grows a
// little while climbing as a side effect, but that's barely noticeable.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sUseTheGrapnel::changeState(5) in script 800
// Fixes bug: #10837
static const uint16 qfg4RopeScalerSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setScale),     // pushi setScale
	0x76,                               // push0 (no args, disposes scaler & freezes size)
	SIG_ADDTOOFFSET(+14),               // ...
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, SIG_UINT16(0x0026),           // send 38d
	SIG_END
};

static const uint16 qfg4RopeScalerPatch[] = {
	0x35, 0x01,                         // ldi 0 (erase 2 bytes)
	0x35, 0x01,                         // ldi 0 (erase 2 bytes)
	PATCH_ADDTOOFFSET(+14+2),           // ...
	0x4a, PATCH_UINT16(0x0022),         // send 34d
	PATCH_END
};

// The fortune teller's third reading has the wrong card at the center. The 3rd
// and 4th reading are about different people, yet both have "Queen of Cups".
//
// The 1st reading establishes "Queen of Cups" as: a woman of wisdom and love,
// kind, generous, and virtuous. This fits the 4th reading: she uses her power
// joyfully, giving gracefully and lovingly to others.
//
// The 1st reading establishes "Queen of Swords" as: a deceiver or deceived,
// having suffered through terrible hardship, she faces her sorrows bravely,
// but with deep loneliness. This fits the 3rd reading better: some cruel event
// shaped her life... ambition, self-deception, and she is falling in love.
//
// We change the 3rd reading's center card to "Queen of Swords".
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sThirdReading:changeState(3) in script 475
// Fixes bug: #10824
static const uint16 qfg4Tarot3QueenSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x03f1),           // ldi 1009d ("Queen of Cups")
	0xa3, 0x00,                         // sal local[0]
	SIG_ADDTOOFFSET(+46),               // ...
	0x39, 0x1f,                         // pushi 31d (say: 1 6 31 0 self)
	SIG_END
};

static const uint16 qfg4Tarot3QueenPatch[] = {
	0x34, PATCH_UINT16(0x03ed),         // ldi 1005d ("Queen of Swords")
	PATCH_END
};

// The fortune teller's third reading displays a right-turned "The Devil" card,
// but Magda says it is "Death".
//
// We change the card to a right-turned "Death".
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sThirdReading:changeState(15) in script 475
// Fixes bug: #10823
static const uint16 qfg4Tarot3DeathSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x03fa),           // ldi 1018d ("The Devil")
	0xa3, 0x00,                         // sal local[0]
	SIG_ADDTOOFFSET(+46),               // ...
	0x39, 0x23,                         // pushi 35d (say: 1 6 35 0 self)
	SIG_END
};

static const uint16 qfg4Tarot3DeathPatch[] = {
	0x34, PATCH_UINT16(0x03fd),         // ldi 1021d ("Death")
	PATCH_END
};

// The fortune teller's third reading places the "Two of Cups" across another
// card, off-center. That View (1023) is cropped (130x64). All other horizontal
// cards (130x86, 130x87) are padded at the bottom with transparent pixels.
//
// A utility script (sShowCard) is scheduled to create each card with a given
// View (passed as local[0]) and move it onto a given pile (passed as local[2]:
// center, west, south, east, north). It has a switch block deciding x,y coords
// depending on the pile.
//
// We optimize a couple cases by consolidating their common code in a
// subroutine to make room. The rewritten case for the east pile checks if the
// requested card was "Two of Cups". If so, a special Y value is used.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sShowCard::changeState() in script 475
// Fixes bug: #10822
static const uint16 qfg4Tarot3TwoOfCupsSignature[] = {
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 4d (case 4)
	0x1a,                               // eq?
	SIG_ADDTOOFFSET(+7),                // ...
	0x38, SIG_SELECTOR16(setStep),      // pushi setStep
	SIG_ADDTOOFFSET(+11),               // ...
	0x51, SIG_ADDTOOFFSET(+1),          // class Scaler
	SIG_ADDTOOFFSET(+16),               // ...
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	SIG_ADDTOOFFSET(+72),               // ...
	0x3a,                               // toss (end of this local[2] switch)
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [end the state switch]
	SIG_END
};

static const uint16 qfg4Tarot3TwoOfCupsPatch[] = {
	0x33, 0x31,                         // jmp 49d [skip subroutine declaration]
	0x38, PATCH_SELECTOR16(moveSpeed),  // pushi moveSpeed
	0x78,                               // push1
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(setStep),    // pushi setStep
	0x7a,                               // push2
	0x39, 0x1e,                         // pushi 30d
	0x39, 0x0a,                         // pushi 10d
	0x38, PATCH_SELECTOR16(setScaler),  // pushi setScaler
	0x39, 0x05,                         // pushi 5d
	0x51, PATCH_GETORIGINALBYTE(26),    // class Scaler
	0x36,                               // push
	0x39, 0x64,                         // pushi 100d
	0x39, 0x23,                         // pushi 35d
	0x38, PATCH_UINT16(0x0096),         // pushi 150d
	0x8f, 0x01,                         // lsp param[1] (setScalar, arg 5 varies)
	0x38, PATCH_SELECTOR16(setMotion),  // pushi setMotion
	0x39, 0x04,                         // pushi 4d
	0x51, PATCH_GETORIGINALBYTE(44),    // class MoveTo
	0x36,                               // push
	0x8f, 0x02,                         // lsp param[2] (setMotion, x arg varies)
	0x8f, 0x03,                         // lsp param[3] (setMotion, y arg varies)
	0x7c,                               // pushSelf
	0x83, 0x01,                         // lal local[1]
	0x4a, PATCH_UINT16(0x0028),         // send 40d
	0x48,                               // ret

	0x3c,                               // dup
	0x35, 0x04,                         // ldi 4d (case 4)
	0x1a,                               // eq?
	0x31, 0x1b,                         // bnt 27d [next case]
	0x39, 0x03,                         // pushi 3d (call has 3 args)
	0x39, 0x6e,                         // pushi 110d (setScalar, arg 5)
	0x38, PATCH_UINT16(0x00d2),         // pushi 210d (setMotion, x arg)
                                        //
	0x8b, 0x00,                         // lsl local[0] (test the card's View number)
	0x34, PATCH_UINT16(0x03ff),         // ldi 1023d ("Two of Cups" is special)
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 4d [regular y arg]
	0x39, 0x66,                         // pushi 102d  (setMotion, special y arg)
	0x33, 0x02,                         // jmp 2d [to the call]
	0x39, 0x6e,                         // pushi 110d (setMotion, regular y arg)
                                        //
	0x41, 0xb0, PATCH_UINT16(0x0006),   // call [-80], 6d
	0x33, 0x13,                         // jmp 19d [end the local[2] switch]

	0x3c,                               // dup
	0x35, 0x05,                         // ldi 5d (case 5)
	0x1a,                               // eq?
	0x31, 0x0d,                         // bnt 13d [end the local[2] switch]
	0x39, 0x03,                         // pushi 3d (call has 3 args)
	0x39, 0x32,                         // pushi 50d (setScalar, arg 5)
	0x38, PATCH_UINT16(0x0090),         // pushi 144d (setMotion, x arg)
	0x39, 0x32,                         // pushi 50d (setMotion, y arg)
	0x41, 0x9b, PATCH_UINT16(0x0006),   // call [-101], 6d

	0x33, 0x0c,                         // jmp 12d [skip to the original toss that ends this switch]
	PATCH_END
};

// The fortune teller's third reading places horizontal cards on top of
// vertical ones. Some then fall *through* the vertical card to the bottom.
//
// A utility script (sShowCard) creates each card and moves it onto a pile
// (center, west, south, east, north). Then the card is assigned a priority
// with setPri(). Generally these piles are two cards deep. Center has one.
//
// Every pile ought to start with priority 0 and increment thereafter. Somebody
// mixed up the setPri() sequence. We change 0;1,0;1,0 to 0;0,1;0,1.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sThirdReading:changeState() in script 475
// Fixes bug: #10845
static const uint16 qfg4Tarot3PrioritySignature[] = {
	0x78,                               // push1 (setPri: 1, "Eight of Swords", West, V)
	SIG_ADDTOOFFSET(+14),               // ...
	0x39, 0x20,                         // pushi 32d (say cond:32)
	SIG_ADDTOOFFSET(+68),               // ...
	0x76,                               // push0 (setPri: 0, "Strength", West, H)
	SIG_ADDTOOFFSET(+84),               // ...
	0x78,                               // push1 (setPri: 1, "The Magician", South, V)
	SIG_ADDTOOFFSET(+84),               // ...
	0x76,                               // push0 (setPri: 0, "Death", South, H)
	SIG_ADDTOOFFSET(+12),               // ...
	SIG_MAGICDWORD,
	0x39, 0x06,                         // pushi 6d (say verb:6)
	0x39, 0x23,                         // pushi 36d (say cond:35)
	SIG_END
};

static const uint16 qfg4Tarot3PriorityPatch[] = {
	0x76,                               // push0 (setPri: 0, "Eight of Swords", West, V)
	PATCH_ADDTOOFFSET(+84),             // ...
	0x78,                               // push1 (setPri: 1, "Strength", West, H)
	PATCH_ADDTOOFFSET(+84),             // ...
	0x76,                               // push0 (setPri: 0, "The Magician", South, V)
	PATCH_ADDTOOFFSET(+84),             // ...
	0x78,                               // push1 (setPri: 1, "Death", South, H)
	PATCH_END
};

// The fortune teller's fifth reading is unusual. It places all cards at the
// center pile, periodically fading out to clear the table. When Magda
// talks about the Sense Ritual, the "Six of Swords" (view 1048) sinks below
// the previous card.
//
// A shared utility script that creates and places cards (sSetTheSignificator)
// uses priority 12 as it deals, after which each card is given a lower value
// with setPri(). "The Falling Tower" (view 1031) did *not* get a new priority.
// Thus "Six of Swords", when given priority 1, sinks below 12.
//
// "Six of Swords" is the last card before a fade. We simply leave its priority
// at 12 as well. Being the most recent card, it will be on top. No worry
// about covering a subsequent card because the table will be cleared.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sFifthReading:changeState(32) in script 475
// Fixes bug: #10846
static const uint16 qfg4Tarot5PrioritySignature[] = {
	0x39, SIG_ADDTOOFFSET(+1),          // pushi setPri
	0x78,                               // push1
	0x78,                               // push1 (setPri: 1, "Six of Swords")
	SIG_MAGICDWORD,
	0x83, 0x01,                         // lal local[1] (card obj)
	0x4a, SIG_UINT16(0x0006),           // send 6d
	SIG_ADDTOOFFSET(+9),                // ...
	0x39, 0x44,                         // pushi 68d (say cond:68)
	SIG_END
};

static const uint16 qfg4Tarot5PriorityPatch[] = {
	0x33, 0x07,                         // jmp 7d [skip the setPri() send]
	PATCH_END
};

// When crossing the cave's tightrope in room 710, a tentacle emerges, and then
// its animation freezes - in ScummVM, not the original interpreter. This
// happens because of an extraneous argument passed to setCycle().
//
// (tentacle setCycle: RandCycle tentacle)
//
// RandCycle can accept an optional arg, but it expects a number, not an
// object. ScummVM doesn't catch the faulty arithmetic and behaves abnormally.
// We remove the bad arg.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sTentacleDeath::changeState(3) in script 710
// Fixes bug: #10615
static const uint16 qfg4TentacleWriggleSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x7a,                               // push2
	0x51, SIG_ADDTOOFFSET(+1),          // class RandCycle
	0x36,                               // push
	0x72, SIG_ADDTOOFFSET(+2),          // loffsa tentacle
	0x36,                               // push
	0x72, SIG_ADDTOOFFSET(+2),          // loffsa tentacle
	0x4a, SIG_UINT16(0x0008),           // send 8d
	SIG_END
};

static const uint16 qfg4TentacleWrigglePatch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                               // push1 (1 setCycle arg)
	PATCH_ADDTOOFFSET(+3),              // ...
	0x35, 0x00,                         // ldi 0 (erase 2 bytes)
	0x35, 0x00,                         // ldi 0 (erase 2 bytes)
	PATCH_ADDTOOFFSET(+3),              // ...
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	PATCH_END
};

// When crossing the cave's tightrope in room 710, a tentacle emerges. The
// tentacle is supposed to reach a state where it waits indefinitely for an
// external cue() to retract. If the speed slider is too high, a fighter
// reaches the other side and sends a cue() before the tentacle is ready to
// receive it. The tentacle never retracts.
//
// The fighter script (crossByHand) drains stamina in state 2 as hero moves
// across. A slower speed would cost extra stamina. We add a delay after that
// part is over, in state 3, just as hero is about to dismount. When state 4
// cues, the tentacle script (sTentacleDeath) will be ready (state 3).
//
// To create that delay we set the "cycles" property for a countdown and remove
// all other advancement mechanisms. State 3 had a cue from say() and a
// self-cue(). The former's "self" arg becomes null. The latter is erased.
//
// Crossing from the left (crossByHandLeft) doesn't require fixing.
//
// This patch doesn't apply to the NRS version which ships with the GOG release
//  as it throttles the frequency of crossByHand:doit which fixes the bug.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: crossByHand::changeState(3) in script 710
// Fixes bug: #10615
static const uint16 qfg4PitRopeFighterSignature[] = {
	0x65, SIG_ADDTOOFFSET(+1),          // aTop state
	SIG_ADDTOOFFSET(+269),              // ...
	0x31, 0x1e,                         // bnt 30d (set a flag and say() on 1st crossing, else cue)
	SIG_ADDTOOFFSET(+8),                // ...
	0x38, SIG_SELECTOR16(say),          // pushi say ("You just barely made it")
	0x38, SIG_UINT16(0x0005),           // pushi 5d
	0x39, 0x0a,                         // pushi 10d
	0x39, 0x06,                         // pushi 6d
	0x39, 0x20,                         // pushi 32d
	0x76,                               // push0
	0x7c,                               // pushSelf
	SIG_ADDTOOFFSET(+5),                // ...
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 4d
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_END
};

static const uint16 qfg4PitRopeFighterPatch[] = {
	PATCH_ADDTOOFFSET(+271),            // ... (2 + 269)
	0x31, 0x1b,                         // bnt 26d [skip the say w/o ending the switch]
	PATCH_ADDTOOFFSET(+21),             // ... (8 + 13)
	0x76,                               // push0 (null caller, so say won't cue crossByHand)
	PATCH_ADDTOOFFSET(+5),              // ...
                                        // (no jmp, self-cue becomes cycles)
	0x35, 0x20,                         // ldi 32d
	0x65, PATCH_GETORIGINALBYTEADJUST(1, +6), // aTop cycles (property offset = @state + 6d)
	0x33, 0x04,                         // jmp 4d [skip waste bytes, end the switch]
	PATCH_END
};

// As above, mages at high speed can get across the pit in room 710 before
// tentacle is ready to receive a cue().
//
// As luck would have it, hero's speed is cached and restored. Twice actually.
//
// Overview of the mage script (sLevitateOverPit)
//  State 1-3:  Cache hero's slider-based speed.
//              Set a temporary speed as hero unfurls the cloth.
//              Restore the original value.
//              Call handsOn(). Wait for an external cue().
//  State 4:    Call handsOff().
//              If cued by the Levitate spell (script 21), go to 5-7.
//              A plain cue() from anywhere else, leads to state 8.
//  State 5-7:  Move across the pit. Skip to state 9.
//  State 8:    An abort message.
//  State 9-10: Cache hero's speed again.
//              Set a temporary speed as hero folds up the cloth.
//              Restore the original value, and normalize hero. Call handsOn().
//
// Patch 1: We overwrite some derelict code in state 5, caching the
// slider-based speed again, in case the player adjusted it before casting
// Levitate, then setting a fixed speed of our own for the crossing.
//
// Patch 2: Patch 1 already cached and clobbered the speed. We remove the
// original attempt to cache again in state 9.
//
// The result is caching/restoration at the beginning, aborting or caching and
// crossing with our fixed value, and a restoration at the end (whichever value
// was last cached). The added travel time has no side effect for mages.
//
// Mages have no other script to levitate across from left to right. At some
// point in development, the meaning of "register" changed. The derelict
// state 5 code thought 0/1 meant move right/left. Whereas state 4 decides 0/1
// means abort/cross, only ever moving left. The rightward MoveTo never runs.
//
// We also include a version of this for the instruction sizes in the NRS patch,
//  which is important as that ships with the GOG version.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sLevitateOverPit::changeState(5) in script 710
// Fixes bug: #10615
static const uint16 qfg4PitRopeMageSignature1[] = {
	0x30, SIG_UINT16(0x0017),           // bnt 23d [if register == 0 (never), move right]
	SIG_ADDTOOFFSET(+20),               // ... (move left)
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]

	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion (move right)
	0x38, SIG_UINT16(0x0004),           // pushi 4d
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00da),           // pushi 218d
	0x39, 0x30,                         // pushi 48d
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, SIG_UINT16(0x000c),           // send 12d
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_END
};

static const uint16 qfg4PitRopeMagePatch1[] = {
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (erase the branch)
	PATCH_ADDTOOFFSET(+20),             // ...

	0x38, PATCH_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0xa3, 0x02,                         // sal local[2] (cache again)
	                                    //
	0x38, PATCH_SELECTOR16(setSpeed),   // pushi setSpeed
	0x78,                               // push1
	0x39, 0x08,                         // pushi 8d (set our fixed speed)
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	0x5c,                               // selfID (erase 1 byte to keep disasm aligned)
	PATCH_END
};

static const uint16 qfg4PitRopeMageNrsSignature1[] = {
	0x30, SIG_UINT16(0x0016),           // bnt 22d [if register == 0 (never), move right]
	SIG_ADDTOOFFSET(+19),               // ... (move left)
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]

	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion (move right)
	0x39, 0x04,                         // pushi 4d
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00da),           // pushi 218d
	0x39, 0x30,                         // pushi 48d
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, SIG_UINT16(0x000c),           // send 12d
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_END
};

static const uint16 qfg4PitRopeMageNrsPatch1[] = {
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (erase the branch)
	PATCH_ADDTOOFFSET(+19),             // ...

	0x38, PATCH_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0xa3, 0x02,                         // sal local[2] (cache again)
	                                    //
	0x38, PATCH_SELECTOR16(setSpeed),   // pushi setSpeed
	0x78,                               // push1
	0x39, 0x08,                         // pushi 8d (set our fixed speed)
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	PATCH_END
};

// Responsible method: sLevitateOverPit::changeState(9) in script 710
static const uint16 qfg4PitRopeMageSignature2[] = {
	SIG_MAGICDWORD,
	0x35, 0x09,                         // ldi 9d (case 9 label)
	0x38, SIG_SELECTOR16(cycleSpeed),   // pushi cycleSpeed
	SIG_ADDTOOFFSET(+6),                // ...
	0xa3, 0x02,                         // sal local[2] (original re-cache)
	SIG_ADDTOOFFSET(+48),               // ...
	0x38, SIG_SELECTOR16(cycleSpeed),   // pushi cycleSpeed
	0x78,                               // push1
	0x8b, 0x02,                         // lsl local[2] (restore cached speed)
	SIG_END
};

static const uint16 qfg4PitRopeMagePatch2[] = {
	PATCH_ADDTOOFFSET(+11),             // (don't cache our fixed speed)
	0x35, 0x00,                         // ldi 0 (erase 2 bytes)
	PATCH_ADDTOOFFSET(+48),             // ...
	0x38, PATCH_SELECTOR16(setSpeed),   // pushi setSpeed (keep cycleSpeed & moveSpeed sync'd)
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm.
// When entering forest room 557 from the east (563), hero is supposed to move
// only a short distance into the room. ScummVM's pathfinding sends hero off
// course, to the middle of the room and back.
//
// There's an unwalkable stream in the SE corner, and hero's coords were within
// its polygon. We lower the top two points to keep hero on the outside.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: rm557::init() in script 557
// Fixes bug: #10857
static const uint16 qfg4Forest557PathfindingSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0119),           // pushi 281d (point 3)
	0x38, SIG_UINT16(0x0087),           // pushi 135d
	0x38, SIG_UINT16(0x013f),           // pushi 319d (point 4)
	0x38, SIG_UINT16(0x0087),           // pushi 135d
	SIG_END
};

static const uint16 qfg4Forest557PathfindingPatch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x38, PATCH_UINT16(0x0089),         // pushi 137d
	PATCH_ADDTOOFFSET(+3),
	0x38, PATCH_UINT16(0x0089),         // pushi 137d
	PATCH_END
};

// The Trigger spell stalls and never reaches handsOn when preceded by a
// successful Summon Staff. An IF block calls hero::setCycle(Beg, self), which
// cues self on completion. Its condition tests hero's "view" property and
// executes if the staff is absent. There are no means to advance when the
// staff is present.
//
// Open (script 13) is largely identical w/o this bug. We match its behavior.
//
// Due to differences between editions, this is addressed with two patches. The
// first inserts a "seconds" property assignment before the IF, where it'll
// always cue. We make room by condensing the IF conditions. There are two
// "view" comparisons. Instead of sending for it twice, we recycle the view
// with pprev. The second patch removes setCycle's cue by nulling its last arg.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: castTriggerScript::changeState(2) in script 11
// Fixes bug: #10860
static const uint16 qfg4TriggerStaffSignature1[] = {
	0x65, SIG_ADDTOOFFSET(+1),          // aTop register

	SIG_ADDTOOFFSET(+9),                // ... (send for hero's view, push for comparison)
	0x35, 0x11,                         // ldi 17d
	0x1e,                               // gt?
	0x31, SIG_ADDTOOFFSET(+1),          // bnt ?? [to the not]
	SIG_ADDTOOFFSET(+9),                // ... (send for hero's view and push again)
	SIG_MAGICDWORD,
	0x35, 0x15,                         // ldi 21d
	0x22,                               // lt?
	0x31, SIG_ADDTOOFFSET(+1),          // bnt ?? [to the not]
	//0x33, 0x00,                       // (Floppy has a jmp 0 here)
	//0x18,                             // not ( !(view > 17 && view < 21) )
	SIG_END
};

static const uint16 qfg4TriggerStaffPatch1[] = {
	PATCH_ADDTOOFFSET(+2),                    // (free bytes later, use them up here)
	0x35, 0x03,                               // ldi 3d
	0x65, PATCH_GETORIGINALBYTEADJUST(1, -8), // aTop seconds (property offset = @register - 8d)
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)

	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, PATCH_UINT16(0x0004),         // send 4d
                                        //
	0x39, 0x11,                         // pushi 17d (push the literal, leave view in acc)
	0x22,                               // lt? (view > 17 becomes 17 < view, set prev = acc)
	0x31, PATCH_GETORIGINALBYTEADJUST(15, -8), // bnt ?? [to the not]
                                        //
	0x60,                               // pprev (push the view from prev, ldi 21 comparison is next)
	PATCH_END
};

static const uint16 qfg4TriggerStaffSignature2[] = {
	SIG_MAGICDWORD,
	0x31, 0x0d,                         // bnt 13d [conditions failed, skip the send]
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x7a,                               // push2
	0x51, SIG_ADDTOOFFSET(+1),          // class Beg
	0x36,                               // push
	0x7c,                               // pushSelf (caller arg is cued afterward)
	SIG_END
};

static const uint16 qfg4TriggerStaffPatch2[] = {
	PATCH_ADDTOOFFSET(+9),
	0x76,                               // push0 (null caller arg, no cue)
	PATCH_END
};

// The Open and Trigger spells init a green Prop for their effect. They don't
// dispose it the first time, and the effect is absent on further castings.
//
// The author specifically nerfed dispose() on its first call by testing if a
// variable has been set before allowing super::dispose(). We erase the
// branch to ensure that the effect is always disposed.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: triggerEffect::dispose() in script 11
//                     openEffect::dispose() in script 13
// Fixes bug: #10860
static const uint16 qfg4EffectDisposalSignature[] = {
	0x83, SIG_ADDTOOFFSET(+1),          // lal local[?] (0 on first call, 1 thereafter)
	SIG_MAGICDWORD,
	0x31, 0x0a,                         // bnt 10d [skip super::dispose()]
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x57, SIG_ADDTOOFFSET(+1), SIG_UINT16(0x0004), // super Prop, 4d
	0x33, 0x04,                         // jmp 4d [ret]

	0x35, 0x01,                         // ldi 1d (enable normal disposal)
	0xa3, SIG_ADDTOOFFSET(+1),          // sal local[?]
	SIG_END
};

static const uint16 qfg4EffectDisposalPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x00,                         // ldi 0 (erase the branch to always dispose)
	PATCH_END
};

// After hero is geas'd in the dungeon (room 670) and teleported to the gate
// (600), hero can walk through the closed gate and exit north to the castle
// entrance. Two IF blocks with inconsistent conditions decide whether the
// gate is open and whether to use a polygon that extends beyond the gate.
// When re-entering from the forest (552), the gate is only open and passable
// if hero is qualified.
//
// The room has distinct situations for merely teleporting from the dungeon
// (local[0] = 11) and for entering from the forest while geas'd and carrying
// all the ritual scrolls (local[0] = 10). The latter sets a vital plot flag.
// Adding those checks and the flag to the former, plus opening the gate,
// would be non-trivial.
//
// We edit the polygon's IF condition to remove the dungeon check, making the
// closed gate impassable so hero will have to return from the forest.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: rm600::init() in script 600
// Fixes bug: #10871
static const uint16 qfg4DungeonGateSignature[] = {
	0x39, 0x05,                         // pushi 5d (5 call args)
	0x89, 0x0c,                         // lsg global[12] (needle value)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x029e),           // pushi 670 (Dungeon)
	0x38, SIG_UINT16(0x032a),           // pushi 810 (Combat)
	0x38, SIG_UINT16(0x0262),           // pushi 610 (Castle entrance)
	0x38, SIG_UINT16(0x0276),           // pushi 630 (Great hall)
	0x46, SIG_UINT16(0xfde7), SIG_UINT16(0x0005), SIG_UINT16(0x000a), // calle [export 5 of script 64999], 10d (is needle in haystack?)
	SIG_END
};

static const uint16 qfg4DungeonGatePatch[] = {
	0x39, 0x04,                         // pushi 4d (4 call args)
	PATCH_ADDTOOFFSET(+2),              // ...
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (erase the Dungeon arg)
	PATCH_ADDTOOFFSET(+9),              // ...
	0x46, PATCH_UINT16(0xfde7), PATCH_UINT16(0x0005), PATCH_UINT16(0x0008), // calle [export 5 of script 64999], 8d (is needle in haystack?)
	PATCH_END
};

// In the room (644) attached to the lower door of the bat-infested stairway,
// a rogue will get stuck when attempting to open either door. Unlike in other
// castle rooms, the door Tellers here aren't arranging to be cued after the
// "It won't budge" message. Without the cue, a Teller won't clean() and return
// control to the player.
//
// We follow the style of other rooms and replace gloryMessager::say() with
// super::sayMessage(), which implicitly cues.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: leftDoorTeller::sayMessage(), rightDoorTeller::sayMessage() in script 644
// Fixes bug: #10874
static const uint16 qfg4StuckDoorSignature[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x38, SIG_UINT16(0x0006),           // pushi 6d
	0x39, 0x03,                         // pushi 3d
	SIG_MAGICDWORD,
	0x39, 0x06,                         // pushi 6d
	0x39, 0x09,                         // pushi 9d
	0x78,                               // push1
	0x76,                               // push0
	0x38, SIG_UINT16(0x0280),           // pushi 640d
	0x81, 0x5b,                         // lag global[91]
	0x4a, SIG_UINT16(0x0010),           // send 16d
	SIG_ADDTOOFFSET(+89),               // ...
	0x57, SIG_ADDTOOFFSET(+1), SIG_UINT16(0x0004), // super Teller, 4d
	SIG_END
};

static const uint16 qfg4StuckDoorPatch[] = {
	0x38, PATCH_SELECTOR16(sayMessage), // pushi sayMessage
	0x39, 0x03,                         // pushi 3d
	0x3c,                               // dup
	0x39, 0x06,                         // pushi 6d
	0x39, 0x09,                         // pushi 9d
	0x59, 0x01,                         // &rest 1d
	0x57, PATCH_GETORIGINALBYTE(112), PATCH_UINT16(0x000a), // super Teller, 10d
	0x32, PATCH_UINT16(0x0003),         // jmp 3d [skip waste bytes]
	PATCH_END
};

// In the thieves' guild (room 430), the tunnel is not immediately walkable
// when it is revealed (by moving a barrel and solving a puzzle). Hero must
// re-enter the room to update the polygon.
//
// Curing Chief *will* immediately replace the polygon. However, most players
// will lack the item necessary on the first visit. Meeting Chief is how they
// learn about the item. If they go get it, they'll re-enter the room.
//
// The room's init has a cond block to check plot flags and declare one of 3
// polygons. The 3rd condition also inits secritExit, an invisible Feature
// that sends hero out of town when walked upon.
//
// Patch 1: Other patches ensure the passage will be walkable the moment it is
//  revealed. Chief is standing inside it. We skip the original code that would
//  set up the passage as he gets cured. It is redundant now. If hero can reach
//  him, the passage is already revealed. We won't let secritExit init twice.
//
// Patch 2: We free bytes in rm340::init() by condensing Feature inits with a
//  loop. Stack up their addresses. Pop & send repeatedly. Then we declare a
//  subroutine that disposes any existing obstacles, jumps into the cond block
//  to declare the 3rd poly, jumps back, passes it to addObstacles(), and inits
//  secritExit.
//
//  When the cond block's 3rd condition runs, we immediately call our
//  subroutine to do everything and end the cond, leaving the original polygon
//  declaration intact below the jump.
//
// Patch 3: The passage starts opening at sBarrelMove state 8. We need more
//  room than case 8 can offer, so we arrange for *multiple* cases to run
//  during state 8 - by omitting the final jump that would short-circuit.
//
//  Cases 1-5 have derelict code, once intended to move the barrel back and
//  forth, now only left. This is because barrel::doVerb(4) schedules
//  sBarrelMove in the absence of flag 254 and sets register=1 if the barrel is
//  in the left position already. Case 0 uses the same criteria in deciding to
//  skip to state 6. Thus cases 1-5 never see register==1. The barrel never
//  moves back, and bytes predicated on register==1 are available.
//
//  We reduce case 2 to only the necessary ops and splice in a new case that
//  runs during state 8 as a prologue to the original case 8. Our prologue
//  calls the subroutine to add the 3rd polygon. This patch has two variants,
//  toggled to match the detected edition with enablePatch() below. Aside from
//  the call offset, they are identical.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: sChangeThief::changeState() in script 340
// Fixes bug: #9894
static const uint16 qfg4GuildWalkSignature1[] = {
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	SIG_ADDTOOFFSET(+20),               // ... (dispose and null global[2]'s "obstacles" property)
	0x4a, SIG_UINT16(0x0006),           // send 6d
	SIG_ADDTOOFFSET(+85),               // ...
	SIG_ADDTOOFFSET(+238),              // ... (secritExit init and addObstacle)
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x00a6),           // send 166d (polygon init)
	0x36,                               // push
	SIG_ADDTOOFFSET(+5),                // ... (global[2] addObstacle: polygon)
	SIG_END
};

static const uint16 qfg4GuildWalkPatch1[] = {
	0x32, PATCH_UINT16(0x0017),         // jmp 23d (skip obstacles disposal)
	PATCH_ADDTOOFFSET(+108),
	0x32, PATCH_UINT16(0x00f4),         // jmp 244d (skip secritExit and polygon)
	PATCH_END
};

// Responsible method: rm340::init() in script 340
static const uint16 qfg4GuildWalkSignature2[] = {
	0x38, SIG_SELECTOR16(init),          // pushi init
	0x76,                                // push0
	0x38, SIG_SELECTOR16(approachVerbs), // pushi approachVerbs
	0x78,                                // push1
	0x39, 0x04,                          // pushi 4d
	0x72, SIG_ADDTOOFFSET(+2),           // lofsa steps1
	0x4a, SIG_UINT16(0x000a),            // send 10d

	SIG_ADDTOOFFSET(+10),               // ... (similar inits follow)
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa steps2
	SIG_ADDTOOFFSET(+13),               // ...
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa barrels1
	SIG_ADDTOOFFSET(+13),               // ...
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa barrels2
	SIG_ADDTOOFFSET(+13),               // ...
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa crack1
	SIG_ADDTOOFFSET(+13),               // ...
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa crack2
	SIG_ADDTOOFFSET(+13),               // ...
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa pillar
	SIG_ADDTOOFFSET(+3),                // ...

	SIG_ADDTOOFFSET(+26),               // ... (global[78]::add() steps1 and steps2)

	SIG_ADDTOOFFSET(+459),              // ... (cond block for polygons)
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x00f7),           // jmp 247d [end the cond] (2nd condition done)

                                        // (else condition)
	0x38, SIG_SELECTOR16(init),         // pushi init
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa secritExit
	0x4a, SIG_UINT16(0x0004),           // send 4d
	SIG_ADDTOOFFSET(+4),                // ... (addObstacle and its arg count)
	SIG_ADDTOOFFSET(+228),              // ... (3rd Polygon type, init, and push)
	SIG_ADDTOOFFSET(+5),                // ... (end of the polygons cond)

	0x38, SIG_SELECTOR16(init),         // pushi init (super init:)
	SIG_END
};

static const uint16 qfg4GuildWalkPatch2[] = {
	0x3f, 0x02,                         // link 02 (set up loop vars, op affects the stack)
	0x74, PATCH_GETORIGINALUINT16(11),  // lofss steps1
	0x74, PATCH_GETORIGINALUINT16(27),  // lofss steps2
	0x74, PATCH_GETORIGINALUINT16(43),  // lofss barrels1
	0x74, PATCH_GETORIGINALUINT16(59),  // lofss barrels2
	0x74, PATCH_GETORIGINALUINT16(75),  // lofss crack1
	0x74, PATCH_GETORIGINALUINT16(91),  // lofss crack2
	0x74, PATCH_GETORIGINALUINT16(107), // lofss pillar
                                        //
	0x35, 0x08,                         // ldi 8d (decrement and send 7 times, while != 0)
	0xa5, 0x00,                         // sat temp[0]
                                        //
	0xe5, 0x00,                         // -at temp[0]
	0x31, 0x13,                         // bnt 19d [on 0, end the loop]
	0xad, 0x01,                            // sst temp[1] (pop the next object into a temp var)
	0x38, PATCH_SELECTOR16(init),          // pushi init
	0x76,                                  // push0
	0x38, PATCH_SELECTOR16(approachVerbs), // pushi approachVerbs
	0x78,                                  // push1
	0x39, 0x04,                            // pushi 4d
	0x85, 0x01,                            // lat temp[1] (accumulate the object)
	0x4a, PATCH_UINT16(0x000a),            // send 10d
	0x33, 0xe9,                         // jmp -23d [loop]

	0x33, 0x33,                         // jmp 51d [skip subroutine declaration]
	0x38, PATCH_SELECTOR16(obstacles),  // pushi obstacles (look up "obstacles", might be null)
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2]
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0x31, 0x11,                         // bnt 17d [skip disposal and nulling]
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x76,                               // push0
	0x4a, PATCH_UINT16(0x0004),         // send 4d ((global[2] obstacles?) dispose:)
                                        //
	0x38, PATCH_SELECTOR16(obstacles),  // pushi obstacles (null the "obstacles" property)
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2]
	0x4a, PATCH_UINT16(0x0006),         // send 6d
                                        //
	0x38, PATCH_SELECTOR16(addObstacle), // pushi addObstacle
	0x78,                               // push1
	0x32, PATCH_UINT16(0x020f),         // jmp 527d [3rd polygon type, init, and push]
                                        // (That will jmp back here)
	0x81, 0x02,                         // lag global[2]
	0x4a, PATCH_UINT16(0x0006),         // send 6d
                                        //
	0x38, PATCH_SELECTOR16(init),       // pushi init
	0x76,                               // push0
	0x72, PATCH_GETORIGINALUINT16(605), // lofsa secritExit
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0x48,                               // ret

	0x33, 0x07,                         // jmp 7d [skip waste bytes, to (global[78] add: steps1)]
	0x5c,                               // selfID (waste 1 byte)
	PATCH_ADDTOOFFSET(+494),            // ...
	0x76,                               // push0 (0 call args, clobber the old secritExit init)
	0x40, PATCH_UINT16(0xfdd6), PATCH_UINT16(0x0000), // call 0d [-554] (the subroutine does everything)
	0x32, PATCH_UINT16(0x00ee),         // jmp 238d [end the cond]
	0x5c,                               // selfID (waste 1 byte)
	PATCH_ADDTOOFFSET(+4),              // ...
	PATCH_ADDTOOFFSET(+228),            // ... (3rd polygon type, init, and push)
	0x32, PATCH_UINT16(0xfd0a),         // jmp -758d [back into the subroutine]
	0x35, 0x00,                         // ldi 0 (erase 2 bytes to keep disasm aligned)
	PATCH_END
};

// Applies to at least: English CD
// Responsible method: sBarrelMove::changeState(2) in script 340
static const uint16 qfg4GuildWalkCDSignature3[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0032),           // bnt 50d [next case]
	0x35, 0x02,                         // ldi 2d (case 2 label)
	SIG_ADDTOOFFSET(+26),               // ... (register branch and derelict say())
	SIG_ADDTOOFFSET(+19),               // ... (else, the rest of case 2 is a necessary say())
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_END
};

static const uint16 qfg4GuildWalkCDPatch3[] = {
	0x31, 0x15,                         // bnt 21d [next case]
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x39, 0x05,                         // pushi 5d
	0x39, 0x06,                         // pushi 6d
	0x39, 0x04,                         // pushi 4d
	0x39, 0x13,                         // pushi 19d
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[91]
	0x4a, PATCH_UINT16(0x000e),         // send 14d
	0x32, PATCH_GETORIGINALUINT16ADJUST(51, +30), // jmp ?? [end the switch]

	0x3c,                               // dup (case 8 prologue)
	0x35, 0x08,                         // ldi 8d
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt 6d [next case]
	0x76,                               // push0 (0 call args)
	0x40, PATCH_UINT16(0xf592), PATCH_UINT16(0x0000), // call [-2670], 0d (patch 2's subroutine)
	0x33, 0x10,                         // jmp 16d [skip waste bytes]
	PATCH_END                           // (don't end the switch, keep testing cases)
};

// Applies to at least: English floppy, German floppy
// Responsible method: sBarrelMove::changeState(2) in script 340
static const uint16 qfg4GuildWalkFloppySignature3[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0032),           // bnt 50d [next case]
	0x35, 0x02,                         // ldi 2d (case 2 label)
	SIG_ADDTOOFFSET(+26),               // ... (register branch and derelict say())
	SIG_ADDTOOFFSET(+19),               // ... (else, the rest of case 2 is a necessary say())
	0x32, SIG_ADDTOOFFSET(+2),          // jmp ?? [end the switch]
	SIG_END
};

static const uint16 qfg4GuildWalkFloppyPatch3[] = {
	0x31, 0x15,                         // bnt 21d [next case]
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x39, 0x05,                         // pushi 5d
	0x39, 0x06,                         // pushi 6d
	0x39, 0x04,                         // pushi 4d
	0x39, 0x13,                         // pushi 19d
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[91]
	0x4a, PATCH_UINT16(0x000e),         // send 14d
	0x32, PATCH_GETORIGINALUINT16ADJUST(51, +30), // jmp ?? [end the switch]

	0x3c,                               // dup (case 8 prologue)
	0x35, 0x08,                         // ldi 8d
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt 6d [next case]
	0x76,                               // push0 (0 call args)
	0x40, PATCH_UINT16(0xf5a8), PATCH_UINT16(0x0000), // call [-2648], 0d (patch 2's subroutine)
	0x33, 0x10,                         // jmp 16d [skip waste bytes]
	PATCH_END                           // (don't end the switch, keep testing cases)
};

// Rations are not properly decremented by daily scheduled meal consumption.
// Rations are consumed periodically as time advances. If rations are the
// active inventory item when the last of them is eaten, that icon will persist
// in the verb bar.
//
// We make room by consolidating common gloryMessager::say() args in a
// subroutine and cache values with temp variables. We add code to clean up
// the verb bar when rations are exhausted (test if rations were the active
// item, advance the cursor, hide the bar's invItem icon) - similar to the
// localproc in script 16, called by combinable items to remove themselves.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: hero::eatMeal() in script 28
// Fixes bug: #10772
static const uint16 qfg4LeftoversSignature[] = {
	0x3f, 0x01,                         // link 1d
	SIG_ADDTOOFFSET(+9),                // ...
	SIG_MAGICDWORD,
	0xe1, 0x88,                         // -ag global[136] (digest a preemptively eaten meal)
	0x35, 0x01,                         // ldi 1d
	SIG_ADDTOOFFSET(+238),              // ...
	0x85, 0x00,                         // lat temp[0] (eaten, unset flags if true)
	SIG_END
};

static const uint16 qfg4LeftoversPatch[] = {
	0x3f, 0x03,                         // link 3d (3 temp vars)

	PATCH_ADDTOOFFSET(+15),             // (cond 1, preemptively eaten meals)
	0x32, PATCH_UINT16(0x00bb),         // jmp 187d [end the cond]

                                        // (cond 2)
	0x39, PATCH_SELECTOR8(at),          // pushi at
	0x78,                               // push1
	0x39, 0x04,                         // pushi 4d (itemId 4, theRations)
	0x81, 0x09,                         // lag global[9] (gloryInv)
	0x4a, PATCH_UINT16(0x0006),         // send 6d
	0xa5, 0x01,                         // sat temp[1] (theRations)

	0x38, PATCH_SELECTOR16(amount),     // pushi amount
	0x76,                               // push0
	0x4a, PATCH_UINT16(0x0004),         // send 4d (theRations amount:)
	0xa5, 0x02,                         // sat temp[2] (amount)

	0x31, 0x50,                         // bnt 80d [next condition]

	0x38, PATCH_SELECTOR16(amount),     // pushi amount
	0x78,                               // push1
	0xed, 0x02,                         // -st temp[2] (amount)
	0x85, 0x01,                         // lat temp[1] (theRations)
	0x4a, PATCH_UINT16(0x0006),         // send 6d (decrement amount)

	0x85, 0x02,                         // lat temp[2] (amount)
	0x2f, 0x3b,                         // bt 59d [skip exhausted item removal]

	0x38, PATCH_SELECTOR16(owner),      // pushi owner
	0x78,                               // push1
	0x76,                               // push0
	0x85, 0x01,                         // lat temp[1] (theRations)
	0x4a, PATCH_UINT16(0x0006),         // send 6d

	0x38, PATCH_SELECTOR16(curInvIcon), // pushi curInvIcon
	0x76,                               // push0
	0x81, 0x45,                         // lag global[69] (mainIconBar)
	0x4a, PATCH_UINT16(0x0004),         // send 4d
	0x8d, 0x01,                         // lst temp[1] (theRations)
	0x1a,                               // eq?
	0x31, 0x23,                         // bnt 35d [skip icon bar disabling]

	0x38, PATCH_SELECTOR16(curInvIcon), // pushi curInvIcon
	0x78,                               // push1
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(advanceCurIcon), // pushi advanceCurIcon
	0x76,                               // push0
	0x38, PATCH_SELECTOR16(disable),    // pushi disable
	0x78,                               // push1
	0x39, 0x06,                         // pushi 6d
	0x81, 0x45,                         // lag global[69] (mainIconBar)
	0x4a, PATCH_UINT16(0x0010),         // send 16d

	0x38, PATCH_SELECTOR16(hide),       // pushi hide
	0x76,                               // push0
                                        //
	0x7a,                               // push2 (2 call args)
	0x39, 0x24,                         // pushi 36d
	0x78,                               // push1
	0x43, 0x02, PATCH_UINT16(0x0004),   // callk ScriptID, 4d (ScriptID 36 1, invItem)
                                        //
	0x4a, PATCH_UINT16(0x0004),         // send 4d (invItem hide:)
                                        // (exhausted item removal end)

	0x35, 0x01,                         // ldi 1d
	0xa5, 0x00,                         // sat temp[0] (eaten)

	0x33, 0x54,                         // jmp 84d [end the cond]

                                        // (cond 3)
	0x78,                               // push1 (1 call arg)
	0x39, 0x03,                         // pushi 3d ("hungry" flag)
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb [export 4 of script 0], 2d (test flag 3)
	0x31, 0x26,                         // bnt 38d [next condition]
                                        //
	0x38, PATCH_SELECTOR16(useStamina), // pushi useStamina
	0x7a,                               // push2
	0x39, 0x08,                         // pushi 8d
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0008),         // self 8d (hero useStamina: 8 0)
                                        //
	0x31, 0x09,                         // bnt 9d [hero dies]
	0x78,                               // push1 (1 call arg)
	0x39, 0x05,                         // pushi 5d (say cond:5, "You're starving.")
	0x41, 0x3a, PATCH_UINT16(0x0002),   // call [58], 2d (gloryMessager say: 1 6 5 1 0 28)
	0x33, 0x36,                         // jmp 54d [end the cond]
                                        //
	0x39, 0x04,                         // pushi 4d (4 call args)
	0x39, 0x08,                         // pushi 8d
	0x39, 0x1c,                         // pushi 28d
	0x38, PATCH_UINT16(0x03e3),         // pushi 995d
	0x78,                               // push1
	0x47, 0x1a, 0x00, PATCH_UINT16(0x0008), // calle [export 0 of script 26], 8d (hero dies)
	0x33, 0x25,                         // jmp 37d [end the cond]

                                        // (cond 4)
	0x78,                               // push1 (1 call arg)
	0x7a,                               // push2 ("missed meal" flag)
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb [export 4 of script 0], 2d (test flag 2)
	0x31, 0x10,                         // bnt 16d [next condition]
                                        //
	0x78,                               // push1 (1 call arg)
	0x39, 0x03,                         // pushi 3d ("hungry" flag)
	0x45, 0x02, PATCH_UINT16(0x0002),   // callb [export 2 of script 0], 2d (set flag 3)
                                        //
	0x78,                               // push1 (1 call arg)
	0x39, 0x06,                         // pushi 6d (say cond:6, "Really getting hungry.")
	0x41, 0x11, PATCH_UINT16(0x0002),   // call [17], 2d (gloryMessager say: 1 6 6 1 0 28)
	0x33, 0x0d,                         // jmp 13d [end the cond]

                                        // (cond else)
	0x78,                               // push1 (1 call arg)
	0x7a,                               // push2 ("missed meal" flag)
	0x45, 0x02, PATCH_UINT16(0x0002),   // callb [export 2 of script 0], 2d (set flag 2)
                                        //
	0x78,                               // push1 (1 call arg)
	0x39, 0x04,                         // pushi 4d (say cond:4, "Get food soon.")
	0x41, 0x02, PATCH_UINT16(0x0002),   // call [2], 2d (gloryMessager say: 1 6 4 1 0 28)

                                        // (cond end)

	0x33, 0x14,                         // jmp 20d [skip subroutine declaration]
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x39, 0x06,                         // pushi 6d
	0x78,                               // push1 (noun)
	0x39, 0x06,                         // pushi 6d (verb)
	0x8f, 0x01,                         // lsp param[1] (cond varies)
	0x78,                               // push1 (seq)
	0x76,                               // push0 (caller)
	0x39, 0x1c,                         // pushi 28d (message pool)
	0x81, 0x5b,                         // lag global[91] (gloryMessager say: 1 6 ? 1 0 28)
	0x4a, PATCH_UINT16(0x0010),         // send 16d
	0x48,                               // ret

	0x33, 0x16,                         // jmp 22d [skip waste bytes]
	0x35, 0x00,                         // ldi 0 (erase 2 bytes to keep disasm aligned)
	PATCH_END
};

// The runes puzzle in room 800 often rejects the correct answer. When a letter
//  is selected it turns red but it's not applied until after a 30 tick delay
//  with no visual indicator. If a letter is clicked during that delay then the
//  previous letter is silently skipped, which is common since the correct
//  answer contains the same letter consecutively.
//
// There are two approaches to fixing this: remove the delay or disable input
//  during it. We do both. Letters are now applied as soon as they turn red, but
//  the delay prevented the puzzle from ending abruptly, and so we still pause
//  after the puzzle is solved but disable input. This preserves the puzzle's
//  external behavior while making it impossible to click too fast.
//
// This bug is in all versions but the CD version exacerbates it with its broken
//  dial. Sierra upgraded the Cycle classes which changed the behavior the dial
//  depends on. CycleTo no longer supports wrapping around cel ranges and so the
//  dial can't move from "C" to "O" (cels 7 to 0) and doesn't spin around when
//  selecting the same letter twice as it did in floppy.
//
// Applies to: All versions
// Responsible methods: proc_58 in script 801, runePuz:handleEvent, sTurnTheDial:changeState
// Fixes bug: #10965
static const uint16 qfg4RunesPuzzleSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0163),           // pushi 0163
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 [ set flag 355, puzzle is solved ]
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x72, SIG_UINT16(0x0010),           // lofsa runesPuz
	0x4a, SIG_UINT16(0x0004),           // send 04 [ runesPuz dispose: ]
	SIG_END
};

static const uint16 qfg4RunesPuzzlePatch1[] = {
	PATCH_ADDTOOFFSET(+7),
	0x32, PATCH_UINT16(0x0007),         // jmp 0007 [ don't exit puzzle immediately ]
	PATCH_END
};

static const uint16 qfg4RunesPuzzleSignature2[] = {
	// runePuz:handleEvent
	0x63, SIG_ADDTOOFFSET(+1),          // pToa register [ always zero, the following code is unused ]
	SIG_MAGICDWORD,
	0x31, 0x21,                         // bnt 21 [ handle mouse/key down events ]
	0x39, 0x04,                         // pushi 04
	0x39, SIG_ADDTOOFFSET(+1),          // pushi type
	0x76,                               // push0
	0x87, 0x01,                         // lap 01
	0x4a, SIG_UINT16(0x0004),           // send 04 [ event type? ]
	SIG_ADDTOOFFSET(+495),
	0x39, SIG_SELECTOR8(claimed),       // pushi claimed
	SIG_ADDTOOFFSET(+852),
	// sTurnTheDial:changeState
	0x30, SIG_UINT16(0x0112),           // bnt 0112 [ state 2 ]
	SIG_ADDTOOFFSET(+268),
	0x35, 0x1e,                         // ldi 1e
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks
	0x33, 0x20,                         // jmp 20 [ end of method ]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x1a,                         // bnt 1a [ end of method ]
	0x76,                               // push0
	0x40, SIG_ADDTOOFFSET(+2),          // call proc_58 [ apply letter to puzzle ]
	      SIG_UINT16(0x0000),
	0x38, SIG_SELECTOR16(canControl),   // pushi canControl
	0x78,                               // push1
	0x78,                               // push1
	0x51, SIG_ADDTOOFFSET(+1),          // class User
	0x4a, SIG_UINT16(0x0006),           // send 06 [ User canControl: 1 (unnecessary) ]
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa sTurnTheDial
	0x4a, SIG_UINT16(0x0004),           // send 04 [ sTurnTheDial dispose: (unnecessary) ]
	SIG_END
};

static const uint16 qfg4RunesPuzzlePatch2[] = {
	// runePuz:handleEvent
	0x78,                               // push1
	0x38, PATCH_UINT16(0x0163),         // pushi 0163
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_3 [ is puzzle solved? ]
	0x31, 0x1b,                         // bnt 1b   [ handle mouse/key down events ]
	0x32, PATCH_UINT16(0x01f0),         // jmp 01f0 [ ignore events if puzzle is solved ]
	PATCH_ADDTOOFFSET(+1350),
	// sTurnTheDial:changeState
	0x30, PATCH_UINT16(0x0123),         // bnt 0123 [ state 2 ]
	PATCH_ADDTOOFFSET(+268),
	0x76,                               // push0
	0x40, PATCH_GETORIGINALUINT16ADJUST(+1648, +12), // call proc_58 [ apply letter to puzzle ]
	      PATCH_UINT16(0x0000),
	0x39, 0x01,                         // push1
	0x38, PATCH_UINT16(0x0163),         // pushi 0163
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_3 [ is puzzle solved? ]
	0x31, 0x10,                         // bnt 10 [ end of method ]
	0x35, 0x1e,                         // ldi 1e
	0x65, PATCH_GETORIGINALBYTE(+1637), // aTop ticks [ pause 30 ticks before exiting puzzle ]
	0x33, 0x0a,                         // jmp 0a [ end of method ]
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0010),         // lofsa runesPuz
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ runesPuz dispose: ]
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_END
};

// The Domovoi in room 320 has a complex bug in the CD version. If you don't
//  talk to him before Bella wakes you up then you can't get the doll and the
//  game can't be completed. The event logic was changed in the floppy patch and
//  again in the CD version, which introduced the bug. Working backwards...
//
// To get the doll from the inn's cabinet:
// - It must be late at night
// - Inn event 15 has occurred or is occurring
//
// To trigger inn event 15 ("You see that the Domovoi is here again"):
// - It must be late at night
// - You saved the monastery's Domovoi
// - You clicked Talk on the Domovoi during inn event 3 (new requirement in CD)
// - You didn't just enter from your room and hear crying
//
// To trigger inn event 3 ("You have the feeling you are being watched"):
// - It must be late at night
// - You haven't already clicked Talk on the Domovoi during inn event 3
// - You haven't been woken by Bella (new requirement in CD)
// - You didn't just enter from your room and hear crying
//
// The two new requirements create an unwinnable state. Once Bella wakes you,
//  event 3 is no longer possible, cascading to event 15 and the doll. This also
//  prevents the Domovoi from appearing in your room as that requires talking
//  to him during event 3. Putting it all together, the new Bella requirement's
//  only effect is to suppress a mandatory event, and so it is safe to remove.
//
// Applies to: English CD
// Responsible methods: rm320:init, heroTeller:respond
// Fixes bug: #10978
static const uint16 qfg4DomovoiInnSignature[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x0088),           // pushi 0088
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 [ is flag 136 set? (has Bella woken you up?) ]
	0x18,                               // not
	0x31,                               // bnt [ skip inn event 3 ]
	SIG_END
};

static const uint16 qfg4DomovoiInnPatch[] = {
	0x32, PATCH_UINT16(0x0008),         // jmp 0008 [ skip flag 136 check ]
	PATCH_END
};

// During the final battle with Ad Avis his initial timer is never stopped,
//  reducing the intended time the player has to complete the sequence by more
//  than half, and bringing Ad Avis back to life after he's killed.
//
// sTimeItOut state 0 sets a timeout when the battle starts. Its length depends
//  on the detected cpu speed and game version. In the floppy versions this was
//  a minimum of 400 seconds, which is so long that it masked the bug, but in CD
//  it was reduced to 20 seconds. This is supposed to be how long the player has
//  to tell the joke, after which sUltimakeJoke sets a second timeout in which
//  the character-specific actions are to be done, but sTimeItOut finishes first
//  and forces the player to complete both phases during the first shorter one.
//  When Ad Avis is killed his death scripts only stop sUltimakeJoke, as they
//  don't expect sTimeItOut to be running, and so when sTimeItOut times out it
//  kills the player unless the death script has already called avis:dispose.
//
// We fix this by patching sTimeItOut state 1 to abort the script if the joke
//  has been told. This is equivalent to the NRS patch that ships with the GOG
//  version, which disposes sTimeItOut when telling the joke, and so this patch
//  is applied to all versions except that one.
//
// Applies to: All versions
// Responsible method: sTimeItOut:changeState(1)
// Fixes bug: #10844
static const uint16 qfg4AdAvisTimeoutSignature[] = {
	0x30, SIG_UINT16(0x002c),           // bnt 002c [ state 1 ]
	SIG_ADDTOOFFSET(+0x29),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x00ae),           // jmp 00ae [ end of method ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0027),           // bnt 0027 [ state 2 ]
	SIG_END
};

static const uint16 qfg4AdAvisTimeoutPatch[] = {
	0x30, PATCH_UINT16(0x0029),         // bnt 0029 [ state 1 ]
	PATCH_ADDTOOFFSET(+0x29),
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x2b,                         // bnt 2b [ state 2 ]
	0x83, 0x04,                         // lal 04 [ has joke been told? ]
	0x2f, 0x27,                         // bt 27  [ abort script if joke has been told ]
	PATCH_END
};

// During the final battle with Ad Avis if the player casts a non-fatal spell at
//  him then they can't cast any more spells. Instead they receive a message
//  about being too busy or it not being a good place and must wait to die.
//  Another symptom of this bug is that fighters and thieves don't get to see
//  the staff transform if they've previously thrown a weapon.
//
// SpellItem:doVerb(4) determines if a spell is allowed. If hero:view is the
//  wrong value then it says "This isn't a good place..." and if the room has
//  a script it says "You're too busy...". avis:getHurt breaks one or both of
//  these conditions by setting the room script to sMessages. If the game speed
//  is set to less than high then avis:getHurt runs while the "project" room
//  script is animating hero. Setting the room script to sMessages interrupts
//  this and hero is left on view 14, breaking the first spell condition. Even
//  if the game speed is set to high and hero's animation completes, sMessages
//  fails to dispose itself, leaving it as the room script when it's complete
//  and breaking the second spell condition.
//
// We fix this by reassigning sMessages from the room's script to midBlast, an
//  arbitrary Prop that no scripts depend on. project is no longer interrupted,
//  hero's animation completes at all speeds, and it no longer matters that
//  sMessage fails to dispose itself. Due to script changes, this patch is only
//  applied once to floppy and twice to CD.
//
// We also include a version of this for the offsets in the NRS patch, which is
//  important as that ships with the GOG version.
//
// Applies to: All versions
// Responsible method: avis:getHurt
// Fixes bug: #10835
static const uint16 qfg4AdAvisSpellsFloppySignature[] = {
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x0096),           // lofsa sMessages
	0x36,                               // push
	0x81, 0x02,                         // lag 02
	0x4a, SIG_UINT16(0x0006),           // send 06 [ rm730 setScript: sMessages ]
	SIG_END
};

static const uint16 qfg4AdAvisSpellsFloppyPatch[] = {
	0x74, PATCH_ADDTOOFFSET(+2),        // lofss sMessages
	0x72, PATCH_UINT16(0x0668),         // lofsa midBlast
	SIG_END
};

static const uint16 qfg4AdAvisSpellsCDSignature[] = {
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x00a6),           // lofsa sMessages
	0x36,                               // push
	0x81, 0x02,                         // lag 02
	0x4a, SIG_UINT16(0x0006),           // send 06 [ rm730 setScript: sMessages ]
	SIG_END
};

static const uint16 qfg4AdAvisSpellsCDPatch[] = {
	0x74, PATCH_ADDTOOFFSET(+2),        // lofss sMessages
	0x72, PATCH_UINT16(0x06b6),         // lofsa midBlast
	SIG_END
};

static const uint16 qfg4AdAvisSpellsNrsSignature[] = {
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x00a8),           // lofsa sMessages
	0x36,                               // push
	0x81, 0x02,                         // lag 02
	0x4a, SIG_UINT16(0x0006),           // send 06 [ rm730 setScript: sMessages ]
	SIG_END
};

static const uint16 qfg4AdAvisSpellsNrsPatch[] = {
	0x74, PATCH_ADDTOOFFSET(+2),        // lofss sMessages
	0x72, PATCH_UINT16(0x06b8),         // lofsa midBlast
	SIG_END
};

// If the magic user defeats Ad Avis with the game speed set to less than high
//  then they aren't allowed to cast the final summon staff spell and complete
//  the game. Instead they receive "You're too busy to cast a spell right now."
//
// Spells can't be cast if a room script is set, as described in the above patch
//  notes. When Ad Avis is killed, avis:getHurt sets hero's view and loop before
//  running sAdavisDies. If the game speed isn't set to high then the "project"
//  script that deployed the final projectile spell is still running and waiting
//  on hero's animation to complete. By changing the view and loop, avis:getHurt
//  prevents project from advancing to its next state and completing, leaving it
//  stuck as the room script and blocking the final spell.
//
// We can't prevent project from being the room script as that's game-wide
//  behavior, and we can't prevent it from being interrupted since avis:getHurt
//  needs to set hero's final view/loop, but we can still fix the bug by setting
//  sAdavisDies as the room's script instead of hero's. This disposes project if
//  it's still running and guarantees that the room script is cleared since
//  sAdavisDies always disposes of itself.
//
// Applies to: All versions
// Responsible method: avis:getHurt
// Fixes bug: #10835
static const uint16 qfg4AdAdvisLastSpellSignature[] = {
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa sAdavisDies
	SIG_MAGICDWORD,
	0x36,                               // push
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0006),           // send 06 [ hero setScript: sAdavisDies ]
	SIG_END
};

static const uint16 qfg4AdAdvisLastSpellPatch[] = {
	PATCH_ADDTOOFFSET(+8),
	0x81, 0x02,                         // lag 02 [ rm730 ]
	SIG_END
};

// When throwing a weapon or casting a spell at Ad Avis in room 730, sMessages
//  tests the projectile type incorrectly and transposes the message responses.
//
// Applies to: All versions
// Responsible method: sMessages:changeState(2)
// Fixes bug: #10989
static const uint16 qfg4AdAvisMessageSignature[] = {
	0x8b, SIG_MAGICDWORD, 0x01,         // lsl 01 [ 0 if weapon thrown, else a spell ]
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	SIG_END
};

static const uint16 qfg4AdAvisMessagePatch[] = {
	PATCH_ADDTOOFFSET(+4),
	0x1c,                               // ne?
	PATCH_END
};

// Throwing a rock or dagger at Ad Avis after telling the joke kills him.
//  avis:getHurt fails to test the projectile type correctly in CD, or at all in
//  floppy, and so all versions mistake this for casting a spell with the staff.
//
// We fix this by testing the projectile type and not allowing a thrown weapon
//  to kill Ad Avis. This replaces an unnecessary hero:script test.
//
// Applies to: All versions
// Responsible method: avis:getHurt
// Fixes bug: #10989
static const uint16 qfg4AdAvisThrowWeaponSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(script),       // pushi script
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0004),           // send 04 [ hero script? ]
	0x18,                               // not
	0x30, SIG_ADDTOOFFSET(+2),          // bnt [ projectile doesn't kill ad avis ]
	0x39, SIG_SELECTOR8(view),          // pushi view
	SIG_END
};

static const uint16 qfg4AdAvisThrowWeaponPatch[] = {
	0x83, 0x01,                        // lal 01 [ 0 if weapon thrown, else a spell ]
	0x33, 0x06,                        // jmp 06 [ throwing a weapon doesn't kill ad avis ]
	PATCH_END
};

// When a fighter or paladin selects the staff in the final battle with Ad Avis
//  after throwing a rock or dagger they enter an infinite animation loop due to
//  not clearing hero:cycler. Multiple bugs in this room prevented getting this
//  far, but we fixed those, so we also fix this by clearing the cycler.
//
// Applies to: All versions
// Responsible method: sDoTheStaff:changeState(3)
// Fixes bug: #10835
static const uint16 qfg4FighterSpearSignature[] = {
	0x39, SIG_SELECTOR8(view),          // pushi view [ start of fighter code, same as paladin ]
	SIG_ADDTOOFFSET(0x3e),
	0x3c,                               // dup
	0x35, SIG_MAGICDWORD, 0x03,         // ldi 03
	0x1a,                               // eq? [ is paladin? (last condition so always true) ]
	0x30, SIG_UINT16(0x0022),           // bnt 0022
	0x39, SIG_SELECTOR8(view),          // pushi view
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0a
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x7a,                               // push2
	0x76,                               // push0
	0x78,                               // push1
	0x38, SIG_SELECTOR16(setCel),       // pushi setCel
	SIG_ADDTOOFFSET(+13),
	0x4a, SIG_UINT16(0x001c),           // send 1c [ hero view: 10 setLoop 0 1 setCel: 0 ... ]
	SIG_END
};

static const uint16 qfg4FighterSpearPatch[] = {
	0x33, 0x3e,                         // jmp 3e [ use patched paladin code for fighter ]
	PATCH_ADDTOOFFSET(0x3e),
	0x39, PATCH_SELECTOR8(view),        // pushi view
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0a
	0x38, PATCH_SELECTOR16(setLoop),    // pushi setLoop
	0x7a,                               // push2
	0x76,                               // push0
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(setCel),     // pushi setCel
	0x39, 0x01,                         // pushi 01
	0x39, 0x00,                         // pushi 00
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	PATCH_ADDTOOFFSET(+13),
	0x4a, PATCH_UINT16(0x0022),         // send 22 [ hero view: 10 setLoop 0 1 setCel: 0 setCycle: 0 ... ]
	PATCH_END
};

// Clicking Do on the inn door in room 260 from certain coordinates crashes the
//  CD version. This is one of several related crashes where the Grooper or
//  Grycler classes send a selector to a non-object in only the CD version.
//
// The inn door script isn't buggy, and neither are Grooper or Grycler. Instead,
//  Sierra "upgraded" the core Cycle classes in the CD version with drastically
//  different behavior after the game was already written for the first ones.
//  It's unclear what they were attempting to accomplish, but the conspicuous
//  regressions include hero stuttering when walking on every screen, the runes
//  dial refusing to spin a full rotation, random crashes at the inn door and
//  on the slippery path in room 800, and probably other problems. Meanwhile
//  GK1, a relatively stable SCI32 game released at the same time, used the same
//  Cycle classes in all its versions as QFG4 floppy without motion problems.
//
// The crashes result from complex motion edge cases but involve hero ending up
//  without a cycler at the wrong moment. These can be avoided by adding a call
//  to hero:normalize to reset a lot of state and set hero:cycler to StopWalk
//  and hero:looper to stopGroop. This is a bit of a kitchen-sink solution but
//  it does the job without side effects and only requires 4 bytes.
//
// We prevent the inn door crash by calling hero:normalize in sInInnDoor.
//
// Applies to: English CD
// Responsible method: sInInnDoor:changeState(1)
// Fixes bug: #10760
static const uint16 qfg4InnDoorCDSignature[] = {
	0x30, SIG_MAGICDWORD,               // bnt 000e [ state 2 ]
	      SIG_UINT16(0x000e),
	0x38, SIG_UINT16(0x0111),           // pushi setHeading [ hard-coded for CD ]
	0x7a,                               // push2
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0008),           // send 08  [ hero setHeading: 0 self ]
	0x32, SIG_UINT16(0x00c3),           // jmp 00c3 [ end of method ]
	SIG_END
};

static const uint16 qfg4InnDoorCDPatch[] = {
	0x31, 0x0f,                         // bnt 0f [ state 2 ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	0x38, PATCH_UINT16(0x0111),         // pushi setHeading [ hard-coded for CD ]
	0x7a,                               // push2
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x000c),         // send 0c [ hero normalize: setHeading: 0 self ]
	PATCH_END
};

// In room 800, at the start of the game, automatically sliding down the top of
//  the slope can crash the CD version in Grooper:doit. See the inn door patch
//  above for details on these motion regressions and their solution.
//
// We fix this by calling hero:normalize at the start of sFallsBackSide to reset
//  hero's state before starting the motion sequence.
//
// This patch is not applied to the NRS fan-patch included in the GOG version.
//  It fixes this bug by adding a spin loop delay that's relative to game speed
//  to sFallsBackSide.
//
// Applies to: English CD
// Responsible method: sFallsBackSide:changeState(0)
// Fixes bug: #9801
static const uint16 qfg4SlidingDownSlopeCDSignature[] = {
	0x87, 0x01,                       // lap 01
	0x65, 0x16,                       // aTop state
	0x36,                             // push
	0x3c,                             // dup
	0x35, SIG_MAGICDWORD, 0x00,       // ldi 00
	0x1a,                             // eq?
	0x31, 0x30,                       // bnt 30 [ state 1 ]
	SIG_ADDTOOFFSET(+42),
	0x4a, SIG_UINT16(0x0014),         // send 14 [ hero: setStep: 1 1 ... ]
	SIG_END
};

static const uint16 qfg4SlidingDownSlopeCDPatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x2f, 0x34,                         // bt 34 [ state 1 ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	PATCH_ADDTOOFFSET(+42),
	0x4a, PATCH_UINT16(0x0018),         // send 18 [ hero: normalize: setStep: 1 1 ... ]
	PATCH_END
};

// Walking around the base of the slippery slope in room 800 can crash the CD
//  version in either the Grooper or Grycler classes. See the inn door patch
//  above for details on these regressions and their solution.
//
// The script sSlippery runs when walking up the slope and sWalksDown runs when
//  walking down. Both are vulnerable to Grooper/Grycler crashes and both can be
//  fixed by adding hero:normalize calls.
//
// We also include a version of the sWalksDown patch for the instruction sizes
//  in the NRS patch, which is important as that ships with the GOG version.
//
// Applies to: English CD
// Responsible methods: sSlippery:changeState(0), sWalksDown:changeState(0)
// Fixes bug: #10747
static const uint16 qfg4WalkUpSlopeCDSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0142),           // pushi setMotion [ hard-coded for CD ]
	0x78,                               // push1
	0x76,                               // push0
	SIG_ADDTOOFFSET(+8),
	0x4a, SIG_UINT16(0x000e),           // send 0e [ hero setMotion: 0 ... ]
	SIG_END
};

static const uint16 qfg4WalkUpSlopeCDPatch[] = {
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x39, 0x00,                         // pushi 00
	PATCH_ADDTOOFFSET(+8),
	0x4a, PATCH_UINT16(0x000c),         // send 0c [ hero normalize: ... ]
	PATCH_END
};

static const uint16 qfg4WalkDownSlopeCDSignature[] = {
	0x3c,                               // dup
	0x35, SIG_MAGICDWORD, 0x00,         // ldi 00
	0x1a,                               // eq?
	0x31, 0x1e,                         // bnt 1e [ state 1 ]
	0x38, SIG_UINT16(0x0218),           // pushi handsOff [ hard-coded for CD ]
	SIG_ADDTOOFFSET(+15),
	0x4a, SIG_UINT16(0x0008),           // send 08 [ hero setStep: ... ]
	SIG_END
};

static const uint16 qfg4WalkDownSlopeCDPatch[] = {
	0x2f, 0x22,                         // bt 22 [ state 1 ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	PATCH_ADDTOOFFSET(+18),
	0x4a, PATCH_UINT16(0x000c),         // send 0c [ hero normalize: setStep: ... ]
	PATCH_END
};

static const uint16 qfg4WalkDownSlopeNrsSignature[] = {
	0x3c,                               // dup
	0x35, SIG_MAGICDWORD, 0x00,         // ldi 00
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x001f),           // bnt 001f [ state 1 ]
	0x38, SIG_UINT16(0x0218),           // pushi handsOff [ hard-coded for CD ]
	SIG_ADDTOOFFSET(+15),
	0x4a, SIG_UINT16(0x0008),           // send 08 [ hero setStep: ... ]
	SIG_END
};

static const uint16 qfg4WalkDownSlopeNrsPatch[] = {
	0x2e, PATCH_UINT16(0x0023),         // bt 0023 [ state 1 ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	PATCH_ADDTOOFFSET(+18),
	0x4a, PATCH_UINT16(0x000c),         // send 0c [ hero normalize: setStep: ... ]
	PATCH_END
};

// The NRS fan-patch for wraiths has a bug which locks up the game. This occurs
//  when a wraith initializes while game time is greater than $7fff. The patch
//  throttles wraith:doit to execute no more than once per game tick, which it
//  does by storing the previous game time in a new local variable whose initial
//  value is zero. This technique is used in several patches but this one is
//  missing a call to Abs that the others have. Once game time reaches $8000 or
//  greater, the signed less-than test will always pass when the local variable
//  is zero, and wraith:doit won't execute.
//
// We fix this by changing the signed less-than comparison to unsigned.
//
// Applies to: English CD with NRS patches 53.HEP/SCR
// Responsible method: wraith:doit
// Fixes bug: #10711
static const uint16 qfg4WraithLockupNrsSignature[] = {
	SIG_MAGICDWORD,
	0x89, 0x58,                         // lsg 58
	0x83, 0x04,                         // lal 04
	0x04,                               // sub
	0x36,                               // push
	0x35, 0x01,                         // ldi 01
	0x22,                               // lt? [ (gameTime - prevGameTime) < 1 ]
	SIG_END
};

static const uint16 qfg4WraithLockupNrsPatch[] = {
	PATCH_ADDTOOFFSET(+8),
	0x2a,                               // ult?
	PATCH_END
};

// The script that determines how much money a revenant has is missing the first
//  parameter to kRandom, which should be zero as it is with other monsters.
//  Instead of awarding the intended 15 to 40 kopeks, it always awards 15 and
//  reseeds the random number generator to 25.
//
// Applies to: All versions
// Responsible method: sSearchMonster:changeState(1)
// Fixes bug: #10966
static const uint16 qfg4SearchRevenantSignature[] = {
	0x39, 0x0f,                         // pushi 0f
	0x78,                               // push1
	0x39, SIG_MAGICDWORD, 0x19,         // pushi 19
	0x43, 0x5d, SIG_UINT16(0x0002),     // callk Random 02
	0x02,                               // add [ 15 + Random 25 ]
	SIG_END
};

static const uint16 qfg4SearchRevenantPatch[] = {
	0x39, 0x02,                         // pushi 02
	0x39, 0x0f,                         // pushi 0f
	0x39, 0x28,                         // pushi 28
	0x43, 0x5d, PATCH_UINT16(0x0004),   // callk Random 04 [ Random 15 40 ]
	PATCH_END
};

// During combat, if a rabbit is all the way to the right and attacks then it
//  won't make any more moves, forcing the player to run away to end the fight.
//  This is due to rabbitCombat failing to pass a caller to the rabbitAttack
//  script and so it gets stuck. We pass the missing "self" parameter.
//
// Applies to: All versions
// Responsible method: rabbitCombat:changeState(1)
// Fixes bug: #11000
static const uint16 qfg4RabbitCombatSignature[] = {
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa rabbitAttack
	0X36,                               // push
	SIG_MAGICDWORD,
	0x54, SIG_UINT16(0x0006),           // self 06 [ self setScript: rabbitAttack ]
	0x32, SIG_UINT16(0x014b),           // jmp 014b
	SIG_END
};

static const uint16 qfg4RabbitCombatPatch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x7a,                               // push2
	0x74, PATCH_ADDTOOFFSET(+2),        // lofss rabbitAttack
	0x7c,                               // pushSelf
	0x54, PATCH_UINT16(0x0008),         // self 08 [ self setScript: rabbitAttack self ]
	PATCH_END
};

// Attempting to open the monastery door in room 250 while Igor is present
//  randomly locks up the game. sHectapusDeath stands Igor up, but this can be
//  interrupted by sIgorCarves animating him at random intervals, leaving
//  sHectapusDeath stuck in handsOff mode.
//
// We fix this by first stopping sIgorCarves as other scripts in this room do.
//
// Applies to: All versions
// Responsible method: sHectapusDeath:changeState(4)
// Fixes bug: #10994
static const uint16 qfg4HectapusDeathSignature[] = {
	0x30, SIG_UINT16(0x0027),           // bnt 0027
	SIG_ADDTOOFFSET(+13),
	0x30, SIG_UINT16(0x0017),           // bnt 0017
	0x38, SIG_MAGICDWORD,               // pushi setLoop
	      SIG_SELECTOR16(setLoop),
	0x7a,                               // push2
	0x7a,                               // push2
	0x78,                               // push1
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x7a,                               // push2
	0x51, SIG_ADDTOOFFSET(+1),          // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa igor
	0x4a, SIG_UINT16(0x0010),           // send 10 [ igor setLoop: 2 1 setCycle: End self ]
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x35, 0x01,                         // ldi 01
	0x65, SIG_ADDTOOFFSET(+1),          // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	SIG_END
};

static const uint16 qfg4HectapusDeathPatch[] = {
	0x30, PATCH_UINT16(0x002b),         // bnt 002b
	PATCH_ADDTOOFFSET(+13),
	0x30, PATCH_UINT16(0x001b),         // bnt 001b
	PATCH_ADDTOOFFSET(+17),
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x76,                               // push0
	0x4a, PATCH_UINT16(0x0016),         // send 16 [ igor setLoop: 2 1 setCycle: End self setScript: 0 ]
	0x3a,                               // toss
	0x48,                               // ret
	0x78,                               // push1
	0x69, PATCH_GETORIGINALBYTE(+45),   // sTop cycles
	PATCH_END
};

// Floppy 1.0 locks up when Ad Avis captures you with a Necrotaur in room 552,
//  and possibly other rooms. sBlackOut:changeState has one too many states
//  and doesn't increment the state number and cue enough in all scenarios.
//
// We fix this by removing the empty state 3 as later floppy versions do.
//
// Applies to: English Floppy 1.0
// Responsible method: sBlackOut:changeState
// Fixes bug: #11001
static const uint16 qfg4AdAvisCaptureSignature[] = {
	0x31, 0x05,                         // bnt 05 [ state 3 ]
	SIG_ADDTOOFFSET(+6),
	0x35, SIG_MAGICDWORD, 0x03,         // ldi 03
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt 05 [ state 4 ]
	SIG_ADDTOOFFSET(+6),
	0x35, 0x04,                         // ldi 04
	SIG_ADDTOOFFSET(+83),
	0x35, 0x05,                         // ldi 05
	SIG_END
};

static const uint16 qfg4AdAvisCapturePatch[] = {
	0x31, 0x10,                         // bnt 10 [ new state 3 ]
	PATCH_ADDTOOFFSET(+17),
	0x35, 0x03,                         // ldi 03 [ state 4 is now state 3 ]
	PATCH_ADDTOOFFSET(+83),
	0x35, 0x04,                         // ldi 04 [ state 5 is now state 4 ]
	PATCH_END
};

// The character selection screen in room 140 can select the wrong character.
//  The showOff script brings each through their door and sets showOff:register
//  as they animate so that myChar:doVerb knows which animating character was
//  clicked. showOff:register is supposed to be 1 - 3 but showOff sets the wrong
//  fighter value and sets the rest at the wrong times.
//
// showOff state 0 sets register to 30 instead of 1 and so clicking the fighter
//  door during the first four seconds doesn't select any character. Instead it
//  proceeds to the skill screen without updating the character type global.
//  This global's initial value is zero, which happens to be the fighter value,
//  and so this appears to work unless a different character was first selected
//  and cancel was clicked. The magic user and thief register values are correct
//  but they're set one state too late, creating 2 - 4 second windows where
//  clicking their doors selects the previously animated character.
//
// We fix this by setting showOff:register to the correct fighter value and
//  setting the magic user and thief values one state earlier.
//
// Applies to: All versions
// Responsible method: showOff:changeState
// Fixes bug: #11002
static const uint16 qfg4CharacterSelectSignature[] = {
	// state 0
	0x35, 0x1e,                         // ldi 1e
	0x65, SIG_ADDTOOFFSET(+1),          // aTop register
	SIG_ADDTOOFFSET(+461),
	// state 5
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x031e),           // jmp 031e [ end of method ]
	0x3c,                               // dup
	SIG_ADDTOOFFSET(+219),
	// state 9
	0x35, 0x02,                         // ldi 02
	0x65, SIG_ADDTOOFFSET(+1),          // aTop seconds
	0x32, SIG_UINT16(0x023b),           // jmp 023b [ end of method ]
	SIG_END
};

static const uint16 qfg4CharacterSelectPatch[] = {
	// state 0
	0x35, 0x01,                         // ldi 01
	PATCH_ADDTOOFFSET(+463),
	// state 5
	0x7a,                               // push2
	0x69, PATCH_GETORIGINALBYTE(+3),    // sTop register
	PATCH_ADDTOOFFSET(+220),
	// state 9
	0x7a,                               // push2
	0x69, PATCH_GETORIGINALBYTE(+691),  // sTop seconds
	0x39, 0x03,                         // pushi 03
	0x69, PATCH_GETORIGINALBYTE(+3),    // sTop register
	PATCH_END
};

// Clicking Look in the dungeon (room 670) responds with the dungeon description
//  followed by the generic message for not seeing anything. rm670:doVerb is
//  missing a return statement and so it proceeds with generic verb handling.
//
// Applies to: All versions
// Responsible method: rm670:doVerb
static const uint16 qfg4LookDungeonSignature[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x38, SIG_UINT16(0x0006),           // pushi 0006
	SIG_MAGICDWORD,
	0x76,                               // push0
	0x78,                               // push1
	0x76,                               // push0
	0x76,                               // push0
	0x76,                               // push0
	0x38, SIG_UINT16(0x029e),           // pushi 029e
	0x81, 0x5b,                         // lag 5b
	0x4a, SIG_UINT16(0x0010),           // send 10 [ gloryMessager say: 0 1 0 0 0 670 ]
	SIG_END
};

static const uint16 qfg4LookDungeonPatch[] = {
	PATCH_ADDTOOFFSET(+11),
	0x89, 0x0b,                         // lsg 0b [ room number, saves a byte ]
	0x81, 0x5b,                         // lag 5b
	0x4a, PATCH_UINT16(0x0010),         // send 10 [ gloryMessager say: 0 1 0 0 0 670 ]
	0x48,                               // ret
	PATCH_END
};

// When approaching the door to the great hall in staircase room 627 at night,
//  the message "You hear voices..." continues to occur even after witnessing
//  the argument between Katrina and Ad Avis in the floppy version. This is due
//  to not testing flag 112, which is set by the argument scene, and was fixed
//  in the CD version. We add the missing flag test.
//
// This incomplete logic to determine if Katrina and Ad Avis are in the great
//  hall is duplicated throughout this script. Although Sierra fixed this
//  instance in the CD version, it's the only one they fixed, while adding more
//  that lack the flag test. We fix those bugs in subsequent patches.
//
// Applies to: English Floppy, German Floppy
// Responsible method: sDisplay:changeState(0)
// Fixes bug: #10799
static const uint16 qfg4ArgumentMessageFloppySignature[] = {
	SIG_MAGICDWORD,
	0x83, 0x02,                         // lal 02   [ message already said? ]
	0x30, SIG_UINT16(0x0013),           // bnt 0013 [ say message ]
	0x38, SIG_SELECTOR16(handsOn),      // pushi handsOn
	0x76,                               // push0
	0x81, 0x01,                         // lag 01
	0x4a, SIG_UINT16(0x0004),           // send 04 [ Glory handsOn: ]
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	0x32, SIG_UINT16(0x0079),           // jmp 0079 [ end of method ]
	SIG_ADDTOOFFSET(+38),
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04  [ self dispose: ]
	0x32, SIG_UINT16(0x0049),           // jmp 0049 [ end of method ]
	SIG_END
};

static const uint16 qfg4ArgumentMessageFloppyPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x2f, 0x09,                         // bt 09 [ skip message if already said ]
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x31, 0x0b,                         // bnt 0b [ say message ]
	0x38, PATCH_SELECTOR16(handsOn),    // pushi handsOn
	0x76,                               // push0
	0x81, 0x01,                         // lag 01
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ Glory handsOn: ]
	0x33, 0x26,                         // jmp 26 [ self dispose:, end of method ]
	PATCH_END
};

// The great hall door options in room 627 are incorrect at night after Katrina
//  and Ad Avis argue. rm620Code:init is missing a flag test to prevent the
//  argument options from reoccurring. We add the missing flag test.
//
// Applies to: All versions
// Responsible methods: rm620Code:init
// Fixes bug: #10799
static const uint16 qfg4Room627DoorOptionsSignature[] = {
	0x89, 0x0b,                         // lsg 0b [ room number ]
	SIG_ADDTOOFFSET(+28),
	0x39, 0x05,                         // pushi 05 [ argument door options ]
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa doorTopTeller
	0x4a, SIG_UINT16(0x000e),           // send 0e [ doorTopTeller init: pUpperDoor 620 8 155 5 ]
	SIG_MAGICDWORD,
	0x33, 0x19,                         // jmp 19
	0x38, SIG_SELECTOR16(init),         // pushi init
	0x38, SIG_UINT16(0x0005),           // pushi 0005
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa pUpperDoor
	0x36,                               // push
	0x38, SIG_UINT16(0x026c),           // pushi 026c
	0x39, 0x08,                         // pushi 08
	0x38, SIG_UINT16(0x009b),           // pushi 009b
	0x78,                               // push1 [ normal door options ]
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa doorTopTeller
	0x4a, SIG_UINT16(0x000e),           // send 0e [ doorTopTeller init: pUpperDoor 620 8 155 1 ]
	SIG_END
};

static const uint16 qfg4Room627DoorOptionsPatch[] = {
	0x33, 0x0a,                         // jmp 0a
	PATCH_ADDTOOFFSET(+28),
	0x89, 0x0b,                         // lsg 0b [ room number ]
	0x34, PATCH_UINT16(0x0273),         // ldi 627d
	0x1a,                               // eq?
	0x31, 0x14,                         // bnt 14 [ normal door options ]
	0x81, 0x79,                         // lag 79 [ night ]
	0x31, 0x10,                         // bnt 10 [ normal door options ]
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x2f, 0x07,                         // bt 07 [ normal door options ]
	0x39, 0x05,                         // pushi 05
	0x33, 0x04,                         // jmp 04 [ argument door options ]
	PATCH_END
};

// The responses to the great hall door options in room 627 have problems. The
//  floppy version is missing message handlers for Open Door and Knock on Door
//  during the argument and so the door doesn't open. The CD version displays
//  the wrong Knock on Door message. All versions fail to test flag 112 to see
//  if the argument has already occurred.
//
// We fix all of this with a two part patch. First, sListened:register now
//  controls which message is displayed before the door opens. This allows
//  doorTopTeller:sayMessage to specify the correct message before running it.
//  To add flag tests, new message handlers, and set sListened:register, we take
//  advantage of identical message handlers in doorTopTeller:sayMessage. These
//  provide plenty of room for new code that then jumps into another handler to
//  continue and complete the work.
//
// Applies to: All versions
// Responsible methods: sListened:changeState(0), doorTopTeller:sayMessage
// Fixes bug: #10799
static const uint16 qfg4Room627DoorResponsesSignature1[] = {
	0x65, SIG_ADDTOOFFSET(+17),         // aTop state
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x38, SIG_UINT16(0x0006),           // pushi 0006
	0x39, 0x08,                         // pushi 08
	0x38, SIG_UINT16(0x009b),           // pushi 009b
	0x39, SIG_MAGICDWORD, 0x09,         // pushi 09
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x38, SIG_UINT16(0x026c),           // pushi 026c
	0x81, 0x5b,                         // lag 5b
	0x4a, SIG_UINT16(0x0010),           // send 10 [ gloryMessager say: 8 155 9 1 self 620 ]
	0x32,                               // jmp ... [ end of method ]
	SIG_END
};

static const uint16 qfg4Room627DoorResponsesPatch1[] = {
	PATCH_ADDTOOFFSET(+21),
	0x39, 0x06,                         // pushi 06
	0x39, 0x08,                         // pushi 08
	0x38, PATCH_UINT16(0x009b),         // pushi 009b
	0x39, 0x09,                         // pushi 09
	0x63, PATCH_GETORIGINALBYTEADJUST(+1, 0x10), // pToa register
	0x02,                               // add
	0x36,                               // push
	0x78,                               // push1
	0x7c,                               // pushSelf
	0x38, PATCH_UINT16(0x026c),         // pushi 026c
	0x81, 0x5b,                         // lag 5b
	0x4a, PATCH_UINT16(0x0010),         // send 10 [ gloryMessager say: 8 155 (9 + register) 1 self 620 ]
	PATCH_END
};

static const uint16 qfg4Room627DoorResponsesFloppySignature2[] = {
	// Pick Lock (no argument) - missing flag check
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x0072),           // bnt 0072 [ next message handler ]
	0x81, 0x79,                         // lag 79 [ night ]
	0x30, SIG_UINT16(0x0028),           // bnt 0028
	0x89, 0x0b,                         // lsg 0b [ room number ]
	0x34, SIG_UINT16(0x0273),           // ldi 627d
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x001f),           // bnt 001f
	0x38, SIG_UINT16(0x00fe),           // pushi clean [ hard-coded for floppy ]
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0x0082),           // lofsa sListened
	0x36,                               // push
	0x72, SIG_UINT16(0x01ae),           // lofsa pUpperDoor
	0x4a, SIG_UINT16(0x0006),           // send 06 [ pUpperDoor setScript: sListened ]
	0x32, SIG_UINT16(0x0131),           // jmp 0131
	0x38, SIG_UINT16(0x0338),           // pushi trySkill [ hard-coded for floppy ]
	0x7a,                               // push2
	0x39, 0x09,                         // pushi 09
	0x88, SIG_UINT16(0x01a6),           // lsg 01a6
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0008),           // send 08 [ hero trySkill: 9 global422 ]
	SIG_ADDTOOFFSET(+0xe4),
	// Open Door (no argument) - missing flag check
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02
	0x38, SIG_UINT16(0x00fe),           // pushi clean [ hard-coded for floppy ]
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	SIG_END
};

static const uint16 qfg4Room627DoorResponsesFloppyPatch2[] = {
	// Pick Lock (no argument) - missing flag check
	0x30, PATCH_UINT16(0x001a),         // bnt 001a [ next message handler ]
	PATCH_ADDTOOFFSET(+2),
	0x30, PATCH_UINT16(0x00a1),         // bnt 00a1 [ normal pick lock code in duplicate handler ]
	PATCH_ADDTOOFFSET(+6),
	0x30, PATCH_UINT16(0x0098),         // bnt 0098 [ normal pick lock code in duplicate handler ]
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x2e, PATCH_UINT16(0x008e),         // bt 008e [ normal pick lock code in duplicate handler ]
	0x33, 0x6d,                         // jmp 6d  [ continue argument code in duplicate handler ]
	// Open Door (argument) - missing from floppy
	0x3c,                               // dup
	0x35, 0x0a,                         // ldi 0a
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt 07 [ next message handler ]
	0x38, PATCH_SELECTOR16(register),   // pushi register
	0x78,                               // push1
	0x78,                               // push1 [ "Ignoring the voices, you fling the door open..." ]
	0x33, 0x0b,                         // jmp 0b
	// Knock on Door (argument) - missing from floppy
	0x3c,                               // dup
	0x35, 0x0b,                         // ldi 0b
	0x1a,                               // eq?
	0x31, 0x45,                         // bnt 45 [ next message handler ]
	0x38, PATCH_SELECTOR16(register),   // pushi register
	0x78,                               // push1
	0x7a,                               // push2 [ "Very polite of you. The door opens to your knock..." ]
	0x72, PATCH_UINT16(0x0082),         // lofsa sListened
	0x4a, PATCH_UINT16(0x0006),         // send 0006 [ sListened register: 1 or 2 ]
	0x32, PATCH_UINT16(0x004c),         // jmp 004c [ continue argument code in duplicate handler ]
	PATCH_ADDTOOFFSET(+0xe4),
	// Open Door (no argument) - missing flag check
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x2f, 0x16,                         // bt 16 [ skip argument code if argument occurred ]
	0x32, PATCH_UINT16(0xff5c),         // jmp ff5c [ continue argument code in duplicate handler ]
	PATCH_END
};

static const uint16 qfg4Room627DoorResponsesCDSignature2[] = {
	// Pick Lock (no argument) - missing flag check
	0x38, SIG_UINT16(0x0101),           // pushi clean [ hard-coded for CD ]
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x54, SIG_UINT16(0x0004),           // self 04
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02
	SIG_ADDTOOFFSET(+0x13b),
	// Open Door (no argument) - missing flag check
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02
	0x38, SIG_UINT16(0x0101),           // pushi clean [ hard-coded for CD ]
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	SIG_ADDTOOFFSET(+0x7c),
	// Knock on Door (argument) - wrong message (CD regression)
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02
	0x38, SIG_UINT16(0x0101),           // pushi clean [ hard-coded for CD ]
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	SIG_END
};

static const uint16 qfg4Room627DoorResponsesCDPatch2[] = {
	// Pick Lock (no argument) - missing flag check
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x2f, 0x16,                         // bt 16 [ skip argument code if argument occurred ]
	0x32, PATCH_UINT16(0x0089),         // jmp 0089 [ continue argument code in duplicate handler ]
	PATCH_ADDTOOFFSET(+0x13d),
	// Open Door (no argument) - missing flag check
	0x78,                               // push1
	0x39, 0x70,                         // pushi 70 [ flag 112 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ has argument occurred? ]
	0x2f, 0x16,                         // bt 16 [ skip argument code if argument occurred ]
	0x32, PATCH_UINT16(0xff40),         // jmp ff40 [ continue argument code in duplicate handler ]
	PATCH_ADDTOOFFSET(+0x7e),
	// Knock on Door (argument) - wrong message (CD regression)
	0x38, PATCH_SELECTOR16(register),   // pushi register
	0x78,                               // push1
	0x7a,                               // push2 [ "Very polite of you. The door opens to your knock..." ]
	0x72, PATCH_UINT16(0x00b6),         // lofsa sListened
	0x4a, PATCH_UINT16(0x0006),         // send 0006 [ sListened register: 2 ]
	0x32, PATCH_UINT16(0xfeb4),         // jmp feb4 [ continue argument code in duplicate handler ]
	PATCH_END
};

// When entering the great hall from room 627 while Katrina and Ad Avis argue,
//  room 627 says that the door squeaks even if it was oiled. The flag tests are
//  incorrect and out of sync with the logic in the great hall that plays the
//  squeak that kills ego. This logic had other bugs in the floppy version,
//  including setting an incorrect flag, and this patch fixes those too. The end
//  result is that the squeak message isn't displayed if the oiled flag is set.
//
// Applies to: All versions
// Responsible methods: sListened:changeState(2), sListened2:changeState(2) (CD)
// Fixes bug: #10799
static const uint16 qfg4Room627SqueakFloppySignature[] = {
	0x89, 0x7d,                         // lsg 7d [ character type ]
	0x35, SIG_MAGICDWORD, 0x02,         // ldi 02 [ thief ]
	0x1a,                               // eq?    [ is thief? ]
	0x31, 0x08,                         // bnt 08 [ skip oil test if thief (incorrect, removed from CD) ]
	0x78,                               // push1
	0x39, 0x72,                         // pushi 72 [ flag 114 ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is door oiled? ]
	0x18,                               // not
	0x2f, 0x07,                         // bt 07 [ squeak message ]
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71 [ flag 113 ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ selected action other than listening? (incorrect) ]
	0x31, 0x33,                         // bnt 33 [ skip squeak message ]
	SIG_ADDTOOFFSET(+20),
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71 [ flag 113 ]
	0x45, 0x02, SIG_UINT16(0x0002),     // callb proc0_2 02 [ set flag 113 (incorrect, removed from CD) ]
	SIG_END
};

static const uint16 qfg4Room627SqueakFloppyPatch[] = {
	0x33, 0x05,                         // jmp 05 [ skip thief test, always test oil flag ]
	PATCH_ADDTOOFFSET(+15),
	0x32, PATCH_UINT16(0x0039),         // jmp 0039 [ skip squeak message if door has been oiled ]
	PATCH_ADDTOOFFSET(+26),
	0x32, PATCH_UINT16(0x0004),         // jmp 0004 [ don't set flag 113 ]
	PATCH_END
};

static const uint16 qfg4Room627SqueakCDSignature[] = {
	0x78,                               // push1
	0x39, 0x72,                         // pushi 72 [ flag 114 ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is door oiled? ]
	SIG_MAGICDWORD,
	0x18,                               // not
	0x2f, 0x07,                         // bt 07 [ squeak message ]
	0x78,                               // push1
	0x39, 0x71,                         // pushi 71 [ flag 113 ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ selected action other than listening? (incorrect) ]
	0x31, 0x2c,                         // bnt 2c [ skip squeak message ]
	SIG_END
};

static const uint16 qfg4Room627SqueakCDPatch[] = {
	PATCH_ADDTOOFFSET(+10),
	0x32, PATCH_UINT16(0x0032),         // jmp 0032 [ skip squeak message if door has been oiled ]
	PATCH_END
};

// Looking through the keyhole in room 627 into the great hall responds with a
//  generic message instead of the specific room description. sPeepingTom only
//  says the great hall message at night, which isn't relevant, and didn't work
//  since doorTeller wouldn't offer the keyhole option at night due to a missing
//  flag check which we fix. We restore the message by removing the night test.
//
// Applies to: All versions
// Responsible method: sPeepingTom:changeState(1)
// Fixes bug: #10799
static const uint16 qfg4GreatHallKeyholeSignature[] = {
	SIG_MAGICDWORD,
	0x81, 0x79,                         // lag 79 [ night ]
	0x31, 0x1a,                         // bnt 1a [ skip keyhole message during day ]
	0x38, SIG_SELECTOR16(say),          // pushi say
	SIG_END
};

static const uint16 qfg4GreatHallKeyholePatch[] = {
	0x33, 0x02,                         // jmp 02 [ say keyhole message regardless of time ]
	PATCH_END
};

// You can talk to the Burgomeister in his office when he's not there. Clicking
//  Talk on hero shows Burgomeister options even when alone because rm300:init
//  initializes heroTeller no matter who is in the room.
//
// We fix this by only initializing heroTeller when someone is in the room. The
//  Burgomeister appears when the time of day is 3 or less and Gypsy Davy
//  appears during room events 4, 5, and 6.
//
// Applies to: All versions
// Responsible method: rm300:init
// Fixes bug: #10754
static const uint16 qfg4EmptyBurgoRoomSignature[] = {
	// start of heroTeller init: ...
	0x38, SIG_SELECTOR16(init),         // pushi init
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0005),           // pushi 0005
	0x89, 0x00,                         // lsg 00     [ hero ]
	0x38, SIG_UINT16(0x012c),           // pushi 300d [ modNum ]
	0x39, 0x19,                         // pushi 25d  [ noun ]
	0x38, SIG_UINT16(0x0080),           // pushi 128d [ verb ]
	0x8b, 0x00,                         // lsl 00 [ event number ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt 05
	0x35, 0x10,                         // ldi 10 [ cond ]
	0x32, SIG_UINT16(0x0048),           // jmp 0048
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 04
	0x35, 0x11,                         // ldi 11 [ cond ]
	0x33, 0x3e,                         // jmp 3e
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 04
	0x35, 0x13,                         // ldi 13 [ cond ]
	0x33, 0x34,                         // jmp 34
	0x3c,                               // dup
	0x35, 0x05,                         // ldi 05
	SIG_END
};

static const uint16 qfg4EmptyBurgoRoomPatch[] = {
	0x89, 0x7b,                         // lsg 7b
	0x35, 0x03,                         // ldi 03
	0x24,                               // le?   [ time of day <= 3 ]
	0x2f, 0x0e,                         // bt 0e [ burgomeister is here, call heroTeller:init ]
	0x8b, 0x00,                         // lsl 00
	0x35, 0x04,                         // ldi 04
	0x22,                               // lt?   [ event number < 4 ]
	0x2f, 0x5f,                         // bt 5f [ no gypsy, skip heroTeller:init ]
	0x8b, 0x00,                         // lsl 00
	0x35, 0x06,                         // ldi 06
	0x1e,                               // gt?   [ event number > 6 ]
	0x2f, 0x58,                         // bt 58 [ no gypsy, skip heroTeller:init ]
	// start of heroTeller init: ...
	0x38, PATCH_SELECTOR16(init),       // pushi init
	0x39, 0x05,                         // pushi 05
	0x89, 0x00,                         // lsg 00     [ hero ]
	0x89, 0x0b,                         // lsg 0b     [ modNum ]
	0x39, 0x19,                         // pushi 25d  [ noun ]
	0x38, PATCH_UINT16(0x0080),         // pushi 128d [ verb ]
	0x83, 0x00,                         // lal 00 [ event number ]
	0x36,                               // push
	0x31, 0x13,                         // bnt 13 [ event number == 0, next condition ]
	0x3c,                               // dup
	0x35, 0x05,                         // ldi 05
	0x1e,                               // gt?
	0x2f, 0x0d,                         // bt 0d [ event number > 5, next condition ]
	0x3c,                               // dup
	0x35, 0x0f,                         // ldi 0f
	0x02,                               // add [ cond = event number + 15d ]
	0x33, 0x31,                         // jmp 31
	PATCH_END
};

// Ad Avis and his necrotaurs chase and catch hero to take him to the dungeon,
//  but this one-time event can repeat along with the entire dungeon sequence.
//
// The chase code is complex and spread across many scripts with many flags.
//  There is no code that resets all the flags upon capture and no code that
//  correctly tests if capture has occurred. This results in many ways to leave
//  at least one flag set and repeat the chase. What these code paths all have
//  in common is that the player has to walk through the town gate, room 290.
//
// We fix this by adding code to the start of rm290:init that clears all of the
//  relevant chase flags if capture has already occurred. Fortunately, this
//  method starts with unused debugging code that can be overwritten.
//
// Applies to: All versions
// Responsible method: rm290:init
// Fixes bug: #11056
static const uint16 qfg4ChaseRepeatsSignature[] = {
	SIG_MAGICDWORD,
	0x81, 0xc9,                         // lag c9 [ debug mode ]
	0x31, 0x4e,                         // bnt 4e [ skip debug code ]
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa prompt
	0x36,                               // push
	0x46, SIG_UINT16(0xfaff),           // calle proc64255_1 02
	      SIG_UINT16(0x0001),
	      SIG_UINT16(0x0002),
	0xa3, 0x02,                         // sal 02
	0x39, 0x05,                         // pushi 05
	0x36,                               // push
	0x78,                               // push1
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	0x39, 0x04,                         // pushi 04
	0x46, SIG_UINT16(0xfde7),           // calle proc64999_5 0a
	      SIG_UINT16(0x0005),
	      SIG_UINT16(0x000a),
	SIG_END
};

static const uint16 qfg4ChaseRepeatsPatch[] = {
	0x78,                               // push1
	0x39, 0x6e,                         // pushi 6e [ flag 110 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ have you been captured? ]
	0x31, 0x49,                         // bnt 49 [ don't clear chase flags ]
	0x78,                               // push1
	0x39, 0x50,                         // pushi 50 [ flag 80 ]
	0x45, 0x03, PATCH_UINT16(0x0002),   // callb proc0_3 02 [ clear chase flag 80 ]
	0x78,                               // push1
	0x39, 0x51,                         // pushi 51 [ flag 81 ]
	0x45, 0x03, PATCH_UINT16(0x0002),   // callb proc0_3 02 [ clear chase flag 81 ]
	0x78,                               // push1
	0x38, PATCH_UINT16(0x00a4),         // pushi 00a4 [ flag 164 ]
	0x45, 0x03, PATCH_UINT16(0x0002),   // callb proc0_3 02 [ clear chase flag 164 ]
	0x33, 0x31,                         // jmp 31 [ continue room init ]
	PATCH_END
};

// When the necrotaurs catch hero in the woods to take him to the dungeon, two
//  different script bugs randomly cause interpreter errors. This patch fixes
//  calling kUpdateScreenItem on a necrotaur with a deleted screen item.
//
// When hero is caught, the screen turns black before going to the dungeon.
//  There is an inconsistent delay and the necrotaurs will sometimes reappear
//  briefly on the black screen. These symptoms hint at the script's problems.
//  sBlackOut sets a 300 cycle delay but this rarely has an effect. Instead each
//  actor is hidden while their motions continue and sBlackOut advances when an
//  invisible necrotaur completes its JumpTo motion. JumpTo stores its client's
//  signal on initialization and unconditionally restores it upon completion. If
//  JumpTo completes after View:hide calls kDeleteScreenItem and sets the hidden
//  signal bit then signal is reverted and Actor:doit calls kUpdateScreenItem.
//
// We fix this by disposing of each cast member before painting black instead of
//  hiding them. This hides them and terminates their motions. This exposes the
//  previously unused 300 cycle delay, which is much longer than normal, so we
//  also change that to 4 seconds. This is consistent with existing behavior
//  and close to the delay used in room 290's working version of this script.
//  The majority of this patch is to free up the single byte needed to change
//  the 8-bit hide selector to 16-bit dispose.
//
// Several rooms that sBlackOut takes place in, such as 557, have doit methods
//  that can initiate new necrotaur motions after the cast has been disposed,
//  which also crashes. We fix this by clearing flag 35 as each of these rooms
//  requires it to be set to set a necrotaur motion. This is the "hunt" flag.
//  It's okay to clear it here as it gets cleared in the dungeon.
//
// Applies to: All versions
// Responsible method: sBlackOut:changeState(3)
// Fixes bug: #11056
static const uint16 qfg4NecrotaurBlackoutSignature[] = {
	0x31, 0x4f,                         // bnt 4f [ next state ]
	SIG_ADDTOOFFSET(+11),
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(hide),          // pushi hide
	0x81, 0x05,                         // lag 05
	0x4a, SIG_UINT16(0x0006),           // send 06 [ cast eachElementDo: hide ]
	0x78,                               // push1 [ kUpdatePlane param count ]
	0x39, SIG_SELECTOR8(back),          // pushi back
	0x78,                               // push1
	0x76,                               // push0
	0x39, SIG_ADDTOOFFSET(+1),          // pushi picture
	0x78,                               // push1
	0x39, 0xff,                         // pushi ff
	0x38, SIG_ADDTOOFFSET(+2),          // pushi yourself [ returns self ]
	0x76,                               // push0
	0x76,                               // push0 [ plane ]
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, SIG_UINT16(0x0004),           // send 04 [ room plane? ]
	0x4a, SIG_UINT16(0x0010),           // send 10 [ room:plane back: 0 picture: -1 yourself: ]
	0x36,                               // push [ room:plane for kUpdatePlane ]
	SIG_ADDTOOFFSET(+29),
	0x34, SIG_UINT16(0x012c),           // ldi 012c
	0x65, SIG_ADDTOOFFSET(+1),          // aTop cycles [ cycles = 300 ]
	0x33, 0x1c,                         // jmp 1c [ end of method ]
	0x3c,                               // dup
	0x35, SIG_ADDTOOFFSET(+1),          // ldi 04 or 05
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt 16 [ end of method ]
	SIG_END
};

static const uint16 qfg4NecrotaurBlackoutPatch[] = {
	0x31, 0x55,                         // bnt 55 [ next state ]
	PATCH_ADDTOOFFSET(+11),
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x81, 0x05,                         // lag 05
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ cast eachElementDo: dispose ]
	0x78,                               // push1 [ kUpdatePlane param count ]
	0x76,                               // push0 [ plane ]
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ room plane? ]
	0x36,                               // push [ room:plane for kUpdatePlane ]
	0x39, PATCH_SELECTOR8(back),        // pushi back
	0x39, 0x01,                         // pushi 01
	0x39, 0x00,                         // pushi 00
	0x39, PATCH_GETORIGINALBYTE(+26),   // pushi picture
	0x39, 0x01,                         // pushi 01
	0x39, 0xff,                         // pushi ff
	0x4a, PATCH_UINT16(0x000c),         // send 0c [ room:plane back: 0 picture: -1 ]
	PATCH_ADDTOOFFSET(+29),
	0x35, 0x04,                         // ldi 04
	0x65, PATCH_GETORIGINALBYTEADJUST(+78, +2), // aTop seconds [ seconds = 4 ]
	0x78,                               // push1
	0x39, 0x23,                         // pushi 23
	0x45, 0x03, PATCH_UINT16(0x0002),   // callb proc0_3 02 [ clear flag 35 ]
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_END
};

// When the necrotaurs catch hero in the woods to take him to the dungeon an
//  effectively random crash from the Grooper class can occur in the CD version.
//  See the inn door's script patch for details on these CD regressions.
//
// The error occurs when sBlackOut sets the motion of a necrotaur while it has
//  no cycler. In this case the cycler should still be Walk but CD regressions
//  can cause it to be cleared at unexpected times. We prevent this by setting
//  necrotaur cyclers to Walk when setting their final PChase motions.
//
// Applies to: PC CD
// Responsible method: sBlackOut:changeState(0)
// Fixes bug: #11056
static const uint16 qfg4NecrotaurCaptureSignature[] = {
	SIG_MAGICDWORD,
	0x18,                               // not
	0x31, 0x38,                         // bnt 38
	0x38, SIG_ADDTOOFFSET(+2),          // pushi distanceTo
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa nec (nec1 or nec2 or nec3)
	0x36,                               // push
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0006),           // send 06 [ hero distanceTo: nec ]
	0x36,                               // push
	0x35, 0x19,                         // ldi 19
	0x1e,                               // gt?
	0x31, 0x19,                         // bnt 19
	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion
	0x38, SIG_UINT16(0x0004),           // pushi 0004
	0x51, 0x6c,                         // class PChase
	0x36,                               // push
	0x89, 0x00,                         // lsg 00
	0x39, 0x19,                         // pushi 19
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa nec
	0x36,                               // push
	0x72,                               // lofsa nec
	SIG_END
};

static const uint16 qfg4NecrotaurCapturePatch[] = {
	0x2f, 0x39,                         // bt 39
	0x38, PATCH_GETORIGINALUINT16(+4),  // pushi distanceTo
	0x78,                               // push1
	0x74, PATCH_GETORIGINALUINT16(+8),  // lofss nec (nec1 or nec2 or nec3)
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ hero distanceTo: nec ]
	0x39, 0x19,                         // pushi 19
	0x24,                               // le?
	0x31, 0x1c,                         // bnt 1c
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x78,                               // push1
	0x51, 0x17,                         // class Walk
	0x36,                               // push
	0x38, PATCH_SELECTOR16(setMotion),  // pushi setMotion
	0x39, 0x04,                         // pushi 04
	0x51, 0x6c,                         // class PChase
	0x36,                               // push
	0x89, 0x00,                         // lsg 00
	0x39, 0x19,                         // pushi 19
	0x72, PATCH_GETORIGINALUINT16(+8),  // lofsa nec
	0x36,                               // push
	PATCH_END
};

// When entering room 600 from the south at night, paladins receive a message
//  about the two necrotaurs guarding the gate even when they're not there. The
//  necrotaurs' appearance depends on flags, events, and if they've been killed,
//  but the message code only tests if it's night.
//
// We fix this by only showing the message if there are at least 4 cast members
//  in the room at night, which only occurs when the necrotaurs are present.
//
// Applies to: All versions
// Responsible method: sFromSouth:changeState(1)
// Fixes bug: #11057
static const uint16 qfg4NecrotaurMessageSignature[] = {
	0x30, SIG_UINT16(0x0052),           // bnt 0052 [ state 1 ]
	SIG_ADDTOOFFSET(+0x4f),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x004b),           // jmp 004b [ end of method ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0044),           // bnt 0044 [ end of method ]
	0x81, 0x79,                         // lag 79 [ night ]
	0x30, SIG_UINT16(0x0022),           // bnt 0022
	0x89, 0x7d,                         // lsg 7d [ character type ]
	0x35, 0x03,                         // ldi 03 [ paladin ]
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0011),           // bnt 0011 [ skip message ]
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x38, SIG_UINT16(0x0003),           // pushi 0003
	SIG_ADDTOOFFSET(+0x31),
	0x3a,                               // toss
	SIG_END
};

static const uint16 qfg4NecrotaurMessagePatch[] = {
	0x3a,                               // toss
	0x31, 0x50,                         // bnt 50 [ state 1 ]
	PATCH_ADDTOOFFSET(+0x4f),
	0x48,                               // ret
	0x81, 0x79,                         // lag 79 [ night ]
	0x31, 0x2c,                         // bnt 2c
	0x7a,                               // push2
	0x81, 0x7d,                         // lag 7d [ character type ]
	0x22,                               // lt?
	0x31, 0x1d,                         // bnt 1d [ skip message ]
	0x39, 0x04,                         // pushi 04
	0x39, PATCH_SELECTOR8(size),        // pushi size
	0x76,                               // push0
	0x81, 0x05,                         // lag 05
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ cast size? ]
	0x24,                               // le?     [ at least 4 cast members? ]
	0x31, 0x10,                         // bnt 10  [ skip message ]
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x39, 0x03,                         // pushi 03
	PATCH_ADDTOOFFSET(+0x31),
	0x48,                               // ret
	PATCH_END
};

// When returning to the castle gate (room 600) from the dungeon (room 670) the
//  gate options and messages can be out of sync with the gatekeeper's presence.
//  Returning from the dungeon is room event 11, which causes rm600:init to
//  advance the time to night and hide the gatekeeper, but this happens after
//  rm600:init has initialized gateTeller based on whether it was day or night.
//
// We fix this by setting the night global before gateTeller is initialized
//  during room event 11. Fortunately, the gateTeller code is preceded by a
//  redundant condition which is always false and can be overwritten.
//
// Applies to: All versions
// Responsible method: rm600:init
// Fixes bug: #11044
static const uint16 qfg4GateOptionsSignature[] = {
	0x35, 0x0a,                         // ldi 0a [ event 10 ]
	0x1a,                               // eq?    [ always false, tested earlier ]
	0x31, SIG_ADDTOOFFSET(+1),          // bnt    [ gateTeller init ]
	0x38, SIG_SELECTOR16(posn),         // pushi posn
	0x7a,                               // push2
	0x39, SIG_MAGICDWORD, 0xe2,         // pushi e2
	0x39, 0x69,                         // pushi 69
	0x72,                               // lofsa aGate
	SIG_END
};

static const uint16 qfg4GateOptionsPatch[] = {
	0x35, 0x0b,                         // ldi 0b [ event 11, came from dungeon ]
	PATCH_ADDTOOFFSET(+3),
	0x35, 0x01,                         // ldi 01
	0xa1, 0x79,                         // sag 79 [ night = 1 ]
	0x33, PATCH_GETORIGINALBYTEADJUST(+4, -6), // jmp [ gateTeller init ]
	PATCH_END
};

// Six castle rooms contain a bug where oiling one door in the room effectively
//  oils the rest when picking locks. sPickLock doesn't test which door is being
//  opened, only their oil flags, and if any are set then it doesn't squeak.
//
// We fix this by adding code to test hero's position to determine which door is
//  being opened and then test the correct flag. Ideally we should only need two
//  versions of this patch: a two-door version and a three-door. Unfortunately,
//  each two-door sPickLock is different. Script 634 adds an unnecessary room
//  test, 643 and 644 transpose the left and right door flags, and script 661
//  tests a flag lower than 128 which changes a key instruction size. Room 631's
//  sPickLock is in script 634 and it is the only room that uses it.
//
// Applies to: All versions
// Responsible method: sPickLock:changeState(1) in scripts 634, 640, 642, 643, 644, 661
// Fixes bug: #10832
static const uint16 qfg4Room631LockSqueakSignature[] = {
	0x89, 0x0b,                         // lsg 0b
	0x34, SIG_UINT16(0x0277),           // ldi 0277
	0x1a,                               // eq? [ is room 631? (always true) ]
	0x31, SIG_MAGICDWORD, 0x14,         // bnt 14 [ right flag test ]
	0x78,                               // push1
	0x38, SIG_UINT16(0x00d1),           // pushi 00d1 [ flag 209 ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x0a,                         // bnt 0a [ right flag test ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	SIG_ADDTOOFFSET(+21),
	0x38, SIG_SELECTOR16(cue),          // pushi cue [ no squeak ]
	SIG_ADDTOOFFSET(+7),
	0x39, SIG_SELECTOR8(play),          // pushi play [ squeak ]
	SIG_END
};

static const uint16 qfg4Room631LockSqueakPatch[] = {
	0x38, PATCH_UINT16(0x00a0),         // pushi 00a0
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero x? ]
	0x1e,                               // gt? [ 160 > hero:x ]
	0x31, 0x0f,                         // bnt 0f [ right flag test ]
	0x78,                               // push1
	0x38, PATCH_UINT16(0x00d1),         // pushi 00d1 [ flag 209 ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x21,                         // bnt 21 [ squeak ]
	0x33, 0x15,                         // jmp 15 [ no squeak ]
	PATCH_END
};

// rooms 640 and 642 have three doors
static const uint16 qfg4Room640LockSqueakSignature[] = {
	0x78,                               // push1
	0x38, SIG_ADDTOOFFSET(+2),          // pushi middle flag
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is middle door oiled? ]
	SIG_MAGICDWORD,
	0x31, 0x0a,                         // bnt 0a [ left door flag test ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x78,                               // push1
	0x38, SIG_ADDTOOFFSET(+2),          // pushi left flag
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x0a,                         // bnt 0a [ right door flag test ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x78,                               // push1
	0x38, SIG_ADDTOOFFSET(+2),          // pushi right flag
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x0a,                         // bnt 0a [ squeak ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	SIG_ADDTOOFFSET(+3),
	0x39, SIG_SELECTOR8(play),          // pushi play [ squeak ]
	SIG_ADDTOOFFSET(+28),
	0x38, SIG_SELECTOR16(cue),          // pushi cue [ no squeak ]
	SIG_END
};

static const uint16 qfg4Room640LockSqueakPatch[] = {
	0x39, 0x2e,                         // pushi 2e
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero x? ]
	0x1e,                               // gt? [ 46 > hero:x ]
	0x31, 0x0c,                         // bnt 0c [ middle door test ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+22), // pushi left flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x26,                         // bnt 26 [ squeak ]
	0x33, 0x42,                         // jmp 42 [ no squeak ]
	0x60,                               // pprev [ hero:x ]
	0x34, PATCH_UINT16(0x00e6),         // ldi 00e6
	0x22,                               // lt? [ hero:x < 230 ]
	0x31, 0x0c,                         // bnt 0c [ right flag test ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+2),  // pushi middle flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is middle door oiled? ]
	0x31, 0x13,                         // bnt 13 [ squeak ]
	0x33, 0x2f,                         // jmp 2f [ no squeak ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+42), // pushi right flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x07,                         // bnt 07 [ squeak ]
	0x33, 0x23,                         // jmp 23 [ no squeak ]
	PATCH_END
};

// room 643 has two doors but no room test as in script 634
static const uint16 qfg4Room643LockSqueakSignature[] = {
	0x78,                               // push1
	0x38, SIG_ADDTOOFFSET(+2),          // pushi flag
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is door oiled? ]
	SIG_MAGICDWORD,
	0x31, 0x0a,                         // bnt 0a [ other door flag test ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x78,                               // push1
	0x38, SIG_ADDTOOFFSET(+2),          // pushi flag
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is other door oiled? ]
	0x31, 0x0a,                         // bnt 0a [ squeak ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	SIG_ADDTOOFFSET(+3),
	0x39, SIG_SELECTOR8(play),          // pushi play [ squeak ]
	SIG_ADDTOOFFSET(+28),
	0x38, SIG_SELECTOR16(cue),          // pushi cue [ no squeak ]
	SIG_END
};

static const uint16 qfg4Room643LockSqueakPatch[] = {
	0x38, PATCH_UINT16(0x00a0),         // pushi 00a0
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero x? ]
	0x1e,                               // gt? [ 160 > hero:x ]
	0x31, 0x0c,                         // bnt 0c [ right flag test ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+22), // pushi left flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x11,                         // bnt 11 [ squeak ]
	0x33, 0x2d,                         // jmp 2d [ no squeak ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+2),  // pushi right flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x05,                         // bnt 05 [ squeak ]
	0x33, 0x21,                         // jmp 21 [ no squeak ]
	PATCH_END
};

// room 644 is the same as 643 except the left and right door flags
//  were transposed, so at least the 643 signature can be reused
static const uint16 qfg4Room644LockSqueakPatch[] = {
	0x38, PATCH_UINT16(0x00a0),         // pushi 00a0
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero x? ]
	0x1e,                               // gt? [ 160 > hero:x ]
	0x31, 0x0c,                         // bnt 0c [ right flag test ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+2),  // pushi left flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x11,                         // bnt 11 [ squeak ]
	0x33, 0x2d,                         // jmp 2d [ no squeak ]
	0x78,                               // push1
	0x38, PATCH_GETORIGINALUINT16(+22), // pushi right flag
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x05,                         // bnt 05 [ squeak ]
	0x33, 0x21,                         // jmp 21 [ no squeak ]
	PATCH_END
};

// room 661 is the same as 644 but has a different instruction size in the middle
static const uint16 qfg4Room661LockSqueakSignature[] = {
	0x78,                               // push1
	0x38, SIG_UINT16(0x00e0),           // pushi 00e0 [ left flag ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is left door oiled? ]
	SIG_MAGICDWORD,
	0x31, 0x0a,                         // bnt 0a [ other door flag test ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	0x78,                               // push1
	0x39, 0x76,                         // pushi 76 [ right flag ]
	0x45, 0x04, SIG_UINT16(0x0002),     // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x0a,                         // bnt 0a [ squeak ]
	0x38, SIG_SELECTOR16(cue),          // pushi cue
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self cue: ]
	SIG_ADDTOOFFSET(+3),
	0x39, SIG_SELECTOR8(play),          // pushi play [ squeak ]
	SIG_ADDTOOFFSET(+28),
	0x38, SIG_SELECTOR16(cue),          // pushi cue [ no squeak ]
	SIG_END
};

static const uint16 qfg4Room661LockSqueakPatch[] = {
	0x38, PATCH_UINT16(0x00a0),         // pushi 00a0
	0x78,                               // push1 [ x ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero x? ]
	0x1e,                               // gt? [ 160 > hero:x ]
	0x31, 0x0c,                         // bnt 0c [ right flag test ]
	0x78,                               // push1
	0x38, PATCH_UINT16(0x00e0),         // pushi 00e0 [ left flag ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is left door oiled? ]
	0x31, 0x10,                         // bnt 10 [ squeak ]
	0x33, 0x2c,                         // jmp 2c [ no squeak ]
	0x78,                               // push1
	0x39, 0x76,                         // pushi 76 [ right flag ]
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4 02 [ is right door oiled? ]
	0x31, 0x05,                         // bnt 05 [ squeak ]
	0x33, 0x21,                         // jmp 21 [ no squeak ]
	PATCH_END
};

// When exiting the Thieves' Guild secret passage (room 340) to the town bridge
//  (room 290), hero appears in an out of bounds area on the far right of the
//  screen for 120 ticks and then abruptly teleports to the bridge secret exit.
//  This is due to not initializing hero until after the 120 tick delay, causing
//  him to be initially visible in his position from the previous room.
//
// We fix this by hiding hero initially so that he appears when emerging from
//  the secret exit under the bridge after the delay.
//
// Applies to: All versions
// Responsible method: sThiefEnter:changeState
// Fixes bug: #10774
static const uint16 qfg4BridgeSecretExitSignature[] = {
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	SIG_MAGICDWORD,
	0x31, 0x07,                         // bnt 07 [ state 1 ]
	0x35, 0x78,                         // ldi 78
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks = 120 ]
	0x32, SIG_UINT16(0x00dd),           // jmp 00dd [ end of method ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0020),           // bnt 0020 [ state 2 ]
	SIG_END
};

static const uint16 qfg4BridgeSecretExitPatch[] = {
	0x2f, 0x0c,                         // bt 0c [ state 1 ]
	0x39, PATCH_SELECTOR8(hide),        // pushi hide
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, PATCH_UINT16(0x0004),         // send 04 [ hero hide: ]
	0x35, 0x78,                         // ldi 78
	0x65, PATCH_GETORIGINALBYTE(+9),    // aTop ticks [ ticks = 120 ]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x20,                         // bnt 20 [ state 2 ]
	PATCH_END
};

// The bone cage in room 770 has two problems. Clicking Do and selecting Jump
//  Out of Cage as a thief leaves the cursor stuck. Selecting Break Cage as a
//  fighter allows the player to click during what's supposed to be a handsOff
//  script and break the game. Both bugs are due to egoTeller:sayMessage. Its
//  Jump handler is missing a call to self:clean and its Break Cage handler
//  incorrectly calls self:clean after running sBreakBones instead of before.
//
// We fix this by calling self:clean before running sBreakBones or sJumpOut.
//
// Applies to: All versions
// Responsible method: egoTeller:sayMessage
// Fixes bug: #11238
static const uint16 qfg4BoneCageTellerSignature[] = {
	0x30, SIG_UINT16(0x001b),           // bnt 001b
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x38, SIG_UINT16(0x0003),           // pushi 0003
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa sBreakBones
	0x36,                               // push
	0x76,                               // push0
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, SIG_UINT16(0x000a),           // send 0a [ rm770 setScript: sBreakBones 0 0 ]
	0x38, SIG_ADDTOOFFSET(+2),          // pushi clean
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04 [ self clean: ]
	0x32, SIG_UINT16(0x0021),           // jmp 0021 [ end of method ]
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x23,                         // ldi 23 [ "Jump Out of Cage" ]
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0010),           // bnt 0010
	SIG_END
};

static const uint16 qfg4BoneCageTellerPatch[] = {
	0x31, 0x15,                         // bnt 15
	0x38, PATCH_GETORIGINALUINT16(+21), // pushi clean
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),         // self 0004 [ self clean: ]
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x74, PATCH_GETORIGINALUINT16(+10), // lofss sBreakBones
	0x81, 0x02,                         // lag 02
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ rm770 setScript: sBreakBones ]
	0x3a,                               // toss
	0x48,                               // ret
	0x3c,                               // dup
	0x35, 0x23,                         // ldi 23 [ "Jump Out of Cage" ]
	0x1a,                               // eq?
	0x30, PATCH_UINT16(0x0017),         // bnt 0017
	0x38, PATCH_GETORIGINALUINT16(+21), // pushi clean
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),         // self 04 [ self clean: ]
	PATCH_END
};

//          script, description,                                     signature                      patch
static const SciScriptPatcherEntry qfg4Signatures[] = {
	{  true,     0, "prevent autosave from deleting save games",   1, qfg4AutosaveSignature,         qfg4AutosavePatch },
	{  true,     0, "fix inventory leaks across restarts",         1, qfg4RestartSignature,          qfg4RestartPatch },
	{  true,     1, "disable volume reset on startup",             1, sci2VolumeResetSignature,      sci2VolumeResetPatch },
	{  true,     1, "disable video benchmarking",                  1, qfg4BenchmarkSignature,        qfg4BenchmarkPatch },
	{  true,     7, "fix consecutive moonrises",                   1, qfg4MoonriseSignature,         qfg4MoonrisePatch },
	{  true,    10, "fix setLooper calls (2/2)",                   2, qfg4SetLooperSignature2,       qfg4SetLooperPatch2 },
	{  true,    11, "fix spell effect disposal",                   1, qfg4EffectDisposalSignature,   qfg4EffectDisposalPatch },
	{  true,    11, "fix trigger after summon staff (1/2)",        1, qfg4TriggerStaffSignature1,    qfg4TriggerStaffPatch1 },
	{  true,    11, "fix trigger after summon staff (2/2)",        1, qfg4TriggerStaffSignature2,    qfg4TriggerStaffPatch2 },
	{  true,    13, "fix spell effect disposal",                   1, qfg4EffectDisposalSignature,   qfg4EffectDisposalPatch },
	{  true,    28, "fix lingering rations icon after eating",     1, qfg4LeftoversSignature,        qfg4LeftoversPatch },
	{  true,    31, "fix setScaler calls",                         1, qfg4SetScalerSignature,        qfg4SetScalerPatch },
	{  true,    41, "fix conditional void calls",                  3, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,    50, "fix random revenant kopeks",                  1, qfg4SearchRevenantSignature,   qfg4SearchRevenantPatch },
	{  true,    51, "Floppy: fix ad avis capture lockup",          1, qfg4AdAvisCaptureSignature,    qfg4AdAvisCapturePatch },
	{  true,    51, "fix necrotaur blackout",                      1, qfg4NecrotaurBlackoutSignature,qfg4NecrotaurBlackoutPatch },
	{  true,    51, "CD: fix necrotaur capture",                   3, qfg4NecrotaurCaptureSignature, qfg4NecrotaurCapturePatch },
	{  true,    53, "NRS: fix wraith lockup",                      1, qfg4WraithLockupNrsSignature,  qfg4WraithLockupNrsPatch },
	{  true,    83, "fix incorrect array type",                    1, qfg4TrapArrayTypeSignature,    qfg4TrapArrayTypePatch },
	{  true,   140, "fix character selection",                     1, qfg4CharacterSelectSignature,  qfg4CharacterSelectPatch },
	{  true,   250, "fix hectapus death lockup",                   1, qfg4HectapusDeathSignature,    qfg4HectapusDeathPatch },
	{  true,   260, "CD: fix inn door crash",                      1, qfg4InnDoorCDSignature,        qfg4InnDoorCDPatch },
	{  true,   270, "fix town gate after a staff dream",           1, qfg4DreamGateSignature,        qfg4DreamGatePatch },
	{  true,   270, "fix town gate doormat at night",              1, qfg4TownGateDoormatSignature,  qfg4TownGateDoormatPatch },
	{  true,   290, "fix chase repeating",                         1, qfg4ChaseRepeatsSignature,     qfg4ChaseRepeatsPatch },
	{  true,   290, "fix bridge secret exit",                      1, qfg4BridgeSecretExitSignature, qfg4BridgeSecretExitPatch },
	{  true,   300, "fix empty burgomeister room teller",          1, qfg4EmptyBurgoRoomSignature,   qfg4EmptyBurgoRoomPatch },
	{  true,   320, "fix pathfinding at the inn",                  1, qfg4InnPathfindingSignature,   qfg4InnPathfindingPatch },
	{  true,   320, "fix talking to absent innkeeper",             1, qfg4AbsentInnkeeperSignature,  qfg4AbsentInnkeeperPatch },
	{  true,   320, "CD: fix domovoi never appearing",             1, qfg4DomovoiInnSignature,       qfg4DomovoiInnPatch },
	{  true,   324, "CD: fix domovoi never appearing",             1, qfg4DomovoiInnSignature,       qfg4DomovoiInnPatch },
	{  true,   340, "CD/Floppy: fix guild tunnel access (1/3)",    1, qfg4GuildWalkSignature1,       qfg4GuildWalkPatch1 },
	{  true,   340, "CD/Floppy: fix guild tunnel access (2/3)",    1, qfg4GuildWalkSignature2,       qfg4GuildWalkPatch2 },
	{  false,  340, "CD: fix guild tunnel access (3/3)",           1, qfg4GuildWalkCDSignature3,     qfg4GuildWalkCDPatch3 },
	{  false,  340, "Floppy: fix guild tunnel access (3/3)",       1, qfg4GuildWalkFloppySignature3, qfg4GuildWalkFloppyPatch3 },
	{  true,   440, "fix setLooper calls (1/2)",                   1, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   475, "fix tarot 3 queen card",                      1, qfg4Tarot3QueenSignature,      qfg4Tarot3QueenPatch },
	{  true,   475, "fix tarot 3 death card",                      1, qfg4Tarot3DeathSignature,      qfg4Tarot3DeathPatch },
	{  true,   475, "fix tarot 3 two of cups placement",           1, qfg4Tarot3TwoOfCupsSignature,  qfg4Tarot3TwoOfCupsPatch },
	{  true,   475, "fix tarot 3 card priority",                   1, qfg4Tarot3PrioritySignature,   qfg4Tarot3PriorityPatch },
	{  true,   475, "fix tarot 5 card priority",                   1, qfg4Tarot5PrioritySignature,   qfg4Tarot5PriorityPatch },
	{  false,  500, "CD: fix rope during Igor rescue (1/2)",       1, qfg4GraveyardRopeSignature1,   qfg4GraveyardRopePatch1 },
	{  false,  500, "CD: fix rope during Igor rescue (2/2)",       1, qfg4GraveyardRopeSignature2,   qfg4GraveyardRopePatch2 },
	{  true,   530, "fix setLooper calls (1/2)",                   4, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   535, "fix setLooper calls (1/2)",                   4, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   541, "fix setLooper calls (1/2)",                   5, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   542, "fix setLooper calls (1/2)",                   5, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   543, "fix setLooper calls (1/2)",                   5, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   545, "fix setLooper calls (1/2)",                   5, qfg4SetLooperSignature1,       qfg4SetLooperPatch1 },
	{  true,   557, "fix forest 557 entry from east",              1, qfg4Forest557PathfindingSignature, qfg4Forest557PathfindingPatch },
	{  true,   600, "fix passable closed gate after geas",         1, qfg4DungeonGateSignature,      qfg4DungeonGatePatch },
	{  true,   600, "fix gate options after geas",                 1, qfg4GateOptionsSignature,      qfg4GateOptionsPatch },
	{  true,   600, "fix paladin's necrotaur message",             1, qfg4NecrotaurMessageSignature, qfg4NecrotaurMessagePatch },
	{  true,   630, "fix great hall entry from barrel room",       1, qfg4GreatHallEntrySignature,   qfg4GreatHallEntryPatch },
	{  true,   633, "fix stairway pathfinding",                    1, qfg4StairwayPathfindingSignature, qfg4StairwayPathfindingPatch },
	{  true,   633, "Floppy: fix argument message",                1, qfg4ArgumentMessageFloppySignature,  qfg4ArgumentMessageFloppyPatch },
	{  true,   633, "fix room 627 door options",                   1, qfg4Room627DoorOptionsSignature, qfg4Room627DoorOptionsPatch },
	{  true,   633, "fix room 627 door responses (1/2)",           1, qfg4Room627DoorResponsesSignature1, qfg4Room627DoorResponsesPatch1 },
	{  true,   633, "Floppy: fix room 627 door responses (2/2)",   1, qfg4Room627DoorResponsesFloppySignature2, qfg4Room627DoorResponsesFloppyPatch2 },
	{  true,   633, "CD: fix room 627 door responses (2/2)",       1, qfg4Room627DoorResponsesCDSignature2, qfg4Room627DoorResponsesCDPatch2 },
	{  true,   633, "Floppy: fix room 627 door squeak",            1, qfg4Room627SqueakFloppySignature,  qfg4Room627SqueakFloppyPatch },
	{  true,   633, "CD: fix room 627 door squeak",                2, qfg4Room627SqueakCDSignature,  qfg4Room627SqueakCDPatch },
	{  true,   633, "fix great hall keyhole message",              1, qfg4GreatHallKeyholeSignature, qfg4GreatHallKeyholePatch },
	{  true,   634, "fix room 631 pick lock squeak",               1, qfg4Room631LockSqueakSignature,qfg4Room631LockSqueakPatch },
	{  true,   640, "fix room 640 pick lock squeak",               1, qfg4Room640LockSqueakSignature,qfg4Room640LockSqueakPatch },
	{  true,   642, "fix room 642 pick lock squeak",               1, qfg4Room640LockSqueakSignature,qfg4Room640LockSqueakPatch },
	{  true,   643, "fix room 643 pick lock squeak",               1, qfg4Room643LockSqueakSignature,qfg4Room643LockSqueakPatch },
	{  true,   643, "fix iron safe's east door sending hero west", 1, qfg4SafeDoorEastSignature,     qfg4SafeDoorEastPatch },
	{  true,   643, "fix iron safe's door oil flags",              1, qfg4SafeDoorOilSignature,      qfg4SafeDoorOilPatch },
	{  true,   644, "fix castle door open message for rogue",      2, qfg4StuckDoorSignature,        qfg4StuckDoorPatch },
	{  true,   644, "fix peer bats, lower door",                   1, qfg4LowerPeerBatsSignature,    qfg4LowerPeerBatsPatch },
	{  true,   644, "fix room 644 pick lock squeak",               1, qfg4Room643LockSqueakSignature,qfg4Room644LockSqueakPatch },
	{  true,   645, "fix extraneous door sound in the castle",     1, qfg4DoubleDoorSoundSignature,  qfg4DoubleDoorSoundPatch },
	{  true,   661, "fix room 661 pick lock squeak",               1, qfg4Room661LockSqueakSignature,qfg4Room661LockSqueakPatch },
	{  false,  663, "CD: fix crest bookshelf",                     1, qfg4CrestBookshelfCDSignature,     qfg4CrestBookshelfCDPatch },
	{  false,  663, "Floppy: fix crest bookshelf",                 1, qfg4CrestBookshelfFloppySignature, qfg4CrestBookshelfFloppyPatch },
	{  true,   663, "CD/Floppy: fix crest bookshelf motion",       1, qfg4CrestBookshelfMotionSignature, qfg4CrestBookshelfMotionPatch },
	{  true,   663, "CD/Floppy: fix peer bats, upper door (1/2)",  1, qfg4UpperPeerBatsSignature1,       qfg4UpperPeerBatsPatch1 },
	{  false,  663, "CD: fix peer bats, upper door (2/2)",         1, qfg4UpperPeerBatsCDSignature2,     qfg4UpperPeerBatsCDPatch2 },
	{  false,  663, "Floppy: fix peer bats, upper door (2/2)",     1, qfg4UpperPeerBatsFloppySignature2, qfg4UpperPeerBatsFloppyPatch2 },
	{  true,   670, "fix look dungeon message",                    1, qfg4LookDungeonSignature,      qfg4LookDungeonPatch },
	{  true,   710, "fix tentacle wriggle cycler",                 1, qfg4TentacleWriggleSignature,  qfg4TentacleWrigglePatch },
	{  true,   710, "fix tentacle retraction for fighter",         1, qfg4PitRopeFighterSignature,   qfg4PitRopeFighterPatch },
	{  true,   710, "fix tentacle retraction for mage (1/2)",      1, qfg4PitRopeMageSignature1,     qfg4PitRopeMagePatch1 },
	{  true,   710, "NRS: fix tentacle retraction for mage (1/2)", 1, qfg4PitRopeMageNrsSignature1,  qfg4PitRopeMageNrsPatch1 },
	{  true,   710, "fix tentacle retraction for mage (2/2)",      1, qfg4PitRopeMageSignature2,     qfg4PitRopeMagePatch2 },
	{  true,   730, "fix ad avis timeout",                         1, qfg4AdAvisTimeoutSignature,    qfg4AdAvisTimeoutPatch },
	{  true,   730, "Floppy: fix casting spells at ad avis",       1, qfg4AdAvisSpellsFloppySignature, qfg4AdAvisSpellsFloppyPatch },
	{  true,   730, "CD: fix casting spells at ad avis",           2, qfg4AdAvisSpellsCDSignature,   qfg4AdAvisSpellsCDPatch },
	{  true,   730, "NRS: fix casting spells at ad avis",          2, qfg4AdAvisSpellsNrsSignature,  qfg4AdAvisSpellsNrsPatch },
	{  true,   730, "fix casting last spell at ad avis",           1, qfg4AdAdvisLastSpellSignature, qfg4AdAdvisLastSpellPatch },
	{  true,   730, "fix ad avis projectile message",              1, qfg4AdAvisMessageSignature,    qfg4AdAvisMessagePatch },
	{  true,   730, "fix throwing weapons at ad avis",             1, qfg4AdAvisThrowWeaponSignature,qfg4AdAvisThrowWeaponPatch },
	{  true,   730, "fix fighter's spear animation",               1, qfg4FighterSpearSignature,     qfg4FighterSpearPatch },
	{  true,   770, "fix bone cage teller",                        1, qfg4BoneCageTellerSignature,   qfg4BoneCageTellerPatch },
	{  true,   800, "fix setScaler calls",                         1, qfg4SetScalerSignature,        qfg4SetScalerPatch },
	{  true,   800, "fix grapnel removing hero's scaler",          1, qfg4RopeScalerSignature,       qfg4RopeScalerPatch },
	{  true,   801, "fix runes puzzle (1/2)",                      1, qfg4RunesPuzzleSignature1,     qfg4RunesPuzzlePatch1 },
	{  true,   801, "fix runes puzzle (2/2)",                      1, qfg4RunesPuzzleSignature2,     qfg4RunesPuzzlePatch2 },
	{  true,   803, "CD: fix sliding down slope",                  1, qfg4SlidingDownSlopeCDSignature, qfg4SlidingDownSlopeCDPatch },
	{  true,   803, "CD: fix walking up slippery slope",           1, qfg4WalkUpSlopeCDSignature,    qfg4WalkUpSlopeCDPatch },
	{  true,   803, "CD: fix walking down slippery slope",         1, qfg4WalkDownSlopeCDSignature,  qfg4WalkDownSlopeCDPatch },
	{  true,   803, "NRS: fix walking down slippery slope",        1, qfg4WalkDownSlopeNrsSignature, qfg4WalkDownSlopeNrsPatch },
	{  true,   820, "fix rabbit combat",                           1, qfg4RabbitCombatSignature,     qfg4RabbitCombatPatch },
	{  true,   810, "fix conditional void calls",                  1, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   830, "fix conditional void calls",                  2, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   835, "fix conditional void calls",                  3, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   840, "fix conditional void calls",                  2, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   855, "fix conditional void calls",                  1, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   870, "fix conditional void calls",                  5, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,    59, "GloryTalker lockup fix",                      1, sciNarratorLockupSignature,    sciNarratorLockupPatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,    sciNarratorLockupPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,        sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,        sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,        sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// Space Quest 3 has some strings hard coded in the scripts file
// We need to patch them for the Hebrew translation

// Replace "Enter input" prompt with Hebrew
static const uint16 sq3HebrewEnterInputSignature[] = {
	SIG_MAGICDWORD,
	0x45, 0x6e, 0x74, 0x65, 0x72, 0x20, 0x69, 0x6e, 0x70, 0x75, 0x74, 0,
	SIG_END
};

static const uint16 sq3HebrewEnterInputPatch[] = {
	0xe4, 0xf7, 0xf9, 0x20, 0xf4, 0xf7, 0xe5, 0xe3, 0xe4, 0x3a, 0,
	PATCH_END
};

// Replace "Space Quest ]I[" in status bar with Hebrew
static const uint16 sq3HebrewStatusBarNameSignature[] = {
	SIG_MAGICDWORD,
	0x53, 0x70, 0x61, 0x63, 0x65, 0x20, 0x51, 0x75, 0x65, 0x73, 0x74, 0x20, 0x0b,		// "Space Quest " + special ]I[ char
	SIG_END
};

static const uint16 sq3HebrewStatusBarNamePatch[] = {
	0xee, 0xf1, 0xf2, 0x20, 0xe1, 0xe7, 0xec, 0xec, 0x20, 0x0b, 0x20, 0x20, 0x20,		// 'Space Quest' in Hebrew: 'Masa Bahalal ' + special ]I[ char
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry sq3Signatures[] = {
	{  false,   0, "Hebrew: Replace name in status bar",    1, sq3HebrewStatusBarNameSignature,                     sq3HebrewStatusBarNamePatch },
	{  false, 996, "Hebrew: Replace 'Enter input' prompt",  1, sq3HebrewEnterInputSignature,                        sq3HebrewEnterInputPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};



// ===========================================================================
//  script 298 of sq4/floppy has an issue. object "nest" uses another property
//   which isn't included in property count. We return 0 in that case, so that
//   the game does not increment nest::x. The problem is that the script also
//   checks if x exceeds nest::x. We never reach that of course, so the
//   incorrect property means that the pterodactyl flight will continue
//	 endlessly. We could either calculate the property count differently,
//   thereby fixing this bug, but I think that just patching it out is cleaner.
// Fixes bug: #5093
static const uint16 sq4FloppySignatureEndlessFlight[] = {
	0x39, 0x04,                         // pushi 04 (selector x)
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x67, 0x08,                         // pTos 08 (property x)
	0x63, SIG_ADDTOOFFSET(+1),          // pToa (invalid property) - 44h for English floppy, 4ch for German floppy
	0x02,                               // add
	SIG_END
};

static const uint16 sq4FloppyPatchEndlessFlight[] = {
	PATCH_ADDTOOFFSET(+5),
	0x35, 0x03,                         // ldi 03 (which would be the content of the property)
	PATCH_END
};

// Floppy-only: When the player tries to throw something at the sequel police
//   in Space Quest X (zero g zone), the game will first show a textbox and
//   then cause a signature mismatch in ScummVM. In Sierra SCI, it'd crash the
//   whole game - or, when the Sierra "patch" got applied, display garbage.
//
// All of this is caused by a typo in the script. Right after the code for
//  showing the textbox, there is similar code for showing another textbox, but
//  without a pointer to the text. This has to be a typo, because there is no
//  unused text to be found within that script.
//
// Sierra's "patch" didn't include a proper fix (as in a modified script).
//  Instead they shipped a dummy text resource, which somewhat "solved" the
//  issue in Sierra SCI, but it still showed another textbox with garbage in
//  it. Funnily Sierra must have known that, because that new text resource
//  contains: "Hi! This is a kludge!"
//
// A copy of this script exists in the arcade when the sequel police arrive, but
//  it has an additional typo that adds another broken function call to the ATM
//  card message. Originally these arcade bugs couldn't be triggered because the
//  police didn't respond to clicks due to their description property not being
//  set. This Feature behavior was changed in later versions, exposing the bug
//  on Mac and Amiga until the script was eventually rewritten for localization.
//
// We properly fix it by removing the faulty code from both scripts, preventing
//  crashes in all versions, and by setting an arbitrary sp2:description in
//  English PC floppy so that the arcade police respond to clicks as intended.
//
// Applies to: English PC Floppy, English Mac Floppy, English Amiga Floppy
// Responsible methods: sp1::doVerb, heap in script 376
// Fixes bug: found by SCI developer
static const uint16 sq4FloppySignatureThrowStuffAtSequelPolice[] = {
	0x47, 0xff, 0x00, 0x02,             // calle [export 0 of script 255], 2
	0x3a,                               // toss
	SIG_MAGICDWORD,
	0x36,                               // push
	0x47, 0xff, 0x00, 0x02,             // calle [export 0 of script 255], 2
	SIG_END
};

static const uint16 sq4FloppyPatchThrowStuffAtSequelPolice[] = {
	PATCH_ADDTOOFFSET(+5),
	0x32, PATCH_UINT16(0x0002),         // jmp 0002
	PATCH_END
};

static const uint16 sq4FloppySignatureClickAtmCardOnSequelPolice[] = {
	0x47, 0xff, 0x00, 0x02,             // calle [export 0 of script 255], 2
	SIG_MAGICDWORD,
	0x36,                               // push
	0x47, 0xff, 0x00, 0x02,             // calle [export 0 of script 255], 2
	SIG_END
};

static const uint16 sq4FloppyPatchClickAtmCardOnSequelPolice[] = {
	PATCH_ADDTOOFFSET(+4),
	0x32, PATCH_UINT16(0x0002),         // jmp 0002
	PATCH_END
};

// set an arbitrary sp2:description so that sp2:doVerb can run.
//  the description isn't used, it just has to be non-zero.
static const uint16 sq4FloppySignatureSequelPoliceDescription[] = {
	SIG_UINT16(0x0000),                 // description = 0
	SIG_UINT16(0x005a),                 // sighAngle = 90
	SIG_UINT16(0x6789),                 // actions = 26505
	SIG_UINT16(0x6789),                 // onMeCheck = 26505
	SIG_UINT16(0x0000),                 // lookStr = 0
	SIG_UINT16(0x0002),                 // yStep = 2
	SIG_MAGICDWORD,
	SIG_UINT16(0x0179),                 // view = 377
	SIG_UINT16(0x0004),                 // view = 4
	SIG_END
};

static const uint16 sq4FloppyPatchSequelPoliceDescription[] = {
	PATCH_UINT16(0x2b21),               // description = "It's one of Vohaul's Sequel Policemen!"
	PATCH_END
};

// Right at the start of Space Quest 4 CD, when walking up in the first room, ego will
//  immediately walk down just after entering the upper room.
//
// This is caused by the scripts setting ego's vertical coordinate to 189 (BDh), which is the
//  trigger in rooms to walk to the room below it. Sometimes this isn't triggered, because
//  the scripts also initiate a motion to vertical coordinate 188 (BCh). When you lower the game's speed,
//  this bug normally always triggers. And it triggers of course also in the original interpreter.
//
// It doesn't happen in PC floppy, because nsRect is not the same as in CD.
//
// We fix it by setting ego's vertical coordinate to 188 and we also initiate a motion to 187.
//
// Applies to at least: English PC CD
// Responsible method: rm045::doit
// Fixes bug: #5468
static const uint16 sq4CdSignatureWalkInFromBelowRoom45[] = {
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x00bd),           // pushi 00BDh
	0x38, SIG_ADDTOOFFSET(+2),          // pushi [setMotion selector]
	0x39, 0x03,                         // pushi 3
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 04 -> get ego::x
	0x36,                               // push
	0x38, SIG_UINT16(0x00bc),           // pushi 00BCh
	SIG_END
};

static const uint16 sq4CdPatchWalkInFromBelowRoom45[] = {
	PATCH_ADDTOOFFSET(+2),
	0x38, PATCH_UINT16(0x00bc),         // pushi 00BCh
	PATCH_ADDTOOFFSET(+15),
	0x38, PATCH_UINT16(0x00bb),         // pushi 00BBh
	PATCH_END
};

// Sierra seems to have forgotten to include code to play the audio
// describing the "Universal Remote Control" inside the "Hz So good" store.
//
// We add it, so that the audio is now playing.
//
// Applies to at least: English PC CD
// Responsible method: doCatalog::changeState(1Ch) in script 391
// Implements enhancement: #10227
static const uint16 sq4CdSignatureMissingAudioUniversalRemote[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x00b1),           // bnt [skip over state 1Ch code]
	0x35, 0x1c,                         // ldi 1Ch
	0x39, 0x0c,                         // pushi 0Ch
	SIG_ADDTOOFFSET(+127),              // skip over to substate 1 code of state 1Ch code
	0x32, SIG_UINT16(0x0028),           // jmp [to toss/ret, substate 0-code]
	// substate 1 code
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0021),           // bnt [skip over substate 1 code]
	0x39, SIG_SELECTOR8(number),        // pushi number
	0x78,                               // push1
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0188),           // pushi 188h
	0x38, SIG_UINT16(0x018b),           // pushi 18Bh
	0x43, 0x3c, 0x04,                   // callk Random, 4
	0x36,                               // push
	0x39, SIG_SELECTOR8(play),          // pushi play
	0x76,                               // push0
	0x81, 0x64,                         // lag global[64h]
	0x4a, 0x0a,                         // send 0Ah
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0x0488),           // lofsa startTerminal
	0x36,                               // push
	0x63, 0x12,                         // pToa client
	0x4a, 0x06,                         // send 06
	0x3a,                               // toss
	0x33, 0x14,                         // jmp [to toss/ret]
	// state 1Dh, called when returning back to main menu from catalog sub menu
	0x3c,                               // dup
	0x35, 0x1d,                         // ldi 1Dh
	0x1a,                               // eq?
	0x31, 0x0e,                         // bnt [skip last state and toss/ret]
	0x35, 0x1d,                         // ldi 1Dh
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0x0488),           // lofsa startTerminal
	0x36,                               // push
	0x63, 0x12,                         // pToa client
	0x4a, 0x06,                         // send 06
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4CdPatchMissingAudioUniversalRemote[] = {
	0x30, PATCH_UINT16(0x00b7),         // bnt [directly to last state code, saving 6 bytes]
	0x32, PATCH_UINT16(0x009a),         // jmp 154d [to our new code]
	// 1 not used byte here
	PATCH_ADDTOOFFSET(+128),            // skip over to substate 1 code of state 1Ch code
	0x32, PATCH_UINT16(0x003f),         // substate 0 code, jumping to toss/ret
	// directly start with substate 1 code, saving 7 bytes
	0x39, PATCH_SELECTOR8(number),      // pushi number
	0x78,                               // push1
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0188),         // pushi 188h
	0x38, PATCH_UINT16(0x018b),         // pushi 18Bh
	0x43, 0x3c, 0x04,                   // callk Random, 4
	0x36,                               // push
	0x39, PATCH_SELECTOR8(play),        // pushi play
	0x76,                               // push0
	0x81, 0x64,                         // lag global[64h]
	0x4a, 0x0a,                         // send 0Ah
	0x33, 0x1a,                         // jmp [state 1Dh directly, saving 12 bytes]
	// additional code for playing missing audio (18 bytes w/o jmp back)
	0x89, 0x5a,                         // lsg global[5Ah]
	0x35, 0x01,                         // ldi 1
	0x1c,                               // ne?
	0x31, 0x0b,                         // bnt [skip play audio]
	0x38, PATCH_SELECTOR16(say),        // pushi say (0123h)
	0x78,                               // push1
	0x39, 0x14,                         // pushi 14h
	0x72, PATCH_UINT16(0x0850),         // lofsa newRob
	0x4a, 0x06,                         // send 06
	// now get back
	0x39, 0x0c,                         // pushi 0Ch
	0x32, PATCH_UINT16(0xff50),         // jmp back
	PATCH_END
};

// It seems that Sierra forgot to set a script flag when cleaning out the bank
// account in Space Quest 4 CD. This was probably caused by the whole bank
// account interaction getting a rewrite and polish in the CD version.
//
// Because of this bug, points for changing back clothes will not get awarded,
// which makes it impossible to get a perfect point score in the CD version of
// the game. The points are awarded by rm371::doit in script 371.
//
// We fix this. PC floppy does not have this bug.
//
// Note: Some Let's Plays on YouTube show points are in fact awarded. But those
//  Let's Plays were of a hacked Space Quest 4 version. It was part Floppy,
//  part CD version. We consider it to be effectively pirated and not a
//  canonical CD version of Space Quest 4. It's easy to identify for having
//  both voices and a store called "Radio Shock" instead of "Hz. So Good".
//
// Applies to at least: English PC CD
// Responsible method: but2Script::changeState(2)
// Fixes bug: #6866
static const uint16 sq4CdSignatureGetPointsForChangingBackClothes[] = {
	0x35, 0x02,                         // ldi 02
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x006a),           // bnt [state 3]
	0x76,
	SIG_ADDTOOFFSET(+46),               // jump over "withdraw funds" code
	0x33, 0x33,                         // jmp [end of state 2, set cycles code]
	SIG_ADDTOOFFSET(+51),               // jump over "clean bank account" code
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x33, 0x0b,                         // jmp [toss/ret]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt [toss/ret]
	SIG_END
};

static const uint16 sq4CdPatchGetPointsForChangingBackClothes[] = {
	PATCH_ADDTOOFFSET(+3),
	0x30, PATCH_UINT16(0x0070),         // bnt [state 3]
	PATCH_ADDTOOFFSET(+47),             // "withdraw funds" code
	0x33, 0x39,                         // jmp [end of state 2, set cycles code]
	PATCH_ADDTOOFFSET(+51),
	0x78,                               // push1
	0x39, 0x1d,                         // pushi 1Dh
	0x45, 0x07, 0x02,                   // callb [export 7 of script 0], 02 (set flag 1Dh - located at global[73h] bit 2)
	0x35, 0x02,                         // ldi 02
	0x65, 0x1c,                         // aTop cycles
	0x33, 0x05,                         // jmp [toss/ret]
	// check for state 3 code removed to save 6 bytes
	PATCH_END
};

// The English Amiga version contains curious changes to the dress logic in
//  Sock's which break the game and weren't included in later versions:
//
// 1. Purchasing the dress is recorded in flag 90 instead of mall:rFlag3
// 2. Flag 90 is cleared when changing clothes after clearing out the ATM
//
// Game flags are global while mall flags are reset upon leaving the mall, which
//  makes this look like a bug fix, but Sock's is closed when returning so this
//  shouldn't change game logic. Unfortunately Sierra forgot to update the other
//  scripts which query mall:rFlag3 and so they never see the dress purchase.
//  This creates scenarios where exiting Sock's (room 371) after paying causes
//  room 370 to kick Roger out for not paying, preventing game completion.
//
// Clearing the dress-purchase flag has no effect other than to allow purchasing
//  the dress a second time as if it never happened, leaving the player without
//  enough money to complete the game, having paid for the dress twice.
//
// We fix both bugs by updating the mall scripts so that they all test flag 90
//  and never clear it. It's possible the flag change was an optimization, which
//  many Amiga tweaks are, since it eliminated a message send in rm371:doit.
//
// Applies to: English Amiga Floppy
// Responsible methods: rm371:doit, rm370:init, warningScript:changeState(1)
// Fixes bug #11004
static const uint16 sq4AmigaSignatureDressPurchaseFlagClear[] = {
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x39, 0x5a,                         // pushi 5a
	0x45, 0x08, 0x02,                   // callb proc0_8 02 [ clear flag 90 ]
	SIG_END
};

static const uint16 sq4AmigaPatchDressPurchaseFlagClear[] = {
	0x32, PATCH_UINT16(0x0003),         // jmp 0003 [ don't clear flag 90 ]
	PATCH_END
};

static const uint16 sq4AmigaSignatureDressPurchaseFlagCheck[] = {
	SIG_MAGICDWORD,
	0x36,                               // push [ mall ]
	0x38, SIG_UINT16(0x0228),           // pushi rFlag3
	0x39, 0x04,                         // pushi 04
	0x46, SIG_UINT16(0x02bc),           // calle proc700_3 06 [ is mall:rFlag3 flag 4 set? ]
	      SIG_UINT16(0x0003), 0x06,
	SIG_END
};

static const uint16 sq4AmigaPatchDressPurchaseFlagCheck[] = {
	0x78,                               // push1
	0x39, 0x5a,                         // pushi 5a
	0x45, 0x06, 0x02,                   // callb proc0_6 02 [ is flag 90 set? ]
	0x32, PATCH_UINT16(0x0003),         // jmp 0003
	PATCH_END
};

// The Big And Tall store (room 381) doesn't display its Look message in the CD
//  version. We add the missing super:doVerb call to theStore:doVerb.
//
// Applies to: English PC CD
// Responsible method: theStore:doVerb
static const uint16 sq4CdSignatureBigAndTallDescription[] = {
	0x3c,                               // dup
	0x35, 0x06,                         // ldi 06
	0x1a,                               // eq? [ verb == smell ]
	0x30, SIG_UINT16(0x0013),           // bnt 0013
	0x38, SIG_SELECTOR16(modNum),       // pushi modNum [ redundant when set to room number ]
	0x78,                               // push1
	0x38, SIG_UINT16(0x017d),           // pushi 017d
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0a
	0x81, 0x59,                         // lag 59
	0x4a, 0x0c,                         // send 0c [ sq4GlobalNarrator modNum: 381 say: 10 ]
	SIG_MAGICDWORD,
	0x33, 0x02,                         // jmp 02
	0x35, 0x00,                         // ldi 00
	0x3a,                               // toss
	SIG_END
};

static const uint16 sq4CdPatchBigAndTallDescription[] = {
	0x35, 0x06,                         // ldi 06
	0x1a,                               // eq? [ verb == smell ]
	0x31, 0x0a,                         // bnt 0a
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0a
	0x81, 0x59,                         // lag 59
	0x4a, 0x06,                         // send 06 [ sq4GlobalNarrator say: 10 ]
	0x87, 0x01,                         // lap 01
	0x78,                               // push1
	0x1a,                               // eq? [ verb == look ]
	0x31, 0x08,                         // bnt 08
	0x38, PATCH_SELECTOR16(doVerb),     // pushi doVerb
	0x78,                               // push1
	0x78,                               // push1
	0x57, 0x7a, 0x06,                   // super Sq4Feature 06 [ super doVerb: 1 ]
	PATCH_END
};

// Clicking Do on the Monolith Burger door responds with the message for looking
//  at the boss and clicking Taste responds with the taste message for the bush.
//  The verb handler is missing the modNum for both and also sets the wrong cond
//  for the second.
//
// Applies to: English PC CD
// Responsible method: door:doVerb(4)
// Fixes bug #10976
static const uint16 sq4CdSignatureMonolithBurgerDoor[] = {
	SIG_MAGICDWORD,
	0x31, 0x13,                         // bnt 13
	0x38, SIG_SELECTOR16(modNum),       // pushi modNum
	0x78,                               // push1
	0x38, SIG_UINT16(0x0177),           // pushi 0177
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	0x78,                               // push1
	0x81, 0x59,                         // lag 59
	0x4a, 0x0c,                         // send 0c [ Sq4GlobalNarrator modNum 375: say: 1 ]
	SIG_ADDTOOFFSET(+9),
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	0x7a,                               // push2
	0x81, 0x59,                         // lag 59
	0x4a, 0x06,                         // send 06 [ Sq4GlobalNarrator say: 2 ]
	SIG_ADDTOOFFSET(+25),
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	0x39, 0x0b,                         // pushi 0b
	0x81, 0x59,                         // lag 59
	0x4a, 0x06,                         // send 06 [ Sq4GlobalNarrator say: 11 ]
	SIG_END
};

static const uint16 sq4CdPatchMonolithBurgerDoor[] = {
	PATCH_ADDTOOFFSET(+13),
	0x36,                               // push
	PATCH_ADDTOOFFSET(+13),
	0x35, 0x02,                         // ldi 02
	0x33, 0xe3,                         // jmp e3 [ Sq4GlobalNarrator modNum 375: say: 2 ]
	PATCH_ADDTOOFFSET(+30),
	0x35, 0x04,                         // ldi 04
	0x33, 0xc1,                         // jmp c1 [ Sq4GlobalNarrator modNum 375: say: 4 ]
	PATCH_END
};

// For Space Quest 4 CD, Sierra added a pick up animation for Roger when he
// picks up the rope.
//
// When the player is detected by the zombie right at the start of the game,
// while picking up the rope, scripts bomb out.
//
// This is caused by code intended to make Roger face the arriving drone. We
// fix it by checking if ego::cycler is actually set before calling that code.
//
// Applies to at least: English PC CD
// Responsible method: droidShoots::changeState(3)
// Fixes bug: #6076
static const uint16 sq4CdSignatureGettingShotWhileGettingRope[] = {
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x02fa),           // jmp [end]
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x0b,                         // bnt [state 3 check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb [export 2 of script 0], 00 (disable controls)
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x02e9),           // jmp [end]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x1e,                         // bnt [state 4 check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb [export 2 of script 0], 00 (disable controls again??)
	0x7a,                               // push2
	0x89, 0x00,                         // lsg global[0]
	0x72, SIG_UINT16(0x0242),           // lofsa deathDroid
	0x36,                               // push
	0x45, 0x0d, 0x04,                   // callb [export 13 of script 0], 04 (set heading of ego to face droid)
	SIG_END
};

static const uint16 sq4CdPatchGettingShotWhileGettingRope[] = {
	PATCH_ADDTOOFFSET(+11),
	// this makes state 2 only do the 2 cycles wait, controls should always be disabled already at this point
	0x2f, 0xf3,                         // bt [previous state aTop cycles code]
	// Now we check for state 3, this change saves us 11 bytes
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x29,                         // bnt [state 4 check]
	// new state 3 code
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb [export 2 of script 0], 00 (disable controls, actually not needed)
	0x38, PATCH_SELECTOR16(cycler),     // pushi cycler
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 04 (get ego::cycler)
	0x30, PATCH_UINT16(0x000a),         // bnt [skip the heading call]
	PATCH_END
};

// During the SQ4 introduction logo, EGA versions increase the number of calls
//  to kPaletteAnimate by 40x. This was probably to achieve the same delay as
//  VGA even though no palette animation occurred. This adjustment interferes
//  with our kPaletteAnimate speed throttling for SQ4 scripts such as this, bug
//  #6057. We remove the EGA delay, making all versions consistent, otherwise
//  the logo is displayed for over 3 minutes instead of 5 seconds.
//
// Applies to: English PC EGA Floppy, Japanese PC-98
// Responsible method: rmScript:changeState
// Fixes bug #6193
static const uint16 sq4SignatureEgaIntroDelay[] = {
	SIG_MAGICDWORD,
	0x89, 0x69,                         // lsg 69 [ system colors ]
	0x35, 0x10,                         // ldi 10
	0x1e,                               // gt?    [ system colors > 16 ]
	0x30,                               // bnt    [ use EGA delay ]
	SIG_END
};

static const uint16 sq4PatchEgaIntroDelay[] = {
	0x33, 0x06,                         // jmp 06 [ don't use EGA delay ]
	PATCH_END
};

// Talking to the red shopper in the mall has a 5% chance of a funny message but
//  this script is broken in the CD version. After the first time the wrong
//  message tuple is attempted by the narrator as its modNum value is cleared.
//  The message text is also accidentally repeated in a message box.
//
// We fix this by specifying the modNum when saying the message and removing
//  the erroneous message box call.
//
// Applies to: English PC CD
// Responsible method: shopper3:doVerb(2)
// Fixes bug #10911
static const uint16 sq4CdSignatureRedShopperMessageFix[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x72, SIG_UINT16(0x057a),           // lofsa wierdNar
	0x4a, 0x06,                         // send 06 [ wierdNar say: 1 ]
	0x78,                               // push1
	0x72, SIG_UINT16(0x0660),           // lofsa "Mr. Carlos sent me..."
	0x36,                               // push
	0x46, SIG_UINT16(0x0399),           // calle proc921_0 [ message box ]
	      SIG_UINT16(0x0000), 0x02,
	SIG_END
};

static const uint16 sq4CdPatchRedShopperMessageFix[] = {
	0x38, PATCH_SELECTOR16(modNum),     // pushi modNum
	0x38, PATCH_UINT16(0x0001),         // pushi 0001
	0x38, PATCH_UINT16(0x02bc),         // pushi 02bc
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x39, 0x01,                         // pushi 01
	0x39, 0x01,                         // pushi 01
	0x72, PATCH_UINT16(0x057a),         // lofsa wierdNar
	0x4a, 0x0c,                         // send 0c [ wierdNar modNum: 700 say: 1 ]
	PATCH_END
};

// When swimming in zero gravity in the mall, the game can lock up if the Sequel
//  Police shoot while ego swims past the edge of the screen.
//
// When the Sequel Police shoot, the object "blast" animates near ego. blast is
//  an obstacle that ego can collide with. If ego is shot at while going beyond
//  the edge of the screen and blast is in the right position then stayInScript
//  can lock up due to ego getting stuck on the invisible blast object and never
//  reaching his destination.
//
// We fix this by setting blast's ignore-actors flag so that ego can't collide
//  with it and get stuck. This does not affect whether or not ego gets shot.
//
// Applies to at least: English PC Floppy, English PC CD, probably all versions
// Responsible method: Heap in scripts 405, 406, 410, and 411
// Fixes bug #10912
static const uint16 sq4SignatureZeroGravityBlast[] = {
	SIG_MAGICDWORD,                     // blast
	SIG_UINT16(0x0002),                 // yStep = 2
	SIG_UINT16(0x001c),                 // view = 128
	SIG_UINT16(0x0000),                 // loop = 0
	SIG_UINT16(0x0000),                 // cel = 0
	SIG_UINT16(0x0000),                 // priority = 0
	SIG_UINT16(0x0000),                 // underBits = 0
	SIG_UINT16(0x0000),                 // signal = 0
	SIG_END
};

static const uint16 sq4PatchZeroGravityBlast[] = {
	PATCH_ADDTOOFFSET(+12),
	PATCH_UINT16(0x4000),               // signal = $4000 [ set ignore-actors flag ]
	PATCH_END
};

// Cedric the owl from KQ5 appears in the CD version of Ms. Astro Chicken, but a
//  bug in this easter egg makes it a much rarer occurrence than intended.
//
// Every 50 ticks there's a 1 in 21 chance of Cedric appearing if there's no
//  rock on screen and he hasn't already been killed by colliding with the
//  player or getting shot by the farmer. The problem is that unless Cedric
//  appears before the farmer, which is very unlikely, then the farmer's first
//  bullet will kill Cedric off-screen due to incorrect collision testing.
//  buckShot:doit tests for collision by calling cedric:onMe, but Cedric isn't
//  initialized until he first appears, and View:onMe always returns true no
//  matter what coordinates are being tested if its rectangle isn't initialized.
//
// We fix this by initializing Cedric's actor-hidden signal flag on the heap.
//  This prevents View:onMe from returning true before Cedric is initialized.
//  The flag is later cleared by cedric:init when he is placed on screen.
//
// Applies to: English PC CD
// Responsible method: Heap in script 290
// Fixes bug #10920
static const uint16 sq4CdSignatureCedricEasterEgg[] = {
	SIG_MAGICDWORD,                     // cedric
	SIG_UINT16(0x0110),                 // view = 272
	SIG_UINT16(0x0000),                 // loop = 0
	SIG_UINT16(0x0000),                 // cel = 0
	SIG_UINT16(0x000d),                 // priority = 13
	SIG_UINT16(0x0000),                 // underBits = 0
	SIG_UINT16(0x0810),                 // signal = $0810
	SIG_END
};

static const uint16 sq4CdPatchCedricEasterEgg[] = {
	PATCH_ADDTOOFFSET(+10),
	PATCH_UINT16(0x0890),               // signal = $0890 [ set actor-hidden flag ]
	PATCH_END
};

// Colliding with Cedric in Ms. Astro Chicken after colliding with an obstacle
//  locks up the game. cedric:doit doesn't check if the player has been hit
//  before running killCedricScript. This interferes with the collision scripts'
//  animations and prevents them from continuing.
//
// We fix this by not running killCedricScript if the player has been hit.
//  Unfortunately there's no single property that can be tested as each
//  collision script does things differently, so this is a two-part patch.
//  First, ScrollActor:doChicken is patched to set User:canControl to 0, making
//  it consistent with the other collision scripts. Second, cedric:doit is
//  patched to test this value. To make room for this we replace testing
//  killCedricScript with testing the flag that killCedricScript sets.
//
// Applies to: English PC CD
// Responsible methods: ScrollActor:doChicken, cedric:doit
// Fixes bug #10920
static const uint16 sq4CdSignatureCedricLockup1[] = {
	SIG_MAGICDWORD,
	0x18,                               // not
	0x30, SIG_UINT16(0x0049),           // bnt 0049 [ end of method ]
	0x63, 0x84,                         // pToa deathLoop
	0x30, SIG_UINT16(0x0044),           // bnt 0044 [ end of method ]
	0x38, SIG_SELECTOR16(stop),         // pushi stop
	0x76,                               // push0
	0x81, 0x64,                         // lag 64
	0x4a, 0x04,                         // send 04 [ longSong2 stop: ]
	0x38, SIG_SELECTOR16(stop),         // pushi stop
	0x76,                               // push0
	0x72, SIG_UINT16(0x0108),           // lofsa eggSplatting
	0x4a, 0x04,                         // send 04 [ eggSplatting stop: ]
	0x39, SIG_SELECTOR8(number),        // pushi number
	0x78,                               // push1
	0x67, 0x86,                         // pTos deathMusic
	0x39, SIG_SELECTOR8(loop),          // pushi loop
	0x78,                               // push1
	0x78,                               // push1 [ unnecessary, loop is initialized to 1 on heap ]
	SIG_ADDTOOFFSET(+7),
	0x4a, 0x12,                         // send 12 [ theSound number: deathMusic loop: 1 play: self ]
	SIG_END
};

static const uint16 sq4CdPatchCedricLockup1[] = {
	0x2f, 0x4b,                         // bt 4b [ end of method ]
	0x63, 0x84,                         // pToa deathLoop
	0x31, 0x47,                         // bnt 47 [ end of method ]
	0x38, PATCH_SELECTOR16(stop),       // pushi stop
	0x3c,                               // dup
	0x76,                               // push0
	0x81, 0x64,                         // lag 64
	0x4a, 0x04,                         // send 04 [ longSong2 stop: ]
	0x76,                               // push0
	0x72, PATCH_UINT16(0x0108),         // lofsa eggSplatting
	0x4a, 0x04,                         // send 04 [ eggSplatting stop: ]
	0x39, PATCH_SELECTOR8(number),      // pushi number
	0x78,                               // push1
	0x67, 0x86,                         // pTos deathMusic
	0x38, PATCH_SELECTOR16(canControl), // pushi canControl
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x50,                         // lag 50
	0x4a, 0x06,                         // send 06 [ User canControl: 0 ]
	PATCH_ADDTOOFFSET(+7),
	0x4a, 0x0c,                         // send 0c [ theSound number: deathMusic play: self ]
	PATCH_END
};

static const uint16 sq4CdSignatureCedricLockup2[] = {
	SIG_MAGICDWORD,
	0x31, 0x17,                         // bnt 17 [ end of method ]
	0x38, SIG_SELECTOR16(script),       // pushi script
	0x76,                               // push0
	0x51, 0x9c,                         // class astroChicken
	0x4a, 0x04,                         // send 04 [ astroChicken script? ]
	0x18,                               // not     [ acc = 1 if killCedricScript not running ]
	0x31, 0x0c,                         // bnt 0c  [ end of method ]
	0x38, SIG_SELECTOR16(setScript),    // pushi setScript
	0x78,                               // push1
	0x72, SIG_UINT16(0x0f7c),           // lofsa killCedricScript
	0x36,                               // push
	0x51, 0x9c,                         // class astroChicken
	0x4a, 0x06,                         // send 06 [ astroChicken setScript: killCedricScript ]
	0x48,                               // ret
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4CdPatchCedricLockup2[] = {
	0x31, 0x18,                         // bnt 18 [ end of method ]
	0x38, PATCH_SELECTOR16(canControl), // pushi canControl
	0x76,                               // push0
	0x81, 0x50,                         // lag 50
	0x4a, 0x04,                         // send 04 [ User canControl? ]
	0x8b, 0x21,                         // lsl 21  [ local33 = 0 if cedric is alive, 1 if dead ]
	0x22,                               // lt?     [ acc = 1 if cedric is alive and user has control ]
	0x31, 0x0b,                         // bnt 0b  [ end of method ]
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x74, PATCH_UINT16(0x0f7c),         // lofss killCedricScript
	0x51, 0x9c,                         // class astroChicken
	0x4a, 0x06,                         // send 06 [ astroChicken setScript: killCedricScript ]
	PATCH_END
};

// Dodging a biker in Ulence Flats before they've reached their first checkpoint
//  causes the player to prematurely regain control, allowing them to break the
//  game by walking to another room before the dodge sequence completes. This is
//  due to the biker scripts in each room having an unnecessary handsOn call, so
//  we remove it. This does not reduce the time that the player has to react,
//  the scripts theDodgeL and theDodgeR call handsOn when ego crouches.
//
// Biker bugs like this only occur in the CD version due to its slower bikes.
//
// Applies to: English PC CD
// Responsible methods: runOverScript1-3:changeState, runOver:changeState,
//                      runOver2:changeState, runOverScript:changeState
// Fixes bug #9806
static const uint16 sq4CdSignatureBikerHandsOn[] = {
	0x38, SIG_UINT16(0x02bb),           // pushi status [ hard-coded for CD ]
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x04,                         // pushi 04
	0x51, 0x98,                         // class ulence
	0x4a, 0x06,                         // send 06 [ ulence status: 4 ]
	0x76,                               // push0
	0x45, 0x03, 0x00,                   // callb proc0_3 [ handsOn ]
	SIG_END
};

static const uint16 sq4CdPatchBikerHandsOn[] = {
	PATCH_ADDTOOFFSET(+10),
	0x33, 0x02,                         // jmp 02 [ skip handsOn ]
	PATCH_END
};

// Clicking Look/Do/etc on an object in Ulence Flats while a biker approaches
//  can bring ego out of crouch-mode and allow the player to break the game by
//  walking to another room before the dodge sequence completes. This occurs if
//  ego isn't facing the object. Ego's heading will be changed and ultimately
//  stopGroop:doit will reset ego's view to walking.
//
// We fix this by clearing ego:looper when placing ego into crouch mode. This
//  causes Actor:setHeading to instead use kDirLoop which respects the flag
//  kSignalDoesntTurn. ego:looper is automatically restored after dodging.
//
// Applies to: English PC CD
// Responsible methods: theDodgeR:changeState, theDodgeL:changeState
// Fixes bug #9806
static const uint16 sq4CdSignatureBikerCrouchVerb[] = {
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x1a,                         // bnt 1a [ state 2 ]
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x45, 0x03, 0x00,                   // callb proc0_3 [ handsOn ]
	0x7a,                               // push2 [ view ]
	0x78,                               // push1
	0x38, SIG_UINT16(0x027b),           // pushi 027b [ crouch ]
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	SIG_ADDTOOFFSET(+1),                // push0 in theDodgeR, push1 in theDodgeL
	0x38, SIG_SELECTOR16(setCel),       // pushi setCel
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x12,                         // send 12 [ ego view: 635 setLoop: * setCel: 0 ]
	0x32,                               // jmp [ end of method ]
	SIG_END
};

static const uint16 sq4CdPatchBikerCrouchVerb[] = {
	0x18,                               // not [ save a byte ]
	0x1a,                               // eq?
	0x31, 0x1b,                         // bnt 1b [ state 2 ]
	0x76,                               // push0
	0x39, PATCH_SELECTOR8(looper),      // pushi looper
	0x78,                               // push1
	0x76,                               // push0
	PATCH_ADDTOOFFSET(+17),
	0x4a, 0x18,                         // send 18 [ ego looper: 0 view: 635 setLoop: * setCel: 0 ]
	0x45, 0x03, 0x00,                   // callb proc0_3 [ handsOn ]
	PATCH_END
};

// Clicking Do on the Ulence Flats bar door in room 610 while a biker approaches
//  causes ego to enter the bar before the dodge sequence completes, breaking
//  the game. Sierra forgot to test ulence:egoBusy as they did when clicking on
//  the timepod in room 613.
//
// We fix this by testing ulence:egoBusy before running enterBar in door:doVerb.
//  Fortunately there is already an unnecessary script test we can overwrite.
//  This script test was copied from rm610:doit where it is necessary.
//
// Applies to: English PC CD
// Responsible method: door:doVerb(4)
// Fixes bug #9806
static const uint16 sq4CdSignatureBikerBarDoor[] = {
	0x38, SIG_SELECTOR16(script),       // pushi script
	0x76,                               // push0
	0x81, 0x02,                         // lag 02
	0x4a, 0x04,                         // send 04 [ rm610 script? ]
	0x36,                               // push
	0x72, SIG_UINT16(0x014a),           // lofsa enterBar
	SIG_MAGICDWORD,
	0x1a,                               // eq? [ rm610:script == enterBar ]
	0x18,                               // not
	0x30, SIG_UINT16(0x0019),           // bnt 0019 [ don't run enterBar ]
	SIG_END
};

static const uint16 sq4CdPatchBikerBarDoor[] = {
	0x38, PATCH_UINT16(0x02cc),         // pushi egoBusy [ hard-coded for CD ]
	0x76,                               // push0
	0x51, 0x98,                         // class ulence
	0x4a, 0x04,                         // send 04 [ ulence egoBusy? ]
	0x18,                               // not
	0x32, PATCH_UINT16(0x0002),         // jmp 0002
	PATCH_END
};

// Clicking Do on the timepod in room 613 while a biker approaches always shows
//  "Not now!" in a message box, even in speech mode. It seems that Sierra
//  thought they didn't have audio for this message and created a text resource
//  just for it, but it also appears in room 90 with audio, so we use that.
//
// Applies to: English PC CD
// Responsible method: ship:doVerb(4)
// Fixes bug #10922
static const uint16 sq4CdSignatureBikerTimepodMessage[] = {
	0x36,                               // push
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x0d,                         // bnt 0d [ skip if ulence:egoBusy != 1 ]
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0265),           // pushi 0265
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x46, SIG_UINT16(0x0330),           // calle proc816_1 04 [ message box ]
	      SIG_UINT16(0x0001), 0x04,
	SIG_END
};

static const uint16 sq4CdPatchBikerTimepodMessage[] = {
	0x31, 0x11,                         // bnt 11 [ skip if ulence:egoBusy == 0 ]
	0x38, PATCH_SELECTOR16(modNum),     // pushi modNum
	0x78,                               // push1
	0x39, 0x5a,                         // pushi 5a
	0x38, PATCH_SELECTOR16(say),        // pushi say
	0x78,                               // push1
	0x7a,                               // push2
	0x81, 0x59,                         // lag 59
	0x4a, 0x0c,                         // send 0c [ Sq4GlobalNarrator modNum: 90 say: 2 ]
	PATCH_END
};

// Hiding from the Sequel Police in the electronics store (room 390) locks up
//  the CD version and has a subsequent animation bug.
//
// This scene is triggered by going east to the escalator after the police
//  arrive and then back to the electronics store. Police appear on both sides
//  and the only way to survive is to hide in the store while they talk before
//  splitting up. Their first message fails to set a caller, locking up the
//  game, and their other messages contain typos and newline characters left
//  over from floppy versions.
//
// We fix the lockup by passing the missing "self" parameter so that the script
//  proceeds. This exposes the next bug, where the police walk to the beltways
//  and instead of standing, shoot wildly into the mall. The script doesn't
//  remove their Walk cyclers and so they continue to animate. This worked in
//  floppy versions but the Cycle classes were upgraded in CD with different
//  behavior. We fix this by removing the Walk cyclers.
//
// Applies to: English PC CD
// Responsible method: sp1Squeeze:changeState
// Fixes bug #10977
static const uint16 sq4CdSignatureHzSoGoodSequelPoliceLockup[] = {
	0x38, SIG_SELECTOR16(say),          // pushi say
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x72, SIG_UINT16(0x0630),           // lofsa tSP1
	0x4a, 0x06,                         // send 06  [ tSP1 say: 1 ]
	0x32, SIG_UINT16(0x01c1),           // jmp 01c1 [ end of method ]
	SIG_END
};

static const uint16 sq4CdPatchHzSoGoodSequelPoliceLockup[] = {
	PATCH_ADDTOOFFSET(+3),
	0x7a,                               // push2
	PATCH_ADDTOOFFSET(+4),
	0x7c,                               // pushSelf
	0x4a, 0x08,                         // send 06 [ tSP1 say: 1 self ]
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_END
};

static const uint16 sq4CdSignatureHzSoGoodSequelPoliceCycler[] = {
	0x31, 0x20,                         // bnt 20 [ state 12 ]
	SIG_ADDTOOFFSET(+29),
	SIG_MAGICDWORD,
	0x32, SIG_UINT16(0x00a9),           // jmp 00a9 [ end of method ]
	0x3c,                               // dup
	0x35, 0x0c,                         // ldi 0c
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0030),           // bnt 0030 [ state 13 ]
	0x7a,                               // push2 [ view ]
	0x78,                               // push1
	0x39, 0x0d,                         // pushi 0d
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	SIG_ADDTOOFFSET(+32),
	0x4a, 0x24,                         // send 24 [ sp1 view: 13 setLoop: ... ]
	SIG_ADDTOOFFSET(+11),
	0x31, 0x21,                         // bnt 21 [ state 14 ]
	SIG_ADDTOOFFSET(+30),
	0x32, SIG_UINT16(0x004b),           // jmp 004b [ end of method ]
	0x3c,                               // dup
	0x35, 0x0e,                         // ldi 0e
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0030),           // bnt 0030 [ state 15 ]
	0x7a,                               // push2 [ view ]
	0x78,                               // push1
	0x39, 0x0d,                         // pushi 0d
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	SIG_ADDTOOFFSET(+37),
	0x4a, 0x26,                         // send 26 [ sp2 view: 13 setLoop: ... ]
	SIG_END
};

static const uint16 sq4CdPatchHzSoGoodSequelPoliceCycler[] = {
	0x31, 0x1d,                         // bnt 1d [ state 12 ]
	PATCH_ADDTOOFFSET(+29),
	0x3c,                               // dup
	0x35, 0x0c,                         // ldi 0c
	0x1a,                               // eq?
	0x31, 0x34,                         // bnt 34 [ state 13 ]
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x78,                               // push1
	0x76,                               // push0
	0x7a,                               // push2 [ view ]
	0x78,                               // push1
	0x39, 0x0d,                         // pushi 0d
	0x39, PATCH_SELECTOR8(loop),        // pushi loop
	PATCH_ADDTOOFFSET(+32),
	0x4a, 0x2a,                         // send 2a [ sp1 setCycle: 0 view: 13 loop: ... ]
	PATCH_ADDTOOFFSET(+11),
	0x31, 0x1e,                         // bnt 1e [ state 14 ]
	PATCH_ADDTOOFFSET(+30),
	0x3c,                               // dup
	0x35, 0x0e,                         // ldi 0e
	0x1a,                               // eq?
	0x31, 0x34,                         // bnt 34 [ state 15 ]
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x78,                               // push1
	0x76,                               // push0
	0x7a,                               // push2 [ view ]
	0x78,                               // push1
	0x39, 0x0d,                         // pushi 0d
	0x39, PATCH_SELECTOR8(loop),        // pushi loop
	PATCH_ADDTOOFFSET(+37),
	0x4a, 0x2c,                         // send 2c [ sp2 setCycle: 0 view: 13 loop: ... ]
	PATCH_END
};

// Room 370 in front of Sock's is missing a flag check and runs a lethal Sequel
//  Police script when escaping the Skate-O-Rama after hiding in the electronics
//  store. A lockup bug in the CD version prevented getting this far. As this is
//  the wrong time to run the script, the police are drawn as if they're still
//  swimming in zero gravity.
//
// Flag 23 is set when the police pursue in escalator room 400 and triggers the
//  squeeze scripts that force entering the Skate-O-Rama in rooms 370 and 390.
//  Flag 22 is set when exiting the Skate-O-Rama after dodging the police. The
//  two rooms handle these flags differently and are out of sync. Room 370
//  clears flag 23 and doesn't test 22 while room 390 doesn't clear 23 and does
//  test 22. The result is that room 370 doesn't see that you've escaped the
//  Skate-O-Rama and assumes the police are still pursuing from room 400.
//
// We fix this by only running sp1Squeeze in room 370 when coming from room 400.
//  This is equivalent to adding the missing flag 22 check but can be done in
//  the available bytes. To make room we use the clear-flag method's return
//  value, which is the previous flag value, and only test if the previous room
//  number is even, which only room 400 is. This patch is split into two parts
//  to surround a bnt instruction whose operand size changed between versions.
//
// Applies to: All versions
// Responsible method: rm570:init
// Fixes bug #10977
static const uint16 sq4SignatureSocksSequelPoliceFlag1[] = {
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x17,                         // pushi 17
	0x45, 0x06, 0x02,                   // callb proc0_6 [ is flag 23 set? ]
	SIG_END
};

static const uint16 sq4PatchSocksSequelPoliceFlag1[] = {
	PATCH_ADDTOOFFSET(+3),
	0x45, 0x08, 0x02,                   // callb proc0_8 [ clear flag 23, was flag set? ]
	PATCH_END
};

static const uint16 sq4SignatureSocksSequelPoliceFlag2[] = {
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x17,                         // pushi 17
	0x45, 0x08, 0x02,                   // callb proc0_8 [ clear flag 23 ]
	SIG_ADDTOOFFSET(+0x27),
	0x38, SIG_SELECTOR16(setRegions),   // pushi setRegions
	SIG_END
};

static const uint16 sq4PatchSocksSequelPoliceFlag2[] = {
	0x81, 0x0c,                         // lag 0c
	0x78,                               // push1
	0x12,                               // and   [ is previous room number odd? ]
	0x2f, 0x27,                         // bt 27 [ skip sp1Squeeze ]
	PATCH_END
};

// Clicking Walk while getting shot by the Sequel Police outside of Sock's in
//  room 370 crashes the CD version. This causes an Oops! error in the original.
//  The lookupSelector error comes from within the Grooper and Grycler classes
//  but the real bug is that this room's script fails to call handsOff, allowing
//  movement during ego's death animation, unlike all the other laser scripts.
//
// We prevent the crash by adding the missing handsOff call.
//
// Applies to: English PC CD
// Responsible method: sp2Squeeze:changeState(3)
// Fixes bug #10974
static const uint16 sq4CdSignatureSocksSequelPoliceHandsOff[] = {
	0x76,                               // push0 [ y ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x04,                         // send 04 [ ego y? ]
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35, 0x20,                         // ldi 20
	0x04,                               // sub
	0xa3, 0x00,                         // sal 00 [ local0 = ego:y ]
	0x38, SIG_SELECTOR16(setMotion),    // pushi setMotion
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x06,                         // send 06 [ ego setMotion: 0 ]
	SIG_END
};

static const uint16 sq4CdPatchSocksSequelPoliceHandsOff[] = {
	0x38, PATCH_SELECTOR16(setMotion),  // pushi setMotion
	0x78,                               // push1
	0x76,                               // push0
	0x76,                               // push0 [ y ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 00
	0x4a, 0x0a,                         // send 0a [ ego setMotion: 0 y?, saves 4 bytes ]
	0x36,                               // push
	0x35, 0x20,                         // ldi 20
	0x04,                               // sub
	0xa3, 0x00,                         // sal 00 [ local0 = ego:y ]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb proc0_2 00 [ handsOff ]
	PATCH_END
};

// The door to Sock's is immediately disposed of in the CD version, breaking its
//  Look message and preventing it from being drawn when restoring a saved game.
//  We remove the incorrect dispose call along with a redundant addToPic.
//
// Applies to: English PC CD
// Responsible method: rm370:init
// Fixes bug #10914
static const uint16 sq4CdSignatureSocksDoor[] = {
	0x38, SIG_SELECTOR16(addToPic),     // pushi addToPic
	0x76,                               // push0
	0x39, SIG_SELECTOR8(dispose),       // pushi dispose
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x0176),           // lofsa door
	0x4a, 0x08,                         // send 08 [ door addToPic: dispose: ]
	SIG_END
};

static const uint16 sq4CdPatchSocksDoor[] = {
	0x32, PATCH_UINT16(0x0009),         // jmp 0009
	PATCH_END
};

// The scripts in SQ4CD support simultaneous playing of speech and subtitles,
// but this was not available as an option. The following two patches enable
// this functionality in the game's GUI options dialog.
//
// Patch 1: iconTextSwitch::show, called when the text options button is shown.
//   This is patched to add the "Both" text resource (i.e. we end up with
//   "Speech", "Text" and "Both")
static const uint16 sq4CdSignatureTextOptionsButton[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x53,                         // sag global[0x53]
	0x39, 0x03,                         // pushi 0x03
	0x78,                               // push1
	0x39, 0x09,                         // pushi 0x09
	0x54, 0x06,                         // self 0x06
	SIG_END
};

static const uint16 sq4CdPatchTextOptionsButton[] = {
	PATCH_ADDTOOFFSET(+7),
	0x39, 0x0b,                         // pushi 0x0b
	PATCH_END
};

// Patch 2: Adjust a check in babbleIcon::init (the two guys from Andromeda),
//  shown when dying/quitting.
//
// Responsible method: babbleIcon::init
// Fixes bug: #6068
static const uint16 sq4CdSignatureBabbleIcon[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x26,                         // bnt 26 [02a7]
	SIG_END
};

static const uint16 sq4CdPatchBabbleIcon[] = {
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x2f, 0x26,                         // bt 26 [02a7]
	PATCH_END
};

// Patch 3: Add the ability to toggle among the three available options
//  when the text options button is clicked: "Speech", "Text" and "Both".
//  Refer to the patch above for additional details.
//
// Responsible method: iconTextSwitch::doit
static const uint16 sq4CdSignatureTextOptions[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[90]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 0x01
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt 0x06 (0x0691)
	0x35, 0x02,                         // ldi 0x02
	0xa1, 0x5a,                         // sag global[90]
	0x33, 0x0a,                         // jmp 0x0a (0x69b)
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 0x02
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt 0x04 (0x069b)
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x5a,                         // sag global[90]
	0x3a,                               // toss
	0x38, SIG_SELECTOR16(show),         // pushi show (0x00d9)
	0x76,                               // push0
	0x54, 0x04,                         // self 0x04
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4CdPatchTextOptions[] = {
	0x89, 0x5a,                         // lsg global[90]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 0x03 (acc = 3)
	0x1a,                               // eq? (global[90] == 3)
	0x2f, 0x07,                         // bt 0x07
	0x89, 0x5a,                         // lsg global[90]
	0x35, 0x01,                         // ldi 0x01 (acc = 1)
	0x02,                               // add (acc = global[90] + 1)
	0x33, 0x02,                         // jmp 0x02
	0x35, 0x01,                         // ldi 0x01 (reset acc to 1)
	0xa1, 0x5a,                         // sag global[90]
	0x33, 0x03,                         // jmp 0x03 (jump over the wasted bytes below)
	0x34, PATCH_UINT16(0x0000),         // ldi 0x0000 (waste 3 bytes)
	0x3a,                               // toss
	// (the rest of the code is the same)
	PATCH_END
};

// Vohaul's scene on the PocketPal (room 545) is incompatible with our dual
//  text+speech mode and reportedly has MIDI timing issues in text mode.
//
// This is an unusual scene in that it uses three empty MIDI songs to control
//  its text delays, which is probably why Sierra didn't fully upgrade it to
//  use a Narrator in the CD version. Instead the tVOHAUL Narrator is only used
//  in speech mode without the formatting it would need to display text. In text
//  mode the original floppy code handles that.
//
// We heavily patch this script to support text+speech mode and remove the MIDIs
//  from the equation. The trick to using tVOHAUL in dual mode is to set its
//  nMsgType to 2. This causes Sq4Narrator to change the game's message mode to
//  speech while it says a message, preventing it from displaying text in a
//  message box since it wasn't provided formatting. Our code needs to know the
//  real message mode while tVOHAUL is speaking so we store that in the script's
//  register for later use.
//
// The audio and text for Vohaul's messages aren't the same. The audio for the
//  second message also contains the third, and the third has no audio resource
//  at all. We work around this by setting a three second timer and displaying
//  the third text while the audio is already playing.
//
// Applies to: English PC CD
// Responsible method: vohaulScript:changeState
// Fixes bug #10241
static const uint16 sq4CdSignatureVohaulPocketPalTextSpeech[] = {
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x26,                         // bnt 26 [ state 1 ]
	SIG_ADDTOOFFSET(+49),
	0x1a,                               // eq? [ is speech mode? ]
	0x31, 0x22,                         // bnt 22
	SIG_ADDTOOFFSET(+13),
	0x38, SIG_SELECTOR16(modNum),       // pushi modNum [ unnecessary when modNum is room number ]
	0x78,                               // push1
	0x38, SIG_UINT16(0x0221),           // pushi 0221
	SIG_ADDTOOFFSET(11),
	0x32, SIG_UINT16(0x00d1),           // jmp 00d1 [ end of method ]
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	SIG_ADDTOOFFSET(+9),
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x78,                               // push1
	0x51, 0x59,                         // class RandCycle
	0x36,                               // push
	0x72, SIG_UINT16(0x0578),           // lofsa vohaulEyes
	0x4a, 0x06,                         // send 06
	0x39, 0x04,                         // pushi 04
	SIG_MAGICDWORD,
	0x76,                               // push0
	0x72, SIG_UINT16(0x0740),           // lofsa "Take a good look, Roger:"
	0x36,                               // push
	0x38, SIG_UINT16(0x0353),           // pushi 0353
	0x7c,                               // pushSelf
	0x40, SIG_UINT16(0xf93e), 0x08,     // call localproc_0062 08 [ display text until midi 851 finishes ]
	0x32, SIG_UINT16(0x00a7),           // jmp 00a7 [ end of method ]
	SIG_ADDTOOFFSET(+10),
	0x1a,                               // eq? [ is text mode? ]
	SIG_ADDTOOFFSET(+52),
	0x1a,                               // eq? [ is speech mode? ]
	0x31, 0x0e,                         // bnt 0e
	SIG_ADDTOOFFSET(+11),
	0x32, SIG_UINT16(0x0057),           // jmp 0057 [ end of method ]
	0x39, 0x04,                         // pushi 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x0760),           // lofsa "Remember this poor wretched soul..."
	0x36,                               // push
	0x38, SIG_UINT16(0x0354),           // pushi 0354
	0x7c,                               // pushSelf
	0x40, SIG_UINT16(0xf8dc), 0x08,     // call localproc_0062 08 [ display text until midi 852 finishes ]
	0x32, SIG_UINT16(0x0045),           // jmp 0045 [ end of method ]
	SIG_ADDTOOFFSET(+6),
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?    [ is text mode? ]
	0x31, 0x17,                         // bnt 17 [ skip text, set cycles = 1 ]
	0x78,                               // push1
	0x8b, 0x00,                         // lsl 00
	0x45, 0x0c, 0x02,                   // call proc0_12 02 [ unnecessary, localproc_0062 calls this ]
	0x39, 0x04,                         // pushi 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x0784),           // lofsa "...for he is your SON!"
	0x36,                               // push
	0x38, SIG_UINT16(0x0355),           // pushi 0355
	0x7c,                               // pushSelf
	0x40, SIG_UINT16(0xf8b7), 0x08,     // call localproc_0062 08 [ display text until midi 853 finishes ]
	0x33, 0x21,                         // jmp 21 [ end of method ]
	0x35, 0x01,                         // ldi 01
	0x65, 0x1a,                         // aTop cycles [ cycles = 1, speech completed or dismissed by user ]
	0x33, 0x1b,                         // jmp 1b [ end of method ]
	SIG_END
};

static const uint16 sq4CdPatchVohaulPocketPalTextSpeech[] = {
	0x2f, 0x2a,                         // bt 26 [ state 1 ]
	0x81, 0x5a,                         // lag 5a
	0x65, 0x24,                         // aTop register [ register = message mode ]
	PATCH_ADDTOOFFSET(+49),
	0x12,                               // and [ is speech or dual mode? ]
	0x31, 0x21,                         // bnt 21
	PATCH_ADDTOOFFSET(+13),
	0x38, PATCH_SELECTOR16(nMsgType),   // pushi nMsgType
	0x78,                               // push1
	0x38, PATCH_UINT16(0x0002),         // pushi 0002 [ speech ]
	PATCH_ADDTOOFFSET(+11),
	0x33, 0x1a,                         // jmp 1a
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x3c,                               // dup [ save 2 bytes ]
	PATCH_ADDTOOFFSET(+9),
	0x78,                               // push1
	0x51, 0x59,                         // class RandCycle
	0x36,                               // push
	0x72, PATCH_UINT16(0x0578),         // lofsa vohaulEyes
	0x4a, 0x06,                         // send 06
	0x35, 0x02,                         // ldi 02
	0x65, 0x1c,                         // aTop seconds [ 2 second delay in text mode ]
	0x63, 0x24,                         // pToa register
	0x78,                               // push1
	0x12,                               // and    [ is text or dual mode? ]
	0x31, 0x0b,                         // bnt 0b [ don't display text ]
	0x39, 0x03,                         // pushi 03
	0x76,                               // push0
	0x74, PATCH_UINT16(0x0740),         // lofss "Take a good look, Roger:"
	0x76,                               // push0
	0x40, PATCH_UINT16(0xf93b), 0x06,   // call localproc_0062 06 [ display text without midi ]
	PATCH_ADDTOOFFSET(+10),
	0x12,                               // and [ is text or dual mode? ]
	PATCH_ADDTOOFFSET(+52),
	0x12,                               // and [ is speech or dual mode? ]
	0x31, 0x0b,                         // bnt 0b
	PATCH_ADDTOOFFSET(+11),
	0x63, 0x24,                         // pToa register
	0x78,                               // push1
	0x12,                               // and    [ is text or dual mode? ]
	0x31, 0x0f,                         // bnt 0f [ don't display text ]
	0x39, 0x03,                         // pushi 03
	0x76,                               // push0
	0x74, PATCH_UINT16(0x0760),         // lofss "Remember this poor wretched soul..."
	0x76,                               // push0
	0x40, PATCH_UINT16(0xf8dd), 0x06,   // call localproc_0062 06 [ display text without midi ]
	0x35, 0x03,                         // ldi 03
	0x65, 0x1c,                         // aTop seconds [ 3 second delay in text or dual mode ]
	PATCH_ADDTOOFFSET(+6),
	0x63, 0x1c,                         // pToa seconds
	0x2f, 0x1d,                         // bnt 1d [ speech dismissed by user, set cycles = 1 ]
	0x63, 0x24,                         // pToa register
	0x78,                               // push1
	0x1a,                               // eq?    [ is text mode? ]
	0x31, 0x04,                         // bnt 04 [ don't set text delay ]
	0x35, 0x03,                         // ldi 03
	0x65, 0x1c,                         // aTop seconds [ 3 second delay in text mode ]
	0x63, 0x24,                         // pToa register
	0x78,                               // push1
	0x12,                               // and    [ is text or dual mode? ]
	0x31, 0x0d,                         // bnt 0d [ skip text, set cycles = 1 ]
	0x39, 0x03,                         // pushi 03
	0x76,                               // push0
	0x74, PATCH_UINT16(0x0784),         // lofss "...for he is your SON!"
	0x76,                               // push0
	0x40, PATCH_UINT16(0xf8b4), 0x06,   // call localproc_0062 06 [ display text without midi ]
	0x33, 0x03,                         // jmp 03
	0x78,                               // push1
	0x69, 0x1a,                         // sTop cycles [ cycles = 1, speech completed ]
	PATCH_END
};

// Walking around the sewer tunnels in the following sequence locks up the game:
//
//  1. Enter the ladder room (90) from the center room (95) while the slime is
//     just below the middle of the screen
//  2. Enter the southwest room (105) from the ladder room (90)
//
// The script enterNorth has a code path which fails to advance the state and so
//  it gets stuck in handsOff mode. If sewer:status is 3, meaning the slime is
//  moving north or south, then enterNorth assumes that sewer:location, the room
//  the slime is in, must be room 105 or 90, but in the sequence above it is 95.
//
// We fix this by setting enterNorth:state to 1 in the problematic code path so
//  that the script advances. Sierra fixed this bug after the English PC floppy
//  versions but forgot to include the fix in the CD version over a year later.
//
// Applies to: English PC Floppy, English PC CD
// Responsible method: enterNorth:changeState(0)
// Fixes bug #10970
static const uint16 sq4FloppySignatureSewerLockup[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0x65, 0x0a,                         // aTop state [ state = 1 ]
	0x32, SIG_UINT16(0x002e),           // jmp 002e   [ end of switch ]
	0x3c,                               // dup
	0x35, 0x5a,                         // ldi 5a [ ladder room ]
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0027),           // bnt 0027 [ end of switch without setting state ]
	SIG_END
};

static const uint16 sq4FloppyPatchSewerLockup[] = {
	PATCH_ADDTOOFFSET(+11),
	0x30, PATCH_UINT16(0xfff2),         // bnt fff2 [ set state before end of switch ]
	PATCH_END
};

static const uint16 sq4CDSignatureSewerLockup[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0x65, 0x14,                         // aTop state [ state = 1 ]
	0x33, 0x2c,                         // jmp 2c     [ end of switch ]
	0x3c,                               // dup
	0x35, 0x5a,                         // ldi 5a [ ladder room ]
	0x1a,                               // eq?
	0x31, 0x26,                         // bnt 26 [ end of switch without setting state ]
	SIG_END
};

static const uint16 sq4CDPatchSewerLockup[] = {
	PATCH_ADDTOOFFSET(+10),
	0x31, 0xf4,                         // bnt f4 [ set state before end of switch ]
	PATCH_END
};

// SQ4CD had an easter egg room of things removed from Sierra games for legal
//  reasons, but the room itself was removed from the game. Instead the room's
//  pic (570) and messages (271) were left in along with the 18 digit timepod
//  code that attempts to load the missing room 271 and of course crashes.
//
// This wouldn't be a problem except that the code is publicly known due to NRS'
//  modified version of the game which includes a script 271 that recreates the
//  room. The code appears in easter egg lists, and players who don't realize it
//  only applies to a modified version attempt it and crash, so we disable it
//  unless script 271 is present.
//
// Applies to: English PC CD
// Responsible method: timeToTimeWarpS:changeState(1)
// Fixes bug #11006
static const uint16 sq4CdSignatureRemovedRoomTimepodCode[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01 [ 1 == room 271 code was entered ]
	0xa3, 0x72,                         // sal 72
	SIG_END
};

static const uint16 sq4CdPatchRemovedRoomTimepodCode[] = {
	0x35, 0x00,                         // ldi 00
	PATCH_END
};

// The NRS modified version of SQ4 contains an impressive script restoration of
//  the Stuff Taken Out for Legal Reasons easter egg room, but clicking on most
//  items underflows the stack. The doVerb methods in script 271 jump into the
//  middle of pod:doVerb to display their messages, but pod:doVerb pushes its
//  parameter on the stack at the start and tosses it at the end.
//
// We fix this by making pod:doVerb safe to jump into by removing the copy of
//  the parameter that it stores on the stack.
//
// Applies to: SQ4 Update by NRS
// Responsible method: pod:doVerb
static const uint16 sq4NrsSignatureRemovedRoomItems[] = {
	0x8f, SIG_MAGICDWORD, 0x01,         // lsp 01
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	SIG_ADDTOOFFSET(+15),
	0x3c,                               // dup
	SIG_ADDTOOFFSET(+27),
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4NrsPatchRemovedRoomItems[] = {
	0x86, PATCH_UINT16(0x0001),         // lap 0001
	0x39, 0x01,                         // pushi 01
	PATCH_ADDTOOFFSET(+15),
	0x60,                               // pprev
	PATCH_ADDTOOFFSET(+27),
	0x48,                               // ret
	PATCH_END
};

// Walking into Sock's dressing room (room 371) can cause ego to escape obstacle
//  boundaries and get stuck behind the wall or counter. Similar problems occur
//  in the original. The dressing room has no obstacle bounding the edge of the
//  screen. Instead, rm371:doit detects if ego has hit the edge and moves him
//  back to x coordinate 173 but doesn't change ego:y. If ego hits the edge on a
//  diagonal pathfinding move then this can place ego around the corner of one
//  of the obstacles that bound the top and bottom of the dressing room.
//  rm371:doit will then move ego within the obstacle and out of bounds.
//
// We fix this by extending the two obstacles an additional 10 pixels past the
//  edge of the screen so that ego can't get around their corners and get stuck.
//  Sierra reduced one of these coordinates in later floppy versions, and it's
//  not clear why, but this change wasn't included in the CD version.
//
// Applies to: All versions
// Responsible method: rm371:init
// Fixes bug #11055
static const uint16 sq4SignatureSocksDressingRoomObstacles[] = {
	0x38, SIG_ADDTOOFFSET(+2),          // pushi 321d or 319d [ x ]
	0x39, 0x46,                         // pushi 70d  [ y ]
	SIG_ADDTOOFFSET(+125),
	0x38, SIG_MAGICDWORD,               // pushi 321d [ x ]
	      SIG_UINT16(0x0141),
	0x39, 0x49,                         // pushi 73d  [ y ]
	SIG_END
};

static const uint16 sq4PatchSocksDressingRoomObstacles[] = {
	0x38, PATCH_UINT16(0x014b),         // pushi 331d [ x ]
	PATCH_ADDTOOFFSET(+127),
	0x38, PATCH_UINT16(0x014b),         // pushi 331d [ x ]
	PATCH_END
};

// SQ4CD lets you keep the unstable ordnance and its points for the entire game.
//
// The bomb in room 40 is a joke item with joke points which kills you when
//  entering the sewer, therefore you're not allowed to pick it up after leaving
//  the sewer. This was originally enforced in the floppy version by setting a
//  short timer which summons the Sequel Police to kill you. The shootEgo script
//  was refactored in the CD version and this code no longer works. Rather than
//  fix this, Sierra left the broken code in place, and added new code to kill
//  you when interacting with the tank if the previous room was the sewer. This
//  is incorrect since you can walk right to room 45 and return to room 40,
//  which defeats both checks and allows you to get and keep the bomb.
//
// We fix this by replacing the previous room test with a flag test. Flag 0 is
//  set when the police are on the streets and is what the original code tested.
//
// Applies to: English PC CD
// Responsible method: tankScript:changeState(2)
// Fixes bug #11077
static const uint16 sq4CdSignatureUnstableOrdnance[] = {
	SIG_MAGICDWORD,
	0x31, 0x2b,                         // bnt 2b
	0x89, 0x0c,                         // lsg 0c [ previous room ]
	0x35, 0x48,                         // ldi 48 [ sewer manhole ]
	0x1a,                               // eq?    [ came from sewer? ]
	SIG_END
};

static const uint16 sq4CdPatchUnstableOrdnance[] = {
	PATCH_ADDTOOFFSET(+2),
	0x78,                               // push1
	0x76,                               // push0 [ flag 0, set when police are on streets ]
	0x45, 0x06, 0x02,                   // callb proc0_6 02 [ is flag 0 set? ]
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry sq4Signatures[] = {
	{  true,     1, "Floppy: EGA intro delay fix",                    2, sq4SignatureEgaIntroDelay,                     sq4PatchEgaIntroDelay },
	{  true,   298, "Floppy: endless flight",                         1, sq4FloppySignatureEndlessFlight,               sq4FloppyPatchEndlessFlight },
	{  true,   376, "Floppy: set sequel police description",          1, sq4FloppySignatureSequelPoliceDescription,     sq4FloppyPatchSequelPoliceDescription },
	{  true,   376, "Floppy: click atm card on sequel police fix",    1, sq4FloppySignatureClickAtmCardOnSequelPolice,  sq4FloppyPatchClickAtmCardOnSequelPolice },
	{  true,   376, "Floppy: throw stuff at sequel police fix",       1, sq4FloppySignatureThrowStuffAtSequelPolice,    sq4FloppyPatchThrowStuffAtSequelPolice },
	{  true,   700, "Floppy: throw stuff at sequel police fix",       1, sq4FloppySignatureThrowStuffAtSequelPolice,    sq4FloppyPatchThrowStuffAtSequelPolice },
	{  true,    40, "CD: unstable ordnance fix",                      1, sq4CdSignatureUnstableOrdnance,                sq4CdPatchUnstableOrdnance },
	{  true,    45, "CD: walk in from below for room 45 fix",         1, sq4CdSignatureWalkInFromBelowRoom45,           sq4CdPatchWalkInFromBelowRoom45 },
	{  true,   105, "Floppy: sewer lockup fix",                       1, sq4FloppySignatureSewerLockup,                 sq4FloppyPatchSewerLockup },
	{  true,   105, "CD: sewer lockup fix",                           1, sq4CDSignatureSewerLockup,                     sq4CDPatchSewerLockup },
	{  true,   271, "NRS: removed room items fix",                    1, sq4NrsSignatureRemovedRoomItems,               sq4NrsPatchRemovedRoomItems },
	{  true,   290, "CD: cedric easter egg fix",                      1, sq4CdSignatureCedricEasterEgg,                 sq4CdPatchCedricEasterEgg },
	{  true,   290, "CD: cedric lockup fix (1/2)",                    1, sq4CdSignatureCedricLockup1,                   sq4CdPatchCedricLockup1 },
	{  true,   290, "CD: cedric lockup fix (2/2)",                    1, sq4CdSignatureCedricLockup2,                   sq4CdPatchCedricLockup2 },
	{  true,   370, "CD: sock's sequel police hands-off fix",         1, sq4CdSignatureSocksSequelPoliceHandsOff,       sq4CdPatchSocksSequelPoliceHandsOff },
	{  true,   370, "CD: sock's door restore and message fix",        1, sq4CdSignatureSocksDoor,                       sq4CdPatchSocksDoor },
	{  true,   370, "CD/Floppy: sock's sequel police flag fix (1/2)", 1, sq4SignatureSocksSequelPoliceFlag1,            sq4PatchSocksSequelPoliceFlag1 },
	{  true,   370, "CD/Floppy: sock's sequel police flag fix (2/2)", 1, sq4SignatureSocksSequelPoliceFlag2,            sq4PatchSocksSequelPoliceFlag2 },
	{  true,   371, "CD/Floppy: sock's dressing room obstacles fix",  1, sq4SignatureSocksDressingRoomObstacles,        sq4PatchSocksDressingRoomObstacles },
	{ false,   370, "Amiga: dress purchase flag check fix",           1, sq4AmigaSignatureDressPurchaseFlagCheck,       sq4AmigaPatchDressPurchaseFlagCheck },
	{ false,   371, "Amiga: dress purchase flag clear fix",           1, sq4AmigaSignatureDressPurchaseFlagClear,       sq4AmigaPatchDressPurchaseFlagClear },
	{ false,   386, "Amiga: dress purchase flag check fix",           1, sq4AmigaSignatureDressPurchaseFlagCheck,       sq4AmigaPatchDressPurchaseFlagCheck },
	{  true,   381, "CD: big and tall room description",              1, sq4CdSignatureBigAndTallDescription,           sq4CdPatchBigAndTallDescription },
	{  true,   385, "CD: monolith burger door message fix",           1, sq4CdSignatureMonolithBurgerDoor,              sq4CdPatchMonolithBurgerDoor },
	{  true,   390, "CD: hz so good sequel police lockup fix",        1, sq4CdSignatureHzSoGoodSequelPoliceLockup,      sq4CdPatchHzSoGoodSequelPoliceLockup },
	{  true,   390, "CD: hz so good sequel police cycler fix",        1, sq4CdSignatureHzSoGoodSequelPoliceCycler,      sq4CdPatchHzSoGoodSequelPoliceCycler },
	{  true,   391, "CD: missing Audio for universal remote control", 1, sq4CdSignatureMissingAudioUniversalRemote,     sq4CdPatchMissingAudioUniversalRemote },
	{  true,   396, "CD: get points for changing back clothes fix",   1, sq4CdSignatureGetPointsForChangingBackClothes, sq4CdPatchGetPointsForChangingBackClothes },
	{  true,   405, "CD/Floppy: zero gravity blast fix",              1, sq4SignatureZeroGravityBlast,                  sq4PatchZeroGravityBlast },
	{  true,   406, "CD/Floppy: zero gravity blast fix",              1, sq4SignatureZeroGravityBlast,                  sq4PatchZeroGravityBlast },
	{  true,   410, "CD/Floppy: zero gravity blast fix",              1, sq4SignatureZeroGravityBlast,                  sq4PatchZeroGravityBlast },
	{  true,   411, "CD/Floppy: zero gravity blast fix",              1, sq4SignatureZeroGravityBlast,                  sq4PatchZeroGravityBlast },
	{ false,   531, "CD: disable timepod code for removed room",      1, sq4CdSignatureRemovedRoomTimepodCode,          sq4CdPatchRemovedRoomTimepodCode },
	{  true,   545, "CD: vohaul pocketpal text+speech fix",           1, sq4CdSignatureVohaulPocketPalTextSpeech,       sq4CdPatchVohaulPocketPalTextSpeech },
	{  true,   610, "CD: biker bar door fix",                         1, sq4CdSignatureBikerBarDoor,                    sq4CdPatchBikerBarDoor },
	{  true,   610, "CD: biker hands-on fix",                         3, sq4CdSignatureBikerHandsOn,                    sq4CdPatchBikerHandsOn },
	{  true,   611, "CD: biker hands-on fix",                         1, sq4CdSignatureBikerHandsOn,                    sq4CdPatchBikerHandsOn },
	{  true,   612, "CD: biker hands-on fix",                         2, sq4CdSignatureBikerHandsOn,                    sq4CdPatchBikerHandsOn },
	{  true,   613, "CD: biker hands-on fix",                         2, sq4CdSignatureBikerHandsOn,                    sq4CdPatchBikerHandsOn },
	{  true,   614, "CD: biker hands-on fix",                         1, sq4CdSignatureBikerHandsOn,                    sq4CdPatchBikerHandsOn },
	{  true,   613, "CD: biker timepod message fix",                  1, sq4CdSignatureBikerTimepodMessage,             sq4CdPatchBikerTimepodMessage },
	{  true,   700, "CD: red shopper message fix",                    1, sq4CdSignatureRedShopperMessageFix,            sq4CdPatchRedShopperMessageFix },
	{  true,   701, "CD: getting shot, while getting rope",           1, sq4CdSignatureGettingShotWhileGettingRope,     sq4CdPatchGettingShotWhileGettingRope },
	{  true,   706, "CD: biker crouch verb fix",                      2, sq4CdSignatureBikerCrouchVerb,                 sq4CdPatchBikerCrouchVerb },
	{  true,     0, "CD: Babble icon speech and subtitles fix",       1, sq4CdSignatureBabbleIcon,                      sq4CdPatchBabbleIcon },
	{  true,   818, "CD: Speech and subtitles option",                1, sq4CdSignatureTextOptions,                     sq4CdPatchTextOptions },
	{  true,   818, "CD: Speech and subtitles option button",         1, sq4CdSignatureTextOptionsButton,               sq4CdPatchTextOptionsButton },
	{  true,   928, "CD: Narrator lockup fix",                        1, ecoquest1Sq4CdNarratorLockupSignature,         ecoquest1Sq4CdNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// When you leave Ulence Flats, another timepod is supposed to appear.
// On fast machines, that timepod appears fully immediately and then
//  starts to appear like it should be. That first appearance is caused
//  by the scripts setting an invalid cel number and the machine being
//  so fast that there is no time for another script to actually fix
//  the cel number. On slower machines, the cel number gets fixed
//  by the cycler and that's why only fast machines are affected.
//  The same issue happens in Sierra SCI.
// We simply set the correct starting cel number to fix the bug.
// Responsible method: robotIntoShip::changeState(9)
static const uint16 sq1vgaSignatureUlenceFlatsTimepodGfxGlitch[] = {
	0x39,
	SIG_MAGICDWORD, SIG_SELECTOR8(cel), // pushi cel
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0x0a (set ship::cel to 10)
	0x38, SIG_UINT16(0x00a0),           // pushi 0x00a0 (ship::setLoop)
	SIG_END
};

static const uint16 sq1vgaPatchUlenceFlatsTimepodGfxGlitch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x09,                         // pushi 0x09 (set ship::cel to 9)
	PATCH_END
};

// In Ulence Flats, there is a space ship, that you will use at some point.
//  Near that space ship are 2 force field generators. When you look at the top
//  of those generators, the game will crash. This happens also in Sierra SCI.
//  It's caused by a jump, that goes out of bounds.
//
// We currently do not know if this was caused by a compiler glitch or if it
//  was a developer error. Anyway we patch this glitchy code, so that the game
//  won't crash anymore.
//
// Applies to at least: English Floppy
// Responsible method: radar1::doVerb
// Fixes bug: #6816
static const uint16 sq1vgaSignatureUlenceFlatsGeneratorGlitch[] = {
	SIG_MAGICDWORD, 0x1a,               // eq?
	0x30, SIG_UINT16(0xcdf4),           // bnt [absolute 0xf000]
	SIG_END
};

static const uint16 sq1vgaPatchUlenceFlatsGeneratorGlitch[] = {
	PATCH_ADDTOOFFSET(+1),
	0x32, PATCH_UINT16(0x0000),         // jmp 0x0000 (waste bytes)
	PATCH_END
};

// No documentation for this patch (TODO)
static const uint16 sq1vgaSignatureEgoShowsCard[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(timesShownID), // pushi timesShownID
	0x78,                               // push1
	0x38, SIG_SELECTOR16(timesShownID), // pushi timesShownID
	0x76,                               // push0
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x04,                         // send 0x04 (get timesShownID)
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x02,                               // add
	0x36,                               // push
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x06,                         // send 0x06 (set timesShownID)
	0x36,                               // push      (wrong, acc clobbered by class, above)
	0x35, 0x03,                         // ldi 0x03
	0x22,                               // lt?
	SIG_END
};

// Note that this script patch is merely a reordering of the
// instructions in the original script.
static const uint16 sq1vgaPatchEgoShowsCard[] = {
	0x38, PATCH_SELECTOR16(timesShownID), // pushi timesShownID
	0x76,                               // push0
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x04,                         // send 0x04 (get timesShownID)
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x02,                               // add
	0x36,                               // push (this push corresponds to the wrong one above)
	0x38, PATCH_SELECTOR16(timesShownID), // pushi timesShownID
	0x78,                               // push1
	0x36,                               // push
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x06,                         // send 0x06 (set timesShownID)
	0x35, 0x03,                         // ldi 0x03
	0x22,                               // lt?
	PATCH_END
};

// The spider droid on planet Korona has a fixed movement speed,
//  which is way faster than the default movement speed of ego.
// This means that the player would have to turn up movement speed,
//  otherwise it will be impossible to escape it.
// We fix this issue by making the droid move a bit slower than ego
//  does (relative to movement speed setting).
//
// Applies to at least: English PC floppy
// Responsible method: spider::doit
static const uint16 sq1vgaSignatureSpiderDroidTiming[] = {
	SIG_MAGICDWORD,
	0x63, 0x4e,                         // pToa script
	0x30, SIG_UINT16(0x0005),           // bnt [further method code]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_UINT16(0x0062),           // jmp [super-call]
	0x38, SIG_UINT16(0x0088),           // pushi 0088h (script)
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] (current room)
	0x4a, 0x04,                         // send 04 (get room script)
	0x30, SIG_UINT16(0x0005),           // bnt [further method code]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_UINT16(0x0052),           // jmp [super-call]
	0x89, 0xa6,                         // lsg global[a6] (set to 1 when ego went up the skeleton tail, set to 2 when going down)
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0012),           // bnt [PChase set code] (when global[A6] != 1)
	0x81, 0xb5,                         // lag global[b5]
	0x30, SIG_UINT16(0x000d),           // bnt [PChase set code] (when global[B5] == 0)
	0x38, SIG_UINT16(0x008c),           // pushi 008c
	0x78,                               // push1
	0x72, SIG_UINT16(0x1cb6),           // lofsa moveToPath
	0x36,                               // push
	0x54, 0x06,                         // self 06
	0x32, SIG_UINT16(0x0038),           // jmp [super-call]
	// PChase set call
	0x81, 0xb5,                         // lag global[B5]
	0x18,                               // not
	0x30, SIG_UINT16(0x0032),           // bnt [super-call] (when global[B5] != 0)
	// followed by:
	// is spider in current room
	// is global[A6h] == 2? -> set PChase
	SIG_END
}; // 58 bytes)

// global[A6h] != 1 (did NOT went up the skeleton)
//  global[B5h] = 0 -> set PChase
//  global[B5h] != 0 -> do not do anything
// global[A6h] = 1 (did went up the skeleton)
//  global[B5h] = 0 -> set PChase
//  global[B5h] != 0 -> set moveToPath

static const uint16 sq1vgaPatchSpiderDroidTiming[] = {
	0x63, 0x4e,                         // pToa script
	0x2f, 0x68,                         // bt [super-call]
	0x38, PATCH_UINT16(0x0088),         // pushi 0088 (script)
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] (current room)
	0x4a, 0x04,                         // send 04
	0x2f, 0x5e,                         // bt [super-call]
	// --> 12 bytes saved
	// new code
	0x38, PATCH_UINT16(0x0176),         // pushi 0176 (egoMoveSpeed)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x04,                         // send 04 - sq1::egoMoveSpeed
	0x36,                               // push
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x0c,                               // shr
	0x02,                               // add --> egoMoveSpeed + (egoMoveSpeed >> 3)
	0x39, 0x01,                         // pushi 01 (waste 1 byte)
	0x02,                               // add --> egoMoveSpeed++
	0x65, 0x4c,                         // aTop cycleSpeed
	0x65, 0x5e,                         // aTop moveSpeed
	// new code end
	0x81, 0xb5,                         // lag global[B5]
	0x31, 0x13,                         // bnt [PChase code chunk]
	0x89, 0xa6,                         // lsg global[A6]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x3e,                         // bnt [super-call]
	0x38, PATCH_UINT16(0x008c),         // pushi 008c
	0x78,                               // push1
	0x72, PATCH_UINT16(0x1cb6),         // lofsa moveToPath
	0x36,                               // push
	0x54, 0x06,                         // self 06 - spider::setScript(movePath)
	0x33, 0x32,                         // jmp [super-call]
	// --> 9 bytes saved
	PATCH_END
};

// The Russian version of SQ1VGA has mangled class names in its scripts. This
//  isn't a problem in Sierra's interpreter since this is just metadata, but our
//  feature detection code looks up several classes by name and requires them to
//  exist. We fix this by patching the Motion, Rm, and Sound strings back to
//  their original values.
//
// Applies to: Russian PC Floppy
// Fixes bug: #10156
static const uint16 sq1vgaSignatureRussianMotionName[] = {
	SIG_MAGICDWORD,
	0x2A, 0x4D, 0x6F, 0x74, 0x69,       // *Motion.
	0x6F, 0x6E, 0x20,
	SIG_END
};

static const uint16 sq1vgaPatchRussianMotionName[] = {
	0x4D, 0x6F, 0x74, 0x69, 0x6F,       // Motion
	0x6E, 0x00,
	PATCH_END
};
static const uint16 sq1vgaSignatureRussianRmName[] = {
	SIG_MAGICDWORD,
	0x2a, 0x52, 0x6d, 0x00,             // *Rm
	SIG_END
};

static const uint16 sq1vgaPatchRussianRmName[] = {
	0x52, 0x6d, 0x00,                   // Rm
	PATCH_END
};

static const uint16 sq1vgaSignatureRussianSoundName[] = {
	SIG_MAGICDWORD,
	0x87, 0xa2, 0xe3, 0xaa, 0x00, 0x00, // ....
	SIG_END
};

static const uint16 sq1vgaPatchRussianSoundName[] = {
	0x53, 0x6f, 0x75, 0x63, 0x64,       // Sound
	PATCH_END
};

//          script, description,                                      signature                                   patch
static const SciScriptPatcherEntry sq1vgaSignatures[] = {
	{  true,    45, "Ulence Flats: timepod graphic glitch",        1, sq1vgaSignatureUlenceFlatsTimepodGfxGlitch, sq1vgaPatchUlenceFlatsTimepodGfxGlitch },
	{  true,    45, "Ulence Flats: force field generator glitch",  1, sq1vgaSignatureUlenceFlatsGeneratorGlitch,  sq1vgaPatchUlenceFlatsGeneratorGlitch },
	{  true,    58, "Sarien armory droid zapping ego first time",  1, sq1vgaSignatureEgoShowsCard,                sq1vgaPatchEgoShowsCard },
	{  true,   704, "spider droid timing issue",                   1, sq1vgaSignatureSpiderDroidTiming,           sq1vgaPatchSpiderDroidTiming },
	{  true,   989, "rename russian Sound class",                  1, sq1vgaSignatureRussianSoundName,            sq1vgaPatchRussianSoundName },
	{  true,   992, "rename russian Motion class",                 1, sq1vgaSignatureRussianMotionName,           sq1vgaPatchRussianMotionName },
	{  true,   994, "rename russian Rm class",                     1, sq1vgaSignatureRussianRmName,               sq1vgaPatchRussianRmName },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// The toolbox in sq5 is buggy. When you click on the upper part of the "put
//  in inventory" button (some items only - for example the hole puncher at the
//  upper left), points will get awarded correctly, and the item will get put
//  into the player's inventory, but you will then get a "not here" message,
//  and the item will also remain as the current mouse cursor.
// The bug report says items may get lost when exiting the toolbox screen,
//  That was not reproduced.
// This is caused by the mouse-click event getting reprocessed (which wouldn't
//  be a problem by itself). Reprocessing treats coordinates differently from
//  the first click (script 226 includes a local subroutine, which checks
//  coordinates in a hardcoded way w/o port-adjustment).
// Because of this, the hotspot for the button is lower than it should be,
//  which results in the game thinking the user didn't click on the button and
//  also results in the "not here" message.
// We fix it by combining state 0 + 1 of takeTool::changeState and so stopping
//  the event from being reprocessed... without touching SCI system scripts.
// Applies to at least: English/German/French PC floppy
// Responsible method: takeTool::changeState
// Fixes bug: #6457
static const uint16 sq5SignatureToolboxFix[] = {
	0x31, 0x13,                    // bnt [check for state 1]
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00aa),      // pushi 00aa
	0x39, 0x05,                    // pushi 05
	0x39, 0x16,                    // pushi 16
	0x76,                          // push0
	0x39, 0x03,                    // pushi 03
	0x76,                          // push0
	0x7c,                          // pushSelf
	0x81, 0x5b,                    // lag global[5b]
	0x4a, 0x0e,                    // send 0e
	0x32, SIG_UINT16(0x0088),      // jmp [end of method]
	0x3c,                          // dup
	0x35, 0x01,                    // ldi 01
	0x1a,                          // eq?
	0x31, 0x28,                    // bnt [check for state 2]
	SIG_END
};

static const uint16 sq5PatchToolboxFix[] = {
	0x31, 0x41,                    // bnt [check for state 2]
	PATCH_ADDTOOFFSET(+16),        // skip to jmp offset
	0x35, 0x01,                    // ldi 01
	0x65, 0x14,                    // aTop [state]
	0x36, 0x00, 0x00,              // ldi 0000 (waste 3 bytes)
	0x35, 0x00,                    // ldi 00 (waste 2 bytes)
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm
// After entering the drive bay (room 1000) through the hallway, clicking walk
//  in most places causes ego to automatically turn around and return to the
//  previous room. This is due to differences in our pathfinding algorithm from
//  Sierra's which results in ego first walking backwards into the control area
//  that triggers the script sExitToHall.
//
// We work around this by adjusting ego's initial MoveTo position by a few
//  pixels to one which doesn't cause pathfinding to send ego backwards.
//
// Applies to: PC Floppy
// Responsible method: sEnterFromHall:changeState(0)
// Fixes bug: #7155
static const uint16 sq5SignatureDriveBayPathfindingFix[] = {
	SIG_MAGICDWORD,
	0x39, 0x0e,                     // pushi 0e [ x = 14d ]
	0x39, 0x6e,                     // pushi 6e [ y = 110d ]
	SIG_END
};

static const uint16 sq5PatchDriveBayPathfindingFix[] = {
	0x39, 0x10,                     // pushi 10 [ x = 16d ]
	0x39, 0x6f,                     // pushi 6f [ y = 111d ]
	PATCH_END
};

// Sitting in the captain's chair while Droole plays paddle ball randomly locks
//  up the game. Upon sitting, sTakeCommand plays a sound using theMusic3 and
//  waits for it to complete. This is the same object that's used to play the
//  paddle ball sound. If ego sits before a paddle ball sound starts or Droole
//  stops paddling and disposes the sound then sTakeCommand is never cued.
//
// We fix this conflict by using a different Sound object for the chair.
//  theMusic4 is only used once while meeting the crew in sNewCaptain.
//
// Applies to: All versions
// Responsible method: sTakeCommand:changeState
// Fixes bug: #6130
static const uint16 sq5SignatureCaptainChairFix[] = {
	SIG_MAGICDWORD,
	0x76,                           // push0
	0x72, SIG_UINT16(0x0018),       // lofsa theMusic4
	SIG_ADDTOOFFSET(+947),
	0x72, SIG_UINT16(0x02ec),       // lofsa theMusic3
	SIG_ADDTOOFFSET(+26),
	0x72, SIG_UINT16(0x02ec),       // lofsa theMusic3
	SIG_END
};

static const uint16 sq5PatchCaptainChairFix[] = {
	PATCH_ADDTOOFFSET(+951),
	0x72, PATCH_UINT16(0x0018),     // lofsa theMusic4
	PATCH_ADDTOOFFSET(+26),
	0x72, PATCH_UINT16(0x0018),     // lofsa theMusic4
	PATCH_END
};

// When using the fruit on WD40 in room 305, she can take off before ego's
//  animation completes and lock up the game. WD40 remains on the log for five
//  seconds but ego's animation runs at the game speed setting and the scripts
//  don't coordinate. At slow speeds, ego's animation can take all five seconds.
//
// We fix this by not allowing WD40 to take off while ego has a script running.
//  To do this we use existing code in sWD40LandOverRog that retries the current
//  state after 10 ticks. This preserves the scene's timing unless WD40 would
//  have gotten stuck, in which case she now waits for sFruitUpWD40 to complete.
//
// Applies to: All versions
// Responsible method: sWD40LandOverRog:changeState
// Fixes bug: #5162
static const uint16 sq5SignatureWd40FruitFix[] = {
	0x6d, 0x14,                     // dpToa state [ state-- ]
	0x35, 0x0a,                     // ldi 0a
	SIG_MAGICDWORD,
	0x65, 0x20,                     // aTop ticks [ ticks = 10 ]
	0x32, SIG_UINT16(0x0106),       // jmp 0106   [ end of method ]
	SIG_ADDTOOFFSET(+57),
	0x31, 0x28,                     // bnt 28 [ state 5 ]
	SIG_ADDTOOFFSET(+7),
	0x31, 0x18,                     // bnt 18
	0x78,                           // push1
	SIG_ADDTOOFFSET(+20),
	0x32, SIG_UINT16(0x00aa),       // jmp 00aa [ end of method ]
	0x35, 0x01,                     // ldi 01
	0x65, 0x1a,                     // aTop cycles
	0x32, SIG_UINT16(0x00a3),       // jmp 00a3 [ end of method ]
	0x3c,                           // dup
	0x35, 0x05,                     // ldi 05
	0x1a,                           // eq?
	0x31, 0x11,                     // bnt 11 [ state 6 ]
	0x39, SIG_SELECTOR8(play),      // pushi play
	0x78,                           // push1
	0x39, 0x4b,                     // pushi 4b
	0x72, SIG_UINT16(0x096e),       // lofsa theMusic3
	0x4a, 0x06,                     // send 06 [ theMusic3 play: 75 ]
	0x35, 0x05,                     // ldi 05
	0x65, 0x1c,                     // aTop seconds [ seconds = 5 ]
	0x32, SIG_UINT16(0x008c),       // jmp 008c [ end of method ]
	0x3c,                           // dup
	0x35, 0x06,                     // ldi 06
	0x1a,                           // eq?
	0x30, SIG_UINT16(0x0053),       // bnt 0053 [ state 7 ]
	SIG_END
};

static const uint16 sq5PatchWd40FruitFix[] = {
	PATCH_ADDTOOFFSET(+66),
	0x31, 0x22,                     // bnt 22 [ state 5 ]
	PATCH_ADDTOOFFSET(+7),
	0x78,                           // push1
	0x31, 0x16,                     // bnt 16
	PATCH_ADDTOOFFSET(+20),
	0x3a,                           // toss
	0x48,                           // ret
	0x69, 0x1a,                     // sTop cycles [ cycles = 1 ]
	0x3c,                           // dup
	0x35, 0x05,                     // ldi 05
	0x1a,                           // eq?
	0x31, 0x0d,                     // bnt 0d [ state 6 ]
	0x39, PATCH_SELECTOR8(play),    // pushi play
	0x78,                           // push1
	0x39, 0x4b,                     // pushi 4b
	0x72, PATCH_UINT16(0x096e),     // lofsa theMusic3
	0x4a, 0x06,                     // send 06 [ theMusic3 play: 75 ]
	0x69, 0x1c,                     // sTop seconds [ seconds = 5 ]
	0x48,                           // ret
	0x3c,                           // dup
	0x35, 0x06,                     // ldi 06
	0x1a,                           // eq?
	0x31, 0x5e,                     // bnt 5e [ state 7 ]
	0X38, PATCH_SELECTOR16(script), // pushi script
	0x76,                           // push0
	0x81, 0x00,                     // lag 00
	0x4a, 0x04,                     // send 04 [ ego script? ]
	0x2e, PATCH_UINT16(0xff76),     // bt ff76 [ state--, ticks = 10 ]
	PATCH_END
};

// In the first release of SQ5, when the cloaking device alarm countdown on
//  WD40's ship expires, a script enters an infinite loop and the interpreter
//  stops responding.
//
// We fix this as Sierra did in later versions by adding a call to SQ5:handsOn
//  and removing the call to sCountDown:dispose before going to deathRoom.
//
// Applies to: English PC 1.03
// Responsible method: sCountDown:changeState(3)
// Fixes bug: #11255
static const uint16 sq5SignatureWd40AlarmCountdownFix[] = {
	0x3c,                           // dup
	0x35, 0x03,                     // ldi 03
	0x1a,                           // eq?
	0x31, 0x0b,                     // bnt 0b [ end of method ]
	0x78,                           // push1
	0x39, 0x15,                     // pushi 15
	SIG_MAGICDWORD,
	0x45, 0x09, 0x02,               // callb proc0_9 02 [ go to deathRoom ]
	0x39, SIG_SELECTOR8(dispose),   // pushi dispose
	0x76,                           // push0
	0x54, 0x04,                     // self 04 [ self dispose: ]
	SIG_END
};

static const uint16 sq5PatchWd40AlarmCountdownFix[] = {
	0x38, PATCH_SELECTOR16(handsOn),// pushi handsOn
	0x76,                           // push0
	0x81, 0x01,                     // lag 01
	0x4a, 0x04,                     // send 04 [ SQ5 handsOn: ]
	0x78,                           // push1
	0x39, 0x15,                     // pushi 15
	0x45, 0x09, 0x02,               // callb proc0_9 02 [ go to deathRoom ]
	0x3a,                           // toss
	0x48,                           // ret
	PATCH_END
};

// In the transporter room, several scripts attempt to temporarily set ego's
//  speed to 6 but instead change the game speed. This prevents ego's speed from
//  being restored. The user must then do this manually in the control panel.
//  These bugs are due to calling ego:setSpeed instead of ego:cycleSpeed, which
//  we fix. This occurs when randomly beaming in with the funnyBeam script and
//  when talking to Cliffy about Bea before curing her.
//
// Applies to: All versions
// Responsible methods: funnyBeam:changeState, talkAboutBea:changeState
// Fixes bug: #11264
static const uint16 sq5SignatureTransporterRoomSpeedFix[] = {
	0x38, SIG_MAGICDWORD,           // pushi setSpeed
	      SIG_SELECTOR16(setSpeed),
	0x78,                           // push1
	0x39, 0x06,                     // pushi 06
	SIG_END
};

static const uint16 sq5PatchTransporterRoomSpeedFix[] = {
	0x38, PATCH_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	PATCH_END
};

// When the elevator doors close in room 250, sElevatorDoors makes an extra
//  call to handsOn which allows ego to walk around and interact with the room
//  during room transitions, and this can lock up the game by running unexpected
//  scripts such as the chicken closet. We patch out this code since all of
//  sElevatorDoors' callers either call handsOn afterwards or change rooms.
//
// Applies to: All versions
// Responsible method: sElevatorDoors:changeState(2)
// Fixes bug: #11605
static const uint16 sq5SignatureElevatorHandsOn[] = {
	0x67, 0x12,                     // pTos client
	0x72, SIG_ADDTOOFFSET(+2),      // lofsa sOpenElev
	SIG_MAGICDWORD,
	0x1c,                           // ne?
	0x31, 0x08,                     // bnt 08
	0x38, SIG_SELECTOR16(handsOn),  // pushi handsOn
	0x76,                           // push0
	0x81, 0x01,                     // lag 01
	0x4a, 0x04,                     // send 04 [ SQ5 handsOn: ]
	SIG_END
};

static const uint16 sq5PatchElevatorHandsOn[] = {
	0x33, 0x0e,                     // jmp 0e [ skip SQ5 handsOn: ]
	PATCH_END
};

// When walking to Genetix room 730 from the bridge in room 760, clicking Walk
//  while ego enters the room interrupts the room script, breaks the exits, and
//  prevents WD-40 from returning the communicator. This can make it impossible
//  to leave. The script sHuman760 is missing a call to handsOff which the other
//  entrance scripts have. We fix this by calling handsOff before starting
//  sHuman760. We make room for this by overwriting a redundant handsOn call.
//
// Applies to: All versions
// Responsible method: rm730:init
// Fixes bug: #11620
static const uint16 sq5SignatureGenetixBridgeHandsOn[] = {
	0x31, 0x16,                       // bnt 16 [ skip fly code if human ]
	0x7a,                             // push2
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00e6),         // pushi 00e6
	0x38, SIG_UINT16(0x0096),         // pushi 0096
	0x47, 0x1f, 0x01, 0x04,           // calle proc31_1 [ fly to 230, 150 ]
	0x38, SIG_SELECTOR16(handsOn),    // pushi handsOn
	0x76,                             // push0
	0x81, 0x01,                       // lag 01
	0x4a, 0x04,                       // send 04 [ SQ5 handsOn: (redundant) ]
	0x32, SIG_UINT16(0x00b1),         // jmp 00b1 [ end of method ]
	SIG_END
};

static const uint16 sq5PatchGenetixBridgeHandsOn[] = {
	0x31, 0x0e,                       // bnt 0e [ skip fly code if human ]
	PATCH_ADDTOOFFSET(+0x0b),
	0x32, PATCH_UINT16(0x00b9),       // jmp 00b9 [ end of method ]
	0x38, PATCH_SELECTOR16(handsOff), // pushi handsOff
	0x76,                             // push0
	0x81, 0x01,                       // lag 01
	0x4a, 0x04,                       // send 04 [ SQ5 handsOff: ]
	PATCH_END
};

//          script, description,                                      signature                             patch
static const SciScriptPatcherEntry sq5Signatures[] = {
	{  true,   200, "captain chair lockup fix",                    1, sq5SignatureCaptainChairFix,          sq5PatchCaptainChairFix },
	{  true,   226, "toolbox fix",                                 1, sq5SignatureToolboxFix,               sq5PatchToolboxFix },
	{  true,   243, "transporter room speed fix",                  3, sq5SignatureTransporterRoomSpeedFix,  sq5PatchTransporterRoomSpeedFix },
	{  true,   250, "elevator handsOn fix",                        1, sq5SignatureElevatorHandsOn,          sq5PatchElevatorHandsOn },
	{  true,   305, "wd40 fruit fix",                              1, sq5SignatureWd40FruitFix,             sq5PatchWd40FruitFix },
	{  true,   335, "wd40 alarm countdown fix",                    1, sq5SignatureWd40AlarmCountdownFix,    sq5PatchWd40AlarmCountdownFix },
	{  true,   730, "genetix bridge handsOn fix",                  1, sq5SignatureGenetixBridgeHandsOn,     sq5PatchGenetixBridgeHandsOn },
	{  true,    30, "ChoiceTalker lockup fix",                     1, sciNarratorLockupSignature,           sciNarratorLockupPatch },
	{  true,   928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,           sciNarratorLockupPatch },
	{  true,  1000, "drive bay pathfinding fix",                   1, sq5SignatureDriveBayPathfindingFix,   sq5PatchDriveBayPathfindingFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark RAMA

// RAMA uses a custom save game format that game scripts read and write
// manually. The save game format serialises object references, which SSCI could
// be done just by writing int16s (since object references were just 16-bit
// indexes), but in ScummVM we have to write the full 32-bit reg_t. We hijack
// kFileIOReadWord/kFileIOWriteWord to do this for us, but we need the game to
// agree to use those kFileIO calls instead of doing raw reads and creating its
// own numbers, as it tries to do here in `SaveManager::readWord`.
static const uint16 ramaSerializeRegTSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(newWith), // pushi newWith ($10b)
	0x7a,                          // push2
	0x7a,                          // push2
	0x72, SIG_UINT16(0x0000),      // lofsa ""
	0x36,                          // push
	0x51, 0x0f,                    // class Str
	SIG_END
};

static const uint16 ramaSerializeRegTPatch1[] = {
	0x38, PATCH_SELECTOR16(readWord),    // pushi readWord
	0x76,                                // push0
	0x62, PATCH_SELECTOR16(saveFilePtr), // pToa saveFilePtr
	0x4a, PATCH_UINT16(0x0004),          // send 4
	0x48,                                // ret
	PATCH_END
};

// When restoring a NukeTimer client, the game makes a self-call to
// `NukeTimer::getSubscriberObj` from `NukeTimer::serialize`, but forgets to
// pass a required argument. In SSCI this happens to work because the value on
// the stack where the first argument should be is the `getSubscriberObj`
// selector, so it evaluates to true, but currently ScummVM defaults
// uninitialised param reads to 0 so the game was following the wrong path and
// breaking.
// Applies to at least: US English
// Fixes bug: #10263
static const uint16 ramaNukeTimerSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),              // line whatever
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(getSubscriberObj), // pushi getSubscriberObj ($3ca)
	0x76,                                   // push0
	0x54, SIG_UINT16(0x0004),               // self 4
	SIG_END
};

static const uint16 ramaNukeTimerPatch[] = {
	0x38, PATCH_SELECTOR16(getSubscriberObj), // pushi getSubscriberObj ($3ca)
	0x78,                                     // push1
	0x38, PATCH_UINT16(0x0001),               // pushi 1 (wasting bytes)
	0x54, PATCH_UINT16(0x0006),               // self 6
	PATCH_END
};

// When opening a datacube on the pocket computer, `DocReader::init` will try
// to perform arithmetic on a pointer to `thighComputer::plane` then use the
// resulting value as the priority for the DocReader. This happened to work in
// SSCI because the plane pointer would just be a high numeric value, but
// ScummVM needs an actual number, not a pointer.
// Applies to at least: US English
static const uint16 ramaDocReaderInitSignature[] = {
	0x39, SIG_SELECTOR8(priority), // pushi priority ($1a)
	0x78,                          // push1
	0x39, SIG_SELECTOR8(plane),    // pushi plane ($19)
	0x76,                          // push0
	0x7a,                          // push2
	SIG_MAGICDWORD,
	0x39, 0x2c,                    // pushi 44
	0x76,                          // push0
	0x43, 0x02, SIG_UINT16(0x04),  // callk ScriptID, 4
	SIG_END
};

static const uint16 ramaDocReaderInitPatch[] = {
	PATCH_ADDTOOFFSET(+3),           // pushi priority, push1
	0x39, PATCH_SELECTOR8(priority), // pushi priority
	PATCH_END
};

// It is not possible to change the directory for ScummVM save games, so
// disable the "change directory" button in the RAMA save dialog.
static const uint16 ramaChangeDirSignature[] = {
	SIG_MAGICDWORD,
	0x7e, SIG_UINT16(0x0064),   // line 100
	0x39, SIG_SELECTOR8(state), // pushi state ($1d)
	0x78,                       // push1
	0x39, 0x03,                 // pushi 3
	0x72, SIG_ADDTOOFFSET(+2),  // lofsa changeDirI
	0x4a, SIG_UINT16(0x000e),   // send 14
	SIG_END
};

static const uint16 ramaChangeDirPatch[] = {
	PATCH_ADDTOOFFSET(+6),    // line 100, pushi state, push1
	0x39, 0x00,               // pushi 0
	PATCH_END
};

static const SciScriptPatcherEntry ramaSignatures[] = {
	{  true,     0, "disable video benchmarking",                     1, sci2BenchmarkSignature,          sci2BenchmarkReversePatch },
	{  true,    15, "disable video benchmarking",                     1, sci2BenchmarkSignature,          sci2BenchmarkReversePatch },
	{  true,    55, "fix bad DocReader::init priority calculation",   1, ramaDocReaderInitSignature,      ramaDocReaderInitPatch },
	{  true,    85, "fix SaveManager to use normal readWord calls",   1, ramaSerializeRegTSignature1,     ramaSerializeRegTPatch1 },
	{  true,   201, "fix crash restoring save games using NukeTimer", 1, ramaNukeTimerSignature,          ramaNukeTimerPatch },
	{  true, 64928, "Narrator lockup fix",                            1, sciNarratorLockupSignature,      sciNarratorLockupPatch },
	{  true, 64990, "disable change directory button",                1, ramaChangeDirSignature,          ramaChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Shivers

// In room 35170, there is a CCTV control station with a joystick that must be
// clicked and dragged to pan the camera. In order to enable dragging, on
// mousedown, the `vJoystick::handleEvent` method calls `vJoystick::doVerb(1)`,
// which enables the drag functionality of the joystick. However,
// `vJoystick::handleEvent` then makes a super call to
// `ShiversProp::handleEvent`, which calls `vJoystick::doVerb()`. This second
// call, which fails to pass an argument, causes an uninitialized read off the
// stack for the first parameter. In SSCI, this happens to work because the
// uninitialized value on the stack happens to be 1. Disabling the super call
// avoids the bad doVerb call without any apparent ill effect.
// The same problem exists when trying to drag the volume & brightness sliders
// in the main menu. These controls are also fixed by this patch.
// Applies to at least: US English
static const uint16 shiversEventSuperCallSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(handleEvent), // pushi handleEvent
	0x78,                              // push1
	0x8f, 0x01,                        // lsp param[1]
	0x59, 0x02,                        // &rest 2
	0x57, 0x7f, SIG_UINT16(0x0006),    // super ShiversProp[7f], 6
	SIG_END
};

static const uint16 shiversEventSuperCallPatch[] = {
	0x48, // ret
	PATCH_END
};

// When the Ixupi is present in the Gods and Items intro room, the game tries to
// play a sound using the play selector, but its arguments are only appropriate
// for the fade selector.
// If the badly constructed sound object from this call ends up receiving a
// signal at any time in the future, the game will try to send to a number and
// crash (because the third argument to play is supposed to be a client object,
// but here it is a number instead). Other rooms make this same call with the
// correct fade selector, so fix the selector here to match.
// Applies to at least: English CD
static const uint16 shiversGodsIxupiPlaySoundSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(play), // pushi play ($33)
	0x38, SIG_UINT16(0x0006),  // pushi 6
	SIG_END
};

static const uint16 shiversGodsIxupiPlaySoundPatch[] = {
	0x38, PATCH_SELECTOR16(fade), // pushi fade ($f3)
	0x39, 0x06,                   // pushi 6
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry shiversSignatures[] = {
	{  true,   942, "disable video benchmarking",                  1, sci2BenchmarkSignature,             sci2BenchmarkPatch },
	{  true,   990, "fix volume & brightness sliders",             2, shiversEventSuperCallSignature,     shiversEventSuperCallPatch },
	{  true, 23090, "fix bad Ixupi sound call",                    1, shiversGodsIxupiPlaySoundSignature, shiversGodsIxupiPlaySoundPatch },
	{  true, 35170, "fix CCTV joystick interaction",               1, shiversEventSuperCallSignature,     shiversEventSuperCallPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Space Quest 6

// After the explosion in the Quarters of Deepship 86, the game tries to perform
// a dramatic long fade, but does this with an unreasonably large number of
// divisions which takes tens of seconds to finish (because transitions are not
// CPU-dependent in ScummVM).
// Fixes bug: #9590
static const uint16 sq6SlowTransitionSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0578),           // pushi $578
	0x51, 0x33,                         // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch1[] = {
	0x38, PATCH_UINT16(0x01f4),         // pushi 500
	PATCH_END
};

// For whatever reason, SQ6 sets the default number of transition divisions to
// be a much larger value at startup (200 vs 30) if it thinks it is running in
// Windows. Room 410 (eulogy room) also unconditionally resets divisions to the
// larger value.
// Fixes bug: #9590
static const uint16 sq6SlowTransitionSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00c8),           // pushi $c8
	0x51, 0x33,                         // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch2[] = {
	0x38, PATCH_UINT16(0x001e),         // pushi 30
	PATCH_END
};

// SQ6 advertises a maximum score of 500 but the game is missing two points.
//  The "Official Player's Guide" authorized by Sierra includes a point list
//  that adds up to 500. It claims that three points should be awarded instead
//  of two when untangling the hose and attaching the staple and celery.
//
// Since an official point list exists that adds up to the 500 points that the
//  game advertises, we add the two missing points to the inventory actions.
//  Two versions of this patch are necessary because the English PC versions
//  were compiled with line number debugging instructions and the subsequent
//  French, German, and Mac versions weren't.
//
// Applies to: All versions
// Responsible methods: Hookah_Hose:cue, Staple:doVerb, Celery:doVerb
// Fixes bug: #11275
static const uint16 sq6MissingPointsSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),      // line
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(points),    // pushi points
	0x78,                           // push1
	0x7a,                           // push2
	SIG_END
};

static const uint16 sq6MissingPointsPatch[] = {
	0x39, PATCH_SELECTOR8(points),  // pushi points
	0x39, 0x01,                     // pushi 01
	0x38, PATCH_UINT16(0x0003),     // pushi 0003
	PATCH_END
};

// French, German, and Mac versions don't include line number instructions
//  and require specific patches.
static const uint16 sq6StapleCeleryPointSignature[] = {
	0x72, SIG_ADDTOOFFSET(+2),      // lofsa Grappling_Hook
	0x36,                           // push
	0x81, 0x09,                     // lag 09
	0x4a, SIG_UINT16(0x0006),       // send 06
	0x39, SIG_SELECTOR8(points),    // pushi points
	SIG_MAGICDWORD,
	0x78,                           // push1
	0x7a,                           // push2
	0x38, SIG_SELECTOR16(setCursor),// pushi setCursor
	SIG_END
};

static const uint16 sq6StapleCeleryPointPatch[] = {
	0x74, PATCH_ADDTOOFFSET(+2),    // lofss Grappling_Hook
	0x81, 0x09,                     // lag 09
	0x4a, PATCH_UINT16(0x0006),     // send 06
	0x39, PATCH_SELECTOR8(points),  // pushi points
	0x78,                           // push1
	0x39, 0x03,                     // pushi 03
	PATCH_END
};

static const uint16 sq6HookahHosePointSignature[] = {
	0x30, SIG_UINT16(0x0054),       // bnt 0054
	SIG_ADDTOOFFSET(+75),
	0x72, SIG_ADDTOOFFSET(+2),      // lofsa Hookah_Connected
	0x36,                           // push
	0x81, 0x09,                     // lag 09
	0x4a, SIG_UINT16(0x000c),       // send 0c
	0x39, SIG_SELECTOR8(points),    // pushi points
	SIG_MAGICDWORD,
	0x78,                           // push1
	0x7a,                           // push2
	0x81, 0x01,                     // lag 01
	SIG_END
};

static const uint16 sq6HookahHosePointPatch[] = {
	0x30, PATCH_UINT16(0x0053),     // bnt 0053
	PATCH_ADDTOOFFSET(+75),
	0x74, PATCH_ADDTOOFFSET(+2),    // lofss Hookah_Connected
	0x81, 0x09,                     // lag 09
	0x4a, PATCH_UINT16(0x000c),     // send 0c
	0x39, PATCH_SELECTOR8(points),  // pushi points
	0x78,                           // push1
	0x39, 0x03,                     // pushi 03
	PATCH_END
};

// Filling the helmet in room 690 awards three points, but this happens every
//  time the player returns to fill the helmet with no limit. We fix this by
//  associating the points with flag 297 so that they're only awarded once, as
//  Sierra did in the French, German, and Mac versions.
//
// Applies to: English PC
// Responsible method: sGetStuff:changeState(8)
static const uint16 sq6DuplicatePointsSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(points),        // pushi points
	0x78,                               // push1
	0x39, 0x03,                         // pushi 03
	0x81, 0x01,                         // lag 01
	0x4a, SIG_UINT16(0x0006),           // send 06 [ SQ6 points: 3 ]
	0x7e,                               // line
	SIG_END
};

static const uint16 sq6DuplicatePointsPatch[] = {
	PATCH_ADDTOOFFSET(+2),
	0x7a,                               // push2
	PATCH_ADDTOOFFSET(+4),
	0x38, PATCH_UINT16(0x0129),         // pushi 0129
	0x4a, PATCH_UINT16(0x0008),         // send 08 [ SQ6 points: 3 297 ]
	PATCH_END
};

// When attempting to restore a game that was saved with a different version of
//  the interpreter, SQ6 displays a standard error dialog but then crashes due
//  to a script bug. This also occurs in the original. SQ6 has custom code in
//  Game:restore to set and restore the cursor differently depending on the
//  current room. If in room 100, the main menu, then it attempts to restore the
//  cursor by sending a message to an uninitialized variable.
//
// We fix this by not restoring the cursor when temp1 is zero. This patch relies
//  on an existing uninitialized read workaround for Game:restore. The dialog
//  still renders poorly, as it does in the original, but that's unrelated.
//
// Applies to: All versions
// Responsible method: Game:restore
// Fixes bug: #9702
static const uint16 sq6RestoreErrorDialogSignature[] = {
	0x38, SIG_SELECTOR16(setCursor),    // pushi setCursor
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x8d, 0x01,                         // lst 01
	0x76,                               // push0
	0x43, 0x54, SIG_UINT16(0x0000),     // callk HaveMouse 00
	0x36,                               // push
	0x54, SIG_UINT16(0x0008),           // self 0008 [ self setCursor: temp1 kHaveMouse ]
	SIG_END
};

static const uint16 sq6RestoreErrorDialogPatch[] = {
	0x85, 0x01,                         // lat 01
	0x30, PATCH_UINT16(0x000a),         // bnt 000a [ skip setCursor if temp1 == 0 ]
	0x38, PATCH_SELECTOR16(setCursor),  // pushi setCursor
	0x7a,                               // push2
	0x8d, 0x01,                         // lst 01
	0x78,                               // push1
	PATCH_END
};

// When the save/restore dialog is in save-mode, it hides the restore message
//  by setting its priority to zero, but this is the same priority as the dialog
//  view which contains the save message. This is another instance of a priority
//  conflict that happens to work in SSCI due to its last ditch sorting which
//  uses internal memory ID values. In our interpreter the restore message is
//  always drawn on top of the dialog and always hides the save message.
//
// As with similar priority conflicts, we work around this by adjusting the
//  restore message's "hidden" priority, and lower it from zero to negative two.
//  Negative one is a reserved sentinel value in View:setPri.
//
// Applies to: All versions
// Responsible method: SRDialog:update
static const uint16 sq6SaveDialogMessageSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(setPri),        // pushi setPri
	0x78,                               // push1
	0x8b, 0x04,                         // lsl 04
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq? 
	0x31, 0x04,                         // bnt 04 [ skip if not saving ]
	0x35, 0x00,                         // ldi 00 [ message priority: 0 ]
	SIG_END
};

static const uint16 sq6SaveDialogMessagePatch[] = {
	PATCH_ADDTOOFFSET(+10),
	0x35, 0xfe,                         // ldi fe [ new priority: -2 ]
	PATCH_END
};

// The scripts that manage the music volume on Polysorbate LX have conflicts
//  which can set the volume too low outside and in the arcade and club.
//
// Outside the arcade and club, room 330 adjusts the current music and volume
//  based on which entrance ego is nearest and the distance. All Polysorbate LX
//  music plays through gSound1. rm330:init records gSound1:vol when ego enters,
/// rm330:doit uses this value in volume calculations, and then sExitToArcade or
//  sExitToClub restores gSound1:vol to its original value. The other two street
//  rooms (320 and 340) slowly fade gSound1:vol in from 0 to the normal value of
//  127 whenever entering from another room.
//
// Entering room 330 from another street while gSound1 is still fading stops the
//  fade and causes the current low volume to be recorded and used. This may be
//  so low that the music can't be heard at all. Entering the arcade or club
//  restores this artificially low volume. Both of these areas expect the volume
//  to already be set to 127 and so it remains low until doing something that
//  adjusts and resets it, like going to and from the basement.
//
// We fix this by ignoring the value of gSound1:vol when entering room 330 so
//  that the scripts always use the normal initial volume of 127 and restore
//  this when entering the arcade or club. This bug is the only circumstance in
//  which gSound1:vol isn't already 127 when entering room 330.
//
// Applies to: All versions
// Responsible method: rm330:init
// Fixes bug: #9578
static const uint16 sq6PolysorbateVolumeSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(vol),           // pushi vol
	0x76,                               // push0
	0x81, 0x68,                         // lag 68
	0x4a, SIG_UINT16(0x0004),           // send 04 [ gSound1 vol? ]
	0xa3, 0x07,                         // sal 07
	SIG_END
};

static const uint16 sq6PolysorbateVolumePatch[] = {
	0x35, 0x7f,                         // ldi 7f [ normal volume ]
	0x32, PATCH_UINT16(0x0003),         // jmp 0003
	PATCH_END
};

// The shuttle's cockpit initializes the control panel incorrectly if the cursor
//  is Walk. If the game is saved in this state then it won't load because
//  walkIcon0's screen item was deleted even though it's the current icon.
//
// We fix this as Sierra did in later versions by setting the current icon to Do
//  when initializing the control panel in room 490.
//
// Applies to: English PC 1.0 (TODO: develop a Mac patch)
// Responsible method: localproc_5c38 in script 490
// Fixes bug: #11673
static const uint16 sq6CockpitIconBarSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	0x38, SIG_SELECTOR16(setupExit),    // pushi setupExit
	0x78,                               // push1
	0x78,                               // push1
	0x81, 0x45,                         // lag 45
	0x4a, SIG_UINT16(0x0006),           // send 06 [ SQIconBar setupExit: 1 ]
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(enable),       // pushi enable
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x45,                         // lag 45
	0x4a, SIG_UINT16(0x0006),           // send 06 [ SQIconBar enable: 0 ]
	0x7e, SIG_ADDTOOFFSET(+2),          // line
	SIG_END
};

static const uint16 sq6CockpitIconBarPatch[] = {
	0x38, PATCH_SELECTOR16(curIcon),    // pushi curIcon
	0x78,                               // push1
	0x39, PATCH_SELECTOR8(at),          // pushi at
	0x78,                               // push1
	0x7a,                               // push2
	0x81, 0x45,                         // lag 45
	0x4a, PATCH_UINT16(0x0006),         // send 06 [ SQIconBar at: 2 ]
	0x36,                               // push
	0x38, PATCH_SELECTOR16(setupExit),  // pushi setupExit
	0x78,                               // push1
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(enable),     // pushi enable
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x45,                         // lag 45
	0x4a, PATCH_UINT16(0x0012),         // send 12 [ SQIconBar curIcon: doIcon2, setupExit: 1, enable: 0 ]
	PATCH_END
};

// The ExitFeature class has a bug which permanently breaks the game's cursors.
//  ExitFeature:doit is responsible for toggling the Walk cursor to and from an
//  Exit cursor. If an ExitFeature's area overlaps with the icon bar and a user
//  mouses from it to the icon bar while the cursor is Exit and clicks on an
//  icon then the new cursor will no longer work. ExitFeature assumes the new
//  icon is Walk and incorrectly sets its message property to Exit, preventing
//  it from ever sending the correct verb again. This occurs in the esophagus
//  where room 640 can be scrolled to a position where the southern ExitFeature
//  is partially under the icon bar.
//
// We fix by this by patching ExitFeature to explicitly use the Walk icon
//  instead of assuming that the current icon is always Walk when the cursor is.
//  This assumption is false when SQIconbar is in the middle of changing icons
//  and calls everyone's doit methods.
//
// Applies to: All versions
// Responsible method: ExitFeature:doit
// Fixes bug: #11640
static const uint16 sq6ExitFeatureIconSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(curIcon),      // pushi curIcon
	0x76,                               // push0
	0x81, 0x45,                         // lag 45
	0x4a, SIG_UINT16(0x0004),           // send 04 [ SQIconbar curIcon? ]
	SIG_END
};

static const uint16 sq6ExitFeatureIconPatch[] = {
	0x38, PATCH_SELECTOR16(walkIconItem), // pushi walkIconItem
	PATCH_END
};

// Narrator lockup fix for SQ6's SQNarrator:sayForReal in versions compiled
//  without debug line number instructions, see sciNarratorLockupSignature.
//  SQ6 also uses the regular Narrator:say which the generic patches handle.
static const uint16 sq6NarratorLockupSignature[] = {
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31, SIG_MAGICDWORD, 0x13,         // bnt 13
	0x38, SIG_SELECTOR16(startAudio),   // pushi startAudio
	0x38, SIG_UINT16(0x0005),           // pushi 0005
	0x67, SIG_ADDTOOFFSET(+1),          // pTos narMod
	0x67, SIG_ADDTOOFFSET(+1),          // pTos narNoun
	0x67, SIG_ADDTOOFFSET(+1),          // pTos narVerb
	0x67, SIG_ADDTOOFFSET(+1),          // pTos narCase
	0x67, SIG_ADDTOOFFSET(+1),          // pTos narSeq
	0x54, SIG_UINT16(0x000e),           // self 0e
	0x67, SIG_ADDTOOFFSET(+1),          // pTos ticks
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x36,                               // push
	0x81, 0x58,                         // lag 58 [ game time ]
	0x02,                               // add
	0x65, SIG_ADDTOOFFSET(+1),          // aTop ticks [ ticks += 60 + game time ]
	SIG_END
};

static const uint16 sq6NarratorLockupPatch[] = {
	0x81, 0x5a,                         // lag 5a
	0x7a,                               // push2
	0x12,                               // and
	0x31, 0x12,                         // bnt 12
	0x38, PATCH_SELECTOR16(startAudio), // pushi startAudio
	0x39, 0x05,                         // pushi 05
	0x67, PATCH_GETORIGINALBYTE(+14),   // pTos narMod
	0x67, PATCH_GETORIGINALBYTE(+16),   // pTos narNoun
	0x67, PATCH_GETORIGINALBYTE(+18),   // pTos narVerb
	0x67, PATCH_GETORIGINALBYTE(+20),   // pTos narCase
	0x67, PATCH_GETORIGINALBYTE(+22),   // pTos narSeq
	0x54, PATCH_UINT16(0x000e),         // self 0e
	0x67, PATCH_GETORIGINALBYTE(+27),   // pTos ticks
	0x35, 0x3c,                         // ldi 3c
	0x02,                               // add
	0x89, 0x58,                         // lsg 58 [ game time ]
	0x02,                               // add
	0x65, PATCH_GETORIGINALBYTE(+27),   // aTop ticks [ ticks += 60 + game time ]
	0x00,                               // bnot
	0x31, 0xfb,                         // bnt fb [ set ticks to 0 if ticks == -1 ]
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry sq6Signatures[] = {
	{  true,     0, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,    15, "fix english pc missing points",               3, sq6MissingPointsSignature,       sq6MissingPointsPatch },
	{  true,    15, "fix staple/celery missing point",             2, sq6StapleCeleryPointSignature,   sq6StapleCeleryPointPatch },
	{  true,    15, "fix hookah hose missing point",               1, sq6HookahHosePointSignature,     sq6HookahHosePointPatch },
	{  true,    15, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,    22, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,    31, "fix ExitFeature breaking icons",              2, sq6ExitFeatureIconSignature,     sq6ExitFeatureIconPatch },
	{  true,    33, "disable video benchmarking",                  1, sci2BenchmarkSignature,          sci2BenchmarkPatch },
	{  true,   330, "fix polysorbate lx music volume",             1, sq6PolysorbateVolumeSignature,   sq6PolysorbateVolumePatch },
	{  true,   410, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,   460, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   490, "fix invalid cockpit icon bar",                1, sq6CockpitIconBarSignature,      sq6CockpitIconBarPatch },
	{  true,   500, "fix slow transitions",                        1, sq6SlowTransitionSignature1,     sq6SlowTransitionPatch1 },
	{  true,   510, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   690, "fix duplicate points",                        1, sq6DuplicatePointsSignature,     sq6DuplicatePointsPatch },
	{  true,    40, "SQNarrator lockup fix",                       1, sq6NarratorLockupSignature,      sq6NarratorLockupPatch },
	{  true,    40, "SQNarrator lockup fix",                       1, sciNarratorLockupLineSignature,  sciNarratorLockupLinePatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,      sciNarratorLockupPatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupLineSignature,  sciNarratorLockupLinePatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,          sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,          sci2NumSavesPatch2 },
	{  true, 64990, "fix save game dialog message",                1, sq6SaveDialogMessageSignature,   sq6SaveDialogMessagePatch },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,          sci2ChangeDirPatch },
	{  true, 64994, "fix restore-error dialog",                    1, sq6RestoreErrorDialogSignature,  sq6RestoreErrorDialogPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Torins Passage

// A subroutine that gets called by 'Torin::init' unconditionally resets the
// audio volumes to defaults, but the game should always use the volume stored
// in ScummVM. This patch is basically identical to the patch for LSL7, except
// that they left line numbers in the LSL7 scripts and changed the music volume.
// Applies to at least: English CD
// Fixes bug: #9700
static const uint16 torinVolumeResetSignature1[] = {
	SIG_MAGICDWORD,
	0x35, 0x28, // ldi $28
	0xa1, 0xe3, // sag global[$e3] (music volume)
	0x35, 0x3c, // ldi $3c
	0xa1, 0xe4, // sag global[$e4] (sfx volume)
	0x35, 0x64, // ldi $64
	0xa1, 0xe5, // sag global[$e5] (speech volume)
	SIG_END
};

static const uint16 torinVolumeResetPatch1[] = {
	0x33, 0x0a, // jmp [past volume resets]
	PATCH_END
};

// A subroutine that gets called by 'Torin::init' unconditionally resets the
// audio volumes to values stored in torin.prf, but the game should always use
// the volume stored in ScummVM. This patch is basically identical to the patch
// for LSL7, except that they left line numbers in the LSL7 scripts.
// Applies to at least: English CD
// Fixes bug: #9700
static const uint16 torinVolumeResetSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(readWord),     // pushi readWord ($020b)
	0x76,                               // push0
	SIG_ADDTOOFFSET(+6),                // ...
	0xa1, 0xe3,                         // sag global[$e3] (music volume)
	SIG_ADDTOOFFSET(+10),               // ...
	0xa1, 0xe4,                         // sag global[$e4] (sfx volume)
	SIG_ADDTOOFFSET(+10),               // ...
	0xa1, 0xe5,                         // sag global[$e5] (speech volume)
	SIG_END
};

static const uint16 torinVolumeResetPatch2[] = {
	PATCH_ADDTOOFFSET(+10),
	0x18, 0x18,                         // (waste bytes)
	PATCH_ADDTOOFFSET(+10),             // ...
	0x18, 0x18,                         // (waste bytes)
	PATCH_ADDTOOFFSET(+10),             // ...
	0x18, 0x18,                         // (waste bytes)
	PATCH_END
};

// In Escarpa, it is possible for Boogle to be left outside of Torin's bag
// when fast-forwarding through the exit animation of the seraglio. If this
// happens, when the player goes from the seraglio to the dragon's cave and
// then tries to worm Boogle to the left side of the cave, the game will hang
// because Boogle is on the wrong side of the navigable area barrier and cannot
// move through it to continue the cutscene. This patch fixes the fast-forward
// code 'soBoogleBackUp::ff' in the seraglio so that Boogle's in-the-bag flag
// is set when fast-forwarding.
// Applies to at least: English CD, Spanish CD
// Fixes bug: #9836
static const uint16 torinSeraglioBoogleFlagSignature[] = {
	0x35, 0x00,                 // ldi 0
	SIG_MAGICDWORD,
	0xa3, 0x00,                 // sal local[0]
	0x38, SIG_SELECTOR16(test), // pushi test
	SIG_ADDTOOFFSET(+0x5a),     // all the rest of the method
	// CHECKME: Spanish version seems to have a total of 0x5d bytes from this point to the ret
	// FIXME: Check for end of method (e.g. ret) and add different signatures in case localized versions are different
	SIG_END
};

static const uint16 torinSeraglioBoogleFlagPatch[] = {
	// @1e5f
	// ldi 0, sal local[0] removed from here (+4 bytes)

	// @1e5f (+4 bytes)
	// local[0] = /* oFlags */ ScriptID(64017, 0);
	0x7a,                               // push2
	0x38, PATCH_UINT16(0xfa11),         // pushi 64017
	0x76,                               // push0
	0x43, 0x02, PATCH_UINT16(0x0004),   // callk ScriptID[2], 4
	0xa3, 0x00,                         // sal local[0] (-2 bytes)

	// @1e6a (+2 bytes)
	// acc = local[0].test(94);
	0x38, PATCH_SELECTOR16(test),       // pushi test
	0x78,                               // push1
	0x39, 0x5e,                         // pushi 94
	0x4a, PATCH_UINT16(0x0006),         // send 6

	// @1e73 (+2 bytes)
	// if (!acc) goto elseCase;
	0x30, PATCH_UINT16(0x0034),         // bnt 0x31 + 3

	// @1e76 (+2 bytes)
	// global[0].get(ScriptID(64001, 0).get(20));
	0x38, PATCH_SELECTOR16(get),        // pushi get
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(get),        // pushi get
	0x78,                               // push1
	0x39, 0x14,                         // pushi 20
	0x7a,                               // push2
	0x38, PATCH_UINT16(0xfa01),         // pushi 64001
	0x76,                               // push0
	0x43, 0x02, PATCH_UINT16(0x0004),   // callk ScriptID[2], 4
	0x4a, PATCH_UINT16(0x0006),         // send 6
	0x36,                               // push
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, PATCH_UINT16(0x0006),         // send 6

	// @1e92 (+2 bytes)
	// local[0].set(52);
	0x38, PATCH_SELECTOR16(set),        // pushi set
	0x78,                               // push1
	0x39, 0x34,                         // pushi 52
	0x83, 0x00,                         // lal local[0] (+7 byte)
	0x4a, PATCH_UINT16(0x0006),         // send 6

	// @1e9d (+9 bytes)
	// goto endOfBranch;
	0x33, 0x0b,                         // jmp [to end of conditional branch] (+1 byte)

	// @1e9f (+10 bytes)
	// elseCase: local[0].clear(97);
	0x38, PATCH_SELECTOR16(clear),      // pushi clear
	0x78,                               // push1
	0x39, 0x61,                         // pushi 97
	0x83, 0x00,                         // lal local[0] (+7 bytes)
	0x4a, PATCH_UINT16(0x0006),         // send 6

	// @1eaa (+17 bytes)
	// endOfBranch: local[0].set(232);
	0x38, PATCH_SELECTOR16(set),        // pushi set (-3 bytes)
	0x78,                               // push1 (-1 byte)
	0x38, PATCH_UINT16(0x00e8),         // pushi 232 (Boogle-in-bag flag) (-3 bytes)
	0x83, 0x00,                         // lal local[0] (-2 bytes)
	0x4a, PATCH_UINT16(0x0006),         // send 6 (-3 bytes)

	// @1eb6 (+5 bytes)
	// local[0] = 0; self.dispose();
	0x38, PATCH_SELECTOR16(dispose),    // pushi dispose
	0x76,                               // push0
	0x3c,                               // dup (-1 byte)
	0xab, 0x00,                         // ssl local[0] (-2 bytes)
	0x54, PATCH_UINT16(0x0004),         // self 4
	0x48,                               // ret

	// @1ec1 (+2 bytes)
	PATCH_END
};

// At least some French PointSoft releases of Torin's Passage managed to get
// released with script 20700 from the official Sierra TORINPAT patch and
// *unpatched* heap 20700. Worse, the selector table is not the same as the one
// in the US release, so it is not possible to just apply TORINPAT to the game
// (it will just explode later when mismatched selectors are used). So, here we
// are hot-patching all of the wrong offsets in the original heap to match the
// patched script.
// Applies to at least: French PointSoft CD release
static const uint16 torinPointSoft20700HeapSignature[] = {
	0xe1, 0x15, 0x23, 0x16, // end of patched 20700.SCR (so we don't
							// accidentally patch the heap when it is correctly
							// matched with an unpatched script)
	SIG_ADDTOOFFSET(1),     // padding byte added by Script::load
	SIG_ADDTOOFFSET(0x1d2), // first bad offset in the heap is at 0x1d2
	SIG_MAGICDWORD,
	SIG_UINT16(0xd8),
	SIG_UINT16(0xd8),
	SIG_ADDTOOFFSET(0x200 - 0x1d2 - 4), // second bad offset, etc.
	SIG_UINT16(0xde),
	SIG_UINT16(0xde),
	SIG_ADDTOOFFSET(0x280 - 0x200 - 4),
	SIG_UINT16(0xe0),
	SIG_UINT16(0xe0),
	SIG_ADDTOOFFSET(0x300 - 0x280 - 4),
	SIG_UINT16(0xe2),
	SIG_UINT16(0xe2),
	SIG_ADDTOOFFSET(0x374 - 0x300 - 4),
	SIG_UINT16(0xe4),
	SIG_UINT16(0xe4),
	SIG_ADDTOOFFSET(0x3ce - 0x374 - 4),
	SIG_UINT16(0xee),
	SIG_UINT16(0xee),
	SIG_ADDTOOFFSET(0x44e - 0x3ce - 4),
	SIG_UINT16(0xf0),
	SIG_UINT16(0xf0),
	SIG_ADDTOOFFSET(0x482 - 0x44e - 4),
	SIG_UINT16(0xf6),
	SIG_UINT16(0xf6),
	SIG_ADDTOOFFSET(0x4b6 - 0x482 - 4),
	SIG_UINT16(0xfc),
	SIG_UINT16(0xfc),
	SIG_ADDTOOFFSET(0x4ea - 0x4b6 - 4),
	SIG_UINT16(0x106),
	SIG_UINT16(0x106),
	SIG_ADDTOOFFSET(0x51e - 0x4ea - 4),
	SIG_UINT16(0x110),
	SIG_UINT16(0x110),
	SIG_ADDTOOFFSET(0x55c - 0x51e - 4),
	SIG_UINT16(0x116),
	SIG_UINT16(0x116),
	SIG_ADDTOOFFSET(0x5a2 - 0x55c - 4),
	SIG_UINT16(0x118),
	SIG_UINT16(0x118),
	SIG_END
};

static const uint16 torinPointSoft20700HeapPatch[] = {
	PATCH_ADDTOOFFSET(4),      // end of patched 20700.SCR
	PATCH_ADDTOOFFSET(1),      // padding byte
	PATCH_ADDTOOFFSET(0x1d2),  // first bad offset
	PATCH_UINT16(0xdc),
	PATCH_UINT16(0xdc),
	PATCH_ADDTOOFFSET(0x200 - 0x1d2 - 4), // second bad offset, etc.
	PATCH_UINT16(0xe6),
	PATCH_UINT16(0xe6),
	PATCH_ADDTOOFFSET(0x280 - 0x200 - 4),
	PATCH_UINT16(0xe8),
	PATCH_UINT16(0xe8),
	PATCH_ADDTOOFFSET(0x300 - 0x280 - 4),
	PATCH_UINT16(0xea),
	PATCH_UINT16(0xea),
	PATCH_ADDTOOFFSET(0x374 - 0x300 - 4),
	PATCH_UINT16(0xec),
	PATCH_UINT16(0xec),
	PATCH_ADDTOOFFSET(0x3ce - 0x374 - 4),
	PATCH_UINT16(0xf6),
	PATCH_UINT16(0xf6),
	PATCH_ADDTOOFFSET(0x44e - 0x3ce - 4),
	PATCH_UINT16(0xf8),
	PATCH_UINT16(0xf8),
	PATCH_ADDTOOFFSET(0x482 - 0x44e - 4),
	PATCH_UINT16(0xfe),
	PATCH_UINT16(0xfe),
	PATCH_ADDTOOFFSET(0x4b6 - 0x482 - 4),
	PATCH_UINT16(0x104),
	PATCH_UINT16(0x104),
	PATCH_ADDTOOFFSET(0x4ea - 0x4b6 - 4),
	PATCH_UINT16(0x10e),
	PATCH_UINT16(0x10e),
	PATCH_ADDTOOFFSET(0x51e - 0x4ea - 4),
	PATCH_UINT16(0x118),
	PATCH_UINT16(0x118),
	PATCH_ADDTOOFFSET(0x55c - 0x51e - 4),
	PATCH_UINT16(0x11e),
	PATCH_UINT16(0x11e),
	PATCH_ADDTOOFFSET(0x5a2 - 0x55c - 4),
	PATCH_UINT16(0x120),
	PATCH_UINT16(0x120),
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry torinSignatures[] = {
	{  true, 20600, "fix wrong boogle bag flag on fast-forward",   1, torinSeraglioBoogleFlagSignature,  torinSeraglioBoogleFlagPatch },
	{  true, 20700, "fix bad heap in PointSoft release",           1, torinPointSoft20700HeapSignature,  torinPointSoft20700HeapPatch },
	{  true, 64000, "disable volume reset on startup (1/2)",       1, torinVolumeResetSignature1,        torinVolumeResetPatch1 },
	{  true, 64000, "disable volume reset on startup (2/2)",       1, torinVolumeResetSignature2,        torinVolumeResetPatch2 },
	{  true, 64866, "increase number of save games",               1, torinLarry7NumSavesSignature,      torinLarry7NumSavesPatch },
	{  true, 64928, "Narrator lockup fix",                         1, sciNarratorLockupSignature,        sciNarratorLockupPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// =================================================================================

ScriptPatcher::ScriptPatcher() {
	int selectorCount = ARRAYSIZE(selectorNameTable);
	int selectorNr;

	// Allocate table for selector-IDs and initialize that table as well
	_selectorIdTable = new Selector[ selectorCount ];
	for (selectorNr = 0; selectorNr < selectorCount; selectorNr++)
		_selectorIdTable[selectorNr] = -1;

	_runtimeTable = NULL;
	_isMacSci11 = false;
}

ScriptPatcher::~ScriptPatcher() {
	delete[] _runtimeTable;
	delete[] _selectorIdTable;
}

// will actually patch previously found signature area
void ScriptPatcher::applyPatch(const SciScriptPatcherEntry *patchEntry, SciSpan<byte> scriptData, int32 signatureOffset) {
	const uint16 *patchData = patchEntry->patchData;
	byte orgData[PATCH_VALUELIMIT];
	int32 offset = signatureOffset;
	uint16 patchWord = *patchEntry->patchData;
	uint16 patchSelector = 0;

	// Copy over original bytes from script
	uint32 orgDataSize = scriptData.size() - offset;
	if (orgDataSize > PATCH_VALUELIMIT)
		orgDataSize = PATCH_VALUELIMIT;
	scriptData.subspan(offset, orgDataSize).unsafeCopyDataTo(orgData);

	while (patchWord != PATCH_END) {
		uint16 patchCommand = patchWord & PATCH_COMMANDMASK;
		uint16 patchValue = patchWord & PATCH_VALUEMASK;
		switch (patchCommand) {
		case PATCH_CODE_ADDTOOFFSET: {
			// add value to offset
			offset += patchValue;
			break;
		}
		case PATCH_CODE_GETORIGINALBYTE: {
			// get original byte from script and adjust it
			if (patchValue >= orgDataSize)
				error("Script-Patcher: can not get requested original byte from script");
			byte orgByte = orgData[patchValue];
			int16 adjustValue;
			patchData++; adjustValue = (int16)(*patchData);
			scriptData[offset] = orgByte + adjustValue;
			offset++;
			break;
		}
		case PATCH_CODE_GETORIGINALUINT16: {
			// get original byte from script and adjust it
			if ((patchValue >= orgDataSize) || (((uint32)patchValue + 1) >= orgDataSize))
				error("Script-Patcher: can not get requested original uint16 from script");
			uint16 orgUINT16;
			int16 adjustValue;

			if (!_isMacSci11) {
				orgUINT16 = orgData[patchValue] | (orgData[patchValue + 1] << 8);
			} else {
				orgUINT16 = orgData[patchValue + 1] | (orgData[patchValue] << 8);
			}
			patchData++; adjustValue = (int16)(*patchData);
			orgUINT16 += adjustValue;
			if (!_isMacSci11) {
				scriptData[offset] = orgUINT16 & 0xFF;
				scriptData[offset + 1] = orgUINT16 >> 8;
			} else {
				scriptData[offset] = orgUINT16 >> 8;
				scriptData[offset + 1] = orgUINT16 & 0xFF;
			}
			offset += 2;
			break;
		}
		case PATCH_CODE_UINT16:
		case PATCH_CODE_SELECTOR16: {
			byte byte1;
			byte byte2;

			switch (patchCommand) {
			case PATCH_CODE_UINT16: {
				byte1 = patchValue & PATCH_BYTEMASK;
				patchData++; patchWord = *patchData;
				if (patchWord & PATCH_COMMANDMASK)
					error("Script-Patcher: Patch inconsistent");
				byte2 = patchWord & PATCH_BYTEMASK;
				break;
			}
			case PATCH_CODE_SELECTOR16: {
				patchSelector = _selectorIdTable[patchValue];
				byte1 = patchSelector & 0xFF;
				byte2 = patchSelector >> 8;
				break;
			}
			default:
				byte1 = 0; byte2 = 0;
			}
			if (!_isMacSci11) {
				scriptData[offset++] = byte1;
				scriptData[offset++] = byte2;
			} else {
				// SCI1.1+ on macintosh had uint16s in script in BE-order
				scriptData[offset++] = byte2;
				scriptData[offset++] = byte1;
			}
			break;
		}
		case PATCH_CODE_SELECTOR8: {
			patchSelector = _selectorIdTable[patchValue];
			if (patchSelector & 0xFF00)
				error("Script-Patcher: 8 bit selector required, game uses 16 bit selector");
			scriptData[offset] = patchSelector & 0xFF;
			offset++;
			break;
		}
		case PATCH_CODE_BYTE:
			scriptData[offset] = patchValue & PATCH_BYTEMASK;
			offset++;
			break;
		default:
			break;
		}
		patchData++;
		patchWord = *patchData;
	}
}

bool ScriptPatcher::verifySignature(uint32 byteOffset, const uint16 *signatureData, const char *signatureDescription, const SciSpan<const byte> &scriptData) {
	uint16 sigSelector = 0;

	uint16 sigWord = *signatureData;
	while (sigWord != SIG_END) {
		uint16 sigCommand = sigWord & SIG_COMMANDMASK;
		uint16 sigValue = sigWord & SIG_VALUEMASK;
		switch (sigCommand) {
		case SIG_CODE_ADDTOOFFSET: {
			// add value to offset
			byteOffset += sigValue;
			break;
		}
		case SIG_CODE_UINT16:
		case SIG_CODE_SELECTOR16: {
			if (byteOffset + 1 < scriptData.size()) {
				byte byte1;
				byte byte2;

				switch (sigCommand) {
				case SIG_CODE_UINT16: {
					byte1 = sigValue & SIG_BYTEMASK;
					signatureData++; sigWord = *signatureData;
					if (sigWord & SIG_COMMANDMASK)
						error("Script-Patcher: signature inconsistent\nFaulty signature: '%s'", signatureDescription);
					byte2 = sigWord & SIG_BYTEMASK;
					break;
				}
				case SIG_CODE_SELECTOR16: {
					sigSelector = _selectorIdTable[sigValue];
					byte1 = sigSelector & 0xFF;
					byte2 = sigSelector >> 8;
					break;
				}
				default:
					byte1 = 0; byte2 = 0;
				}
				if (!_isMacSci11) {
					if ((scriptData[byteOffset] != byte1) || (scriptData[byteOffset + 1] != byte2))
						sigWord = SIG_MISMATCH;
				} else {
					// SCI1.1+ on macintosh had uint16s in script in BE-order
					if ((scriptData[byteOffset] != byte2) || (scriptData[byteOffset + 1] != byte1))
						sigWord = SIG_MISMATCH;
				}
				byteOffset += 2;
			} else {
				sigWord = SIG_MISMATCH;
			}
			break;
		}
		case SIG_CODE_SELECTOR8: {
			if (byteOffset < scriptData.size()) {
				sigSelector = _selectorIdTable[sigValue];
				if (sigSelector & 0xFF00)
					error("Script-Patcher: 8 bit selector required, game uses 16 bit selector\nFaulty signature: '%s'", signatureDescription);
				if (scriptData[byteOffset] != (sigSelector & 0xFF))
					sigWord = SIG_MISMATCH;
				byteOffset++;
			} else {
				sigWord = SIG_MISMATCH; // out of bounds
			}
			break;
		}
		case SIG_CODE_BYTE:
			if (byteOffset < scriptData.size()) {
				if (scriptData[byteOffset] != sigWord)
					sigWord = SIG_MISMATCH;
				byteOffset++;
			} else {
				sigWord = SIG_MISMATCH; // out of bounds
			}
			break;
		default:
			break;
		}

		if (sigWord == SIG_MISMATCH)
			break;

		signatureData++;
		sigWord = *signatureData;
	}

	if (sigWord == SIG_END) // signature fully matched?
		return true;
	return false;
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 ScriptPatcher::findSignature(uint32 magicDWord, int magicOffset, const uint16 *signatureData, const char *patchDescription, const SciSpan<const byte> &scriptData) {
	if (scriptData.size() < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	// magicDWord is in platform-specific BE/LE form, so that the later match will work, this was done for performance
	const uint32 searchLimit = scriptData.size() - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == scriptData.getUint32At(DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + magicOffset;

			if (verifySignature(offset, signatureData, patchDescription, scriptData))
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

int32 ScriptPatcher::findSignature(const SciScriptPatcherEntry *patchEntry, const SciScriptPatcherRuntimeEntry *runtimeEntry, const SciSpan<const byte> &scriptData) {
	return findSignature(runtimeEntry->magicDWord, runtimeEntry->magicOffset, patchEntry->signatureData, patchEntry->description, scriptData);
}

// Attention: Magic DWord is returned using platform specific byte order. This is done on purpose for performance.
void ScriptPatcher::calculateMagicDWordAndVerify(const char *signatureDescription, const uint16 *signatureData, bool magicDWordIncluded, uint32 &calculatedMagicDWord, int &calculatedMagicDWordOffset) {
	Selector curSelector = -1;
	int magicOffset;
	byte magicDWord[4];
	int magicDWordLeft = 0;
	uint16 curWord;
	uint16 curCommand;
	uint32 curValue;
	byte byte1 = 0;
	byte byte2 = 0;

	memset(magicDWord, 0, sizeof(magicDWord));

	curWord = *signatureData;
	magicOffset = 0;
	while (curWord != SIG_END) {
		curCommand = curWord & SIG_COMMANDMASK;
		curValue   = curWord & SIG_VALUEMASK;
		switch (curCommand) {
		case SIG_MAGICDWORD: {
			if (magicDWordIncluded) {
				if ((calculatedMagicDWord) || (magicDWordLeft))
					error("Script-Patcher: Magic-DWORD specified multiple times in signature\nFaulty patch: '%s'", signatureDescription);
				magicDWordLeft = 4;
				calculatedMagicDWordOffset = magicOffset;
			} else {
				error("Script-Patcher: Magic-DWORD sequence found in patch data\nFaulty patch: '%s'", signatureDescription);
			}
			break;
		}
		case SIG_CODE_ADDTOOFFSET: {
			magicOffset -= curValue;
			if (magicDWordLeft)
				error("Script-Patcher: Magic-DWORD contains AddToOffset command\nFaulty patch: '%s'", signatureDescription);
			break;
		}
		case SIG_CODE_UINT16:
		case SIG_CODE_SELECTOR16: {
			// UINT16 or 1
			switch (curCommand) {
			case SIG_CODE_UINT16: {
				signatureData++; curWord = *signatureData;
				if (curWord & SIG_COMMANDMASK)
					error("Script-Patcher: signature entry inconsistent\nFaulty patch: '%s'", signatureDescription);
				if (!_isMacSci11) {
					byte1 = curValue;
					byte2 = curWord & SIG_BYTEMASK;
				} else {
					byte1 = curWord & SIG_BYTEMASK;
					byte2 = curValue;
				}
				break;
			}
			case SIG_CODE_SELECTOR16: {
				curSelector = _selectorIdTable[curValue];
				if (curSelector == -1) {
					curSelector = g_sci->getKernel()->findSelector(selectorNameTable[curValue]);
					_selectorIdTable[curValue] = curSelector;
				}
				if (!_isMacSci11) {
					byte1 = curSelector & 0x00FF;
					byte2 = curSelector >> 8;
				} else {
					byte1 = curSelector >> 8;
					byte2 = curSelector & 0x00FF;
				}
				break;
			}
			default:
				break;
			}
			magicOffset -= 2;
			if (magicDWordLeft) {
				// Remember current word for Magic DWORD
				magicDWord[4 - magicDWordLeft] = byte1;
				magicDWordLeft--;
				if (magicDWordLeft) {
					magicDWord[4 - magicDWordLeft] = byte2;
					magicDWordLeft--;
				}
				if (!magicDWordLeft) {
					// Magic DWORD is now known, convert to platform specific byte order
					calculatedMagicDWord = READ_UINT32(magicDWord);
				}
			}
			break;
		}
		case SIG_CODE_BYTE:
		case SIG_CODE_SELECTOR8: {
			if (curCommand == SIG_CODE_SELECTOR8) {
				curSelector = _selectorIdTable[curValue];
				if (curSelector == -1) {
					curSelector = g_sci->getKernel()->findSelector(selectorNameTable[curValue]);
					_selectorIdTable[curValue] = curSelector;
					if (curSelector != -1) {
						if (curSelector & 0xFF00)
							error("Script-Patcher: 8 bit selector required, game uses 16 bit selector\nFaulty patch: '%s'", signatureDescription);
					}
				}
				curValue = curSelector;
			}
			magicOffset--;
			if (magicDWordLeft) {
				// Remember current byte for Magic DWORD
				magicDWord[4 - magicDWordLeft] = (byte)curValue;
				magicDWordLeft--;
				if (!magicDWordLeft) {
					// Magic DWORD is now known, convert to platform specific byte order
					calculatedMagicDWord = READ_UINT32(magicDWord);
				}
			}
			break;
		}
		case PATCH_CODE_GETORIGINALBYTE:
		case PATCH_CODE_GETORIGINALUINT16: {
			signatureData++; // skip over extra uint16
			break;
		}
		default:
			break;
		}
		signatureData++;
		curWord = *signatureData;
	}

	if (magicDWordLeft)
		error("Script-Patcher: Magic-DWORD beyond End-Of-Signature\nFaulty patch: '%s'", signatureDescription);
	if (magicDWordIncluded) {
		if (!calculatedMagicDWord) {
			error("Script-Patcher: Magic-DWORD not specified in signature\nFaulty patch: '%s'", signatureDescription);
		}
	}
}

// This method calculates the magic DWORD for each entry in the signature table
//  and it also initializes the selector table for selectors used in the signatures/patches of the current game
void ScriptPatcher::initSignature(const SciScriptPatcherEntry *patchTable) {
	const SciScriptPatcherEntry *curEntry = patchTable;
	SciScriptPatcherRuntimeEntry *curRuntimeEntry;
	int patchEntryCount = 0;

	// Count entries and allocate runtime data
	while (curEntry->signatureData) {
		patchEntryCount++; curEntry++;
	}
	_runtimeTable = new SciScriptPatcherRuntimeEntry[patchEntryCount];
	memset(_runtimeTable, 0, sizeof(SciScriptPatcherRuntimeEntry) * patchEntryCount);

	curEntry = patchTable;
	curRuntimeEntry = _runtimeTable;
	while (curEntry->signatureData) {
		// process signature
		curRuntimeEntry->active = curEntry->defaultActive;
		curRuntimeEntry->magicDWord = 0;
		curRuntimeEntry->magicOffset = 0;

		// We verify the signature data and remember the calculated magic DWord from the signature data
		calculateMagicDWordAndVerify(curEntry->description, curEntry->signatureData, true, curRuntimeEntry->magicDWord, curRuntimeEntry->magicOffset);
		// We verify the patch data
		calculateMagicDWordAndVerify(curEntry->description, curEntry->patchData, false, curRuntimeEntry->magicDWord, curRuntimeEntry->magicOffset);

		curEntry++; curRuntimeEntry++;
	}
}

// This method enables certain patches
//  It's used for patches, which are not meant to get applied all the time
void ScriptPatcher::enablePatch(const SciScriptPatcherEntry *patchTable, const char *searchDescription) {
	const SciScriptPatcherEntry *curEntry = patchTable;
	SciScriptPatcherRuntimeEntry *runtimeEntry = _runtimeTable;
	int searchDescriptionLen = strlen(searchDescription);
	int matchCount = 0;

	while (curEntry->signatureData) {
		if (strncmp(curEntry->description, searchDescription, searchDescriptionLen) == 0) {
			// match found, enable patch
			runtimeEntry->active = true;
			matchCount++;
		}
		curEntry++; runtimeEntry++;
	}

	if (!matchCount)
		error("Script-Patcher: no patch found to enable");
}

void ScriptPatcher::processScript(uint16 scriptNr, SciSpan<byte> scriptData) {
	const SciScriptPatcherEntry *signatureTable = NULL;
	const SciScriptPatcherEntry *curEntry = NULL;
	SciScriptPatcherRuntimeEntry *curRuntimeEntry = NULL;
	const Sci::SciGameId gameId = g_sci->getGameId();

	switch (gameId) {
	case GID_CAMELOT:
		signatureTable = camelotSignatures;
		break;
	case GID_ECOQUEST:
		signatureTable = ecoquest1Signatures;
		break;
	case GID_ECOQUEST2:
		signatureTable = ecoquest2Signatures;
		break;
	case GID_FANMADE:
		signatureTable = fanmadeSignatures;
		break;
	case GID_FREDDYPHARKAS:
		signatureTable = freddypharkasSignatures;
		break;
	case GID_HOYLE4:
		signatureTable = hoyle4Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_HOYLE5:
		if (g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 100)) &&
			g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 700)))
			signatureTable = hoyle5Signatures;
		else if (g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 100)) &&
			    !g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 700)))
			signatureTable = hoyle5ChildrensCollectionSignatures;
		else if (!g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 100)) &&
			      g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 700)))
			signatureTable = hoyle5BridgeSignatures;
		break;
	case GID_GK1:
		signatureTable = gk1Signatures;
		break;
	case GID_GK2:
		signatureTable = gk2Signatures;
		break;
#endif
	case GID_ICEMAN:
		signatureTable = icemanSignatures;
		break;
	case GID_ISLANDBRAIN:
		signatureTable = islandBrainSignatures;
		break;
	case GID_KQ4:
		signatureTable = kq4Signatures;
		break;
	case GID_KQ5:
		signatureTable = kq5Signatures;
		break;
	case GID_KQ6:
		signatureTable = kq6Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_KQ7:
		signatureTable = kq7Signatures;
		break;
#endif
	case GID_LAURABOW:
		signatureTable = laurabow1Signatures;
		break;
	case GID_LAURABOW2:
		signatureTable = laurabow2Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_LIGHTHOUSE:
		signatureTable = lighthouseSignatures;
		break;
#endif
	case GID_LONGBOW:
		signatureTable = longbowSignatures;
		break;
	case GID_LSL1:
		signatureTable = larry1Signatures;
		break;
	case GID_LSL2:
		signatureTable = larry2Signatures;
		break;
	case GID_LSL3:
		signatureTable = larry3Signatures;
		break;
	case GID_LSL5:
		signatureTable = larry5Signatures;
		break;
	case GID_LSL6:
		signatureTable = larry6Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_LSL6HIRES:
		signatureTable = larry6HiresSignatures;
		break;
	case GID_LSL7:
		signatureTable = larry7Signatures;
		break;
#endif
	case GID_MOTHERGOOSE256:
		signatureTable = mothergoose256Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_MOTHERGOOSEHIRES:
		signatureTable = mothergooseHiresSignatures;
		break;

	case GID_PHANTASMAGORIA:
		signatureTable = phantasmagoriaSignatures;
		break;

	case GID_PHANTASMAGORIA2:
		signatureTable = phantasmagoria2Signatures;
		break;
#endif
	case GID_PEPPER:
		signatureTable = pepperSignatures;
		break;
	case GID_PQ1:
		signatureTable = pq1vgaSignatures;
		break;
	case GID_PQ3:
		signatureTable = pq3Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_PQ4:
		signatureTable = pq4Signatures;
		break;
	case GID_PQSWAT:
		signatureTable = pqSwatSignatures;
		break;
#endif
	case GID_QFG1:
		signatureTable = qfg1egaSignatures;
		break;
	case GID_QFG1VGA:
		signatureTable = qfg1vgaSignatures;
		break;
	case GID_QFG2:
		signatureTable = qfg2Signatures;
		break;
	case GID_QFG3:
		signatureTable = qfg3Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_QFG4:
		signatureTable = qfg4Signatures;
		break;
	case GID_RAMA:
		signatureTable = ramaSignatures;
		break;
	case GID_SHIVERS:
		signatureTable = shiversSignatures;
		break;
#endif
	case GID_SQ1:
		signatureTable = sq1vgaSignatures;
		break;
	case GID_SQ3:
		signatureTable = sq3Signatures;
		break;
	case GID_SQ4:
		signatureTable = sq4Signatures;
		break;
	case GID_SQ5:
		signatureTable = sq5Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_SQ6:
		signatureTable = sq6Signatures;
		break;
	case GID_TORIN:
		signatureTable = torinSignatures;
		break;
#endif
	default:
		break;
	}

	if (signatureTable) {
		_isMacSci11 = (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_1_1);

		if (!_runtimeTable) {
			// signature table needs to get initialized (Magic DWORD set, selector table set)
			initSignature(signatureTable);

			// Do additional game-specific initialization
			switch (gameId) {
			case GID_FREDDYPHARKAS:
				if (_isMacSci11 && !g_sci->getResMan()->testResource(ResourceId(kResourceTypeView, 844))) {
					enablePatch(signatureTable, "Mac: skip broken hop singh scene");
				}
				break;
			case GID_GK1:
				if (_isMacSci11 || g_sci->isCD()) {
					enablePatch(signatureTable, "cd/mac: Narrator lockup fix");
				} else {
					enablePatch(signatureTable, "floppy: Narrator lockup fix");
				}

				if (_isMacSci11 && !g_sci->getResMan()->testResource(ResourceId(kResourceTypeView, 56))) {
					enablePatch(signatureTable, "mac: fix missing talisman view");
				}
				break;
			case GID_GK2:
				// Enable subtitle compatibility if a sync resource is present
				if (g_sci->getResMan()->testResource(ResourceId(kResourceTypeSync, 10))) {
					enablePatch(signatureTable, "subtitle patch compatibility");
				}
				break;
			case GID_KQ5:
				if (g_sci->_features->useAltWinGMSound()) {
					// See the explanation in the kq5SignatureWinGMSignals comment
					enablePatch(signatureTable, "Win: GM Music signal checks");
				}
				// enable a patch for dealing with a buggy sound that appears in
				//  various forms in a sporadic set of versions
				if (g_sci->getLanguage() == Common::FR_FRA ||
					g_sci->getLanguage() == Common::IT_ITA ||
					g_sci->getPlatform() == Common::kPlatformMacintosh ||
					(g_sci->getPlatform() == Common::kPlatformAmiga && g_sci->getLanguage() == Common::EN_ANY)) {
					enablePatch(signatureTable, "Crispin intro signal");
				}
				break;
			case GID_KQ6:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for King's Quest 6
					enablePatch(signatureTable, "CD: audio + text support");
				}
				if (_isMacSci11) {
					// Enables Mac-only patch to work around missing pic
					enablePatch(signatureTable, "Mac: Drink Me pic");
				}
				break;
			case GID_LAURABOW2:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for Laura Bow 2
					enablePatch(signatureTable, "CD: audio + text support");
				}
				break;
			case GID_QFG4:
				if (g_sci->isCD()) {
					// Floppy doesn't need this
					enablePatch(signatureTable, "CD: fix rope during Igor rescue (1/2)");
					enablePatch(signatureTable, "CD: fix rope during Igor rescue (2/2)");

					// Similar signatures that patch with different addresses/offsets
					enablePatch(signatureTable, "CD: fix guild tunnel access (3/3)");
					enablePatch(signatureTable, "CD: fix crest bookshelf");
					enablePatch(signatureTable, "CD: fix peer bats, upper door (2/2)");
				} else {
					enablePatch(signatureTable, "Floppy: fix guild tunnel access (3/3)");
					enablePatch(signatureTable, "Floppy: fix crest bookshelf");
					enablePatch(signatureTable, "Floppy: fix peer bats, upper door (2/2)");
				}
				break;
			case GID_SQ3:
				if (g_sci->getLanguage() == Common::HE_ISR) {
					enablePatch(signatureTable, "Hebrew: Replace name in status bar");
					enablePatch(signatureTable, "Hebrew: Replace 'Enter input' prompt");
				}
				break;
			case GID_SQ4:
				// Enable the dress-purchase flag fixes for English Amiga only.
				//  One of these patches is applied to scripts that are the same as those
				//  in other versions which must not be patched, including German Amiga.
				if (g_sci->getPlatform() == Common::kPlatformAmiga) {
					// Check for buggy Sock's (room 371) script from English Amiga version
					Resource *socksScript = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 371), false);
					if (socksScript && socksScript->size() == 14340) {
						enablePatch(signatureTable, "Amiga: dress purchase flag check fix");
						enablePatch(signatureTable, "Amiga: dress purchase flag clear fix");
					}
				}

				if (g_sci->isCD() && !g_sci->getResMan()->testResource(ResourceId(kResourceTypeScript, 271))) {
					enablePatch(signatureTable, "CD: disable timepod code for removed room");
				}
				break;
			default:
				break;
			}
		}

		curEntry = signatureTable;
		curRuntimeEntry = _runtimeTable;

		while (curEntry->signatureData) {
			if ((scriptNr == curEntry->scriptNr) && (curRuntimeEntry->active)) {
				int32 foundOffset = 0;
				int16 applyCount = curEntry->applyCount;
				do {
					foundOffset = findSignature(curEntry, curRuntimeEntry, scriptData);
					if (foundOffset != -1) {
						// found, so apply the patch
						debugC(kDebugLevelPatcher, "Script-Patcher: '%s' on script %d offset %d", curEntry->description, scriptNr, foundOffset);
						applyPatch(curEntry, scriptData, foundOffset);
					}
					applyCount--;
				} while ((foundOffset != -1) && (applyCount));
			}
			curEntry++; curRuntimeEntry++;
		}
	}
}

} // End of namespace Sci
