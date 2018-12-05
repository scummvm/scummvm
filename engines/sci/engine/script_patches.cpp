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
	"localize",     // Freddy Pharkas
	"put",          // Police Quest 1 VGA
	"say",          // Quest For Glory 1 VGA
	"solvePuzzle",  // Quest For Glory 3
	"timesShownID", // Space Quest 1 VGA
	"startText",    // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"startAudio",   // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"modNum",       // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"has",          // King's Quest 6, GK1
	"modeless",     // King's Quest 6 CD
	"cycler",       // Space Quest 4 / system selector
	"setLoop",      // Laura Bow 1 Colonel's Bequest, QFG4
	"ignoreActors", // Laura Bow 1 Colonel's Bequest
#ifdef ENABLE_SCI32
	"newWith",      // SCI2 array script
	"scrollSelections", // GK2
	"posn",         // SCI2 benchmarking script
	"detailLevel",  // GK2 benchmarking
	"view",         // RAMA benchmarking, GK1
	"fade",         // Shivers
	"handleEvent",  // Shivers
	"test",         // Torin
	"get",          // Torin, GK1
	"newRoom",      // GK1
	"normalize",    // GK1
	"signal",       // GK1
	"set",          // Torin
	"clear",        // Torin
	"masterVolume", // SCI2 master volume reset
	"data",         // Phant2
	"format",       // Phant2
	"setSize",      // Phant2
	"setCel",       // Phant2, GK1
	"iconV",        // Phant2
	"update",       // Phant2
	"xOff",         // Phant2
	"fore",         // KQ7
	"back",         // KQ7
	"font",         // KQ7
	"setScale",     // LSL6hires, QFG4
	"setScaler",    // LSL6hires, QFG4
	"readWord",     // LSL7, Phant1, Torin
	"points",       // PQ4
	"select",       // PQ4
	"addObstacle",  // QFG4
	"handle",       // RAMA
	"saveFilePtr",  // RAMA
	"priority",     // RAMA
	"plane",        // RAMA
	"state",        // RAMA
	"getSubscriberObj", // RAMA
	"setLooper",    // QFG4
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
	SELECTOR_localize,
	SELECTOR_put,
	SELECTOR_say,
	SELECTOR_solvePuzzle,
	SELECTOR_timesShownID,
	SELECTOR_startText,
	SELECTOR_startAudio,
	SELECTOR_modNum,
	SELECTOR_has,
	SELECTOR_modeless,
	SELECTOR_cycler,
	SELECTOR_setLoop,
	SELECTOR_ignoreActors
#ifdef ENABLE_SCI32
	,
	SELECTOR_newWith,
	SELECTOR_scrollSelections,
	SELECTOR_posn,
	SELECTOR_detailLevel,
	SELECTOR_view,
	SELECTOR_fade,
	SELECTOR_handleEvent,
	SELECTOR_test,
	SELECTOR_get,
	SELECTOR_newRoom,
	SELECTOR_normalize,
	SELECTOR_signal,
	SELECTOR_set,
	SELECTOR_clear,
	SELECTOR_masterVolume,
	SELECTOR_data,
	SELECTOR_format,
	SELECTOR_setSize,
	SELECTOR_setCel,
	SELECTOR_iconV,
	SELECTOR_update,
	SELECTOR_xOff,
	SELECTOR_fore,
	SELECTOR_back,
	SELECTOR_font,
	SELECTOR_setScale,
	SELECTOR_setScaler,
	SELECTOR_readWord,
	SELECTOR_points,
	SELECTOR_select,
	SELECTOR_addObstacle,
	SELECTOR_handle,
	SELECTOR_saveFilePtr,
	SELECTOR_priority,
	SELECTOR_plane,
	SELECTOR_state,
	SELECTOR_getSubscriberObj,
	SELECTOR_setLooper
#endif
};

#ifdef ENABLE_SCI32
// It is not possible to change the directory for ScummVM save games, so disable
// the "change directory" button in the standard save dialogue
static const uint16 sci2ChangeDirSignature[] = {
	0x72, SIG_ADDTOOFFSET(+2), // lofsa changeDirI
	0x4a, SIG_UINT16(0x04),    // send 4
	SIG_MAGICDWORD,
	0x36,                      // push
	0x35, 0xF7,                // ldi $f7
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
	0x4a, 0x8,                     // send $8
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
// Applies to at least: GK1 floppy, PQ4 floppy, PQ4CD, LSL6hires, Phant1,
// Shivers, SQ6
static const uint16 sci2BenchmarkSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(init),  // pushi init
	0x76,                        // push0
	0x38, SIG_SELECTOR16(posn),  // pushi posn
	SIG_END
};

static const uint16 sci2BenchmarkPatch[] = {
	0x7a,                         // push2
	0x38, SIG_UINT16(64908),      // pushi 64908
	0x76,                         // push0
	0x43, 0x03, SIG_UINT16(0x04), // callk DisposeScript[3], 4
	0x48,                         // ret
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
	0x81, 0x01,                         // lag 1
	0x4a, SIG_UINT16(0x06),             // send 6
	SIG_END
};

static const uint16 sci2VolumeResetPatch[] = {
	0x32, PATCH_UINT16(8), // jmp 8 [past volume reset]
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
	0x38, SIG_UINT16(3),                // pushi 03
	0x39, 0x0e,                         // pushi 0e
	0x8d, 0x0b,                         // lst temp[0b]
	0x36,                               // push
	0x54, SIG_UINT16(0x000a),           // self 0a
	0x33, 0x0b,                         // jmp to [ret]
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
// peepingTom calls the object behind global b9h. This global variable is
//  properly initialized, when walking there manually (method fawaz::doit).
// When you instead walk there automatically (method fawaz::handleEvent), that
//  global isn't initialized, which then results in the Oops-message in Sierra SCI
//  and an error message in ScummVM/SCI.
//
// We fix the script by patching in a jump to the proper code inside fawaz::doit.
// Responsible method: fawaz::handleEvent
// Fixes bug: #6402
static const uint16 camelotSignaturePeepingTom[] = {
	0x72, SIG_MAGICDWORD, SIG_UINT16(0x077e), // lofsa fawaz <-- start of proper initializion code
	0xa1, 0xb9,                      // sag b9h
	SIG_ADDTOOFFSET(+571),           // skip 571 bytes
	0x39, 0x7a,                      // pushi 7a <-- initialization code when walking automatically
	0x78,                            // push1
	0x7a,                            // push2
	0x38, SIG_UINT16(0x00a9), // + 0xa9, 0x00,   // pushi 00a9 - script 169
	0x78,                            // push1
	0x43, 0x02, 0x04,                // call kScriptID
	0x36,                            // push
	0x81, 0x00,                      // lag 00
	0x4a, 0x06,                      // send 06
	0x32, SIG_UINT16(0x0520),        // jmp [end of fawaz::handleEvent]
	SIG_END
};

static const uint16 camelotPatchPeepingTom[] = {
	PATCH_ADDTOOFFSET(+576),
	0x32, PATCH_UINT16(0xfdbd),      // jmp to fawaz::doit / properly init peepingTom code
	PATCH_END
};

//         script, description,                                       signature                   patch
static const SciScriptPatcherEntry camelotSignatures[] = {
	{ true,    62, "fix peepingTom Sierra bug",                    1, camelotSignaturePeepingTom, camelotPatchPeepingTom },
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
	0x45, 0x01, 0x00,                // callb export1 from script 0 (switching control off)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0122),        // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, SIG_SELECTOR8(init),       // pushi "init"
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
	0x45, 0x01, 0x00,                // callb export1 from script 0 (switching control off)
	0x38, PATCH_UINT16(0x0122),      // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, PATCH_SELECTOR8(init),     // pushi "init"
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

//          script, description,                                      signature                      patch
static const SciScriptPatcherEntry ecoquest1Signatures[] = {
	{  true,   660, "CD: bad messagebox and freeze",               1, ecoquest1SignatureStayAndHelp, ecoquest1PatchStayAndHelp },
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
	0x38, PATCH_UINT16(0x2631),      // pushi 2631
	0x39, 0x6a,                      // pushi 6a
	0x39, 0x64,                      // pushi 64
	0x43, 0x1b, 0x14,                // call kDisplay
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
	0x2f, 0x02,                      // bt [to pushi 07]
	0x3a,                            // toss
	0x48,                            // ret
	0x38, PATCH_UINT16(0x0007),      // pushi 07 (parameter count) (waste 1 byte)
	0x39, 0x0b,                      // push (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x38, PATCH_UINT16(0x0017),      // pushi 17 (color) (waste 1 byte)
	0x43, 0x6c, 0x0e,                // call kGraph
	0x38, PATCH_UINT16(0x0005),      // pushi 05 (parameter count) (waste 1 byte)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // call kGraph
	PATCH_END
};

// ===========================================================================
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
	0x43, 0x1b, 0x14,                // call kDisplay
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
	//0x39, 0x07,                    // pushi 07 (parameter count)
	0x39, 0x0b,                      // push (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x39, 0x17,                      // pushi 17 (color)
	0x43, 0x6c, 0x0e,                // call kGraph
	// The parameter count below should be 5, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,                            // push1 (parameter count)
	//0x39, 0x05,                    // pushi 05 (parameter count)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // call kGraph
	// We are out of bytes to patch at this point,
	// so we skip 494 (0x1EE) bytes to reuse this code:
	// ldi 1e
	// aTop 20
	// jmp 030e (jump to end)
	0x32, PATCH_UINT16(0x01ee),      // skip 494 (0x1EE) bytes
	PATCH_END
};

//          script, description,                                       signature                          patch
static const SciScriptPatcherEntry ecoquest2Signatures[] = {
	{  true,    50, "initial text not removed on ecorder",          1, ecoquest2SignatureEcorder,         ecoquest2PatchEcorder },
	{  true,   333, "initial text not removed on ecorder tutorial", 1, ecoquest2SignatureEcorderTutorial, ecoquest2PatchEcorderTutorial },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Fan-made games
// Attention: Try to make script patches as specific as possible

// CascadeQuest::autosave in script 994 is called various times to auto-save the game.
// The script use a fixed slot "999" for this purpose. This doesn't work in ScummVM, because we do not let
//  scripts save directly into specific slots, but instead use virtual slots / detect scripts wanting to
//  create a new slot.
//
// For this game we patch the code to use slot 99 instead. kSaveGame also checks for Cascade Quest,
//  will then check, if slot 99 is asked for and will then use the actual slot 0, which is the official
//  ScummVM auto-save slot.
//
// Responsible method: CascadeQuest::autosave
// Fixes bug: #7007
static const uint16 fanmadeSignatureCascadeQuestFixAutoSaving[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x03e7),        // pushi 3E7 (999d) -> save game slot 999
	0x74, SIG_UINT16(0x06f8),        // lofss "AutoSave"
	0x89, 0x1e,                      // lsg global[1E]
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
	0x87, 0x01,                      // lap 01
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

//          script, description,                                      signature                                  patch
static const SciScriptPatcherEntry fanmadeSignatures[] = {
	{  true,   994, "Cascade Quest: fix auto-saving",              1, fanmadeSignatureCascadeQuestFixAutoSaving, fanmadePatchCascadeQuestFixAutoSaving },
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
	PATCH_ADDTOOFFSET(1),            // push0 for first code, push1 for second code
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
	0x43, 0x2c, 0x00,                // callk GameIsRestarting <-- add this so that our speed throttler is triggered
	SIG_ADDTOOFFSET(+47),            // skip almost all of inner loop
	0x33, 0xca,                      // jmp [inner loop start]
	PATCH_END
};

//  script 0 of freddy pharkas/CD PointsSound::check waits for a signal and if
//   no signal received will call kDoSound(0xD) which is a dummy in sierra sci
//   and ScummVM and will use acc (which is not set by the dummy) to trigger
//   sound disposal. This somewhat worked in sierra sci, because the sample
//   was already playing in the sound driver. In our case we would also stop
//   the sample from playing, so we patch it out
//   The "score" code is already buggy and sets volume to 0 when playing
// Applies to at least: English PC-CD
// Responsible method: unknown
static const uint16 freddypharkasSignatureScoreDisposal[] = {
	0x67, 0x32,                      // pTos 32 (selector theAudCount)
	0x78,                            // push1
	SIG_MAGICDWORD,
	0x39, 0x0d,                      // pushi 0d
	0x43, 0x75, 0x02,                // call kDoAudio
	0x1c,                            // ne?
	0x31,                            // bnt (-> to skip disposal)
	SIG_END
};

static const uint16 freddypharkasPatchScoreDisposal[] = {
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	PATCH_END
};

//  script 235 of freddy pharkas rm235::init and sEnterFrom500::changeState
//   disable icon 7+8 of iconbar (CD only). When picking up the canister after
//   placing it down, the scripts will disable all the other icons. This results
//   in IconBar::disable doing endless loops even in sierra sci, because there
//   is no enabled icon left. We remove disabling of icon 8 (which is help),
//   this fixes the issue.
// Applies to at least: English PC-CD
// Responsible method: rm235::init and sEnterFrom500::changeState
static const uint16 freddypharkasSignatureCanisterHang[] = {
	0x38, SIG_SELECTOR16(disable),   // pushi disable
	0x7a,                            // push2
	SIG_MAGICDWORD,
	0x39, 0x07,                      // pushi 07
	0x39, 0x08,                      // pushi 08
	0x81, 0x45,                      // lag 45
	0x4a, 0x08,                      // send 08 - call IconBar::disable(7, 8)
	SIG_END
};

static const uint16 freddypharkasPatchCanisterHang[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                            // push1
	PATCH_ADDTOOFFSET(+2),
	0x33, 0x00,                      // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET(+3),
	0x06,                            // send 06 - call IconBar::disable(7)
	PATCH_END
};

//  script 215 of freddy pharkas lowerLadder::doit and highLadder::doit actually
//   process keyboard-presses when the ladder is on the screen in that room.
//   They strangely also call kGetEvent. Because the main User::doit also calls
//   kGetEvent, it's pure luck, where the event will hit. It's the same issue
//   as in QfG1VGA and if you turn dos-box to max cycles, and click around for
//   ego, sometimes clicks also won't get registered. Strangely it's not nearly
//   as bad as in our sci, but these differences may be caused by timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
// Applies to at least: English PC-CD, German Floppy, English Mac
// Responsible method: lowerLadder::doit and highLadder::doit
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
	0x38, SIG_SELECTOR16(localize),
	0x76,                            // push0
	0x4a, 0x04,                      // send 04 - call curEvent::localize
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
// Responsible method: unknown
static const uint16 freddypharkasSignatureMacInventory[] = {
	SIG_MAGICDWORD,
	0x39, 0x23,                      // pushi 23
	0x39, 0x74,                      // pushi 74
	0x78,                            // push1
	0x38, SIG_UINT16(0x0174),        // pushi 0174 (on mac it's actually 0x01, 0x74)
	0x85, 0x15,                      // lat 15
	SIG_END
};

static const uint16 freddypharkasPatchMacInventory[] = {
	0x39, 0x02,                      // pushi 02 (now matches the DOS version)
	PATCH_ADDTOOFFSET(+23),
	0x39, 0x04,                      // pushi 04 (now matches the DOS version)
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry freddypharkasSignatures[] = {
	{  true,     0, "CD: score early disposal",                    1, freddypharkasSignatureScoreDisposal, freddypharkasPatchScoreDisposal },
	{  true,    15, "Mac: broken inventory",                       1, freddypharkasSignatureMacInventory,  freddypharkasPatchMacInventory },
	{  true,   110, "intro scaling workaround",                    2, freddypharkasSignatureIntroScaling,  freddypharkasPatchIntroScaling },
	{  true,   235, "CD: canister pickup hang",                    3, freddypharkasSignatureCanisterHang,  freddypharkasPatchCanisterHang },
	{  true,   320, "ladder event issue",                          2, freddypharkasSignatureLadderEvent,   freddypharkasPatchLadderEvent },
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
	0x38, SIG_SELECTOR16(setScale), // pushi $14b (setScale)
	0x38, SIG_UINT16(0x05),         // pushi 5
	0x51, 0x2c,                     // class 2c (Scaler)
	SIG_END
};

static const uint16 hoyle5PatchSetScale[] = {
	0x38, PATCH_SELECTOR16(setScaler), // pushi $14f (setScaler)
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

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5Signatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   200, "fix setScale calls",                         11, hoyle5SetScaleSignature,          hoyle5PatchSetScale },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	// This entry has been placed so that the broken Poker game is disabled. This game uses an external DLL, PENGIN16.DLL,
	// which is invoked via kWinDLL. We need to reverse the logic in PENGIN16.DLL and call it directly, in order to get this
	// game to work properly. Until then, this game entry will be disabled.
	{  true,   975, "disable Poker",                               1, hoyle5SignaturePoker,             hoyle5PatchDisableGame },
	SCI_SIGNATUREENTRY_TERMINATOR
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5ChildrensCollectionSignatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   200, "fix setScale calls",                         11, hoyle5SetScaleSignature,          hoyle5PatchSetScale },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
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
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true,   975, "disable Gin Rummy",                           1, hoyle5SignatureGinRummy,          hoyle5PatchDisableGame },
	{  true,   975, "disable Cribbage",                            1, hoyle5SignatureCribbage,          hoyle5PatchDisableGame },
	{  true,   975, "disable Klondike",                            1, hoyle5SignatureKlondike,          hoyle5PatchDisableGame },
	{  true,   975, "disable Poker",                               1, hoyle5SignaturePoker,             hoyle5PatchDisableGame },
	{  true,   975, "disable Hearts",                              1, hoyle5SignatureHearts,            hoyle5PatchDisableGame },
	{  true,   975, "disable Backgammon",                          1, hoyle5SignatureBackgammon,        hoyle5PatchDisableGame },
	{  true,   975, "disable Crazy Eights",                        1, hoyle5SignatureCrazyEights,       hoyle5PatchDisableGame },
	{  true,   975, "disable Old Maid",                            1, hoyle5SignatureOldMaid,           hoyle5PatchDisableGame },
	{  true,   975, "disable Checkers",                            1, hoyle5SignatureCheckers,          hoyle5PatchDisableGame },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 1

// `daySixBeignet::changeState(4)` is called when the cop goes outside. It sets
// cycles to 220. This is a CPU-speed dependent value and not usually enough
// time to get to the door, so patch it to 22 seconds.
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

// `sInGateWithPermission::changeState(0)` is called whenever the player walks
// through the swinging door. On day 6, when the cop is outside for the beignet,
// this action will also reset the puzzle timer so the player has 200 cycles to
// get through the area before the cop returns. This is a CPU-speed dependent
// value and not usually enough time to get to the door, so patch it to 20
// seconds instead.
// Applies to at least: English PC-CD, German PC-CD, English Mac
static const uint16 gk1Day6PoliceBeignetSignature2[] = {
	0x72, SIG_ADDTOOFFSET(+2),    // lofsa daySixBeignet
	0x1a,                         // eq?
	0x31, 0x0d,                   // bnt [skip set cycles]
	0x38, SIG_SELECTOR16(cycles), // pushi (cycles)
	0x78,                         // push1
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0xc8),       // pushi 200
	0x72,                         // lofsa
	SIG_END
};

static const uint16 gk1Day6PoliceBeignetPatch2[] = {
	PATCH_ADDTOOFFSET(+6),
	0x38, PATCH_SELECTOR16(seconds), // pushi (seconds)
	0x78,                            // push1
	0x38, PATCH_UINT16(0x14),        // pushi 20
	PATCH_END
};

// `sargSleeping::changeState(8)` is called when the cop falls asleep and sets
// the puzzle timer to 220 cycles. This is CPU-speed dependent and not usually
// enough time to get to the door, so patch it to 22 seconds instead.
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
	0x38, SIG_SELECTOR16(get),          // pushi $200 (get)
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x0e,                         // pushi $e
	0x81, 0x00,                         // lag global[0]
	0x4a, SIG_UINT16(0x06),             // send 6 - GKEgo::get($e)
	// end of giving player drum book code
	0x38, SIG_SELECTOR16(has),          // pushi $202 (has)
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
	0x33, 0x0D,                         // jmp [over the check inventory for drum book code]
	// check inventory for drum book
	0x38, PATCH_SELECTOR16(has),        // pushi $202 (has)
	0x78,                               // push1
	0x39, 0x0e,                         // pushi $e
	0x81, 0x00,                         // lag global[0]
	0x4a, PATCH_UINT16(0x06),           // send 6 - GKEgo::has($e)
	0x2f, 0x23,                         // bt [veve newspaper code] (adjusted, saves 2 bytes)
	PATCH_END
};

// When Gabriel goes to the phone, the script softlocks at
// `startOfDay5::changeState(32)`.
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
	0x35, 0x06,                                // ldi 1
	0x65, PATCH_GETORIGINALBYTEADJUST(+6, +6), // aTop ticks
	PATCH_END
};

// When Gabriel is grabbing a vine 'vineSwing::changeState(1)',
// him saying "I can't believe I'm doing this..." is cut off.
// We change it so the scripts wait for the audio.
// This is not supposed to be applied to the Floppy version.
//
// Applies to at lesat: English PC-CD, German PC-CD, Spanish PC-CD
static const uint16 gk1Day9VineSwingSignature[] = {
	0x38, SIG_UINT16(4),              // pushi $4
	0x51, 0x17,                       // class CT
	0x36,                             // push
	0x39, 0x0b,                       // pushi $b
	0x78,                             // push1
	0x7c,                             // pushSelf
	0x81, 0x00,                       // lag global[$0]
	0x4a, SIG_UINT16(0x20),           // send $20
	0x38, SIG_SELECTOR16(setMotion),  // pushi setMotion
	0x78,                             // push1
	0x76,                             // push0
	0x72, SIG_UINT16(0x0412),         // lofsa guard1
	0x4a, SIG_UINT16(0x06),           // send $6
	0x38, SIG_SELECTOR16(say),        // pushi say
	0x38, SIG_UINT16(0x04),           // pushi $4
	SIG_MAGICDWORD,
	0x39, 0x07,                       // pushi $7
	0x39, 0x08,                       // pushi $8
	0x39, 0x10,                       // pushi $10
	0x78,                             // push1
	0x81, 0x5b,                       // lsg global[$5b]
	0x4a, SIG_UINT16(0x000c),         // send $c
	SIG_END
};

static const uint16 gk1Day9VineSwingPatch[] = {
	0x38, SIG_UINT16(3),              // pushi $3
	0x51, 0x17,                       // class CT
	0x36,                             // push
	0x39, 0x0b,                       // pushi $b
	0x78,                             // push1
	0x81, 0x00,                       // lag global[$0]
	0x4a, SIG_UINT16(0x1e),           // send $20
	0x38, SIG_SELECTOR16(setMotion),  // pushi setMotion
	0x78,                             // push1
	0x76,                             // push0
	0x72, SIG_UINT16(0x0412),         // lofsa guard1
	0x4a, SIG_UINT16(0x06),           // send $6
	0x38, SIG_SELECTOR16(say),        // pushi say
	0x38, SIG_UINT16(0x05),           // pushi $5
	0x39, 0x07,                       // pushi $7
	0x39, 0x08,                       // pushi $8
	0x39, 0x10,                       // pushi $10
	0x78,                             // push1
	0x7c,                             // pushSelf
	0x81, 0x5b,                       // lsg global[$5b]
	0x4a, SIG_UINT16(0x000e),         // send $c
	PATCH_END
};


// In GK1, the `view` selector is used to store view numbers in some cases and
// object references to Views in other cases. `Interrogation::dispose` compares
// an object stored in the `view` selector with a number (which is not valid)
// because its checks are in the wrong order. The check order was fixed in the
// CD version, so just do what the CD version does.
// Applies to at least: English Floppy
// TODO: Check if English Mac is affected too and if this patch applies
static const uint16 gk1InterrogationBugSignature[] = {
	SIG_MAGICDWORD,
	0x65, 0x4c,                      // aTop $4c
	0x67, 0x50,                      // pTos $50
	0x34, SIG_UINT16(0x2710),        // ldi $2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 8  [05a0]
	0x67, 0x50,                      // pTos $50
	0x34, SIG_UINT16(0x2710),        // ldi $2710
	0x04,                            // sub
	0x65, 0x50,                      // aTop $50
	0x63, 0x50,                      // pToa $50
	0x31, 0x15,                      // bnt $15  [05b9]
	0x39, SIG_SELECTOR8(view),       // pushi $e (view)
	0x76,                            // push0
	0x4a, SIG_UINT16(0x04),          // send 4
	0xa5, 0x00,                      // sat 0
	0x38, SIG_SELECTOR16(dispose),   // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa $50
	0x4a, SIG_UINT16(0x04),          // send 4
	0x85, 0x00,                      // lat 0
	0x65, 0x50,                      // aTop $50
	SIG_END
};

static const uint16 gk1InterrogationBugPatch[] = {
	0x65, 0x4c,                      // aTop $4c
	0x63, 0x50,                      // pToa $50
	0x31, 0x15,                      // bnt $15  [05b9]
	0x39, PATCH_SELECTOR8(view),     // pushi $e (view)
	0x76,                            // push0
	0x4a, PATCH_UINT16(0x04),        // send 4
	0xa5, 0x00,                      // sat 00
	0x38, PATCH_SELECTOR16(dispose), // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa $50
	0x4a, PATCH_UINT16(0x04),        // send 4
	0x85, 0x00,                      // lat 0
	0x65, 0x50,                      // aTop $50
	0x67, 0x50,                      // pTos $50
	0x34, PATCH_UINT16(0x2710),      // ldi $2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 8  [05b9]
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
// Fixes bug #10767
static const uint16 gk1HonfourUnlockDoorSignature[] = {
	0x7c,                           // pushSelf
	0x81, 0x5b,                     // lag 5b
	0x4a, SIG_MAGICDWORD,           // send e [ gkMessager:say ... self ]
	SIG_UINT16(0x000e),
	0x38, SIG_UINT16(0x0216),       // push 0216 [ handsOff ]
	SIG_END
};

static const uint16 gk1HonfourUnlockDoorPatch[] = {
	0x76,                           // push0
	0x81, 0x5b,                     // lag 5b
	0x4a, PATCH_UINT16(0x000e),     // send e [ gkMessager:say ... 0 ]
	0x38, PATCH_UINT16(0x0217),     // push 0217 [ handsOn ]
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
// Fixes bug #10797
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

	0x89, 0x0c,                         // lsg 0c [ previous room ]
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
// Fixes bug #10763
static const uint16 gk1Day5MoselyVevePointsSignature[] = {
	0x78,                                   // push1
	0x39, 0x1b,                             // pushi 1b
	0x47, 0x0d, 0x00, SIG_UINT16(0x002),    // calle proc13_0 [ is flag 1b set? ]
	0x30, SIG_UINT16(0x001e),               // bnt 001e [ haven't shown notes yet ]
	0x78,                                   // push1
	0x39, 0x1a,                             // pushi 1a
	0x47, 0x0d, 0x01, SIG_UINT16(0x002),    // calle pro13_1 [ set flag 1a ]
	0x38, SIG_UINT16(0x00f2),               // pushi 00f2 [ say ]
	0x38, SIG_UINT16(0x0005),               // pushi 0005
	0x39, 0x11,                             // pushi 11 [ noun ]
	SIG_MAGICDWORD,
	0x39, 0x10,                             // pushi 10 [ verb ]
	0x39, 0x38,                             // pushi 38 [ cond ]
	0x76,                                   // push0
	0x7c,                                   // pushSelf
	0x81, 0x5b,                             // lag 5b [ GkMessager ]
	0x4a, SIG_UINT16(0x000e),               // send 000e [ GkMessager:say ]
	0x32, SIG_UINT16(0x0013),               // jmp 0013
	0x38, SIG_UINT16(0x00f2),               // pushi 00f2 [ say ]
	SIG_END
};

static const uint16 gk1Day5MoselyVevePointsPatch[] = {
	0x38, SIG_UINT16(0x00f2),               // pushi 00f2 [ say ]
	0x39, 0x05,                             // pushi 05
	0x39, 0x11,                             // pushi 11 [ noun ]
	0x39, 0x10,                             // pushi 10 [ verb ]
	0x78,                                   // push1
	0x39, 0x1b,                             // pushi 1b
	0x47, 0x0d, 0x00, SIG_UINT16(0x002),    // calle proc13_0 [ is flag 1b set? ]
	0x31, 0x20,                             // bnt 20 [ pushi 37, continue GkMessager:say ]
	0x38, SIG_UINT16(0x02fa),               // pushi 02fa [ getPoints ]
	0x7a,                                   // push2
	0x38, SIG_UINT16(0xfc19),               // pushi fc19 [ no flag ]
	0x7a,                                   // push2 [ 2 points ]
	0x81, 0x00,                             // lag 0
	0x4a, SIG_UINT16(0x0008),               // send 8 [ GKEgo:getPoints -999 2 ]
	0x78,                                   // push1
	0x39, 0x1a,                             // pushi 1a
	0x47, 0x0d, 0x01, SIG_UINT16(0x002),    // calle pro13_1 [ set flag 1a ]
	0x39, 0x38,                             // pushi 38 [ cond ]
	0x33, 0x09,                             // jmp 9 [ continue GkMessager:say ]
	PATCH_END
};

// GK1 english pc floppy has a missing message when showing certain items to
//  Magentia in room 290 such as the flashlight. This triggers an error message.
//  We fix this by passing GkMessager:say the correct cond as later versions do.
//
// Applies to: English PC Floppy 1.0
// Responsible method: magentia:doVerb
// Fixes bug #10782
static const uint16 gk1ShowMagentiaItemSignature[] = {
	SIG_MAGICDWORD,
	0x76,                           // push0
	0x39, 0x23,                     // push 23 [ invalid message cond ]
	0x76,                           // push0
	0x81, 0x5b,                     // lag 51 [ GkMessager ]
	SIG_END
};

static const uint16 gk1ShowMagentiaItemPatch[] = {
	PATCH_ADDTOOFFSET(+1),
	0x39, 0x00,                     // push 0 [ "Does this mean anything to you?" ]
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
// Fixes bug #10793
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
	0x81, 0x00,                         // lag 00
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

// this just changes ego's second x coordinate but unfortunately that promotes it to 16 bits
static const uint16 gk1Day5SnakeAttackSignature2[] = {
	SIG_MAGICDWORD,
	0x39, 0x7a,                         // push 7a [ x for second walking loop ]
	0x39, 0x7c,                         // push 7c
	0x38, SIG_SELECTOR16(setCycle),     // pushi setCycle
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag 00
	0x4a, SIG_UINT16(0x0022),           // send 22
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [ end of method ]
	SIG_END
};

static const uint16 gk1Day5SnakeAttackPatch2[] = {
	0x38, PATCH_UINT16(0x008b),         // push 008b [ new x for second walking loop ]
	0x39, 0x7c,                         // push 7c
	0x38, PATCH_SELECTOR16(setCycle),   // pushi setCycle
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag 00
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
//  through the door in sGabeEnters:changeState(5). This requires overwriting
//  the instructions in state 4 which set GKEgo:ignoreActors to 0 but that's
//  okay because normalize does that.
//
// There are two versions of this patch due to two significantly different
//  versions of this script. The first is in english pc floppy prior to
//  Sierra's 1.0b patch and the second is in cd and localized floppies. The
//  script was restructured and the compiler used different sized instructions.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: sGabeEnters:changeState
// Fixes bug #10780
static const uint16 gk1PoliceEgoSpeedFixV1Signature[] = {
	0x31, 0x1f,                         // bnt 1f [ state 5 ]
	SIG_ADDTOOFFSET(+19),
	0x38, SIG_SELECTOR16(ignoreActors), // pushi ignoreActors
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag 0
	0x4a, SIG_UINT16(0x000c),           // send c [ GKEgo:setPri: -1, ignoreActors: 0 ]
	0x33, 0x45,                         // jmp 45 [ end of method ]
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x05,                         // ldi 5
	0x1a,                               // eq?
	0x31, 0x3f,                         // bnt 3f [ end of method ]
	SIG_END
};

static const uint16 gk1PoliceEgoSpeedFixV1Patch[] = {
	0x31, 0x1b,                         // bnt 1b [ state 5 ]
	SIG_ADDTOOFFSET(+19),
	0x81, 0x00,                         // lag 0
	0x4a, PATCH_UINT16(0x0006),         // send 6 [ GKEgo:setPri: -1 ]
	0x3a,                               // toss
	0x48,                               // ret
	0x33, 0x00,                         // jmp 0 [ waste 2 bytes ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi normalize
	0x76,                               // push0
	0x81, 0x00,                         // lag 0
	0x4a, PATCH_UINT16(0x0004),         // send 4 [ GKEgo:normalize ]
	PATCH_END
};

// cd / localized floppy / floppy 1.0b version of the above signature/patch
static const uint16 gk1PoliceEgoSpeedFixV2Signature[] = {
	0x31, 0x27,                         // bnt 27 [ state 5 ]
	SIG_ADDTOOFFSET(+26),
	0x38, SIG_SELECTOR16(ignoreActors), // pushi ignoreActors
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag 0
	0x4a, SIG_UINT16(0x000c),           // send c [ GKEgo:setPri: -1, ignoreActors: 0 ]
	0x32, SIG_UINT16(0x004c),           // jmp 004c [ end of method ]
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x05,                         // ldi 5
	0x1a,                               // eq?
	0x31, 0x46,                         // bnt 46 [ end of method ]
	SIG_END
};

static const uint16 gk1PoliceEgoSpeedFixV2Patch[] = {
	0x31, 0x24,                         // bnt 24 [ state 5 ]
	SIG_ADDTOOFFSET(+26),
	0x81, 0x00,                         // lag 0
	0x4a, PATCH_UINT16(0x0006),         // send 6 [ GKEgo:setPri: -1 ]
	0x3a,                               // toss
	0x48,                               // ret
	0x32, PATCH_UINT16(0x0000),         // jmp 0 [ waste 3 bytes ]
	0x38, PATCH_SELECTOR16(normalize),  // pushi 0300 [ normalize ]
	0x76,                               // push0
	0x81, 0x00,                         // lag 0
	0x4a, PATCH_UINT16(0x0004),         // send 4 [ GKEgo:normalize ]
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
// Fixes bug #10780
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
	0x81, 0x02,                         // lag 2
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
	0x81, 0x00,                         // lag 0
	0x4a, PATCH_UINT16(0x0004),         // send 4 [ GKEgo:normalize ]
	0x38, PATCH_SELECTOR16(newRoom),    // pushi newRoom
	0x78,                               // push1
	0x38, PATCH_UINT16(0x00c8),         // pushi 00c8 [ map ]
	0x81, 0x02,                         // lag 2
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
// Fixes bug #10787
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
	0x81, 0x00,                 // lag 00
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
	0x81, 0x00,                 // lag 00
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
// Fixes bug #10781
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
// Fixes bugs #9760, #10707
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
	0x81, 0x00,                         // lag 0
	0x4a, SIG_UINT16(0x001e),           // send 1e [ GKEgo: ... setCel: 5, setLoop: 0 ... ]
	0x35, 0x01,                         // ldi 1
	0xa3, 0x00,                         // sal local0 [ local0 = 1, a non-zero value indicates ego is sitting ]
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
	0x81, 0x00,                         // lag 0
	0xa3, 0x00,                         // sal local0 [ setting local0 to a non-zero object instead of 1 saves 2 bytes ]
	0x4a, PATCH_UINT16(0x0020),         // send 20 [ GKEgo: ... cel: 5, setLoop: 0 1 ... ]
	0x33, 0x87,                         // jmp -79 [ add knitting basket obstacle to room ]
	PATCH_END
};

// Using the money on the fortune teller Lorelei is scripted to respond with
//  a different message depending on whether she's sitting or dancing. This
//  feature manages to have three bugs which all occur in Sierra's interpreter.
//
// Bug 1: The script transposes the sitting and dancing responses.
//        We reverse the test so that the right messages are attempted.
//
// Bug 2: The script passes the wrong message tuple when Lorelei is sitting
//        and so a missing message error occurs. We pass the right tuple.
//
// Bug 3: The audio36 resource for message 420 2 32 0 1 has the wrong tuple and
//        so no audio plays when using the money on Lorelei while dancing.
//        This is a CD resource bug which we fix in the audio loader.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: lorelei:doVerb(32)
// Fixes bug #10819
static const uint16 gk1LoreleiMoneySignature[] = {
	0x30, SIG_UINT16(0x000d),           // bnt 000d [ lorelei is sitting ]
	SIG_ADDTOOFFSET(+19),
	SIG_MAGICDWORD,
	0x7a,                               // pushi2   [ noun ]
	0x8f, 0x01,                         // lsp 01   [ verb (32d) ]
	0x39, 0x03,                         // pushi 03 [ cond ]
	SIG_END
};

static const uint16 gk1LoreleiMoneyPatch[] = {
	0x2e, PATCH_UINT16(0x000d),         // bt 000d [ lorelei is dancing ]
	PATCH_ADDTOOFFSET(+22),
	0x39, 0x02,                         // pushi 02 [ correct cond ]
	PATCH_END
};

// Using "Operate" on the fortune teller Lorelei's right chair causes a
//  missing message error when she's standing in english pc floppy.
//  We fix the message tuple as Sierra did in later versions.
//
// Applies to: English PC Floppy 1.0
// Responsible method: chair2:doVerb(8)
// Fixes bug #10820
static const uint16 gk1OperateLoreleiChairSignature[] = {
	// we have to reach far ahead of the doVerb method for a unique byte
	//  sequence to base a signature off of. chair2:doVerb has no unique bytes
	//  and is surrounded by doVerb methods which are duplicates of others.
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x023a),           // lofsa sPickupVeil
	0x36,                               // push
	SIG_ADDTOOFFSET(+913),
	0x39, 0x03,                         // pushi 03 [ cond ]
	0x81, 0x5b,                         // lag 5b [ gkMessager ]
	SIG_END
};

static const uint16 gk1OperateLoreleiChairPatch[] = {
	PATCH_ADDTOOFFSET(+917),
	0x39, 0x07,                         // pushi 07 [ correct cond ]
	PATCH_END
};

// Using the photocopy of the veve on the artist causes a missing message error
//  after giving the sketch from the lake and then the original veve file.
//  This is due to using the wrong verb in the message tuple, which we fix.
//
// Applies to: All PC Floppy and CD versions. TODO: Test Mac, should apply
// Responsible method: artist:doVerb(24)
// Fixes bug #10818
static const uint16 gk1ArtistVeveCopySignature[] = {
	SIG_MAGICDWORD,
	0x39, 0x30,                         // pushi 30 [ verb: original veve file ]
	0x39, 0x1f,                         // pushi 1f [ cond ]
	SIG_END
};

static const uint16 gk1ArtistVeveCopyPatch[] = {
	0x39, 0x18,                         // pushi 18 [ verb: veve photocopy ]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry gk1Signatures[] = {
	{  true,     0, "remove alt+n syslogger hotkey",               1, gk1SysLoggerHotKeySignature,      gk1SysLoggerHotKeyPatch },
	{  true,    51, "fix interrogation bug",                       1, gk1InterrogationBugSignature,     gk1InterrogationBugPatch },
	{  true,   211, "fix day 1 grace phone speech timing",         1, gk1Day1GracePhoneSignature,       gk1Day1GracePhonePatch },
	{  true,   212, "fix day 5 drum book dialogue error",          1, gk1Day5DrumBookDialogueSignature, gk1Day5DrumBookDialoguePatch },
	{  true,   212, "fix day 5 phone softlock",                    1, gk1Day5PhoneFreezeSignature,      gk1Day5PhoneFreezePatch },
	{  true,   230, "fix day 6 police beignet timer issue (1/2)",  1, gk1Day6PoliceBeignetSignature1,   gk1Day6PoliceBeignetPatch1 },
	{  true,   230, "fix day 6 police beignet timer issue (2/2)",  1, gk1Day6PoliceBeignetSignature2,   gk1Day6PoliceBeignetPatch2 },
	{  true,   230, "fix day 6 police sleep timer issue",          1, gk1Day6PoliceSleepSignature,      gk1Day6PoliceSleepPatch },
	{  true,   230, "fix police station ego speed (version 1)",    1, gk1PoliceEgoSpeedFixV1Signature,  gk1PoliceEgoSpeedFixV1Patch },
	{  true,   230, "fix police station ego speed (version 2)",    1, gk1PoliceEgoSpeedFixV2Signature,  gk1PoliceEgoSpeedFixV2Patch },
	{  true,   240, "fix day 5 mosely veve missing points",        1, gk1Day5MoselyVevePointsSignature, gk1Day5MoselyVevePointsPatch },
	{  true,   250, "fix ego speed when exiting drug store",       1, gk1DrugStoreEgoSpeedFixSignature, gk1DrugStoreEgoSpeedFixPatch },
	{  true,   260, "fix day 5 snake attack (1/2)",                1, gk1Day5SnakeAttackSignature1,     gk1Day5SnakeAttackPatch1 },
	{  true,   260, "fix day 5 snake attack (2/2)",                1, gk1Day5SnakeAttackSignature2,     gk1Day5SnakeAttackPatch2 },
	{  true,   280, "fix pathfinding in Madame Cazanoux's house",  1, gk1CazanouxPathfindingSignature,  gk1CazanouxPathfindingPatch },
	{  true,   290, "fix magentia missing message",                1, gk1ShowMagentiaItemSignature,     gk1ShowMagentiaItemPatch },
	{  true,   380, "fix Gran's room obstacles and ego flicker",   1, gk1GranRoomInitSignature,         gk1GranRoomInitPatch },
	{  true,   410, "fix day 2 binoculars lockup",                 1, gk1Day2BinocularsLockupSignature, gk1Day2BinocularsLockupPatch },
	{  true,   410, "fix artist veve photocopy missing message",   1, gk1ArtistVeveCopySignature,       gk1ArtistVeveCopyPatch },
	{  true,   420, "fix lorelei chair missing message",           1, gk1OperateLoreleiChairSignature,  gk1OperateLoreleiChairPatch },
	{  true,   420, "fix lorelei money messages",                  1, gk1LoreleiMoneySignature,         gk1LoreleiMoneyPatch },
	{  true,   710, "fix day 9 vine swing speech playing",         1, gk1Day9VineSwingSignature,        gk1Day9VineSwingPatch },
	{  true,   800, "fix day 10 honfour unlock door lockup",       1, gk1HonfourUnlockDoorSignature,    gk1HonfourUnlockDoorPatch },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 2

// The down scroll button in GK2 jumps up a pixel on mousedown because there is
// a send to scrollSelections using an immediate value 1, which means to scroll
// up by 1 pixel. This patch fixes the send to scrollSelections by passing the
// button's delta instead of 1 in 'ScrollButton::track'.
//
// Applies to at least: English CD 1.00, English Steam 1.01
static const uint16 gk2InvScrollSignature[] = {
	0x7e, SIG_ADDTOOFFSET(2),               // line whatever
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(scrollSelections), // pushi $2c3
	0x78,                                   // push1
	0x78,                                   // push1
	0x63, 0x98,                             // pToa $98
	0x4a, SIG_UINT16(0x06),                 // send 6
	SIG_END
};

static const uint16 gk2InvScrollPatch[] = {
	0x38, PATCH_SELECTOR16(scrollSelections), // pushi $2c3
	0x78,                                     // push1
	0x67, 0x9a,                               // pTos $9a (delta)
	0x63, 0x98,                               // pToa $98
	0x4a, PATCH_UINT16(0x06),                 // send 6
	0x18, 0x18,                               // waste bytes
	PATCH_END
};

// The init code 'GK2::init' that runs when GK2 starts up unconditionally resets
// the music volume to 63, but the game should always use the volume stored in
// ScummVM.
// Applies to at least: English 1.00 CD
static const uint16 gk2VolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x3f, // ldi $3f
	0xa1, 0x4c, // sag $4c (music volume)
	SIG_END
};

static const uint16 gk2VolumeResetPatch[] = {
	0x33, 0x02,  // jmp 2 [past volume changes]
	PATCH_END
};

// GK2 has custom video benchmarking code that needs to be disabled in a subroutine
// which is called from 'GK2::init'; see sci2BenchmarkSignature
// TODO: Patch is not applied to localized versions and needs to get adjusted
static const uint16 gk2BenchmarkSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class Actor
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x39, SIG_SELECTOR8(view), // pushi $e (view)
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0xfdd4),  // pushi 64980
	SIG_END
};

static const uint16 gk2BenchmarkPatch[] = {
	0x38, PATCH_SELECTOR16(detailLevel), // pushi detailLevel
	0x78,                                // push1
	0x38, PATCH_UINT16(399),             // pushi 10000 / 25 - 1
	0x81, 0x01,                          // lag 1
	0x4a, PATCH_UINT16(0x06),            // send 6
	0x34, PATCH_UINT16(10000),           // ldi 10000
	0x48,                                // ret
	PATCH_END
};

//          script, description,                                              signature                         patch
static const SciScriptPatcherEntry gk2Signatures[] = {
	{  true,     0, "disable volume reset on startup",                     1, gk2VolumeResetSignature,          gk2VolumeResetPatch },
	{  true,     0, "disable video benchmarking",                          1, gk2BenchmarkSignature,            gk2BenchmarkPatch },
	{  true,    23, "fix inventory scroll start direction",                1, gk2InvScrollSignature,            gk2InvScrollPatch },
	{  true, 64990, "increase number of save games (1/2)",                 1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",                 1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",                     1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// at least during harpy scene export 29 of script 0 is called in kq5cd and
//  has an issue for those calls, where temp 3 won't get inititialized, but
//  is later used to set master volume. This issue makes sierra sci set
//  the volume to max. We fix the export, so volume won't get modified in
//  those cases.
static const uint16 kq5SignatureCdHarpyVolume[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0191),        // lag global[191h]
	0x18,                            // not
	0x30, SIG_UINT16(0x002c),        // bnt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, SIG_UINT16(0x0191),        // sag global[191h] (setting global 191h to 1)
	0x38, SIG_UINT16(0x017b),        // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume in temp 3)
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
	0xa5, 0x03,                      // sat temp[3] (2 new bytes) -> write to temp 3
	0x80, PATCH_UINT16(0x0191),      // lag global[191h]
	// saving 1 byte due optimization
	0x2e, PATCH_UINT16(0x0023),      // bt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, PATCH_UINT16(0x0191),      // sag global[191h] (setting global 191h to 1)
	0x38, PATCH_UINT16(0x017b),      // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume in temp 3)
	// saving 8 bytes due removing of duplicate code
	0x39, 0x04,                      // pushi 04 (saving 1 byte due swapping)
	0x22,                            // lt? (because we switched values)
	PATCH_END
};

// This is a heap patch, and it modifies the properties of an object, instead
// of patching script code.
//
// The witchCage object in script 200 is broken and claims to have 12
// variables instead of the 8 it should have because it is a Cage.
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

// The multilingual releases of KQ5 hang right at the end during the magic battle with Mordack.
// It seems additional code was added to wait for signals, but the signals are never set and thus
// the game hangs. We disable that code, so that the battle works again.
// This also happened in the original interpreter.
// We must not change similar code, that happens before.

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
	0x32,                            // change BNT into JMP
	PATCH_END
};

// In the final battle, the DOS version uses signals in the music to handle
// timing, while in the Windows version another method is used and the GM
// tracks do not contain these signals.
// The original kq5 interpreter used global 400 to distinguish between
// Windows (1) and DOS (0) versions.
// We replace the 4 relevant checks for global 400 by a fixed true when
// we use these GM tracks.
//
// Instead, we could have set global 400, but this has the possibly unwanted
// side effects of switching to black&white cursors (which also needs complex
// changes to GameFeatures::detectsetCursorType() ) and breaking savegame
// compatibilty between the DOS and Windows CD versions of KQ5.
// TODO: Investigate these side effects more closely.
static const uint16 kq5SignatureWinGMSignals[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0190),        // lag 0x190
	0x18,                            // not
	0x30, SIG_UINT16(0x001b),        // bnt +0x001B
	0x89, 0x57,                      // lsg 0x57
	SIG_END
};

static const uint16 kq5PatchWinGMSignals[] = {
	0x34, PATCH_UINT16(0x0001),      // ldi 0x0001
	PATCH_END
};

//          script, description,                                      signature                  patch
static const SciScriptPatcherEntry kq5Signatures[] = {
	{  true,     0, "CD: harpy volume change",                     1, kq5SignatureCdHarpyVolume,            kq5PatchCdHarpyVolume },
	{  true,   200, "CD: witch cage init",                         1, kq5SignatureWitchCageInit,            kq5PatchWitchCageInit },
	{  true,   124, "Multilingual: Ending glitching out",          3, kq5SignatureMultilingualEndingGlitch, kq5PatchMultilingualEndingGlitch },
	{ false,   124, "Win: GM Music signal checks",                 4, kq5SignatureWinGMSignals,             kq5PatchWinGMSignals },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// When giving the milk bottle to one of the babies in the garden in KQ6 (room
// 480), script 481 starts a looping baby cry sound. However, that particular
// script also has an overriden check method (cryMusic::check). This method
// explicitly restarts the sound, even if it's set to be looped, thus the same
// sound is played twice, squelching all other sounds. We just rip the
// unnecessary cryMusic::check method out, thereby stopping the sound from
// constantly restarting (since it's being looped anyway), thus the normal
// game speech can work while the baby cry sound is heard.
// Fixes bug: #4955
static const uint16 kq6SignatureDuplicateBabyCry[] = {
	SIG_MAGICDWORD,
	0x83, 0x00,                      // lal 00
	0x31, 0x1e,                      // bnt 1e  [07f4]
	0x78,                            // push1
	0x39, 0x04,                      // pushi 04
	0x43, 0x75, 0x02,                // callk DoAudio[75] 02
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
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: KqInv::showSelf
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
	0x38, SIG_SELECTOR16(show),         // pushi "show" ("show" is e1h for KQ6CD)
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
	0x00,                               // neg (either 2000 or 0000 in acc, this will create dfff or ffff) - saves 2 bytes
	0x12,                               // and
	0x65, 0x30,                         // aTop state
	0x38,                               // pushi "show"
	PATCH_GETORIGINALUINT16(+22),
	0x78,                               // push1
	0x87, 0x00,                         // lap param[0]
	0x31, 0x04,                         // bnt [call show using global 0]
	0x8f, 0x01,                         // lsp param[1], save 1 byte total with lsg global[0] combined
	0x33, 0x02,                         // jmp [call show using param 1]
	0x89, 0x00,                         // lsg global[0], save 1 byte total, see above
	0x54, 0x06,                         // self 06 (call x::show)
	0x31,                               // bnt [menu exit code]
	PATCH_GETORIGINALBYTEADJUST(+39, +6),// dynamic offset must be 0x0E for PC and 0x0D for mac
	0x34, PATCH_UINT16(0x2000),         // ldi 2000
	0x12,                               // and
	0x2f, 0x05,                         // bt [to return]
	0x39, 0x39,                         // pushi 39
	0x76,                               // push0
	0x54, 0x04,                         // self 04 (self::doit)
	0x48,                               // ret (saves 2 bytes for PC, 1 byte for mac)
	PATCH_END
};

// The "Drink Me" bottle code doesn't repaint the AddToPics elements to the screen,
//  when Alexander returns back from the effect of the bottle.
//  It's pretty strange that Sierra didn't find this bug, because it occurs when
//  drinking the bottle right on the screen, where the bottle is found.
// This bug also occurs in Sierra SCI.
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: drinkMeScript::changeState
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
	PATCH_ADDTOOFFSET(+5),              // skip to bnt offset
	PATCH_GETORIGINALBYTEADJUST(+5, +13), // adjust jump to [check for 11h code]
	PATCH_ADDTOOFFSET(+162),
	0x39, PATCH_SELECTOR8(doit),        // pushi (doit)
	0x76,                               // push0
	0x81, 0x0a,                         // lag 0a
	0x4a, 0x04,                         // send 04 (call addToPics::doit)
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_ADDTOOFFSET(+8),              // skip to check 11h code
	0x35, 0x10,                         // ldi 10 instead of 11
	PATCH_ADDTOOFFSET(+23),             // skip to check 12h code
	0x35, 0x11,                         // ldi 11 instead of 12
	PATCH_ADDTOOFFSET(+23),             // skip to check 13h code
	0x35, 0x12,                         // ldi 12 instead of 13
	PATCH_END
};

// During the common Game Over cutscene, one of the guys says "Tickets, only",
// but the subtitle says "Tickets, please".
// Normally people wouldn't have noticed, but ScummVM supports audio + subtitles
// in this game at the same time.
// This is caused by a buggy message, which really has this text + audio attached.
// We assume that "Tickets, only" (the audio) is the correct one and there is a
// message with "Tickets, only" in both text and audio.
// We change message 1, 0, 1, 1 to message 5, 0, 0, 2 to fix this issue.
//
// This mismatch also occurs in Sierra SCI.
// Applies to at least: PC-CD
// Responsible method: modeLessScript::changeState(0)
static const uint16 kq6SignatureTicketsOnly[] = {
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	SIG_MAGICDWORD,
	0x31, 0x2b,                         // bnt [skip over state 0]
	0x39, 0x1e,                         // pushi (font) (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, SIG_UINT16(0x009a),           // pushi (posn)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c8),           // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, SIG_UINT16(0x00ab),           // pushi (say)
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
	0x39, 0x1e,                         // pushi (font) (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, PATCH_UINT16(0x009a),         // pushi (posn)
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x00c8),         // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, PATCH_UINT16(0x00ab),         // pushi (say)
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
// Applies to: PC Floppy, PC CD, Mac Floppy
// Responsible method: ribbon:doVerb(1)
// Fixes bug #10801
static const uint16 kq6SignatureLookRibbonFix[] = {
	0x30, SIG_ADDTOOFFSET(+2),          // bnt [ verb != Look ]
	0x38, SIG_SELECTOR16(has),          // pushi has
	0x78,                               // push1
	0x39, 0x04,                         // pushi 04
	0x81, SIG_MAGICDWORD, 0x00,         // lag 00
	0x4a, 0x06,                         // send 6 [ ego:has 4 (beauty's hair) ]
	0x2e,                               // bt [ continue hair tests ]
	SIG_END
};

static const uint16 kq6PatchLookRibbonFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                               // push1
	0x38, PATCH_UINT16(0x008f),         // pushi 008f
	0x46, PATCH_UINT16(0x0391),         // calle proc913_0 [ is flag 8f set? ]
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
// Responsible method: wallFlowerDance:changeState(9)
// Fixes bug #10811
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

// Audio + subtitles support - SHARED! - used for King's Quest 6 and Laura Bow 2
//  this patch gets enabled, when the user selects "both" in the ScummVM "Speech + Subtitles" menu
//  We currently use global 98d to hold a kMemory pointer.
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
	0x43, SIG_MAGICDWORD, 0x72, 0x04,   // kMemory
	0xa5, 0xc9,                         // sat global[c9]
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport2[] = {
	PATCH_ADDTOOFFSET(+7),
	0xa1, 98,                           // sag global[98d]
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (release audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport3[] = {
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	SIG_MAGICDWORD,
	0x8d, 0xc9,                         // lst temp[c9]
	0x43, 0x72, 0x04,                   // kMemory
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport3[] = {
	PATCH_ADDTOOFFSET(+3),
	0x89, 98,                           // lsg global[98d]
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
	0x33, 0x03,                         // skip over 3 unused bytes
	PATCH_ADDTOOFFSET(+22),
	0x89, 98,                           // lsp global[98d]
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
	0x18, 0x18, 0x18, 0x18,             // waste bytes, do nothing
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
//  Fixes special windows, used for example in the Pawn shop (room 280),
//   when the man in a robe complains about no more mints.
//   Or also in room 300 at the cliffs (aka copy protection), when Alexander falls down the cliffs (closes automatically, but too late).
//   Or in room 210, when Alexander gives the ring to the nightingale (these ones will need a mouse click).
//
//  We have to change even more code, because the game uses PODialog class for
//   text windows and myDialog class for audio. Both are saved to KQ6Print::dialog
//
//  Changing KQ6Print::dialog is disabled for now, because it has side-effects (breaking game over screens)
//
//  Original comment:
//  Sadly PODialog is created during KQ6Print::addText, myDialog is set during
//   KQ6Print::showSelf, which is called much later and KQ6Print::addText requires
//   KQ6Print::dialog to be set, which means we have to set it before calling addText
//   for audio mode, otherwise the user would have to click to get those windows disposed.
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
	0x31, 0x68,                         // adjust jump to reuse audio mode addText-calling code
	PATCH_ADDTOOFFSET(+85),             // right at the MAGIC_DWORD
	// check, if text is supposed to be shown. If yes, skip the follow-up check (param[1])
	0x89, 0x5a,                         // lsg global[5Ah]
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
	0x34, 0x00, 0x00,
	0x35, 0x00,
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
	SIG_ADDTOOFFSET(6),                 // skip reset of bust, eyes and mouth
	0x33, 0x24,                         // jmp [to super class code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport4[] = {
	PATCH_ADDTOOFFSET(+12),
	0x33, PATCH_GETORIGINALBYTEADJUST(+13, -6), // adjust jump to also include setSize call
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
	0x33, 0x79,                         // jmp to new text+dual code
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

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq6Signatures[] = {
	{  true,   481, "duplicate baby cry",                          1, kq6SignatureDuplicateBabyCry,             kq6PatchDuplicateBabyCry },
	{  true,   907, "inventory stack fix",                         1, kq6SignatureInventoryStackFix,            kq6PatchInventoryStackFix },
	{  true,   907, "look ribbon fix",                             1, kq6SignatureLookRibbonFix,                kq6PatchLookRibbonFix },
	{  true,    87, "Drink Me bottle fix",                         1, kq6SignatureDrinkMeFix,                   kq6PatchDrinkMeFix },
	{  true,   640, "Tickets, only fix",                           1, kq6SignatureTicketsOnly,                  kq6PatchTicketsOnly },
	{  true,   480, "CD: wallflower dance fix",                    1, kq6CDSignatureWallFlowerDanceFix,         kq6CDPatchWallFlowerDanceFix },
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
	0x39, SIG_SELECTOR8(fore), // pushi $25 (fore)
	0x78,                      // push1
	0x39, 0x06,                // pushi 6 - sets fore to 6
	0x39, SIG_SELECTOR8(back), // pushi $26 (back)
	0x78,                      // push1
	0x78,                      // push1 - sets back to 1
	0x39, SIG_SELECTOR8(font), // pushi $2a (font)
	0x78,                      // push1
	0x89, 0x16,                // lsg global[$16] - sets font to global[$16]
	0x7a,                      // push2 (y)
	0x78,                      // push1
	0x76,                      // push0 - sets y to 0
	0x54, SIG_UINT16(0x18),    // self $18
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
	0x4a, SIG_UINT16(0x04),             // send 4
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
	0x4a, SIG_UINT16(0x06),             // send 6
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
	0x54, SIG_UINT16(0x04),     // self 4
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
	PATCH_ADDTOOFFSET(+2),       // skip over "pToa initialized code"
	0x2f, 0x0c,                  // bt [skip init code] - saved 1 byte
	0x38,
	PATCH_GETORIGINALUINT16(+6), // pushi (init)
	0x76,                        // push0
	0x54, PATCH_UINT16(0x04),    // self 4
	// additionally set background color here (5 bytes)
	0x34, PATCH_UINT16(0xFF),    // pushi 255
	0x65, 0x2e,                  // aTop back
	// end of init code
	0x8f, 0x00,                  // lsp param[0]
	0x35, 0x01,                  // ldi 1 - this may get optimized to get another byte
	0x1e,                        // gt?
	0x31, 0x04,                  // bnt [set acc to 0]
	0x87, 0x02,                  // lap param[2]
	0x2f, 0x02,                  // bt [over set acc to 0 code]
	PATCH_END
};

// KQ7 has custom video benchmarking code that needs to be disabled in a subroutine
// that is called by KQ7CD::init; see sci2BenchmarkSignature
static const uint16 kq7BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class Actor
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x39, SIG_SELECTOR8(view), // pushi $e (view)
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
// with that item, the game temporarily displays an X cursor, but does this by
// spinning for 90000 cycles inside 'KQ7CD::pragmaFail', which make the duration
// dependent on CPU speed, maxes out the CPU for no reason, and keeps the engine
// from polling for events (which may make the window appear nonresponsive to the OS)
// Applies to at least: KQ7 English 2.00b
static const uint16 kq7PragmaFailSpinSignature[] = {
	0x35, 0x00,               // ldi 0
	0xa5, 0x02,               // sat 2
	SIG_MAGICDWORD,
	0x8d, 0x02,               // lst 2
	0x35, 0x03,               // ldi 3
	0x22,                     // lt?
	SIG_END
};

static const uint16 kq7PragmaFailSpinPatch[] = {
	0x78,                                     // push1
	0x39, 0x12,                               // pushi 18 (~300ms)
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x33, 0x16,                               // jmp to setCursor
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq7Signatures[] = {
	{  true,     0, "disable video benchmarking",                  1, kq7BenchmarkSignature,                    kq7BenchmarkPatch },
	{  true,     0, "remove hardcoded spinloop",                   1, kq7PragmaFailSpinSignature,               kq7PragmaFailSpinPatch },
	{  true,    31, "enable subtitles (1/3)",                      1, kq7SubtitleFixSignature1,                 kq7SubtitleFixPatch1 },
	{  true, 64928, "enable subtitles (2/3)",                      1, kq7SubtitleFixSignature2,                 kq7SubtitleFixPatch2 },
	{  true, 64928, "enable subtitles (3/3)",                      1, kq7SubtitleFixSignature3,                 kq7SubtitleFixPatch3 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Lighthouse

// When going to room 5 (the sierra logo & menu room) from room 380 (the credits
// room), the game tries to clear flags from 0 (global 116 bit 0) to 1423
// (global 204 bit 15), but global 201 is not a flag global (it holds a
// reference to theInvisCursor). This patch stops clearing after 1359 (global
// 200 bit 15). Hopefully that is good enough to not break the game.
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
	0x8d, 0x02,             // lst 2
	0x35, 0x0a,             // ldi 10
	0x24,                   // le?
	0x31, 0x3b,             // bnt [to second digit overflow]
	SIG_ADDTOOFFSET(+4),    // ldi, sat
	0x8d, 0x03,             // lst 3
	0x35, 0x0a,             // ldi 10
	SIG_END
};

static const uint16 lighthouseMemoryCountPatch[] = {
	PATCH_ADDTOOFFSET(+2), // lst 2
	0x35, 0x02,            // ldi 2
	PATCH_ADDTOOFFSET(+9), // le?, bnt, ldi, sat, lst
	0x35, 0x02,            // ldi 2
	PATCH_END
};


//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry lighthouseSignatures[] = {
	{  true,     5, "fix bad globals clear after credits",         1, lighthouseFlagResetSignature,     lighthouseFlagResetPatch },
	{  true,   360, "fix slow computer memory counter",            1, lighthouseMemoryCountSignature,   lighthouseMemoryCountPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Script 210 in the German version of Longbow handles the case where Robin
// hands out the scroll to Marion and then types his name using the hand code.
// The German version script contains a typo (probably a copy/paste error),
// and the function that is used to show each letter is called twice. The
// second time that the function is called, the second parameter passed to
// the function is undefined, thus kStrCat() that is called inside the function
// reads a random pointer and crashes. We patch all of the 5 function calls
// (one for each letter typed from "R", "O", "B", "I", "N") so that they are
// the same as the English version.
// Applies to at least: German floppy
// Responsible method: unknown
// Fixes bug: #5264
static const uint16 longbowSignatureShowHandCode[] = {
	0x78,                            // push1
	0x78,                            // push1
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa (letter, that was typed)
	0x36,                            // push
	0x40, SIG_ADDTOOFFSET(+2),       // call
	0x02,                            // perform the call above with 2 parameters
	0x36,                            // push
	0x40, SIG_ADDTOOFFSET(+2),       // call
	SIG_MAGICDWORD,
	0x02,                            // perform the call above with 2 parameters
	0x38, SIG_SELECTOR16(setMotion), // pushi "setMotion" (0x11c in Longbow German)
	0x39, SIG_SELECTOR8(x),          // pushi "x" (0x04 in Longbow German)
	0x51, 0x1e,                      // class MoveTo
	SIG_END
};

static const uint16 longbowPatchShowHandCode[] = {
	0x39, 0x01,                      // pushi 1 (combine the two push1's in one, like in the English version)
	PATCH_ADDTOOFFSET(+3),           // leave the lofsa call untouched
	// The following will remove the duplicate call
	0x32, PATCH_UINT16(0x0002),      // jmp 02 - skip 2 bytes (the remainder of the first call)
	0x48,                            // ret (dummy, should never be reached)
	0x48,                            // ret (dummy, should never be reached)
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
	0x85, 00,                        // lat temp[0]
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
	0x38, SIG_UINT16(0x0110),        // pushi 0110h - that's what's normally at local[110h]
	0x33, 0x18,                      // jmp [kRandom call]
	// check temp[0] further
	0x78,                            // push1
	0x1a,                            // eq?
	0x31, 0x05,                      // bt [skip over case 1]
	// temp[0] == 1
	0x38, SIG_UINT16(0x002b),        // pushi 002Bh - that's what's normally at local[111h]
	0x33, 0x0F,                      // jmp [kRandom call]
	// temp[0] >= 2
	0x8d, 00,                        // lst temp[0]
	0x35, 0x02,                      // ldi 02
	0x04,                            // sub
	0x9a, SIG_UINT16(0x0112),        // lsli local[112h] -> look up value in 2nd table
	                                 // this may not be needed at all and was just added for safety reasons
	// waste 9 spare bytes
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
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
// Fixes bug #9688
static const uint16 longbowSignatureAmigaPubFix[] = {
	SIG_MAGICDWORD,
	0x67, 0x20,                     // pTos onMeCheck
	0x39, 0x03,                     // pushi 03
	0x39, 0x04,                     // pushi 04
	0x8f, 0x01,                     // lsp 01
	0x8f, 0x02,                     // lsp 02
	0x43, 0x4e, 0x06,               // callk OnControl 6
	SIG_END
};

static const uint16 longbowPatchAmigaPubFix[] = {
	0x38, PATCH_UINT16(0x00c4),     // pushi 00c4 [ onMe, hard-coded for amiga ]
	0x76,                           // push0
	0x59, 0x01,                     // &rest 1
	0x57, 0x2c, 0x04,               // super Feature 4 [ super: onMe &rest ]
	0x48,                           // ret
	PATCH_END
};

//          script, description,                                      signature                     patch
static const SciScriptPatcherEntry longbowSignatures[] = {
	{  true,   210, "hand code crash",                             5, longbowSignatureShowHandCode, longbowPatchShowHandCode },
	{  true,   225, "arithmetic berry bush fix",                   1, longbowSignatureBerryBushFix, longbowPatchBerryBushFix },
	{  true,   530, "amiga pub fix",                               1, longbowSignatureAmigaPubFix,  longbowPatchAmigaPubFix },
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
// We patch the script to use global 90, which seems to be unused in the whole game.
// Applies to at least: English floppy
// Responsible method: rm63Script::handleEvent
// Fixes bug: #6346
static const uint16 larry2SignatureWearParachutePoints[] = {
	0x35, 0x01,                      // ldi 01
	0xa1, SIG_MAGICDWORD, 0x8e,      // sag 8e
	0x80, SIG_UINT16(0x01e0),        // lag 1e0
	0x18,                            // not
	0x30, SIG_UINT16(0x000f),        // bnt [don't give points]
	0x35, 0x01,                      // ldi 01
	0xa0, 0xe0, 0x01,                // sag 1e0
	SIG_END
};

static const uint16 larry2PatchWearParachutePoints[] = {
	PATCH_ADDTOOFFSET(+4),
	0x80, PATCH_UINT16(0x005a),      // lag 5a (global 90)
	PATCH_ADDTOOFFSET(+6),
	0xa0, PATCH_UINT16(0x005a),      // sag 5a (global 90)
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry larry2Signatures[] = {
	{  true,    63, "plane: no points for wearing parachute",      1, larry2SignatureWearParachutePoints, larry2PatchWearParachutePoints },
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
	0x45, 0x0a, 0x04,                   // call export 10 of script 0
	0x78,                               // push1
	0x39, 0x26,                         // pushi 26h (limo arrived flag)
	0x45, 0x07, 0x02,                   // call export 7 of script 0 (sets flag)
	SIG_END
};

static const uint16 larry5PatchGreenCardLimoBug[] = {
	PATCH_ADDTOOFFSET(+8),
	0x34, PATCH_UINT16(0),              // ldi 0000 (dummy)
	0x34, PATCH_UINT16(0),              // ldi 0000 (dummy)
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
	PATCH_UINT16(0x005a),               // change pattiTalker::x to 90
	PATCH_END
};

//          script, description,                                      signature                               patch
static const SciScriptPatcherEntry larry5Signatures[] = {
	{  true,   280, "English-only: fix green card limo bug",       1, larry5SignatureGreenCardLimoBug,        larry5PatchGreenCardLimoBug },
	{  true,   380, "German-only: Enlarge Patti Textbox",          1, larry5SignatureGermanEndingPattiTalker, larry5PatchGermanEndingPattiTalker },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
// Applies to at least: German PC-CD
// Responsible method: unknown
static const uint16 larry6SignatureDeathDialog[] = {
	SIG_MAGICDWORD,
	0x3e, SIG_UINT16(0x0133),        // link 0133 (offset 0x20)
	0x35, 0xff,                      // ldi ff
	0xa3, 0x00,                      // sal 00
	SIG_ADDTOOFFSET(+680),           // [skip 680 bytes]
	0x8f, 0x01,                      // lsp 01 (offset 0x2cf)
	0x7a,                            // push2
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,                            // push
	0x43, 0x7c, 0x0e,                // kMessage[7c] 0e
	SIG_ADDTOOFFSET(+90),            // [skip 90 bytes]
	0x38, SIG_UINT16(0x00d6),        // pushi 00d6 (offset 0x335)
	0x78,                            // push1
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,                            // push
	SIG_ADDTOOFFSET(+76),            // [skip 76 bytes]
	0x38, SIG_UINT16(0x00cd),        // pushi 00cd (offset 0x38b)
	0x39, 0x03,                      // pushi 03
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,
	SIG_END
};

static const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,                // link 0200
	PATCH_ADDTOOFFSET(+687),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_ADDTOOFFSET(+98),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_ADDTOOFFSET(+82),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_END
};

//          script, description,                                      signature                   patch
static const SciScriptPatcherEntry larry6Signatures[] = {
	{  true,    82, "death dialog memory corruption",              1, larry6SignatureDeathDialog, larry6PatchDeathDialog },
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
	0x38, SIG_SELECTOR16(setScale), // pushi $14b (setScale)
	0x38, SIG_UINT16(0x05),         // pushi 5
	0x51, 0x2c,                     // class 2c (Scaler)
	SIG_END
};

static const uint16 larry6HiresSetScalePatch[] = {
	0x38, PATCH_SELECTOR16(setScaler), // pushi $14f (setScaler)
	PATCH_END
};

// The init code that runs when LSL6hires starts up unconditionally resets the
// master music volume to 12 (and the volume dial to 11), but the game should
// always use the volume stored in ScummVM.
// Applies to at least: English CD
static const uint16 larry6HiresVolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x0b,                         // ldi $0b
	0xa1, 0xc2,                         // sag $c2
	SIG_END
};

static const uint16 larry6HiresVolumeResetPatch[] = {
	0x32, PATCH_UINT16(1),  // jmp 1 [past volume change]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry larry6HiresSignatures[] = {
	{  true,    71, "disable volume reset on startup (1/2)",       1, sci2VolumeResetSignature,         sci2VolumeResetPatch },
	{  true,    71, "disable volume reset on startup (2/2)",       1, larry6HiresVolumeResetSignature,  larry6HiresVolumeResetPatch },
	{  true,   270, "fix incorrect setScale call",                 1, larry6HiresSetScaleSignature,     larry6HiresSetScalePatch },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
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
	0x35, 0x41,               // ldi $41
	0xa1, 0xe3,               // sag $e3 (music volume)
	0x7e, SIG_ADDTOOFFSET(2), // line whatever
	0x35, 0x3c,               // ldi $3c
	0xa1, 0xe4,               // sag $e4 (sfx volume)
	0x7e, SIG_ADDTOOFFSET(2), // line whatever
	0x35, 0x64,               // ldi $64
	0xa1, 0xe5,               // sag $e5 (speech volume)
	SIG_END
};

static const uint16 larry7VolumeResetPatch1[] = {
	0x33, 0x10, // jmp [past volume resets]
	PATCH_END
};

// The init code that runs when LSL7 starts up unconditionally resets the
// audio volumes to values stored in larry7.prf, but the game should always use
// the volume stored in ScummVM. This patch is basically identical to the patch
// for Torin, except that they left line numbers in the LSL7 scripts.
// Applies to at least: English CD
static const uint16 larry7VolumeResetSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(readWord), // pushi readWord
	0x76,                           // push0
	SIG_ADDTOOFFSET(6),             // advance file stream
	0xa1, 0xe3,                     // sag $e3 (music volume)
	SIG_ADDTOOFFSET(3),             // line whatever
	SIG_ADDTOOFFSET(10),            // advance file stream
	0xa1, 0xe4,                     // sag $e4 (sfx volume)
	SIG_ADDTOOFFSET(3),             // line whatever
	SIG_ADDTOOFFSET(10),            // advance file stream
	0xa1, 0xe5,                     // sag $e5 (speech volume)
	SIG_END
};

static const uint16 larry7VolumeResetPatch2[] = {
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(3),  // line whatever
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(3),  // line whatever
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
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
	0x38, SIG_SELECTOR16(setPri),    // pushi (setPri)
	SIG_MAGICDWORD,
	0x78,                            // push1
	0x38, SIG_UINT16(500),           // pushi $1f4
	SIG_END
};

static const uint16 larry7MakeCheesePriorityPatch[] = {
	PATCH_ADDTOOFFSET(+4),           // pushi setPri, push1
	0x38, PATCH_UINT16(374),         // pushi $176
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
	0xa1, 0x5a, // sag $5a
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

// Applies to at least: English PC Floppy
// Responsible method: room4::init
static const uint16 laurabow1SignatureEasterEggViewFix[] = {
	0x78,                               // push1
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi "setLoop"
	0x78,                               // push1
	0x39, 0x03,                         // pushi 3 (loop 3, view only has 3 loops)
	SIG_END
};

static const uint16 laurabow1PatchEasterEggViewFix[] = {
	PATCH_ADDTOOFFSET(+7),
	0x02,                            // change loop to 2
	PATCH_END
};

// When oiling the armor or opening the visor of the armor, the scripts
//  first check if Laura/ego is near the armor and if she is not, they will move her
//  to the armor. After that further code is executed.
//
// The current location is checked by a ego::inRect() call.
//
// The given rect for the inRect call inside openVisor::changeState was made larger for Atari ST/Amiga versions.
//  We change the PC version to use the same rect.
//
// Additionally the coordinate, that Laura is moved to, is 152, 107 and may not be reachable depending on where
//  Laura/ego was, when "use oil on helmet of armor" / "open visor of armor" got entered.
//  Bad coordinates are for example 82, 110, which then cause collisions and effectively an endless loop.
//  Game will effectively "freeze" and the user is only able to restore a previous game.
//  This also happened, when using the original interpreter.
//  We change the destination coordinate to 152, 110, which seems to be reachable all the time.
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
	0x38, SIG_UINT16(0x96),             // pushi 0096 (150d)
	0x39, 0x6c,                         // pushi 6c (108d)
	0x38, SIG_UINT16(0x98),             // pushi 0098 (152d)
	SIG_END
};

static const uint16 laurabow1PatchArmorOpenVisorFix[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x68,                         // pushi 68 (104d)   (-2)
	0x38, SIG_UINT16(0x94),             // pushi 0094 (148d) (-2)
	0x39, 0x6f,                         // pushi 6f (111d)   (+3)
	0x38, SIG_UINT16(0x9a),             // pushi 009a (154d) (+2)
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

// In some cases like for example when the player oils the arm of the armor, command input stays
// disabled, even when the player exits fast enough, so that Laura doesn't die.
//
// This is caused by the scripts only enabling control (directional movement), but do not enable command input as well.
//
// This bug also happens, when using the original interpreter.
// And it was fixed for the Atari ST + Amiga versions of the game.
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
	0x72, SIG_UINT16(0x1a59),           // lofsa "Can"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x19a1),           // lofsa "Visor"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x194d),           // lofsa "note"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x18f9),           // lofsa "valve" 18f3
	0x4a, 0x04,                         // send 04
	// new code to enable input as well, needs 9 spare bytes
	0x38, SIG_UINT16(0x00e2),           // canInput
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
// We fix this by changing the test from if global155 equals 11, which it
//  never does, to if it's greater than 11. The global is set to 12 in
//  lightCandles:changeState(11) and it continues to increment as Jeeves'
//  chore sequence progresses, ending with 17.
//
// Applies to: DOS, Amiga, Atari ST
// Responsible method: Room58:init
// Fixes bug #10743
static const uint16 laurabow1SignatureChapelCandlesPersistence[] = {
	SIG_MAGICDWORD,
	0x89, 0x9b,                         // lsg global155 [ Jeeves' act 2 state ]
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
//  This is due to not setting global 195 which tracks who is in the room.
//  We fix this by setting the global as Amiga and Atari ST versions do.
//
// Applies to: DOS only
// Responsible method: Room44:init
// Fixes bug #10742
static const uint16 laurabow1SignatureLillianBedFix[] = {
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x10f8),           // lofsa suit2 [ only matches DOS version ]
	0x4a, 0x14,                         // send 14
	SIG_ADDTOOFFSET(+8),
	0x89, 0x76,                         // lsg global118
	0x35, 0x02,                         // ldi 2
	0x12,                               // and
	0x30, SIG_UINT16(0x000d),           // bnt d [ haven't seen Lillian in study ]
	0x35, 0x01,                         // ldi 1
	SIG_END
};

static const uint16 laurabow1PatchLillianBedFix[] = {
	PATCH_ADDTOOFFSET(+13),
	0x81, 0x76,                         // lag global118
	0x7a,                               // push2
	0x12,                               // and
	0x31, 0x0f,                         // bnt f [ haven't seen Lillian in study ]
	0x35, 0x20,                         // ldi 20 [ Lillian ]
	0xa1, 0xc3,                         // sag global195 [ set Lillian as in the room ]
	PATCH_END
};

// When you tell Lilly about Gertie in room 35, Lilly will then walk to the left and off the screen.
// In case Laura (ego) is in the way, the whole game will basically block and you won't be able
// to do anything except saving + restoring the game.
//
// If this happened already, the player can enter
// "send Lillian ignoreActors 1" inside the debugger to fix this situation.
//
// This issue is very difficult to solve, because Lilly also walks diagonally after walking to the left right
// under the kitchen table. This means that even if we added a few more rectangle checks, there could still be
// spots, where the game would block.
//
// Also the mover "PathOut" is used for Lillian instead of the regular "MoveTo", which would avoid other
// actors by itself.
//
// So instead we set Lilly to ignore other actors during that cutscene, which is the least invasive solution.
//
// Applies to at least: English PC Floppy, English Amiga Floppy, English Atari ST Floppy
// Responsible method: goSee::changeState(1) in script 236
// Fixes bug: (happened during GOG Let's Play)
static const uint16 laurabow1SignatureTellLillyAboutGerieBlockingFix1[] = {
	0x7a,                               // puah2
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00c1),           // pushi 00C1h
	0x38, SIG_UINT16(0x008f),           // pushi 008Fh
	0x38, SIG_SELECTOR16(ignoreActors), // pushi (ignoreActors)
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
	0x39, SIG_ADDTOOFFSET(+1),          // pushi (view)
	0x78,                               // push1
	0x38, SIG_UINT16(0x0203),           // pushi 203h (515d)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi (posn)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c9),           // pushi C9h (201d)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0084),           // pushi 84h (132d)
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa Lillian (different offsets for different platforms)
	0x4a, 0x0e,                         // send 0Eh
	SIG_END
};

static const uint16 laurabow1PatchTellLillyAboutGertieBlockingFix2[] = {
	0x38, PATCH_SELECTOR16(ignoreActors), // pushi (ignoreActors)
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
// Fixes bug #10733
static const uint16 laurabow1SignatureObstacleCollisionLockupsFix[] = {
	SIG_MAGICDWORD,
	0x30, SIG_UINT16(0x002f),           // bnt 2f
	0x38, SIG_UINT16(0x00a3),           // pushi a3 [ startUpd ]
	0x76,                               // push0
	0x54, 0x04,                         // self 4
	0x7a,                               // push2 [ -info- ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param1
	0x4a, 0x04,                         // send 4
	0x36,                               // push
	0x34, SIG_UINT16(0x8000),           // ldi 8000
	0x12,                               // and
	0x30, SIG_UINT16(0x000a),           // bnt a
	0x39, 0x56,                         // pushi 56 [ new ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param1
	0x4a, 0x04,                         // send 4
	0x32, SIG_UINT16(0x0002),           // jmp 2
	0x87, 0x01,                         // lap param1
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
	0x87, 0x01,                         // lap param1
	0x4a, 0x04,                         // send 4
	0x38, PATCH_UINT16(0x8000),         // pushi 8000 [ save 1 byte ]
	0x12,                               // and
	0x31, 0x09,                         // bnt 9 [ save 1 byte ]
	0x39, 0x56,                         // pushi 56 [ new ]
	0x76,                               // push0
	0x87, 0x01,                         // lap param1
	0x4a, 0x04,                         // send 4
	0x33, 0x02,                         // jmp 2 [ save 1 byte ]
	0x87, 0x01,                         // lap param1
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

//          script, description,                                signature                                             patch
static const SciScriptPatcherEntry laurabow1Signatures[] = {
	{  true,     4, "easter egg view fix",                      1, laurabow1SignatureEasterEggViewFix,                laurabow1PatchEasterEggViewFix },
	{  true,    37, "armor open visor fix",                     1, laurabow1SignatureArmorOpenVisorFix,               laurabow1PatchArmorOpenVisorFix },
	{  true,    37, "armor move to fix",                        2, laurabow1SignatureArmorMoveToFix,                  laurabow1PatchArmorMoveToFix },
	{  true,    37, "allowing input, after oiling arm",         1, laurabow1SignatureArmorOilingArmFix,               laurabow1PatchArmorOilingArmFix },
	{  true,    44, "lillian bed fix",                          1, laurabow1SignatureLillianBedFix,                   laurabow1PatchLillianBedFix },
    {  true,    58, "chapel candles persistence",               1, laurabow1SignatureChapelCandlesPersistence,        laurabow1PatchChapelCandlesPersistence },
	{  true,   236, "tell Lilly about Gertie blocking fix 1/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix1, laurabow1PatchTellLillyAboutGertieBlockingFix1 },
	{  true,   236, "tell Lilly about Gertie blocking fix 2/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix2, laurabow1PatchTellLillyAboutGertieBlockingFix2 },
	{  true,   998, "obstacle collision lockups fix",           1, laurabow1SignatureObstacleCollisionLockupsFix,     laurabow1PatchObstacleCollisionLockupsFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Laura Bow 2
//
// Moving away the painting in the room with the hidden safe is problematic
//  for the CD version of the game. safePic::doVerb gets triggered by the mouse-click.
// This method sets local 0 as signal, which is only meant to get handled, when
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
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x18,                               // not
	0x31, 0x21,                         // bnt [exit]
	0x38, SIG_UINT16(0x0283),           // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 32)
	0x4a, 0x04,                         // send 04 (get sHeimlich::room)
	0x36,                               // push
	0x81, 0x0b,                         // lag global[b] (current room)
	0x1c,                               // ne?
	0x31, 0x0e,                         // bnt [exit]
	0x35, 0x00,                         // ldi 00
	0xa3, 0x00,                         // sal local[0] -> reset safePic signal
	SIG_END
};

static const uint16 laurabow2CDPatchPaintingClosing[] = {
	PATCH_ADDTOOFFSET(+2),
	0x3c,                               // dup (1 additional byte)
	0x76,                               // push0
	0x3c,                               // dup (1 additional byte)
	0xab, 0x00,                         // ssl local[0] -> reset safePic signal
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0231),         // pushi 0231h (561)
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x1a,                               // eq?
	0x31, 0x1d,                         // bnt [exit]
	0x38, PATCH_UINT16(0x0283),         // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 32)
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
	0xa1, 0x74,                      // sag 74h
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
	0x38, SIG_UINT16(0x0086),           // push 0086 [ pippin-dead flag ]
	0x45, 0x02, 0x02,                   // call proc0_2 [ is pippin-dead flag set? ]
	0x30, SIG_UINT16(0x0016),           // bnt 0016 [ pippin-dead message ]
	SIG_END
};

static const uint16 laurabow2CDPatchFixYvetteTutResponse[] = {
	PATCH_ADDTOOFFSET(+14),
	0x2e,                               // change to bt
	PATCH_END
};

// When entering the main musem party room (w/ the golden Egyptian head),
// Laura is waslking a bit into the room automatically.
// In case you press a mouse button while this is happening, you will get
// stuck inside that room and won't be able to exit it anymore.
//
// Users, who played the game w/ a previous version of ScummVM can simply
// enter the debugger and then enter "send rm350 script 0:0", which will
// fix the script state.
//
// This is caused by the user controls not being locked at that point.
// Pressing a button will cause the cue from the PolyPath walker to never
// happen, which then causes sEnterSouth to never dispose itself.
//
// User controls are locked in the previous room 335, but controls
// are unlocked by frontDoor::cue.
// We do not want to change this, because it could have side-effects.
// We instead add another LB2::handsOff call inside the script responsible
// for making Laura walk into the room (sEnterSouth::changeState(0).
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
	0x32, PATCH_UINT16(+151),
	SIG_ADDTOOFFSET(+149),             // skip to end of follow-up code
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
	0x32, PATCH_UINT16(0xFF5e),        // jmp [back to start of step 0 processing]
	PATCH_END
};

// second patch, which only inserts pushi handsOff inside our new code
// There is no other way to do this except making 2 full patches for floppy + CD, because handsOff/handsOn
// is not the same value between floppy + CD *and* floppy doesn't even have a vocab, so we can't figure out the id
// by ourselves.
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
	0x38,                              // pushi
	PATCH_GETORIGINALUINT16ADJUST(+96, -1), // get handsOff code and ubstract 1 from it to get handsOn
	PATCH_END
};

// Opening/Closing the east door in the pterodactyl room doesn't
//  check, if it's locked and will open/close the door internally
//  even when it is.
//
// It will get wired shut later in the game by Laura Bow and will be
//  "locked" because of this. We patch in a check for the locked
//  state. We also add code, that will set the "locked" state
//  in case our eastDoor-wired-global is set. This makes the locked
//  state effectively persistent.
//
// Applies to at least: English PC-CD, English PC-Floppy
// Responsible method (CD): eastDoor::doVerb
// Responsible method (Floppy): eastDoor::<noname300>
// Fixes bug: #6458 (partly, see additional patch below)
static const uint16 laurabow2SignatureFixWiredEastDoor[] = {
	0x30, SIG_UINT16(0x0022),           // bnt [skip hand action]
	0x67, SIG_ADDTOOFFSET(+1),          // pTos CD: doorState, Floppy: state
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x08,                         // bnt [close door code]
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x63,                         // pushi 63h
	0x45, 0x04, 0x02,                   // callb export000_4, 02 (sets door-bitflag)
	0x33, 0x06,                         // jmp [super-code]
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x45, 0x03, 0x02,                   // callb export000_3, 02 (resets door-bitflag)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi CD: 011dh, Floppy: 012ch
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[01]
	0x59, 0x02,                         // rest 02
	0x57, SIG_ADDTOOFFSET(+1), 0x06,    // super CD: LbDoor, Floppy: Door, 06
	0x33, 0x0b,                         // jmp [ret]
	SIG_END
};

static const uint16 laurabow2PatchFixWiredEastDoor[] = {
	0x31, 0x23,                         // bnt [skip hand action] (saves 1 byte)
	0x81,   97,                         // lag 97d (get our eastDoor-wired-global)
	0x31, 0x04,                         // bnt [skip setting locked property]
	0x35, 0x01,                         // ldi 01
	0x65, 0x6a,                         // aTop locked (set eastDoor::locked to 1)
	0x63, 0x6a,                         // pToa locked (get eastDoor::locked)
	0x2f, 0x17,                         // bt [skip hand action]
	0x63, PATCH_GETORIGINALBYTE(+4),    // pToa CD: doorState, Floppy: state
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x2f, 0x05,                         // bt [close door code]
	0x45, 0x04, 0x02,                   // callb export000_4, 02 (sets door-bitflag)
	0x33, 0x0b,                         // jmp [super-code]
	0x45, 0x03, 0x02,                   // callb export000_3, 02 (resets door-bitflag)
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
	0xa1, PATCH_UINT16(97),             // sag 97d (set our eastDoor-wired-global)
	PATCH_END
};

// WORKAROUND: Script needed, because of differences in our pathfinding
// algorithm
// It's possible to walk through the closed door in room 448 and enter the crate
//  room before act 5 due to differences in our pathfinding algorithm from Sierra's.
//  Ego is able to stand one pixel farther than Sierra's algorithm allowed and
//  reach the control area behind the door which triggers the room change.
//  We work around this by expanding the closed door's polygon points by one
//  pixel to prevent ego from being able to reach the control area.
//
// Applies to: All Floppy and CD versions
// Responsible method: transomDoor:createPoly
// Fixes bug #9952
static const uint16 laurabow2SignatureFixArmorHallDoorPathfinding[] = {
	SIG_MAGICDWORD,
	0x39, 0x6c,                         // pushi 6c [ x = 108 ]
	0x39, 0x78,                         // pushi 78 [ y = 120 ]
	0x39, 0x58,                         // pushi 58 [ x =  88 ]
	0x38, SIG_UINT16(0x0083),           // pushi 83 [ y = 131 ]
	SIG_END
};

static const uint16 laurabow2PatchFixArmorHallDoorPathfinding[] = {
	0x39, 0x6d,                         // pushi 6d [ x = 109 ]
	PATCH_ADDTOOFFSET(+4),
	0x38, PATCH_UINT16(0x0084),         // pushi 84 [ y = 132 ]
	PATCH_END
};

// The crate room (room 460) in act 5 locks up the game if you enter from the
//  elevator (room 660), swing the hanging crate, and then attempt to leave
//  back through the elevator door.
//
// The state of the wall crate that blocks the elevator door is tracked by
//  setting local0 to 1 when you push it out of the way, but Sierra forgot
//  to reinitialize local0 when you re-enter via the elevator door, causing
//  it to be out of sync with the room state. When you then swing the hanging
//  crate, sSwingIt:changeState(6) tests local0 to see which polygon it should
//  set as the room's obstacle and incorrectly uses the one that blocks both
//  doors. Attempting to use the elevator door then locks up the game as the
//  obstacle polygon prevents ego from reaching the destination.
//
// Someone noticed that local0 wasn't always initialized as shoveCrate:doVerb(4)
//  tests both local0 and the previous room to see if it was the elevator.
//
// We fix this by setting local0 to 1 if the previous room was the elevator
//  during sSwingIt:changeState(3), just in time before it gets tested in
//  sSwingIt:changeState(6). Luckily for us, the handlers for states 3 and 4
//  don't do anything but load zero, making them two consecutive conditions
//  of no-ops. By merging them into a single condition for state 3 we have
//  a whopping 13 bytes available to add code to set local0 correctly.
//
// Affects floppy/cd, all versions, all languages, and occurs in Sierra's interpreter.
// Fixes bug #10701
static const uint16 laurabow2SignatureFixCrateRoomEastDoorLockup[] = {
	0x1a,                               // eq? [ state 3? ]
	SIG_MAGICDWORD,
	0x31, 0x05,                         // bnt [ state 4 ]
	0x35, 0x00,                         // ldi 0
	0x32, SIG_ADDTOOFFSET(2),           // jmp [ exit switch. floppy: b3, cd: bb ]
	0x3c,                               // dup
	0x35, 0x04,                         // ldi 4
	0x1a,                               // eq? [ state 4? ]
	0x31, 0x05,                         // bnt [ state 5 ]
	SIG_END
};

static const uint16 laurabow2PatchFixCrateRoomEastDoorLockup[] = {
	PATCH_ADDTOOFFSET(1),               // eq? [ state 3? ]
	0x31, 0x10,                         // bnt [ state 5 ]
	0x89, 0x0c,                         // lsg global12 [ previous room # ]
	0x34, PATCH_UINT16(0x0294),         // ldi 660d [ elevator room # ]
	0x1a,                               // eq?
	0x8b, 0x00,                         // lsl local0
	0x02,                               // add
	0xa3, 0x00,                         // sal local0 [ local0 += (global12 == 660d) ]
	PATCH_END
};

// Ego can get stuck in the elevator (room 660) by walking to the lower left.
//  This also happens in Sierra's interpreter. We adjust the room's obstacle
//  polygon so that ego can't reach the problematic corner positions.
//  This is a heap patch for the coordinates used in poly2660a:points.
//
// Applies to: All Floppy and CD versions
// Fixes bug #10702
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
// Fixes bug #10729
static const uint16 laurabow2SignatureFixBackRubEastEntranceLockup[] = {
	SIG_MAGICDWORD,
	0x31, 0x0c,                         // bnt 0c    [ unused default verb handler ]
	0x38, PATCH_UINT16(0x0092),         // push 0092 [ setScript/<noname146> ]
	0x78,                               // push1
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa sBackRubInterrupted [ cd: 0c94, floppy: 0c70 ]
	0x36,                               // push
	0x54, 0x06,                         // self 6 [ self:setScript sBackRubInterrupted ]
	0x33, 0x09,                         // jmp 9  [ exit switch ]
	0x38, SIG_ADDTOOFFSET(+2),          // push doVerb/<noname300> [ cd: 011d, floppy: 012c ]
	SIG_END
};

static const uint16 laurabow2PatchFixBackRubEastEntranceLockup[] = {
	PATCH_ADDTOOFFSET(+10),
	0x81, 0x02,                         // lag 2  [ rm550 ]
	0x4a, 0x06,                         // send 6 [ rm550:setScript sBackRubInterrupted ]
	0x32, PATCH_UINT16(0x0006),         // jmp 6  [ exit switch ]
	PATCH_END
};

// LB2 Floppy 1.0 doesn't initialize act 4 correctly when triggered by finding
//  the dagger, causing the act 4 scene in Yvette's (room 550) to lockup the game.
//
// The Yvette/Olympia/Steve scene in act 4 (rooms 550 and 510) expects global111
//  to be set to 11. This global tracks Yvette's state throughout acts 3 and 4
//  and increments as you listen to her conversations and witness her scenes.
//  Some of these are optional and so at the end of act 3 it can be less than 11.
//  rm510:init initializes global111 to 11 when act 4 is triggered by reporting
//  Ernie's death but no such initialization occurs when act 4 is triggered by
//  finding the dagger (rooms 610 and 620). What happens when the global isn't 11
//  depends on its value but some values, such as 8, cause the act 4 scene to
//  never complete and never restore control to the user.
//
// We fix this the way Sierra did in floppy 1.1 and cd versions by setting global111
//  to 11 in actBreak:init when act 4 starts so that it's always initialized.
//
// Applies to: Floppy 1.0 English only
// Responsible method: actBreak:<noname150> which is really init
// Fixes bug: #10716
static const uint16 laurabow2SignatureFixAct4Initialization[] = {
	SIG_MAGICDWORD,
	0xa3, 0x08,                         // sal 8    [ 1.0 floppy only ]
	0x89, 0x0c,                         // lsg 0c   [ previous room ]
	0x34, SIG_UINT16(0x026c),           // ldi 026c [ room 620 ]
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt 5
	0x34, SIG_UINT16(0x0262),           // ldi 0262 [ room 610 ]
	0x33, 0x03,                         // jmp 3
	0x34, SIG_UINT16(0x01fe),           // ldi 01fe [ room 510 ]
	0xa3, 0x00,                         // sal 0    [ local0 = (previous room == 620) ? 610 : 510 ]
	0x33, 0x2d,                         // jmp 2d   [ exit switch ]
	SIG_END
};

static const uint16 laurabow2PatchFixAct4Initialization[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x0b,                         // ldi 0b
	0xa1, 0x6f,                         // sag 6f   [ global111 = 11 ]
	0x89, 0x0c,                         // lsg 0c   [ previous room ]
	0x34, PATCH_UINT16(0x026c),         // ldi 026c [ room 620 ]
	0x1a,                               // eq?
	0x39, 0x64,                         // push 64
	0x06,                               // mul
	0x38, PATCH_UINT16(0x01fe),         // push 01fe
	0x02,                               // add      [ acc = ((previous room == 620) * 100) + 510 ]
	0x32, PATCH_UINT16(0x0013),         // jmp 0013 [ jmp to: sal 0, jmp exit switch ]
	PATCH_END
};

// LB2 Floppy 1.0 attempts to show a non-existent message when using the
//  carbon paper on the desk lamp in room 550.
//
// deskLamp:<noname300>(39), which is really doVerb, attempts to show a message
//  for its own noun (5) instead of the expected noun (45) when the lamp is off.
//  This results in "<Messager> 550: 5, 39, 6, 1 not found".
//
// We fix this the way Sierra did in version 1.1 by passing the correct noun.
//
// Applies to: English floppy 1.000
// Responsible method: deskLamp:<noname300>(39), which is really doVerb
// Fixes bug: #10706
static const uint16 laurabow2SignatureMissingDeskLampMessage[] = {
	SIG_MAGICDWORD,
	0x33, 0x1a,                         // jmp 1a
	0x38, SIG_UINT16(0x0127),           // pushi 127h [ say, hardcoded as we only patch one floppy version ]
	0x39, 0x03,                         // pushi 3
	0x67, 0x1a,                         // pTos 1a [ deskLamp noun (5) ]
	SIG_END
};

static const uint16 laurabow2PatchMissingDeskLampMessage[] = {
	PATCH_ADDTOOFFSET(+7),
	0x39, 0x2d,                         // pushi 45d [ correct message noun ]
	PATCH_END
};

// LB2 Floppy 1.0 doesn't handle events for the inset of the corpse in the armor in room 440,
//  preventing its messages from being displayed.
//
// The inset has messages that respond to look, do, and the magnifying glass, but rm440:<noname133>,
//  which is really handleEvent, never passes events to it. Sierra fixed this in later floppy and
//  cd versions by adding a condition to rm440:handleEvent that first tests if the room has an inset
//  and calls its handleEvent if so.
//
// We fix this by patching rm440:handleEvent to call super:handleEvent if the room has an inset.
//  This is equivalent to Sierra's fix but can be done within the existing space as there is already
//  code to call super:handleEvent on Move events. This patch just extends that condition to also
//  include if an inset exists. This works because Rm:handleEvent contains the same inset handling
//  code that Sierra added to rm440:handleEvent.
//
// This fix is for floppy 1.0 but the signature also matches later floppy versions. That's okay,
//  it's compatible with their fix. Making the signature only match 1.0 would add almost 100 bytes
//  as the closest difference is at the start of the method and the patch is at the end.
//
// Applies to: English floppy 1.000
// Responsible method: rm440:<noname133>, which is really handleEvent
// Fixes bug: #10709
static const uint16 laurabow2SignatureHandleArmorInsetEvents[] = {
	SIG_MAGICDWORD,
	0x31, 0x0b,                         // bnt 0b [ event type isn't Move ]
	0x38, SIG_UINT16(0x0085),           // push 0085 [ <noname113> aka handleEvent ]
	0x78,                               // push1
	0x8f, 0x01,                         // lsp 01
	0x57, 0x7a, 0x06,                   // super LBRoom[7a] 6 [ handle event ]
	0x33, 0x03,                         // jmp 3
	0x35, 0x00,                         // ldi 0 [ event not handled ]
	0x48,                               // ret
	0x48,                               // ret
	SIG_END
};

static const uint16 laurabow2PatchHandleArmorInsetEvents[] = {
	0x2f, 0x04,                         // bt 4 [ event type is Move ]
	0x63, 0x3a,                         // pToa <noname365> aka inset
	0x31, 0x09,                         // bnt 9 [ room has no inset, event not handled ]
	0x38, PATCH_UINT16(0x0085),         // push 0085 [ <noname113> aka handleEvents ]
	0x78,                               // push1
	0x8f, 0x01,                         // lsp 01
	0x57, 0x7a, 0x06,                   // super LBRoom[7a] 6 [ handle event ]
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
// Fixes bug #10730
static const uint16 laurabow2FloppySignatureFixBugsWithMeat[] = {
	SIG_MAGICDWORD,
	0x57, 0x32, 0x06,                   // super Actor[32], 6 [ floppy: 32, cd: 31 ]
	0x3a,                               // toss
	0x48,                               // ret [ end of bugsWithMeat:<noname300> aka doVerb ]
	0x38, SIG_UINT16(0x008e),           // pushi 008e [ <noname142> aka script ]
	0x76,                               // push0
	0x81, 0x02,                         // lag 2 [ rm600 ]
	0x4a, 0x04,                         // send 4
	0x31, 0x0e,                         // bnt 0e [ run sDoMeat if not rm600:<noname142>? ]
	SIG_END
};

static const uint16 laurabow2FloppyPatchFixBugsWithMeat[] = {
	PATCH_ADDTOOFFSET(+5),
	0x38, PATCH_UINT16(0x00ed),         // pushi 00ed [ <noname237> aka canControl ]
	0x76,                               // push0
	0x81, 0x50,                         // lag 50 [ User ]
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
	0x81, 0x02,                         // lag 2 [ rm600 ]
	0x4a, 0x04,                         // send 4
	0x31, 0x0e,                         // bnt 0e [ run sDoMeat if not rm600:script? ]
	SIG_END
};

static const uint16 laurabow2CDPatchFixBugsWithMeat[] = {
	PATCH_ADDTOOFFSET(+5),
	0x38, PATCH_UINT16(0x00f6),         // pushi 00f6 [ canControl ]
	0x76,                               // push0
	0x81, 0x50,                         // lag 50 [ User ]
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
// Fixes bug #10808
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
	0x38, PATCH_UINT16(0x005a),         // pushi 005a [ handle ]
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
// Fixes bug #10761
static const uint16 laurabow2SignatureDisableSpeedTest[] = {
	0x89, 0x57,                         // lsg 87 [ speed test result ]
	SIG_MAGICDWORD,
	0x35, 0x03,                         // ldi 03 [ low-speed threshold ]
	0x24,                               // le?
	0x31, 0x04,                         // bnt 04
	0x35, 0x01,                         // ldi 01 [ lowest detail ]
	0x33, 0x0d,                         // jmp 0d
	0x89, 0x57,                         // lsg global87 [ speed test result ]
	SIG_END
};

static const uint16 laurabow2PatchDisableSpeedTest[] = {
	0x38, PATCH_UINT16(0x0005),         // pushi 0005
	0x81, 0x01,                         // lag 01
	0x4a, 0x06,                         // send 06 [ LB2:detailLevel = 5, max detail ]
	0x35, 0x0f,                         // ldi 0f
	0xa1, 0x57,                         // sag global87 [ global87 = f, max cpu speed ]
	0x33, 0x10,                         // jmp 10 [ continue init ]
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

// Directly use global 5a for view-cel id
//  That way it's possible to use a new "dual" mode view in the game menu
// View 995, loop 13, cel 0 -> "text"
// View 995, loop 13, cel 1 -> "speech"
// View 995, loop 13, cel 2 -> "dual"  (this view is injected by us into the game)
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
	0xa5, 0x00,                         // sat temp[0] - calculate global[5a] - 1 to use as view cel id
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
	{  true,    26, "Floppy: fix act 4 initialization",               1, laurabow2SignatureFixAct4Initialization,        laurabow2PatchFixAct4Initialization },
	{  true,   550, "Floppy: missing desk lamp message",              1, laurabow2SignatureMissingDeskLampMessage,       laurabow2PatchMissingDeskLampMessage },
	{  true,   440, "Floppy: handle armor inset events",              1, laurabow2SignatureHandleArmorInsetEvents,       laurabow2PatchHandleArmorInsetEvents },
	{  true,   600, "Floppy: fix bugs with meat",                     1, laurabow2FloppySignatureFixBugsWithMeat,        laurabow2FloppyPatchFixBugsWithMeat },
	{  true,   600, "CD: fix bugs with meat",                         1, laurabow2CDSignatureFixBugsWithMeat,            laurabow2CDPatchFixBugsWithMeat },
	{  true,   480, "CD: fix act 5 finale music",                     1, laurabow2CDSignatureFixAct5FinaleMusic,         laurabow2CDPatchFixAct5FinaleMusic },
	{  true,    28, "CD/Floppy: disable speed test",                  1, laurabow2SignatureDisableSpeedTest,             laurabow2PatchDisableSpeedTest },
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
	0xa1, SIG_ADDTOOFFSET(+1),       // sag global[57h] -> FM-Towns [9Dh]
	// 35 bytes
	0x39, 0x03,                      // pushi 03
	0x89, SIG_ADDTOOFFSET(+1),       // lsg global[1Dh] -> FM-Towns [1Eh]
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
	0x5b, 0x00, 0xbe,                // lea global[BE]
	0x36,                            // push
	0x39, 0x03,                      // pushi 03
	0x43, 0x62, 0x04,                // callk StrAt
	// 10 bytes
	0x36,                            // push
	0x35, SIG_MAGICDWORD, 0x20,      // ldi 20
	0x04,                            // sub
	0xa1, 0xb3,                      // sag global[b3]
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
	0x38, PATCH_UINT16(200),         // push 200
	0x38, PATCH_UINT16(320),         // push 320
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

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry mothergoose256Signatures[] = {
	{  true,     0, "replay save issue",                           1, mothergoose256SignatureReplay,    mothergoose256PatchReplay },
	{  true,     0, "save limit dialog (SCI1.1)",                  1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	{  true,   994, "save limit dialog (SCI1)",                    1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
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
	0x38, SIG_SELECTOR16(init),      // pushi $8e (init)
	SIG_MAGICDWORD,
	0x76,                            // push0
	0x72, SIG_UINT16(0x82),          // lofsa logo[82]
	0x4a, SIG_UINT16(0x04),          // send $4
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
	0x39, SIG_SELECTOR8(setPri), // pushi $4a (setPri)
	0x78,                        // push1
	0x38, SIG_UINT16(0xb7),      // pushi $b7
	SIG_END
};

static const uint16 mothergooseHiresHorsePatch[] = {
	PATCH_ADDTOOFFSET(3),     // pushi setPri, push1
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
static const uint16 phant1SavedVolumeSignature[] = {
	0x7a,                           // push2
	0x39, 0x08,                     // pushi 8
	0x38, SIG_SELECTOR16(readWord), // push $20b (readWord)
	0x76,                           // push0
	0x72, SIG_UINT16(0x13c),        // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),         // send 4
	SIG_MAGICDWORD,
	0xa1, 0xbc,                     // sag $bc
	0x36,                           // push
	0x43, 0x76, SIG_UINT16(0x04),   // callk DoAudio[76], 4
	0x7a,                           // push2
	0x76,                           // push0
	0x38, SIG_SELECTOR16(readWord), // push $20b (readWord)
	0x76,                           // push0
	0x72, SIG_UINT16(0x13c),        // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),         // send 4
	0xa1, 0xbb,                     // sag $bb
	0x36,                           // push
	0x43, 0x75, SIG_UINT16(0x04),   // callk DoSound[75], 4
	SIG_END
};

static const uint16 phant1SavedVolumePatch[] = {
	0x32, PATCH_UINT16(36),         // jmp [to prefFile::close]
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
static const uint16 phant1RatSignature[] = {
	SIG_MAGICDWORD,
	0x78,                         // push1
	0x39, 0x1a,                   // pushi $1a
	0x45, 0x03, SIG_UINT16(0x02), // callb 3, 2
	0x18,                         // not
	0x31, 0x18,                   // bnt $18
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
// cursor leaving the button (which would reset global 193 to 0), and the
// dialogue does not reset global193 itself, so it remains at 2 until a new
// button gets hovered and unhovered.
// There is not enough space in the confirmation dialogue code to add a reset
// of global 193, so we just remove the check entirely, since it is only used
// to avoid resetting the cursor's view on every mouse movement, and this
// button type is only used on the main menu and the in-game control panel.
//
// Applies to at least: English CD
static const uint16 phant1RedQuitCursorSignature[] = {
	SIG_MAGICDWORD,
	0x89, 0xc1,                   // lsg $c1
	0x35, 0x02,                   // ldi 02
	SIG_END
};

static const uint16 phant1RedQuitCursorPatch[] = {
	0x33, 0x05,                   // jmp [past global193 check]
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry phantasmagoriaSignatures[] = {
	{  true,    23, "make cursor red after clicking quit",         1, phant1RedQuitCursorSignature,    phant1RedQuitCursorPatch },
	{  true,   901, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,  1111, "ignore audio settings from save game",        1, phant1SavedVolumeSignature,      phant1SavedVolumePatch },
	{  true, 20200, "fix broken rat init in sEnterFromAlcove",     1, phant1RatSignature,              phant1RatPatch },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,          sci2BenchmarkPatch },
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
	0xa5, 0x00, // sat 0
	0x8d, 0x00, // lst 0
	0x87, 0x01, // lap 1
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
// directly and uses global 227 as the fade delta for each frame. Global 227
// normally contains the value 1, which means that these fades are quite slow.
// This patch replaces the use of global 227 with an immediate value that gives
// a reasonable fade speed.
// Applies to at least: US English
static const uint16 phant2SlowIFadeSignature[] = {
	0x43, 0x21, SIG_UINT16(0), // callk FrameOut, 0
	SIG_MAGICDWORD,
	0x67, 0x03,                // pTos 03 (scratch)
	0x81, 0xe3,                // lag $e3 (227)
	SIG_END
};

static const uint16 phant2SlowIFadePatch[] = {
	PATCH_ADDTOOFFSET(6),      // skip to lag
	0x35, 0x05,                // ldi 5
	PATCH_END
};

// The game uses a spin loop during music transitions which causes the mouse to
// appear unresponsive during scene changes. Replace the spin loop with a call
// to ScummVM kWait.
// Applies to at least: US English
// Responsible method: P2SongPlyr::wait4Fade
static const uint16 phant2Wait4FadeSignature[] = {
	SIG_MAGICDWORD,
	0x76,                      // push0
	0x43, 0x79, SIG_UINT16(0), // callk GetTime, 0
	0xa5, 0x01,                // sat 1
	0x78,                      // push1
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
	0x89, 0x1b,                   // lsg $1b
	0x8d, 0x05,                   // lst 5
	0x39, 0x09,                   // pushi 9
	0x43, 0x5d, SIG_UINT16(0x08), // callk FileIO, 8
	0x7a,                         // push2
	0x78,                         // push1
	0x8d, 0x05,                   // lst 5
	0x43, 0x5d, SIG_UINT16(0x04), // callk FileIO, 4
	0x35, 0x01,                   // ldi 1
	0xa1, 0xd8,                   // sag $d8
	SIG_END
};

static const uint16 phant2GetVersionPatch[] = {
	0x39, 0x04,                     // pushi 4
	0x39, 0x05,                     // pushi 5
	0x81, 0x1b,                     // lag $1b
	0x39, PATCH_SELECTOR8(data),    // pushi data
	0x76,                           // push0
	0x4a, PATCH_UINT16(4),          // send 4
	0x36,                           // push
	0x39, 0x09,                     // pushi 9
	0x8d, 0x05,                     // lst 5
	0x43, 0x5d, PATCH_UINT16(0x08), // callk FileIO, 8
	0x7a,                           // push2
	0x78,                           // push1
	0x8d, 0x05,                     // lst 5
	0x43, 0x5d, PATCH_UINT16(0x04), // callk FileIO, 4
	0x78,                           // push1
	0xa9, 0xd8,                     // ssg $d8
	PATCH_END
};

// The game uses a spin loop when displaying the success animation of the ratboy
// puzzle, which causes the mouse to appear unresponsive. Replace the spin loop
// with a call to ScummVM kWait.
// Applies to at least: US English
static const uint16 phant2RatboySignature[] = {
	0x8d, 0x01,                   // lst 1
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
// in-game volume (misleading labelled "music volume"), but really needs the
// in-game volume to always be significantly lower than the movie volume in
// order for dialogue in movies to be consistently audible, so patch the in-game
// volume slider to limit it to our maximum.
// Applies to at least: US English
static const uint16 phant2AudioVolumeSignature[] = {
	SIG_MAGICDWORD,
	0x39, 0x7f,           // pushi 127 (clientMax value)
	0x39, 0x14,           // pushi 20  (clientPageSize value)
	SIG_ADDTOOFFSET(+10), // skip other init arguments
	0x51, 0x5e,           // class P2ScrollBar
	SIG_ADDTOOFFSET(+3),  // skip send
	0xa3, 0x06,           // sal 6 (identifies correct slider)
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
	0xa5, 0x00,                  // sat 0
	0x39, SIG_SELECTOR8(format), // pushi format
	SIG_END
};

static const uint16 phant2SaveNamePatch2[] = {
	PATCH_ADDTOOFFSET(+2), // sat 0
	0x33, 0x68,            // jmp [past name mangling]
	PATCH_END
};

// Phant2-specific version of sci2NumSavesSignature1/2
// Applies to at least: English CD
static const uint16 phant2NumSavesSignature1[] = {
	SIG_MAGICDWORD,
	0x8d, 0x01, // lst 1
	0x35, 0x14, // ldi 20
	0x1a,       // eq?
	SIG_END
};

static const uint16 phant2NumSavesPatch1[] = {
	PATCH_ADDTOOFFSET(+2), // lst 1
	0x35, 0x63,            // ldi 99
	PATCH_END
};

static const uint16 phant2NumSavesSignature2[] = {
	SIG_MAGICDWORD,
	0x8d, 0x00, // lst 0
	0x35, 0x14, // ldi 20
	0x22,       // lt?
	SIG_END
};

static const uint16 phant2NumSavesPatch2[] = {
	PATCH_ADDTOOFFSET(+2), // lst 0
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
	0x35, 0x0a,                // ldi 10
	0x22,                      // lt?
	0x31, 0x17,                // bnt [end of loop]
	0x76,                      // push0
	0x43, 0x79, SIG_UINT16(0), // callk GetTime, 0
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
static const uint16 phant2BadPositionSignature[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(setSize), // pushi setSize
	0x76,                         // push0
	0x39, SIG_SELECTOR8(init),    // pushi init
	0x78,                         // pushi 1
	0x89, 0x03,                   // lsg 3
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
	0x89, 0x03,                         // lsg 3
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
	0x4a, SIG_UINT16(0x04),       // send 4
	0x4a, SIG_UINT16(0x06),       // send 6
	SIG_END
};

static const uint16 phant2BadIconPatch[] = {
	PATCH_ADDTOOFFSET(+5),          // pushi setCel, push1, push1
	0x38, PATCH_SELECTOR16(update), // pushi update
	0x76,                           // push0
	0x4a, PATCH_UINT16(0x0a),       // send 10
	0x33, 0x04,                     // jmp [past unused bytes]
	PATCH_END
};

// The left and right arrows move inventory items a pixel more than each
// inventory item is wide, which causes the inventory to creep to the left by
// one pixel per scrolled item.
// Applies to at least: US English
static const uint16 phant2InvLeftDeltaSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x42), // delta
	SIG_UINT16(0x19), // moveDelay
	SIG_END
};

static const uint16 phant2InvLeftDeltaPatch[] = {
	PATCH_UINT16(0x41), // delta
	PATCH_END
};

static const uint16 phant2InvRightDeltaSignature[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0xffbe), // delta
	SIG_UINT16(0x19),   // moveDelay
	SIG_END
};

static const uint16 phant2InvRightDeltaPatch[] = {
	PATCH_UINT16(0xffbf), // delta
	PATCH_END
};

// The first inventory item is put too far to the right, which causes wide items
// to get cut off on the right side of the inventory.
// Applies to at least: US English
static const uint16 phant2InvOffsetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x26,                 // ldi 38
	0x64, SIG_SELECTOR16(xOff), // aTop xOff
	SIG_END
};

static const uint16 phant2InvOffsetPatch[] = {
	0x35, 0x1d, // ldi 29
	PATCH_END
};

// The text placement of "File" and "Note" content inside DocuStore File
// Retrieval System makes some letters especially "g" overlap the
// corresponding box. Set by 'WynNetDoco::open'.
// We fix this by changing the position of those 2 inside the heap of
// subclass 'WynNetDoco' slightly.
// Applies to at least: English CD, Japanese CD, German CD
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
	PATCH_UINT16(0x0006),  // new nameY
	PATCH_ADDTOOFFSET(+12),
	PATCH_UINT16(0x001b),  // new noteY
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
	PATCH_UINT16(0x0050), // new nameX
	PATCH_UINT16(0x0006), // new nameY
	SIG_ADDTOOFFSET(+10),
	PATCH_UINT16(0x0050), // new noteX
	PATCH_UINT16(0x001b), // new noteY
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
	{  true, 64990, "remove save game name mangling (1/2)",        1, phant2SaveNameSignature1,                  phant2SaveNamePatch1 },
	{  true, 64990, "increase number of save games (1/2)",         1, phant2NumSavesSignature1,                  phant2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         2, phant2NumSavesSignature2,                  phant2NumSavesPatch2 },
	{  true, 64994, "remove save game name mangling (2/2)",        1, phant2SaveNameSignature2,                  phant2SaveNamePatch2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

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
	0x45, 0x02, 0x00,                    // call export 2 of script 0 (disable control)
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
	0x33, 0x0a,                      // jmp to lsl local[2], skip over export 2 and ::notify
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
// Fixes bug: #5705 / #6400
static const uint16 pq1vgaSignaturePutGunInLockerBug[] = {
	0x35, 0x00,                      // ldi 00
	0x1a,                            // eq?
	0x31, 0x25,                      // bnt [next state check]
	SIG_ADDTOOFFSET(+22),            // [skip 22 bytes]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(put),       // pushi "put"
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag 00
	0x4a, 0x06,                      // send 06 - ego::put(0)
	0x35, 0x02,                      // ldi 02
	0x65, 0x1c,                      // aTop 1c (set timer to 2 seconds)
	0x33, 0x0e,                      // jmp [end of method]
	0x3c,                            // dup --- next state check target
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x08,                      // bnt [end of method]
	0x39, SIG_SELECTOR8(dispose),    // pushi "dispose"
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
	0x3c,                            // dup --- next state check target
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x11,                      // bnt [end of method]
	0x38, PATCH_SELECTOR16(put),     // pushi "put"
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag 00
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
	0x43, 0x08, 0x08,                    // kDrawPic (8)
	SIG_END
};

static const uint16 pq1vgaPatchMapSaveRestoreBug[] = {
	0x38, PATCH_SELECTOR16(overlay), // pushi "overlay"
	0x7a,                            // push2
	0x89, 0xf9,                      // lsg global[f9]
	0x39, 0x64,                      // pushi 64 (no transition)
	0x81, 0x02,                      // lag global[02] (current room object)
	0x4a, 0x08,                      // send 08
	0x18,                            // not (waste byte)
	PATCH_END
};

//          script, description,                                         signature                            patch
static const SciScriptPatcherEntry pq1vgaSignatures[] = {
	{  true,   152, "getting stuck while briefing is about to start", 1, pq1vgaSignatureBriefingGettingStuck, pq1vgaPatchBriefingGettingStuck },
	{  true,   341, "put gun in locker bug",                          1, pq1vgaSignaturePutGunInLockerBug,    pq1vgaPatchPutGunInLockerBug },
	{  true,   500, "map save/restore bug",                           2, pq1vgaSignatureMapSaveRestoreBug,    pq1vgaPatchMapSaveRestoreBug },
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
	0x45, 0x06, 0x04,                    // callb export000_6, 4
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

//          script, description,                                 signature                     patch
static const SciScriptPatcherEntry pq3Signatures[] = {
	{  true, 36, "give locket missing points",                1, pq3SignatureGiveLocketPoints, pq3PatchGiveLocketPoints },
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
	0x38, SIG_SELECTOR16(init),   // pushi $93 (init)
	0x76,                         // push0
	0x59, 0x01,                   // &rest 01
	0x57, 0x8f, SIG_UINT16(0x04), // super GCItem
	0x48,                         // ret

	// iconText::select
	0x38, SIG_SELECTOR16(select), // pushi $1c4 (select)
	0x76,                         // push0
	0x59, 0x01,                   // &rest 01
	0x57, 0x8f, SIG_UINT16(0x04), // super GCItem, 4
	0x89, 0x5a,                   // lsg global[$5a]
	0x35, 0x02,                   // ldi 2
	0x12,                         // and
	0x30, SIG_UINT16(0x1f),       // bnt [jump to currently-in-text-mode code]
	SIG_ADDTOOFFSET(+67),         // skip over the rest
	0x48,                         // ret
	SIG_END
};

static const uint16 pq4CdSpeechAndSubtitlesPatch[] = {
	// iconText::init
	0x76,                           // push0
	0x41, 0x02, PATCH_UINT16(0x00), // call [our new subroutine which sets view+loop+cel], 0
	0x33, 0x40,                     // jmp [to original init, super GCItem call]
	// new code for setting view+loop+cel
	0x34, PATCH_UINT16(0x2aeb),     // ldi 10987
	0x65, 0x78,                     // aTop mainView - always set this view, because it's used by 2 states
	0x89, 0x5a,                     // lsg global[$5a]
	0x35, 0x03,                     // ldi 3
	0x1a,                           // eq?
	0x31, 0x04,                     // bnt [skip over follow up code]
	// speech+subtitles mode
	0x78,                           // push1
	0x69, 0x7a,                     // sTop mainLoop
	0x48,                           // ret
	0x89, 0x5a,                     // lsg global[$5a]
	0x35, 0x01,                     // ldi 1
	0x12,                           // and
	0x31, 0x04,                     // bnt [skip over follow up code]
	// subtitles mode
	0x76,                           // push0
	0x69, 0x7a,                     // sTop mainLoop
	0x48,                           // ret
	// speech mode
	0x34, PATCH_UINT16(0x2ae6),     // ldi 10982
	0x65, 0x78,                     // aTop mainView
	0x35, 0x0f,                     // ldi 15
	0x65, 0x7a,                     // aTop mainLoop
	0x48,                           // ret
	PATCH_ADDTOOFFSET(+38),         // skip to iconText::select

	// iconText::select
	PATCH_ADDTOOFFSET(+10),         // skip over the super code
	0xc1, 0x5a,                     // +ag $5a
	0xa1, 0x5a,                     // sag $5a
	0x36,                           // push
	0x35, 0x04,                     // ldi 4
	0x28,                           // uge?
	0x31, 0x03,                     // bnt [skip over follow up code]
	0x78,                           // push1
	0xa9, 0x5a,                     // ssg $5a
	0x76,                           // push0
	0x41, 0x99, PATCH_UINT16(0x00), // call [our new subroutine which sets view+loop+cel, effectively -103], 0
	0x33, 0x2f,                     // jmp [to end of original select, show call]
	PATCH_END
};

// When showing the red shoe to Barbie after showing the police badge but before
// exhausting the normal dialogue tree, the game plays the expected dialogue but
// fails to award points or set an internal flag indicating this interaction has
// occurred (which is needed to progress in the game). This is because the game
// checks global $9a (dialogue progress flag) instead of local 3 (badge shown
// flag) when interacting with Barbie. The game uses the same
// `shoeShoe::changeState(0)` method for showing the shoe to the young woman at the
// bar earlier in the game, and checks local 3 then, so just check local 3 in
// both cases to prevent the game from appearing to be in an unwinnable state
// just because the player interacted in the "wrong" order.
// Applies to at least: English floppy, German floppy, English CD, German CD
static const uint16 pq4BittyKittyShowBarieRedShoeSignature[] = {
	// stripper::noun check is for checking, if police badge was shown
	SIG_MAGICDWORD,
	0x89, 0x9a,                         // lsg global[$9a]
	0x35, 0x02,                         // ldi 2
	0x1e,                               // gt?
	0x30, SIG_UINT16(0x0028),           // bnt [skip 2 points code]
	0x39, SIG_SELECTOR8(points),       // pushi $61 (points)
	SIG_END
};

static const uint16 pq4BittyKittyShowBarbieRedShoePatch[] = {
	0x83, 0x03,                         // lal local[3]
	0x30, PATCH_UINT16(0x002b),         // bnt [skip 2 points code]
	0x33, 1,                            // jmp 1 (waste some bytes)
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
// Applies to at least: English Floppy, German floppy
// Responsible method: metzAttack::changeState(2) - 120 ticks (player needs to draw gun)
//                     stickScr::changeState(0) - 180 ticks (player needs to tell enemy to drop gun)
//                     dropStick::changeState(5) - 120 ticks (player needs to tell enemy to turn around)
//                     turnMetz::changeState(5) - 600/420 ticks (player needs to cuff Metz)
//                     all in script 390
//
// TODO: The object structure changed in PQ4CD so ticks moved from 0x20 to 0x22.
// Additional signatures/patches will need to be added for CD version.
static const uint16 pq4FloppyCityHallDrawGunTimerSignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x08), // send 8
	0x32,                   // jmp [ret]
	SIG_ADDTOOFFSET(+8),    // skip over some code
	0x35, 0x78,             // pushi $78 (120)
	0x65, 0x20,             // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallDrawGunTimerPatch[] = {
	PATCH_ADDTOOFFSET(+12), // send 8, jmp, skip over some code
	0x35, 0x05,             // pushi 4 (120t/2s -> 4s)
	0x65, 0x1c,             // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallTellEnemyDropWeaponTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0xb4), // pushi $b4 (180)
	0x65, 0x20,             // aTop ticks
	0x32, SIG_UINT16(0x5e), // jmp to ret
	SIG_END
};

static const uint16 pq4FloppyCityHallTellEnemyDropWeaponTimerPatch[] = {
	0x34, PATCH_UINT16(0x05), // pushi 5 (180t/3s -> 5s)
	0x65, 0x1c,               // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallTellEnemyTurnAroundTimerSignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x04), // send 4
	0x35, 0x78,             // pushi $78 (120)
	0x65, 0x20,             // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallTellEnemyTurnAroundTimerPatch[] = {
	PATCH_ADDTOOFFSET(+3), // send 4
	0x35, 0x03,            // pushi 3 (120t/2s -> 3s)
	0x65, 0x1c,            // aTop seconds
	PATCH_END
};

static const uint16 pq4FloppyCityHallCuffEnemyTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x258), // pushi $258 (600)
	0x65, 0x20,              // aTop ticks
	SIG_ADDTOOFFSET(+3),
	0x34, SIG_UINT16(0x1a4), // pushi $1a4 (420)
	0x65, 0x20,              // aTop ticks
	SIG_END
};

static const uint16 pq4FloppyCityHallCuffEnemyTimerPatch[] = {
	0x34, PATCH_UINT16(0x0a), // pushi 10 (600t/10s)
	0x65, 0x1c,               // aTop seconds
	PATCH_ADDTOOFFSET(+3),
	0x34, SIG_UINT16(0x07),   // pushi 7 (420t/7s)
	0x65, 0x1c,               // aTop seconds
	PATCH_END
};

// The end game action sequence also uses ticks instead of seconds. See the
// description of city hall action sequence issues for more information.
// Applies to at least: English Floppy, German floppy, English CD
// Responsible method: comeInLast::changeState(11)
static const uint16 pq4LastActionHeroTimerSignature[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x12c),   // pushi $12c (300)
	0x65, SIG_ADDTOOFFSET(+1), // aTop ticks ($20 for floppy, $22 for CD)
	SIG_END
};

static const uint16 pq4LastActionHeroTimerPatch[] = {
	0x34, PATCH_UINT16(0x05),                 // pushi 5 (300t/5s)
	0x65, PATCH_GETORIGINALBYTEADJUST(4, -4), // aTop seconds
	PATCH_END
};

//          script, description,                                          signature                                           patch
static const SciScriptPatcherEntry pq4Signatures[] = {
	{  true,     9, "add speech+subtitles to in-game UI",              1, pq4CdSpeechAndSubtitlesSignature,                   pq4CdSpeechAndSubtitlesPatch },
	{  true,   315, "fix missing points showing barbie the red shoe",  1, pq4BittyKittyShowBarieRedShoeSignature,             pq4BittyKittyShowBarbieRedShoePatch },
	{  true,   390, "change floppy city hall use gun timer",           1, pq4FloppyCityHallDrawGunTimerSignature,             pq4FloppyCityHallDrawGunTimerPatch },
	{  true,   390, "change floppy city hall say 'drop weapon' timer", 1, pq4FloppyCityHallTellEnemyDropWeaponTimerSignature, pq4FloppyCityHallTellEnemyDropWeaponTimerPatch },
	{  true,   390, "change floppy city hall say 'turn around' timer", 1, pq4FloppyCityHallTellEnemyTurnAroundTimerSignature, pq4FloppyCityHallTellEnemyTurnAroundTimerPatch },
	{  true,   390, "change floppy city hall use handcuffs timer",     1, pq4FloppyCityHallCuffEnemyTimerSignature,           pq4FloppyCityHallCuffEnemyTimerPatch },
	{  true,   755, "change last action sequence timer",               1, pq4LastActionHeroTimerSignature,                    pq4LastActionHeroTimerPatch },
	{  true, 64908, "disable video benchmarking",                      1, sci2BenchmarkSignature,                             sci2BenchmarkPatch },
	{  true, 64918, "fix Str::strip in floppy version",                1, sci2BrokenStrStripSignature,                        sci2BrokenStrStripPatch },
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
static const uint16 pqSwatVolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(masterVolume), // pushi masterVolume
	0x78,                               // push1
	0x39, 0x7f,                         // push $7f
	0x54, SIG_UINT16(0x06),             // self 6
	SIG_END
};

static const uint16 pqSwatVolumeResetPatch[] = {
	0x32, PATCH_UINT16(6), // jmp 6 [past volume reset]
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
//   will hit. It's the same issue as in freddy pharkas and if you turn dos-box
//   to max cycles, sometimes clicks also won't get registered. Strangely it's
//   not nearly as bad as in our sci, but these differences may be caused by
//   timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
// Applies to at least: English floppy
// Responsible method: pointBox::doit
static const uint16 qfg1vgaSignatureFightEvents[] = {
	0x39, SIG_MAGICDWORD,
	SIG_SELECTOR8(new),                 // pushi "new"
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
	0x38, PATCH_SELECTOR16(curEvent), // pushi 15a (selector curEvent)
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
	0x87, 0x00,                         // lap 0
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
// Fixes bug #6249
static const uint16 qfg1vgaSignatureCheetaurDescription[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x01b8),           // ldi 01b8
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt 16
	0x38, SIG_SELECTOR16(say),          // pushi 0127h (selector "say")
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
// Problem is that the Sierra programmers forgot to disable the door, that
//  gets opened by pulling the chain. So when ego falls down and then
//  rolls through the door, one method thinks that the player walks through
//  it and acts that way and the other method is still doing the roll animation.
// Local 5 of that room is a timer, that closes the door (object door11).
// Setting it to 1 during happyFace::changeState(0) stops door11::doit from
//  calling goTo6::init, so the whole issue is stopped from happening.
//
// Applies to at least: English floppy
// Responsible method: happyFace::changeState, door11::doit
// Fixes bug #6181
static const uint16 qfg1vgaSignatureFunnyRoomFix[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0025),           // bnt 0025 [-> next state]
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal 4e
	SIG_END
};

static const uint16 qfg1vgaPatchFunnyRoomFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x2e, PATCH_UINT16(0x0029),         // bt 0029 [-> next state] - saves 4 bytes
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal 4e
	0xa3, 0x05,                         // sal 05 (sets local 5 to 1)
	0xa3, 0x05,                         // and again to make absolutely sure (actually to waste 2 bytes)
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
// Fixes bug #6706
static const uint16 qfg1vgaSignatureHealerHutNoDelay[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt 07 [-> next state]
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c (60 ticks)
	0x65, 0x20,                         // aTop ticks
	0x32,                               // jmp [-> end of method]
	SIG_END
};

static const uint16 qfg1vgaPatchHealerHutNoDelay[] = {
	PATCH_ADDTOOFFSET(+9),
	0x35, 0x01,                         // ldi 01 (1 tick only, so that execution will resume as soon as dialog box is closed)
	PATCH_END
};

// When following the white stag, you can actually enter the 2nd room from the mushroom/fairy location,
//  which results in ego entering from the top. When you then throw a dagger at the stag, one animation
//  frame will stay on screen, because of a script bug.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: stagHurt::changeState
// Fixes bug #6135
static const uint16 qfg1vgaSignatureWhiteStagDagger[] = {
	0x87, 0x01,                         // lap param[1]
	0x65, 0x14,                         // aTop state
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt [next parameter check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb export 2 from script 0, 0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(say),          // pushi 0127h (selector "say")
	0x39, 0x05,                         // pushi 05
	0x39, 0x03,                         // pushi 03
	0x39, 0x51,                         // pushi 51h
	0x76,                               // push0
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[5Bh] -> qg1Messager
	0x4a, 0x0e,                         // send 0Eh -> qg1Messager::say(3, 51h, 0, 0, stagHurt)
	0x33, 0x12,                         // jmp -> [ret]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt [ret]
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
	0x45, 0x02, 0x00,                   // callb export 2 from script 0, 0
	0x38, PATCH_SELECTOR16(say),        // pushi 0127h (selector "say")
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

// The dagger range has a script bug that can freeze the game or cause Brutus to kill you even after you've killed him.
// This is a bug in the original game.
//
// When Bruno leaves, a 300 tick countdown starts. If you kill Brutus or leave room 73 within those 300 ticks then
// the game is left in a broken state. For the rest of the game, if you ever return to the dagger range from the
// east or west during the first half of the day then the game will freeze or Brutus will come back to life
// and kill you, even if you already killed him.
//
// Special thanks, credits and kudos to sluicebox, who did a ton of research on this and even found this game bug originally.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: brutusWaits::changeState
// Fixes bug #9558
static const uint16 qfg1vgaSignatureBrutusScriptFreeze[] = {
	0x78,                               // push1
	0x38, SIG_UINT16(0x144),            // pushi 144h (324d)
	0x45, 0x05, 0x02,                   // call export 5 of script 0
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x12c),            // ldi 12Ch (300d)
	0x65, 0x20,                         // aTop ticks
	SIG_END
};

static const uint16 qfg1vgaPatchBrutusScriptFreeze[] = {
	0x34, PATCH_UINT16(0),              // ldi 0 (waste 7 bytes)
	0x35, 0x00,                         // ldi 0
	0x35, 0x00,                         // ldi 0
	PATCH_END
};

// Speed up the speed test by a factor 50, ensuring the detected speed
// will end up at the highest level. This improves the detail in
// Yorick's room (96), and slightly changes the timing in other rooms.
//
// Method changed: speedTest::changeState
static const uint16 qfg1vgaSignatureSpeedTest[] = {
	0x76,                               // push0
	0x43, 0x42, 0x00,                   // callk GetTime 0
	SIG_MAGICDWORD,
	0xa3, 0x01,                         // sal 1
	0x35, 0x32,                         // ldi 50
	0x65, 0x1a,                         // aTop cycles
	SIG_END
};

static const uint16 qfg1vgaPatchSpeedTest[] = {
	PATCH_ADDTOOFFSET(+6),
	0x35, 0x01,                         // ldi 1
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
//  remain on screen and can't block ego's exit. This is consistent with room 85
//  where they also appear but without a southern exit. The Wander motion is
//  only used by antwerps and the sparkles above Yorick in room 96 so it can be
//  safely patched to enforce a southern limit. We make room for this patch by
//  replacing Wander's calculations with their known results, since the default
//  Wander:distance of 30 is always used, and by overwriting Wander:onTarget
//  which no script calls and just returns zero.
//
// Applies to: PC Floppy, Mac Floppy
// Responsible method: Wander:setTarget
// Fixes bug #9564
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
	0xa5, 0x00,                             // sat 00 [ temp0 = distance * 2 ]
	0x36,                                   // push
	0x43, 0x3c, 0x04,                       // callk Random 4
	0x04,                                   // sub
	0x02,                                   // add
	0x65, 0x16,                             // aTop x [ x = client:x + (distance - Random(0, temp0)) ]
	0x76,                                   // push0
	0x76,                                   // push0
	0x63, 0x12,                             // pToa client
	0x4a, 0x04,                             // send 4
	0x36,                                   // push
	0x67, 0x30,                             // pTos distance
	0x7a,                                   // push2
	0x76,                                   // push0
	0x8d, 0x00,                             // lst 00
	0x43, 0x3c, 0x04,                       // callk Random 4
	0x04,                                   // sub
	0x02,                                   // add
	0x65, 0x18,                             // aTop y [ y = client:y + (distance - Random(0, temp0)) ]
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
	0x43, 0x3c, 0x04,                       // callk Random 4
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
	0x43, 0x3c, 0x04,                       // callk Random 4
	0x04,                                   // sub
	0x02,                                   // add
	0x7a,                                   // push2
	0x36,                                   // push
	0x38, PATCH_UINT16(0x00b4),             // pushi 00b4
	0x46, PATCH_UINT16(0x03e7),             // calle proc999_2 4 [ Min ]
	      PATCH_UINT16(0x0002), 0x04,
	0x65, 0x18,                             // aTop y [ y = Min(client:y + (30d - Random(0, 60d))), 180d) ]
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry qfg1vgaSignatures[] = {
	{  true,    41, "moving to castle gate",                       1, qfg1vgaSignatureMoveToCastleGate,    qfg1vgaPatchMoveToCastleGate },
	{  true,    55, "healer's hut, no delay for buy/steal",        1, qfg1vgaSignatureHealerHutNoDelay,    qfg1vgaPatchHealerHutNoDelay },
	{  true,    73, "brutus script freeze glitch",                 1, qfg1vgaSignatureBrutusScriptFreeze,  qfg1vgaPatchBrutusScriptFreeze },
	{  true,    77, "white stag dagger throw animation glitch",    1, qfg1vgaSignatureWhiteStagDagger,     qfg1vgaPatchWhiteStagDagger },
	{  true,    96, "funny room script bug fixed",                 1, qfg1vgaSignatureFunnyRoomFix,        qfg1vgaPatchFunnyRoomFix },
	{  true,   210, "cheetaur description fixed",                  1, qfg1vgaSignatureCheetaurDescription, qfg1vgaPatchCheetaurDescription },
	{  true,   215, "fight event issue",                           1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   216, "weapon master event issue",                   1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   299, "speedtest",                                   1, qfg1vgaSignatureSpeedTest,           qfg1vgaPatchSpeedTest },
	{  true,   331, "moving to crusher",                           1, qfg1vgaSignatureMoveToCrusher,       qfg1vgaPatchMoveToCrusher },
	{  true,   814, "window text temp space",                      1, qfg1vgaSignatureTempSpace,           qfg1vgaPatchTempSpace },
	{  true,   814, "dialog header offset",                        3, qfg1vgaSignatureDialogHeader,        qfg1vgaPatchDialogHeader },
	{  true,   970, "antwerps wandering off-screen",               1, qfg1vgaSignatureAntwerpWander,       qfg1vgaPatchAntwerpWander },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// This is a very complicated bug.
// When the player encounters an enemy in the desert while riding a saurus and later
//  tries to get back on it by entering "ride", the game will not give control back
//  to the player.
//
// This is caused by script mountSaurus getting triggered twice.
//  Once by entering the command "ride" and then a second time by a proximity check.
//
// Both are calling mountSaurus::init() in script 20, this one disables controls
//  then mountSaurus::changeState() from script 660 is triggered
//  mountSaurus::changeState(5) finally calls mountSaurus::dispose(), which is also in script 20
//  which finally re-enables controls
//
// A fix is difficult to implement. The code in script 20 is generic and used by multiple objects
//
// Originally I decided to change the responsible globals (66h and A1h) during mountSaurus::changeState(5).
//  This worked as far as for controls, but mountSaurus::init changes a few selectors of ego as well, which
//  won't get restored in that situation, which then messes up room changes and other things.
//
// I have now decided to change sheepScript::changeState(2) in script 665 instead.
//
// This fix could cause issues in case there is a cutscene, where ego is supposed to get onto the saurus using
//  sheepScript.
//
// Applies to at least: English PC Floppy, English Amiga Floppy
// Responsible method: mountSaurus::changeState(), mountSaurus::init(), mountSaurus::dispose()
// Fixes bug: #5156
static const uint16 qfg2SignatureSaurusFreeze[] = {
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 5
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0043),           // bnt [ret]
	0x76,                               // push0
	SIG_ADDTOOFFSET(+61),               // skip to dispose code
	0x39, SIG_SELECTOR8(dispose),       // pushi "dispose"
	0x76,                               // push0
	0x54, 0x04,                         // self 04
	SIG_END
};

static const uint16 qfg2PatchSaurusFreeze[] = {
	0x81, 0x66,                         // lag 66h
	0x2e, SIG_UINT16(0x0040),           // bt [to dispose code]
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	PATCH_END
};

// The Jackalmen combat code has at least one serious issue.
//
// Jackalmen may attack in groups.
// This is handled by 2 globals.
// Global 136h contains the amount of cur. Jackalmen alive.
// Global 137h contains the amount of cur. Jackalmen killed during combat.
//
// Global 137h is subtracted from Global 136h after combat
// has ended, BUT when the player manages to hit the last enemy
// AFTER defeating it during its death animation (yes, that is possible - don't ask),
// the code is called a second time. Subtracting 137h twice, which will make global 136h
// negative and which will then make it so that there is an inconsistent state.
// Some variables will show that there is still an enemy, while others don't.
//
// Which will then make the game crash when leaving the current room.
// The original interpreter would show the infamous "Oops, you did something we weren't expecting...".
// 
// Applies to at least: English Floppy (1.102+1.105)
// TODO: Check, if patch works for 1.000. That version surely has the same bug.
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
	0x31, 0x0E,                         // bnt [skip everything] - requires 5 extra bytes
	0x8b, 0x00,                         // lsl local[0]
	0x35, 0x00,                         // ldi 0
	0x22,                               // lt?
	0x31, 0x08,                         // bnt [Jackalman death animation code] - save 1 byte
	0x38, PATCH_GETORIGINALUINT16(+9),  // pushi (die)
	0x76,                               // push0
	0x57, 0x66, 0x04,                   // super Monster, 4
	0x48,                               // ret
	// Jackalman death animation code
	0x83, 0x00,                         // lal local[0]
	0x18,                               // not
	0x31, 0x03,                         // bnt [make next enemy walk in] - save 1 byte
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
	0x43, 0x72, 0x04,                   // callk Memory 4
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
	0xa8, SIG_UINT16(0x0248),           // ssg global[0248h] <-- patched to save 2 bytes
	0x8b, 0x1f,                         // lsl local[1Fh]
	0xa8, SIG_UINT16(0x0155),           // ssg global[0155h] <-- patched to save 2 bytes
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
	0x43, 0x72, 0x04,                   // callk Memory 4
	0x35, 0x00,                         // ldi 00
	0x65, 0x2a,                         // aTop text
	SIG_END
};

static const uint16 qfg3PatchImportDialog[] = {
	PATCH_ADDTOOFFSET(+4),
	0x48,                               // ret
	PATCH_END
};



// ===========================================================================
// Patch for the Woo dialog option in Uhura's conversation.
// Problem: The Woo dialog option (0xffb5) is negative, and therefore
// treated as an option opening a submenu. This leads to uhuraTell::doChild
// being called, which calls hero::solvePuzzle and then proceeds with
// Teller::doChild to open the submenu. However, there is no actual submenu
// defined for option -75 since -75 does not show up in uhuraTell::keys.
// This will cause Teller::doChild to run out of bounds while scanning through
// uhuraTell::keys.
// Strategy: there is another conversation option in uhuraTell::doChild calling
// hero::solvePuzzle (0xfffc) which does a ret afterwards without going to
// Teller::doChild. We jump to this call of hero::solvePuzzle to get that same
// behaviour.
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
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi 0297
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag 00
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialog[] = {
	PATCH_ADDTOOFFSET(+0x29),
	0x33, 0x11,                         // jmp to 0x6a2, the call to hero::solvePuzzle for 0xFFFC
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
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi 0297
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag 00
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialogAlt[] = {
	PATCH_ADDTOOFFSET(+0x2C),
	0x33, 0x12,                         // jmp to 0x708, the call to hero::solvePuzzle for 0xFFFC
	PATCH_END
};

// When exporting characters at the end of Quest for Glory 3, the underlying
//  code has issues with values, that are above 9999.
//  For further study: https://github.com/Blazingstix/QFGImporter/blob/master/QFGImporter/QFGImporter/QFG3.txt
//
// If a value is above 9999, parts or even the whole character file will get corrupted.
//
// We are fixing the code because of that. We are patching code, that is calculating the checksum
//  and add extra code to lower such values to 9999.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: saveHero::changeState
// Fixes bug #6807
static const uint16 qfg3SignatureExportChar[] = {
	0x35, SIG_ADDTOOFFSET(+1),          // ldi  00 / ldi 01 (2 loops, we patch both)
	0xa5, 0x00,                         // sat  temp[0] [contains index to data]
	0x8d, 0x00,                         // lst  temp[0]
	SIG_MAGICDWORD,
	0x35, 0x2c,                         // ldi  2c
	0x22,                               // lt?  [index above or equal 2Ch (44d)?
	0x31, 0x23,                         // bnt  [exit loop]
	// from this point it's actually useless code, maybe a sci compiler bug
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0x9b, 0x00,                         // lsli local[0] ---------- load local[0 + ACC] onto stack
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0xb3, 0x00,                         // sali local[0] ---------- save stack to local[0 + ACC]
	// end of useless code
	0x8b, SIG_ADDTOOFFSET(+1),          // lsl  local[36h/37h] ---- load local[36h/37h] onto stack
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0x93, 0x00,                         // lali local[0] ---------- load local[0 + ACC] into ACC
	0x02,                               // add -------------------- add ACC + stack and put into ACC
	0xa3, SIG_ADDTOOFFSET(+1),          // sal  local[36h/37h] ---- save ACC to local[36h/37h]
	0x8d, 0x00,                         // lst temp[0] ------------ temp[0] to stack
	0x35, 0x02,                         // ldi 02
	0x02,                               // add -------------------- add 2 to stack
	0xa5, 0x00,                         // sat temp[0] ------------ save ACC to temp[0]
	0x33, 0xd6,                         // jmp [loop]
	SIG_END
};

static const uint16 qfg3PatchExportChar[] = {
	PATCH_ADDTOOFFSET(+11),
	0x85, 0x00,                         // lat  temp[0]
	0x9b, 0x01,                         // lsli local[0] + 1 ------ load local[ ACC + 1] onto stack
	0x3c,                               // dup
	0x34, PATCH_UINT16(0x2710),         // ldi  2710h (10000d)
	0x2c,                               // ult? ------------------- is value smaller than 10000?
	0x2f, 0x0a,                         // bt   [jump over]
	0x3a,                               // toss
	0x38, PATCH_UINT16(0x270f),         // pushi 270fh (9999d)
	0x3c,                               // dup
	0x85, 0x00,                         // lat  temp[0]
	0xba, PATCH_UINT16(0x0001),         // ssli local[0] + 1 ------ save stack to local[ ACC + 1] (UINT16 to waste 1 byte)
	// jump offset
	0x83, PATCH_GETORIGINALBYTE(+26),   // lal  local[37h/36h] ---- load local[37h/36h] into ACC
	0x02,                               // add -------------------- add local[37h/36h] + data value
	PATCH_END
};

// Quest for Glory 3 doesn't properly import the character type of Quest for Glory 1 character files.
//  This issue was never addressed. It's caused by Sierra reading data directly from the local
//  area, which is only set by Quest For Glory 2 import data, instead of reading the properly set global variable.
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
	0xa3, 0x01,                         // sal 01           -> also set local[01]
	0x89, 0xfc,                         // lsg global[0xFD] -> save 2 bytes
	PATCH_END
};

// The chief in his hut (room 640) is not drawn using the correct priority,
//  which results in a graphical glitch. This is a game bug and also happens
//  in Sierra's SCI. We adjust priority accordingly to fix it.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: heap in script 640
// Fixes bug #5173
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
	PATCH_UINT16(0x000A),               // new priority 0x000A (10d)
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x1010),               // signal       0x1010 (set fixed priority flag)
	PATCH_END
};

// There are 3 points that can't be achieved in the game. They should've been
// awarded for telling Rakeesh and Kreesha (room 285) about the Simabni
// initiation.
// However the array of posibble messages the hero can tell in that room
// (local 156) is missing the "Tell about Initiation" message (#31) which
// awards these points.
// This patch adds the message to that array, thus allowing the hero to tell
// that message (after completing the initiation) and gain the 3 points.
// A side effect of increasing the local156 array is that the next local
// array is shifted and shrinks in size from 4 words to 3. The patch changes
// the 2 locations in the script that reference that array, to point to the new
// location ($aa --> $ab). It is safe to shrink the 2nd array to 3 words
// because only the first element in it is ever used.
//
// Note: You have to re-enter the room in case a saved game was loaded from a
// previous version of ScummVM and that saved game was made inside that room.
//
// Applies to: English, French, German, Italian, Spanish and the GOG release.
// Responsible method: heap in script 285
// Fixes bug #7086
static const uint16 qfg3SignatureMissingPoints1[] = {
	// local[$9c] = [0 -41 -76 1 -30 -77 -33 -34 -35 -36 -37 -42 -80 999]
	// local[$aa] = [0 0 0 0]
	SIG_UINT16(0x0000),                 //   0 START MARKER
	SIG_MAGICDWORD,
	SIG_UINT16(0xFFD7),                 // -41 "Greet"
	SIG_UINT16(0xFFB4),                 // -76 "Say Good-bye"
	SIG_UINT16(0x0001),                 //   1 "Tell about Tarna"
	SIG_UINT16(0xFFE2),                 // -30 "Tell about Simani"
	SIG_UINT16(0xFFB3),                 // -77 "Tell about Prisoner"
	SIG_UINT16(0xFFDF),                 // -33 "Dispelled Leopard Lady"
	SIG_UINT16(0xFFDE),                 // -34 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDD),                 // -35 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDC),                 // -36 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDB),                 // -37 "Tell about Village"
	SIG_UINT16(0xFFD6),                 // -42 "Greet"
	SIG_UINT16(0xFFB0),                 // -80 "Say Good-bye"
	SIG_UINT16(0x03E7),                 // 999 END MARKER
	SIG_ADDTOOFFSET(+2),                // local[$aa][0]
	SIG_END
};

static const uint16 qfg3PatchMissingPoints1[] = {
	PATCH_ADDTOOFFSET(+14),
	PATCH_UINT16(0xFFE1),               // -31 "Tell about Initiation"
	PATCH_UINT16(0xFFDE),               // -34 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDD),               // -35 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDC),               // -36 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDB),               // -37 "Tell about Village"
	PATCH_UINT16(0xFFD6),               // -42 "Greet"
	PATCH_UINT16(0xFFB0),               // -80 "Say Good-bye"
	PATCH_UINT16(0x03E7),               // 999 END MARKER
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
	0xab,                               // local[$aa] ==> local[$ab]
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
// Fixes bug #6247
static const uint16 qfg3SignatureCombatSpeedThrottling1[] = {
	0x31, 0x0d,                         // bnt [skip code]
	SIG_MAGICDWORD,
	0x89, 0xd2,                         // lsg global[D2h]
	0x35, 0x00,                         // ldi 0
	0x1e,                               // gt?
	0x31, 0x06,                         // bnt [skip code]
	0xe1, 0xd2,                         // -ag global[D2h] (jump skips over this)
	0x81, 0x58,                         // lag global[58h]
	0xa3, 0x01,                         // sal local[01]
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling1[] = {
	0x80, 0xd2,                         // lsg global[D2h]
	0x14,                               // or
	0x31, 0x06,                         // bnt [skip code] - saves 4 bytes
	0xe1, 0xd2,                         // -ag global[D2h]
	0x81, 0x58,                         // lag global[58h]
	0xa3, 0x01,                         // sal local[01] (jump skips over this)
	// our code
	0x76,                               // push0
	0x43, 0x2c, 0x00,                   // callk GameIsRestarting <-- add this so that our speed throttler is triggered
	PATCH_END
};

static const uint16 qfg3SignatureCombatSpeedThrottling2[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(12),                     // priority 12
	SIG_UINT16(0),                      // underbits 0
	SIG_UINT16(0x4010),                 // signal 4010h
	SIG_ADDTOOFFSET(+18),
	SIG_UINT16(0),                      // scaleSignal 0
	SIG_UINT16(128),                    // scaleX
	SIG_UINT16(128),                    // scaleY
	SIG_UINT16(128),                    // maxScale
	SIG_UINT16(0),                      // cycleSpeed
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling2[] = {
	PATCH_ADDTOOFFSET(+32),
	PATCH_UINT16(5),                    // set cycleSpeed to 5
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
// Fixes bug #6693
static const uint16 qfg3SignatureRoom750Bounds1[] = {
	// (if (< (ego y?) 42)
	0x76,                               // push0 ("y")
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x04,                         // send 4
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35,   42,                         // ldi 42 <-- comparing ego.y with 42
	0x22,                               // lt?
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds1[] = {
	// (if (< (ego y?) 50)
	PATCH_ADDTOOFFSET(+8),
	50,                                 // 42 --> 50
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds2[] = {
	// (ego x: 294 y: 39)
	0x78,                               // push1 ("x")
	0x78,                               // push1
	0x38, SIG_UINT16(294),              // pushi 294
	0x76,                               // push0 ("y")
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39,   29,                         // pushi 29
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x0c,                         // send 12
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds2[] = {
	// (ego x: 320 y: 39)
	PATCH_ADDTOOFFSET(+3),
	PATCH_UINT16(320),                  // 294 --> 320
	PATCH_ADDTOOFFSET(+3),
	39,                                 //  29 -->  39
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds3[] = {
	// (ego setMotion: MoveTo 282 29 self)
	0x38, SIG_SELECTOR16(setMotion),    // pushi "setMotion" 0x133 in QfG3
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
	PATCH_UINT16(309),                  // 282 --> 309
	PATCH_ADDTOOFFSET(+1),
	35,                                 //  29 -->  35
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
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Quest for Glory 4

// The 'Trap::init' code incorrectly creates an int array for string data.
// Applies to at least: English CD
static const uint16 qfg4TrapArrayTypeSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x78,                      // push1
	0x38, SIG_UINT16(0x80),    // pushi $80 (128)
	SIG_MAGICDWORD,
	0x51, 0x0b,                // class $b (IntArray)
	0x4a, SIG_UINT16(0x06),    // send 6
	SIG_END
};

static const uint16 qfg4TrapArrayTypePatch[] = {
	PATCH_ADDTOOFFSET(+4),     // pushi $92 (new), push1
	0x38, PATCH_UINT16(0x100), // pushi $100 (256)
	0x51, 0x0d,                // class $d (ByteArray)
	PATCH_END
};

// The 'Trap::init' code incorrectly creates an int array for string data.
// Applies to at least: English floppy, German floppy
static const uint16 qfg4TrapArrayTypeFloppySignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x78,                      // push1
	0x38, SIG_UINT16(0x80),    // pushi $80 (128)
	SIG_MAGICDWORD,
	0x51, 0x0a,                // class $a (IntArray)
	0x4a, SIG_UINT16(0x06),    // send 6
	SIG_END
};

static const uint16 qfg4TrapArrayTypeFloppyPatch[] = {
	PATCH_ADDTOOFFSET(+4),     // pushi $92 (new), push1
	0x38, PATCH_UINT16(0x100), // pushi $100 (256)
	0x51, 0x0c,                // class $c (ByteArray)
	PATCH_END
};

// QFG4 has custom video benchmarking code inside a subroutine, which is called
// by 'glryInit::init', that needs to be disabled; see sci2BenchmarkSignature
// Applies to at least: English CD, German Floppy
static const uint16 qfg4BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class View
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x39, SIG_SELECTOR8(view), // pushi $e (view)
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0x270f),  // push $270f (9999)
	SIG_END
};

static const uint16 qfg4BenchmarkPatch[] = {
	0x35, 0x01, // ldi 0
	0xa1, 0xbf, // sag $bf (191)
	0x48,       // ret
	PATCH_END
};

// Right at the start of the game inside room 800, when automatically sliding down a slope
// an error may happen inside Grooper::doit caused by a timing issue.
//
// We delay a bit, so that hero::cycler should always be set.
//
// Applies to: English CD, English floppy, German floppy
// Responsible method: sFallsBackSide::changeState (script 803)
// Fixes bug #9801
static const uint16 qfg4SlidingDownSlopeSignature[] = {
	0x87, 0x01,                       // lap param[1]
	0x65, SIG_ADDTOOFFSET(+1),        // aTop state
	0x36,                             // push
	0x3c,                             // dup
	0x35, 0x00,                       // ldi 00
	0x1a,                             // eq?
	0x31, 0x30,                       // bnt [skip state 0]
	0x38, SIG_SELECTOR16(handsOff),   // pushi handsOff
	0x76,                             // push0
	0x81, 0x01,                       // lag global[1]
	0x4a, SIG_UINT16(0x0004),         // send 04
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	0x76,                             // push0
	0x81, 0x00,                       // lag global[0]
	0x4a, SIG_UINT16(0x0004),         // send 04
	0xa3, 0x00,                       // sal local[0]
	0x38, SIG_SELECTOR16(setStep),    // pushi setStep
	0x7a,                             // push2
	0x78,                             // push1
	0x78,                             // push1
	0x38, SIG_SELECTOR16(setMotion),  // pushi setMotion
	0x38, SIG_UINT16(0x0004),         // pushi 04
	0x51, SIG_ADDTOOFFSET(+1),        // class PolyPath
	0x36,                             // push
	0x39, 0x49,                       // pushi $49
	0x39, 0x50,                       // pushi $50
	0x7c,                             // pushSelf
	0x81, 0x00,                       // lag global[0]
	0x4a, SIG_UINT16(0x0014),         // send $14
	SIG_END
};

static const uint16 qfg4SlidingDownSlopePatch[] = {
	PATCH_ADDTOOFFSET(+5),
	0x2f, 0x34,                         // bt [skip state 0]
	0x38, PATCH_SELECTOR16(handsOff),   // pushi handsOff
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1]
	0x4a, PATCH_UINT16(0x0004),         // send 04

	0x78,                                     // push1
	0x39, 0x20,                               // pushi $20
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, $2
	0x38, PATCH_SELECTOR16(setStep),    // pushi setStep
	0x7a,                               // push2
	0x78,                               // push1
	0x78,                               // push1
	0x38, PATCH_SELECTOR16(setMotion),  // pushi setMotion
	0x38, PATCH_UINT16(0x0004),         // pushi 04
	0x51, PATCH_GETORIGINALBYTE(+44),   // class PolyPath
	0x36,                               // push
	0x39, 0x49,                         // pushi $49
	0x39, 0x50,                         // pushi $50
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	0x38, PATCH_SELECTOR16(cycleSpeed), // pushi cycleSpeed
	0x76,                               // push0
	0x4a, PATCH_UINT16(0x0018),         // send $18
	0xa3, 0x00,                         // sal local[0]
	0x3a,                               // toss
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
// Applies to: English CD, English floppy, German floppy
//
// Fixes bug #10693
static const uint16 qg4InnPathfindingSignature[] = {
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

static const uint16 qg4InnPathfindingPatch[] = {
	PATCH_ADDTOOFFSET(+30),
	0x38, PATCH_UINT16(0x013f), // pushi x = 319 (was 324)
	0x39, 0x77,                 // pushi y = 119
	PATCH_END
};

// When autosave is enabled, Glory::save() (script 0) deletes savegame files in
// a loop: while disk space is insufficient for a new save, or while there are
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
// Responsible method: Glory::save()
// Fixes bug: #10758
static const uint16 qg4AutosaveSignature[] = {
	0x30, SIG_ADDTOOFFSET(+2),          // bnt [end the loop]
	0x78,                               // push1
	0x39, 0x79,                         // pushi data
	0x76,                               // push0
	SIG_ADDTOOFFSET(+2),                // CD="lag global29", floppy="lat temp6"
	0x4a, SIG_UINT16(0x0004),           // send 04
	0x36,                               // push
	SIG_MAGICDWORD,
	0x43, 0x3f, SIG_UINT16(0x0002),     // callk CheckFreeSpace[3f], 02
	0x18,                               // not
	0x2f, 0x05,                         // bt 05 [skip other OR condition]
	0x8d, 0x09,                         // lst temp[9] (savegame file count)
	0x35, 0x14,                         // ldi 20d
	0x20,                               // ge?
	SIG_END
};

static const uint16 qg4AutosavePatch[] = {
	0x32, // ...                        // jmp [end the loop]
	PATCH_END
};

// The swamp areas have typos where a Grooper object is passed to
// View::setLoop(), a method which expects an integer to store in the "loop"
// property. This leads to arithmetic crashes later. We change it to
// Actor::setLooper().
//
// Applies to at least: English CD, English floppy
// Responsible method:
//                     Script 440
//                         sToWater::changeState()
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
static const uint16 qg4SetLooperSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push 1
	0x51, 0x5a,                         // class Grooper
	SIG_END
};

static const uint16 qg4SetLooperPatch1[] = {
	0x38, PATCH_SELECTOR16(setLooper),  // pushi setLooper
	PATCH_END
};

// As above, except it's an exported subclass of Grooper: stopGroop.
//
// Applies to at least: English CD, English floppy
// Responsible method:
//                      Script 10
//                          sJumpWater::changeState(3)
//                          sToJump::changeState(2)
// Fixes bug: #10777
static const uint16 qg4SetLooperSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi setLoop
	0x78,                               // push1
	0x7a,                               // push2
	0x39, 0x1c,                         // pushi 28d
	0x78,                               // push1
	0x43, 0x02, SIG_UINT16(0x0004),     // callk 04 (ScriptID 28 1)
	SIG_END
};
static const uint16 qg4SetLooperPatch2[] = {
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
	0x35, 0x06,                         // ldi 6
	0x1c,                               // ne?
	0x2f, 0x06,                         // bt 6d [skip remaining OR conditions]
	0x89, 0x78,                         // lsg 120d
	0x34, SIG_UINT16(0x01f4),           // ldi 500d
	0x1e,                               // gt?
	0x31, 0x02,                         // bnt 2d [skip the if block]
	0xc5, 0x00,                         // +at temp[0]
	SIG_END
};
static const uint16 qfg4MoonrisePatch[] = {
	0x35, 0x00,                         // ldi 0 (reset the is-night var)
	0xa3, 0x05,                         // sal local[5]
	0x33, 0x0f,                         // jmp 15d (skip waste bytes)
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
// Responsible method: rm320::init()
// Fixes bug: #10753
static const uint16 qfg4AbsentInnkeeperSignature[] = {
	SIG_MAGICDWORD,                     // (Block 10, partway through.)
	0x31, 0x1c,                         // bnt 28d [block 11]
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x03,                         // ldi 3d
	0x24,                               // le?
                                        // (~~ Junk begins ~~)
	0x2f, 0x0f,                         // bt 15d [after the calle]
	0x39, 0x03,                         // pushi 3d
	0x89, 0x7b,                         // lsg global[123]
	0x39, 0x04,                         // pushi 4d
	0x39, 0x05,                         // pushi 5d
	0x46, SIG_UINT16(0xfde7), SIG_UINT16(0x0005), SIG_UINT16(0x0006), // calle proc64999_5(global[123], 4, 5) 06
                                        // (~~ Junk ends ~~)
	0x31, 0x04,                         // bnt 4d [block 11]
	0x35, 0x0a,                         // ldi 10d
	0x33, 0x29,                         // jmp 41d (done, local[2]=acc)
                                        //
	SIG_ADDTOOFFSET(+25),               // (...Block 11...) (Patch ends after this.)
	SIG_ADDTOOFFSET(+14),               // (...Block 12...)
	SIG_ADDTOOFFSET(+2),                // (...Else 0...)
	0xa3, 0x02,                         // sal local[2] (all blocks set acc and jmp here)
	PATCH_END
};

static const uint16 qfg4AbsentInnkeeperPatch[] = {
	0x31, 0x0e,                         // bnt 14d [block 11]
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d (make it t <= 5)
	0x24,                               // le?
                                        // (*snip*)
	0x31, 0x07,                         // bnt 7d [block 11]
	0x35, 0x0a,                         // ldi 10d
	0x33, 0x3a,                         // jmp 58d (done, local[2]=acc)
                                        //
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
                                        // (14 freed bytes remain.)
                                        // (Use 7 freed bytes to prepend block 11.)
                                        // (And shift all of block 11 up by 7 bytes.)
                                        // (Use that new gap below to prepend block 12.)
                                        //
                                        // (Block 11. Prepend a time check.)
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d
	0x24,                               // le?
	0x31, 0x19,                         // bnt 25d [block 12]
                                        // (Block 11 original ops shift up.)
	0x78,                               // push1
	0x38, PATCH_UINT16(0x0084),         // pushi 132
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4(132) 02
	0x31, 0x0f,                         // bnt 15d [next block]
	0x78,                               // push1
	0x38, PATCH_UINT16(0x0086),         // pushi 134
	0x45, 0x04, PATCH_UINT16(0x0002),   // callb proc0_4(134) 02
	0x18,                               // not
	0x31, 0x04,                         // bnt 4d [block 12]
	0x35, 0x0b,                         // ldi 11d
	0x33, 0x17,                         // jmp 23d (done, local[2]=acc)
                                        //
                                        // (Block 12. Prepend a time check.)
	0x89, 0x7b,                         // lsg global[123]
	0x35, 0x05,                         // ldi 5d
	0x24,                               // le?
	0x31, 0x0e,                         // bnt 14d [else block]
                                        // (Block 12 original ops continue here.)
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

// When the fortune teller's wagon room is disposed, it attempts to call
// hero::show(), hero has a null "plane" property, and ScummVM crashes.
//
// The problematic line was removed in the CD edition. We remove it, too.
//
// Note: This patch is a workaround. The floppy edition SSCI did not crash, and
// its implementation of AddScreenItem() should be checked to find out why.
//
// Applies to at least: English floppy, German floppy
// Responsible method: rm470::dispose()
// Fixes bug: #10778
static const uint16 qfg4MagdaDisposalSignature[] = {
	0x38, SIG_SELECTOR16(posn),         // posn
	SIG_ADDTOOFFSET(+8),                // ...
	SIG_MAGICDWORD,
	0x81, 0x00,                         // lag global[0] (hero)
	0x4a, SIG_UINT16(0x000c),           // send 12d (posn: 1000 1000 show:)
	SIG_END
};

static const uint16 qfg4MagdaDisposalPatch[] = {
	0x33, 0x0e,                         // jmp 14d (skip the entire hero send)
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
	0x4a, SIG_UINT16(0x003e),           // send 3e
	0x36,                               // push
	SIG_ADDTOOFFSET(+5),                // ...
	0x38, SIG_SELECTOR16(handsOn),      // pushi handsOn (begin clobbering)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1] (Glory)
	0x4a, SIG_UINT16(0x0004),           // send 04
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	SIG_END
};

static const uint16 qfg4CrestBookshelfCDPatch[] = {
	PATCH_ADDTOOFFSET(+9),
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x72, PATCH_UINT16(0x01a4),         // lofsa sLeaveSecretly
	0x36,                               // push
	0x81, 0x02,                         // lag global[2] (rm663)
	0x4a, PATCH_UINT16(0x0006),         // send 06
	0x34, PATCH_UINT16(0x0000),         // ldi 0 (waste 3 bytes)
	PATCH_END
};

// Applies to at least: English floppy, German floppy
static const uint16 qfg4CrestBookshelfFloppySignature[] = {
	SIG_MAGICDWORD,
	0x4a, SIG_UINT16(0x003e),           // send 3e
	0x36,                               // push
	SIG_ADDTOOFFSET(+5),                // ...
	0x38, SIG_SELECTOR16(handsOn),      // pushi handsOn (begin clobbering)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1] (Glory)
	0x4a, SIG_UINT16(0x0004),           // send 04
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	SIG_END
};

static const uint16 qfg4CrestBookshelfFloppyPatch[] = {
	PATCH_ADDTOOFFSET(+9),
	0x38, PATCH_SELECTOR16(setScript),  // pushi setScript
	0x78,                               // push1
	0x72, PATCH_UINT16(0x018c),         // lofsa sLeaveSecretly
	0x36,                               // push
	0x81, 0x02,                         // lag global[2] (rm663)
	0x4a, PATCH_UINT16(0x0006),         // send 06
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
 
// The castle's great hall has a doorMat region that intermittently sends hero
// back to the room they just left (the barrel room) the instant they arrive.
//
// Entry from room 623 starts hero at (0, 157), the edge of the doorMat. We
// shrink the region by 2 pixels. Then sEnterTheRoom moves hero safely across.
// The region is a rectangle. Point 0 is top-left. Point 3 is bottom-left.
//
// Does not apply to English floppy 1.0. It lacked a western doorMat entirely.
//
// Applies to at least: English CD, English floppy, German floppy
// Responsible method: vClosentDoor::init()
// Fixes bug: #10731
static const uint16 qfg4GreatHallEntrySignature[] = {
	SIG_MAGICDWORD,
	0x76,                               // push0 (point 0)
	0x38, SIG_UINT16(0x0088),           // pushi 136
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
	0x43, 0x0a, SIG_UINT16(0x00002),    // callk 02 (SetNowSeen stackedView)
	0x36,                               // push (void func didn't set acc!)
	0x35, 0x01,                         // ldi 1d
	0x14,                               // or (whatever that was, make it non-zero)
	SIG_END
};

static const uint16 qfg4ConditionalVoidPatch[] = {
	PATCH_ADDTOOFFSET(+4),              //
	0x78,                               // push1 (Feed OR a literal 1)
	PATCH_END
};

// The copy protection in floppy versions has a script bug which attempts to add
//  views with no planes to the screen. Our interpreter does not allow this and
//  treats it as an error. This appears to work in Sierra's interpreter, which
//  presumably ignores it, although the script bug was fixed in the CD version.
//
// When asking Dr. Cranium in room 370 about certain potions the game switches
//  to a copy protection screen and then back to the conversation. Before the
//  switch, craniumTalker is disposed, which in turn disposes craniumThumbs and
//  craniumBrow. Disposing these views clears their planes. After returning from
//  the protection screen craniumTalker:showAgain is called even though it has
//  been disposed. This causes kAddScreenItem to be called on views without
//  planes, which is treated as an error by our interpreter.
//
// We work around this by reinitializing craniumTalker after the copy protection
//  so that showAgain can be safely called. craniumTalker is reinitialized when
//  navigating through the conversation menus so this is normal behavior.
//
// Applies to: English PC Floppy, German PC Floppy
// Responsible method: delayMsg:changeState(0)
// Fixes bug: #10773
static const uint16 qfg4CopyProtectionSignature[] = {
	0x31, 0x06,                         // bnt 06
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0x65, 0x24,                         // aTop register
	SIG_ADDTOOFFSET(+6),
	0x38, SIG_UINT16(0x0300),           // pushi 0300 [ showAgain, hard-coded for floppy ]
	SIG_ADDTOOFFSET(+11),
	0x4a, SIG_UINT16(0x0004),           // send 04 [ craniumTalker: showAgain ]
	SIG_END
};

static const uint16 qfg4CopyProtectionPatch[] = {
	0x65, 0x24,                         // aTop register
	0x38, PATCH_SELECTOR16(init),       // pushi init
	0x76,                               // push0
	PATCH_ADDTOOFFSET(+20),
	0x4a, PATCH_UINT16(0x0008),         // send 08 [ craniumTalker: init, showAgain ]
	PATCH_END
};

//          script, description,                                     signature                      patch
static const SciScriptPatcherEntry qfg4Signatures[] = {
	{  true,     0, "prevent autosave from deleting save games",   1, qg4AutosaveSignature,          qg4AutosavePatch },
	{  true,     1, "disable volume reset on startup",             1, sci2VolumeResetSignature,      sci2VolumeResetPatch },
	{  true,     1, "disable video benchmarking",                  1, qfg4BenchmarkSignature,        qfg4BenchmarkPatch },
	{  true,     7, "fix consecutive moonrises",                   1, qfg4MoonriseSignature,         qfg4MoonrisePatch },
	{  true,    10, "fix setLooper calls (2/2)",                   2, qg4SetLooperSignature2,        qg4SetLooperPatch2 },
	{  true,    31, "fix setScaler calls",                         1, qfg4SetScalerSignature,        qfg4SetScalerPatch },
	{  true,    41, "fix conditional void calls",                  3, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,    83, "fix incorrect array type",                    1, qfg4TrapArrayTypeSignature,    qfg4TrapArrayTypePatch },
	{  true,    83, "fix incorrect array type (floppy)",           1, qfg4TrapArrayTypeFloppySignature,    qfg4TrapArrayTypeFloppyPatch },
	{  true,   320, "fix pathfinding at the inn",                  1, qg4InnPathfindingSignature,    qg4InnPathfindingPatch },
	{  true,   320, "fix talking to absent innkeeper",             1, qfg4AbsentInnkeeperSignature,  qfg4AbsentInnkeeperPatch },
	{  true,   370, "Floppy: fix copy protection",                 1, qfg4CopyProtectionSignature,   qfg4CopyProtectionPatch },
	{  true,   440, "fix setLooper calls (1/2)",                   1, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   470, "fix Magda room disposal",                     1, qfg4MagdaDisposalSignature,    qfg4MagdaDisposalPatch },
	{  true,   530, "fix setLooper calls (1/2)",                   4, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   535, "fix setLooper calls (1/2)",                   4, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   541, "fix setLooper calls (1/2)",                   5, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   542, "fix setLooper calls (1/2)",                   5, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   543, "fix setLooper calls (1/2)",                   5, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   545, "fix setLooper calls (1/2)",                   5, qg4SetLooperSignature1,        qg4SetLooperPatch1 },
	{  true,   630, "fix great hall entry from barrel room",       1, qfg4GreatHallEntrySignature,   qfg4GreatHallEntryPatch },
	{  true,   633, "fix stairway pathfinding",                    1, qfg4StairwayPathfindingSignature, qfg4StairwayPathfindingPatch },
	{  false,  663, "CD: fix crest bookshelf",                     1, qfg4CrestBookshelfCDSignature, qfg4CrestBookshelfCDPatch },
	{  false,  663, "Floppy: fix crest bookshelf",                 1, qfg4CrestBookshelfFloppySignature,   qfg4CrestBookshelfFloppyPatch },
	{  true,   663, "CD/Floppy: fix crest bookshelf motion",       1, qfg4CrestBookshelfMotionSignature,   qfg4CrestBookshelfMotionPatch },
	{  true,   800, "fix setScaler calls",                         1, qfg4SetScalerSignature,        qfg4SetScalerPatch },
	{  true,   803, "fix sliding down slope",                      1, qfg4SlidingDownSlopeSignature, qfg4SlidingDownSlopePatch },
	{  true,   810, "fix conditional void calls",                  1, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   830, "fix conditional void calls",                  2, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   835, "fix conditional void calls",                  3, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   840, "fix conditional void calls",                  2, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   855, "fix conditional void calls",                  1, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true,   870, "fix conditional void calls",                  5, qfg4ConditionalVoidSignature,  qfg4ConditionalVoidPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,        sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,        sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,        sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
//  script 298 of sq4/floppy has an issue. object "nest" uses another property
//   which isn't included in property count. We return 0 in that case, the game
//   adds it to nest::x. The problem is that the script also checks if x exceeds
//   we never reach that of course, so the pterodactyl-flight will go endlessly
//   we could either calculate property count differently somehow fixing this
//   but I think just patching it out is cleaner.
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

// Floppy-only: When the player tries to throw something at the sequel police in Space Quest X (zero g zone),
//   the game will first show a textbox and then cause a signature mismatch in ScummVM/
//   crash the whole game in Sierra SCI/display garbage (the latter when the Sierra "patch" got applied).
//
// All of this is caused by a typo in the script. Right after the code for showing the textbox,
//  there is more similar code for showing another textbox, but without a pointer to the text.
//  This has to be a typo, because there is no unused text to be found within that script.
//
// Sierra's "patch" didn't include a proper fix (as in a modified script). Instead they shipped a dummy
//  text resource, which somewhat "solved" the issue in Sierra SCI, but it still showed another textbox
//  with garbage in it. Funnily Sierra must have known that, because that new text resource contains:
//  "Hi! This is a kludge!"
//
// We properly fix it by removing the faulty code.
// Applies to at least: English Floppy
// Responsible method: sp1::doVerb
// Fixes bug: found by SCI developer
static const uint16 sq4FloppySignatureThrowStuffAtSequelPoliceBug[] = {
	0x47, 0xff, 0x00, 0x02,             // call export 255_0, 2
	0x3a,                               // toss
	SIG_MAGICDWORD,
	0x36,                               // push
	0x47, 0xff, 0x00, 0x02,             // call export 255_0, 2
	SIG_END
};

static const uint16 sq4FloppyPatchThrowStuffAtSequelPoliceBug[] = {
	PATCH_ADDTOOFFSET(+5),
	0x48,                            // ret
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
	0x51, SIG_ADDTOOFFSET(+1),          // class [MoveTo]
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
	0x39, SIG_SELECTOR8(number),        // pushi (number)
	0x78,                               // push1
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0188),           // pushi 188h
	0x38, SIG_UINT16(0x018b),           // pushi 18Bh
	0x43, 0x3C, 0x04,                   // call kRandom, 4
	0x36,                               // push
	0x39, SIG_SELECTOR8(play),          // pushi (play)
	0x76,                               // push0
	0x81, 0x64,                         // lag global[64h]
	0x4a, 0x0a,                         // send 0Ah
	0x38, SIG_SELECTOR16(setScript),    // pushi (setScript)
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
	0x38, SIG_SELECTOR16(setScript),    // pushi (setScript)
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
	0x30, SIG_UINT16(0x00b7),           // bnt [now directly to last state code, saving 6 bytes]
	0x32, PATCH_UINT16(+154),           // jmp [to our new code]
	// 1 not used byte here
	SIG_ADDTOOFFSET(+128),              // skip over to substate 1 code of state 1Ch code
	0x32, PATCH_UINT16(0x003f),         // substate 0 code, jumping to toss/ret
	// directly start with substate 1 code, saving 7 bytes
	0x39, PATCH_SELECTOR8(number),      // pushi 28h (number)
	0x78,                               // push1
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0188),         // pushi 188h
	0x38, PATCH_UINT16(0x018b),         // pushi 18Bh
	0x43, 0x3C, 0x04,                   // call kRandom, 4
	0x36,                               // push
	0x39, PATCH_SELECTOR8(play),        // pushi (play)
	0x76,                               // push0
	0x81, 0x64,                         // lag global[64h]
	0x4a, 0x0a,                         // send 0Ah
	0x33, 0x1a,                         // jmp [state 1Dh directly, saving 12 bytes]
	// additional code for playing missing audio (18 bytes w/o jmp back)
	0x89, 0x5a,                         // lsg global[5Ah]
	0x35, 0x01,                         // ldi 1
	0x1c,                               // ne?
	0x31, 0x0b,                         // bnt [skip play audio]
	0x38, PATCH_SELECTOR16(say),        // pushi 0123h (say)
	0x78,                               // push1
	0x39, 0x14,                         // pushi 14h
	0x72, PATCH_UINT16(0x0850),         // lofsa newRob
	0x4a, 0x06,                         // send 06
	// now get back
	0x39, 0x0c,                         // pushi 0Ch
	0x32, PATCH_UINT16(0xff50),         // jmp back
	PATCH_END
};

// It seems that Sierra forgot to set a script flag, when cleaning out the bank account
// in Space Quest 4 CD. This was probably caused by the whole bank account interaction
// getting a rewrite and polish in the CD version.
//
// Because of this bug, points for changing back clothes will not get awarded, which
// makes it impossible to get a perfect point score in the CD version of the game.
// The points are awarded by rm371::doit in script 371.
//
// We fix this. Bug also happened, when using the original interpreter.
// Bug does not happen for PC floppy.
//
// Attention: Some Let's Plays on youtube show that points are in fact awarded. Which is true.
//            But those Let's Plays were actually created by playing a hacked Space Quest 4 version
//            (which is part Floppy, part CD version - we consider it to be effectively pirated)
//            and not the actual CD version of Space Quest 4.
//            It's easy to identify - talkie + store called "Radio Shack" -> is hacked version.
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
	0x39, 0x1d,                         // ldi 1Dh
	0x45, 0x07, 0x02,                   // call export 7 of script 0 (set flag) -> effectively sets global 73h, bit 2
	0x35, 0x02,                         // ldi 02
	0x65, 0x1c,                         // aTop cycles
	0x33, 0x05,                         // jmp [toss/ret]
	// check for state 3 code removed to save 6 bytes
	PATCH_END
};


// For Space Quest 4 CD, Sierra added a pick up animation for Roger, when he picks up the rope.
//
// When the player is detected by the zombie right at the start of the game, while picking up the rope,
// scripts bomb out. This also happens, when using the original interpreter.
//
// This is caused by code, that's supposed to make Roger face the arriving drone.
// We fix it, by checking if ego::cycler is actually set before calling that code.
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
	0x45, 0x02, 0x00,                   // call export 2 of script 0 -> disable controls
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x02e9),           // jmp [end]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x1e,                         // bnt [state 4 check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // call export 2 of script 0 -> disable controls again??
	0x7a,                               // push2
	0x89, 0x00,                         // lsg global[0]
	0x72, SIG_UINT16(0x0242),           // lofsa deathDroid
	0x36,                               // push
	0x45, 0x0d, 0x04,                   // call export 13 of script 0 -> set heading of ego to face droid
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
	0x45, 0x02, 0x00,                   // call export 2 of script 0 (disable controls, actually not needed)
	0x38, PATCH_SELECTOR16(cycler),     // pushi cycler
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 04 (get ego::cycler)
	0x30, PATCH_UINT16(10),             // bnt [jump over heading call]
	PATCH_END
};

// The scripts in SQ4CD support simultaneous playing of speech and subtitles,
// but this was not available as an option. The following two patches enable
// this functionality in the game's GUI options dialog.
// Patch 1: iconTextSwitch::show, called when the text options button is shown.
// This is patched to add the "Both" text resource (i.e. we end up with
// "Speech", "Text" and "Both")
static const uint16 sq4CdSignatureTextOptionsButton[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x53,                         // sag 0x53
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

// Patch 2: Adjust a check in babbleIcon::init, which handles the babble icon
// (e.g. the two guys from Andromeda) shown when dying/quitting.
// Fixes bug: #6068
static const uint16 sq4CdSignatureBabbleIcon[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x26,                         // bnt 26  [02a7]
	SIG_END
};

static const uint16 sq4CdPatchBabbleIcon[] = {
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x2f, 0x26,                         // bt 26  [02a7]
	PATCH_END
};

// Patch 3: Add the ability to toggle among the three available options,
// when the text options button is clicked: "Speech", "Text" and "Both".
// Refer to the patch above for additional details.
// iconTextSwitch::doit (called when the text options button is clicked)
static const uint16 sq4CdSignatureTextOptions[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack)
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 0x01
	0x1a,                               // eq? (global 90 == 1)
	0x31, 0x06,                         // bnt 0x06 (0x0691)
	0x35, 0x02,                         // ldi 0x02
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x33, 0x0a,                         // jmp 0x0a (0x69b)
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 0x02
	0x1a,                               // eq? (global 90 == 2)
	0x31, 0x04,                         // bnt 0x04 (0x069b)
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x3a,                               // toss
	0x38, SIG_SELECTOR16(show),         // pushi 0x00d9
	0x76,                               // push0
	0x54, 0x04,                         // self 0x04
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4CdPatchTextOptions[] = {
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack)
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 0x03 (acc = 3)
	0x1a,                               // eq? (global 90 == 3)
	0x2f, 0x07,                         // bt 0x07
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack again)
	0x35, 0x01,                         // ldi 0x01 (acc = 1)
	0x02,                               // add: acc = global 90 (on stack) + 1 (previous acc value)
	0x33, 0x02,                         // jmp 0x02
	0x35, 0x01,                         // ldi 0x01 (reset acc to 1)
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x33, 0x03,                         // jmp 0x03 (jump over the wasted bytes below)
	0x34, PATCH_UINT16(0x0000),         // ldi 0x0000 (waste 3 bytes)
	0x3a,                               // toss
	// (the rest of the code is the same)
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry sq4Signatures[] = {
	{  true,   298, "Floppy: endless flight",                         1, sq4FloppySignatureEndlessFlight,               sq4FloppyPatchEndlessFlight },
	{  true,   700, "Floppy: throw stuff at sequel police bug",       1, sq4FloppySignatureThrowStuffAtSequelPoliceBug, sq4FloppyPatchThrowStuffAtSequelPoliceBug },
	{  true,    45, "CD: walk in from below for room 45 fix",         1, sq4CdSignatureWalkInFromBelowRoom45,           sq4CdPatchWalkInFromBelowRoom45 },
	{  true,   391, "CD: missing Audio for universal remote control", 1, sq4CdSignatureMissingAudioUniversalRemote,     sq4CdPatchMissingAudioUniversalRemote },
	{  true,   396, "CD: get points for changing back clothes fix",   1, sq4CdSignatureGetPointsForChangingBackClothes, sq4CdPatchGetPointsForChangingBackClothes },
	{  true,   701, "CD: getting shot, while getting rope",           1, sq4CdSignatureGettingShotWhileGettingRope,     sq4CdPatchGettingShotWhileGettingRope },
	{  true,     0, "CD: Babble icon speech and subtitles fix",       1, sq4CdSignatureBabbleIcon,                      sq4CdPatchBabbleIcon },
	{  true,   818, "CD: Speech and subtitles option",                1, sq4CdSignatureTextOptions,                     sq4CdPatchTextOptions },
	{  true,   818, "CD: Speech and subtitles option button",         1, sq4CdSignatureTextOptionsButton,               sq4CdPatchTextOptionsButton },
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
	SIG_MAGICDWORD, SIG_SELECTOR8(cel), // pushi "cel"
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
//  Near that space ship are 2 force field generators.
//  When you look at the top of those generators, the game will crash.
//  This happens also in Sierra SCI. It's caused by a jump, that goes out of bounds.
//  We currently do not know if this was caused by a compiler glitch or if it was a developer error.
//  Anyway we patch this glitchy code, so that the game won't crash anymore.
//
// Applies to at least: English Floppy
// Responsible method: radar1::doVerb
// Fixes bug: #6816
static const uint16 sq1vgaSignatureUlenceFlatsGeneratorGlitch[] = {
	SIG_MAGICDWORD, 0x1a,               // eq?
	0x30, SIG_UINT16(0xcdf4),           // bnt absolute 0xf000
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
	0x38, SIG_SELECTOR16(timesShownID), // push "timesShownID"
	0x78,                               // push1
	0x38, SIG_SELECTOR16(timesShownID), // push "timesShownID"
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
	0x38, PATCH_SELECTOR16(timesShownID), // push "timesShownID"
	0x76,                               // push0
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x04,                         // send 0x04 (get timesShownID)
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x02,                               // add
	0x36,                               // push (this push corresponds to the wrong one above)
	0x38, PATCH_SELECTOR16(timesShownID), // push "timesShownID"
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
	0x4a, 0x04,                         // send 04 (get [current room].script)
	0x30, SIG_UINT16(0x0005),           // bnt [further method code]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_UINT16(0x0052),           // jmp [super-call]
	0x89, 0xa6,                         // lsg global[a6] <-- flag gets set to 1 when ego went up the skeleton tail, when going down it's set to 2
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0012),           // bnt [PChase set code], in case global A6 <> 1
	0x81, 0xb5,                         // lag global[b5]
	0x30, SIG_UINT16(0x000d),           // bnt [PChase set code], in case global B5 == 0
	0x38, SIG_UINT16(0x008c),           // pushi 008c
	0x78,                               // push1
	0x72, SIG_UINT16(0x1cb6),           // lofsa 1CB6 (moveToPath)
	0x36,                               // push
	0x54, 0x06,                         // self 06
	0x32, SIG_UINT16(0x0038),           // jmp [super-call]
	// PChase set call
	0x81, 0xb5,                         // lag global[B5]
	0x18,                               // not
	0x30, SIG_UINT16(0x0032),           // bnt [super-call], in case global B5 <> 0
	// followed by:
	// is spider in current room
	// is global A6h == 2? -> set PChase
	SIG_END
}; // 58 bytes)

// Global A6h <> 1 (did NOT went up the skeleton)
//  Global B5h = 0 -> set PChase
//  Global B5h <> 0 -> do not do anything
// Global A6h = 1 (did went up the skeleton)
//  Global B5h = 0 -> set PChase
//  Global B5h <> 0 -> set moveToPath

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
	0x39, 0x01,                         // push 01 (waste 1 byte)
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

//          script, description,                                      signature                                   patch
static const SciScriptPatcherEntry sq1vgaSignatures[] = {
	{  true,    45, "Ulence Flats: timepod graphic glitch",        1, sq1vgaSignatureUlenceFlatsTimepodGfxGlitch, sq1vgaPatchUlenceFlatsTimepodGfxGlitch },
	{  true,    45, "Ulence Flats: force field generator glitch",  1, sq1vgaSignatureUlenceFlatsGeneratorGlitch,  sq1vgaPatchUlenceFlatsGeneratorGlitch },
	{  true,    58, "Sarien armory droid zapping ego first time",  1, sq1vgaSignatureEgoShowsCard,                sq1vgaPatchEgoShowsCard },
	{  true,   704, "spider droid timing issue",                   1, sq1vgaSignatureSpiderDroidTiming,           sq1vgaPatchSpiderDroidTiming },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// The toolbox in sq5 is buggy. When you click on the upper part of the "put
//  in inventory"-button (some items only - for example the hole puncher - at the
//  upper left), points will get awarded correctly and the item will get put into
//  the player's inventory, but you will then get a "not here" message and the
//  item will also remain to be the current mouse cursor.
// The bug report also says that items may get lost. I wasn't able to reproduce
//  that part.
// This is caused by the mouse-click event getting reprocessed (which wouldn't
//  be a problem by itself) and during this reprocessing coordinates are not
//  processed the same as during the first click (script 226 includes a local
//  subroutine, which checks coordinates in a hardcoded way w/o port-adjustment).
// Because of this, the hotspot for the button is lower than it should be, which
//  then results in the game thinking that the user didn't click on the button
//  and also results in the previously mentioned message.
// This happened in Sierra SCI as well (of course).
// We fix it by combining state 0 + 1 of takeTool::changeState and so stopping
//  the event to get reprocessed. This was the only way possible, because everything
//  else is done in SCI system scripts and I don't want to touch those.
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
	0x81, 0x5b,                    // lag 5b
	0x4a, 0x0e,                    // send 0e
	0x32, SIG_UINT16(0x0088),      // jmp [end-of-method]
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
// Fixes bug #7155
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

//          script, description,                                      signature                             patch
static const SciScriptPatcherEntry sq5Signatures[] = {
	{  true,   226, "toolbox fix",                                 1, sq5SignatureToolboxFix,               sq5PatchToolboxFix },
	{  true,  1000, "drive bay pathfinding fix",                   1, sq5SignatureDriveBayPathfindingFix,   sq5PatchDriveBayPathfindingFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark RAMA

// RAMA has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature
static const uint16 ramaBenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(view), // pushi view
	SIG_MAGICDWORD,
	0x78,                       // push1
	0x38, SIG_UINT16(0xfdd4),   // pushi 64980
	SIG_END
};

static const uint16 ramaBenchmarkPatch[] = {
	0x34, PATCH_UINT16(0x2710), // ldi 10000
	0x48,                       // ret
	PATCH_END
};

// RAMA uses a custom save game format that game scripts read and write
// manually. The save game format serialises object references, which SSCI could
// be done just by writing int16s (since object references were just 16-bit
// indexes), but in ScummVM we have to write the full 32-bit reg_t. We hijack
// kFileIOReadWord/kFileIOWriteWord to do this for us, but we need the game to
// agree to use those kFileIO calls instead of doing raw reads and creating its
// own numbers, as it tries to do here in `SaveManager::readWord`.
static const uint16 ramaSerializeRegTSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(newWith), // pushi $10b (newWith)
	0x7a,                          // push2
	0x7a,                          // push2
	0x72, SIG_UINT16(0x00),        // lofsa ""
	0x36,                          // push
	0x51, 0x0f,                    // class Str
	SIG_END
};

static const uint16 ramaSerializeRegTPatch1[] = {
	0x38, PATCH_SELECTOR16(readWord),    // pushi readWord
	0x76,                                // push0
	0x62, PATCH_SELECTOR16(saveFilePtr), // pToa saveFilePtr
	0x4a, PATCH_UINT16(0x04),            // send 4
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
static const uint16 ramaNukeTimerSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2),              // line whatever
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(getSubscriberObj), // pushi $3ca (getSubscriberObj)
	0x76,                                   // push0
	0x54, SIG_UINT16(0x04),                 // self 4
	SIG_END
};

static const uint16 ramaNukeTimerPatch[] = {
	0x38, PATCH_SELECTOR16(getSubscriberObj), // pushi $3ca (getSubscriberObj)
	0x78,                                     // push1
	0x38, PATCH_UINT16(0x01),                 // pushi 1 (wasting bytes)
	0x54, PATCH_UINT16(0x06),                 // self 6
	PATCH_END
};

// When opening a datacube on the pocket computer, `DocReader::init` will try
// to perform arithmetic on a pointer to `thighComputer::plane` and then use the
// resulting value as the priority for the DocReader. This happened to work in
// SSCI because the plane pointer would just be a high numeric value, but
// ScummVM needs an actual number, not a pointer.
// Applies to at least: US English
static const uint16 ramaDocReaderInitSignature[] = {
	0x39, SIG_SELECTOR8(priority), // pushi $1a (priority)
	0x78,                          // push1
	0x39, SIG_SELECTOR8(plane),    // pushi $19 (plane)
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

// It is not possible to change the directory for ScummVM save games, so disable
// the "change directory" button in the RAMA save dialogue
static const uint16 ramaChangeDirSignature[] = {
	SIG_MAGICDWORD,
	0x7e, SIG_UINT16(0x64),     // line 100
	0x39, SIG_SELECTOR8(state), // pushi $1d (state)
	0x78,                       // push1
	0x39, 0x03,                 // pushi 3
	0x72, SIG_ADDTOOFFSET(+2),  // lofsa changeDirI
	0x4a, SIG_UINT16(0x0e),     // send 14
	SIG_END
};

static const uint16 ramaChangeDirPatch[] = {
	PATCH_ADDTOOFFSET(+6),    // line 100, pushi state, push1
	0x39, 0x00,               // pushi 0
	PATCH_END
};

static const SciScriptPatcherEntry ramaSignatures[] = {
	{  true,    55, "fix bad DocReader::init priority calculation",   1, ramaDocReaderInitSignature,      ramaDocReaderInitPatch },
	{  true,    85, "fix SaveManager to use normal readWord calls",   1, ramaSerializeRegTSignature1,     ramaSerializeRegTPatch1 },
	{  true,   201, "fix crash restoring save games using NukeTimer", 1, ramaNukeTimerSignature,          ramaNukeTimerPatch },
	{  true, 64908, "disable video benchmarking",                     1, ramaBenchmarkSignature,          ramaBenchmarkPatch },
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
	0x8f, 0x01,                        // lsp 1
	0x59, 0x02,                        // &rest 2
	0x57, 0x7f, SIG_UINT16(0x06),      // super ShiversProp[7f], 6
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
	0x39, SIG_SELECTOR8(play), // pushi $33
	0x38, SIG_UINT16(0x06),    // pushi 6
	SIG_END
};

static const uint16 shiversGodsIxupiPlaySoundPatch[] = {
	0x38, PATCH_SELECTOR16(fade), // pushi $f3
	0x39, 0x06,                   // pushi 6
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry shiversSignatures[] = {
	{  true,   990, "fix volume & brightness sliders",             2, shiversEventSuperCallSignature,     shiversEventSuperCallPatch },
	{  true, 23090, "fix bad Ixupi sound call",                    1, shiversGodsIxupiPlaySoundSignature, shiversGodsIxupiPlaySoundPatch },
	{  true, 35170, "fix CCTV joystick interaction",               1, shiversEventSuperCallSignature,     shiversEventSuperCallPatch },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,             sci2BenchmarkPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Space Quest 6

// After the explosion in the Quarters of Deepship 86, the game tries to perform
// a dramatic long fade, but does this with an unreasonably large number of
// divisions which takes tens of seconds to finish (because transitions are not
// CPU-dependent in ScummVM).
static const uint16 sq6SlowTransitionSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x578), // pushi $578
	0x51, 0x33,              // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch1[] = {
	0x38, SIG_UINT16(0x1f4), // pushi 500
	PATCH_END
};

// For whatever reason, SQ6 sets the default number of transition divisions to
// be a much larger value at startup (200 vs 30) if it thinks it is running in
// Windows. Room 410 (eulogy room) also unconditionally resets divisions to the
// larger value.
static const uint16 sq6SlowTransitionSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0xc8), // pushi $c8
	0x51, 0x33,             // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch2[] = {
	0x38, SIG_UINT16(0x1e), // pushi 30
	PATCH_END
};

// SQ6 has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature. (The sci2BenchmarkPatch is suitable for use with
// SQ6 as well.)
static const uint16 sq6BenchmarkSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(init),  // pushi init
	0x76,                        // push0
	0x7e, SIG_ADDTOOFFSET(+2),   // line
	0x38, SIG_SELECTOR16(posn),  // pushi $140 (posn)
	SIG_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry sq6Signatures[] = {
	{  true,     0, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,    15, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,    22, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   410, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,   460, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   500, "fix slow transitions",                        1, sq6SlowTransitionSignature1,     sq6SlowTransitionPatch1 },
	{  true,   510, "fix invalid array construction",              1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true, 64908, "disable video benchmarking",                  1, sq6BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games (1/2)",         1, sci2NumSavesSignature1,          sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games (2/2)",         1, sci2NumSavesSignature2,          sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,          sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Torins Passage

// A subroutine that gets called by 'Torin::init' unconditionally resets the
// audio volumes to defaults, but the game should always use the volume stored
// in ScummVM. This patch is basically identical to the patch for LSL7, except
// that they left line numbers in the LSL7 scripts and changed the music volume.
// Applies to at least: English CD
static const uint16 torinVolumeResetSignature1[] = {
	SIG_MAGICDWORD,
	0x35, 0x28, // ldi $28
	0xa1, 0xe3, // sag $e3 (music volume)
	0x35, 0x3c, // ldi $3c
	0xa1, 0xe4, // sag $e4 (sfx volume)
	0x35, 0x64, // ldi $64
	0xa1, 0xe5, // sag $e5 (speech volume)
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
static const uint16 torinVolumeResetSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(readWord), // pushi $020b (readWord)
	0x76,                           // push0
	SIG_ADDTOOFFSET(6),             // advance file stream
	0xa1, 0xe3,                     // sag $e3 (music volume)
	SIG_ADDTOOFFSET(10),            // advance file stream
	0xa1, 0xe4,                     // sag $e4 (sfx volume)
	SIG_ADDTOOFFSET(10),            // advance file stream
	0xa1, 0xe5,                     // sag $e5 (speech volume)
	SIG_END
};

static const uint16 torinVolumeResetPatch2[] = {
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_END
};

// In Escarpa, it is possible for Boogle to be left outside of Torin's bag
// when fast-forwarding through the exit animation of the seraglio. If this
// happens, when the player goes from the seraglio to the dragon's cave and then
// tries to worm Boogle to the left side of the cave, the game will hang because
// Boogle is on the wrong side of the navigable area barrier and cannot move
// through it to continue the cutscene. This patch fixes the fast-forward code
// 'soBoogleBackUp::ff' in the seraglio so that Boogle's in-the-bag flag is set
// when fast forwarding.
// Applies to at least: English CD, Spanish CD
static const uint16 torinSeraglioBoogleFlagSignature[] = {
	0x35, 0x00,                 // ldi 0
	SIG_MAGICDWORD,
	0xa3, 0x00,                 // sal 0
	0x38, SIG_SELECTOR16(test), // pushi test
	SIG_ADDTOOFFSET(0x5a),      // all the rest of the method
	// CHECKME: Spanish version seems to have a total of 0x5d bytes from this point to the ret
	// FIXME: Check for end of method (e.g. ret) and add different signatures in case localized versions are different
	SIG_END
};

static const uint16 torinSeraglioBoogleFlagPatch[] = {
	// @1e5f
	// ldi 0, sal 0 removed from here (+4 bytes)

	// @1e5f (+4 bytes)
	// local[0] = /* oFlags */ ScriptID(64017, 0);
	0x7a,                        // push2
	0x38, PATCH_UINT16(0xfa11),  // pushi 64017
	0x76,                        // push0
	0x43, 0x02, PATCH_UINT16(4), // callk ScriptID[2], 4
	0xa3, 0x00,                  // sal 0 (-2 bytes)

	// @1e6a (+2 bytes)
	// acc = local[0].test(94);
	0x38, PATCH_SELECTOR16(test), // pushi test
	0x78,                         // push1
	0x39, 0x5e,                   // pushi 94
	0x4a, PATCH_UINT16(0x06),     // send 6

	// @1e73 (+2 bytes)
	// if (!acc) goto elseCase;
	0x30, PATCH_UINT16(0x34),     // bnt 0x31 + 3

	// @1e76 (+2 bytes)
	// global[0].get(ScriptID(64001, 0).get(20));
	0x38, PATCH_SELECTOR16(get), // pushi get
	0x78,                        // push1
	0x38, PATCH_SELECTOR16(get), // pushi get
	0x78,                        // push1
	0x39, 0x14,                  // pushi 20
	0x7a,                        // push2
	0x38, PATCH_UINT16(0xfa01),  // pushi 64001
	0x76,                        // push0
	0x43, 0x02, PATCH_UINT16(4), // callk ScriptID[2], 4
	0x4a, PATCH_UINT16(0x06),    // send 6
	0x36,                        // push
	0x81, 0x00,                  // lag 0 (ego)
	0x4a, PATCH_UINT16(0x06),    // send 6

	// @1e92 (+2 bytes)
	// local[0].set(52);
	0x38, PATCH_SELECTOR16(set), // pushi set
	0x78,                        // push1
	0x39, 0x34,                  // pushi 52
	0x83, 0x00,                  // lal 0 (+7 byte)
	0x4a, PATCH_UINT16(0x06),    // send 6

	// @1e9d (+9 bytes)
	// goto endOfBranch;
	0x33, 0x0b,                  // jmp [to end of conditional branch] (+1 byte)

	// @1e9f (+10 bytes)
	// elseCase: local[0].clear(97);
	0x38, PATCH_SELECTOR16(clear), // pushi clear
	0x78,                          // push1
	0x39, 0x61,                    // pushi 97
	0x83, 0x00,                    // lal 0 (+7 bytes)
	0x4a, PATCH_UINT16(0x06),      // send 6

	// @1eaa (+17 bytes)
	// endOfBranch: local[0].set(232);
	0x38, PATCH_SELECTOR16(set),   // pushi set (-3 bytes)
	0x78,                          // push1 (-1 byte)
	0x38, PATCH_UINT16(0xe8),      // pushi 232 (Boogle-in-bag flag) (-3 bytes)
	0x83, 0x00,                    // lal 0 (-2 bytes)
	0x4a, PATCH_UINT16(0x06),      // send 6 (-3 bytes)

	// @1eb6 (+5 bytes)
	// local[0] = 0; self.dispose();
	0x38, PATCH_SELECTOR16(dispose), // pushi dispose
	0x76,                            // push0
	0x3c,                            // dup (-1 byte)
	0xab, 0x00,                      // ssl 0 (-2 bytes)
	0x54, PATCH_UINT16(4),           // self 4
	0x48,                            // ret

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
			// Abort, in case selectors are not yet initialized (happens for games w/o selector-dictionary)
			if (!g_sci->getKernel()->selectorNamesAvailable())
				return;

			// signature table needs to get initialized (Magic DWORD set, selector table set)
			initSignature(signatureTable);

			// Do additional game-specific initialization
			switch (gameId) {
			case GID_KQ5:
				if (g_sci->_features->useAltWinGMSound()) {
					// See the explanation in the kq5SignatureWinGMSignals comment
					enablePatch(signatureTable, "Win: GM Music signal checks");
				}
				break;
			case GID_KQ6:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for King's Quest 6
					enablePatch(signatureTable, "CD: audio + text support");
				}
				break;
			case GID_LAURABOW2:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for Laura Bow 2
					enablePatch(signatureTable, "CD: audio + text support");
				}
				break;
			case GID_QFG4:
				// Chooses between similar signatures that patch with a different lofsa address
				if (g_sci->isCD()) {
					enablePatch(signatureTable, "CD: fix crest bookshelf");
				} else {
					enablePatch(signatureTable, "Floppy: fix crest bookshelf");
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
						debugC(kDebugLevelScriptPatcher, "Script-Patcher: '%s' on script %d offset %d", curEntry->description, scriptNr, foundOffset);
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
